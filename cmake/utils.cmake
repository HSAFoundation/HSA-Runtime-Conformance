## Test utilities library name.
set (TARGET ${UTILS_LIBRARY})

## Specify the SRC_DIR.
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src/utils")

## Included source files.
set (SOURCE_FILES agent_utils.c concurrent_utils.c dispatch_utils.c finalize_utils.c image_utils.c queue_utils.c)

## Library build directives.
include(buildlib)
