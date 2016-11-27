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
 * Test Name: hsa_isa_compatible
 * Scope: Conformance
 *
 * Purpose: Verify that the hsa_isa_compatible API works correctly.
 *
 * Test Description:
 *
 * 1) Create a code object and obtain the code objects isa. Query
 * a valid dispatch agent for its isa. Use the hsa_isa_compatible
 * API to query if the two isas are compatible. The call to hsa_isa_compatible
 * should succeed. The two isa may or may not be compatible.
 *
 * 2) Attempt to call hsa_isa_compatible twice; first with an invalid
 * code_object_isa and then with an invalid agent_isa. In both cases
 * the API should return HSA_STATUS_ERROR_INVALID_ISA.
 *
 * 3) Call hsa_isa_compatible with a NULL result argument. The API
 * should return HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 */

#include <stdio.h>
#include <hsa.h>
#include <hsa_ext_image.h>
#include <framework.h>
#include <finalize_utils.h>
#include "test_helper_func.h"

int test_hsa_isa_compatible() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the finalization funtion pointer table
    hsa_ext_finalizer_pfn_t pfn;
    status = get_finalization_fnc_tbl(&pfn);
    // This indicates that finalization isn't
    // supported. The test will succeed in that
    // case.
    if(HSA_STATUS_SUCCESS != status) {
        return 0;
    }

    // Get the kernel dispatch agent
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(
        callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Get the ISA from this agent
    hsa_isa_t agent_isa;
    agent_isa.handle = (uint64_t)-1;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &agent_isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != agent_isa.handle);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_base_or_full_module_from_file(agent,
                                                   "no_op_base_large.brig",
                                                   "no_op.brig",
                                                   &module));

    hsa_machine_model_t machine_model;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_MACHINE_MODEL, &machine_model);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_profile_t profile;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_PROFILE, &profile);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_default_float_rounding_mode_t default_float_rounding_mode;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE, &default_float_rounding_mode);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create the program
    hsa_ext_program_t program;
    memset(&program, 0, sizeof(hsa_ext_program_t));
    status = pfn.hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, &program);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Add the brig modules to the program
    status = pfn.hsa_ext_program_add_module(program, module);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Finalize the program and extract the code object
    hsa_code_object_t code_object;
    memset(&code_object, 0, sizeof(hsa_code_object_t));
    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
    status = pfn.hsa_ext_program_finalize(program, agent_isa, 0, control_directives, NULL, HSA_CODE_OBJECT_TYPE_PROGRAM, &code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the ISA from the code object
    hsa_isa_t code_object_isa;
    code_object_isa.handle = (uint64_t)-1;
    status = hsa_code_object_get_info(code_object,
        HSA_CODE_OBJECT_INFO_ISA, &code_object_isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != code_object_isa.handle);

    // Check if these two ISAs are compatible
    bool compatible;
    status = hsa_isa_compatible(code_object_isa, agent_isa, &compatible);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_code_object_destroy(code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = pfn.hsa_ext_program_destroy(program);
    ASSERT(HSA_STATUS_SUCCESS == status);

    destroy_module(module);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

int test_hsa_isa_compatible_invalid_isa() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the finalization funtion pointer table
    hsa_ext_finalizer_pfn_t pfn;
    status = get_finalization_fnc_tbl(&pfn);
    // This indicates that finalization isn't
    // supported. The test will succeed in that
    // case.
    if(HSA_STATUS_SUCCESS != status) {
        return 0;
    }

    // Get the kernel dispatch agent
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(
        callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Get the ISA from this agent
    hsa_isa_t agent_isa;
    agent_isa.handle = (uint64_t)-1;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &agent_isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != agent_isa.handle);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_base_or_full_module_from_file(agent,
                                                   "no_op_base_large.brig",
                                                   "no_op.brig",
                                                   &module));

    hsa_machine_model_t machine_model;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_MACHINE_MODEL, &machine_model);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_profile_t profile;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_PROFILE, &profile);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_default_float_rounding_mode_t default_float_rounding_mode;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE, &default_float_rounding_mode);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create the program
    hsa_ext_program_t program;
    memset(&program, 0, sizeof(hsa_ext_program_t));
    status = pfn.hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, &program);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Add the brig modules to the program
    status = pfn.hsa_ext_program_add_module(program, module);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Finalize the program and extract the code object
    hsa_code_object_t code_object;
    memset(&code_object, 0, sizeof(hsa_code_object_t));
    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
    status = pfn.hsa_ext_program_finalize(program, agent_isa, 0, control_directives, NULL, HSA_CODE_OBJECT_TYPE_PROGRAM, &code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the ISA from the code object
    hsa_isa_t code_object_isa;
    code_object_isa.handle = (uint64_t)-1;
    status = hsa_code_object_get_info(code_object,
        HSA_CODE_OBJECT_INFO_ISA, &code_object_isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != code_object_isa.handle);

    // Check that an invalid code object isa generates
    // a HSA_STATUS_ERROR_INVALID_ISA error 
    bool compatible;
    hsa_isa_t invalid_isa;
    invalid_isa.handle = 0;
    status = hsa_isa_compatible(invalid_isa, agent_isa, &compatible);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ISA == status, "The hsa_isa_compatible API did not return HSA_STATUS_ERROR_INVALID_ISA when passed an invalid object code isa.");

    // Check that an invalid agent isa generates
    // a HSA_STATUS_ERROR_INVALID_ISA error 
    status = hsa_isa_compatible(code_object_isa, invalid_isa, &compatible);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ISA == status, "The hsa_isa_compatible API did not return HSA_STATUS_ERROR_INVALID_ISA when passed an invalid agent isa.");

    status = hsa_code_object_destroy(code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = pfn.hsa_ext_program_destroy(program);
    ASSERT(HSA_STATUS_SUCCESS == status);

    destroy_module(module);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

int test_hsa_isa_compatible_null_result() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the finalization funtion pointer table
    hsa_ext_finalizer_pfn_t pfn;
    status = get_finalization_fnc_tbl(&pfn);
    // This indicates that finalization isn't
    // supported. The test will succeed in that
    // case.
    if(HSA_STATUS_SUCCESS != status) {
        return 0;
    }

    // Get the kernel dispatch agent
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(
        callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Get the ISA from this agent
    hsa_isa_t agent_isa;
    agent_isa.handle = (uint64_t)-1;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &agent_isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != agent_isa.handle);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_base_or_full_module_from_file(agent,
                                                   "no_op_base_large.brig",
                                                   "no_op.brig",
                                                   &module));

    hsa_machine_model_t machine_model;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_MACHINE_MODEL, &machine_model);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_profile_t profile;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_PROFILE, &profile);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_default_float_rounding_mode_t default_float_rounding_mode;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE, &default_float_rounding_mode);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create the program
    hsa_ext_program_t program;
    memset(&program, 0, sizeof(hsa_ext_program_t));
    status = pfn.hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, &program);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Add the brig modules to the program
    status = pfn.hsa_ext_program_add_module(program, module);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Finalize the program and extract the code object
    hsa_code_object_t code_object;
    memset(&code_object, 0, sizeof(hsa_code_object_t));
    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
    status = pfn.hsa_ext_program_finalize(program, agent_isa, 0, control_directives, NULL, HSA_CODE_OBJECT_TYPE_PROGRAM, &code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the ISA from the code object
    hsa_isa_t code_object_isa;
    code_object_isa.handle = (uint64_t)-1;
    status = hsa_code_object_get_info(code_object,
        HSA_CODE_OBJECT_INFO_ISA, &code_object_isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != code_object_isa.handle);

    // Check if these two ISAs are compatible
    status = hsa_isa_compatible(code_object_isa, agent_isa, NULL);
    ASSERT(HSA_STATUS_ERROR_INVALID_ARGUMENT == status);

    status = hsa_code_object_destroy(code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = pfn.hsa_ext_program_destroy(program);
    ASSERT(HSA_STATUS_SUCCESS == status);

    destroy_module(module);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
