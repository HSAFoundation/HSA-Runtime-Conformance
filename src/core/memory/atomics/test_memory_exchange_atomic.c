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

/*
 * Test Name: memory_exchange_atomic
 * Purpose: Test that if an agent supports atomic exchange
 * operations from global memory.
 *
 * Test Description:
 * 1. For all agents on the system that support kernel dispatch:
 * 2. Load and finalize the memory_exchange_global set of kernels,
 *    targeting add operations for all data types and memory ordering types.
 * 3. Allocate global memory appropriate for each kernel execution.
 * 4. Execute each of the kernels on the agent.
 *
 * Expected Results: The exchange kernels should be supported on the agent.
 * The kernels should be able to get dispatched and execute correctly.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include "test_helper_func.h"

int test_memory_exchange_atomic() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("memory_ops.brig", &module));

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Repeat the test for each agent
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Check if the agent supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Check if this agent has a global memory region that supports fine grained memory
        hsa_region_t global_region;
        global_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region_fine_grained, &global_region);
        if ((uint64_t)-1 == global_region.handle) {
            continue;
        }

        // Create a queue.
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate the "data" and "value" buffers
        void* data;
        void* value;
        status = hsa_memory_allocate(global_region, sizeof(uint64_t), (void**) &data);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_allocate(global_region, sizeof(uint64_t), (void**) &value);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Finalize the executable
        hsa_ext_control_directives_t control_directives;
        memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
        hsa_code_object_t code_object;
        hsa_executable_t executable;

        status = finalize_executable(agent_list.agents[ii],
                                     1,
                                     &module,
                                     HSA_MACHINE_MODEL_LARGE,
                                     HSA_PROFILE_FULL,
                                     HSA_DEFAULT_FLOAT_ROUNDING_MODE_ZERO,
                                     HSA_CODE_OBJECT_TYPE_PROGRAM,
                                     0,
                                     control_directives,
                                     &code_object,
                                     &executable);

        ASSERT(HSA_STATUS_SUCCESS == status);

        const int num_symbols = 16;
        char* symbol_names[num_symbols];
        int symbol_index = 0;

        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_rlx_agent_b32_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_rlx_agent_b64_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_rlx_system_b32_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_rlx_system_b64_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_scacq_agent_b32_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_scacq_agent_b64_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_scacq_system_b32_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_scacq_system_b64_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_screl_agent_b32_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_screl_agent_b64_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_screl_system_b32_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_screl_system_b64_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_scar_agent_b32_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_scar_agent_b64_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_scar_system_b32_kernel";
        symbol_names[symbol_index++] = "&__memory_atomic_exch_global_scar_system_b64_kernel";

        // Get the symbol and the symbol info
        symbol_record_t symbol_record[num_symbols];
        memset(&symbol_record, 0, sizeof(symbol_record_t));

        status = get_executable_symbols(executable, agent_list.agents[ii], 0, num_symbols, symbol_names, symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        const int num_kernel_instances = 64;

        // Launch kernels with b32 data type
        symbol_index = 0;
        while (symbol_index < num_symbols) {
            memset(data, 0, sizeof(uint64_t));
            memset(value, 0, sizeof(uint64_t));

            uint32_t* data_uint32 = (uint32_t*)data;
            uint32_t* value_uint32= (uint32_t*)value;
            *data_uint32 = 0;
            *value_uint32= 0xCCCCCCCC;

            launch_memory_atomic_kernel(&(agent_list.agents[ii]), queue, &symbol_record[symbol_index],
                    data, value, num_kernel_instances);

            // Verify the kernel is executed correctly
            uint32_t data_expected_u32 = *value_uint32;

            ASSERT(data_expected_u32 == *data_uint32);

            // Increment the symbol index
            symbol_index += 2;
        }

        // Launch kernels with b64 data type
        symbol_index = 1;
        while (symbol_index < num_symbols) {
            memset(data, 0, sizeof(uint64_t));
            memset(value, 0, sizeof(uint64_t));

            uint64_t* data_uint64 = (uint64_t*)data;
            uint64_t* value_uint64= (uint64_t*)value;
            *data_uint64 = 0;
            *value_uint64= 0xCCCCCCCCCCCCCCCC;

            launch_memory_atomic_kernel(&(agent_list.agents[ii]), queue, &symbol_record[symbol_index],
                   data, value, num_kernel_instances);

            // Verify the kernel is executed correctly
            uint64_t data_expected_u64 = *value_uint64;

            ASSERT(data_expected_u64 == *data_uint64);

            // Increment the symbol index
            symbol_index += 2;
        }


        status = hsa_memory_free(data);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_free(value);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
