# SPDX-License-Identifier: GPL-2.0

import os
from pathlib import Path
from typing import List, Optional
from .base import Compiler, CompileResult


class ElixirCompiler(Compiler):
    \"\"\"Wrapper for the Elixir compiler (elixirc).\"\"\"
    
    def default_executable(self) -> str:
        return "elixirc"
    
    def name(self) -> str:
        return "Elixir"
    
    def compile(self, source: Path, output: Optional[Path] = None,
                options: Optional[List[str]] = None,
                include_paths: Optional[List[Path]] = None) -> CompileResult:
        \"\"\"
        Compile an Elixir source file (.ex) to BEAM bytecode.
        
        Args:
            source: .ex or .exs source file
            output: Output directory for .beam files
            options: Additional elixirc flags
            include_paths: Additional code paths (-pa flags)
        \"\"\"
        source = Path(source)
        if not source.exists():
            return CompileResult(
                success=False, returncode=-1, stdout="", stderr="",
                diagnostics=[{"type": "error", "message": f"Source not found: {source}"}]
            )
        
        cmd = [self.executable]
        
        if output:
            output = Path(output)
            output.mkdir(parents=True, exist_ok=True)
            cmd.extend(["-o", str(output)])
        
        if include_paths:
            for p in include_paths:
                cmd.extend(["-pa", str(p)])
        
        # Debug info for OS-level debugging
        cmd.append("--debug-info")
        
        if options:
            cmd.extend(options)
        
        cmd.append(str(source))
        
        result = self._run(cmd, cwd=str(source.parent))
        if output:
            result.output_file = output / (source.stem + ".beam")
        return result
    
    def compile_project(self, project_dir: Path, 
                       env: Optional[dict] = None) -> CompileResult:
        \"\"\"Compile a full Elixir project using mix.\"\"\"
        cmd = ["mix", "compile"]
        env_vars = {**(env or {})}
        return self._run(cmd, cwd=str(project_dir))
    
    def build_release(self, project_dir: Path,
                     release_name: str = "os_release") -> CompileResult:
        \"\"\"Build an OTP release for embedding in the OS.\"\"\"
        cmd = ["mix", "release", release_name]
        return self._run(cmd, cwd=str(project_dir))
    
    def compile_script(self, source: Path, output: Optional[Path] = None) -> CompileResult:
        \"\"\"Compile an Elixir script (.exs) to an escript executable.\"\"\"
        cmd = ["mix", "escript.build"]
        if output:
            cmd.extend(["--output", str(output)])
        return self._run(cmd, cwd=str(source.parent))