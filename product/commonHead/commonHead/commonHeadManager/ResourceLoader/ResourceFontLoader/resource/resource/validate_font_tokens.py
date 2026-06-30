"""
Validate font token consistency across token.json, theme JSON files and palette.json.

Checks performed:
  1. token.json and every theme file define exactly the same set of token names.
  2. Every theme entry's font references a fontType / fontSize / fontWeight that
     actually exists in palette.json (catches typos / accidental renames).
  3. Every theme entry's `italic` field (when present) is a boolean.

Exit with code 1 on any mismatch, printing a per-theme diagnostic.
"""

import argparse
import json
import sys


def load_token_names(token_file_path):
    """Read token.json and return the set of defined token names."""
    with open(token_file_path, encoding="utf-8") as file:
        return set(json.load(file)["tokens"])


def load_theme_entries(theme_file_path):
    """Read a theme JSON file and return (theme_name, list_of_resource_entries)."""
    with open(theme_file_path, encoding="utf-8") as file:
        data = json.load(file)
    return data["name"], data["resource"]


def load_palette_values(palette_file_path):
    """Read palette.json and return (font_types, font_sizes, font_weights) as sets."""
    with open(palette_file_path, encoding="utf-8") as file:
        data = json.load(file)
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
        theme_name, theme_entries = load_theme_entries(theme_file_path)
        theme_tokens = {entry["token"] for entry in theme_entries}

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

            font_type = font.get("fontType")
            if font_type not in palette_font_types:
                validation_errors.append(
                    f"[{theme_name}] token '{token_name}': unknown fontType '{font_type}' "
                    f"(valid: {sorted(palette_font_types)})"
                )

            font_size = font.get("fontSize")
            if font_size not in palette_font_sizes:
                validation_errors.append(
                    f"[{theme_name}] token '{token_name}': unknown fontSize '{font_size}' "
                    f"(valid: {sorted(palette_font_sizes)})"
                )

            font_weight = font.get("fontWeight")
            if font_weight not in palette_font_weights:
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
    main()
