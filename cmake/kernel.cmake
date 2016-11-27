## Specify the BRIG_DIR.
set (BRIG_DIR "${CMAKE_SOURCE_DIR}/src/kernels")

## Included source files.
set (BRIG_FILES agent_dispatch.brig depend_module1.brig depend_module2.brig global_vector_copy.brig global_vector_copy_base_large.brig global_agent_vector_copy.brig group_memory.brig init_data.brig memory_ops.brig mixed_scope.brig module_scope.brig no_op2.brig no_op.brig no_op_small.brig private_memory.brig program_scope.brig readonly_vector_copy.brig signal_operations.brig vector_copy.brig verify_image_region.brig kernarg_align.brig recursive_func.brig control_device.brig global_agent_vector_copy_base_large.brig no_op_base_large.brig recursive_func_base_large.brig program_scope_base_large.brig module_scope_base_large.brig mixed_scope_base_large.brig vector_copy_base_large.brig global_vector_copy_base_large.brig readonly_vector_copy_base_large.brig)

add_custom_target(copy-brig-files ALL)

foreach (BRIG_FILE ${BRIG_FILES})

    ## Copy to the binary directory to support `make test`.
    add_custom_command (TARGET copy-brig-files COMMAND ${CMAKE_COMMAND} -E copy ${BRIG_DIR}/${BRIG_FILE} ${CMAKE_BINARY_DIR})

    ## Add support for the `make install` command.
    install (FILES ${BRIG_DIR}/${BRIG_FILE} DESTINATION ${INSTALL_DIR})

endforeach(BRIG_FILE)

