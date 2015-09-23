## Target executable name.
set (TARGET hsa_init)

## Specify the SRC_DIR.
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src/core/init")

## Included source files.
set (SOURCE_FILES hsa_init.c test_concurrent_init.c test_concurrent_init_shutdown.c test_concurrent_shutdown.c test_refcount.c test_reinitialize.c)

## Test list.
set (TEST_LIST concurrent_init concurrent_shutdown concurrent_init_shutdown refcount reinitialize)

include (build)
include (test)
