import os
import json
import re
from jinja2 import Environment, FileSystemLoader
import sys

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
    
    def _get_platform_string(self, string_obj: dict) -> str:
        """
        根据平台从字符串对象获取对应值

        Args:
            string_obj: 单个字符串对象 {"token": "...", "value": "...", "conditions": [...]}
        Returns:
            对应平台的字符串值
        """
        if not string_obj or not isinstance(string_obj, dict):
            return ""

        # 返回平台特定值或默认值
        result = string_obj.get('value', '')
        conditions = string_obj.get('conditions', [])
        current_platform = self._get_platform()
        if conditions and isinstance(conditions, list):
            for condition in conditions:
                if isinstance(condition, dict) and condition.get('platform') == current_platform:
                    result = condition.get('value', result)
                    break
                
        return result
    def _get_platform(self):
        """获取操作系统平台"""
        platform = sys.platform

        if platform.startswith('win'):
            return 'windows'
        elif platform.startswith('darwin'):
            return 'macos'
        elif platform.startswith('linux'):
            return 'linux'
        else:
            return 'common'
    
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
