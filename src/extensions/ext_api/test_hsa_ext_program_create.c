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

#include <stdio.h>
#include <stdlib.h>
#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>

/**
 *
 * Test Name: hsa_ext_program_create
 *
 * Purpose:
 * Verify that if the extension  API works as expected
 *
 * Description:
 *
 * 1) Iterate over all agents and create a program with the one that supports kernel dispatch.
 *
 * 2) Before the runtime is initialized call hsa_ext_program_create and check
 *    that the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) Call hsa_ext_program_create with an invalid machine model ,
 *    an invalid profile, and invalid FLOAT_ROUNDING_MODE, invalid options.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_AGENT.
 *
 */

/**
 *
 * @Brief:
 * Implement Description #1
 *
 * @Return:
 * int
 *
 */

int test_hsa_ext_program_create(){
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

    hsa_machine_model_t machine_model;
    status = hsa_system_get_info(HSA_SYSTEM_INFO_MACHINE_MODEL, &machine_model);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get machine_model.\n");

    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    hsa_profile_t profile;
    status = hsa_agent_get_info(agent,HSA_AGENT_INFO_PROFILE, &profile);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent_info_profile.\n");

    hsa_default_float_rounding_mode_t default_float_rounding_mode;
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Attempting to create a program with hsa_ext_program_create API failed");
    status = hsa_agent_get_info(agent,HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE, &default_float_rounding_mode);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent_info_default_float_rounding_mode.\n");

    status = hsa_system_get_info(HSA_SYSTEM_INFO_MACHINE_MODEL, &machine_model);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get machine_model.\n");

    hsa_ext_program_t program;
    memset(&program, 0, sizeof(hsa_ext_program_t));
    status = pfn.hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, &program);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Attempting to create a program with hsa_ext_program_create API failed");

    status = pfn.hsa_ext_program_destroy(program);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

/**
 *
 * @Brief:
 * Implement Description #2
 *
 * @Return:
 * int
 *
 */

int test_hsa_ext_program_create_not_initialized() {
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

    hsa_machine_model_t machine_model = 0;
    status = hsa_system_get_info(HSA_SYSTEM_INFO_MACHINE_MODEL, &machine_model);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get machine_model.\n");
    ASSERT(machine_model == HSA_MACHINE_MODEL_LARGE || machine_model == HSA_MACHINE_MODEL_SMALL);

    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    hsa_profile_t profile;
    status = hsa_agent_get_info(agent,HSA_AGENT_INFO_PROFILE, &profile);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent_info_profile.\n");

    hsa_default_float_rounding_mode_t default_float_rounding_mode;
    status = hsa_agent_get_info(agent,HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE, &default_float_rounding_mode);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent_info_default_float_rounding_mode.\n");

    hsa_ext_program_t program;
    memset(&program, 0, sizeof(hsa_ext_program_t));

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = pfn.hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, &program);
    if(HSA_STATUS_ERROR_INVALID_AGENT == status){
        ASSERT_MSG(0, "The hsa_ext_program_create API returned HSA_STATUS_ERROR_INVALID_AGENT instead of HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime wasn't initialized or is shutdown before API is called.\n");
    } else if(HSA_STATUS_ERROR_NOT_INITIALIZED == status) {
        // This is the expected error
    } else {
        ASSERT_MSG(0, "The hsa_ext_program_create API returned an error other than HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime wasn't initialized.\n");
    }

    status = pfn.hsa_ext_program_destroy(program);
    if(HSA_STATUS_ERROR_INVALID_AGENT == status){
        ASSERT_MSG(0, "The hsa_ext_program_create API returned HSA_STATUS_ERROR_INVALID_AGENT instead of HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime wasn't initialized or is shutdown before API is called.\n");
    } else if(HSA_STATUS_ERROR_NOT_INITIALIZED == status) {
        // This is the expected error
    } else {
        ASSERT_MSG(0, "The hsa_ext_program_create API returned an error other than HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime wasn't initialized.\n");
    }

    return 0;
}

/**
 *
 * @Brief:
 * Implement Description #3
 *
 * @Return:
 * int
 *
 */

int test_hsa_ext_program_create_invalid_argument(){
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

    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    hsa_profile_t profile;
    status = hsa_agent_get_info(agent,HSA_AGENT_INFO_PROFILE, &profile);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent_info_profile.\n");

    hsa_default_float_rounding_mode_t default_float_rounding_mode;
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Attempting to create a program with hsa_ext_program_create API failed");
    status = hsa_agent_get_info(agent,HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE, &default_float_rounding_mode);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent_info_default_float_rounding_mode.\n");

    hsa_ext_program_t program;
    memset(&program, 0, sizeof(hsa_ext_program_t));

    // Invalid machine_model
    hsa_machine_model_t machine_model = 2;
    status = pfn.hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, &program);
    if(HSA_STATUS_SUCCESS == status){
        ASSERT_MSG(1,"ext_program is created with  a wrong machine_model. ERROR_INVALID_ARGUMET is expected, ERROR received.\n");
    } else if(HSA_STATUS_ERROR_INVALID_ARGUMENT == status) {
        // This indicate proper behaviour
    } else {
        ASSERT(0);
    }

    // Invalid machine_model
    machine_model = -1;
    status = pfn.hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, &program);
    if(HSA_STATUS_SUCCESS == status) {
        ASSERT_MSG(1,"hsa_ext_program_create is created with  a wrong machine_model. ERROR_INVALID_ARGUMET is expected, ERROR received.\n");
    } else if(HSA_STATUS_ERROR_INVALID_ARGUMENT == status) {
    // This indicate proper behaviour
    } else {
        ASSERT(0);
    }

    // Invalid profile
    machine_model = 0;
    hsa_profile_t invalid_profile = -1 ;
    status = pfn.hsa_ext_program_create(machine_model, invalid_profile, default_float_rounding_mode, NULL, &program);
    if(HSA_STATUS_SUCCESS == status){
        ASSERT_MSG(1,"hsa_ext_program_create is created with a wrong profile. ERROR_INVALID_ARGUMENT is expected, ERROR received\n");
    } else if(HSA_STATUS_ERROR_INVALID_ARGUMENT == status) {
        // This indicate proper behaviour
    } else {
        ASSERT(0);
    }

    // Invalid default_floating_mode
    hsa_default_float_rounding_mode_t invalid_default_float_rounding_mode = -1 ;
    status = pfn.hsa_ext_program_create(machine_model, profile,invalid_default_float_rounding_mode, NULL, &program);
    if(HSA_STATUS_SUCCESS == status) {
        ASSERT_MSG(1,"hsa_ext_program_create is created with a wrong default_float_rounding_mode.ERROR_INVALID_ARGUMENT is expected. ERROR received\n");
    } else if(HSA_STATUS_ERROR_INVALID_ARGUMENT == status) {
        // This indicate proper behaviour
    } else {
        ASSERT(0);
    }

    // Invalid program
    status = pfn.hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, NULL);
    if(HSA_STATUS_SUCCESS == status) {
        ASSERT_MSG(1,"hsa_ext_program_create is created with a NULL program .ERROR_INVALID_ARGUMENT is expected. ERROR received\n");
    } else if(HSA_STATUS_ERROR_INVALID_ARGUMENT == status) {
        // This indicate proper behaviour
    } else {
        ASSERT(0);
    }

    // Since program is never created, destroy is not needed.

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
