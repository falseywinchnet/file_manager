#!/usr/bin/env python3
"""Build an independent consumer and check installed CMake component failures."""
from __future__ import annotations
import argparse
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile


def run(command: list[str], expected_success: bool = True) -> str:
    result: subprocess.CompletedProcess[str] = subprocess.run(command, text=True, encoding='utf-8', stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, check=False)
    if (result.returncode == 0) != expected_success:
        raise RuntimeError("Unexpected command result: " + " ".join(command) + "\n" + result.stdout)
    return result.stdout


def main() -> None:
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("prefix", type=Path)
    parser.add_argument("--native", action="store_true", help="Exercise the selected native Application component")
    parser.add_argument("--toolchain", type=Path, help="Target CMake toolchain for cross-build verification")
    parser.add_argument("--runner", help="CTest cross-compiling emulator command, such as wine")
    parser.add_argument("--runtime-dir", type=Path, help="Directory containing the MinGW runtime DLLs for a Wine run")
    parser.add_argument("--relocate", action="store_true", help="Copy the installed SDK to a new prefix before verification")
    args: argparse.Namespace = parser.parse_args()
    prefix: Path = args.prefix.resolve(strict=True)
    project: Path = Path(__file__).resolve().parent.parent
    scratch: str
    with tempfile.TemporaryDirectory(prefix="gui-forms-installed-consumer-") as scratch:
        scratch_path: Path = Path(scratch)
        if args.relocate:
            relocated: Path = scratch_path / "relocated-sdk"
            shutil.copytree(prefix, relocated, symlinks=True)
            prefix = relocated
        package_arguments: list[str] = ["-DCMAKE_PREFIX_PATH=" + str(prefix)]
        if args.toolchain:
            toolchain_path: Path = args.toolchain.resolve(strict=True)
            package_arguments += ["-DCMAKE_TOOLCHAIN_FILE=" + str(toolchain_path),
                                  "-DCMAKE_FIND_ROOT_PATH=" + str(prefix)]
        if args.runner:
            emulator_arguments: list[str] = shlex.split(args.runner)
            emulator_value: str = ";".join(emulator_arguments)
            package_arguments.append("-DCMAKE_CROSSCOMPILING_EMULATOR=" + emulator_value)
        consumer: Path = scratch_path / "consumer"
        shutil.copytree(project / "examples" / "paint_contract", consumer)
        build: Path = scratch_path / "build"
        configure: list[str] = ["cmake", "-S", str(consumer), "-B", str(build)] + package_arguments
        if args.native:
            configure.append("-DGUI_FORMS_EXAMPLE_NATIVE=ON")
        run(configure)
        run(["cmake", "--build", str(build), "--parallel", "2"])
        if args.runtime_dir:
            name: str
            for name in ("libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll"):
                shutil.copy2(args.runtime_dir / name, build / name)
        test_output: str = run(["ctest", "--test-dir", str(build), "--output-on-failure"])
        print(test_output)
        invalid: Path = scratch_path / "invalid"
        invalid.mkdir()
        components: list[str] = ["NotAComponent"]
        if not args.native:
            components.append("Application")
        component: str
        for component in components:
            invalid_configuration: Path = invalid / "CMakeLists.txt"
            invalid_configuration.write_text(
                "cmake_minimum_required(VERSION 3.25)\n"
                "project(InvalidGUIFormsConsumer LANGUAGES CXX)\n"
                "find_package(GUIForms CONFIG REQUIRED COMPONENTS " + component + ")\n", encoding="utf-8")
            output: str = run(["cmake", "-S", str(invalid), "-B", str(scratch_path / component)] +
                         package_arguments, expected_success=False)
            if "Requested GUIForms component '" + component + "' is not installed" not in output:
                raise RuntimeError("Failure was not the expected component diagnostic:\n" + output)
            print("Missing component rejected: " + component)
    print("Installed-package verification passed")


if __name__ == "__main__":
    main()
