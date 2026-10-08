#!/usr/bin/env python3
"""
Network Proxy Addon — mitmproxy companion for TemplateTool.

This script runs as a mitmproxy addon and communicates with the Qt client
via a TCP socket using newline-delimited JSON.

Features:
  - Capture HTTP/HTTPS requests and responses
  - Mock rules (regex URL matching, custom response)
  - Breakpoints (pause flow until client resumes)
  - Blacklist (block matching requests)
  - Map Local (serve local file instead of remote)
  - Map Remote (redirect request to different URL)
  - Rewrite request/response headers and bodies
  - Throttle (bandwidth limiting)
  - WebSocket message capture
  - Process identification (macOS/Windows/Linux)

Usage:
  mitmdump -s proxy_addon.py --set proxy_port=8080 --set control_port=9876
  or standalone:
  python proxy_addon.py --proxy-port 8080 --control-port 9876

Communication protocol (JSON over TCP, newline-delimited):
  Addon -> Client:
    {"type":"request",  "flow_id":"...", "method":"GET", "url":"...", ...}
    {"type":"response", "flow_id":"...", "status_code":200, ...}
    {"type":"intercepted", "flow_id":"...", ...}
    {"type":"intercept_finished", "flow_id":"...", "reason":"resumed|dropped|timeout|disconnected|shutdown|disabled|capacity|cancelled"}
    {"type":"status", "message":"..."}
    {"type":"error",  "message":"..."}
    {"type":"proxy_config_result", "session_id":"...", "revision":"1", "accepted":true, "message":"..."}
  Client -> Addon:
    {"type":"apply_proxy_config", "session_id":"...", "revision":"1", "config":{...}}
    {"type":"update_mock_rules",       "rules":[...]}
    {"type":"update_breakpoint_rules", "rules":[...]}
    {"type":"update_blacklist",        "rules":[...]}
    {"type":"update_map_local",        "rules":[...]}
    {"type":"update_map_remote",       "rules":[...]}
    {"type":"update_rewrite_rules",    "rules":[...]}
    {"type":"set_intercept",           "enabled":true}
    {"type":"set_throttle",            "enabled":true, "download_kbps":100, "upload_kbps":50}
    {"type":"resume_flow",             "flow_id":"..."}
    {"type":"drop_flow",               "flow_id":"..."}
"""

import argparse
import asyncio
import json
import logging
import os
import platform
import re
import socket
import subprocess
import threading
import time
from collections.abc import Sequence
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path

import base64

from mitmproxy import http, ctx, websocket
from mitmproxy.flow import Flow

from rewrite_rules import RewriteRule, apply_rules, compile_rules
from proxy_config import ProxyConfig, compile_config, config_identity

logger = logging.getLogger("proxy_addon")

MAX_PENDING_INTERCEPTS = 128
INTERCEPT_TIMEOUT_SECONDS = 300


@dataclass
class PendingIntercept:
    waiter: asyncio.Future[str]
    decision: str | None = None


DEFAULT_AI_BYPASS_HOSTS = [
    # OpenAI / ChatGPT / Codex
    r"(^|\.)openai\.com(:\d+)?$",
    r"(^|\.)chatgpt\.com(:\d+)?$",
    r"(^|\.)oaistatic\.com(:\d+)?$",
    r"(^|\.)oaiusercontent\.com(:\d+)?$",
    r"(^|\.)openaiapi-site\.azureedge\.net(:\d+)?$",
    r"(^|\.)codex\.openai\.com(:\d+)?$",
    # GitHub Copilot
    r"(^|\.)githubcopilot\.com(:\d+)?$",
    r"(^|\.)copilot\.microsoft\.com(:\d+)?$",
    r"(^|\.)copilot-proxy\.githubusercontent\.com(:\d+)?$",
    r"(^|\.)individual\.githubcopilot\.com(:\d+)?$",
    r"(^|\.)business\.githubcopilot\.com(:\d+)?$",
    r"(^|\.)enterprise\.githubcopilot\.com(:\d+)?$",
    # Anthropic / Claude
    r"(^|\.)anthropic\.com(:\d+)?$",
    r"(^|\.)claude\.ai(:\d+)?$",
    # Google Gemini / Bard
    r"(^|\.)gemini\.google\.com(:\d+)?$",
    r"(^|\.)bard\.google\.com(:\d+)?$",
    r"(^|\.)generativelanguage\.googleapis\.com(:\d+)?$",
    r"(^|\.)aistudio\.google\.com(:\d+)?$",
    # DeepSeek
    r"(^|\.)deepseek\.com(:\d+)?$",
    r"(^|\.)deepseek\.ai(:\d+)?$",
    # Mistral
    r"(^|\.)mistral\.ai(:\d+)?$",
    # xAI / Grok
    r"(^|\.)x\.ai(:\d+)?$",
    r"(^|\.)grok\.com(:\d+)?$",
    # Perplexity
    r"(^|\.)perplexity\.ai(:\d+)?$",
    # Cursor
    r"(^|\.)cursor\.sh(:\d+)?$",
    r"(^|\.)cursor\.com(:\d+)?$",
    # Cody / Sourcegraph
    r"(^|\.)sourcegraph\.com(:\d+)?$",
    # Codeium / Windsurf
    r"(^|\.)codeium\.com(:\d+)?$",
    r"(^|\.)windsurf\.ai(:\d+)?$",
    # Tabnine
    r"(^|\.)tabnine\.com(:\d+)?$",
    # Hugging Face
    r"(^|\.)huggingface\.co(:\d+)?$",
    # ByteDance Doubao / Volcengine
    r"(^|\.)doubao\.com(:\d+)?$",
    r"(^|\.)volces\.com(:\d+)?$",
    r"(^|\.)volcengineapi\.com(:\d+)?$",
    # Moonshot / Kimi
    r"(^|\.)moonshot\.cn(:\d+)?$",
    r"(^|\.)moonshot\.ai(:\d+)?$",
    r"(^|\.)kimi\.com(:\d+)?$",
    # Zhipu / GLM
    r"(^|\.)bigmodel\.cn(:\d+)?$",
    r"(^|\.)zhipuai\.cn(:\d+)?$",
    # Alibaba Qwen / Tongyi
    r"(^|\.)tongyi\.aliyun\.com(:\d+)?$",
    r"(^|\.)dashscope\.aliyuncs\.com(:\d+)?$",
    r"(^|\.)qwen\.ai(:\d+)?$",
    # Baidu Wenxin / ERNIE
    r"(^|\.)wenxin\.baidu\.com(:\d+)?$",
    r"(^|\.)yiyan\.baidu\.com(:\d+)?$",
    # Tencent Hunyuan
    r"(^|\.)hunyuan\.tencent\.com(:\d+)?$",
    # 01.AI / Yi
    r"(^|\.)01\.ai(:\d+)?$",
    r"(^|\.)lingyiwanwu\.com(:\d+)?$",
    # MiniMax
    r"(^|\.)minimax\.chat(:\d+)?$",
    r"(^|\.)minimaxi\.com(:\d+)?$",
    # Baichuan
    r"(^|\.)baichuan-ai\.com(:\d+)?$",
    # SenseTime
    r"(^|\.)sensetime\.com(:\d+)?$",
    r"(^|\.)sensenova\.cn(:\d+)?$",
    # Stepfun
    r"(^|\.)stepfun\.com(:\d+)?$",
]


def _encode_body(raw_bytes: bytes, content_type: str = "") -> tuple[str, bool]:
    """Encode body bytes for JSON transport.

    Returns (body_string, is_base64).
    - For text-like content: decoded string, False
    - For binary content: base64-encoded string, True
    """
    if not raw_bytes:
        return "", False

    ct = content_type.lower()
    text_indicators = [
        "text/", "json", "xml", "html", "form-urlencoded",
        "javascript", "ecmascript", "css", "csv", "yaml",
        "x-www-form-urlencoded", "soap", "graphql",
    ]
    is_likely_text = any(t in ct for t in text_indicators) or not ct

    if is_likely_text:
        try:
            return raw_bytes.decode("utf-8"), False
        except UnicodeDecodeError:
            pass
        # Try latin-1 which is lossless for any byte value 0-255
        try:
            return raw_bytes.decode("latin-1"), False
        except Exception:
            pass

    # Try UTF-8 strict even for "binary" content types (some APIs use wrong CT)
    try:
        return raw_bytes.decode("utf-8"), False
    except UnicodeDecodeError:
        pass

    # Fall back to base64 for truly binary content
    return base64.b64encode(raw_bytes).decode("ascii"), True


# ──────────────────────────────────────────────────────────────
# TCP Client — connects to the Qt control server
# ──────────────────────────────────────────────────────────────

class TcpClient:
    """Thread-safe TCP client that connects to the Qt control server."""

    def __init__(self, host: str, port: int):
        self._host = host
        self._port = port
        self._sock: socket.socket | None = None
        self._lock = threading.RLock()
        self._connected = False
        self._connection_generation = 0
        self._running = True
        self._recv_thread: threading.Thread | None = None
        self._reconnect_thread: threading.Thread | None = None
        self._on_message = None
        self._on_disconnect = None

    @property
    def connected(self) -> bool:
        with self._lock:
            return self._connected

    def set_message_handler(self, handler):
        """Set callback: handler(dict, generation) for each received JSON message."""
        self._on_message = handler

    def run_if_current(self, generation: int, callback, *args):
        """Keep a queued command and its reply attached to one TCP connection.

        The callback is synchronous. Holding the connection lock prevents the
        receive/reconnect threads from replacing its socket during a commit.
        """
        with self._lock:
            if self._connected and generation == self._connection_generation:
                callback(*args)

    def set_disconnect_handler(self, handler):
        """Set callback: handler() when an established connection is lost."""
        self._on_disconnect = handler

    def start(self):
        """Start connection and reconnect loop."""
        self._running = True
        self._reconnect_thread = threading.Thread(
            target=self._reconnect_loop, daemon=True
        )
        self._reconnect_thread.start()

    def stop(self):
        """Shut down the client."""
        self._running = False
        self._mark_disconnected()

    def send(self, data: dict) -> bool:
        """Send a JSON message (newline-delimited)."""
        with self._lock:
            if not self._connected or not self._sock:
                logger.warning("[ADDON-DEBUG] TCP send skipped: connected=%s, sock=%s", self._connected, self._sock is not None)
                return False
            sock = self._sock
            try:
                raw = json.dumps(data, ensure_ascii=False).encode("utf-8") + b"\n"
                sock.sendall(raw)
                logger.info("[ADDON-DEBUG] TCP sent %d bytes, type=%s", len(raw), data.get('type', '?'))
                return True
            except OSError as e:
                logger.warning("TCP send error: %s", e)
                self._mark_disconnected(sock)
                return False

    # ── internal ──

    def _reconnect_loop(self):
        while self._running:
            if not self._connected:
                self._try_connect()
            time.sleep(2)

    def _try_connect(self):
        s = None
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(5)
            s.connect((self._host, self._port))
            s.settimeout(None)
            with self._lock:
                if not self._running:
                    s.close()
                    return
                self._sock = s
                self._connected = True
                self._connection_generation += 1
                generation = self._connection_generation
            logger.info("Connected to control server %s:%d", self._host, self._port)
            self._recv_thread = threading.Thread(
                target=self._recv_loop, args=(s, generation), daemon=True
            )
            self._recv_thread.start()
        except OSError as e:
            if s:
                s.close()
            logger.debug("Connect attempt failed: %s", e)

    def _recv_loop(self, sock: socket.socket, generation: int):
        recv_buffer = b""
        while self._running:
            try:
                with self._lock:
                    if sock is not self._sock or not self._connected:
                        break
                data = sock.recv(65536)
                if not data:
                    self._mark_disconnected(sock)
                    break
                recv_buffer += data
                recv_buffer = self._process_buffer(recv_buffer, generation)
            except OSError:
                self._mark_disconnected(sock)
                break

    def _process_buffer(self, recv_buffer: bytes, generation: int) -> bytes:
        while b"\n" in recv_buffer:
            line, recv_buffer = recv_buffer.split(b"\n", 1)
            if not line.strip():
                continue
            try:
                msg = json.loads(line.decode("utf-8"))
                if self._on_message:
                    self._on_message(msg, generation)
            except (json.JSONDecodeError, UnicodeDecodeError) as e:
                logger.warning("Bad JSON from control: %s", e)
        return recv_buffer

    def _mark_disconnected(self, expected_sock: socket.socket | None = None):
        with self._lock:
            if expected_sock is not None and expected_sock is not self._sock:
                return
            was_connected = self._connected
            if self._sock:
                try:
                    self._sock.shutdown(socket.SHUT_RDWR)
                except OSError:
                    pass
                try:
                    self._sock.close()
                except OSError:
                    pass
                self._sock = None
            self._connected = False
        if was_connected:
            logger.info("Disconnected from control server")
            if self._on_disconnect:
                self._on_disconnect()


# ──────────────────────────────────────────────────────────────
# Process Identification
# ──────────────────────────────────────────────────────────────

def identify_process(src_port: int) -> str:
    """Try to identify which process owns the given source port."""
    system = platform.system()
    try:
        if system == "Darwin":
            out = subprocess.check_output(
                ["lsof", "-i", f"TCP:{src_port}", "-sTCP:ESTABLISHED", "-n", "-P"],
                timeout=2, stderr=subprocess.DEVNULL
            ).decode()
            for line in out.strip().split("\n")[1:]:
                parts = line.split()
                if len(parts) >= 1:
                    return parts[0]
        elif system == "Windows":
            out = subprocess.check_output(
                ["netstat", "-ano", "-p", "TCP"],
                timeout=2, stderr=subprocess.DEVNULL
            ).decode()
            for line in out.strip().split("\n"):
                if "ESTABLISHED" not in line:
                    continue
                parts = line.split()
                # netstat format: Proto  LocalAddr  ForeignAddr  State  PID
                # We need to match the LOCAL port (client's ephemeral port)
                # which connects to the proxy (foreign addr should be 127.0.0.1:proxy_port)
                if len(parts) < 5:
                    continue
                local_addr = parts[1]   # e.g. "127.0.0.1:54321"
                # Extract the port from local address
                local_port_str = local_addr.rsplit(":", 1)[-1]
                if local_port_str == str(src_port):
                    pid = parts[-1]
                    try:
                        name_out = subprocess.check_output(
                            ["tasklist", "/FI", f"PID eq {pid}", "/FO", "CSV"],
                            timeout=2, stderr=subprocess.DEVNULL
                        ).decode()
                        for row in name_out.strip().split("\n")[1:]:
                            return row.split(",")[0].strip('"')
                    except Exception:
                        return f"PID:{pid}"
        elif system == "Linux":
            out = subprocess.check_output(
                ["ss", "-tnp", f"sport = :{src_port}"],
                timeout=2, stderr=subprocess.DEVNULL
            ).decode()
            for line in out.strip().split("\n")[1:]:
                match = re.search(r'users:\(\("([^"]+)"', line)
                if match:
                    return match.group(1)
    except Exception:
        pass
    return ""


# ──────────────────────────────────────────────────────────────
# The Mitmproxy Addon
# ──────────────────────────────────────────────────────────────

class ProxyAddon:
    """Mitmproxy addon that captures traffic and communicates with Qt client."""

    def __init__(self, control_host: str = "127.0.0.1", control_port: int = 9876):
        self._tcp = TcpClient(control_host, control_port)
        self._tcp.set_message_handler(self._queue_command)
        self._tcp.set_disconnect_handler(self._on_disconnected)
        self._lock = threading.RLock()
        self._loop: asyncio.AbstractEventLoop | None = None
        self._shutting_down = False
        self._config_connection_generation: int | None = None
        self._config_session_id: str | None = None
        self._config_revision = -1

        # Rules (protected by _lock)
        self._mock_rules: Sequence[dict] = ()
        self._breakpoint_rules: Sequence[dict] = ()
        self._blacklist_rules: Sequence[dict] = ()
        self._map_local_rules: Sequence[dict] = ()
        self._map_remote_rules: Sequence[dict] = ()
        self._rewrite_rules: tuple[RewriteRule, ...] = ()
        self._bypass_hosts: list[str] = list(DEFAULT_AI_BYPASS_HOSTS)

        # Intercept state
        self._intercept_enabled = False
        # Accessed on the mitmproxy event loop, or by done() after it has stopped.
        self._pending_intercepts: dict[str, PendingIntercept] = {}

        # Throttle
        self._throttle_enabled = False
        self._throttle_dl_kbps = 0
        self._throttle_ul_kbps = 0

    def load(self, loader):
        """Called by mitmproxy on addon load."""
        self._apply_bypass_hosts()

    def running(self):
        """Start control I/O only after mitmproxy's event loop is available."""
        self._loop = asyncio.get_running_loop()
        self._tcp.start()
        logger.info("ProxyAddon running, connecting to control server...")

    def _normalize_bypass_hosts(self, hosts) -> list[str]:
        normalized: list[str] = []
        if not isinstance(hosts, list):
            return normalized
        for item in hosts:
            if isinstance(item, str):
                value = item.strip()
                if value:
                    normalized.append(value)
            elif isinstance(item, dict):
                value = str(item.get("host_pattern", "")).strip()
                if value:
                    normalized.append(value)
        return normalized

    def _apply_bypass_hosts(self):
        """Configure mitmproxy passthrough hosts to avoid TLS MITM for pinned clients."""
        try:
            ctx.options.update(ignore_hosts=self._bypass_hosts)
            logger.info("Configured %d bypass hosts for passthrough", len(self._bypass_hosts))
            self._tcp.send({
                "type": "status",
                "message": f"Bypass hosts active: {len(self._bypass_hosts)}",
            })
        except Exception as exc:
            logger.warning("Failed to apply bypass hosts: %s", exc)

    def done(self):
        """Called by mitmproxy on shutdown."""
        self._shutting_down = True
        self._release_intercepts("shutdown")
        # done() may run after the event loop stops. Finalize directly because
        # suspended request hooks may never get another turn to run finally.
        for flow_id, pending in list(self._pending_intercepts.items()):
            self._finish_intercept(flow_id, pending)
        self._tcp.stop()

    def _schedule_on_loop(self, callback, *args):
        loop = self._loop
        if self._shutting_down or loop is None or loop.is_closed():
            return
        try:
            loop.call_soon_threadsafe(self._run_callback, callback, args)
        except RuntimeError:
            # The loop can close between is_closed() and scheduling.
            logger.debug("Control callback ignored while event loop shuts down")

    def _run_callback(self, callback, args):
        if not self._shutting_down:
            callback(*args)

    def _queue_command(self, msg: dict, generation: int):
        if isinstance(msg, dict):
            self._schedule_on_loop(self._dispatch_command, msg, generation)
        else:
            logger.warning("Control message must be a JSON object")

    def _dispatch_command(self, msg: dict, generation: int):
        # A previous receive thread may already have queued commands when a new
        # socket connects. Never let those commands mutate the new session.
        self._tcp.run_if_current(generation, self._on_current_command, msg, generation)

    def _on_current_command(self, msg: dict, generation: int):
        if self._config_connection_generation != generation:
            self._config_connection_generation = generation
            self._config_session_id = None
            self._config_revision = -1
        self._on_command(msg)

    def _on_disconnected(self):
        self._schedule_on_loop(self._release_intercepts, "disconnected")

    def _resolve_intercept(self, flow_id: str, reason: str):
        if not isinstance(flow_id, str):
            return
        pending = self._pending_intercepts.get(flow_id)
        if pending is None or pending.decision is not None:
            return
        pending.decision = reason
        if not pending.waiter.done():
            try:
                pending.waiter.set_result(reason)
            except RuntimeError:
                # done() is also called when the loop has already closed.
                logger.debug("Intercept resolved after event loop closed: %s", flow_id)

    def _release_intercepts(self, reason: str):
        for flow_id in list(self._pending_intercepts):
            self._resolve_intercept(flow_id, reason)

    def _report_intercept_finished(self, flow_id: str, reason: str):
        self._tcp.send({
            "type": "intercept_finished",
            "flow_id": flow_id,
            "reason": reason,
        })

    def _finish_intercept(self, flow_id: str, pending: PendingIntercept):
        if self._pending_intercepts.get(flow_id) is not pending:
            return
        self._pending_intercepts.pop(flow_id)
        self._report_intercept_finished(flow_id, pending.decision or "cancelled")

    async def _wait_for_intercept(self, flow: http.HTTPFlow):
        if self._shutting_down:
            self._report_intercept_finished(flow.id, "shutdown")
            return
        if not self._tcp.connected:
            self._report_intercept_finished(flow.id, "disconnected")
            return
        if len(self._pending_intercepts) >= MAX_PENDING_INTERCEPTS:
            self._report_intercept_finished(flow.id, "capacity")
            self._tcp.send({
                "type": "status",
                "message": f"Intercept limit reached ({MAX_PENDING_INTERCEPTS}); request forwarded",
            })
            return

        pending = PendingIntercept(asyncio.get_running_loop().create_future())
        self._pending_intercepts[flow.id] = pending
        try:
            if not self._tcp.send({
                "type": "intercepted",
                "flow_id": flow.id,
                "method": flow.request.method,
                "url": flow.request.pretty_url,
            }):
                self._resolve_intercept(flow.id, "disconnected")

            try:
                # Shield keeps timeout/cancellation from overwriting a command
                # decision that has already completed the underlying future.
                await asyncio.wait_for(
                    asyncio.shield(pending.waiter), timeout=INTERCEPT_TIMEOUT_SECONDS
                )
            except asyncio.TimeoutError:
                self._resolve_intercept(flow.id, "timeout")
            except asyncio.CancelledError:
                self._resolve_intercept(flow.id, "cancelled")
                raise

            if pending.decision == "dropped":
                flow.kill()
        finally:
            self._finish_intercept(flow.id, pending)

    def _report_request(self, flow: http.HTTPFlow, tag: str = ""):
        """Send request info to the UI capture list."""
        src_port = flow.client_conn.peername[1] if flow.client_conn.peername else 0
        process_name = identify_process(src_port) if src_port else ""

        request_body = ""
        request_body_base64 = False
        content = flow.request.get_content(strict=False)
        if content:
            raw = content[:102400]
            req_ct = flow.request.headers.get("content-type", "")
            request_body, request_body_base64 = _encode_body(raw, req_ct)

        msg = {
            "type": "request",
            "flow_id": flow.id,
            "method": flow.request.method,
            "url": flow.request.pretty_url,
            "is_https": flow.request.scheme == "https",
            "is_websocket": False,
            "timestamp": datetime.now(timezone.utc).isoformat(),
            "process_name": process_name,
            "request_headers": dict(flow.request.headers),
            "request_content_length": len(content) if content else 0,
            "request_body": request_body,
            "request_body_base64": request_body_base64,
            **self._rewrite_feedback(flow, "request"),
        }
        if tag:
            msg["tag"] = tag
        self._tcp.send(msg)

    def _report_response(self, flow: http.HTTPFlow):
        """Report the final response after rewrites, including synthetic responses."""
        if not flow.response:
            return
        ct = flow.response.headers.get("content-type", "")
        body = ""
        body_base64 = False
        content = flow.response.get_content(strict=False)
        if content:
            raw = content[:102400]
            body, body_base64 = _encode_body(raw, ct)

        tag = flow.metadata.get("template_tool_source_tag", "")
        duration = 0
        if not tag and flow.response.timestamp_end and flow.request.timestamp_start:
            duration = flow.response.timestamp_end - flow.request.timestamp_start
        msg = {
            "type": "response",
            "flow_id": flow.id,
            "status_code": flow.response.status_code,
            "response_content_type": ct,
            "response_content_length": len(content) if content else 0,
            "duration": duration,
            "response_headers": dict(flow.response.headers),
            "response_body": body,
            "response_body_base64": body_base64,
            **self._rewrite_feedback(flow, "response"),
        }
        if tag:
            msg["tag"] = tag
        self._tcp.send(msg)

    @staticmethod
    def _rewrite_feedback(flow: http.HTTPFlow, stage: str) -> dict:
        return {
            f"{stage}_rewrite_rules": flow.metadata.get(f"{stage}_rewrite_rules", []),
            f"{stage}_rewrite_errors": flow.metadata.get(f"{stage}_rewrite_errors", []),
        }

    def _rewrite_message(self, flow: http.HTTPFlow, stage: str):
        # Synthetic and server responses use the same hook. Guard each stage so
        # an accidentally repeated hook cannot apply a non-idempotent rule twice.
        key = f"{stage}_rewrite_rules"
        if key in flow.metadata:
            return
        with self._lock:
            rules = self._rewrite_rules
        applied, errors = apply_rules(flow, stage, rules)
        flow.metadata[key] = applied
        flow.metadata[f"{stage}_rewrite_errors"] = errors
        for error in errors:
            logger.warning("Rewrite %s (%s): %s", stage, error["rule_id"], error["error"])

    # ── mitmproxy hooks ──

    async def request(self, flow: http.HTTPFlow):
        """Called for each HTTP request."""
        logger.info("[ADDON-DEBUG] request hook called: %s %s", flow.request.method, flow.request.pretty_url)
        # Blacklist check
        with self._lock:
            for rule in self._blacklist_rules:
                pattern = rule.get("url_pattern", "")
                if pattern and re.search(pattern, flow.request.pretty_url):
                    self._report_request(flow, tag="BLOCKED")
                    flow.kill()
                    return

        # Map Remote check
        with self._lock:
            for rule in self._map_remote_rules:
                src = rule.get("src_pattern", "")
                dest = rule.get("dest_url", "")
                if src and dest and re.search(src, flow.request.pretty_url):
                    new_url = re.sub(src, dest, flow.request.pretty_url)
                    flow.request.url = new_url
                    break

        # Rewrite the full outbound message after any URL redirect. Captured
        # bodies are only a preview and never serve as the rewrite source.
        self._rewrite_message(flow, "request")

        # Map Local check
        with self._lock:
            for rule in self._map_local_rules:
                pattern = rule.get("url_pattern", "")
                local_path = rule.get("local_path", "")
                if pattern and local_path and re.search(pattern, flow.request.pretty_url):
                    p = Path(local_path)
                    if p.is_file():
                        content = p.read_bytes()
                        # Guess content type from extension
                        ext = p.suffix.lower()
                        ct_map = {
                            ".json": "application/json",
                            ".html": "text/html",
                            ".js": "application/javascript",
                            ".css": "text/css",
                            ".xml": "application/xml",
                            ".txt": "text/plain",
                        }
                        ct = ct_map.get(ext, "application/octet-stream")
                        flow.response = http.Response.make(200, content, {"Content-Type": ct})
                        flow.metadata["template_tool_source_tag"] = "MAP-LOCAL"
                        self._report_request(flow, tag="MAP-LOCAL")
                        return

        # Mock rule check
        with self._lock:
            for rule in self._mock_rules:
                pattern = rule.get("url_pattern", "")
                if pattern and re.search(pattern, flow.request.pretty_url):
                    status = rule.get("status_code", 200)
                    ct = rule.get("content_type", "application/json")
                    body = rule.get("body", "")
                    # Build response headers
                    resp_headers = {"Content-Type": ct}
                    extra_headers = rule.get("headers", "")
                    if extra_headers:
                        # Parse "Header: value" lines (split by newline or comma)
                        for line in re.split(r'[\n,]', extra_headers):
                            line = line.strip()
                            if ':' in line:
                                k, v = line.split(':', 1)
                                resp_headers[k.strip()] = v.strip()
                    flow.response = http.Response.make(
                        status,
                        body.encode("utf-8") if isinstance(body, str) else body,
                        resp_headers,
                    )
                    flow.metadata["template_tool_source_tag"] = "MOCK"
                    self._report_request(flow, tag="MOCK")
                    return

        self._report_request(flow)

        # Breakpoint check
        should_intercept = False
        with self._lock:
            if self._intercept_enabled:
                for rule in self._breakpoint_rules:
                    pattern = rule.get("url_pattern", "")
                    method = rule.get("method", "ANY")
                    if pattern and re.search(pattern, flow.request.pretty_url):
                        if method == "ANY" or method == flow.request.method:
                            should_intercept = True
                            break

        if should_intercept:
            await self._wait_for_intercept(flow)

    async def response(self, flow: http.HTTPFlow):
        """Called for each HTTP response."""
        logger.info("[ADDON-DEBUG] response hook called: %s %s -> %s",
                    flow.request.method, flow.request.pretty_url,
                    flow.response.status_code if flow.response else 'None')
        if not flow.response:
            return

        self._rewrite_message(flow, "response")

        # Throttle simulation (simple delay based on content size)
        if self._throttle_enabled and self._throttle_dl_kbps > 0:
            content = flow.response.get_content(strict=False)
            size_kb = len(content) / 1024 if content else 0
            delay = size_kb / self._throttle_dl_kbps
            if delay > 0:
                await asyncio.sleep(min(delay, 30))  # Cap at 30s

        self._report_response(flow)

    def websocket_message(self, flow: http.HTTPFlow):
        """Called for each WebSocket message."""
        if not flow.websocket:
            return
        msg = flow.websocket.messages[-1]
        self._tcp.send({
            "type": "request",
            "flow_id": flow.id + f"_ws_{len(flow.websocket.messages)}",
            "method": "WS",
            "url": flow.request.pretty_url,
            "is_https": flow.request.scheme == "https",
            "is_websocket": True,
            "timestamp": datetime.now(timezone.utc).isoformat(),
            "process_name": "",
            "ws_direction": "client" if msg.from_client else "server",
            "ws_content": msg.text if msg.is_text else f"<binary {len(msg.content)} bytes>",
            "request_content_length": len(msg.content) if msg.content else 0,
            "request_rewrite_rules": [],
            "request_rewrite_errors": [],
        })

    # ── command handler ──

    def _reply_proxy_config(self, msg: dict, accepted: bool, message: str):
        # The validated path echoes the exact strings sent by the client. For
        # malformed envelopes, bound echoed values as well as the error text.
        session_id = msg.get("session_id")
        revision = msg.get("revision")
        self._tcp.send({
            "type": "proxy_config_result",
            "session_id": session_id if isinstance(session_id, str) and len(session_id) <= 128 else "",
            "revision": revision if isinstance(revision, str) and len(revision) <= 20 else "",
            "accepted": accepted,
            "message": message[:1024],
        })

    def _apply_proxy_config(self, msg: dict):
        try:
            session_id, revision = config_identity(msg)
        except ValueError as exc:
            self._reply_proxy_config(msg, False, str(exc))
            return

        if self._config_session_id is not None and session_id != self._config_session_id:
            self._reply_proxy_config(msg, False, "session_id changed within the same connection")
            return
        if revision <= self._config_revision:
            self._reply_proxy_config(msg, False, "configuration revision is stale")
            return
        # A rejected newer configuration also advances the version gate. An
        # older snapshot must never subsequently replace the active state.
        self._config_session_id = session_id
        self._config_revision = revision

        try:
            candidate = compile_config(msg.get("config"))
            # mitmproxy rolls this option update back if a configure hook
            # rejects it. Do not use _apply_bypass_hosts(), which swallows errors.
            # The transaction stays on the event loop without yielding, so no
            # request can observe the new bypass hosts with old Python rules.
            ctx.options.update(ignore_hosts=list(candidate.bypass_hosts))
        except Exception as exc:
            logger.warning("Rejected proxy configuration %s: %s", revision, exc)
            self._reply_proxy_config(msg, False, str(exc))
            return

        self._install_proxy_config(candidate)
        if not candidate.intercept_enabled:
            self._release_intercepts("disabled")
        self._reply_proxy_config(msg, True, "Proxy configuration applied")
        logger.info("Applied proxy configuration %s (session %s)", revision, session_id)

    def _install_proxy_config(self, candidate: ProxyConfig):
        with self._lock:
            self._mock_rules = candidate.mock_rules
            self._breakpoint_rules = candidate.breakpoint_rules
            self._blacklist_rules = candidate.blacklist_rules
            self._map_local_rules = candidate.map_local_rules
            self._map_remote_rules = candidate.map_remote_rules
            self._rewrite_rules = candidate.rewrite_rules
            self._bypass_hosts = list(candidate.bypass_hosts)
            self._throttle_enabled = candidate.throttle_enabled
            self._throttle_dl_kbps = candidate.download_kbps
            self._throttle_ul_kbps = candidate.upload_kbps
            self._intercept_enabled = candidate.intercept_enabled

    def _on_command(self, msg: dict):
        """Handle a command from the Qt client."""
        cmd_type = msg.get("type", "")

        if cmd_type == "apply_proxy_config":
            self._apply_proxy_config(msg)
            return

        with self._lock:
            if cmd_type == "update_mock_rules":
                self._mock_rules = msg.get("rules", [])
                logger.info("Updated %d mock rules", len(self._mock_rules))

            elif cmd_type == "update_breakpoint_rules":
                self._breakpoint_rules = msg.get("rules", [])
                logger.info("Updated %d breakpoint rules", len(self._breakpoint_rules))

            elif cmd_type == "update_blacklist":
                self._blacklist_rules = msg.get("rules", [])
                logger.info("Updated %d blacklist rules", len(self._blacklist_rules))

            elif cmd_type == "update_map_local":
                self._map_local_rules = msg.get("rules", [])
                logger.info("Updated %d map-local rules", len(self._map_local_rules))

            elif cmd_type == "update_map_remote":
                self._map_remote_rules = msg.get("rules", [])
                logger.info("Updated %d map-remote rules", len(self._map_remote_rules))

            elif cmd_type == "update_rewrite_rules":
                try:
                    rules = compile_rules(msg.get("rules"))
                except ValueError as exc:
                    logger.warning("Rejected rewrite configuration: %s", exc)
                    self._tcp.send({"type": "error", "message": str(exc)})
                    return
                self._rewrite_rules = rules
                logger.info("Updated %d rewrite rules", len(rules))

            elif cmd_type == "set_intercept":
                self._intercept_enabled = msg.get("enabled", False)
                if not self._intercept_enabled:
                    self._release_intercepts("disabled")
                logger.info("Intercept %s", "enabled" if self._intercept_enabled else "disabled")

            elif cmd_type == "set_throttle":
                self._throttle_enabled = msg.get("enabled", False)
                self._throttle_dl_kbps = msg.get("download_kbps", 0)
                self._throttle_ul_kbps = msg.get("upload_kbps", 0)
                logger.info("Throttle %s (DL=%dKB/s UL=%dKB/s)",
                            "on" if self._throttle_enabled else "off",
                            self._throttle_dl_kbps, self._throttle_ul_kbps)

            elif cmd_type == "update_bypass_hosts":
                hosts = msg.get("hosts", msg.get("rules", []))
                self._bypass_hosts = self._normalize_bypass_hosts(hosts)
                self._apply_bypass_hosts()
                logger.info("Updated bypass hosts: %d", len(self._bypass_hosts))

            elif cmd_type == "resume_flow":
                flow_id = msg.get("flow_id", "")
                self._resolve_intercept(flow_id, "resumed")

            elif cmd_type == "drop_flow":
                flow_id = msg.get("flow_id", "")
                self._resolve_intercept(flow_id, "dropped")

            else:
                logger.warning("Unknown command: %s", cmd_type)


# ──────────────────────────────────────────────────────────────
# Addon instance (used when loaded by mitmdump -s)
# ──────────────────────────────────────────────────────────────

# Default instance; control_port can be overridden via mitmproxy options
addons = [ProxyAddon()]


# ──────────────────────────────────────────────────────────────
# Standalone entry point
# ──────────────────────────────────────────────────────────────

def main():
    """Run as a standalone script using mitmproxy's Python API."""
    parser = argparse.ArgumentParser(description="Network Proxy Addon for TemplateTool")
    parser.add_argument("--proxy-port", type=int, default=8080,
                        help="HTTP proxy listen port (default: 8080)")
    parser.add_argument("--control-port", type=int, default=9876,
                        help="TCP control server port (default: 9876)")
    parser.add_argument("--verbose", action="store_true",
                        help="Enable verbose logging")
    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
    )

    try:
        from mitmproxy.tools import dump
        from mitmproxy.options import Options
    except ImportError:
        logger.error("mitmproxy is not installed. Run: pip install mitmproxy")
        return

    # Create addon with the correct control port
    addon = ProxyAddon(control_port=args.control_port)

    # Override the global addons list (used when loaded as a script)
    global addons
    addons = [addon]

    logger.info("Starting mitmdump on port %d, control port %d",
                args.proxy_port, args.control_port)

    # Use mitmproxy's Python API directly (works with PyInstaller)
    # DumpMaster requires a running asyncio event loop
    async def run_proxy():
        opts = Options(listen_port=args.proxy_port)
        master = dump.DumpMaster(opts)
        master.addons.add(addon)
        try:
            await master.run()
        except KeyboardInterrupt:
            master.shutdown()

    asyncio.run(run_proxy())


if __name__ == "__main__":
    main()
