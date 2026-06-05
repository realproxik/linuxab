# SPDX-License-Identifier: GPL-2.0
import os

from pathlib import Path
from typing import List, Optional
from .base import Compiler, CompileResult


class GCCCompiler(Compiler):
    \"\"\"Wrapper for the GNU Compiler Collection (GCC).\"\"\"
    
    def default_executable(self) -> str:
        return "gcc"
    
    def name(self) -> str:
        return "GCC"
    
    def compile(self, source: Path, output: Optional[Path] = None,
                options: Optional[List[str]] = None,
                std: str = "c11",
                optimize: int = 0,
                include_dirs: Optional[List[str]] = None,
                define_macros: Optional[List[str]] = None) -> CompileResult:
        \"\"\"
        Compile a C/C++ source file.
        
        Args:
            source: Source file path (.c, .cpp, .s, etc.)
            output: Output binary/object path
            options: Additional GCC flags
            std: C/C++ standard (c89, c99, c11, c17, c++11, c++17, c++20)
            optimize: Optimization level (0-3, or s for size)
            include_dirs: Additional include directories
            define_macros: Macros to define (-DNAME or -DNAME=VALUE)
        \"\"\"
        source = Path(source)
        if not source.exists():
            return CompileResult(
                success=False, returncode=-1, stdout="", stderr=f"",
                diagnostics=[{"type": "error", "message": f"Source not found: {source}"}]
            )
        
        cmd = [self.executable]
        
        # Standard
        if std:
            cmd.append(f"-std={std}")
        
        # Optimization
        cmd.append(f"-O{optimize}")
        
        # Includes
        if include_dirs:
            for inc in include_dirs:
                cmd.extend(["-I", str(inc)])
        
        # Defines
        if define_macros:
            for d in define_macros:
                cmd.append(f"-D{d}")
        
        # Warnings
        cmd.extend(["-Wall", "-Wextra"])
        
        # User options
        if options:
            cmd.extend(options)
        
        # Output
        if output:
            output = Path(output)
            cmd.extend(["-o", str(output)])
        else:
            # Default output name
            cmd.append("-c")  # Compile only, produce .o
        
        cmd.append(str(source))
        
        result = self._run(cmd, cwd=str(source.parent))
        result.output_file = output
        return result
    
    def compile_static_library(self, objects: List[Path], 
                               output: Path) -> CompileResult:
        \"\"\"Create a static library using ar.\"\"\"
        cmd = ["ar", "rcs", str(output)] + [str(o) for o in objects]
        return self._run(cmd)
    
    def link_executable(self, objects: List[Path], output: Path,
                       libs: Optional[List[str]] = None,
                       lib_dirs: Optional[List[str]] = None) -> CompileResult:
        \"\"\"Link object files into an executable.\"\"\"
        cmd = [self.executable, "-o", str(output)]
        cmd += [str(o) for o in objects]
        
        if lib_dirs:
            for d in lib_dirs:
                cmd.extend(["-L", str(d)])
        
        if libs:
            for lib in libs:
                prefix = "" if lib.startswith("lib") else "lib"
                cmd.extend(["-l", f"{prefix}{lib}"])
        
        return self._run(cmd)
    
    def compile_kernel_module(self, source: Path, output: Path,
                              arch: str = "x86_64") -> CompileResult:
        \"\"\"Compile a kernel module with OS-specific flags.\"\"\"
        options = [
            "-D__KERNEL__",
            "-DMODULE",
            "-nostdlib",
            "-fno-builtin",
            "-fno-stack-protector",
            "-mcmodel=kernel",
            f"-march={arch}",
            "-ffreestanding",
        ]
        return self.compile(source, output, options=options, std="c11")