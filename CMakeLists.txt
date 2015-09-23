project (hsa_conformance C)

cmake_minimum_required(VERSION 2.8)

## Windows is currently not supported. Disable windows builds.
if(WIN32)
  MESSAGE("-------- Windows build is not supported.")
  RETURN()
endif()

## Specify the location of the configuration files. 
set (CMAKE_CONFIG_DIR "${CMAKE_SOURCE_DIR}/cmake")

## Set the CMAKE module path the CMAKE_CONFIG_DIR.
set (CMAKE_MODULE_PATH ${CMAKE_CONFIG_DIR})

## Disable RPATH options
set (CMAKE_SKIP_RPATH TRUE)

## Determine if the required packages are installed.
find_package(Check REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})

find_package(HSA REQUIRED)
include_directories(${HSA_RUNTIME_INCLUDE_DIRS})

## Include common settings. 
include (common)

## Create the test utilities library.
include (utils)

## Enable testing.
enable_testing()

## Build the tests.
include (kernel)
include (script)
include (api)
include (agent)
include (aql)
include (async)
include (code)
include (init)
include (memory)
include (atomics)
include (queue)
include (signals)
include (ext_api)
include (finalization)
include (image_clear)
include (image_copy)
include (image_import_export)
