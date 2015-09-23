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

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>
#include <finalize_utils.h>

/**
 *
 * Test Name: hsa_ext_program_destroy
 *
 * Purpose:
 * Verify that if the extension  API works as expected
 *
 * Description:
 *
 * 1) Destroy a HSAIL program, use hsa_ext_program_create to create one.
 *
 * 2) Before the runtime is initialized call hsa_ext_program_destroy and check
 *    that the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) Call hsa_ext_program_destroy with an invalid HSAIL program
 *    check if the return value is HSA_EXT_STATUS_ERROR_INVALID_PROGRAM.
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


int test_hsa_ext_program_destroy() {
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
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Attempt to destroy a HSAIL progrma with hsa_ext_program_destroy API failed");
 
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

int test_hsa_ext_program_destroy_not_initialized() {
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
    if(HSA_STATUS_ERROR_NOT_INITIALIZED == status) {
        // This is the expected error
    } else{
        ASSERT_MSG(0, "The hsa_ext_program_create API returned an error other than HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime wasn't initialized.\n");
    }

    status = pfn.hsa_ext_program_destroy(program);
    if(HSA_STATUS_ERROR_NOT_INITIALIZED == status){
        // This is the expected error
    } else {
        ASSERT_MSG(0, "The hsa_ext_program_destroy API returned an error other than HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime wasn't initialized. ERROR received.\n");
    }
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

int test_hsa_ext_program_destroy_invalid_program() {
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

    hsa_ext_program_t invalid_program; 
    invalid_program.handle  = (uint64_t)-1;

    status = pfn.hsa_ext_program_destroy(invalid_program);
    ASSERT(HSA_EXT_STATUS_ERROR_INVALID_PROGRAM == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    return 0;
}
