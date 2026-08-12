"""
Validate that token.json and all theme JSON files define the same set of tokens.
Exit with code 1 on mismatch, printing which tokens are missing or extra per theme.
"""

import argparse
import json
import sys


def load_token_names(token_file_path):
    """Read token.json and return the set of defined token names."""
    with open(token_file_path) as file:
        return set(json.load(file)["tokens"])


def load_theme_token_names(theme_file_path):
    """Read a theme JSON file and return (theme_name, set_of_token_names)."""
    with open(theme_file_path) as file:
        data = json.load(file)
    return data["name"], {resource_item["token"] for resource_item in data["resource"]}


def load_palette_color_names(palette_file_path):
    """Read palette.json and return the set of palette color names, or None."""
    if not palette_file_path:
        return None
    with open(palette_file_path) as file:
        data = json.load(file)
    # palette.json is a top-level array of {name, hex, rgb} objects
    if isinstance(data, list):
        return {color_entry["name"] for color_entry in data}
    return {color_entry["name"] for color_entry in data["colors"]}


def main():
    argument_parser = argparse.ArgumentParser(
        description="Validate color token consistency across token.json and theme files"
    )
    argument_parser.add_argument("--token", required=True, help="Path to token.json")
    argument_parser.add_argument("--themes", nargs="+", required=True, help="Paths to theme JSON files")
    argument_parser.add_argument("--palette", help="Path to palette.json (optional)")
    args = argument_parser.parse_args()

    defined_tokens = load_token_names(args.token)
    palette_colors = load_palette_color_names(args.palette)
    validation_errors = []

    for theme_file_path in args.themes:
        theme_name, theme_tokens = load_theme_token_names(theme_file_path)
        missing_tokens = defined_tokens - theme_tokens
        extra_tokens = theme_tokens - defined_tokens
        if missing_tokens:
            validation_errors.append(f"[{theme_name}] missing tokens: {sorted(missing_tokens)}")
        if extra_tokens:
            validation_errors.append(f"[{theme_name}] extra tokens:   {sorted(extra_tokens)}")

    if validation_errors:
        print("ERROR: Color token inconsistency detected!", file=sys.stderr)
        for error_message in validation_errors:
            print(f"  {error_message}", file=sys.stderr)
        sys.exit(1)

    print("Color token validation passed.")


if __name__ == "__main__":
    main()
