"""Validate a complete proxy configuration before any live state is changed."""

import json
import re
import uuid
from dataclasses import dataclass

from rewrite_rules import HEADER_TOKEN, RewriteRule, compile_rules


MAX_CONFIG_BYTES = 4 * 1024 * 1024
MAX_REVISION = (1 << 64) - 1
CONFIG_FIELDS = {
    "mock_rules", "breakpoint_rules", "blacklist_rules", "map_local_rules",
    "map_remote_rules", "rewrite_rules", "bypass_hosts", "throttle",
    "intercept_enabled",
}


@dataclass(frozen=True)
class ProxyConfig:
    mock_rules: tuple[dict, ...]
    breakpoint_rules: tuple[dict, ...]
    blacklist_rules: tuple[dict, ...]
    map_local_rules: tuple[dict, ...]
    map_remote_rules: tuple[dict, ...]
    rewrite_rules: tuple[RewriteRule, ...]
    bypass_hosts: tuple[str, ...]
    throttle_enabled: bool
    download_kbps: int
    upload_kbps: int
    intercept_enabled: bool


def config_identity(message: dict) -> tuple[str, int]:
    session_id = message.get("session_id")
    revision = message.get("revision")
    if not isinstance(session_id, str) or len(session_id) != 36:
        raise ValueError("session_id must be a UUID")
    try:
        uuid.UUID(session_id)
    except ValueError as exc:
        raise ValueError("session_id must be a UUID") from exc
    if (not isinstance(revision, str) or len(revision) > 20
            or not re.fullmatch(r"0|[1-9][0-9]*", revision)):
        raise ValueError("revision must be a decimal uint64 string")
    number = int(revision)
    if number > MAX_REVISION:
        raise ValueError("revision exceeds uint64")
    return session_id, number


def _text(value, label: str, *, empty: bool = True, allow_nul: bool = False) -> str:
    if not isinstance(value, str) or (not empty and not value):
        raise ValueError(f"{label} must be {'a' if empty else 'a nonempty'} string")
    if not allow_nul and "\x00" in value:
        raise ValueError(f"{label} must not contain NUL")
    return value


def _boolean(value, label: str) -> bool:
    if not isinstance(value, bool):
        raise ValueError(f"{label} must be a boolean")
    return value


def _pattern(value, label: str) -> re.Pattern[str]:
    return re.compile(_text(value, label, empty=False))


def _header(name: str, value: str):
    if not HEADER_TOKEN.fullmatch(name):
        raise ValueError(f"invalid HTTP header name: {name}")
    if any(ord(ch) < 32 and ch != "\t" for ch in value):
        raise ValueError(f"HTTP header {name} contains control characters")


def _rule_array(config: dict, key: str, compile_rule) -> tuple[dict, ...]:
    raw_rules = config[key]
    if not isinstance(raw_rules, list):
        raise ValueError(f"{key} must be an array")
    rules = []
    for index, raw in enumerate(raw_rules):
        try:
            if not isinstance(raw, dict):
                raise ValueError("rule must be an object")
            rules.append(compile_rule(raw))
        except (ValueError, TypeError, re.error, OverflowError, IndexError, RecursionError) as exc:
            raise ValueError(f"{key} rule {index + 1}: {exc}") from exc
    return tuple(rules)


def _url_rule(raw: dict) -> dict:
    return {"url_pattern": _pattern(raw.get("url_pattern"), "URL pattern")}


def _mock_rule(raw: dict) -> dict:
    rule = _url_rule(raw)
    status = raw.get("status_code", 200)
    if isinstance(status, bool) or not isinstance(status, int) or not 100 <= status <= 599:
        raise ValueError("status_code must be an integer from 100 to 599")
    content_type = _text(raw.get("content_type", "application/json"), "content_type")
    _header("Content-Type", content_type)
    body = _text(raw.get("body", ""), "body", allow_nul=True)
    headers = _text(raw.get("headers", ""), "headers")
    # Match the legacy Mock header parsing so comma/newline separated input
    # keeps its existing meaning. Lines without ':' remain ignored.
    for line in re.split(r"[\n,]", headers):
        line = line.strip()
        if ":" in line:
            name, value = line.split(":", 1)
            _header(name.strip(), value.strip())
    rule.update(status_code=status, content_type=content_type, body=body, headers=headers)
    return rule


def _breakpoint_rule(raw: dict) -> dict:
    rule = _url_rule(raw)
    method = _text(raw.get("method", "ANY"), "method", empty=False)
    if not HEADER_TOKEN.fullmatch(method) or method != method.upper():
        raise ValueError("method must be ANY or an uppercase HTTP method")
    rule["method"] = method
    return rule


def _map_local_rule(raw: dict) -> dict:
    rule = _url_rule(raw)
    rule["local_path"] = _text(raw.get("local_path"), "local_path", empty=False)
    return rule


def _map_remote_rule(raw: dict) -> dict:
    pattern = _pattern(raw.get("src_pattern"), "source URL pattern")
    destination = _text(raw.get("dest_url"), "dest_url", empty=False)
    # re.sub compiles the replacement template even when no match exists.
    # Reject unknown group references and invalid escapes before a request hook.
    pattern.sub(destination, "")
    return {"src_pattern": pattern, "dest_url": destination}


def compile_config(raw) -> ProxyConfig:
    if not isinstance(raw, dict):
        raise ValueError("config must be an object")
    missing = CONFIG_FIELDS.difference(raw)
    if missing:
        raise ValueError(f"config is missing: {', '.join(sorted(missing))}")
    try:
        encoded = json.dumps(raw, ensure_ascii=False, allow_nan=False,
                             separators=(",", ":")).encode("utf-8")
    except (TypeError, ValueError, UnicodeError, RecursionError) as exc:
        raise ValueError("config must contain valid JSON") from exc
    if len(encoded) > MAX_CONFIG_BYTES:
        raise ValueError("proxy configuration exceeds 4 MiB")

    mock = _rule_array(raw, "mock_rules", _mock_rule)
    breakpoint = _rule_array(raw, "breakpoint_rules", _breakpoint_rule)
    blacklist = _rule_array(raw, "blacklist_rules", _url_rule)
    map_local = _rule_array(raw, "map_local_rules", _map_local_rule)
    map_remote = _rule_array(raw, "map_remote_rules", _map_remote_rule)
    rewrite = compile_rules(raw["rewrite_rules"])

    bypass = raw["bypass_hosts"]
    if not isinstance(bypass, list):
        raise ValueError("bypass_hosts must be an array")
    hosts = []
    for index, host in enumerate(bypass):
        try:
            host = _text(host, "host pattern", empty=False)
            re.compile(host)
            hosts.append(host)
        except (ValueError, TypeError, re.error, OverflowError, RecursionError) as exc:
            raise ValueError(f"bypass_hosts pattern {index + 1}: {exc}") from exc

    throttle = raw["throttle"]
    if not isinstance(throttle, dict):
        raise ValueError("throttle must be an object")
    enabled = _boolean(throttle.get("enabled"), "throttle.enabled")
    rates = []
    for key in ("download_kbps", "upload_kbps"):
        rate = throttle.get(key)
        if isinstance(rate, bool) or not isinstance(rate, int) or rate < 0:
            raise ValueError(f"throttle.{key} must be a nonnegative integer")
        rates.append(rate)
    intercept = _boolean(raw["intercept_enabled"], "intercept_enabled")
    return ProxyConfig(mock, breakpoint, blacklist, map_local, map_remote,
                       rewrite, tuple(hosts), enabled, rates[0], rates[1], intercept)
