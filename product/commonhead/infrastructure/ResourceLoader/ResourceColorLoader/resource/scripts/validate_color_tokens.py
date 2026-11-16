"""Validate color registrations, theme states, and optional palette references."""

import argparse
from collections import Counter
import json
import sys


COLOR_STATES = {"normal", "hovered", "pressed", "disabled", "focused", "checked", "selected"}


def load_json(file_path):
    with open(file_path, encoding="utf-8") as file:
        try:
            return json.load(file)
        except json.JSONDecodeError as error:
            raise ValueError(f"{file_path}: {error}") from error


def duplicate_names(names):
    return sorted(name for name, count in Counter(names).items() if count > 1)


def load_token_names(token_file_path, validation_errors):
    """Read registrations without silently discarding duplicate token names."""
    data = load_json(token_file_path)
    names = data.get("tokens") if isinstance(data, dict) else None
    if not isinstance(names, list) or any(not isinstance(name, str) or not name for name in names):
        raise ValueError(f"{token_file_path}: 'tokens' must be an array of non-empty strings")
    duplicates = duplicate_names(names)
    if duplicates:
        validation_errors.append(f"[{token_file_path}] duplicate registered tokens: {duplicates}")
    return set(names)


def load_palette_color_names(palette_file_path, validation_errors):
    """Read palette.json and return the set of palette color names, or None."""
    if not palette_file_path:
        return None
    data = load_json(palette_file_path)
    colors = data.get("colors") if isinstance(data, dict) else data
    if not isinstance(colors, list) or any(
        not isinstance(entry, dict) or not isinstance(entry.get("name"), str) or not entry["name"]
        for entry in colors
    ):
        raise ValueError(f"{palette_file_path}: palette must be an array, or a 'colors' array, of named colors")
    names = [entry["name"] for entry in colors]
    duplicates = duplicate_names(names)
    if duplicates:
        validation_errors.append(f"[{palette_file_path}] duplicate palette colors: {duplicates}")
    return set(names)


def validate_theme(theme_file_path, defined_tokens, palette_colors, validation_errors):
    data = load_json(theme_file_path)
    resources = data.get("resource") if isinstance(data, dict) else None
    if not isinstance(resources, list) or any(
        not isinstance(item, dict) or not isinstance(item.get("token"), str) or not item["token"]
        for item in resources
    ):
        raise ValueError(f"{theme_file_path}: 'resource' must be an array of named tokens")
    context = f"{data.get('name', 'unnamed theme')} ({theme_file_path})"
    names = [item["token"] for item in resources]
    duplicates = duplicate_names(names)
    if duplicates:
        validation_errors.append(f"[{context}] duplicate theme tokens: {duplicates}")
    theme_tokens = set(names)
    missing_tokens = defined_tokens - theme_tokens
    extra_tokens = theme_tokens - defined_tokens
    if missing_tokens:
        validation_errors.append(f"[{context}] missing tokens: {sorted(missing_tokens)}")
    if extra_tokens:
        validation_errors.append(f"[{context}] extra tokens: {sorted(extra_tokens)}")

    for item in resources:
        token_context = f"[{context}] {item['token']}"
        colors = item.get("color")
        if not isinstance(colors, dict):
            validation_errors.append(f"{token_context}: 'color' must be a state-to-palette mapping")
            continue
        if "normal" not in colors:
            validation_errors.append(f"{token_context}: missing required 'normal' state")
        unknown_states = set(colors) - COLOR_STATES
        if unknown_states:
            validation_errors.append(f"{token_context}: unknown states: {sorted(unknown_states)}")
        for state, color_name in colors.items():
            if not isinstance(color_name, str) or not color_name:
                validation_errors.append(f"{token_context}.{state}: palette color must be a non-empty string")
            elif palette_colors is not None and color_name not in palette_colors:
                validation_errors.append(f"{token_context}.{state}: unknown palette color '{color_name}'")


def main():
    argument_parser = argparse.ArgumentParser(
        description="Validate color token registrations, theme states, and optional palette references"
    )
    argument_parser.add_argument("--token", required=True, help="Path to token.json")
    argument_parser.add_argument("--themes", nargs="+", required=True, help="Paths to theme JSON files")
    argument_parser.add_argument("--palette", help="Path to palette.json (optional)")
    args = argument_parser.parse_args()

    validation_errors = []
    try:
        defined_tokens = load_token_names(args.token, validation_errors)
        palette_colors = load_palette_color_names(args.palette, validation_errors)
        for theme_file_path in args.themes:
            validate_theme(theme_file_path, defined_tokens, palette_colors, validation_errors)
    except (OSError, ValueError) as error:
        validation_errors.append(str(error))

    if validation_errors:
        print("ERROR: Color configuration validation failed!", file=sys.stderr)
        for error_message in validation_errors:
            print(f"  {error_message}", file=sys.stderr)
        sys.exit(1)

    print("Color token validation passed.")


if __name__ == "__main__":
    main()
