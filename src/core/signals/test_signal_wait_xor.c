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
*
* Test Name: signal_wait_or
*
* Purpose:
* Verify atomicity feature of signal operation
*
* Description:
*
* 1) Create a signal with an initial value of 0.
*    Create NUM_THREAD threads
*    Each thread should call hsa_signal_wait_acquire that requires
*    a signal value that won't awake any of the other threads.
*    Each thread should wait on the signal with that condition.
*    In the main thread, use the various flavors of hsa_signal_or
*    to satisfy those conditions, one at a time.
*    For each modification of the signal value, check to see if the
*    appropriate thread, and only the appropriate thread, finished
*    waiting.
*
* 2) Create a signal with an initial value of 0.
*    Create NUM_THREAD threads
*    Each thread should call hsa_signal_wait_relaxed that requires
*    a signal value that won't awake any of the other threads.
*    Each thread should wait on the signal with that condition.
*    In the main thread, use the various flavors of hsa_signal_or
*    to satisfy those conditions, one at a time.
*    For each modification of the signal value, check to see if the
*    appropriate thread, and only the appropriate thread, finished
*    waiting.
*/

#include <hsa.h>
#include "config.h"
#include "test_signal_wait_utils.h"

hsa_signal_value_t xor_initial_val() {
    return (hsa_signal_value_t) (uint64_t) 0;
}

hsa_signal_value_t xor_wakeup_val(int indx) {
    hsa_signal_value_t value = 0;
    int ii;
    for (ii = 0; ii <= indx; ++ii) {
        value |= (1 << ii);
    }
    return value;
}

hsa_signal_value_t xor_set_val(int indx) {
    return (hsa_signal_value_t) (1 << indx);
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

int test_signal_wait_acquire_xor() {
    // Test the various xor signal operations with wait acquire
    signal_wait_test_v2(hsa_signal_xor_acquire, signal_wait_acquire_test, xor_initial_val, xor_wakeup_val, xor_set_val, NUM_THREADS);
    signal_wait_test_v2(hsa_signal_xor_release, signal_wait_acquire_test, xor_initial_val, xor_wakeup_val, xor_set_val, NUM_THREADS);
    signal_wait_test_v2(hsa_signal_xor_relaxed, signal_wait_acquire_test, xor_initial_val, xor_wakeup_val, xor_set_val, NUM_THREADS);
    signal_wait_test_v2(hsa_signal_xor_acq_rel, signal_wait_acquire_test, xor_initial_val, xor_wakeup_val, xor_set_val, NUM_THREADS);
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

int test_signal_wait_relaxed_xor() {
    // Test the various xor signal operations with wait relaxed
    signal_wait_test_v2(hsa_signal_xor_acquire, signal_wait_relaxed_test, xor_initial_val, xor_wakeup_val, xor_set_val, NUM_THREADS);
    signal_wait_test_v2(hsa_signal_xor_release, signal_wait_relaxed_test, xor_initial_val, xor_wakeup_val, xor_set_val, NUM_THREADS);
    signal_wait_test_v2(hsa_signal_xor_relaxed, signal_wait_relaxed_test, xor_initial_val, xor_wakeup_val, xor_set_val, NUM_THREADS);
    signal_wait_test_v2(hsa_signal_xor_acq_rel, signal_wait_relaxed_test, xor_initial_val, xor_wakeup_val, xor_set_val, NUM_THREADS);
    return 0;
}
