"""
Validate asset token consistency across token.json and theme JSON files,
and verify every referenced asset file actually exists on disk.

Checks performed (per theme, per category e.g. images / videos):
  1. token.json and the theme define the same set of tokens
     (compared after enum normalization, so 'icon-user' == 'icon_user').
  2. No duplicate tokens inside a category (after normalization).
  3. Every theme entry's `path` points to a file that exists under the asset root.
  4. The declared `format` matches the file's actual extension.

Token name normalization mirrors renderer.py's str_to_enum so that the check
matches what the C++ codegen actually produces.

Exit with code 1 on any problem, printing a per-theme diagnostic.
"""

import argparse
import json
import os
import re
import sys


def normalize_token(token_name):
    """Mirror renderer.py str_to_enum: '-'/whitespace -> '_', capitalize each word.

    e.g. 'icon-user' -> 'Icon_User', 'icon_user' -> 'Icon_User'.
    """
    if not token_name:
        return ""
    collapsed = re.sub(r"[\s-]+", "_", token_name.strip())
    return "_".join(part[:1].upper() + part[1:] for part in collapsed.split("_") if part)


def load_token_categories(token_file_path):
    """Read token.json -> {category: [raw_token, ...]}."""
    with open(token_file_path, encoding="utf-8") as file:
        return json.load(file)["tokens"]


def load_theme(theme_file_path):
    """Read a theme JSON file -> (theme_name, {category: [entry, ...]})."""
    with open(theme_file_path, encoding="utf-8") as file:
        data = json.load(file)
    return data["name"], data["resources"]


def main():
    argument_parser = argparse.ArgumentParser(
        description="Validate asset token consistency and on-disk asset existence"
    )
    argument_parser.add_argument("--token", required=True, help="Path to token.json")
    argument_parser.add_argument("--themes", nargs="+", required=True, help="Paths to theme JSON files")
    argument_parser.add_argument(
        "--asset-root", required=True,
        help="Base directory that asset 'path' fields are relative to (the resource/ dir)",
    )
    args = argument_parser.parse_args()

    token_categories = load_token_categories(args.token)
    validation_errors = []

    for theme_file_path in args.themes:
        theme_name, theme_categories = load_theme(theme_file_path)

        # Category-level consistency between token.json and the theme.
        token_only_categories = set(token_categories) - set(theme_categories)
        theme_only_categories = set(theme_categories) - set(token_categories)
        for category in sorted(token_only_categories):
            validation_errors.append(f"[{theme_name}] category '{category}' in token.json but missing from theme")
        for category in sorted(theme_only_categories):
            validation_errors.append(f"[{theme_name}] category '{category}' in theme but missing from token.json")

        for category, theme_entries in theme_categories.items():
            defined_tokens = {normalize_token(t) for t in token_categories.get(category, [])}

            # 2. duplicate detection (normalized)
            seen_tokens = set()
            theme_tokens = set()
            for entry in theme_entries:
                normalized = normalize_token(entry.get("token", ""))
                if normalized in seen_tokens:
                    validation_errors.append(
                        f"[{theme_name}/{category}] duplicate token: '{entry.get('token')}'"
                    )
                seen_tokens.add(normalized)
                theme_tokens.add(normalized)

            # 1. token set consistency
            missing_tokens = defined_tokens - theme_tokens
            extra_tokens = theme_tokens - defined_tokens
            if missing_tokens:
                validation_errors.append(f"[{theme_name}/{category}] missing tokens: {sorted(missing_tokens)}")
            if extra_tokens:
                validation_errors.append(f"[{theme_name}/{category}] extra tokens:   {sorted(extra_tokens)}")

            # 3 & 4. file existence + format/extension match
            for entry in theme_entries:
                token_name = entry.get("token", "<unknown>")
                relative_path = entry.get("path", "")
                if not relative_path:
                    validation_errors.append(f"[{theme_name}/{category}] token '{token_name}': empty path")
                    continue

                absolute_path = os.path.normpath(os.path.join(args.asset_root, relative_path))
                if not os.path.isfile(absolute_path):
                    validation_errors.append(
                        f"[{theme_name}/{category}] token '{token_name}': asset file not found: {relative_path}"
                    )
                    continue

                declared_format = (entry.get("format") or "").lower()
                actual_extension = os.path.splitext(relative_path)[1].lstrip(".").lower()
                if declared_format and actual_extension and declared_format != actual_extension:
                    validation_errors.append(
                        f"[{theme_name}/{category}] token '{token_name}': format '{declared_format}' "
                        f"does not match file extension '.{actual_extension}' ({relative_path})"
                    )

    if validation_errors:
        print("ERROR: Asset token inconsistency detected!", file=sys.stderr)
        for error_message in validation_errors:
            print(f"  {error_message}", file=sys.stderr)
        sys.exit(1)

    print("Asset token validation passed.")


if __name__ == "__main__":
    main()
