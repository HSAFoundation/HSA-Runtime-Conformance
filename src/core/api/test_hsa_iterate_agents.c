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
#include <framework.h>
#include "test_helper_func.h"

/**
 *
 * Test Name: hsa_iterate_agents
 *
 * Purpose:
 * Verify that if the API works as expected
 *
 * Description:
 *
 * 1) After init HsaRt, call hsa_iterate_agents API with the callback of callback_test
 *      Check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Before init HsaRt, call hsa_iterate_agents, and check if the return value is HSA_STATUS_ERROR_NOT_INITIALIZED
 *
 * 3) Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT when passing NULL to callback
 */

hsa_status_t callback_test(hsa_agent_t agent, void* data) {
    return HSA_STATUS_SUCCESS;
}

/**
 *
 * @Brief:
 * Implement Description #1
 *
 * @Return:
 * int
 *
 */

int test_hsa_iterate_agents() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_iterate_agents(callback_test, NULL);

    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "The hsa_iterate_agents API failed.\n");

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

int test_hsa_iterate_agents_not_initialized() {
    hsa_status_t status;

    int number = 0;
    status = hsa_iterate_agents(callback_test, &number);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "The hsa_iterate_agents API failed to return HSA_STATUS_ERROR_NOT_INITIALIZED when called before initializing the runtime.\n");

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

int test_hsa_iterate_agents_invalid_callback() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_iterate_agents(NULL, NULL);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_iterate_agents API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when called with a NULL callback function.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
