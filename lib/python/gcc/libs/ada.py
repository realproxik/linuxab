# SPDX-License-Identifier: GPL-2.0
import os

from pathlib import Path
from typing import List, Optional
from .base import Compiler, CompileResult


class AdaCompiler(Compiler):
    \"\"\"Wrapper for GNAT (GNU Ada Translator).\"\"\"
    
    def default_executable(self) -> str:
        return "gnatmake"
    
    def name(self) -> str:
        return "GNAT Ada"
    
    def compile(self, source: Path, output: Optional[Path] = None,
                options: Optional[List[str]] = None,
                mode: str = "make",
                project_file: Optional[Path] = None) -> CompileResult:
        \"\"\"
        Compile Ada source using gnatmake or gcc.
        
        Args:
            source: Main .adb or .ads file
            output: Output executable name
            options: Additional flags
            mode: 'make' (gnatmake) or 'gcc' (direct gcc invocation)
            project_file: Optional .gpr project file
        \"\"\"
        source = Path(source)
        if not source.exists():
            return CompileResult(
                success=False, returncode=-1, stdout="", stderr="",
                diagnostics=[{"type": "error", "message": f"Source not found: {source}"}]
            )
        
        if mode == "gcc":
            cmd = ["gcc", "-c", "-gnat2012"]
            if output:
                cmd.extend(["-o", str(output)])
            if options:
                cmd.extend(options)
            cmd.append(str(source))
        else:
            # gnatmake mode
            cmd = [self.executable]
            if output:
                cmd.extend(["-o", str(output)])
            if options:
                cmd.extend(options)
            if project_file:
                cmd.extend(["-P", str(project_file)])
            cmd.append(str(source))
        
        result = self._run(cmd, cwd=str(source.parent))
        result.output_file = output
        return result
    
    def bind(self, ali_file: Path, options: Optional[List[str]] = None) -> CompileResult:
        \"\"\"Run gnatbind to bind compiled units.\"\"\"
        cmd = ["gnatbind", str(ali_file)]
        if options:
            cmd.extend(options)
        return self._run(cmd)
    
    def link(self, ali_file: Path, output: Path,
             options: Optional[List[str]] = None) -> CompileResult:
        \"\"\"Run gnatlink to create executable.\"\"\"
        cmd = ["gnatlink", str(ali_file), "-o", str(output)]
        if options:
            cmd.extend(options)
        return self._run(cmd)
    
    def compile_package(self, spec: Path, body: Optional[Path] = None,
                        output_dir: Optional[Path] = None) -> CompileResult:
        \"\"\"Compile an Ada package (spec + optional body).\"\"\"
        cmd = ["gnatmake", "-c", str(spec)]
        if body:
            cmd.append(str(body))
        if output_dir:
            cmd.extend(["-D", str(output_dir)])
        return self._run(cmd)