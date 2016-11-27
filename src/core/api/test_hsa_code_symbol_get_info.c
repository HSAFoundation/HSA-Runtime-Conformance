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
 * Test Name: hsa_code_symbol_get_info
 * Scope: Conformance
 *
 * Purpose: Iterate through all code symbols, and
 *   apply hsa_code_symbol_get_info() on each code code symbol object within the
 *   iteration callback.
 *
 * Test Description:
 * 1. Find an agent that supports kernel dispatch.
 * 2. Create a code object by finalizing the vector_copy kernel.
 * 3. Iterate through all code symbols by
 *    calling hsa_code_object_iterate_symbols().
 * 4. Within the callback of the iteration, query all symbol infos.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <hsa.h>
#include <finalize_utils.h>
#include <framework.h>
#include "test_helper_func.h"

hsa_status_t callback_iterate_symbols(hsa_code_object_t code_object,
                                      hsa_code_symbol_t code_symbol,
                                      void* data) {
    hsa_status_t status;
    hsa_symbol_kind_t type;
    uint32_t name_length;
    uint32_t module_name_length;
    hsa_symbol_linkage_t linkage;
    hsa_variable_allocation_t variable_allocation;
    hsa_variable_segment_t variable_segment;
    uint32_t variable_alignment;
    uint32_t variable_size;
    bool variable_is_const;
    uint32_t kernel_kernarg_segment_size;
    uint32_t kernel_kernarg_segment_alignment;
    uint32_t kernel_group_segment_size;
    uint32_t kernel_private_segment_size;
    bool kernel_dynamic_callstack;
    uint32_t indirect_function_call_convention;

    status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_TYPE, &type);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's type.\n");

    status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_NAME_LENGTH, &name_length);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's name length.\n");
    ASSERT(name_length > 0);

    char name[name_length + 1];
    status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_NAME, &name);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's name.\n");
    name[name_length] = '\0';

    status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_MODULE_NAME_LENGTH, &module_name_length);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's model name length.\n");

    char module_name[module_name_length + 1];
    if (module_name_length > 0) {
        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_MODULE_NAME, &module_name);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's module name.\n");
        module_name[module_name_length] = '\0';
    }

    status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_LINKAGE, &linkage);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's likage.\n");

    switch (type) {
    case HSA_SYMBOL_KIND_VARIABLE:
        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_VARIABLE_ALLOCATION, &variable_allocation);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's variable alloation.\n");

        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_VARIABLE_SEGMENT, &variable_segment);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's variable segment.\n");

        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_VARIABLE_ALIGNMENT, &variable_alignment);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's variable alignment.\n");

        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_VARIABLE_SIZE, &variable_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's variable size.\n");

        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_VARIABLE_IS_CONST, &variable_is_const);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's variable is const.\n");
        break;

    case HSA_SYMBOL_KIND_KERNEL:
        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, &kernel_kernarg_segment_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's kernel segment size.\n");

        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_ALIGNMENT, &kernel_kernarg_segment_alignment);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's kernarg segment alignment.\n");
        ASSERT(kernel_kernarg_segment_alignment <= 16);

        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, &kernel_group_segment_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's kernel group segment size.\n");

        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, &kernel_private_segment_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's kernel private group size.\n");

        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_KERNEL_DYNAMIC_CALLSTACK, &kernel_dynamic_callstack);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's kernel dynamic callstack.\n");
        break;

    case HSA_SYMBOL_KIND_INDIRECT_FUNCTION:
        status = hsa_code_symbol_get_info(code_symbol, HSA_CODE_SYMBOL_INFO_INDIRECT_FUNCTION_CALL_CONVENTION, &indirect_function_call_convention);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get code symbol's indirect function call convention.\n");
        break;
    default:
        break;
    }

    return HSA_STATUS_SUCCESS;
}

int test_hsa_code_symbol_get_info() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the kernel dispatch agent
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(
        callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Load a valid brig module
    hsa_ext_module_t module;
    ASSERT(0 == load_base_or_full_module_from_file(agent,
                                                   "vector_copy_base_large.brig",
                                                   "vector_copy.brig",
                                                   &module));

    // Get machine model and profile to create a program
    hsa_machine_model_t machine_model;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_MACHINE_MODEL, &machine_model);
    ASSERT(HSA_STATUS_SUCCESS == status);
    hsa_profile_t profile;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_PROFILE, &profile);
    ASSERT(HSA_STATUS_SUCCESS == status);
    hsa_default_float_rounding_mode_t default_float_rounding_mode;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE, &default_float_rounding_mode);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Finalize the module and get the code object & executable
    hsa_code_object_t code_object;
    hsa_executable_t executable;
    hsa_code_object_type_t code_object_type = HSA_CODE_OBJECT_TYPE_PROGRAM;
    int32_t call_convention = 0;
    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
    status = finalize_executable(agent,
                                 1,
                                 &module,
                                 machine_model,
                                 profile,
                                 default_float_rounding_mode,
                                 code_object_type,
                                 call_convention,
                                 control_directives,
                                 &code_object,
                                 &executable);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_code_object_iterate_symbols(code_object,
        callback_iterate_symbols, NULL);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_code_object_destroy(code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_executable_destroy(executable);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
