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

/**
 * Test Name: hsa_executable_symbol_get_info
 * Scope: Conformance
 *
 * Purpose: get infos from an executable symbol.
 *
 * Test Description:
 * 1. Initialize HSA runtime, then properly create an executable.
 * 2. Create a "vector_copy" code object.
 * 3. Load the code object into the executable.
 * 4. Query executable symbol infos.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <hsa.h>
#include <framework.h>
#include "test_helper_func.h"

hsa_status_t callback_executable_iterate_symbols(hsa_executable_t exe, hsa_executable_symbol_t exe_symbol, void* data) {
    hsa_status_t status;

    // Get the executable status
    hsa_executable_state_t exe_state;
    status = hsa_executable_get_info(exe, HSA_EXECUTABLE_INFO_STATE, &exe_state);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_symbol_kind_t type;
    uint32_t name_length;
    char* name;
    uint32_t module_name_length;
    char* module_name;
    hsa_agent_t agent;
    uint64_t variable_address;
    hsa_symbol_linkage_t linkage;
    hsa_variable_allocation_t variable_allocation;
    hsa_variable_segment_t variable_segment;
    uint32_t variable_alignment;
    uint32_t variable_size;
    bool variable_is_const;
    uint64_t kernel_object;
    uint32_t kernel_kernarg_segment_size;
    uint32_t kernel_kernarg_segment_alignment;
    uint32_t kernel_group_segment_size;
    uint32_t kernel_private_segment_size;
    bool kernel_dynamic_callstack;
    uint64_t indirect_function_object;
    uint32_t indirect_function_call_convention;

    status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_TYPE, &type);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's type.\n");

    status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_NAME_LENGTH, &name_length);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's name length.\n");
    ASSERT(name_length > 0);

    name = (char*)malloc(sizeof(char) * (name_length + 1));
    status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_NAME, &name);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's name.\n");
    name[name_length] = '\0';

    status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME_LENGTH, &module_name_length);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's model name length.\n");

    if (module_name_length > 0) {
        module_name = (char*)malloc(sizeof(char) * (module_name_length + 1));
        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME, &module_name);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's module name.\n");
        module_name[module_name_length] = '\0';
    }

    status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_LINKAGE, &linkage);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's linkage.\n");

    if (HSA_SYMBOL_KIND_VARIABLE == type) {
        if (HSA_SYMBOL_KIND_VARIABLE == type && HSA_VARIABLE_ALLOCATION_AGENT == variable_allocation) {
            status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_AGENT, &agent);
            ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's agent.\n");
        }

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ADDRESS, &variable_address);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's variable address.\n");

        if (HSA_EXECUTABLE_STATE_UNFROZEN == exe_state) {
            ASSERT(0 == variable_address);
        }

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ALLOCATION, &variable_allocation);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's variable allocation.\n");

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_SEGMENT, &variable_segment);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's variable segment.\n");

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ALIGNMENT, &variable_alignment);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's vairable alignment.\n");

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_SIZE, &variable_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's variable size.\n");

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_IS_CONST, &variable_is_const);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's variable is const.\n");
    }

    if (HSA_SYMBOL_KIND_KERNEL == type) {
        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT, &kernel_object);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's kernel object.\n");
        if (HSA_EXECUTABLE_STATE_UNFROZEN == exe_state) {
            ASSERT(0 == kernel_object);
        }

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, &kernel_kernarg_segment_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's kernel kernarg segment size.\n");

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_ALIGNMENT, &kernel_kernarg_segment_alignment);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's kernel kernarg segment alignment.\n");
        ASSERT(kernel_kernarg_segment_alignment <= 16);

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, &kernel_group_segment_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's kernel group segment size.\n");

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, &kernel_private_segment_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's kernel private segment size.\n");

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_DYNAMIC_CALLSTACK, &kernel_dynamic_callstack);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's kernel dynamic callstack.\n");
    }

    if (HSA_SYMBOL_KIND_INDIRECT_FUNCTION == type) {
        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_INDIRECT_FUNCTION_OBJECT, &indirect_function_object);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's indirect function object.\n");
        if (HSA_EXECUTABLE_STATE_UNFROZEN == exe_state) {
            ASSERT(0 == indirect_function_object);
        }

        status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_INDIRECT_FUNCTION_CALL_CONVENTION, &indirect_function_call_convention);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's indirect function call convention.\n");
    }
}

int test_hsa_executable_symbol_get_info() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the kernel dispatch agent
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(
        callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Finalize the "vector_copy" kernel
    hsa_code_object_t code_object;
    code_object.handle = (uint64_t)-1;
    code_object = load_code_object(&agent, "vector_copy.brig", "&__vector_copy_kernel");
    ASSERT((uint64_t)-1 != code_object.handle);

    // Create the executable
    hsa_profile_t profile;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_PROFILE, &profile);
    ASSERT(HSA_STATUS_SUCCESS == status);
    hsa_executable_t exe;
    status = hsa_executable_create(profile,
        HSA_EXECUTABLE_STATE_UNFROZEN, NULL, &exe);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the code object into this executable, no error should occur
    status = hsa_executable_load_code_object(exe, agent, code_object, NULL);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_executable_iterate_symbols(exe, callback_executable_iterate_symbols, NULL);

    status = hsa_executable_destroy(exe);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_code_object_destroy(code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
