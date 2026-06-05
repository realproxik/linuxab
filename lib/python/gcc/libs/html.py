# SPDX-License-Identifier: GPL-2.0
import os
import re
import html as html_module
from pathlib import Path
from typing import List, Optional
from .base import Compiler, CompileResult


class HTMLCompiler(Compiler):
    """HTML processor for minification and static asset bundling."""
    
    def default_executable(self) -> str:
        return "python"
    
    def name(self) -> str:
        return "HTML Processor"
    
    def compile(self, source: Path, output: Optional[Path] = None,
                options: Optional[List[str]] = None,
                minify: bool = True,
                inline_css: bool = False,
                inline_js: bool = False) -> CompileResult:
        """
        Process HTML file: minify, inline assets, bundle.
        """
        source = Path(source)
        if not source.exists():
            return CompileResult(
                success=False, returncode=-1, stdout="", stderr="",
                diagnostics=[{"type": "error", "message": f"Source not found: {source}"}]
            )
        
        try:
            content = source.read_text(encoding="utf-8")
            base_dir = source.parent
            
            if inline_css:
                content = self._inline_stylesheets(content, base_dir)
            
            if inline_js:
                content = self._inline_scripts(content, base_dir)
            
            if minify:
                content = self._minify_html(content)
            
            if output:
                output = Path(output)
                output.parent.mkdir(parents=True, exist_ok=True)
                output.write_text(content, encoding="utf-8")
            
            return CompileResult(
                success=True,
                returncode=0,
                stdout=f"Processed {source} -> {output or \'memory\'}",
                stderr="",
                output_file=output
            )
            
        except Exception as e:
            return CompileResult(
                success=False,
                returncode=-1,
                stdout="",
                stderr=str(e),
                diagnostics=[{"type": "error", "message": str(e)}]
            )
    
    def _minify_html(self, content: str) -> str:
        """Basic HTML minification."""
        content = re.sub(r\'<!--(?!\\s*\\[if)[\\s\\S]*?-->\', \'\', content)
        content = re.sub(r\'>\\s+<\', \'><\', content)
        content = re.sub(r\'\\s{2,}\', \' \', content)
        content = \'\\n\'.join(line.strip() for line in content.splitlines())
        return content.strip()
    
    def _inline_stylesheets(self, content: str, base_dir: Path) -> str:
        """Replace <link rel=stylesheet> with inline <style>."""
        def replace_link(match):
            href = match.group(1)
            css_path = base_dir / href
            if css_path.exists():
                css = css_path.read_text(encoding="utf-8")
                css = self._minify_css(css)
                return f"<style>{css}</style>"
            return match.group(0)
        
        return re.sub(
            r\'<link[^>]+rel=["\']stylesheet["\'][^>]+href=["\']([^"\']+)["\'][^>]*>\',
            replace_link,
            content,
            flags=re.IGNORECASE
        )
    
    def _inline_scripts(self, content: str, base_dir: Path) -> str:
        """Replace <script src=...> with inline <script>."""
        def replace_script(match):
            src = match.group(1)
            js_path = base_dir / src
            if js_path.exists():
                js = js_path.read_text(encoding="utf-8")
                return f"<script>{js}</script>"
            return match.group(0)
        
        return re.sub(
            r\'<script[^>]+src=["\']([^"\']+)["\'][^>]*>\',
            replace_script,
            content,
            flags=re.IGNORECASE
        )
    
    def _minify_css(self, css: str) -> str:
        """Basic CSS minification."""
        css = re.sub(r\'/\\*.*?\\*/\', \'\', css, flags=re.DOTALL)
        css = re.sub(r\'\\s*\\{\\s*\', \'{\', css)
        css = re.sub(r\'\\s*\\}\\s*\', \'}\', css)
        css = re.sub(r\'\\s*;\\s*\', \';\', css)
        css = re.sub(r\'\\s*,\\s*\', \',\', css)
        css = re.sub(r\'\\s+\', \' \', css)
        return css.strip()
    
    def bundle(self, files: List[Path], output: Path,
               wrap_in_html: bool = True,
               title: str = "OS Bundle") -> CompileResult:
        """Bundle multiple HTML fragments into a single document."""
        try:
            parts = []
            for f in files:
                parts.append(Path(f).read_text(encoding="utf-8"))
            
            if wrap_in_html:
                content = "<!DOCTYPE html>\\n<html><head><meta charset=\\"UTF-8\\"><title>" + html_module.escape(title) + "</title></head><body>" + "".join(parts) + "</body></html>"
            else:
                content = "\\n\\n".join(parts)
            
            output = Path(output)
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_text(content, encoding="utf-8")
            
            return CompileResult(
                success=True,
                returncode=0,
                stdout=f"Bundled {len(files)} files -> {output}",
                stderr="",
                output_file=output
            )
        except Exception as e:
            return CompileResult(
                success=False, returncode=-1, stdout="", stderr=str(e),
                diagnostics=[{"type": "error", "message": str(e)}]
            )