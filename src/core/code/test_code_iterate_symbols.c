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
 * Test Name: test_code_iterate_symbols
 * Scope: Conformance
 *
 * Purpose: Iterate through all code object and executable symbols, and
 * apply hsa_code_object_symbol_get_info() on each symbol in the
 * code object and executable exposed by the symbol iteration callback.
 *
 * Test Description:
 * 1. Create a code object by finalizing the vector_copy kernel.
 * 2. For each agent that supports kernel dispatch, create
 *    an executable and load the code object.
 * 3. Iterate through all code symbols by calling hsa_code_object_iterate_symbols().
 * 4. Within the callback of the iteration, query all symbol information.
 * 5. For each executable, iterate through all symbols by calling
 *    hsa_executable_iterate_symbols().
 * 6. Within the callback, query all symbol information.
 * 7. Compare the symbol information obtained from the code object and the executable,
 *    and ensure it is consistent.
 *
 * Expected Result: All symbols that were not added by an executable define call should
 * be reported by both the code object and the executable.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include "test_helper_func.h"

typedef struct symbol_info_s {
    hsa_symbol_kind_t symbol;
    uint32_t name_length;
    char name[256];
    uint32_t module_name_length;
    char module_name[256];
    hsa_variable_allocation_t variable_allocation;
    hsa_variable_segment_t variable_segment;
    uint32_t variable_alignment;
    uint32_t variable_size;
    bool variable_is_const;
    uint32_t kernel_kernarg_segment_size;
    uint32_t kernel_kernarg_segment_alignment;
    uint32_t kernel_group_segment_size;
    uint32_t kernel_private_segment_size;
    uint32_t indirect_function_call_convention;
} symbol_info_t;

uint32_t exe_symbol_index;
uint32_t code_symbol_index;

hsa_status_t callback_count_executable_symbols(hsa_executable_t exe,
                                               hsa_executable_symbol_t symbol,
                                               void* data) {
    uint32_t* count = (uint32_t*)data;
    (*count) += 1;
    return HSA_STATUS_SUCCESS;
}

hsa_status_t callback_count_code_symbols(hsa_code_object_t code_object,
                                         hsa_code_symbol_t symbol,
                                         void* data) {
    uint32_t* count = (uint32_t*)data;
    (*count) += 1;
    return HSA_STATUS_SUCCESS;
}

hsa_status_t callback_fill_executable_symbol_info(hsa_executable_t exe,
                                               hsa_executable_symbol_t exe_symbol,
                                               void* data) {
    hsa_status_t status;

    // Get the executable status
    hsa_executable_state_t exe_state;
    status = hsa_executable_get_info(exe, HSA_EXECUTABLE_INFO_STATE, &exe_state);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_symbol_kind_t type;
    uint32_t name_length;
    uint32_t module_name_length;
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

    char name[name_length + 1];
    status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_NAME, &name);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's name.\n");
    name[name_length] = '\0';

    status = hsa_executable_symbol_get_info(exe_symbol, HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME_LENGTH, &module_name_length);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get executable symbol's model name length.\n");

    char module_name[module_name_length];
    if (module_name_length > 0) {
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

    symbol_info_t* symbol_info = (symbol_info_t*)data;
    symbol_info += exe_symbol_index;
    ++exe_symbol_index;

    symbol_info->symbol = type;
    symbol_info->name_length = name_length;
    strcmp(symbol_info->name, name);
    symbol_info->module_name_length = module_name_length;
    if (module_name_length > 0) {
        strcmp(symbol_info->module_name, module_name);
    }
    if (HSA_SYMBOL_KIND_VARIABLE == type) {
        symbol_info->variable_allocation = variable_allocation;
        symbol_info->variable_segment = variable_segment;
        symbol_info->variable_size = variable_size;
        symbol_info->variable_alignment = variable_alignment;
        symbol_info->variable_is_const = variable_is_const;
    }
    if (HSA_SYMBOL_KIND_KERNEL == type) {
        symbol_info->kernel_kernarg_segment_size = kernel_kernarg_segment_size;
        symbol_info->kernel_kernarg_segment_alignment = kernel_kernarg_segment_alignment;
        symbol_info->kernel_group_segment_size = kernel_group_segment_size;
        symbol_info->kernel_private_segment_size = kernel_private_segment_size;
    }
    if (HSA_SYMBOL_KIND_INDIRECT_FUNCTION == type) {
        symbol_info->indirect_function_call_convention = indirect_function_call_convention;
    }

    return HSA_STATUS_SUCCESS;
}

hsa_status_t callback_fill_code_symbol_info(hsa_code_object_t code_object,
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

    char module_name[module_name_length];
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

    symbol_info_t* symbol_info = (symbol_info_t*)data;
    symbol_info += code_symbol_index;
    ++code_symbol_index;

    symbol_info->symbol = type;
    symbol_info->name_length = name_length;
    strcmp(symbol_info->name, name);
    symbol_info->module_name_length = module_name_length;
    if (module_name_length > 0) {
        strcmp(symbol_info->module_name, module_name);
    }
    if (HSA_SYMBOL_KIND_VARIABLE == type) {
        symbol_info->variable_allocation = variable_allocation;
        symbol_info->variable_segment = variable_segment;
        symbol_info->variable_size = variable_size;
        symbol_info->variable_alignment = variable_alignment;
        symbol_info->variable_is_const = variable_is_const;
    }
    if (HSA_SYMBOL_KIND_KERNEL == type) {
        symbol_info->kernel_kernarg_segment_size = kernel_kernarg_segment_size;
        symbol_info->kernel_kernarg_segment_alignment = kernel_kernarg_segment_alignment;
        symbol_info->kernel_group_segment_size = kernel_group_segment_size;
        symbol_info->kernel_private_segment_size = kernel_private_segment_size;
    }
    if (HSA_SYMBOL_KIND_INDIRECT_FUNCTION == type) {
        symbol_info->indirect_function_call_convention = indirect_function_call_convention;
    }

    return HSA_STATUS_SUCCESS;
}

int test_code_iterate_symbols() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("vector_copy.brig", &module));

    // Get a list of agents, and iterate throught the list
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Skip if this agent does not support kernel dispatch
        uint32_t feature = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &feature);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (HSA_AGENT_FEATURE_KERNEL_DISPATCH != feature) {
            continue;
        }

        // Get the ISA from this agent
        hsa_isa_t agent_isa;
        agent_isa.handle = (uint64_t)-1;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_ISA, &agent_isa);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT((uint64_t)-1 != agent_isa.handle);

        // Get machine model and profile to create a program
        hsa_machine_model_t machine_model;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_MACHINE_MODEL, &machine_model);
        ASSERT(HSA_STATUS_SUCCESS == status);
        hsa_profile_t profile;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_PROFILE, &profile);
        ASSERT(HSA_STATUS_SUCCESS == status);
        hsa_default_float_rounding_mode_t default_float_rounding_mode;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE, &default_float_rounding_mode);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Finalize the module and get the code object & executable
        hsa_code_object_t code_object;
        hsa_executable_t executable;
        hsa_code_object_type_t code_object_type = HSA_CODE_OBJECT_TYPE_PROGRAM;
        int32_t call_convention = 0;
        hsa_ext_control_directives_t control_directives;
        memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
        status = finalize_executable(agent_list.agents[ii],
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

        uint32_t count_exe_symbols = 0;
        uint32_t count_code_symbols = 0;
        status = hsa_code_object_iterate_symbols(code_object, callback_count_code_symbols, &count_code_symbols);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_executable_iterate_symbols(executable, callback_count_executable_symbols, &count_exe_symbols);
        ASSERT(HSA_STATUS_SUCCESS == status);

        size_t symbol_info_total_size = sizeof(symbol_info_t) * count_exe_symbols;
        symbol_info_t* exe_symbol_info = (symbol_info_t*)malloc(symbol_info_total_size);
        symbol_info_t* code_symbol_info= (symbol_info_t*)malloc(symbol_info_total_size);
        memset(exe_symbol_info, 0, symbol_info_total_size);
        memset(code_symbol_info, 0, symbol_info_total_size);

        status = hsa_code_object_iterate_symbols(code_object, callback_fill_code_symbol_info, code_symbol_info);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_executable_iterate_symbols(executable, callback_fill_executable_symbol_info, exe_symbol_info);
        ASSERT(HSA_STATUS_SUCCESS == status);

        int cmp = memcmp(exe_symbol_info, code_symbol_info, symbol_info_total_size);
        ASSERT(0 == cmp);

        free(exe_symbol_info);
        free(code_symbol_info);

        // Destroy the executable, and the code object
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    free_agent_list(&agent_list);

    destroy_module(module);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
