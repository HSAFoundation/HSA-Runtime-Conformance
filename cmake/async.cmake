## Target executable name.
set (TARGET hsa_async)

## Specify the SRC_DIR.
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src/core/async")

## Included source files.
set (SOURCE_FILES hsa_async.c test_async_utils.c test_async_invalid_group_memory.c test_async_invalid_packet.c test_async_invalid_dimensions.c test_async_invalid_kernel_object.c test_async_invalid_workgroup_size.c)

## Test list. 
set (TEST_LIST async_invalid_group_memory async_invalid_dimensions async_invalid_kernel_object)

include (build)
include (test)
