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

/**
 *
 * Test Name: hsa_signal_destroy
 *
 * Purpose:
 * Verify that if the API works as expected.
 *
 * Description:
 *
 * 1) Create an new signal.
 *    Destroy the signal and check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Before initialize HSA runtime, call hsa_signal_destroy.
 *    Check if the return value is HSA_STATUS_ERROR_NOT_INITIALIZED
 *
 * 3) Call hsa_signal_destroy an invalid signal.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT
 *
 * 4) Call hsa_signal_destroy using a signal that was previously destroyed.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT
 *
 */

/**
 *
 * @Brief:
 * Implement description #1
 *
 * @Return
 * int
 *
 */

int test_hsa_signal_destroy() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = 100;

    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_signal_destroy(signal_handle);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to destroy the signal!\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

/**
 *
 * @Brief:l
 * Implement description #2
 *
 * @Return
 * int
 *
 */

int test_hsa_signal_destroy_not_initialized() {
    hsa_status_t status;

    hsa_signal_t signal_handle;
    status = hsa_signal_destroy(signal_handle);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "The hsa_signal_destroy API failed to return HSA_STATUS_ERROR_NOT_INITIALIZED when called before runtime initialization.\n");

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

int test_hsa_signal_destroy_invalid_arg() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t invalid_signal;
    invalid_signal.handle = 0;
    status = hsa_signal_destroy(invalid_signal);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_signal_destroy failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when called with an invalid signal.\n");

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

int test_hsa_signal_destroy_invalid_signal() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = 100;

    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_signal_destroy(signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Attempt to destroy the signal again
    status = hsa_signal_destroy(signal_handle);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_SIGNAL == status, "The hsa_signal_destroy failed to return HSA_STATUS_ERROR_INVALID_SIGNAL when called with an invalid signal.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
