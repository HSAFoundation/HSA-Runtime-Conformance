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
#include <hsa_ext_image.h>
#include <agent_utils.h>
#include <framework.h>

/**
 *
 * Test Name: hsa_agent_get_exception_policies
 *
 * Purpose:
 * Verify that if the API hsa_agent_get_exception_policies API
 * works as expected.
 *
 * Description:
 *
 * 1) Call hsa_agent_get_exception_policies API for a supported profile.
 *    Check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Call hsa_agent_get_exception_policies API before initializing the runtime.
 *    Check if the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) Call hsa_agent_get_exception_policies API with an invalid agent.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_AGENT.
 *
 * 4) Call hsa_agent_get_exception_policies API with a NULL mask parameter.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.

 * 4) Call hsa_agent_get_exception_policies API with an invalid profile.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 */

/**
 *
 * @Brief:
 * Implement Description #1
 *
 * @Return
 * int
 *
 */

int test_hsa_agent_get_exception_policies() {
    uint16_t mask;
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;

    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        status = hsa_agent_get_exception_policies(agent_list.agents[ii], HSA_PROFILE_FULL, &mask);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}

/**
 *
 * @Brief:
 * Implement Description #2
 *
 * @Return
 * int
 *
 */

int test_hsa_agent_get_exception_policies_not_initialized() {
    uint16_t mask;
    hsa_status_t status;

    hsa_agent_t agent;
    agent.handle = 0;

    status = hsa_agent_get_exception_policies(agent, HSA_PROFILE_FULL, &mask);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "The hsa_agent_get_exception_policies API failed to return HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime was not initialized.\n");

    return 0;
}

/**
 *
 * @Brief:
 * Implement Description #3
 *
 * @Return
 * int
 *
 */

int test_hsa_agent_get_exception_policies_invalid_agent() {
    uint16_t mask;
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_agent_t agent;
    agent.handle = 0;

    status = hsa_agent_get_exception_policies(agent, HSA_PROFILE_FULL, &mask);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_AGENT == status, "The hsa_agent_get_exception_policies API failed to return HSA_STATUS_ERROR_INVALID_AGENT when passed an invalid agent.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

/**
 *
 * @Brief:
 * Implement Description #4
 *
 * @Return:
 * int
 *
 */

int test_hsa_agent_get_exception_policies_null_mask_ptr() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        status = hsa_agent_get_exception_policies(agent_list.agents[ii], HSA_PROFILE_FULL, NULL);
        ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_agent_get_exception_policies API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when a NULL mask pointer was used.\n");
    }

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}

/**
 *
 * @Brief:
 * Implement Description #5
 *
 * @Return:
 * int
 *
 */

int test_hsa_agent_get_exception_policies_invalid_profile() {
    uint16_t mask;
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        status = hsa_agent_get_exception_policies(agent_list.agents[ii], -1, &mask);
        ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_agent_get_exception_policies API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when an invalid profile was specified.\n");
    }

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
