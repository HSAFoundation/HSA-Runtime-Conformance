## Build the list of source files.
set (SOURCE_LIST)

foreach (SOURCE_FILE ${SOURCE_FILES})

    set (SOURCE_LIST ${SOURCE_LIST} ${SRC_DIR}/${SOURCE_FILE})

endforeach ()

## Specify the link dierctories.
link_directories (${LIBRARY_LINK_DIRS})

## Add the executable.
add_executable (${TARGET} ${SOURCE_LIST})

## Specify the link targets.
target_link_libraries (${TARGET} ${TEST_LIBRARIES} ${SYSTEM_LIBRARIES} ${HSA_LIBRARIES})

## Indicate the executable should be installed.
install (PROGRAMS ${CMAKE_BINARY_DIR}/${TARGET} DESTINATION ${INSTALL_DIR})
