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
#include <stdint.h>

/**
 *
 * Test Name: hsa_signal_create
 *
 * Purpose:
 * Verify that if the API works as expect.
 *
 * Description:
 *
 * 1) Attempt to create a signal.
 *
 * 2) Before initializing the runtime, attempt to create a signal.
 *    Check if the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) Attempt to create a signal passing NULL as the signal value.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 * 4) Attempt to create a signal using 1 for num_consumer value and NULL
 *    as the consumers list.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 * 5) Attempt to create a signal using 2 for num_consumer value and an
 *    agent list that contains duplicate agents.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 */

/**
 *
 * @Brief:
 * Implement Description #1.
 *
 * @Return
 * int
 *
 */

int test_hsa_signal_create() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;

    hsa_signal_value_t initial_value = 100;

    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Signal creation failed.\n");

    status = hsa_signal_destroy(signal_handle);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

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

int test_hsa_signal_create_not_initialized() {
    hsa_status_t status;

    hsa_signal_value_t initial_value = 100;
    hsa_signal_t signal_handle;

    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "The hsa_signal_create API didn't return HSA_STATUS_ERROR_NOT_INITIALIZED when called before runtime initialization.\n");

    return 0;
}

/**
 *
 * @Brief:
 * Implement description #3
 *
 * @Return
 * int
 *
 */

int test_hsa_signal_create_null_signal() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_value_t initial_value = 100;

    status = hsa_signal_create(initial_value, 0, NULL, NULL);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_signal_create API didn't return HSA_STATUS_ERROR_INVALID_ARGUMENT when called with a NULL signal.\n");

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

int test_hsa_signal_create_invalid_arg() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_value_t initial_value = 100;
    hsa_signal_t signal_handle;

    status = hsa_signal_create(initial_value, 1, NULL, &signal_handle);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_signal_create API didn't return HSA_STATUS_ERROR_INVALID_ARGUMENT when called 1 consumer but a NULL consumer list.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
