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
 * Test Name: hsa_executable_load_code_object
 * Scope: Conformance
 *
 * Purpose: load code object into HSA executables with various parameter
 *   settings.
 *
 * Test Description:
 * 1. Without initializing the HSA runtime, declare a set of agent, executable,
 *    and code object. Load the code object into the executable, expect to
 *    receive NOT_INITIALIZED error.
 * 2. Initialize HSA runtime, then properly create a set of agent, executable,
 *    and code object.
 * 3. Load the code object into the executable. No error should occur at this
 *    point.
 * 4. For each invalid agent, invalid executable, and invalid code object, call
 *    the hsa_executable_load_code_object() and pass these invalid object as
 *    argument. The error code should indicate which argument is invalid.
 * 5. Freeze the valid executable object, do a load again with all correct
 *    arguments. Expect to receive FROZEN_EXECUTABLE error.
 * 6. Destroy all the object, shutdown the HSA runtime, and finish.
 *
 */

#include <stdio.h>
#include <hsa.h>
#include <framework.h>
#include <finalize_utils.h>
#include "test_helper_func.h"

void load_module_finalize_program(
        hsa_ext_finalizer_pfn_t* pfn,
        hsa_ext_module_t* module_ptr,
        hsa_code_object_t* code_object_ptr,
        hsa_ext_program_t* program_ptr,
        hsa_executable_t* exe_ptr) {
    hsa_status_t status;
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;

    status = hsa_iterate_agents(callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // get the ISA from this agent
    hsa_isa_t agent_isa;
    agent_isa.handle = (uint64_t)-1;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &agent_isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != agent_isa.handle);

    // Load the BRIG module
    ASSERT(0 == load_base_or_full_module_from_file(agent,
                                                   "no_op_base_large.brig",
                                                   "no_op.brig",
                                                   module_ptr));

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
    memset(program_ptr, 0, sizeof(hsa_ext_program_t));
    status = pfn->hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, program_ptr);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Add the brig modules to the program
    status = pfn->hsa_ext_program_add_module(*program_ptr, *module_ptr);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Finalize the program and extract the code object
    memset(code_object_ptr, 0, sizeof(hsa_code_object_t));
    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
    status = pfn->hsa_ext_program_finalize(*program_ptr, agent_isa, 0, control_directives, NULL, HSA_CODE_OBJECT_TYPE_PROGRAM, code_object_ptr);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_executable_create(profile,
        HSA_EXECUTABLE_STATE_UNFROZEN, NULL, exe_ptr);
    ASSERT(HSA_STATUS_SUCCESS == status);
}

int test_hsa_executable_load_code_object() {
    hsa_status_t status;

    // initialize the HSA runtime, and create all required objects for a
    // successful load
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

    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    hsa_ext_module_t module;
    hsa_code_object_t code_object;
    hsa_ext_program_t program;
    hsa_executable_t exe;
    load_module_finalize_program(&pfn, &module, &code_object, &program, &exe);

    // load the code object into this executable, no error should occur
    status = hsa_executable_load_code_object(exe, agent, code_object, NULL);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_executable_destroy(exe);
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

int test_hsa_executable_load_code_object_not_initialized() {
    hsa_status_t status;

    // load code object without initializing the HSA runtime
    hsa_agent_t invalid_agent;
    hsa_executable_t invalid_exe;
    hsa_code_object_t invalid_code_obj;
    status = hsa_executable_load_code_object(invalid_exe, invalid_agent, invalid_code_obj, NULL);
    ASSERT(HSA_STATUS_ERROR_NOT_INITIALIZED == status);

    return 0;
}

int test_hsa_executable_load_code_object_invalid_executable() {
    hsa_status_t status;

    // initialize the HSA runtime, and create all required objects
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

    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    hsa_ext_module_t module;
    hsa_code_object_t code_object;
    hsa_ext_program_t program;
    hsa_executable_t exe;
    load_module_finalize_program(&pfn, &module, &code_object, &program, &exe);

    // load this valid code object into an invalid executable
    hsa_executable_t invalid_exe;
    invalid_exe.handle = (uint64_t)-1;
    status = hsa_executable_load_code_object(invalid_exe, agent, code_object, NULL);
    ASSERT(HSA_STATUS_ERROR_INVALID_EXECUTABLE == status);

    status = hsa_code_object_destroy(code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = pfn.hsa_ext_program_destroy(program);
    ASSERT(HSA_STATUS_SUCCESS == status);
    destroy_module(module);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    return 0;
}

int test_hsa_executable_load_code_object_invalid_agent() {
    hsa_status_t status;

    // initialize the HSA runtime, and create all required objects
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

    hsa_ext_module_t module;
    hsa_code_object_t code_object;
    hsa_ext_program_t program;
    hsa_executable_t exe;
    load_module_finalize_program(&pfn, &module, &code_object, &program, &exe);

    // load the code object with an invalid agent
    hsa_agent_t invalid_agent;
    invalid_agent.handle = (uint64_t)-1;
    status = hsa_executable_load_code_object(exe, invalid_agent, code_object, NULL);
    ASSERT(HSA_STATUS_ERROR_INVALID_AGENT == status);

    status = hsa_code_object_destroy(code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = pfn.hsa_ext_program_destroy(program);
    ASSERT(HSA_STATUS_SUCCESS == status);
    destroy_module(module);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    return 0;
}

int test_hsa_executable_load_code_object_invalid_code_object() {
    hsa_status_t status;

    // initialize the HSA runtime, and create all required objects
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    hsa_profile_t profile;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_PROFILE, &profile);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_executable_t exe;
    status = hsa_executable_create(profile,
        HSA_EXECUTABLE_STATE_UNFROZEN, NULL, &exe);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // load an invalid code object into the executable
    hsa_code_object_t invalid_code_object;
    invalid_code_object.handle = (uint64_t)-1;
    status = hsa_executable_load_code_object(exe, agent, invalid_code_object, NULL);
    ASSERT(HSA_STATUS_ERROR_INVALID_CODE_OBJECT == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    return 0;
}

int test_hsa_executable_load_code_object_frozen_executable() {
    hsa_status_t status;

    // initialize the HSA runtime, and create all required objects
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

    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    hsa_ext_module_t module;
    hsa_code_object_t code_object;
    hsa_ext_program_t program;
    hsa_executable_t exe;
    load_module_finalize_program(&pfn, &module, &code_object, &program, &exe);

    // load this valid code object into an invalid executable
    status = hsa_executable_freeze(exe, NULL);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_executable_load_code_object(exe, agent, code_object, NULL);
    ASSERT(HSA_STATUS_ERROR_FROZEN_EXECUTABLE == status);

    status = hsa_code_object_destroy(code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = pfn.hsa_ext_program_destroy(program);
    ASSERT(HSA_STATUS_SUCCESS == status);
    destroy_module(module);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    return 0;
}
