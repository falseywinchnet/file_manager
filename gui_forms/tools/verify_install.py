#!/usr/bin/env python3
"""Build an independent consumer and check installed CMake component failures."""
import argparse
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile


def run(command, expected_success=True):
    result = subprocess.run(command, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, check=False)
    if (result.returncode == 0) != expected_success:
        raise RuntimeError("Unexpected command result: " + " ".join(command) + "\n" + result.stdout)
    return result.stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("prefix", type=Path)
    parser.add_argument("--native", action="store_true", help="Exercise the selected native Application component")
    parser.add_argument("--toolchain", type=Path, help="Target CMake toolchain for cross-build verification")
    parser.add_argument("--runner", help="CTest cross-compiling emulator command, such as wine")
    parser.add_argument("--runtime-dir", type=Path, help="Directory containing the MinGW runtime DLLs for a Wine run")
    parser.add_argument("--relocate", action="store_true", help="Copy the installed SDK to a new prefix before verification")
    args = parser.parse_args()
    prefix = args.prefix.resolve(strict=True)
    project = Path(__file__).resolve().parent.parent
    with tempfile.TemporaryDirectory(prefix="gui-forms-installed-consumer-") as scratch:
        scratch_path = Path(scratch)
        if args.relocate:
            relocated = scratch_path / "relocated-sdk"
            shutil.copytree(prefix, relocated, symlinks=True)
            prefix = relocated
        package_arguments = ["-DCMAKE_PREFIX_PATH=" + str(prefix)]
        if args.toolchain:
            package_arguments += ["-DCMAKE_TOOLCHAIN_FILE=" + str(args.toolchain.resolve(strict=True)),
                                  "-DCMAKE_FIND_ROOT_PATH=" + str(prefix)]
        if args.runner:
            package_arguments.append("-DCMAKE_CROSSCOMPILING_EMULATOR=" + ";".join(shlex.split(args.runner)))
        consumer = scratch_path / "consumer"
        shutil.copytree(project / "examples" / "paint_contract", consumer)
        build = scratch_path / "build"
        configure = ["cmake", "-S", str(consumer), "-B", str(build)] + package_arguments
        if args.native:
            configure.append("-DGUI_FORMS_EXAMPLE_NATIVE=ON")
        run(configure)
        run(["cmake", "--build", str(build), "--parallel", "2"])
        if args.runtime_dir:
            for name in ("libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll"):
                shutil.copy2(args.runtime_dir / name, build / name)
        print(run(["ctest", "--test-dir", str(build), "--output-on-failure"]))
        invalid = scratch_path / "invalid"
        invalid.mkdir()
        components = ["NotAComponent"]
        if not args.native:
            components.append("Application")
        for component in components:
            (invalid / "CMakeLists.txt").write_text(
                "cmake_minimum_required(VERSION 3.25)\n"
                "project(InvalidGUIFormsConsumer LANGUAGES CXX)\n"
                "find_package(GUIForms CONFIG REQUIRED COMPONENTS " + component + ")\n")
            output = run(["cmake", "-S", str(invalid), "-B", str(scratch_path / component)] +
                         package_arguments, expected_success=False)
            if "Requested GUIForms component '" + component + "' is not installed" not in output:
                raise RuntimeError("Failure was not the expected component diagnostic:\n" + output)
            print("Missing component rejected: " + component)
    print("Installed-package verification passed")


if __name__ == "__main__":
    main()
