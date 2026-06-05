# SPDX-License-Identifier: GPL-2.0

import os

from pathlib import Path
from typing import List, Optional
from .base import Compiler, CompileResult


class CCompiler(Compiler):
    \"\"\"Dedicated C compiler wrapper. Defaults to gcc but supports clang/tcc.\"\"\"
    
    def __init__(self, backend: str = "gcc", executable: Optional[str] = None, 
                 os_env: Optional[dict] = None):
        self._backend = backend
        super().__init__(executable=executable or backend, os_env=os_env)
    
    def default_executable(self) -> str:
        return self._backend
    
    def name(self) -> str:
        return f"C ({self._backend})"
    
    @property
    def backend(self) -> str:
        return self._backend
    
    def compile(self, source: Path, output: Optional[Path] = None,
                options: Optional[List[str]] = None,
                std: str = "c11",
                optimize: int = 0,
                freestanding: bool = False,
                arch: Optional[str] = None) -> CompileResult:
        \"\"\"
        Compile a C source file.
        
        Args:
            source: .c file
            output: Output file (.o or executable)
            options: Additional compiler flags
            std: C standard
            optimize: Optimization level
            freestanding: OS kernel / freestanding environment flags
            arch: Target architecture
        \"\"\"
        source = Path(source)
        if not source.exists():
            return CompileResult(
                success=False, returncode=-1, stdout="", stderr="",
                diagnostics=[{"type": "error", "message": f"Source not found: {source}"}]
            )
        
        cmd = [self.executable]
        
        if std:
            cmd.append(f"-std={std}")
        
        cmd.append(f"-O{optimize}")
        cmd.extend(["-Wall", "-Wextra", "-pedantic"])
        
        if freestanding:
            cmd.extend(["-ffreestanding", "-nostdlib", "-nostartfiles"])
        
        if arch:
            cmd.extend(["-march=" + arch, "-mtune=" + arch])
        
        if options:
            cmd.extend(options)
        
        if output:
            cmd.extend(["-o", str(output)])
        else:
            cmd.append("-c")
        
        cmd.append(str(source))
        
        result = self._run(cmd, cwd=str(source.parent))
        result.output_file = output
        return result
    
    def preprocess_only(self, source: Path, output: Path) -> CompileResult:
        \"\"\"Run preprocessor only (-E).\"\"\"
        cmd = [self.executable, "-E", str(source), "-o", str(output)]
        return self._run(cmd)
    
    def generate_assembly(self, source: Path, output: Path,
                         intel_syntax: bool = False) -> CompileResult:
        \"\"\"Generate assembly output (-S).\"\"\"
        cmd = [self.executable, "-S", str(source), "-o", str(output)]
        if intel_syntax and self._backend in ("gcc", "clang"):
            cmd.append("-masm=intel")
        return self._run(cmd)
    
    def compile_object(self, source: Path, output: Path) -> CompileResult:
        \"\"\"Compile to object file only.\"\"\"
        return self.compile(source, output, options=["-c"])
