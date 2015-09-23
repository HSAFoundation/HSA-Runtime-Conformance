## Target executable name.
set (TARGET hsa_memory_atomics)

## Specify the SRC_DIR.
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src/core/memory/atomics")

## Source files.
set (SOURCE_FILES hsa_memory_atomics.c test_helper_func.c test_memory_add_atomic.c test_memory_and_atomic.c test_memory_cas_atomic.c test_memory_decrement_atomic.c test_memory_exchange_atomic.c test_memory_increment_atomic.c test_memory_load_store_atomic.c test_memory_maximum_atomic.c test_memory_minimum_atomic.c test_memory_or_atomic.c test_memory_subtract_atomic.c test_memory_xor_atomic.c)

## Test list.
set (TEST_LIST memory_add_atomic memory_and_atomic memory_cas_atomic memory_decrement_atomic memory_exchange_atomic memory_increment_atomic memory_load_store_atomic memory_maximum_atomic memory_minimum_atomic memory_or_atomic memory_subtract_atomic memory_xor_atomic) 

include (build)
include (test)
