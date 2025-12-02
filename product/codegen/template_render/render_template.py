#!/usr/bin/env python3
import argparse
from render.renderer import TemplateRenderer


def main():
    parser = argparse.ArgumentParser(description="Render a Jinja2 template using a JSON input file.")
    parser.add_argument('--template', required=True, help='Path to the Jinja2 template file')
    parser.add_argument('--input', required=True, help='Path to the input JSON file')
    parser.add_argument('--output', required=True, help='Path to the output file to generate')
    parser.add_argument('--param', action='append', help='Extra parameters in format key=value')
    args = parser.parse_args()

    try:
        renderer = TemplateRenderer(args.template)
        
        extra_params = {}
        if args.param:
            for param in args.param:
                if '=' in param:
                    key, value = param.split('=', 1)
                    extra_params[key] = value
                else:
                    print(f"[WARNING] Invalid parameter format: {param}, expected key=value")
        
        renderer.render(args.input, args.output, **extra_params)
        print(f"Generated: {args.output}")
    except Exception as e:
        print(f"[ERROR] {e}")
        exit(1)


if __name__ == '__main__':
    main()
