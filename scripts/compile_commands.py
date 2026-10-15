#!/usr/bin/env python3
"""Generate compile_commands.json for editors (VS Code C/C++, clangd).

Runs `make -Bn` in every C build directory and records each compile
command with its working directory: the kernel and the user programs use
different include roots (both have an `include/def.h`), so a single global
include path cannot resolve them correctly.
"""
import glob
import json
import os
import shlex
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DIRS = ["src", "src/boot", "programs/libs"] + sorted(glob.glob("programs/sources/*", root_dir=ROOT))
SOURCE_EXT = (".c", ".s", ".S")


def commands(rel_dir):
    directory = os.path.join(ROOT, rel_dir)
    if not os.path.isfile(os.path.join(directory, "makefile")):
        return []
    env = dict(os.environ, LC_ALL="C")
    out = subprocess.run(["make", "-Bn", "-C", directory], env=env,
                         capture_output=True, text=True).stdout
    entries = []
    for line in out.splitlines():
        try:
            args = shlex.split(line)
        except ValueError:
            continue
        if not args or not args[0].endswith("gcc") or "-c" not in args:
            continue
        for arg in args[1:]:
            if arg.endswith(SOURCE_EXT):
                entries.append({
                    "directory": directory,
                    "file": os.path.normpath(os.path.join(directory, arg)),
                    "arguments": args,
                })
    return entries


def main():
    db = [e for d in DIRS for e in commands(d)]
    path = os.path.join(ROOT, "compile_commands.json")
    with open(path, "w") as f:
        json.dump(db, f, indent=2)
    print(f"{path}: {len(db)} entries", file=sys.stderr)


if __name__ == "__main__":
    main()
