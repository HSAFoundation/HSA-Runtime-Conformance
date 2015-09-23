## Target executable name.
set (TARGET hsa_image_clear)

## Specify the SRC_DIR.
set (SRC_DIR "${CMAKE_SOURCE_DIR}/src/extensions/images/clear")

## Included source files.
set (SOURCE_FILES hsa_image_clear.c test_image_clear.c) 

include (image_data)

set (TEST_LIST "")

set (VALID "")
 
## Test list
foreach (CHANNEL_TYPE ${CHANNEL_TYPES})

    foreach (CHANNEL_ORDER ${CHANNEL_ORDERS})

        foreach (GEOMETRY ${GEOMETRIES})
          
            valid_image(${CHANNEL_TYPE} ${CHANNEL_ORDER} ${GEOMETRY} VALID)

            if(${VALID} MATCHES TRUE)

                set (TEST_LIST ${TEST_LIST} image_clear_${CHANNEL_TYPE}_${CHANNEL_ORDER}_${GEOMETRY})

            endif()

        endforeach()

    endforeach()

endforeach()

include (build)
include (test)
