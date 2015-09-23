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
 * Test Name: aql_zero_wg_size
 * Scope: Conformance
 *
 * Purpose: Verifies that an aql dispatch that specifies a work group size
 * of 0 in a valid work group dimension will succeed in executing, but will
 * do no meaningful work.
 *
 * Test Description:
 * 1) Obtain the list of all agents that support the HSA_QUEUE_FEATURE_DISPATCH
 *    queue feature.
 * 2) For each agent create a queue.
 * 3) Load and initialize the init_data kernel.
 * 4) Allocate a memory region that is large enough to represent a 3 dimensional
 *    init_data kernel of moderate size, possibly 256x1x1.
 * 5) Execute several kernels, iterating through the possible values of:
 *    a) dimensions - 1 to 3
 *    b) Work group size per dimension x - 0 if dimensions is 1, 1 otherwise.
 *    c) Work group size per dimension y - 0 if dimensions is 2, 1 otherwise.
 *    d) Work group size per dimension z - 0 if dimensions is 3, 1 otherwise.
 *    e) Use a row_pitch equal to the grid_size[x]
 *    f) Use a slice_pitch equal to the grid_size[x] * grid_size[y] value
 * 6) Verify after each execution that only the the kernel executed (the
 *    completion signal decrements) but that no part of the memory region was modified.
 *
 * Expected Result: The executions should complete, but the memory should not
 * be modified.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include "test_helper_func.h"

#define MIN_GRID_SIZE 256

int test_aql_zero_wg_size() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("init_data.brig", &module));

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            // Continue if this agent does not support DISPATCH
            continue;
        }

        // Find a memory region in the global segment
        hsa_region_t global_region;
        global_region.handle=(uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region_fine_grained, &global_region);
        if ((uint64_t)-1 == global_region.handle) {
            // Skip the test if global fine grained memory isn't available
            continue;
        }

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

        uint32_t   grid_max_size;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_GRID_MAX_SIZE, &grid_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(grid_max_size > MIN_GRID_SIZE);

        // Allocate the data buffer
        uint32_t *data;
        status = hsa_memory_allocate(global_region, MIN_GRID_SIZE * sizeof(uint32_t), (void**) &data);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the queues
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Finalize the executable
        hsa_code_object_t code_object;
        hsa_executable_t executable;

        hsa_ext_control_directives_t control_directives;
        memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));

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
        status = hsa_memory_allocate(kernarg_region, sizeof(kernarg_t), &kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        int dim;
        hsa_dim3_t grid_dim;
        hsa_dim3_t workgroup_dim;
        for (dim = 1; dim <= 3; ++dim) {
            workgroup_dim.x = (dim == 1) ? 0: 1;
            workgroup_dim.y = (dim == 2) ? 0: 1;
            workgroup_dim.z = (dim == 3) ? 0: 1;

            grid_dim.x = (dim == 1) ? 0: MIN_GRID_SIZE;
            grid_dim.y = (dim == 2) ? 0: MIN_GRID_SIZE;
            grid_dim.z = (dim == 3) ? 0: MIN_GRID_SIZE;

            launch_kernel(queue, data, MIN_GRID_SIZE, 1, dim, grid_dim, workgroup_dim, symbol_record.kernel_object, kernarg_buffer);

            // Verify that no data was modified
            int jj;
            for (jj = 0; jj < MIN_GRID_SIZE; ++jj) {
                ASSERT(0 == data[jj]);
            }
        }

        // Free the kernarg_buffer that was allocated on kernarg_region
        status = hsa_memory_free(kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy queues
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free the data buffers
        status = hsa_memory_free(data);
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
