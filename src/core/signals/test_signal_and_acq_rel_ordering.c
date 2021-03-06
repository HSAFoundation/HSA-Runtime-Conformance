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
* Test Name: signal_and_acq_rel_ordering
*
* Purpose:
* Verify that sequential memory ordering of the hsa_signal_and_acq_rel
* API is correct.
*
* Description:
*
* 1) Create several signals and store the handles in an array,
*    denoted by x[]. All the signal values should be initialized to have
*    the last bit set.
*    Create a control signal, denoted by y, also initialized to have only
*    the last bit set.
*    Start one thread that
*    a) Checks the value of y in a loop using hsa_cas_acq_rel using
*    0 as the exchange value.
*    b) When the value of y has only the last bit set, the thread stops looping, and
*    c) Checks all of the x signal values with the signal_load_relaxed
*    API, expecting all x values to have their last bit set .
*    d) Replaces the x values by using signal_load_relaxed to set all bits,
*    and then uses signal_and_relaxed to mask out all bits but the first.
*    e) Replaces the value of y by using signal_load_relaxed to set all bits,
*    and then uses signal_and_acq_rel to mask out all bits but the first.
*    f) Starts over.
*    Start a second thread that does exactly the same set of operations,
*    but sets the last bit for all values, not the first, and triggers when
*    the first bit of y is set, not the last.
*    Let both threads run for thousands iterations.
*
*/

#include <hsa.h>
#include <framework.h>
#include <pthread.h>
#include "config.h"

typedef struct {
    hsa_signal_t* signal_x;
    hsa_signal_t signal_y;
} param;

static void* T1(void* arg) {
    param* param_ptr = (param*)arg;
    hsa_signal_t* signal_x = param_ptr->signal_x;
    hsa_signal_t signal_y = param_ptr->signal_y;

    hsa_signal_value_t value;
    int j;
    for (j = 0; j < NUM_ITERATION; ++j) {
        while (1 != hsa_signal_cas_acq_rel(signal_y, 1, 0));
        int i;
        for (i = 0; i < NUM_SIGNAL; ++i) {
            value = hsa_signal_load_relaxed(signal_x[i]);
            ASSERT_MSG(1 == value, "The value of signal_x[%d] is not equal to 1!\n", i);
            // Set all bits of each x and mask out all bits but first
            hsa_signal_store_relaxed(signal_x[i], -1);
            hsa_signal_and_relaxed(signal_x[i], FIRST_BIT);
        }
        hsa_signal_store_relaxed(signal_y, -1);
        hsa_signal_and_acquire(signal_y, FIRST_BIT);
    }
    return NULL;
}

static void* T2(void* arg) {
    param* param_ptr = (param*)arg;
    hsa_signal_t* signal_x = param_ptr->signal_x;
    hsa_signal_t signal_y = param_ptr->signal_y;

    hsa_signal_value_t value;
    int j;
    for (j = 0; j < NUM_ITERATION; ++j) {
        while (FIRST_BIT != hsa_signal_cas_acq_rel(signal_y, FIRST_BIT, 0));

        int i;
        for (i = 0; i < NUM_SIGNAL; ++i) {
            value = hsa_signal_load_relaxed(signal_x[i]);
            ASSERT_MSG(FIRST_BIT == value, "The value of x[%d] is not equal to the min negative value!\n", i);
            // Set all bits of each x and mask out all bits but last
            hsa_signal_store_relaxed(signal_x[i], -1);
            hsa_signal_and_relaxed(signal_x[i], 1);
        }
        hsa_signal_store_relaxed(signal_y, -1);
        hsa_signal_and_acq_rel(signal_y, 1);
    }
    return NULL;
}

int test_signal_and_acq_rel_ordering() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_x[NUM_SIGNAL], signal_y;
    // Set value of signal_x and signal_y to 1
    hsa_signal_value_t initial_value = 1;
    int i;
    for (i = 0; i < NUM_SIGNAL; ++i) {
        status = hsa_signal_create(initial_value, 0, NULL, &signal_x[i]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }
    status = hsa_signal_create(initial_value, 0, NULL, &signal_y);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Prepare data for threads
    param arg[2];
    arg[0].signal_x = signal_x;
    arg[0].signal_y = signal_y;
    arg[1].signal_x = signal_x;
    arg[1].signal_y = signal_y;

    pthread_t id[2];
    pthread_create(&id[0], NULL, T1, &arg[0]);
    pthread_create(&id[1], NULL, T2, &arg[1]);

    pthread_join(id[0], NULL);
    pthread_join(id[1], NULL);

    // Destroy signal
    for (i = 0; i < NUM_SIGNAL; ++i) {
        status = hsa_signal_destroy(signal_x[i]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }
    status = hsa_signal_destroy(signal_y);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
