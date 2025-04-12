import os
import lit.formats
import lit.LitConfig

config: lit.LitConfig

config.name = 'Tea'

config.suffixes = ['.tea']

config.test_source_root = os.path.join(config.src_root, 'tests/lit')
config.test_exec_root = os.path.join(config.obj_root, 'tests/lit')

config.substitutions.append(('%tlang', config.tea_path))
config.substitutions.append(("%FileCheck", config.llvm_filecheck))
config.substitutions.append(("%llc", config.llvm_llc))
config.substitutions.append(("%clang", config.llvm_clang))

executor = os.path.join(config.src_root, 'tests/lit/execution/executor.py')
execute_order = f"{executor} {config.tea_path} {config.library} {config.llvm_llc} {config.llvm_clang}"
config.substitutions.append(("%execute", execute_order))

config.test_format = lit.formats.ShTest()
