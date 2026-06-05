# SPDX-License-Identifier: GPL-2.0
import os
from .toolchain import ToolchainManager
from .gcc import GCCCompiler
from .elixir import ElixirCompiler
from .ada import AdaCompiler
from .c import CCompiler
from .html import HTMLCompiler
from .python import PythonCompiler

__version__ = "1.0.0"
__all__ = [
    "ToolchainManager",
    "GCCCompiler",
    "ElixirCompiler", 
    "AdaCompiler",
    "CCompiler",
    "HTMLCompiler",
    "PythonCompiler",
]
"""

with open(os.path.join(base_dir, "os_compiler_lib", "__init__.py"), "w") as f:
    f.write(init_py)

# 2. base.py - Abstract base class
base_py = """\"\"\"
Base compiler interface.
\"\"\"

import shutil
import subprocess
from abc import ABC, abstractmethod
from typing import List, Optional, Dict, Any
from dataclasses import dataclass
from pathlib import Path


@dataclass
class CompileResult:
    \"\"\"Result of a compilation operation.\"\"\"
    success: bool
    returncode: int
    stdout: str
    stderr: str
    output_file: Optional[Path] = None
    diagnostics: List[Dict[str, Any]] = None
    
    def __post_init__(self):
        if self.diagnostics is None:
            self.diagnostics = []


class Compiler(ABC):
    \"\"\"Abstract base class for all compilers.\"\"\"
    
    def __init__(self, executable: Optional[str] = None, os_env: Optional[Dict[str, str]] = None):
        self.executable = executable or self.default_executable()
        self.os_env = os_env or {}
        self._available: Optional[bool] = None
    
    @abstractmethod
    def default_executable(self) -> str:
        \"\"\"Return the default executable name.\"\"\"
        pass
    
    @abstractmethod
    def name(self) -> str:
        \"\"\"Return compiler name.\"\"\"
        pass
    
    @property
    def available(self) -> bool:
        \"\"\"Check if compiler is available in PATH.\"\"\"
        if self._available is None:
            self._available = shutil.which(self.executable) is not None
        return self._available
    
    def get_version(self) -> CompileResult:
        \"\"\"Get compiler version information.\"\"\"
        return self._run([self.executable, "--version"])
    
    def _run(self, cmd: List[str], cwd: Optional[str] = None, 
             input_data: Optional[str] = None) -> CompileResult:
        \"\"\"Execute a compiler command.\"\"\"
        env = {**os.environ, **self.os_env}
        try:
            proc = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                cwd=cwd,
                env=env,
                input=input_data,
                timeout=300
            )
            return CompileResult(
                success=proc.returncode == 0,
                returncode=proc.returncode,
                stdout=proc.stdout,
                stderr=proc.stderr,
                diagnostics=self._parse_diagnostics(proc.stderr)
            )
        except FileNotFoundError:
            return CompileResult(
                success=False,
                returncode=-1,
                stdout="",
                stderr=f"Executable not found: {cmd[0]}",
                diagnostics=[{"type": "error", "message": f"Executable not found: {cmd[0]}"}]
            )
        except subprocess.TimeoutExpired:
            return CompileResult(
                success=False,
                returncode=-1,
                stdout="",
                stderr="Compilation timed out after 300 seconds",
                diagnostics=[{"type": "error", "message": "Timeout"}]
            )
    
    def _parse_diagnostics(self, stderr: str) -> List[Dict[str, Any]]:
        \"\"\"Parse stderr for structured diagnostics. Override in subclasses.\"\"\"
        diagnostics = []
        for line in stderr.strip().split("\\n"):
            line = line.strip()
            if "error" in line.lower():
                diagnostics.append({"type": "error", "message": line})
            elif "warning" in line.lower():
                diagnostics.append({"type": "warning", "message": line})
        return diagnostics
    
    @abstractmethod
    def compile(self, source: Path, output: Optional[Path] = None, 
                options: Optional[List[str]] = None) -> CompileResult:
        \"\"\"Compile a source file.\"\"\"
        pass