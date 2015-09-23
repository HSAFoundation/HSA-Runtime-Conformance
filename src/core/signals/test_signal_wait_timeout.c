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

/*
 * Test Name: signal_wait_timeout
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_signal_wait APIs properly use timeout hints.
 *
 * Test Description:
 * 1) Create a signal.
 * 2) Specify a value, condition operator and compare value that
 * create a unsatisfied condition.
 * 3) Wait on the signal with one of the hsa_signal_wait APIs using
 * a non-zero timeout hint.
 * 4) Use the system TIMESTAMP attribute to calculate how long the
 * API waited before returning.
 *
 * Expected Results: The API should wait for the specified timeout
 * hint, with a reasonable variation.
 */

#include<hsa.h>
#include<framework.h>

int test_signal_wait_timeout(hsa_signal_value_t (*wait_fnc)(hsa_signal_t signal, hsa_signal_condition_t condition, hsa_signal_value_t compare_value, uint64_t timeout_hint, hsa_wait_state_t wait_state_hint)) {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    uint16_t timestamp_freq;
    status = hsa_system_get_info(HSA_SYSTEM_INFO_TIMESTAMP_FREQUENCY, &timestamp_freq);
    ASSERT(status == HSA_STATUS_SUCCESS);

    hsa_signal_t signal;
    status = hsa_signal_create(0, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    uint64_t start_time, stop_time;

    status = hsa_system_get_info(HSA_SYSTEM_INFO_TIMESTAMP, &start_time);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // The wait time should be 1 second if the timestamp_freq value is used
    // Try both wait states
    wait_fnc(signal, HSA_SIGNAL_CONDITION_EQ, 1, timestamp_freq, HSA_WAIT_STATE_BLOCKED);
    wait_fnc(signal, HSA_SIGNAL_CONDITION_EQ, 1, timestamp_freq, HSA_WAIT_STATE_ACTIVE);

    status = hsa_system_get_info(HSA_SYSTEM_INFO_TIMESTAMP, &stop_time);
    ASSERT(status == HSA_STATUS_SUCCESS);

    uint64_t wait_delta = (stop_time - start_time);

    // The timeout value is a hint, so the actual wait time is arbitrary, but should
    // be greater than zero.
    ASSERT(wait_delta > 0);

    status = hsa_signal_destroy(signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

int test_signal_wait_acquire_timeout() {
    test_signal_wait_timeout(hsa_signal_wait_acquire);
    return 0;
}

int test_signal_wait_relaxed_timeout() {
    test_signal_wait_timeout(hsa_signal_wait_relaxed);
    return 0;
}
