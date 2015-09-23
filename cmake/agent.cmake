## Target executable name.
set (TARGET hsa_agent)

## Specify the SRC_DIR.
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src/core/agent")

## Included source files.
set (SOURCE_FILES hsa_agent.c test_concurrent_iterate.c test_concurrent_query.c test_iterate_null_data.c test_iterate_terminate.c test_query_attributes.c test_query_system_attributes.c)

## Test list.
set (TEST_LIST iterate_null_data iterate_terminate iterate_query_attributes concurrent_query concurrent_iterate query_system_attributes)

include (build)
include (test)
