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
 *
 * Test Name: signal_kernel_set
 * Scope: Conformance
 *
 * Purpose: Verifies that a signal handle can be passed as a kernel argument
 * and used during kernel execution.
 *
 * Test Description:
 * 1) Create a signal.
 * 2) Launch a kernel with a single workitem, passing the signal as
 * a kernel argument. The kernel should be launched with HSA_FENCE_SCOPE_SYSTEM
 * for both the acquire and release scopes.
 * 3) Use an HSAIL instruction to modify the value of the signal.
 * 4) After the kernel finishes executing, check the value using a
 * hsa_signal_ld API.
 *
 * Expected Results: After the kernel finishes executing, the value of the
 * signal should be modified to the correct value.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <queue_utils.h>

int test_signal_kernel_set() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("signal_operations.brig", &module));

    // Get a list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Repeat the test for each agent
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Find a memory region that supports fine grained memory
        hsa_region_t global_region;
        global_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region_fine_grained, &global_region);
        if ((uint64_t)-1 != global_region.handle) {
            continue;
        }

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

        // Create a queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
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
        symbol_names[0] = "&__signal_st_rlx_kernel";
        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, symbol_names, &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // The kernarg data structure
        typedef struct __attribute__ ((aligned(16))) signal_args_s {
            uint32_t     count;
            hsa_signal_t* signal_handles;
            hsa_signal_value_t* signal_values;
        } signal_args_t;
        signal_args_t signal_args;

        // Allocate the kernel argument buffer from the correct region
        signal_args_t* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region,
                                     symbol_record.kernarg_segment_size,
                                     (void**)(&kernarg_buffer));
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the completion signal
        hsa_signal_t completion_signal;
        status = hsa_signal_create(1, 0, NULL, &completion_signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the kernel signal
        hsa_signal_t kernel_signal;
        status = hsa_signal_create(1, 0, NULL, &kernel_signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate and initialize the set value
        hsa_signal_value_t *set_value;
        status = hsa_memory_allocate(global_region, sizeof(hsa_signal_value_t), (void**) &set_value);
        ASSERT(HSA_STATUS_SUCCESS == status);
        *set_value = 0;

        // Fill in the kernel argument list
        signal_args.count = 1;
        signal_args.signal_handles = &kernel_signal;
        signal_args.signal_values = set_value;
        memcpy(kernarg_buffer, &signal_args, symbol_record.kernarg_segment_size);

        // Setup the dispatch packet
        hsa_kernel_dispatch_packet_t dispatch_packet;
        memset(&dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
        dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        dispatch_packet.header |= 1 << HSA_PACKET_HEADER_BARRIER;
        dispatch_packet.setup |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
        dispatch_packet.workgroup_size_x = 1;
        dispatch_packet.workgroup_size_y = 1;
        dispatch_packet.workgroup_size_z = 1;
        dispatch_packet.grid_size_x = 1;
        dispatch_packet.grid_size_y = 1;
        dispatch_packet.grid_size_z = 1;
        dispatch_packet.kernel_object = symbol_record.kernel_object;
        dispatch_packet.group_segment_size = symbol_record.group_segment_size;
        dispatch_packet.private_segment_size = symbol_record.private_segment_size;
        dispatch_packet.kernarg_address = (void*) kernarg_buffer;
        dispatch_packet.completion_signal = completion_signal;

        // Dispatch the kernel
        enqueue_dispatch_packet(queue, &dispatch_packet);

        // Wait on the completion signal
        hsa_signal_wait_relaxed(completion_signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

        // Check kernel signal
        *set_value = hsa_signal_load_relaxed(kernel_signal);
        ASSERT(0 == *set_value);

        status = hsa_signal_destroy(kernel_signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        status = hsa_signal_destroy(completion_signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        status = hsa_memory_free(set_value);
        ASSERT(HSA_STATUS_SUCCESS == status);

        status = hsa_memory_free(kernarg_buffer);
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

    // Shutdown runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
