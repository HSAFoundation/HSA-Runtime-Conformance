## Set the value of LIBRARY_LINK_DIRS.
set (LIBRARY_LINK_DIRS ${HSA_RUNTIME_LIBRARY_DIR})

## Name of the utilities library.
set (UTILS_LIBRARY hsa_utils)

## Test libraries.
set (TEST_LIBRARIES ${UTILS_LIBRARY})

## HSA libraries.
set (HSA_LIBRARIES ${HSA_RUNTIME_LIBRARY})

## System libraries.
set (SYSTEM_LIBRARIES check rt m pthread)

## Coding standard used.
set (C_STANDARD "-std=c99")

## Include directories compilation command.
set (INCLUDE_DIRS "-I ${CMAKE_SOURCE_DIR}/src/framework -I ${CMAKE_SOURCE_DIR}/src/utils -I ${HSA_RUNTIME_INCLUDE_DIR}")
MESSAGE("-- INCLUDE_DIRS ${INCLUDE_DIRS}")

## C flags.
set (CMAKE_C_FLAGS "${C_STANDARD} ${INCLUDE_DIRS}")
MESSAGE("-- CMAKE_C_FLAGS ${CMAKE_C_FLAGS}")

## Link flags.
set (CMAKE_EXE_LINKER_FLAGS "-Wl,--unresolved-symbols=ignore-in-shared-libs")
MESSAGE("-- CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINK_FLAGS}")

## Execution script to use for testing.
## Default to run.sh.
if(NOT DEFINED EXECUTION_SCRIPT)
    set (EXECUTION_SCRIPT "run.sh")
endif()

## Set the installation directory
if(NOT DEFINED INSTALL_DIR)
    set (INSTALL_DIR "hsa_conformance")
endif()
