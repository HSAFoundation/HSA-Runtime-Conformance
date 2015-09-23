## Target executable name.
set (TARGET hsa_aql)

## Specify the SRC_DIR.
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src/core/aql")

## Included source files.
set (SOURCE_FILES hsa_aql.c test_aql_barrier_and.c test_aql_barrier_bit_not_set.c test_aql_barrier_bit_set.c test_aql_barrier_cross_queue_dependency.c test_aql_barrier_cross_queue_dependency_negative_value.c test_aql_barrier_multiple_barriers.c test_aql_barrier_negative_value.c test_aql_barrier_or.c test_aql_group_memory.c test_aql_group_memory_overspecified.c test_aql_launch_size.c test_aql_private_memory.c test_aql_private_memory_overspecified.c test_helper_func.c test_aql_zero_wg_size.c)

## Test list.
set (TEST_LIST aql_launch_size aql_barrier_bit_not_set aql_barrier_bit_set aql_barrier_cross_queue_dependency aql_barrier_cross_queue_dependency_negative_value aql_barrier_multiple_barriers aql_group_memory aql_group_memory_overspecified aql_private_memory aql_private_memory_overspecified aql_barrier_and aql_barrier_or aql_zero_wg_size) 

include (build)
include (test)
