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
 * Test Name: test_code_define_global_program
 * Scope: Conformance
 *
 * Purpose: Verify that a global variable with program allocation can
 * be defined for an executable and used during a dispatch.
 *
 *
 * Test Description:
 * 1. Create a code object by loading the global_variable Brig module
      and finalize the module.
 * 2. For each agent that supports kernel dispatch:
 *    a) Create an executable and load the code object.
 *    b) Define the global object using the hsa_executable_global_variable_define
 *       API with a valid name and address. Do not specify the agent.
 *    c) Extract a kernel that uses the global variable.
 *    d) Execute the kernel on the agent.
 *
 *  Expected Results: The kernel should execute correctly on each agent and the global
 *  variable should be accessed correctly.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <dispatch_utils.h>
#include "test_helper_func.h"

int test_code_define_global_program() {
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

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("global_vector_copy.brig", &module));

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

        hsa_code_object_t code_object;
        hsa_executable_t executable;
        hsa_code_object_type_t code_object_type = HSA_CODE_OBJECT_TYPE_PROGRAM;
        int32_t call_convention = 0;
        hsa_ext_control_directives_t control_directives;
        memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));

        // Create the program
        hsa_ext_program_t program;
        memset(&program, 0, sizeof(hsa_ext_program_t));
        status = pfn.hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, &program);
        ASSERT(HSA_STATUS_SUCCESS == status);
        // Add the brig modules to the program
        status = pfn.hsa_ext_program_add_module(program, module);
        ASSERT(HSA_STATUS_SUCCESS == status);
        // Finalize the program and extract the code object
        status = pfn.hsa_ext_program_finalize(program, agent_isa, call_convention, control_directives, "", code_object_type, &code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);
        // Create the empty executable
        status = hsa_executable_create(profile, HSA_EXECUTABLE_STATE_UNFROZEN, "", &executable);
        ASSERT(HSA_STATUS_SUCCESS == status);
        
        const uint32_t data_size = 256;
        // Allocate a global data array
        uint32_t* global_data = (uint32_t*)malloc(data_size * sizeof(uint32_t));
        memset(global_data, 0, data_size * sizeof(uint32_t));

        // Define this global variable
        status = hsa_executable_global_variable_define(executable, "&b", (void*)global_data);
        ASSERT(HSA_STATUS_SUCCESS == status);        
        
        // Load the code object
        status = hsa_executable_load_code_object(executable, agent_list.agents[ii], code_object, "");
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Freeze the executable; it can now be queried for symbols
        status = hsa_executable_freeze(executable, "");
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Releasing the program should not affect the executable
        pfn.hsa_ext_program_destroy(program);

        // Create a queue for dispatching
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate data to be used as the input of the kernel
        uint32_t* data_input;
        data_input = (uint32_t*)malloc(sizeof(uint32_t) * data_size);
        // Initialize the data
        uint32_t jj;
        for (jj = 0; jj < data_size; ++jj) {
            data_input[jj] = jj;
        }

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);
        // Allocate the kernel argument buffer from the correct region
        void* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region, sizeof(uint32_t*), &kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);
        // Setup the kernarg buffer
        memcpy((void*)kernarg_buffer, &data_input, sizeof(uint32_t*));

        // Get the symbol and the symbol info
        symbol_record_t symbol_record;
        memset(&symbol_record, 0, sizeof(symbol_record_t));
        char* symbol_names[1];
        symbol_names[0] = "&__global_vector_copy_kernel";
        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Launch the vector_copy kernel
        dispatch_kernel_1d_data(queue, data_size, (uint64_t)symbol_record.kernel_object, (void*)kernarg_buffer);

        // Verify the output data block is updated
        int cmp_results = memcmp(data_input, global_data, sizeof(uint32_t) * data_size);
        ASSERT(0 == cmp_results);

        // Free the kernarg memory buffer
        status = hsa_memory_free(kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);
        
        // Free the global data
        free(global_data);

        // Free the input data
        free(data_input);

        // Destroy the queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    free_agent_list(&agent_list);

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
