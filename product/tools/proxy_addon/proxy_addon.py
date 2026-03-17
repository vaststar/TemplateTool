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
    {"type":"status", "message":"..."}
    {"type":"error",  "message":"..."}
  Client -> Addon:
    {"type":"update_mock_rules",       "rules":[...]}
    {"type":"update_breakpoint_rules", "rules":[...]}
    {"type":"update_blacklist",        "rules":[...]}
    {"type":"update_map_local",        "rules":[...]}
    {"type":"update_map_remote",       "rules":[...]}
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
from datetime import datetime, timezone
from pathlib import Path

import base64

from mitmproxy import http, ctx, websocket
from mitmproxy.flow import Flow

logger = logging.getLogger("proxy_addon")


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
        self._recv_buffer = b""
        self._connected = False
        self._running = True
        self._recv_thread: threading.Thread | None = None
        self._reconnect_thread: threading.Thread | None = None
        self._on_message = None

    @property
    def connected(self) -> bool:
        with self._lock:
            return self._connected

    def set_message_handler(self, handler):
        """Set callback: handler(dict) for each received JSON message."""
        self._on_message = handler

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
        with self._lock:
            if self._sock:
                try:
                    self._sock.close()
                except OSError:
                    pass
                self._sock = None
            self._connected = False

    def send(self, data: dict):
        """Send a JSON message (newline-delimited)."""
        with self._lock:
            if not self._connected or not self._sock:
                logger.warning("[ADDON-DEBUG] TCP send skipped: connected=%s, sock=%s", self._connected, self._sock is not None)
                return
            try:
                raw = json.dumps(data, ensure_ascii=False).encode("utf-8") + b"\n"
                self._sock.sendall(raw)
                logger.info("[ADDON-DEBUG] TCP sent %d bytes, type=%s", len(raw), data.get('type', '?'))
            except OSError as e:
                logger.warning("TCP send error: %s", e)
                self._mark_disconnected()

    # ── internal ──

    def _reconnect_loop(self):
        while self._running:
            if not self._connected:
                self._try_connect()
            time.sleep(2)

    def _try_connect(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(5)
            s.connect((self._host, self._port))
            s.settimeout(None)
            with self._lock:
                self._sock = s
                self._connected = True
                self._recv_buffer = b""
            logger.info("Connected to control server %s:%d", self._host, self._port)
            self._recv_thread = threading.Thread(
                target=self._recv_loop, daemon=True
            )
            self._recv_thread.start()
        except OSError as e:
            logger.debug("Connect attempt failed: %s", e)

    def _recv_loop(self):
        while self._running and self._connected:
            try:
                with self._lock:
                    sock = self._sock
                if not sock:
                    break
                data = sock.recv(65536)
                if not data:
                    self._mark_disconnected()
                    break
                self._recv_buffer += data
                self._process_buffer()
            except OSError:
                self._mark_disconnected()
                break

    def _process_buffer(self):
        while b"\n" in self._recv_buffer:
            line, self._recv_buffer = self._recv_buffer.split(b"\n", 1)
            if not line.strip():
                continue
            try:
                msg = json.loads(line.decode("utf-8"))
                if self._on_message:
                    self._on_message(msg)
            except (json.JSONDecodeError, UnicodeDecodeError) as e:
                logger.warning("Bad JSON from control: %s", e)

    def _mark_disconnected(self):
        with self._lock:
            if self._sock:
                try:
                    self._sock.close()
                except OSError:
                    pass
                self._sock = None
            self._connected = False
        logger.info("Disconnected from control server")


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
                if f":{src_port}" in line and "ESTABLISHED" in line:
                    parts = line.split()
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
        self._tcp.set_message_handler(self._on_command)
        self._lock = threading.RLock()

        # Rules (protected by _lock)
        self._mock_rules: list[dict] = []
        self._breakpoint_rules: list[dict] = []
        self._blacklist_rules: list[dict] = []
        self._map_local_rules: list[dict] = []
        self._map_remote_rules: list[dict] = []

        # Intercept state
        self._intercept_enabled = False
        self._intercepted_flows: dict[str, threading.Event] = {}
        self._dropped_flows: set[str] = set()

        # Throttle
        self._throttle_enabled = False
        self._throttle_dl_kbps = 0
        self._throttle_ul_kbps = 0

    def load(self, loader):
        """Called by mitmproxy on addon load."""
        self._tcp.start()
        logger.info("ProxyAddon loaded, connecting to control server...")

    def done(self):
        """Called by mitmproxy on shutdown."""
        self._tcp.stop()

    def _report_request(self, flow: http.HTTPFlow, tag: str = ""):
        """Send request info to the UI capture list."""
        src_port = flow.client_conn.peername[1] if flow.client_conn.peername else 0
        process_name = identify_process(src_port) if src_port else ""

        request_body = ""
        request_body_base64 = False
        if flow.request.content:
            raw = flow.request.content[:102400]
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
            "request_content_length": len(flow.request.content) if flow.request.content else 0,
            "request_body": request_body,
            "request_body_base64": request_body_base64,
        }
        if tag:
            msg["tag"] = tag
        self._tcp.send(msg)

    def _report_synthetic_response(self, flow: http.HTTPFlow, tag: str = ""):
        """Send a synthetic response (mock/map-local) to the UI capture list."""
        if not flow.response:
            return
        ct = flow.response.headers.get("content-type", "")
        body = ""
        body_base64 = False
        if flow.response.content:
            raw = flow.response.content[:102400]
            body, body_base64 = _encode_body(raw, ct)

        self._tcp.send({
            "type": "response",
            "flow_id": flow.id,
            "status_code": flow.response.status_code,
            "response_content_type": ct,
            "response_content_length": len(flow.response.content) if flow.response.content else 0,
            "duration": 0,
            "response_headers": dict(flow.response.headers),
            "response_body": body,
            "response_body_base64": body_base64,
            "tag": tag,
        })

    # ── mitmproxy hooks ──

    def request(self, flow: http.HTTPFlow):
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
                        self._report_request(flow, tag="MAP-LOCAL")
                        self._report_synthetic_response(flow, tag="MAP-LOCAL")
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
                    self._report_request(flow, tag="MOCK")
                    self._report_synthetic_response(flow, tag="MOCK")
                    return

        # Identify source process
        src_port = flow.client_conn.peername[1] if flow.client_conn.peername else 0
        process_name = identify_process(src_port) if src_port else ""

        # Build request body string (truncate at 100KB)
        request_body = ""
        request_body_base64 = False
        if flow.request.content:
            raw = flow.request.content[:102400]
            req_ct = flow.request.headers.get("content-type", "")
            request_body, request_body_base64 = _encode_body(raw, req_ct)

        # Send request info to client
        self._tcp.send({
            "type": "request",
            "flow_id": flow.id,
            "method": flow.request.method,
            "url": flow.request.pretty_url,
            "is_https": flow.request.scheme == "https",
            "is_websocket": False,
            "timestamp": datetime.now(timezone.utc).isoformat(),
            "process_name": process_name,
            "request_headers": dict(flow.request.headers),
            "request_content_length": len(flow.request.content) if flow.request.content else 0,
            "request_body": request_body,
            "request_body_base64": request_body_base64,
        })

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
            event = threading.Event()
            with self._lock:
                self._intercepted_flows[flow.id] = event

            self._tcp.send({
                "type": "intercepted",
                "flow_id": flow.id,
                "method": flow.request.method,
                "url": flow.request.pretty_url,
            })

            # Block until resume or drop
            event.wait(timeout=300)  # 5 min timeout

            with self._lock:
                self._intercepted_flows.pop(flow.id, None)
                if flow.id in self._dropped_flows:
                    self._dropped_flows.discard(flow.id)
                    flow.kill()
                    return

    def response(self, flow: http.HTTPFlow):
        """Called for each HTTP response."""
        logger.info("[ADDON-DEBUG] response hook called: %s %s -> %s",
                    flow.request.method, flow.request.pretty_url,
                    flow.response.status_code if flow.response else 'None')
        if not flow.response:
            return

        # Throttle simulation (simple delay based on content size)
        if self._throttle_enabled and self._throttle_dl_kbps > 0:
            size_kb = len(flow.response.content) / 1024 if flow.response.content else 0
            delay = size_kb / self._throttle_dl_kbps
            if delay > 0:
                time.sleep(min(delay, 30))  # Cap at 30s

        # Determine content type
        ct = flow.response.headers.get("content-type", "")

        # Truncate body for display (max 100KB)
        body = ""
        body_base64 = False
        if flow.response.content:
            raw = flow.response.content[:102400]
            body, body_base64 = _encode_body(raw, ct)

        self._tcp.send({
            "type": "response",
            "flow_id": flow.id,
            "status_code": flow.response.status_code,
            "response_content_type": ct,
            "response_content_length": len(flow.response.content) if flow.response.content else 0,
            "duration": (flow.response.timestamp_end - flow.request.timestamp_start)
                        if flow.response.timestamp_end and flow.request.timestamp_start else 0,
            "response_headers": dict(flow.response.headers),
            "response_body": body,
            "response_body_base64": body_base64,
        })

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
        })

    # ── command handler ──

    def _on_command(self, msg: dict):
        """Handle a command from the Qt client."""
        cmd_type = msg.get("type", "")

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

            elif cmd_type == "set_intercept":
                self._intercept_enabled = msg.get("enabled", False)
                logger.info("Intercept %s", "enabled" if self._intercept_enabled else "disabled")

            elif cmd_type == "set_throttle":
                self._throttle_enabled = msg.get("enabled", False)
                self._throttle_dl_kbps = msg.get("download_kbps", 0)
                self._throttle_ul_kbps = msg.get("upload_kbps", 0)
                logger.info("Throttle %s (DL=%dKB/s UL=%dKB/s)",
                            "on" if self._throttle_enabled else "off",
                            self._throttle_dl_kbps, self._throttle_ul_kbps)

            elif cmd_type == "resume_flow":
                flow_id = msg.get("flow_id", "")
                ev = self._intercepted_flows.get(flow_id)
                if ev:
                    ev.set()
                    logger.info("Resumed flow %s", flow_id)

            elif cmd_type == "drop_flow":
                flow_id = msg.get("flow_id", "")
                self._dropped_flows.add(flow_id)
                ev = self._intercepted_flows.get(flow_id)
                if ev:
                    ev.set()
                    logger.info("Dropped flow %s", flow_id)

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
