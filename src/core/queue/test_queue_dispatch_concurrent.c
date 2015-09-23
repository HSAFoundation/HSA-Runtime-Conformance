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
 * Test Name: queue_dispatch_concurrent
 * Scope: Conformance
 *
 * Purpose: Verifies that a queue can have AQL packets written
 * to it concurrently, and that the AQL packets are properly
 * executed.
 *
 * Test Description:
 * 1) Query the list of agents for all agents that support the
 * HSA_QUEUE_FEATURE_DISPATCH queue feature.
 * 2) For each agent create a queue associated with the agent with a specified
 * size less than HSA_AGENT_INFO_QUEUE_MAX_SIZE attribute.
 * 3) Create several different threads that concurrently operate on the queue,
 * performing the following operations:
 *    a) Create a signal for use in dispatches.
 *    b) Allocate a small memory location for use with the data_init
 *       kernel.
 *    c) Load and initialize the data_init kernel.
 *    d) Call hsa_queue_add_write_index_acquire to obtain a valid write
 *       index in the queue.
 *    e) Calls hsa_queue_load_read_index_relaxed in a loop until the write
 *       index is less than the sum of the read index and the queue size.
 *    f) Populates the packet at the write index with a dispatch packet
 *       that launches the init_data kernel. The packet and the kernel parameters
 *       should be configured such that a small, one dimensional memory location
 *       should be initialized with a unique value associated with the thread.
 *    g) The thread should wait on the signal for the dispatch to finish.
 *    h) The thread should verify that the memory location was properly initialized.
 *    i) Repeat steps d through i several times, terminating only when the write
 *       index is equal to a set multiple of the specified queue size.
 * 4) Repeat this test for each agent/queue.
 *
 * Expected Results: All dispatches should succeed and the data should be initialized
 * correctly.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <queue_utils.h>
#include <stdlib.h>

#define ARGUMENT_ALIGN_BYTES 16
#define CON_QUEUE_DISP_NUM_THREADS 32
#define CON_QUEUE_DISP_DEFAULT_QUEUE_SIZE 256
#define CON_QUEUE_DISP_TERMINATION_MULTIPLES 2

typedef struct queue_dispatch_params {
    hsa_agent_t* agent;
    hsa_queue_t* queue;
    symbol_record_t* symbol_record;
} queue_dispatch_params_t;

// Work function for concurrent queue dispatch
void thread_proc_dispatch(void* data) {
    hsa_status_t status;

    queue_dispatch_params_t* param = (queue_dispatch_params_t*) data;
    int num_dispatch_packets = param->queue->size * CON_QUEUE_DISP_TERMINATION_MULTIPLES;

    // Allocate a memory block used by the kernel.
    hsa_region_t global_region;
    global_region.handle=(uint64_t)-1;
    hsa_agent_iterate_regions(*(param->agent), get_global_memory_region_fine_grained, &global_region);
    ASSERT((uint64_t)-1 != global_region.handle);

    const int block_size = 1024;
    uint32_t* data_block;
    status = hsa_memory_allocate(global_region, block_size * sizeof(uint32_t), (void**) &data_block);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Declare the kernarg data structure
    struct __attribute__ ((aligned(ARGUMENT_ALIGN_BYTES))) con_queue_dispatch_arg_t {
        void* data;
        uint32_t value;
        uint32_t row_pitch;
        uint32_t slice_pitch;
    } args;

    // Find a memory region that supports kernel arguments
    hsa_region_t kernarg_region;
    kernarg_region.handle = (uint64_t)-1;
    hsa_agent_iterate_regions(*(param->agent), get_kernarg_memory_region, &kernarg_region);
    ASSERT((uint64_t)-1 != kernarg_region.handle);

    // Allocate the kernel argument buffer from the correct region
    void* kernarg_buffer = NULL;
    status = hsa_memory_allocate(kernarg_region, sizeof(args), &kernarg_buffer);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create a signal for dispatch
    hsa_signal_t signal;
    status = hsa_signal_create(1, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Fill in info for the default dispatch packet
    hsa_kernel_dispatch_packet_t dispatch_packet;
    memset(&dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
    dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
    dispatch_packet.header |= 1 << HSA_PACKET_HEADER_BARRIER;
    dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
    dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
    dispatch_packet.setup |= 1 << HSA_PACKET_HEADER_BARRIER;
    dispatch_packet.setup  |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
    dispatch_packet.workgroup_size_x = 256;
    dispatch_packet.workgroup_size_y = 1;
    dispatch_packet.workgroup_size_z = 1;
    dispatch_packet.grid_size_x = 256;
    dispatch_packet.grid_size_x = 1;
    dispatch_packet.grid_size_x = 1;
    dispatch_packet.group_segment_size = param->symbol_record->group_segment_size;
    dispatch_packet.private_segment_size = param->symbol_record->private_segment_size;
    dispatch_packet.kernel_object = param->symbol_record->kernel_object;
    dispatch_packet.kernarg_address = 0;
    dispatch_packet.completion_signal = signal;

    int ii;
    for (ii = 0; ii < num_dispatch_packets; ++ii) {
        // Reinitialize the signal's value
        hsa_signal_store_relaxed(signal, 1);

        // Setup the kernarg arguments
        args.data = data_block;
        args.value = (uint32_t)ii;
        args.row_pitch = (uint32_t)0;
        args.slice_pitch = (uint32_t)0;
        memcpy(kernarg_buffer, &args, sizeof(args));

        // Initialize the packet with specific parameters.
        dispatch_packet.kernarg_address = (void*) kernarg_buffer;

        // Dispatch the kernel
        enqueue_dispatch_packet(param->queue, &dispatch_packet);

        // Wait until the kernel complete
        hsa_signal_wait_relaxed(signal, HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

        // Verify the kernel executed correctly
        int valid = 1;
        int failIndex = 0;
        int jj;
        for (jj = 0; jj < block_size; ++jj) {
            if (data_block[jj] != (uint32_t)ii) {
                failIndex = jj;
                valid = 0;
            }
        }
    }

    status = hsa_signal_destroy(signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_memory_free(kernarg_buffer);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_memory_free(data_block);
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}

int test_queue_dispatch_concurrent() {
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
        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Check if a queue on this agent support QUEUE_TYPE_MULTI
        hsa_queue_type_t queue_type;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUE_TYPE, &queue_type);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (HSA_QUEUE_TYPE_MULTI != queue_type) {
            continue;
        }

        // Get the maximum number of queues that is supported on this agent
        uint32_t queue_max;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUES_MAX, &queue_max);
        ASSERT(HSA_STATUS_SUCCESS == status);

        if (queue_max < 1) {
            // This agent does not support any queue, skip it
            continue;
        }

        // Adjust the queue size
        uint32_t queue_size = CON_QUEUE_DISP_DEFAULT_QUEUE_SIZE;
        uint32_t queue_max_size;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUE_MAX_SIZE, &queue_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        while (queue_size > queue_max_size) {
            queue_size /= 2;
        }

        // Create a queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], queue_size, HSA_QUEUE_TYPE_MULTI, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
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

        // The thread parameters
        queue_dispatch_params_t params;
        params.agent = agent_list.agents + ii;
        params.queue = queue;
        params.symbol_record = &symbol_record;

        // Create the test group
        struct test_group* tg_concurrent_queue_dispatch = test_group_create(CON_QUEUE_DISP_NUM_THREADS);
        test_group_add(tg_concurrent_queue_dispatch, &thread_proc_dispatch, &params, CON_QUEUE_DISP_NUM_THREADS);
        test_group_thread_create(tg_concurrent_queue_dispatch);
        test_group_start(tg_concurrent_queue_dispatch);
        test_group_wait(tg_concurrent_queue_dispatch);
        test_group_exit(tg_concurrent_queue_dispatch);
        test_group_destroy(tg_concurrent_queue_dispatch);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy queue
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
