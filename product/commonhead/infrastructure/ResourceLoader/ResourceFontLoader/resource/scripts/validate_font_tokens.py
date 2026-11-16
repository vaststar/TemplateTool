"""
Validate font token consistency across token.json, theme JSON files and palette.json.

Checks performed:
  1. Unique token names / generated enums and identical token sets in every theme.
  2. Palette sizes are positive integer points, weights are integers in 1..1000,
     and font families are non-empty string lists with an available Default stack.
  3. Theme font references exist in palette.json; only known font fields are used,
     and `italic` (when present) is a boolean.

Exit with code 1 on any mismatch, printing a diagnostic.
"""

import argparse
import json
import re
import sys


def validate_token_names(names, context):
    """Check names before converting to a set; match codegen's str_to_enum filter."""
    if not isinstance(names, list):
        raise ValueError(f"{context}: tokens must be a list")
    enum_sources = {}
    for name in names:
        if not isinstance(name, str) or not name.strip():
            raise ValueError(f"{context}: token names must be non-empty strings")
        normalized = re.sub(r"[\s-]+", "_", name.strip())
        enum_name = "_".join(part[:1].upper() + part[1:] for part in normalized.split("_") if part)
        if not re.fullmatch(r"[A-Za-z_]\w*", enum_name, flags=re.ASCII) or enum_name in {"None", "LastFont"}:
            raise ValueError(f"{context}: token {name!r} generates invalid/reserved enum {enum_name!r}")
        if enum_name in enum_sources:
            raise ValueError(f"{context}: duplicate token/generated enum: {enum_sources[enum_name]!r}, {name!r} -> {enum_name}")
        enum_sources[enum_name] = name
    return set(names)


def load_token_names(token_file_path):
    """Read token.json and return the set of defined token names."""
    with open(token_file_path, encoding="utf-8") as file:
        return validate_token_names(json.load(file)["tokens"], token_file_path)


def load_theme_entries(theme_file_path):
    """Read a theme JSON file and return its name, entries and unique token names."""
    with open(theme_file_path, encoding="utf-8") as file:
        data = json.load(file)
    if not isinstance(data["name"], str) or not data["name"].strip():
        raise ValueError(f"{theme_file_path}: name must be a non-empty string")
    entries = data["resource"]
    if not isinstance(entries, list) or any(not isinstance(entry, dict) for entry in entries):
        raise ValueError(f"{theme_file_path}: resource must be a list of objects")
    tokens = validate_token_names([entry["token"] for entry in entries], theme_file_path)
    return data["name"], entries, tokens


def load_palette_values(palette_file_path):
    """Validate palette values and return the referenceable names in each group."""
    with open(palette_file_path, encoding="utf-8") as file:
        data = json.load(file)
    for group in ("FontType", "FontSize", "FontWeight"):
        if not isinstance(data[group], dict) or not data[group]:
            raise ValueError(f"{palette_file_path}: {group} must be a non-empty object")
    for name, size in data["FontSize"].items():
        if type(size) is not int or size <= 0:
            raise ValueError(f"{palette_file_path}: FontSize.{name} must be a positive integer (pt)")
    for name, weight in data["FontWeight"].items():
        if type(weight) is not int or not 1 <= weight <= 1000:
            raise ValueError(f"{palette_file_path}: FontWeight.{name} must be an integer in 1..1000")
    platforms = {"Windows", "MacOSX", "Linux", "iOS", "Android", "Default"}
    for name, families in data["FontType"].items():
        if not isinstance(families, dict) or "Default" not in families:
            raise ValueError(f"{palette_file_path}: FontType.{name} must define a Default family stack")
        if set(families) - platforms:
            raise ValueError(f"{palette_file_path}: FontType.{name} has unknown platforms: {sorted(set(families) - platforms)}")
        for platform, stack in families.items():
            if not isinstance(stack, list) or not stack or any(not isinstance(family, str) or not family.strip() for family in stack):
                raise ValueError(f"{palette_file_path}: FontType.{name}.{platform} must be a non-empty list of non-empty strings")
    return (
        set(data["FontType"].keys()),
        set(data["FontSize"].keys()),
        set(data["FontWeight"].keys()),
    )


def main():
    argument_parser = argparse.ArgumentParser(
        description="Validate font token consistency across token.json, theme files and palette.json"
    )
    argument_parser.add_argument("--token", required=True, help="Path to token.json")
    argument_parser.add_argument("--themes", nargs="+", required=True, help="Paths to theme JSON files")
    argument_parser.add_argument("--palette", required=True, help="Path to palette.json")
    args = argument_parser.parse_args()

    defined_tokens = load_token_names(args.token)
    palette_font_types, palette_font_sizes, palette_font_weights = load_palette_values(args.palette)
    validation_errors = []

    for theme_file_path in args.themes:
        theme_name, theme_entries, theme_tokens = load_theme_entries(theme_file_path)

        # 1. token set consistency
        missing_tokens = defined_tokens - theme_tokens
        extra_tokens = theme_tokens - defined_tokens
        if missing_tokens:
            validation_errors.append(f"[{theme_name}] missing tokens: {sorted(missing_tokens)}")
        if extra_tokens:
            validation_errors.append(f"[{theme_name}] extra tokens:   {sorted(extra_tokens)}")

        # 2 & 3. each entry's font references valid palette values
        for entry in theme_entries:
            token_name = entry.get("token", "<unknown>")
            font = entry.get("font", {})
            if not isinstance(font, dict):
                validation_errors.append(f"[{theme_name}] token '{token_name}': font must be an object")
                continue
            unknown_fields = set(font) - {"fontType", "fontSize", "fontWeight", "italic"}
            if unknown_fields:
                validation_errors.append(f"[{theme_name}] token '{token_name}': unknown font fields: {sorted(unknown_fields)}")

            font_type = font.get("fontType")
            if not isinstance(font_type, str) or font_type not in palette_font_types:
                validation_errors.append(
                    f"[{theme_name}] token '{token_name}': unknown fontType '{font_type}' "
                    f"(valid: {sorted(palette_font_types)})"
                )

            font_size = font.get("fontSize")
            if not isinstance(font_size, str) or font_size not in palette_font_sizes:
                validation_errors.append(
                    f"[{theme_name}] token '{token_name}': unknown fontSize '{font_size}' "
                    f"(valid: {sorted(palette_font_sizes)})"
                )

            font_weight = font.get("fontWeight")
            if not isinstance(font_weight, str) or font_weight not in palette_font_weights:
                validation_errors.append(
                    f"[{theme_name}] token '{token_name}': unknown fontWeight '{font_weight}' "
                    f"(valid: {sorted(palette_font_weights)})"
                )

            if "italic" in font and not isinstance(font["italic"], bool):
                validation_errors.append(
                    f"[{theme_name}] token '{token_name}': italic must be a boolean, got {font['italic']!r}"
                )

    if validation_errors:
        print("ERROR: Font token inconsistency detected!", file=sys.stderr)
        for error_message in validation_errors:
            print(f"  {error_message}", file=sys.stderr)
        sys.exit(1)

    print("Font token validation passed.")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"ERROR: Invalid font configuration: {error}", file=sys.stderr)
        sys.exit(1)
