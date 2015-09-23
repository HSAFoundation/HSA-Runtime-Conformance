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
 * Test Name: memory_device
 * Purpose: Verifies that a kernel can access device memory successfully
 *
 * Test Description:
 * 1. Initialize hsa runtime by calling hsa_init();
 * 2. Iterate all of the agents in the system and find a GPU device;
 * 3. Iterate all of the regions for this GPU to find a device memory region;
 * 4. Allocate a memory block in this region, and launch a kernel using this block as its input;
 * 5. In the kernel, try to write something to this memory block.
 *
 * Expected Results: The kernel should be able to access the device memory without error.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "test_helper_func.h"

int test_memory_device() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("init_data.brig", &module));

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Repeat the test for each agent
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Check if the agent is a GPU agent
        hsa_device_type_t device_type;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_DEVICE, &device_type);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (HSA_DEVICE_TYPE_GPU != device_type) {
            // continue if this agent is not a GPU agent
            continue;
        }

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

        // Create the queue
        const uint32_t queue_size = 256;
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], queue_size, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
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

        // Get the symbol and the symbol info
        symbol_record_t symbol_record;
        memset(&symbol_record, 0, sizeof(symbol_record_t));

        char* symbol_names[1];
        symbol_names[0] = "&__init_int_data_kernel";
        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate the kernel argument buffer from the correct region
        void* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region, sizeof(kernarg_init_data_t), &kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Get a list of all regions available to this agent
        struct region_list_s region_list;
        get_region_list(agent_list.agents[ii], &region_list);

        // For each of the region, get region info
        int jj;
        for (jj = 0; jj < region_list.num_regions; ++jj) {
            // Get the region info
            region_info_t info;
            get_region_info(region_list.regions[jj], &info);

            // Verify this is the device memory:
            // size > 0, alloc_max_size > 0
            if (info.size > 0 && info.alloc_max_size > 0) {
                // Allocate on this region
                const size_t data_size = 1024;
                int* data;
                status = hsa_memory_allocate(region_list.regions[jj], data_size, (void*) &data);
                ASSERT(HSA_STATUS_SUCCESS == status);

                hsa_dim3_t grid_dim;
                hsa_dim3_t workgroup_dim;

                int dim;
                dim = 2;
                // Consistent with data_size
                workgroup_dim.x = 256;
                workgroup_dim.y = 4;
                workgroup_dim.z = 1;
                grid_dim.x = workgroup_dim.x;
                grid_dim.y = workgroup_dim.y;
                grid_dim.z = workgroup_dim.z;
                const int value = 0xcccccccc;
                launch_init_data_kernel(queue,
                        data, data_size, value, dim,
                        grid_dim, workgroup_dim,
                        (uint64_t)(symbol_record.kernel_object),
                        (void*)kernarg_buffer);
                // Verify the kernel was executed correctly
                int kk;
                for (kk = 0; kk < data_size; ++kk) {
                    if (data[kk] != value) {
                        ASSERT(0);
                    }
                }

                status = hsa_memory_free(data);
                ASSERT(HSA_STATUS_SUCCESS == status);
            }
        }

        // Free the region list
        if (region_list.num_regions > 0) {
            free(region_list.regions);
        }

        // Free the kernarg_buffer that was allocated on kernarg_region
        status = hsa_memory_free(kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy queues
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Destroy the loaded module
    destroy_module(module);

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
