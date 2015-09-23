## Build the list of source files.
foreach (SOURCE_FILE ${SOURCE_FILES})

    set (SOURCE_LIST ${SOURCE_LIST} ${SRC_DIR}/${SOURCE_FILE})

endforeach ()

## Add the library.
add_library (${TARGET} STATIC ${SOURCE_LIST})
