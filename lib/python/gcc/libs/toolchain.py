# SPDX-License-Identifier: GPL-2.0
import os
from pathlib import Path
from typing import Dict, List, Optional, Type
from .base import Compiler, CompileResult
from .gcc import GCCCompiler
from .elixir import ElixirCompiler
from .ada import AdaCompiler
from .c import CCompiler
from .html import HTMLCompiler
from .python import PythonCompiler


class ToolchainManager:
    """Manages all compilers in the OS build environment."""
    
    COMPILER_MAP: Dict[str, Type[Compiler]] = {
        "gcc": GCCCompiler,
        "elixir": ElixirCompiler,
        "ada": AdaCompiler,
        "c": CCompiler,
        "html": HTMLCompiler,
        "python": PythonCompiler,
    }
    
    def __init__(self, os_prefix: Optional[Path] = None,
                 cross_compile: bool = False,
                 target_arch: str = "x86_64"):
        """
        Initialize the toolchain manager.
        
        Args:
            os_prefix: Root directory of the custom OS (for sysroot)
            cross_compile: Enable cross-compilation mode
            target_arch: Target architecture
        """
        self.os_prefix = Path(os_prefix) if os_prefix else None
        self.cross_compile = cross_compile
        self.target_arch = target_arch
        self._compilers: Dict[str, Compiler] = {}
        self._env = self._build_env()
    
    def _build_env(self) -> Dict[str, str]:
        """Build environment variables for OS compilation."""
        env = os.environ.copy()
        if self.os_prefix:
            env["SYSROOT"] = str(self.os_prefix)
            env["C_INCLUDE_PATH"] = str(self.os_prefix / "usr" / "include")
            env["LIBRARY_PATH"] = str(self.os_prefix / "usr" / "lib")
        if self.cross_compile:
            env["TARGET_ARCH"] = self.target_arch
        return env
    
    def get_compiler(self, name: str, **kwargs) -> Compiler:
        """Get or create a compiler instance."""
        if name not in self._compilers:
            compiler_cls = self.COMPILER_MAP.get(name)
            if not compiler_cls:
                raise ValueError(f"Unknown compiler: {name}. Available: {list(self.COMPILER_MAP.keys())}")
            
            # Merge environment
            instance_env = {**self._env, **kwargs.pop("os_env", {})}
            self._compilers[name] = compiler_cls(os_env=instance_env, **kwargs)
        
        return self._compilers[name]
    
    def detect_available(self) -> Dict[str, bool]:
        """Detect which compilers are available in PATH."""
        return {
            name: compiler_cls().available
            for name, compiler_cls in self.COMPILER_MAP.items()
        }
    
    def build_os_kernel(self, source_dir: Path, output_dir: Path,
                        sources: Dict[str, List[Path]]) -> Dict[str, CompileResult]:
        """
        Build OS kernel components from multiple languages.
        
        Args:
            source_dir: Root source directory
            output_dir: Build output directory
            sources: Dict mapping compiler name to list of source files
            
        Returns:
            Dict of compiler name -> CompileResult
        """
        output_dir = Path(output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
        results = {}
        
        for compiler_name, files in sources.items():
            compiler = self.get_compiler(compiler_name)
            
            for src in files:
                src = Path(src)
                if not src.is_absolute():
                    src = source_dir / src
                
                out = output_dir / src.with_suffix(".o").name
                
                if compiler_name == "gcc":
                    result = compiler.compile_kernel_module(src, out, arch=self.target_arch)
                elif compiler_name == "c":
                    result = compiler.compile(src, out, freestanding=True, arch=self.target_arch)
                elif compiler_name == "ada":
                    result = compiler.compile(src, out, mode="gcc")
                elif compiler_name == "python":
                    result = compiler.compile_to_os_module(src, output_dir / "py_modules", src.stem)
                elif compiler_name == "html":
                    result = compiler.compile(src, output_dir / "ui" / src.name, minify=True, inline_css=True)
                else:
                    result = compiler.compile(src, out)
                
                results[f"{compiler_name}:{src.name}"] = result
        
        return results
    
    def build_userland(self, source_dir: Path, output_dir: Path,
                       programs: List[Dict]) -> Dict[str, CompileResult]:
        """
        Build userland programs.
        
        Args:
            programs: List of dicts with keys: name, compiler, source, options
        """
        output_dir = Path(output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
        results = {}
        
        for prog in programs:
            name = prog["name"]
            compiler = self.get_compiler(prog["compiler"])
            src = Path(prog["source"])
            
            if not src.is_absolute():
                src = source_dir / src
            
            out = output_dir / name
            
            if prog["compiler"] == "elixir":
                result = compiler.compile_project(src.parent)
            elif prog["compiler"] == "python":
                result = compiler.compile(src, out, optimize=2)
            else:
                result = compiler.compile(src, out, options=prog.get("options"))
            
            results[name] = result
        
        return results
    
    def link_os_image(self, objects: List[Path], output: Path,
                     linker_script: Optional[Path] = None) -> CompileResult:
        """Link all object files into the final OS image."""
        gcc = self.get_compiler("gcc")
        
        cmd = [gcc.executable, "-nostdlib", "-fno-builtin", "-ffreestanding"]
        if linker_script:
            cmd.extend(["-T", str(linker_script)])
        cmd.extend(["-o", str(output)])
        cmd.extend([str(o) for o in objects])
        
        return gcc._run(cmd)