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
 * Test Name: queue_multi_gap
 * Scope: Conformance
 *
 * Purpose: Verifies a queue can contain regions where packets have type
 * HSA_PACKET_TYPE_ALWAYS_RESERVED between regions where there are valid
 * packet types, i.e. HSA_PACKET_TYPE_DISPATCH, and that the packet processor
 * will block until the packet type has changed.
 *
 * Test Description:
 * 1) Obtain the list of all agents that support the HSA_QUEUE_FEATURE_DISPATCH
 *    queue feature.
 * 2) For a valid agent create a queue of type HSA_QUEUE_TYPE_MULTI.
 * 3) Load and initialize the init_data kernel.
 * 4) Allocate small memory locations that can be used by the execution of the
 *    init_data kernel. There should be enough for each packet slot.
 * 5) Create three signals.
 * 6) Reserve three packet regions in the queue of equal size.
 * 7) Populate the first and last packet regions with valid dispatch packets.
 * 8) Use the first signal as the completion signal in the last packet of the first region.
 * 9) Use the third signal as the completion signal in the last packet of the third region.
 * 10) Ring the doorbell twice, once with the first region's last write index and once
 *     with the third regions last write index.
 * 11) Wait on the first signal to trigger.
 * 12) Validate the memory regions initialized by the execution of the packets
 *     in the first region.
 * 12) Check the value of the third signal and ensure it has not triggered.
 * 13) Populate the middle region with valid dispatch packets.
 * 14) Use the second signal as the completion signal in the last packet of the middle region.
 * 15) Ring the doorbell using the last write index of the middle region as the value.
 * 16) Wait on the second signal to trigger.
 * 17) Wait on the third signal to trigger.
 * 12) Validate that the memory locations associated with the packets in the second and
 *     third regions have been successfully updated.
 * 13) Repeat 6 through 12 several times.
 * 14) Repeat 2 through 13 for each agent.
 *
 * Expected Results: All dispatches should finish and all of the data should be initialized correctly.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <queue_utils.h>
#include <stdlib.h>

#define ARGUMENT_ALIGN_BYTES     16
#define MULTI_GAP_QUEUE_SIZE    128
#define MULTI_GAP_REGION_SIZE    32
#define MULTI_GAP_TEST_REPEAT    4
#define MULTI_GAP_KERNEL_DATA_SIZE 1024

// Check the memory block of each individual region
void verify_data(uint32_t** data_blocks, int initialized, int region_index) {
    int ii;
    int fail = 0;
    for (ii = 0; ii < MULTI_GAP_REGION_SIZE; ++ii) {
        uint32_t expected_value;
        if (initialized) {
            // The data block has been initialized by the kernels
            expected_value = ii + 1;
        } else {
            // The data block has not been initialized
            expected_value = 0;
        }
        int jj;
        for (jj = 0; jj < MULTI_GAP_KERNEL_DATA_SIZE; ++jj) {
            if (data_blocks[ii][jj] != expected_value) {
                fail = 1;
                break;
            }
        }
        if (fail) {
            break;
        }
    }
}

int test_queue_multi_gap() {
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
        // check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Find a memory region in the global segment that supports fine grained memory
        hsa_region_t global_region;
        global_region.handle=(uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region_fine_grained, &global_region);
        if ((uint64_t)-1 == global_region.handle) {
            // Skip this agent if it doesn't support fine grained memory
            continue;
        }

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t)-1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t)-1 != kernarg_region.handle);

        // Get the maximum number of queues that is supported on this agent
        uint32_t queues_max;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUES_MAX, &queues_max);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (queues_max < 1) {
            continue;
        }

        // Create the queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], MULTI_GAP_QUEUE_SIZE, HSA_QUEUE_TYPE_MULTI, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
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

        // Allocate memory blocks used by the kernel arg
        int jj;
        uint32_t** data_blocks = (uint32_t**) malloc(3 * MULTI_GAP_REGION_SIZE * sizeof(uint32_t*));
        for (jj = 0; jj < 3*MULTI_GAP_REGION_SIZE; ++jj) {
            status = hsa_memory_allocate(global_region, MULTI_GAP_KERNEL_DATA_SIZE * sizeof(uint32_t), (void**) &data_blocks[jj]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        // The kernarg data structure
        typedef struct __attribute__ ((aligned(ARGUMENT_ALIGN_BYTES))) queue_multi_gap_arg {
            void* data;
            uint32_t value;
            uint32_t row_pitch;
            uint32_t slice_pitch;
        } queue_multi_gap_arg_t;
        queue_multi_gap_arg_t args;

        // Allocate the kernel argument buffer from the correct region
        queue_multi_gap_arg_t* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region, 3 * MULTI_GAP_REGION_SIZE * sizeof(args), (void**)(&kernarg_buffer));
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Setup the kernel arg data structure
        for (jj = 0; jj < 3*MULTI_GAP_REGION_SIZE; ++jj) {
            args.data = data_blocks[jj];
            args.value = jj % MULTI_GAP_REGION_SIZE + 1;
            args.row_pitch = 0;
            args.slice_pitch = 0;
            memcpy(kernarg_buffer + jj, &args, sizeof(queue_multi_gap_arg_t));
        }

        // Setup the dispatch packet.
        hsa_kernel_dispatch_packet_t dispatch_packet;
        size_t packet_size = sizeof(hsa_kernel_dispatch_packet_t);
        memset(&dispatch_packet, 0, packet_size);
        dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        dispatch_packet.header |= 1 << HSA_PACKET_HEADER_BARRIER;
        dispatch_packet.setup  |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
        dispatch_packet.workgroup_size_x = MULTI_GAP_KERNEL_DATA_SIZE;
        dispatch_packet.workgroup_size_y = 1;
        dispatch_packet.workgroup_size_z = 1;
        dispatch_packet.grid_size_x = MULTI_GAP_KERNEL_DATA_SIZE;
        dispatch_packet.grid_size_y = 1;
        dispatch_packet.grid_size_z = 1;
        dispatch_packet.group_segment_size = symbol_record.group_segment_size;
        dispatch_packet.private_segment_size = symbol_record.private_segment_size;
        dispatch_packet.kernel_object = symbol_record.kernel_object;
        dispatch_packet.kernarg_address = 0;
        dispatch_packet.completion_signal.handle = 0;

        // Repeat the tests on the same queue
        for (jj = 0; jj < MULTI_GAP_TEST_REPEAT; ++jj) {
                // Clear the memory blocks
                int kk;
                for (kk = 0; kk < 3*MULTI_GAP_REGION_SIZE; ++kk) {
                    memset(data_blocks[kk], 0, MULTI_GAP_KERNEL_DATA_SIZE * sizeof(uint32_t));
                }

                // Reserve write index's for each region.
                uint64_t write_index[3];
                write_index[0] = hsa_queue_add_write_index_acquire(queue, MULTI_GAP_REGION_SIZE);
                write_index[1] = hsa_queue_add_write_index_acquire(queue, MULTI_GAP_REGION_SIZE);
                write_index[2] = hsa_queue_add_write_index_acquire(queue, MULTI_GAP_REGION_SIZE);

                // Create a completion signal for each region
                hsa_signal_t signals[3];
                status = hsa_signal_create(MULTI_GAP_REGION_SIZE, 0, NULL, &signals[0]);
                ASSERT(HSA_STATUS_SUCCESS == status);
                status = hsa_signal_create(MULTI_GAP_REGION_SIZE, 0, NULL, &signals[1]);
                ASSERT(HSA_STATUS_SUCCESS == status);
                status = hsa_signal_create(MULTI_GAP_REGION_SIZE, 0, NULL, &signals[2]);
                ASSERT(HSA_STATUS_SUCCESS == status);

                // Populate the first region with packets, and ring the doorbell
                dispatch_packet.completion_signal = signals[0];
                for (kk = 0; kk < MULTI_GAP_REGION_SIZE; ++kk) {
                    dispatch_packet.kernarg_address = (void*)(kernarg_buffer + kk);
                    enqueue_dispatch_packet_at((uint64_t) (write_index[0] + kk), queue, &dispatch_packet);
                }

                // Populate the third region with packets, and ring the doorbell
                dispatch_packet.completion_signal = signals[2];
                for (kk = 0; kk < MULTI_GAP_REGION_SIZE; ++kk) {
                    dispatch_packet.kernarg_address = (void*)(kernarg_buffer + 2 * MULTI_GAP_REGION_SIZE + kk);
                    enqueue_dispatch_packet_at((uint64_t) (write_index[2] + kk), queue, &dispatch_packet);
                }

                // Wait for the first signal to indicate all kernels have finished
                hsa_signal_wait_relaxed(signals[0], HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

                // Verify the second signal hasn't been modified
                hsa_signal_value_t sig_value = hsa_signal_load_acquire(signals[1]);
                ASSERT(MULTI_GAP_REGION_SIZE == sig_value);

                // Verify the third signal hasn't been modified
                sig_value = hsa_signal_load_acquire(signals[2]);
                ASSERT(MULTI_GAP_REGION_SIZE == sig_value);

                // Verify the data has been updated in the 1st region
                verify_data(data_blocks, 1, 0);

                // Populate the second region with packets and ring the doorbell
                dispatch_packet.completion_signal = signals[1];
                for (kk = 0; kk < MULTI_GAP_REGION_SIZE; ++kk) {
                    dispatch_packet.kernarg_address = (void*)(kernarg_buffer + MULTI_GAP_REGION_SIZE + kk);
                    enqueue_dispatch_packet_at((uint64_t) (write_index[1] + kk), queue, &dispatch_packet);
                }

                // Wait for 2nd signal to indicate all kernels have finished
                hsa_signal_wait_relaxed(signals[1], HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

                // Wait for 3rd signal to indicate all kernels have finished
                hsa_signal_wait_relaxed(signals[2], HSA_SIGNAL_CONDITION_EQ, 0, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

                // Verify the data has been updated in the 2nd region
                verify_data(data_blocks + MULTI_GAP_REGION_SIZE, 1, 1);

                // Verify the data has been updated in the 3rd region
                verify_data(data_blocks + 2 * MULTI_GAP_REGION_SIZE, 1, 2);

                // Destroy the signals
                status = hsa_signal_destroy(signals[0]);
                ASSERT(HSA_STATUS_SUCCESS == status);
                status = hsa_signal_destroy(signals[1]);
                ASSERT(HSA_STATUS_SUCCESS == status);
                status = hsa_signal_destroy(signals[2]);
                ASSERT(HSA_STATUS_SUCCESS == status);
        }

        status = hsa_memory_free(kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free the memory addresses used by kernel arg
        for (jj = 0; jj < 3*MULTI_GAP_REGION_SIZE; ++jj) {
            status = hsa_memory_free(data_blocks[jj]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        free(data_blocks);

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
