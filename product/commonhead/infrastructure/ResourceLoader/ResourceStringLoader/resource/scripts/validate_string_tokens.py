"""Validate the string manifest before generating C++ enums and string maps.

This checks configuration and parameter markers, not translation completeness.
Only LocalizedStringWithParam treats double braces as parameter syntax.
"""

import argparse
import json
import re
import sys


CATEGORIES = {"LocalizedString", "LocalizedStringWithParam", "NonLocalizedString"}
PLATFORMS = {"windows", "macos", "linux", "android", "ios", "common"}
PARAMETER = re.compile(r"\{\{(0|[1-9][0-9]*)\}\}")


def require(condition, message):
    if not condition:
        raise ValueError(message)


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, f"duplicate JSON key: {key!r}")
        result[key] = value
    return result


def check_fields(value, required, optional, context):
    require(isinstance(value, dict), f"{context}: must be an object")
    missing = required - set(value)
    unknown = set(value) - required - optional
    require(not missing, f"{context}: missing fields: {sorted(missing)}")
    require(not unknown, f"{context}: unknown fields: {sorted(unknown)}")


def check_text(value, context, parameterized):
    require(isinstance(value, str), f"{context}: must be a string")
    require("\0" not in value, f"{context}: NUL is unsupported by const char* consumers")
    require(not any(0xD800 <= ord(char) <= 0xDFFF for char in value),
            f"{context}: isolated Unicode surrogate cannot be encoded as UTF-8")
    if not parameterized:
        return set()
    # Keep single braces as text, including JSON braces next to a valid marker.
    remainder = PARAMETER.sub("", value)
    require("{{" not in remainder and "}}" not in remainder,
            f"{context}: invalid parameter marker; expected {{{{0}}}}, {{{{1}}}}, etc. without leading zeros")
    return set(PARAMETER.findall(value))


def check_entries(category, entries):
    require(isinstance(entries, list), f"{category}: must be a list")
    enum_sources = {}
    parameterized = category == "LocalizedStringWithParam"
    for index, entry in enumerate(entries):
        context = f"{category}[{index}]"
        check_fields(entry, {"token", "value"}, {"conditions"}, context)
        token = entry["token"]
        require(isinstance(token, str) and bool(token.strip()),
                f"{context}.token: must be a non-empty string")
        # Match renderer.py's str_to_enum filter exactly.
        normalized = re.sub(r"[\s-]+", "_", token.strip())
        enum_name = "_".join(part[:1].upper() + part[1:]
                             for part in normalized.split("_") if part)
        require(bool(re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", enum_name)),
                f"{context}.token: {token!r} generates invalid enum {enum_name!r}")
        require(enum_name not in {"None", f"Last{category}"},
                f"{context}.token: {token!r} generates reserved enum {enum_name!r}")
        require(enum_name not in enum_sources,
                f"{context}.token: duplicate token/generated enum {enum_name!r} "
                f"(previous token: {enum_sources.get(enum_name)!r})")
        enum_sources[enum_name] = token
        context = f"{category}[{token!r}]"
        markers = check_text(entry["value"], f"{context}.value", parameterized)
        conditions = entry.get("conditions", [])
        require(isinstance(conditions, list), f"{context}.conditions: must be a list")
        seen_platforms = set()
        for condition_index, condition in enumerate(conditions):
            condition_context = f"{context}.conditions[{condition_index}]"
            check_fields(condition, {"platform", "value"}, set(), condition_context)
            platform = condition["platform"]
            require(isinstance(platform, str) and platform in PLATFORMS,
                    f"{condition_context}.platform: expected one of {sorted(PLATFORMS)}")
            require(platform not in seen_platforms,
                    f"{condition_context}.platform: duplicate platform {platform!r}")
            seen_platforms.add(platform)
            condition_markers = check_text(condition["value"], f"{condition_context}.value", parameterized)
            if parameterized:
                require(condition_markers == markers,
                        f"{condition_context}.value: parameter set {sorted(condition_markers)} "
                        f"differs from default {sorted(markers)}")


def main():
    parser = argparse.ArgumentParser(description="Validate ResourceString.json before code generation")
    parser.add_argument("--input", required=True, help="Path to ResourceString.json")
    args = parser.parse_args()
    try:
        with open(args.input, encoding="utf-8") as file:
            data = json.load(file, object_pairs_hook=unique_object)
        check_fields(data, CATEGORIES, set(), "root")
        for category, entries in data.items():
            check_entries(category, entries)
    except (OSError, ValueError) as error:
        print(f"ERROR: {args.input}: {error}", file=sys.stderr)
        return 1
    print("String token validation passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
