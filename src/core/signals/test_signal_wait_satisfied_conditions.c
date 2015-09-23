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
 * Test Name: signal_wait_satisfied_conditions
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_signal_wait APIs properly use all of the
 * hsa_signal_condition_t specifiers, specifically when the signal value
 * already satisfies the condition.
 *
 * Test Description:
 * 1) Create a signal.
 * 2) For each of the hsa_signal_condition_t specifiers
 *    a) Set the signal's value and a compare value such that the
 *    condition is already satisfied.
 *    b) Wait on the signal using one of the hsa_signal_wait APIs
 * 3) Repeat this for all hsa_signal_wait APIs
 *
Expected Results: The waiting thread should return immediately
 */

#include <hsa.h>
#include <framework.h>

void do_signal_wait_satisfied_conditions(hsa_signal_condition_t wait_condition,
                                         hsa_signal_value_t set_value,
                                         hsa_signal_value_t wait_value) {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Initialize the signals with 0
    hsa_signal_t signals[2];
    status = hsa_signal_create(0, 0, NULL, signals);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_signal_create(0, 0, NULL, signals + 1);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Set the values
    hsa_signal_store_relaxed(signals[0], set_value);
    hsa_signal_store_relaxed(signals[1], set_value);

    // Wait for the response
    hsa_signal_wait_relaxed(signals[0], wait_condition, wait_value, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    hsa_signal_wait_acquire(signals[1], wait_condition, wait_value, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    return;
}

void signal_wait_satisfied_conditions_eq() {
    do_signal_wait_satisfied_conditions(HSA_SIGNAL_CONDITION_EQ, 1, 1);
    return;
}

void signal_wait_satisfied_conditions_ne() {
    do_signal_wait_satisfied_conditions(HSA_SIGNAL_CONDITION_NE, 1, 2);
    return;
}

void signal_wait_satisfied_conditions_lt() {
    do_signal_wait_satisfied_conditions(HSA_SIGNAL_CONDITION_LT, 1, 2);
    return;
}

void signal_wait_satisfied_conditions_gte() {
    do_signal_wait_satisfied_conditions(HSA_SIGNAL_CONDITION_GTE, 2, 2);
    do_signal_wait_satisfied_conditions(HSA_SIGNAL_CONDITION_GTE, 2, 1);
    return;
}

int test_signal_wait_satisfied_conditions() {
    // Do NOT set the "set_value" or "wait_value" to 0.
    // Signal values are initialized to 0.
    signal_wait_satisfied_conditions_eq();
    signal_wait_satisfied_conditions_ne();
    signal_wait_satisfied_conditions_lt();
    signal_wait_satisfied_conditions_gte();
    return 0;
}
