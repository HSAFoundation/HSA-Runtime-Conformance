## Target executable name.
set (TARGET hsa_ext_api)

## Specify the SRC_DIR.
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src/extensions/ext_api")

## Included source files.
set (SOURCE_FILES hsa_ext_api.c test_hsa_ext_program_create.c test_hsa_ext_program_finalize test_hsa_ext_program_destroy.c)

## Test list.
## Disable the hsa_ext_program_destroy_invalid_program test.
set (TEST_LIST hsa_ext_program_create hsa_ext_program_create_not_initialized hsa_ext_program_create_invalid_argument hsa_ext_program_destroy hsa_ext_program_destroy_not_initialized hsa_ext_program_destroy_invalid_program hsa_ext_program_add_module hsa_ext_program_add_module_not_initialized hsa_ext_program_add_module_errors)
##set (TEST_LIST hsa_ext_program_create hsa_ext_program_create_not_initialized hsa_ext_program_create_invalid_argument hsa_ext_program_destroy hsa_ext_program_destroy_not_initialized hsa_ext_program_add_module hsa_ext_program_add_module_not_initialized hsa_ext_program_add_module_errors)

include (build)
include (test)
