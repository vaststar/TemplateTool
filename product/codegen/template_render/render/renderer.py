import os
import json
from jinja2 import Environment, FileSystemLoader


class TemplateRenderer:
    def __init__(self, template_path: str):
        self.template_path = template_path
        self.template_dir = os.path.dirname(template_path)
        self.template_name = os.path.basename(template_path)

        self.env = Environment(
            loader=FileSystemLoader(self.template_dir),
            trim_blocks=True,
            lstrip_blocks=True
        )

        self.env.filters['clean_capitalize'] = self._clean_capitalize

    def _clean_capitalize(self, s):
        """去除空格并首字母大写，其他字符保持原样"""
        if not s:
            return s
        s = str(s)
        no_spaces = ''.join(s.split())
        return no_spaces[0].upper() + no_spaces[1:] if no_spaces else ''
    
    def load_template(self):
        try:
            return self.env.get_template(self.template_name)
        except Exception as e:
            raise RuntimeError(f"Failed to load Jinja template: {self.template_path}") from e

    def load_json(self, input_path: str):
        try:
            with open(input_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except json.JSONDecodeError as e:
            raise ValueError(f"Invalid JSON in file: {input_path}\n{e}") from e
        except FileNotFoundError:
            raise FileNotFoundError(f"JSON input file not found: {input_path}")

    def render(self, input_path: str, output_path: str):
        context = self.load_json(input_path)
        template = self.load_template()
        rendered = template.render(data=context)

        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(rendered)
