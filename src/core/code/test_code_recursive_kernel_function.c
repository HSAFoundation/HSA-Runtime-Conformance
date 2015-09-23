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

#include <stdlib.h>
#include <stdio.h>
#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <queue_utils.h>
#include "test_helper_func.h"

#define BLOCK_SIZE 1024
#define RECURSE_COUNT 16

int test_code_recursive_kernel_function() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Find an agent that supports kernel dispatch
    hsa_agent_t agent;
    status = hsa_iterate_agents(get_kernel_dispatch_agent, &agent);
    if(status == HSA_STATUS_INFO_BREAK) { status = HSA_STATUS_SUCCESS; }
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create a queue for dispatch
    hsa_queue_t* queue;
    status = hsa_queue_create(agent, 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Find a memory region in the global segment
    hsa_region_t global_region;
    global_region.handle=(uint64_t)-1;
    hsa_agent_iterate_regions(agent, get_global_memory_region_fine_grained, &global_region);
    ASSERT((uint64_t)-1 != global_region.handle);

    // Find a memory region that supports kernel arguments
    hsa_region_t kernarg_region;
    kernarg_region.handle = (uint64_t)-1;
    hsa_agent_iterate_regions(agent, get_kernarg_memory_region, &kernarg_region);
    ASSERT((uint64_t)-1 != kernarg_region.handle);

    // Load the BRIG module
    hsa_ext_module_t module;
    load_module_from_file("recursive_func.brig", &module);

    // Finalize the executable
    hsa_code_object_t code_object;
    hsa_executable_t executable;

    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));

    status = finalize_executable(agent,
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

    // Find the executable symbols for dispatch
    char* symbol_name = "&__recursive_func_kernel";
    symbol_record_t symbol_record;
    status = get_executable_symbols(executable,
                                    agent,
                                    HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
                                    1,
                                    &symbol_name,
                                    &symbol_record);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Define the argument structure
    struct args_t {
        uint32_t* out;
        uint32_t  in0;
        uint32_t  in1;
    } args;

    // Allocate and initialize the kernel arguments and data.
    args.in0 = 0;
    args.in1 = RECURSE_COUNT;

    status = hsa_memory_allocate(global_region, BLOCK_SIZE * sizeof(uint32_t), (void**) &(args.out));
    ASSERT(HSA_STATUS_SUCCESS == status);
    memset(args.out, 0, BLOCK_SIZE * sizeof(uint32_t));

    // Allocate the kernel argument buffer from the correct region.
    char* kernarg_buffer;
    status = hsa_memory_allocate(kernarg_region, symbol_record.kernarg_segment_size, (void**) &kernarg_buffer);
    ASSERT(HSA_STATUS_SUCCESS == status);
    memcpy(kernarg_buffer, &args, symbol_record.kernarg_segment_size);

    // Create a completion signal
    hsa_signal_t completion_signal;
    status=hsa_signal_create(1, 0, NULL, &completion_signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Setup the dispatch packet.
    hsa_kernel_dispatch_packet_t dispatch_packet;
    memset(&dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
    dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
    dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
    dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
    dispatch_packet.header |= 1 << HSA_PACKET_HEADER_BARRIER;
    dispatch_packet.setup |= 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
    dispatch_packet.workgroup_size_x = 256;
    dispatch_packet.workgroup_size_y = 1;
    dispatch_packet.workgroup_size_z = 1;
    dispatch_packet.grid_size_x = BLOCK_SIZE;
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

    // Validate the data in the output buffer
    int ii;
    for(ii=0;ii<BLOCK_SIZE;++ii) {
        ASSERT(RECURSE_COUNT == args.out[ii]);
    }

    // Cleanup all allocated resources
    status = hsa_memory_free(args.out);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_memory_free(kernarg_buffer);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Destroy the signal 
    status=hsa_signal_destroy(completion_signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Destroy the code and executable objects
    status=hsa_executable_destroy(executable);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status=hsa_code_object_destroy(code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Destroy the queue
    status=hsa_queue_destroy(queue);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Shut down the hsa runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
