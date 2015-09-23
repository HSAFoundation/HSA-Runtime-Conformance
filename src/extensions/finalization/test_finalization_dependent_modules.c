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
 * Test Name: finalization_dependent_modules
 * Scope: Extension (Finalization)
 * Support: This test assumes that the system supports the finalization
 * extension and that a viable agent that supports that extension
 * can be found.
 *
 * Purpose: Verify that a program that has two or more dependent
 * modules can be finalized and the associated code can be used
 * in dispatch.
 *
 * Test Description:
 * 1) Create a hsa_ext_program_t object.
 * 2) Load a module with a defined kernel that depends on functions
 *    and variables defined and declared in a second module.
 * 3) Add the module to the program and attempt to finalize the program.
 *    The finalization attempt should fail with a status of
 *    HSA_EXT_STATUS_ERROR_INVALID_PROGRAM.
 * 4) Add the second module module an attempt to finalize again. It should
 *    succeed.
 * 5) Extract symbols (kernels) associated with each module from the finalized
 *    program.
 * 6) Dispatch each of the kernels on a valid agent.
 *
 * Expected Results: On the second attempt program should be properly
 * finalized and all kernels should execute successfully.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <queue_utils.h>

#define BLOCK_SIZE 1024

int test_finalization_dependent_modules() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Find an agent that supports kernel dispatch
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

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

    // Load the modules
    const int num_modules = 2;
    char* module_names[] = {
        "depend_module1.brig",
        "depend_module2.brig"};
    hsa_ext_module_t modules[num_modules];

    // Load the 1st brig module from a file
    int load_status = load_module_from_file(module_names[0], &modules[0]);
    ASSERT(HSA_STATUS_SUCCESS == load_status);

    // Load the 2nd brig module from a file
    load_status = load_module_from_file(module_names[1], &modules[1]);
    ASSERT(HSA_STATUS_SUCCESS == load_status);

    // Finalize the executable
    hsa_code_object_t code_object;
    hsa_executable_t executable;

    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));

    status = finalize_executable(agent,
                                 num_modules,
                                 modules,
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
    char* symbol_name = "&__element_add_kernel";
    symbol_record_t symbol_record;
    status = get_executable_symbols(executable,
                                    agent,
                                    HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
                                    1,
                                    &symbol_name,
                                    &symbol_record);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Define the argument structure
    typedef struct __attribute__ ((aligned(16))) add_args_s {
        uint32_t* in_one;
        uint32_t* in_two;
        uint32_t* out;
    } add_args_t;
    add_args_t add_args;

    // Allocate the kernel argument buffer from the correct region
    char* kernarg_buffer = NULL;
    status = hsa_memory_allocate(kernarg_region,
                                 symbol_record.kernarg_segment_size,
                                 (void**)(&kernarg_buffer));
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Allocate the arguments
    status = hsa_memory_allocate(global_region, BLOCK_SIZE * sizeof(uint32_t), (void**) &(add_args.in_one));
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_memory_allocate(global_region, BLOCK_SIZE * sizeof(uint32_t), (void**) &(add_args.in_two));
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_memory_allocate(global_region, BLOCK_SIZE * sizeof(uint32_t), (void**) &(add_args.out));
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Initialize the data
    int ii;
    for(ii=0;ii<BLOCK_SIZE;++ii) {
        add_args.in_one[ii] = ii;
        add_args.in_two[ii] = ii;
        add_args.out[ii] = 0;
    }

    //Set up the kernel arguments
    memcpy(kernarg_buffer,&add_args,sizeof(add_args_t));

    // Create the completion signal
    hsa_signal_t completion_signal;
    status = hsa_signal_create(1, 0, NULL, &completion_signal);
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

    // Verify the data
    for(ii=0;ii<BLOCK_SIZE;++ii) {
        int expected = add_args.in_one[ii] + add_args.in_two[ii];
        ASSERT(add_args.out[ii] == expected);
    }

    // Destroy the signal
    status = hsa_signal_destroy(completion_signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Release program resources
    status = hsa_executable_destroy(executable);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_code_object_destroy(code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Releasing module resources
    for(ii=0;ii<num_modules;++ii) {
        destroy_module(modules[ii]);
    }

    status = hsa_queue_destroy(queue);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
