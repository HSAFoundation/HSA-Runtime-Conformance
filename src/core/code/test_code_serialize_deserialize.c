/*
 * =============================================================================
 *   HSA Runtime Conformance Release License
 * =============================================================================
 * The University of Illinois/NCSA
 * Open Source License (NCSA)
 *
 * Copyright (c) 2014, Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Developed by:
 *
 *                 AMD Research and AMD HSA Software Development
 *
 *                 Advanced Micro Devices, Inc.
 *
 *                 www.amd.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimers.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimers in
 *    the documentation and/or other materials provided with the distribution.
 *  - Neither the names of <Name of Development Group, Name of Institution>,
 *    nor the names of its contributors may be used to endorse or promote
 *    products derived from this Software without specific prior written
 *    permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS WITH THE SOFTWARE.
 *
 */

/**
 * Test Name: test_code_serialize_deserialize
 * Scope: Conformance
 *
 * Purpose: Serialize a code object to a binary blob, and then
 * deserialize the blob back into a code object.
 *
 * Test Description:
 * 1. Create a code object by loading the no_op Brig module.
 * 2. For each agent that supports kernel dispatch, perform the followings.
 * 3. Finalizing the module with the profile and machine model match to the
 *    current agent.
 * 4. Serialize the code object, using valid callbacks and parameters to
 *    perform the serialization.
 * 5. Extract and compare the symbols from code objects and Executable before and after serialize-deserialize for their correctness in various attributes
 * 6. Destroy the original code object.
 * 7. Deserialize the serialized data to create a new code object.
 * 8.
 *    a) Create an executable and load the code object.
 *    b) Extract the no_op kernel from the executable.
 *    c) Dispatch the executable on the agent.
 * 9. Destroy the Brig module.
 *
 * Expected Result: The final code object should be usable
 * and the associated symbols should be dispatchable on
 * all agents.And all the code object and excutable symbols should be matching after serialize-deserialize as well
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <inttypes.h>
#include "test_helper_func.h"

int test_code_serialize_deserialize() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the finalization funtion pointer table
    hsa_ext_finalizer_pfn_t pfn;
    status = get_finalization_fnc_tbl(&pfn);
    // This indicates that finalization isn't
    // supported. The test will succeed in that
    // case.
    if(HSA_STATUS_SUCCESS != status) {
        return 0;
    }

    // Get a list of agents, and iterate throught the list
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Skip if this agent does not support kernel dispatch
        uint32_t feature = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &feature);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (HSA_AGENT_FEATURE_KERNEL_DISPATCH != feature) {
            continue;
        }
        
        // Load the BRIG module
        hsa_ext_module_t module;
        ASSERT(0 == load_base_or_full_module_from_file(agent_list.agents[ii],
                                                       "no_op_base_large.brig",
                                                       "no_op.brig",
                                                       &module));
        // Get the ISA from this agent
        hsa_isa_t agent_isa;
        agent_isa.handle = (uint64_t)-1;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_ISA, &agent_isa);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT((uint64_t)-1 != agent_isa.handle);

        // Get machine model and profile to create a program
        hsa_machine_model_t machine_model;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_MACHINE_MODEL, &machine_model);
        ASSERT(HSA_STATUS_SUCCESS == status);
        hsa_profile_t profile;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_PROFILE, &profile);
        ASSERT(HSA_STATUS_SUCCESS == status);
        hsa_default_float_rounding_mode_t default_float_rounding_mode;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE, &default_float_rounding_mode);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the program
        hsa_ext_program_t program;
        memset(&program, 0, sizeof(hsa_ext_program_t));
        status = pfn.hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, &program);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Add the brig modules to the program
        status = pfn.hsa_ext_program_add_module(program, module);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Finalize the program and extract the code object
        hsa_code_object_t code_object;
        memset(&code_object, 0, sizeof(hsa_code_object_t));
        hsa_ext_control_directives_t control_directives;
        memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
        status = pfn.hsa_ext_program_finalize(program, agent_isa, 0, control_directives, NULL, HSA_CODE_OBJECT_TYPE_PROGRAM, &code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the empty executable
        hsa_executable_t executable;
        status = hsa_executable_create(profile, HSA_EXECUTABLE_STATE_UNFROZEN, "", &executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Load the code object
        status = hsa_executable_load_code_object(executable, agent_list.agents[ii], code_object, "");
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Freeze the executable; it can now be queried for symbols
        status = hsa_executable_freeze(executable, "");
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Get the symbol and the symbol info from code object and executable before serialzation
        const int num_symbols = 1;
        char* symbol_names[1], name_1[1];
        char* code_object_symbol_name[1];
        symbol_names[0] = "&__no_op_kernel";
        code_object_symbol_name[0] = "&__no_op_kernel";
        int jj;
        struct symbol_attribute_info_s {
                        const char name[1];
                        uint32_t name_length;
                        uint32_t module_name_length;
                        char module_name[1];
                        const bool is_definition;
                        uint32_t variable_size;
                        const bool is_variable_const;
                        uint32_t kernel_kernarg_seg_size;
                        uint32_t kernel_group_seg_size;
                        uint32_t kernel_private_seg_size;
        };

        uint64_t kernel_object_1;
        const bool program_linkage[2] = {false, true};
        struct symbol_attribute_info_s *symbol_attribute_info[num_symbols];
        struct symbol_attribute_info_s *executable_symbol_attribute_info[num_symbols];
        for (jj = 0; jj < num_symbols; ++jj) {
            // Query the symbol from the code_object
            hsa_code_symbol_t code_symbol;
            symbol_attribute_info[jj] = (struct symbol_attribute_info_s *)malloc(sizeof(struct symbol_attribute_info_s));
            executable_symbol_attribute_info[jj] = (struct symbol_attribute_info_s *)malloc(sizeof(struct symbol_attribute_info_s));
            code_symbol.handle = (uint64_t)-1;
            status = hsa_code_object_get_symbol(code_object, symbol_names[jj], &code_symbol);
            ASSERT(HSA_STATUS_SUCCESS == status);

            // Query the symbol info from code_object
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_NAME, (void *)symbol_attribute_info[jj]->name);
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_NAME_LENGTH, (void*) &(symbol_attribute_info[jj]->name_length));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_MODULE_NAME_LENGTH, (void*) &(symbol_attribute_info[jj]->module_name_length));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_MODULE_NAME, (void*) &(symbol_attribute_info[jj]->module_name));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_IS_DEFINITION, (void*) &(symbol_attribute_info[jj]->is_definition));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, (void*) &(symbol_attribute_info[jj]->kernel_kernarg_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, (void*) &(symbol_attribute_info[jj]->kernel_group_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, (void*) &(symbol_attribute_info[jj]->kernel_private_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);
            // Query the symbol from the executable
            hsa_executable_symbol_t executable_symbol;
            executable_symbol.handle = (uint64_t)-1;
            int32_t call_convention = 0;
            const char* executable_module_name = NULL;
            status = hsa_executable_get_symbol(executable,
                                               executable_module_name,
                                               symbol_names[jj],
                                               agent_list.agents[ii],
                                               call_convention,
                                               &executable_symbol);
            ASSERT(HSA_STATUS_SUCCESS == status);
            // Query the symbol info from the executable
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_NAME, (void*) executable_symbol_attribute_info[jj]->name);
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_NAME_LENGTH, (void*) &(executable_symbol_attribute_info[jj]->name_length));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME_LENGTH, (void*) &(executable_symbol_attribute_info[jj]->module_name_length));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME, (void*) &(executable_symbol_attribute_info[jj]->module_name));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_IS_DEFINITION, (void*) &(executable_symbol_attribute_info[jj]->is_definition));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, (void*) &(executable_symbol_attribute_info[jj]->kernel_kernarg_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, (void*) &(executable_symbol_attribute_info[jj]->kernel_group_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, (void*) &(executable_symbol_attribute_info[jj]->kernel_private_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);

        }

        // Serialize the code object
        void* serialized_blob;
        size_t serialized_size;
        hsa_callback_data_t callback_data;
        callback_data.handle = (uint64_t)-1;
        status = hsa_code_object_serialize(code_object, callback_serialize_alloc,
            callback_data, NULL, &serialized_blob, &serialized_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Deserialize the memory blob
        hsa_code_object_t deserialized_code_object;
        status = hsa_code_object_deserialize(serialized_blob,
            serialized_size, NULL, &deserialized_code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free the memory blob
        free(serialized_blob);
        // Free the original code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the empty executable
        hsa_executable_t executable_1;
        status = hsa_executable_create(profile, HSA_EXECUTABLE_STATE_UNFROZEN, "", &executable_1);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Load the deserialized  code object
        status = hsa_executable_load_code_object(executable_1, agent_list.agents[ii], deserialized_code_object, "");
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Freeze the executable; it can now be queried for symbols
        status = hsa_executable_freeze(executable_1, "");
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Get the symbol and the symbol info
        symbol_record_t symbol_record;
        memset(&symbol_record, 0, sizeof(symbol_record_t));
        status = get_executable_symbols(executable_1, agent_list.agents[ii], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Extract the symbols info for comaprision on serialize-deserialized code object
        struct symbol_attribute_info_s *symbol_attribute_info_after[num_symbols];
        struct symbol_attribute_info_s *executable_symbol_attribute_info_after[num_symbols];
        for (jj = 0; jj < num_symbols; ++jj) {
            symbol_attribute_info_after[jj] = (struct symbol_attribute_info_s *)malloc(sizeof(struct symbol_attribute_info_s));
            executable_symbol_attribute_info_after[jj] = (struct symbol_attribute_info_s *)malloc(sizeof(struct symbol_attribute_info_s));
            // Query the symbol from the deserialized code_object
            hsa_code_symbol_t code_symbol;
            code_symbol.handle = (uint64_t)-1;
            status = hsa_code_object_get_symbol(deserialized_code_object, symbol_names[jj], &code_symbol);
            ASSERT(HSA_STATUS_SUCCESS == status);
            // Query the symbol info on deserialized code object
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_NAME, (void*) &(symbol_attribute_info_after[jj]->name));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_NAME_LENGTH, (void*) &(symbol_attribute_info_after[jj]->name_length));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_MODULE_NAME_LENGTH, (void*) &(symbol_attribute_info_after[jj]->module_name_length));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_MODULE_NAME, (void*) &(symbol_attribute_info_after[jj]->module_name));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_IS_DEFINITION, (void*) &(symbol_attribute_info_after[jj]->is_definition));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, (void*) &(symbol_attribute_info_after[jj]->kernel_kernarg_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, (void*) &(symbol_attribute_info_after[jj]->kernel_group_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, (void*) &(symbol_attribute_info_after[jj]->kernel_private_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);
           
            // Query the symbol from the deserialized code object executable
            hsa_executable_symbol_t executable_symbol;
            executable_symbol.handle = (uint64_t)-1;
            int32_t call_convention = 0;
            const char* executable_module_name = NULL;
            status = hsa_executable_get_symbol(executable_1,
                                               executable_module_name,
                                               symbol_names[jj],
                                               agent_list.agents[ii],
                                               call_convention,
                                               &executable_symbol);
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_NAME, (void*) &(executable_symbol_attribute_info_after[jj]->name));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_NAME_LENGTH, (void*) &(executable_symbol_attribute_info_after[jj]->name_length));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME_LENGTH, (void*) &(executable_symbol_attribute_info_after[jj]->module_name_length));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME, (void*) &(executable_symbol_attribute_info_after[jj]->module_name));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_IS_DEFINITION, (void*) &(executable_symbol_attribute_info_after[jj]->is_definition));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, (void*) &(executable_symbol_attribute_info_after[jj]->kernel_kernarg_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, (void*) &(executable_symbol_attribute_info_after[jj]->kernel_group_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);
            status = hsa_executable_symbol_get_info(executable_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, (void*) &(executable_symbol_attribute_info_after[jj]->kernel_private_seg_size));
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Compare the symbols info before and after serialization/deserialization on both code object and executable
        for (jj = 0; jj < num_symbols; ++jj) {
            // Compare Code object symbol attribute values
            if ((strcmp(symbol_attribute_info[jj]->name, symbol_attribute_info_after[jj]->name) == 0) &&
                (strcmp(symbol_attribute_info[jj]->module_name, symbol_attribute_info_after[jj]->module_name) == 0) &&
                (symbol_attribute_info[jj]->name_length == symbol_attribute_info_after[jj]->name_length) &&
                (symbol_attribute_info[jj]->module_name_length == symbol_attribute_info_after[jj]->module_name_length) &&
                (symbol_attribute_info[jj]->is_definition == symbol_attribute_info_after[jj]->is_definition)&&
                (symbol_attribute_info[jj]->kernel_kernarg_seg_size == symbol_attribute_info_after[jj]->kernel_kernarg_seg_size) &&
                (symbol_attribute_info[jj]->kernel_group_seg_size == symbol_attribute_info_after[jj]->kernel_group_seg_size) &&
                (symbol_attribute_info[jj]->kernel_private_seg_size == symbol_attribute_info_after[jj]->kernel_private_seg_size)) {
                // The comparisons passed.
            } else {
                ASSERT_MSG(0,"Not all of the symbol attributes generated from the deserialized code object matched the original.");
            }

            // Compare executable symbol attribute values
            if ((strcmp(executable_symbol_attribute_info[jj]->name, executable_symbol_attribute_info_after[jj]->name) == 0) &&
                (strcmp(executable_symbol_attribute_info[jj]->module_name, executable_symbol_attribute_info_after[jj]->module_name) == 0) &&
                (executable_symbol_attribute_info[jj]->name_length == executable_symbol_attribute_info_after[jj]->name_length) &&
                (executable_symbol_attribute_info[jj]->module_name_length == executable_symbol_attribute_info_after[jj]->module_name_length) &&
                (executable_symbol_attribute_info[jj]->is_definition == executable_symbol_attribute_info_after[jj]->is_definition)&&
                (executable_symbol_attribute_info[jj]->kernel_kernarg_seg_size == executable_symbol_attribute_info_after[jj]->kernel_kernarg_seg_size) &&
                (executable_symbol_attribute_info[jj]->kernel_group_seg_size == executable_symbol_attribute_info_after[jj]->kernel_group_seg_size) &&
                (executable_symbol_attribute_info[jj]->kernel_private_seg_size == executable_symbol_attribute_info_after[jj]->kernel_private_seg_size) ) {
            } else {
                ASSERT_MSG(0,"Not all of the symbol attributes generated from the deserialized executable matched the original.");
            }
        }

        // Use deserialized code object to dispatch a kernel
        // Queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 256, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Signal
        hsa_signal_t signal;
        status = hsa_signal_create(1, 0, NULL, &signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Dispatch packet
        hsa_kernel_dispatch_packet_t dispatch_packet;
        const size_t packet_size = sizeof(hsa_kernel_dispatch_packet_t);

        // Fill info for the default dispatch_packet
        memset(&dispatch_packet, 0, packet_size);
        dispatch_packet.header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        dispatch_packet.setup |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
        dispatch_packet.workgroup_size_x = 256;
        dispatch_packet.workgroup_size_y = 1;
        dispatch_packet.workgroup_size_z = 1;
        dispatch_packet.grid_size_x = 256;
        dispatch_packet.grid_size_y = 1;
        dispatch_packet.grid_size_z = 1;
        dispatch_packet.group_segment_size = symbol_record.group_segment_size;
        dispatch_packet.private_segment_size = symbol_record.private_segment_size;
        dispatch_packet.kernel_object = symbol_record.kernel_object;
        dispatch_packet.kernarg_address = 0;
        dispatch_packet.completion_signal = signal;

        // Enqueue dispatch packets
        hsa_kernel_dispatch_packet_t* queue_packet;
        // Increment the write index of the queue
        uint64_t write_index = hsa_queue_add_write_index_relaxed(queue, 1);
        // Obtain the address of the queue packet entry
        queue_packet = (hsa_kernel_dispatch_packet_t*)(queue->base_address + write_index * packet_size);
        // Copy the initialized packet to the queue packet entry
        memcpy(queue_packet, &dispatch_packet, packet_size);
        // Set the queue packet entries header.type value to HSA_PACKET_TYPE_KERNEL_DISPATCH
        // This allows the command processor to process this packet.
        queue_packet->header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
        // Ring the doorbell
        hsa_signal_store_relaxed(queue->doorbell_signal, write_index);

        // Wait until all dispatch packets finish executing
        hsa_signal_value_t value = hsa_signal_wait_relaxed(signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
        ASSERT(0 == value);

        // Destroy signal
        status = hsa_signal_destroy(signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable, program, and the deserialized code object
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_executable_destroy(executable_1);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(deserialized_code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = pfn.hsa_ext_program_destroy(program);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);
        for(jj = 0; jj < num_symbols; ++jj) {
            free(symbol_attribute_info[jj]);
            free(executable_symbol_attribute_info[jj]);
            free(symbol_attribute_info_after[jj]);
            free(executable_symbol_attribute_info_after[jj]);
            }
            destroy_module(module);
    }

    free_agent_list(&agent_list);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
