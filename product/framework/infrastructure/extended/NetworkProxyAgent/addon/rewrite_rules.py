"""Validated, ordered HTTP message rewrites for the proxy addon.

Each matching rule is committed only after both its header and body operations
succeed. The capture payload is deliberately not involved in this module.
"""

import codecs
import json
import re
from dataclasses import dataclass
from email.message import Message as MimeMessage
from typing import Any

from mitmproxy import http


MAX_REWRITE_RULES = 128
MAX_REWRITE_CONFIG_BYTES = 1024 * 1024
MAX_HEADER_OPERATIONS = 64
HEADER_TOKEN = re.compile(r"[!#$%&'*+\-.^_`|~0-9A-Za-z]+")
MANAGED_HEADERS = {"content-length", "transfer-encoding"}
BODY_OPERATIONS = {"none", "replace", "json_set", "json_remove", "text_replace"}


@dataclass(frozen=True)
class HeaderOperation:
    operation: str
    name: str
    value: str


@dataclass(frozen=True)
class BodyOperation:
    operation: str
    value: Any = None
    path: tuple[str, ...] = ()
    find: str = ""
    replacement: str = ""


@dataclass(frozen=True)
class RewriteRule:
    rule_id: str
    enabled: bool
    url_pattern: re.Pattern[str]
    method: str
    stage: str
    headers: tuple[HeaderOperation, ...]
    body: BodyOperation


def _string(value, label: str, max_length: int, *, empty: bool = True, allow_nul: bool = False) -> str:
    if not isinstance(value, str) or len(value) > max_length or (not empty and not value):
        raise ValueError(f"{label} must be a string of at most {max_length} characters")
    if (not allow_nul and "\x00" in value) or any(0xD800 <= ord(ch) <= 0xDFFF for ch in value):
        raise ValueError(f"{label} contains invalid Unicode or NUL")
    return value


def _pointer(path: str) -> tuple[str, ...]:
    if not path:
        return ()
    if not path.startswith("/"):
        raise ValueError("JSON Pointer must be empty or start with /")
    tokens = []
    for token in path[1:].split("/"):
        if re.search(r"~(?![01])", token):
            raise ValueError("JSON Pointer only accepts ~0 and ~1 escapes")
        tokens.append(token.replace("~1", "/").replace("~0", "~"))
    return tuple(tokens)


def compile_rules(raw_rules) -> tuple[RewriteRule, ...]:
    """Validate a whole replacement configuration before installing any rules."""
    if not isinstance(raw_rules, list) or len(raw_rules) > MAX_REWRITE_RULES:
        raise ValueError(f"Rewrite requires an array of at most {MAX_REWRITE_RULES} rules")
    try:
        serialized = json.dumps(raw_rules, ensure_ascii=False, allow_nan=False, separators=(",", ":")).encode("utf-8")
    except (TypeError, ValueError, UnicodeError, RecursionError) as exc:
        raise ValueError("Rewrite configuration must contain valid JSON") from exc
    if len(serialized) > MAX_REWRITE_CONFIG_BYTES:
        raise ValueError("Rewrite configuration exceeds 1 MiB")

    compiled = []
    seen_ids = set()
    for index, raw in enumerate(raw_rules):
        try:
            if not isinstance(raw, dict):
                raise ValueError("rule must be an object")
            rule_id = _string(raw.get("id"), "id", 128, empty=False)
            if rule_id in seen_ids:
                raise ValueError("duplicate rule id")
            seen_ids.add(rule_id)
            enabled = raw.get("enabled", True)
            if not isinstance(enabled, bool):
                raise ValueError("enabled must be a boolean")
            pattern = _string(raw.get("url_pattern"), "URL pattern", 4096, empty=False)
            method = _string(raw.get("method", "ANY"), "method", 64, empty=False)
            if not HEADER_TOKEN.fullmatch(method) or method != method.upper():
                raise ValueError("method must be ANY or an uppercase HTTP method")
            stage = raw.get("stage")
            if not isinstance(stage, str) or stage not in {"request", "response"}:
                raise ValueError("stage must be request or response")
            body = _compile_body(raw.get("body", {"operation": "none"}))
            headers = _compile_headers(raw.get("headers", []), body.operation != "none")
            compiled.append(RewriteRule(
                rule_id, enabled, re.compile(pattern), method, stage, headers, body
            ))
        except (ValueError, TypeError, re.error) as exc:
            raise ValueError(f"Rewrite rule {index + 1}: {exc}") from exc
    return tuple(compiled)


def _compile_headers(raw_headers, rewrites_body: bool) -> tuple[HeaderOperation, ...]:
    if not isinstance(raw_headers, list) or len(raw_headers) > MAX_HEADER_OPERATIONS:
        raise ValueError(f"headers must contain at most {MAX_HEADER_OPERATIONS} operations")
    compiled = []
    for raw in raw_headers:
        if not isinstance(raw, dict):
            raise ValueError("header operation must be an object")
        operation = raw.get("operation")
        if not isinstance(operation, str) or operation not in {"set", "add", "remove"}:
            raise ValueError("header operation must be set, add or remove")
        name = _string(raw.get("name"), "header name", 256, empty=False)
        if not HEADER_TOKEN.fullmatch(name):
            raise ValueError("header name is not a valid HTTP token")
        lower_name = name.lower()
        if lower_name in MANAGED_HEADERS:
            raise ValueError(f"{name} is managed by the proxy")
        if lower_name == "content-encoding" and not rewrites_body:
            raise ValueError("Content-Encoding requires a body operation in the same rule")
        value = _string(raw.get("value", ""), "header value", 65536)
        if len(value.encode("utf-8")) > 65536:
            raise ValueError("header value exceeds 64 KiB")
        if any(ord(ch) < 32 and ch != "\t" for ch in value):
            raise ValueError("header value must not contain control characters except TAB")
        compiled.append(HeaderOperation(operation, name, value))
    return tuple(compiled)


def _compile_body(raw) -> BodyOperation:
    if not isinstance(raw, dict):
        raise ValueError("body must be an object")
    operation = raw.get("operation", "none")
    if not isinstance(operation, str) or operation not in BODY_OPERATIONS:
        raise ValueError("unsupported body operation")
    if operation == "replace":
        return BodyOperation(operation, value=_string(raw.get("value"), "replacement body", MAX_REWRITE_CONFIG_BYTES, allow_nul=True))
    if operation in {"json_set", "json_remove"}:
        path = _pointer(_string(raw.get("path"), "JSON Pointer", 4096, allow_nul=True))
        if operation == "json_remove" and not path:
            raise ValueError("removing the JSON root is not supported")
        if operation == "json_set" and "value" not in raw:
            raise ValueError("json_set requires a JSON value")
        return BodyOperation(operation, value=raw.get("value"), path=path)
    if operation == "text_replace":
        find = _string(raw.get("find"), "find text", MAX_REWRITE_CONFIG_BYTES, empty=False, allow_nul=True)
        replacement = _string(raw.get("replacement", ""), "replacement text", MAX_REWRITE_CONFIG_BYTES, allow_nul=True)
        return BodyOperation(operation, find=find, replacement=replacement)
    return BodyOperation("none")


def _charset(headers: http.Headers) -> str:
    mime = MimeMessage()
    mime["Content-Type"] = headers.get("content-type", "")
    # An unspecified charset is treated as UTF-8. Fail rather than silently
    # reinterpret binary/non-UTF-8 bytes as editable text.
    charset = mime.get_content_charset() or "utf-8"
    codecs.lookup(charset)
    return charset


def _array_index(token: str, length: int, *, append: bool = False) -> int:
    if token == "-" and append:
        return length
    if not re.fullmatch(r"0|[1-9][0-9]*", token):
        raise ValueError("JSON Pointer array token must be an index")
    index = int(token)
    if index >= length:
        raise ValueError("JSON Pointer array index does not exist")
    return index


def _json_edit(document, body: BodyOperation):
    if not body.path:
        return body.value
    parent = document
    for token in body.path[:-1]:
        if isinstance(parent, dict):
            if token not in parent:
                raise ValueError("JSON Pointer parent does not exist")
            parent = parent[token]
        elif isinstance(parent, list):
            parent = parent[_array_index(token, len(parent))]
        else:
            raise ValueError("JSON Pointer parent is not an object or array")
    token = body.path[-1]
    if isinstance(parent, dict):
        if body.operation == "json_remove":
            if token not in parent:
                raise ValueError("JSON Pointer field does not exist")
            del parent[token]
        else:
            parent[token] = body.value
    elif isinstance(parent, list):
        index = _array_index(token, len(parent), append=body.operation == "json_set")
        if body.operation == "json_remove":
            del parent[index]
        elif token == "-":
            parent.append(body.value)
        else:
            parent[index] = body.value
    else:
        raise ValueError("JSON Pointer parent is not an object or array")
    return document


def _body_text(message: http.Message, body: BodyOperation) -> str:
    if body.operation == "replace":
        if message.raw_content is None:
            raise ValueError("message body is unavailable (streamed or missing)")
        return body.value
    content = message.get_content(strict=True)
    if content is None:
        raise ValueError("message body is unavailable (streamed or missing)")
    if body.operation == "text_replace":
        content_type = message.headers.get("content-type", "").lower()
        if content_type and not any(value in content_type for value in (
            "text/", "json", "xml", "javascript", "form-urlencoded", "graphql", "yaml"
        )):
            raise ValueError("text replacement requires a textual Content-Type")
    text = content.decode(_charset(message.headers), errors="strict")
    if body.operation == "text_replace":
        return text.replace(body.find, body.replacement)
    # Python accepts NaN/Infinity by default, but the edited payload must be JSON.
    document = json.loads(text, parse_constant=_invalid_json_constant)
    edited = _json_edit(document, body)
    return json.dumps(edited, ensure_ascii=False, allow_nan=False, separators=(",", ":"))


def _invalid_json_constant(value: str):
    raise ValueError(f"invalid JSON constant {value}")


def _apply_rule(message: http.Message, rule: RewriteRule, *, no_body: bool):
    rewrites_body = rule.body.operation != "none"
    if rewrites_body and no_body:
        raise ValueError("this response does not permit a body")
    # Decode against the original headers, then encode against any headers the
    # rule changes. A header-only rule never reads or re-encodes the body.
    text = _body_text(message, rule.body) if rewrites_body else None
    candidate = message.copy()
    for operation in rule.headers:
        if operation.operation == "set":
            candidate.headers[operation.name] = operation.value
        elif operation.operation == "add":
            candidate.headers.insert(len(candidate.headers.fields), operation.name, operation.value)
        elif operation.name in candidate.headers:
            del candidate.headers[operation.name]
    if rewrites_body:
        target_encoding = candidate.headers.get("content-encoding")
        if "transfer-encoding" in candidate.headers:
            candidate.headers.pop("content-length", None)
        candidate.content = text.encode(_charset(candidate.headers), errors="strict")
        # Message.set_content removes an invalid Content-Encoding as a fallback.
        # Treat that as failure so the selected rule cannot silently change it.
        if target_encoding and candidate.headers.get("content-encoding") != target_encoding:
            raise ValueError("unsupported target Content-Encoding")
    message.headers = candidate.headers
    message.raw_content = candidate.raw_content


def apply_rules(flow: http.HTTPFlow, stage: str, rules: tuple[RewriteRule, ...]) -> tuple[list[str], list[dict]]:
    """Apply matching rules in order; failure rolls back only the failing rule."""
    message = flow.request if stage == "request" else flow.response
    if message is None:
        return [], []
    applied = []
    errors = []
    url = flow.request.pretty_url
    method = flow.request.method
    no_body = stage == "response" and (
        method == "HEAD" or 100 <= flow.response.status_code < 200
        or flow.response.status_code in {204, 205, 304}
        or (method == "CONNECT" and 200 <= flow.response.status_code < 300)
    )
    for rule in rules:
        if not rule.enabled or rule.stage != stage or (rule.method != "ANY" and rule.method != method):
            continue
        if not rule.url_pattern.search(url):
            continue
        try:
            _apply_rule(message, rule, no_body=no_body)
            applied.append(rule.rule_id)
        except (ValueError, TypeError, LookupError, UnicodeError, RecursionError) as exc:
            errors.append({"rule_id": rule.rule_id, "error": str(exc)[:512]})
    return applied, errors
