## Target executable name.
set (TARGET hsa_code)

## Specify the SRC_DIR.
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src/core/code")

## Included source files.
set (SOURCE_FILES hsa_code.c test_helper_func.c test_code_define_global_agent.c test_code_define_global_program.c test_code_define_readonly_agent.c test_code_mixed_scope.c test_code_module_scope_symbol.c test_code_program_scope_symbol.c test_code_multiple_executables.c test_code_serialize_deserialize.c test_code_iterate_symbols.c test_code_kernarg_alignment.c test_code_recursive_kernel_function.c)

## Test list.
set (TEST_LIST code_define_globaL_agent code_define_global_program code_define_readonly_agent code_mixed_scope code_module_scope_symbol code_program_scope_symbol code_multiple_executables code_serialize_deserialize code_iterate_symbols code_kernarg_alignment code_recursive_kernel_function)

include (build)
include (test)
