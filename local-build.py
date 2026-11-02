"""
Author: Aren Permadi
Email: aren.permadi@digipen.edu
Copyright: DigiPen Institute of Technology 2026
Brief:
    Python script meant for locally building Grossly Godot.
"""


import argparse
from operator import sub
import subprocess
import sys
from pathlib import Path


USER_CACHE_PATH = Path.home() / "AppData/Local/SConsCache"
BASE_ARGS = ["scons",
    "platform=windows",
    "module_mono_enabled=yes"]
GLUE_ARGS = ["./bin/godot.windows.editor.x86_64.llvm.mono",
    "--headless",
    "--generate-mono-glue",
    "modules/mono/glue"]
ASMB_ARGS = ["python",
    "./modules/mono/build_scripts/build_assemblies.py",
    "--godot-output-dir=./bin",
    "--godot-platform=windows",
    "--push-nupkgs-local"]
EDITOR_ARGS = ["target=editor", "use_llvm=yes"]
RELEASE_ARGS = ["target=template_release"]
DEBUG_ARGS = ["target=template_debug"]


def check_scons_cache_path() -> bool:
    if sys.platform != "win32":
        print("This script only supports Windows! Now exiting.")
        return False

    if not USER_CACHE_PATH.is_dir():
        print("Making directory. . .")
        USER_CACHE_PATH.mkdir()
    else:
        print("Directory exists, continuing . . .")

    return True


def build_templates():
    _result = subprocess.run(BASE_ARGS+RELEASE_ARGS)
    _result = subprocess.run(BASE_ARGS+DEBUG_ARGS)


def build_editor(do_build_temps):
    _result = subprocess.run(BASE_ARGS+EDITOR_ARGS)
    if do_build_temps:
        build_templates()
    _result = subprocess.run(GLUE_ARGS)
    _result = subprocess.run(ASMB_ARGS)

def main():
    parser = argparse.ArgumentParser("local-build")
    parser.add_argument("--nugetpath",
                        help="Path to desired Local Nuget Source location.")
    parser.add_argument("--templates",
                        help="Optional flag to build the export templates for Grossly Godot. Defaults to False",
                        type=bool,
                        choices=[True, False],
                        default=False)

    nuget_path = parser.parse_args().nugetpath
    if not nuget_path:
        print("No nuget path was designated! Now exiting.")
        return -1
    ASMB_ARGS.append(nuget_path)

    if check_scons_cache_path() is False:
        return -1

    build_editor(parser.parse_args().templates)


if __name__ == "__main__":
    main()
