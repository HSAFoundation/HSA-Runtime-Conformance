## Target executable name.
set (TARGET hsa_image_import_export)

## Specify the SRC_DIR.
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src/extensions/images/import_export")

## Included source files.
set (SOURCE_FILES hsa_image_import_export.c test_image_import_export.c) 

include (image_data)

## Test list
foreach (CHANNEL_TYPE ${CHANNEL_TYPES})

    foreach (CHANNEL_ORDER ${CHANNEL_ORDERS})

        foreach (GEOMETRY ${GEOMETRIES})

            valid_image(${CHANNEL_TYPE} ${CHANNEL_ORDER} ${GEOMETRY} VALID)

            if(${VALID} MATCHES TRUE)

                set (TEST_LIST ${TEST_LIST} image_import_export_${CHANNEL_TYPE}_${CHANNEL_ORDER}_${GEOMETRY})

            endif()

        endforeach()

    endforeach()

endforeach()

include (build)
include (test)
