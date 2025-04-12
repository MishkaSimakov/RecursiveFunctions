#!/usr/bin/env python

import sys
import os
import subprocess
import tempfile


def main():
    [_, tea_compiler, library, llc, clang, program] = sys.argv

    with tempfile.TemporaryDirectory() as tempdir:
        # compile tea file
        ir = os.path.join(tempdir, "out.ll")
        res = subprocess.run([tea_compiler, program, "--emit", "ir", "-o", ir])
        assert res.returncode == 0

        # compile llvm ir into assembly
        asm = os.path.join(tempdir, "out.s")
        subprocess.run([llc, ir, "-o", asm])

        # link our program and library
        exe = os.path.join(tempdir, "exe")
        subprocess.run([clang, asm, library, "-o", exe])

        # run resulting program
        subprocess.run([exe])


if __name__ == "__main__":
    main()
