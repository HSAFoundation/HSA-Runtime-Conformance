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
 * Test Name: signal_create_initial_value
 * Scope: Conformance
 *
 * Purpose: Verifies that when signals are created the initial value
 * specified in the API sets the signals value.
 *
 * Test Description:
 * 1) Create a signal, specifying a positive initial signal value.
 * 2) Query the value with an appropriate hsa_signal_load API call.
 * 3) Repeat this using a signal value of 0.
 * 4) Repeat using a signal value that is negative.
 *
 * Expected Results: All of the signals should be created successfully,
 * and the initial value should be properly set.
 */

#include <hsa.h>
#include <framework.h>

int test_signal_create_initial_value() {
    hsa_status_t status;
    hsa_signal_t signal;
    hsa_signal_value_t signal_value;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    // create a signal with initial signal value 1
    status = hsa_signal_create(1, 0, NULL, &signal);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // load signal value, and check if the return value equals to 1
    signal_value = hsa_signal_load_acquire(signal);
    ASSERT(signal_value == 1);

    status = hsa_signal_destroy(signal);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // create a signal with initial signal value 0
    status = hsa_signal_create(0, 0, NULL, &signal);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // load signal value, and check if the return value equals to 0
    signal_value = hsa_signal_load_acquire(signal);
    ASSERT(signal_value == 0);

    status = hsa_signal_destroy(signal);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // create a signal with initial signal value -1
    status = hsa_signal_create(-1, 0, NULL, &signal);
    ASSERT(status == HSA_STATUS_SUCCESS);

    // load signal value, and check if the return value equals to -1
    signal_value = hsa_signal_load_acquire(signal);
    ASSERT(signal_value == -1);

    status = hsa_signal_destroy(signal);
    ASSERT(status == HSA_STATUS_SUCCESS);

    return 0;
}
