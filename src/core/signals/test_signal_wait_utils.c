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
#include <pthread.h>

typedef struct {
    hsa_signal_t signal_handle;
    volatile int num;
    int* flag;
} param;

void* signal_wait_acquire_test(void* arg) {
    param* param_ptr = (param*)arg;
    hsa_signal_value_t signal_value = param_ptr->num;
    hsa_signal_t signal_handle = param_ptr->signal_handle;
    int* flag = param_ptr->flag;

    // Wait on signal with memory ordering of acquire
    hsa_signal_wait_acquire(signal_handle, HSA_SIGNAL_CONDITION_EQ, signal_value, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    // Set flag to indicate the thread has waken up
    *flag = 1;

    return NULL;
}

void* signal_wait_relaxed_test(void* arg) {
    param* param_ptr = (param*)arg;
    hsa_signal_value_t signal_value = param_ptr->num;
    hsa_signal_t signal_handle = param_ptr->signal_handle;
    int* flag = param_ptr->flag;

    // Wait on signal with memory ordering of relaxed
    hsa_signal_wait_relaxed(signal_handle, HSA_SIGNAL_CONDITION_EQ, signal_value, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    // Set flag to indicate the thread has waken up
    *flag = 1;

    return NULL;
}

int signal_wait_test_v1(hsa_signal_value_t (*signal_func)(hsa_signal_t signal,
                     hsa_signal_value_t expected, hsa_signal_value_t value),
                     void* (*wait_test)(void* arg),
                     hsa_signal_value_t (*initial_val)(),
                     hsa_signal_value_t (*wakeup_val)(int index),
                     hsa_signal_value_t (*expect_val)(int index),
                     hsa_signal_value_t (*set_val)(int index),
                     int num_threads) {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = initial_val();
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Prepare data for threads
    pthread_t id[num_threads];
    param arg[num_threads];
    int flag[num_threads];

    int ii;
    for (ii = 0; ii < num_threads; ++ii) {
        flag[ii] = 0;
        arg[ii].signal_handle = signal_handle;
        arg[ii].num = wakeup_val(ii);
        arg[ii].flag = &flag[ii];
        pthread_create(&id[ii], NULL, wait_test, &arg[ii]);
    }

    // Set signal value to wake up specific thread
    for (ii = 0; ii < num_threads; ++ii) {
        hsa_signal_value_t value = signal_func(signal_handle, expect_val(ii), set_val(ii));
        hsa_signal_value_t expect = expect_val(ii);
        ASSERT(expect == value);
        pthread_join(id[ii], NULL);
        // Check the flag data to make sure just one thread wakes up
        int jj;
        for (jj = 0; jj < num_threads; ++jj) {
            if (jj <= ii) {
                ASSERT(1 == flag[jj]);
            } else {
                ASSERT(0 == flag[jj]);
            }
        }
    }

    // Check if all of the flag has been set to 1
    for (ii = 0; ii < num_threads; ++ii) {
        ASSERT(1 == flag[ii]);
    }

    status = hsa_signal_destroy(signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

int signal_wait_test_v2(void (*signal_func)(hsa_signal_t signal, hsa_signal_value_t value),
                     void* (*wait_test)(void* arg),
                     hsa_signal_value_t (*initial_val)(),
                     hsa_signal_value_t (*wakeup_val)(int index),
                     hsa_signal_value_t (*set_val)(int index),
                     int num_threads) {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = initial_val();
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Prepare data for threads
    pthread_t id[num_threads];
    param arg[num_threads];
    int flag[num_threads];

    int ii;
    for (ii = 0; ii < num_threads; ++ii) {
        flag[ii] = 0;
        arg[ii].signal_handle = signal_handle;
        arg[ii].num = wakeup_val(ii);
        arg[ii].flag = &flag[ii];
        pthread_create(&id[ii], NULL, wait_test, &arg[ii]);
    }

    // Set signal value to wake up specific thread
    for (ii = 0; ii < num_threads; ++ii) {
        hsa_signal_value_t value = set_val(ii);
        signal_func(signal_handle, value);
        pthread_join(id[ii], NULL);
        // Check the flag data to make sure just one thread wakes up
        int jj;
        for (jj = 0; jj < num_threads; ++jj) {
            if (jj <= ii) {
                ASSERT(1 == flag[jj]);
            } else {
                ASSERT(0 == flag[jj]);
            }
        }
    }

    // Check if all of the flag has been set to 1
    for (ii = 0; ii < num_threads; ++ii) {
        ASSERT(1 == flag[ii]);
    }

    status = hsa_signal_destroy(signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

int signal_wait_test_v3(hsa_signal_value_t (*signal_func)(hsa_signal_t signal,
                     hsa_signal_value_t value),
                     void* (*wait_test)(void* arg),
                     hsa_signal_value_t (*initial_val)(),
                     hsa_signal_value_t (*wakeup_val)(int index),
                     hsa_signal_value_t (*expect_val)(int index),
                     hsa_signal_value_t (*set_val)(int index),
                     int num_threads) {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = initial_val();
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Prepare data for threads
    pthread_t id[num_threads];
    param arg[num_threads];
    int flag[num_threads];

    int ii;
    for (ii = 0; ii < num_threads; ++ii) {
        flag[ii] = 0;
        arg[ii].signal_handle = signal_handle;
        arg[ii].num = wakeup_val(ii);
        arg[ii].flag = &flag[ii];
        pthread_create(&id[ii], NULL, wait_test, &arg[ii]);
    }

    // Set signal value to wake up specific thread
    for (ii = 0; ii < num_threads; ++ii) {
        hsa_signal_value_t value = signal_func(signal_handle, set_val(ii));
        hsa_signal_value_t expect = expect_val(ii);
        ASSERT(expect == value);
        pthread_join(id[ii], NULL);
        // Check the flag data to make sure just one thread wakes up
        int jj;
        for (jj = 0; jj < num_threads; ++jj) {
            if (jj <= ii) {
                ASSERT(1 == flag[jj]);
            } else {
                ASSERT(0 == flag[jj]);
            }
        }
    }

    // Check if all of the flag has been set to 1
    for (ii = 0; ii < num_threads; ++ii) {
        ASSERT(1 == flag[ii]);
    }

    status = hsa_signal_destroy(signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
