import os
import lit.formats
import lit.LitConfig

config: lit.LitConfig

config.name = 'Tea'

config.suffixes = ['.tea']

config.test_source_root = os.path.join(config.src_root, 'tests/lit')
config.test_exec_root = os.path.join(config.obj_root, 'tests/lit')

config.substitutions.append(('%tlang', config.tea_path))
config.substitutions.append(("%FileCheck", config.filecheck_path))

config.test_format = lit.formats.ShTest()
