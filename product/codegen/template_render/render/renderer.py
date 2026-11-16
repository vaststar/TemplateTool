import os
import json
import re
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
        self.env.filters['str_to_enum'] = self._str_to_enum
        self.env.filters['get_platform_string'] = self._get_platform_string
        self.env.filters['cpp_string_literal'] = self._cpp_string_literal

    def _cpp_string_literal(self, value: str) -> str:
        """Quote UTF-8 resource text as a C++ string literal."""
        if not isinstance(value, str):
            raise TypeError("cpp_string_literal expects a string")
        if '\0' in value:
            raise ValueError("Resource strings cannot contain NUL: consumers use const char*")
        escapes = {'"': '\\"', '\\': '\\\\', '\n': '\\n', '\r': '\\r', '\t': '\\t'}
        escaped = []
        for char in value:
            if char in escapes:
                escaped.append(escapes[char])
            elif ord(char) < 32 or ord(char) == 127:
                # Three octal digits keep following digits/letters outside the escape.
                escaped.append(f'\\{ord(char):03o}')
            else:
                escaped.append(char)
        return '"' + ''.join(escaped) + '"'

    def _clean_capitalize(self, s):
        """去除空格并首字母大写，其他字符保持原样"""
        if not s:
            return s
        s = str(s)
        no_spaces = ''.join(s.split())
        return no_spaces[0].upper() + no_spaces[1:] if no_spaces else ''
    
    def _str_to_enum(self, s):
        """将字符串转换为枚举格式,即去除首尾空格,每个单词首字母大写,短横和中间空格替换为下划线, eg."example-token name" -> "Example_Token_Name" """
        if not s:
            return ""
        s = re.sub(r'[\s-]+', '_', s.strip())
        if not s:
            return ""
        return '_'.join(p[:1].upper() + p[1:] for p in s.split('_') if p)
    
    def _get_platform_string(self, string_obj: dict, platform: str) -> str:
        """
        根据明确指定的平台取值，不检测生成脚本所在系统

        Args:
            string_obj: 单个字符串对象 {"token": "...", "value": "...", "conditions": [...]}
            platform: 模板正在生成的分支，例如 android、ios、common
        Returns:
            对应平台的字符串值
        """
        if not string_obj or not isinstance(string_obj, dict):
            return ""

        # 返回平台特定值或默认值
        result = string_obj.get('value', '')
        conditions = string_obj.get('conditions', [])
        if conditions and isinstance(conditions, list):
            for condition in conditions:
                if isinstance(condition, dict) and condition.get('platform') == platform:
                    result = condition.get('value', result)
                    break
                
        return result

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

    def render(self, input_path: str, output_path: str, **extra_params):
        context = self.load_json(input_path)
        template = self.load_template()
        render_context = {'data': context}
        if extra_params:
            render_context['extra'] = extra_params
        else:
            render_context['extra'] = {}

        rendered = template.render(render_context)

        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(rendered)
