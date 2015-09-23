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
 * Test Name: aql_barrier_bit_set
 * Scope: Conformance
 *
 * Purpose: Verifies that if a dispatch packet hsa the barrier bit set, all
 * proceeding packets must complete before processing of this packet occurs.
 *
 * Test Description:
 * 1) Generate a list of agents that support the HSA_QUEUE_FEATURE_DISPATCH
 * queue feature.
 * 2) Load and initialize the signal_st_rlx and the signal_wait_rlx kernels.
 * 3) Select an agent from the list and create a queue.
 * 4) Create a test signal with an initial value of 1.
 * 5) Create several completion signals for dispatches.
 * 6) Enqueue several dispatch packets to the queue that have the barrier bit
 * set to 0, and that all execute the signal_wait_rlx kernel on the created
 * test signal and wait for it to have a value of 1. Each should have its
 * own completion signal.
 * 7) Enqueue a signal dispatch packet that has the barrier bit set to 1,
 * and that executes the signal_st_rlx kernel on the created test signal, setting
 * the value to 0. It should also have its own completion signal.
 * 8) Wait on the completion signal of the dispatch packet with the barrier bit
 * set, with a timeout specified.
 * 9) After the timeout has elapsed, check the other completion signals and
 * verify that the kernels are not yet complete.
 * 10) Set the value of the test signal to 0 on the host.
 * 11) Wait on on the completion signal of the dispatch packet with the barrier bit
 * set, this time without a timeout.
 *
 * Expected results: None of the wait kernels should complete until the host sets
 * the signal value. The dispatch signal_st_rlx kernel should only execute after
 * they have all completed.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <queue_utils.h>

#define NUM_WAIT_KERNELS 8

int test_aql_barrier_bit_set() {
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

        // Check if a queue on this agent support HSA_QUEUE_TYPE_MULTI
        hsa_queue_type_t queue_type;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUE_TYPE, &queue_type);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (HSA_QUEUE_TYPE_MULTI != queue_type) {
            continue;
        }

        // Find the global memory region that is fine grained
        hsa_region_t global_region;
        global_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region_fine_grained, &global_region);
        if ((uint64_t)-1 != global_region.handle) {
            // Skip this agent if a fine grained memory region isn't available.
            continue;
        }

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

        // Create the queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 1024, HSA_QUEUE_TYPE_MULTI, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
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
        symbol_record_t symbol_record[2];
        memset(&symbol_record, 0, sizeof(symbol_record));

        char* symbol_names[2];
        symbol_names[0] = "&__signal_st_rlx_kernel";
        symbol_names[1] = "&__signal_wait_eq_rlx_kernel";
        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 2, symbol_names, symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // The kernarg data structure
        typedef struct __attribute__ ((aligned(16))) signal_args_s {
            uint32_t     count;
            hsa_signal_t* signal_handles;
            hsa_signal_value_t* signal_values;
        } signal_args_t;
        signal_args_t signal_args;

        // Allocate the kernel argument buffer from the correct region
        // Assume the size is the same for both wait and set kernels
        signal_args_t* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region,
                                     symbol_record[1].kernarg_segment_size,
                                     (void**)(&kernarg_buffer));
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the test signal.
        hsa_signal_t test_signal;
        status = hsa_signal_create((hsa_signal_value_t)1, 0, NULL, &test_signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate and initialize space for the wait value
        // parameter
        hsa_signal_value_t *wait_value;
        status = hsa_memory_allocate(global_region, sizeof(hsa_signal_value_t), (void**) &wait_value);
        ASSERT(HSA_STATUS_SUCCESS == status);
        *wait_value = 0;

        // Fill in the kernel argument list
        signal_args.count = 1;
        signal_args.signal_handles = &test_signal;
        signal_args.signal_values = wait_value;
        memcpy(kernarg_buffer, &signal_args, symbol_record[1].kernarg_segment_size);

        // Create the set kernel completion signal
        hsa_signal_t set_kernel_completion_signal;
        status = hsa_signal_create((hsa_signal_value_t)1, 0, NULL, &set_kernel_completion_signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the wait kernel completion signals
        hsa_signal_t wait_kernel_completion_signal[NUM_WAIT_KERNELS];
        int jj;
        for (jj = 0; jj < NUM_WAIT_KERNELS; ++jj) {
            status = hsa_signal_create((hsa_signal_value_t)1, 0, NULL, &wait_kernel_completion_signal[jj]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Setup the dispatch packet
        hsa_kernel_dispatch_packet_t dispatch_packet;
        memset(&dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
        dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        dispatch_packet.header |= 0 << HSA_PACKET_HEADER_BARRIER;
        dispatch_packet.setup = 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
        dispatch_packet.workgroup_size_x = 1;
        dispatch_packet.workgroup_size_y = 1;
        dispatch_packet.workgroup_size_z = 1;
        dispatch_packet.grid_size_x = 1;
        dispatch_packet.grid_size_y = 1;
        dispatch_packet.grid_size_z = 1;
        dispatch_packet.kernel_object = symbol_record[1].kernel_object;
        dispatch_packet.group_segment_size = symbol_record[1].group_segment_size;
        dispatch_packet.private_segment_size = symbol_record[1].private_segment_size;
        dispatch_packet.kernarg_address = (void*) kernarg_buffer;

        // Dispatch the wait kernels
        for (jj = 0; jj < NUM_WAIT_KERNELS; ++jj) {
            // Set the appropriate completion signal
            dispatch_packet.completion_signal = wait_kernel_completion_signal[jj];
            // Dispatch the kernel
            enqueue_dispatch_packet(queue, &dispatch_packet);
        }

        // Dispatch the set kernel, setting the barrier bit to 1
        dispatch_packet.header |= 1 == HSA_PACKET_HEADER_BARRIER;

        // Set the appropriate completion signal and code descriptor values
        dispatch_packet.kernel_object = symbol_record[0].kernel_object;
        dispatch_packet.group_segment_size = symbol_record[0].group_segment_size;
        dispatch_packet.private_segment_size = symbol_record[0].private_segment_size;

        // Dispatch the set kernel
        enqueue_dispatch_packet(queue, &dispatch_packet);

        // Query the systems timestamp frequency for wait timeout
        uint16_t freq;
        status = hsa_system_get_info(HSA_SYSTEM_INFO_TIMESTAMP_FREQUENCY, (void*) &freq);

        // Wait on the completion signal of the set kernel, but
        // timeout after 1 second
        uint64_t wait_time = (uint64_t) freq;
        hsa_signal_value_t signal_value;
        signal_value = hsa_signal_wait_relaxed(set_kernel_completion_signal, HSA_SIGNAL_CONDITION_EQ, 0, wait_time, HSA_WAIT_STATE_ACTIVE);
        ASSERT(1 == signal_value);

        // Wait on the completion signals of each of the wait kernels, again timing out after 1 second
        for (jj = 0; jj < NUM_WAIT_KERNELS; ++jj) {
            signal_value = hsa_signal_wait_relaxed(wait_kernel_completion_signal[jj], HSA_SIGNAL_CONDITION_EQ, 0, wait_time, HSA_WAIT_STATE_ACTIVE);
            ASSERT(1 != signal_value);
        }

        // Set the test_signal value to 0 from the host
        hsa_signal_store_relaxed(test_signal, 0);

        // Wait on the completion signal of the set kernel again, but
        // no timeout should be specified
        while (hsa_signal_wait_relaxed(set_kernel_completion_signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_ACTIVE) != 0) {}

        // Destroy the set kernel completion signal
        status = hsa_signal_destroy(set_kernel_completion_signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Wait on the values of the wait kernel signals
        // Destroy the signal at that time.
        for (jj = 0; jj < NUM_WAIT_KERNELS; ++jj) {
            while (hsa_signal_wait_relaxed(wait_kernel_completion_signal[jj], HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_ACTIVE) != 0) {}
            status = hsa_signal_destroy(wait_kernel_completion_signal[jj]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // Destroy the test completion signal
        status = hsa_signal_destroy(test_signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        status = hsa_memory_free(wait_value);
        ASSERT(HSA_STATUS_SUCCESS == status);

        status = hsa_memory_free(kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

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

    // Destroy the loaded module
    destroy_module(module);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
