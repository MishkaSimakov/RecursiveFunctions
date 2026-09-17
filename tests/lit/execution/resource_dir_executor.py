#!/usr/bin/env python

import os
import shutil
import sys
import subprocess
import tempfile


def main():
    [_, tea_compiler, resource_dir, program] = sys.argv

    tempdir = tempfile.mkdtemp()

    try:
        # the copied compiler can't find the standard library next to itself
        compiler = shutil.copy(tea_compiler, tempdir)

        # compile
        exe = os.path.join(tempdir, "exe")
        res = subprocess.run([compiler, program, "--emit", "exe", "-o", exe,
                              "--resource-dir", resource_dir])
        assert res.returncode == 0

        # run
        subprocess.run([exe])
    finally:
        shutil.rmtree(tempdir)


if __name__ == "__main__":
    main()
