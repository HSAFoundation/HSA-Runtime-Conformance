## Add tests to the test list

set (COMMAND_STRING "")

foreach (TEST ${TEST_LIST})

    string (STRIP ${TEST} TEST)

    if (TEST)

        add_test (NAME ${TEST} WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${EXECUTION_SCRIPT} ${TARGET} ${TEST})

        file (APPEND ${CMAKE_BINARY_DIR}/test.lst  "${TEST}:${TARGET}\n")

    endif ()

endforeach ()

## Install the test.lst file to support traditional execution
install(FILES ${CMAKE_BINARY_DIR}/test.lst DESTINATION ${INSTALL_DIR})

set (TEST_LIST "")
