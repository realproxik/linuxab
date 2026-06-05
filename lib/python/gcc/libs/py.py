# SPDX-License-Identifier: GPL-2.0
import os
import py_compile
import compileall
from pathlib import Path
from typing import List, Optional
from .base import Compiler, CompileResult


class PythonCompiler(Compiler):
    """Wrapper for Python compilation tools."""
    
    def __init__(self, backend: str = "cpython", executable: Optional[str] = None,
                 os_env: Optional[dict] = None):
        self._backend = backend
        super().__init__(executable=executable or "python3", os_env=os_env)
    
    def default_executable(self) -> str:
        return "python3"
    
    def name(self) -> str:
        return f"Python ({self._backend})"
    
    @property
    def backend(self) -> str:
        return self._backend
    
    def compile(self, source: Path, output: Optional[Path] = None,
                options: Optional[List[str]] = None,
                optimize: int = 0) -> CompileResult:
        """
        Compile Python source to bytecode or native code.
        
        Args:
            source: .py file or directory
            output: Output path
            options: Additional flags
            optimize: Optimization level (0, 1, 2)
        """
        source = Path(source)
        if not source.exists():
            return CompileResult(
                success=False, returncode=-1, stdout="", stderr="",
                diagnostics=[{"type": "error", "message": f"Source not found: {source}"}]
            )
        
        if self._backend == "cpython":
            return self._compile_cpython(source, output, optimize)
        elif self._backend == "cython":
            return self._compile_cython(source, output, options)
        elif self._backend == "nuitka":
            return self._compile_nuitka(source, output, options)
        else:
            return CompileResult(
                success=False, returncode=-1, stdout="", stderr="",
                diagnostics=[{"type": "error", "message": f"Unknown backend: {self._backend}"}]
            )
    
    def _compile_cpython(self, source: Path, output: Optional[Path],
                         optimize: int) -> CompileResult:
        """Compile to .pyc bytecode using py_compile."""
        try:
            if source.is_dir():
                # Compile directory
                compileall.compile_dir(
                    str(source),
                    force=True,
                    optimize=optimize,
                    quiet=1
                )
                return CompileResult(
                    success=True,
                    returncode=0,
                    stdout=f"Compiled directory {source} to bytecode (opt={optimize})",
                    stderr="",
                    output_file=output
                )
            else:
                # Compile single file
                if output:
                    py_compile.compile(str(source), cfile=str(output), optimize=optimize)
                else:
                    py_compile.compile(str(source), optimize=optimize)
                
                return CompileResult(
                    success=True,
                    returncode=0,
                    stdout=f"Compiled {source} to bytecode (opt={optimize})",
                    stderr="",
                    output_file=output
                )
        except py_compile.PyCompileError as e:
            return CompileResult(
                success=False,
                returncode=-1,
                stdout="",
                stderr=str(e),
                diagnostics=[{"type": "error", "message": str(e)}]
            )
    
    def _compile_cython(self, source: Path, output: Optional[Path],
                        options: Optional[List[str]]) -> CompileResult:
        """Compile .py to C using Cython, then to native binary."""
        cmd = ["cython", "--fast-fail", "-3", str(source)]
        if output:
            c_output = output.with_suffix(".c")
            cmd.extend(["-o", str(c_output)])
        
        result = self._run(cmd)
        if not result.success:
            return result
        
        # Compile generated C code with gcc
        c_file = output.with_suffix(".c") if output else source.with_suffix(".c")
        if c_file.exists():
            gcc_cmd = ["gcc", "-O3", "-shared", "-fPIC", "-I/usr/include/python3.11",
                      str(c_file), "-o", str(output or source.with_suffix(".so"))]
            return self._run(gcc_cmd)
        
        return result
    
    def _compile_nuitka(self, source: Path, output: Optional[Path],
                        options: Optional[List[str]]) -> CompileResult:
        """Compile Python to standalone binary using Nuitka."""
        cmd = ["python3", "-m", "nuitka", "--standalone", "--follow-imports"]
        
        if output:
            cmd.extend(["--output-dir", str(output.parent), "--output-filename", output.name])
        
        if options:
            cmd.extend(options)
        
        cmd.append(str(source))
        return self._run(cmd)
    
    def compile_to_os_module(self, source: Path, output_dir: Path,
                             module_name: str) -> CompileResult:
        """Compile Python code as an embeddable OS module."""
        output_dir = Path(output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
        
        # Generate optimized bytecode
        pyc_dir = output_dir / "__pycache__"
        pyc_dir.mkdir(exist_ok=True)
        
        try:
            compileall.compile_file(
                str(source),
                ddir=str(output_dir),
                force=True,
                quiet=1
            )
            
            # Create module init
            init_file = output_dir / "__init__.py"
            init_file.write_text(f"# OS Python module: {module_name}\\n")
            
            return CompileResult(
                success=True,
                returncode=0,
                stdout=f"Created OS module {module_name} at {output_dir}",
                stderr="",
                output_file=output_dir
            )
        except Exception as e:
            return CompileResult(
                success=False, returncode=-1, stdout="", stderr=str(e),
                diagnostics=[{"type": "error", "message": str(e)}]
            }