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
 * Test Name: aql_launch_size
 * Scope: Conformance
 *
 * Purpose: Verifies that an agent supports all work-item values that are
 * reported in the HSA_AGENT_INFO_WORKGROUP_MAX_DIM and HSA_AGENT_INFO_GRID_MAX_DIM
 * agent attributes. The total number of work-items must be limited to values
 * specified in the HSA_AGENT_INFO_WORKGROUP_MAX_SIZE and HSA_AGENT_INFO_GRID_MAX_SIZE
 * values.
 *
 * Test Description:
 * 1) Obtain the list of all agents that support the HSA_QUEUE_FEATURE_DISPATCH
 * queue feature.
 * 2) For each agent create a queue.
 * 3) Load and initialize the init_data kernel.
 * 4) Allocate a memory region that is large enough to represent a maximal execution
 * of the 3 dimensional init_data kernel, i.e. it should represent a 3 dimensional
 * region that is min(grid_max_dim[0] x grid_max_dim[1] x grid_max_dim[2] | grid_max_size)
 * in size.
 * 5) Execute several kernels, iterating through the possible values of:
 *    a) dimensions - 1 to 3
 *    b) Work group size per dimension x - 1 to wg_max_dim[x] in increments of 1
 *    c) Work group size per dimension y - 0 to wg_max_dim[y] in increments of 1
 *    d) Work group size per dimension z - 0 to wg_max_dim[z] in increments of 1
 *    e) Grid size per dimension x - 1 to grid_max_size[x] in increments of 1
 *    f) Grid size per dimension y - 0 to grid_max_size[y] in increments of 1
 *    g) Grid size per dimension z - 0 to grid_max_size[z] in increments of 1
 *    h) Use a row_pitch equal to the grid_size[x] value
 *    i) Use a slice_pitch equal to the grid_size[x] * grid_size[y] value
 * 6) Verify after each execution that only the specified part of the memory region was
 * modified.
 * 7) Reinitialize the memory region for each execution.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include "test_helper_func.h"

void launch_kernel_grid_size_workgroup_size(
        hsa_queue_t* queue,
        uint32_t* data,
        uint32_t data_size,
        uint32_t total_size,
        uint32_t value,
        int dim,
        hsa_dim3_t grid_dim,
        hsa_dim3_t workgroup_dim,
        uint64_t kernel_obj_address,
        void* kernarg_address) {
    // Launch the kernel
    launch_kernel(queue, data, total_size, value, dim, grid_dim, workgroup_dim, kernel_obj_address, kernarg_address);

    // Verify the data[0 --> data_size -1] has been updated correctly
    int ii;
    for (ii = 0; ii < data_size; ++ii) {
        if (data[ii] != value) {
            ASSERT(0);
        }
    }

    // Verify the rest of the data, data[data_size --> total_size - 1], has not been touched
    for (; ii < total_size; ++ii) {
        if (data[ii] != 0) {
            ASSERT(0);
        }
    }
}

int test_aql_launch_size() {
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

        hsa_dim3_t grid_max_dim;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_GRID_MAX_DIM, &grid_max_dim);
        ASSERT(HSA_STATUS_SUCCESS == status);

        uint32_t   grid_max_size;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_GRID_MAX_SIZE, &grid_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        uint16_t workgroup_max_dim[3];
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_WORKGROUP_MAX_DIM, workgroup_max_dim);
        ASSERT(HSA_STATUS_SUCCESS == status);

        uint32_t   workgroup_max_size;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_WORKGROUP_MAX_SIZE, &workgroup_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Overwrite the grid_max_dim, grid_max_size, workgroup_max_dim, workgroup_max_size
        const uint32_t n = 1024;
        grid_max_dim.x = n;
        grid_max_dim.y = n;
        grid_max_dim.z = 4;
        grid_max_size = n * n * 4;
        workgroup_max_dim[0] = 256;
        workgroup_max_dim[1] = 256;
        workgroup_max_dim[2] = 256;
        workgroup_max_size = 256;

        // Allocate the data buffer
        uint32_t total_size = grid_max_size;
        uint32_t *data;
        status = hsa_memory_allocate(global_region, total_size * sizeof(uint32_t), (void**) &data);
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

        hsa_dim3_t grid_dim;
        hsa_dim3_t workgroup_dim;
        int value = 1;
        int dim;

        uint32_t X_Min;
        uint32_t X_Max;
        uint32_t X_Step;
        uint32_t Y_Min;
        uint32_t Y_Max;
        uint32_t Y_Step;
        uint32_t Z_Min;
        uint32_t Z_Max;
        uint32_t Z_Step;

        // Launch kernels to work on 1D data
        // test every size of grid_dim_x
        X_Min = 1;
        X_Max = 1024;
        X_Step = 1;
        // uint32_t X = grid_max_dim.x;
        dim = 1;
        grid_dim.y = 1;
        grid_dim.z = 1;
        workgroup_dim.y = 1;
        workgroup_dim.z = 1;
        for (grid_dim.x = X_Min; grid_dim.x <= X_Max; grid_dim.x += X_Step) {
            uint32_t data_size = grid_dim.x * grid_dim.y * grid_dim.z;
            workgroup_dim.x = 1;
            while (workgroup_dim.x < workgroup_max_size) {
                launch_kernel_grid_size_workgroup_size(
                        queue,
                        data,
                        data_size,
                        (int)total_size,
                        value, dim,
                        grid_dim, workgroup_dim,
                        symbol_record.kernel_object, kernarg_buffer);
                ++value;
                workgroup_dim.x *= 2;
            }
        }

        // Launch kernels to work on 2D data
        X_Min = 960;
        X_Max = 1024;
        X_Step = 1;
        Y_Min = 1022;
        Y_Max = 1024;
        Y_Step = 1;
        dim = 2;
        grid_dim.z = 1;
        workgroup_dim.z = 1;
        for (grid_dim.y = Y_Min; grid_dim.y <= Y_Max; grid_dim.y += Y_Step) {
            for (grid_dim.x = X_Min; grid_dim.x <= X_Max; grid_dim.x += X_Step) {
                workgroup_dim.x = 1;
                workgroup_dim.y = 1;
                uint32_t data_size = grid_dim.x * grid_dim.y * grid_dim.z;
                while (workgroup_dim.x < workgroup_max_size) {
                    while (workgroup_dim.y < workgroup_max_size) {
                        launch_kernel_grid_size_workgroup_size(
                            queue,
                            data,
                            data_size,
                            (int)total_size,
                            value, dim,
                            grid_dim, workgroup_dim,
                            symbol_record.kernel_object, kernarg_buffer);
                        ++value;
                        workgroup_dim.y *= 4;
                    }
                    workgroup_dim.x *= 4;
                }
            }
        }

        // Launch kernels to work on 3D data
        X_Min = 960;
        X_Max = 1024;
        X_Step = 1;
        Y_Min = 1023;    // 960;
        Y_Max = 1024;
        Y_Step = 1;
        Z_Min = 2;
        Z_Max = 4;
        Z_Step = 1;
        dim = 3;
        for (grid_dim.z = Z_Min; grid_dim.z <= Z_Max; grid_dim.z += Z_Step) {
            for (grid_dim.y = Y_Min; grid_dim.y <= Y_Max; grid_dim.y += Y_Step) {
                for (grid_dim.x = X_Min; grid_dim.x <= X_Max; grid_dim.x += X_Step) {
                    // The workgroup's y and z dimensions are larger than the data,
                    // this should reduce the efficiency of the kernel, but would
                    // would show how the API handles these cases.
                    workgroup_dim.x = 1;
                    workgroup_dim.y = 1;
                    workgroup_dim.z = 1;
                    uint32_t data_size = grid_dim.x * grid_dim.y * grid_dim.z;
                    while (workgroup_dim.x < workgroup_max_size) {
                        while (workgroup_dim.y < workgroup_max_size) {
                            while (workgroup_dim.z < workgroup_max_size) {
                                launch_kernel_grid_size_workgroup_size(
                                    queue,
                                    data,
                                    data_size,
                                    (int)total_size,
                                    value, dim,
                                    grid_dim, workgroup_dim,
                                    symbol_record.kernel_object, kernarg_buffer);
                                    ++value;
                                workgroup_dim.z *= 8;
                            }
                            workgroup_dim.y *= 8;
                        }
                        workgroup_dim.x *= 8;
                    }
                }
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
