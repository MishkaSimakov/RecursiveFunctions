#!/usr/bin/env python

import sys
import os
import subprocess
import tempfile


def main():
    [_, tea_compiler, _, _, program] = sys.argv

    with tempfile.TemporaryDirectory() as tempdir:
        # compile
        exe = os.path.join(tempdir, "exe")
        res = subprocess.run([tea_compiler, program, "--emit", "exe", "-o", exe])
        assert res.returncode == 0

        # run
        subprocess.run([exe])


if __name__ == "__main__":
    main()
