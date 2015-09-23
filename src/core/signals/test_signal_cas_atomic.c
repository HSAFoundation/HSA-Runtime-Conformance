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
 * Test Name: signal_cas_atomic
 *
 * Purpose:
 * Verify atomicity feature of signal operation
 *
 * Description:
 *
 * 1) Init HsaRt, create an new signal, and initialize it to 0.
 *    Create 4 threads, that
 *    T1 call hsa_signal_cas_acquire, compare with 0 and set to 1
 *    T2 call hsa_signal_cas_acquire, compare with 0 and set to 2
 *    T3 call hsa_signal_cas_acquire, compare with 0 and set to 3
 *    T4 call hsa_signal_cas_acquire, compare with 0 and set to 4
 *    Check if only one thread returns 0 and load signal value check
 *    if the value equal to what is set by corresponding thread.
 *
 * 2) Init HsaRt, create an new signal, and initialize it to 0.
 *    Create 4 threads, that
 *    T1 call hsa_signal_cas_release, compare with 0 and set to 1
 *    T2 call hsa_signal_cas_release, compare with 0 and set to 2
 *    T3 call hsa_signal_cas_release, compare with 0 and set to 3
 *    T4 call hsa_signal_cas_release, compare with 0 and set to 4
 *    Check if only one thread returns 0 and load signal value check
 *    if the value equal to what is set by corresponding thread.
 *
 * 3) Init HsaRt, create an new signal, and initialize it to 0.
 *    Create 4 threads, that
 *    T1 call hsa_signal_cas_relaxed, compare with 0 and set to 1
 *    T2 call hsa_signal_cas_relaxed, compare with 0 and set to 2
 *    T3 call hsa_signal_cas_relaxed, compare with 0 and set to 3
 *    T4 call hsa_signal_cas_relaxed, compare with 0 and set to 4
 *    Check if only one thread returns 0 and load signal value check
 *    if the value equal to what is set by corresponding thread.
 *
 * 4) Init HsaRt, create an new signal, and initialize it to 0.
 *    Create 4 threads, that
 *    T1 call hsa_signal_cas_acq_rel, compare with 0 and set to 1
 *    T2 call hsa_signal_cas_acq_rel, compare with 0 and set to 2
 *    T3 call hsa_signal_cas_acq_rel, compare with 0 and set to 3
 *    T4 call hsa_signal_cas_acq_rel, compare with 0 and set to 4
 *    Check if only one thread returns 0 and load signal value check
 *    if the value equal to what is set by corresponding thread.
 *
 */

#include <hsa.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "config.h"

typedef struct test_group test_group;

// Define a structure to pass parameter to child function
typedef struct {
    volatile hsa_signal_t signal_handle;
    volatile int num;
    volatile int* retval;
} param;

static void child_func_acquire(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    param* param_ptr = (param*)data;
    hsa_signal_t signal_handle = param_ptr->signal_handle;
    int num = param_ptr->num;

    *(param_ptr->retval) = hsa_signal_cas_acquire(signal_handle, 0, num+1);

    return;
}

static void child_func_release(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    param* param_ptr = (param*)data;
    hsa_signal_t signal_handle = param_ptr->signal_handle;
    int num = param_ptr->num;

    *(param_ptr->retval) = hsa_signal_cas_release(signal_handle, 0, num+1);

    return;
}

static void child_func_relaxed(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    param* param_ptr = (param*)data;
    hsa_signal_t signal_handle = param_ptr->signal_handle;
    int num = param_ptr->num;

    *(param_ptr->retval) = hsa_signal_cas_relaxed(signal_handle, 0, num+1);

    return;
}

static void child_func_acq_rel(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    param* param_ptr = (param*)data;
    hsa_signal_t signal_handle = param_ptr->signal_handle;
    int num = param_ptr->num;

    *(param_ptr->retval) = hsa_signal_cas_acq_rel(signal_handle, 0, num+1);

    return;
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

int test_signal_cas_atomic_acquire() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    // Set all bits of initial value to 0
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of 4
    test_group* group_ptr = NULL;
    group_ptr = test_group_create(4);
    ASSERT(NULL != group_ptr);
    int ii;
    int jj;
    // Set parameter structure for each thread
    volatile int retval[4];
    for (ii = 0; ii < 4; ++ii)
        retval[ii] = 1000; // set to none 1, none 2, none 3, none 4
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
        (param_ptr+ii)->retval = retval+ii;
    }
    // Add tests
    for (ii = 0; ii < 4; ++ii) {
        test_group_add(group_ptr, child_func_acquire, param_ptr+ii, 1);
    }
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running for OP_COUNT times
    for (ii = 0; ii < OP_COUNT; ++ii) {
        test_group_start(group_ptr);
        test_group_wait(group_ptr);
        // Here, check if only on thread return 0 when calling cas API
        int num_zero = 0, index;
        for (jj = 0; jj < 4; ++jj) {
            if (retval[jj] == 0) {
                num_zero++;
                index = jj+1;
            }
        }
        ASSERT_MSG(1 == num_zero, "Only one zero should be observed!\n");
        // Load signal value, check if value equal to index
        hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
        hsa_signal_store_release(signal_handle, 0);
        ASSERT_MSG(index == loaded_value, "Failed to perform CAS successfully!\n");
    }
    // Exit current test group
    test_group_exit(group_ptr);
    test_group_destroy(group_ptr);
    // Destroy the resource
    status = hsa_signal_destroy(signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

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

int test_signal_cas_atomic_release() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    // Set all bits of initial value to 0
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of 4
    test_group* group_ptr = NULL;
    group_ptr = test_group_create(4);
    ASSERT(NULL != group_ptr);
    int ii;
    int jj;
    // Set parameter structure for each thread
    volatile int retval[4];
    for (ii = 0; ii < 4; ++ii)
        retval[ii] = 1000; // set to none 1, none 2, none 3, none 4
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
        (param_ptr+ii)->retval = retval+ii;
    }
    // Add tests
    for (ii = 0; ii < 4; ++ii) {
        test_group_add(group_ptr, child_func_release, param_ptr+ii, 1);
    }
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running for OP_COUNT times
    for (ii = 0; ii < OP_COUNT; ++ii) {
        test_group_start(group_ptr);
        test_group_wait(group_ptr);
        // Here, check if only on thread return 0 when calling cas API
        int num_zero = 0, index;
        for (jj = 0; jj < 4; ++jj) {
            if (retval[jj] == 0) {
                num_zero++;
                index = jj+1;
            }
        }
        ASSERT_MSG(1 == num_zero, "Only one zero should be observed!\n");
        // Load signal value, check if value equal to index
        hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
        hsa_signal_store_release(signal_handle, 0);
        ASSERT_MSG(index == loaded_value, "Failed to perform CAS successfully!\n");
    }
        // Exit current test group
    test_group_exit(group_ptr);
    test_group_destroy(group_ptr);
    // Destroy the resource
    status = hsa_signal_destroy(signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

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

int test_signal_cas_atomic_relaxed() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    // Set all bits of initial value to 0
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of 4
    test_group* group_ptr = NULL;
    group_ptr = test_group_create(4);
    ASSERT(NULL != group_ptr);
    int ii;
    int jj;
    // Set parameter structure for each thread
    volatile int retval[4];
    for (ii = 0; ii < 4; ++ii)
        retval[ii] = 1000;
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
        (param_ptr+ii)->retval = retval+ii;
    }
    // Add tests
    for (ii = 0; ii < 4; ++ii) {
        test_group_add(group_ptr, child_func_relaxed, param_ptr+ii, 1);
    }
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running for OP_COUNT times
    for (ii = 0; ii < OP_COUNT; ++ii) {
        test_group_start(group_ptr);
        test_group_wait(group_ptr);
        // Here, check if only on thread return 0 when calling cas API
        int num_zero = 0, index;
        for (jj = 0; jj < 4; ++jj) {
            if (retval[jj] == 0) {
                num_zero++;
                index = jj+1;
            }
        }
            ASSERT_MSG(1 == num_zero, "Only one zero should be observed!\n");
            // Load signal value, check if value equal to index
            hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
            hsa_signal_store_release(signal_handle, 0);
            ASSERT_MSG(index == loaded_value, "Failed to perform CAS successfully!\n");
    }
        // Exit current test group
    test_group_exit(group_ptr);
    test_group_destroy(group_ptr);
    // Destroy the resource
    status = hsa_signal_destroy(signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

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

int test_signal_cas_atomic_acq_rel() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    // Set all bits of initial value to 0
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of 4
    test_group* group_ptr = NULL;
    group_ptr = test_group_create(4);
    ASSERT(NULL != group_ptr);
    int ii;
    int jj;
    // Set parameter structure for each thread
    volatile int retval[4];
    for (ii = 0; ii < 4; ++ii)
        retval[ii] = 1000;
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
        (param_ptr+ii)->retval = retval+ii;
    }
    // Add tests
    for (ii = 0; ii < 4; ++ii) {
        test_group_add(group_ptr, child_func_acq_rel, param_ptr+ii, 1);
    }
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running for OP_COUNT times
    for (ii = 0; ii < OP_COUNT; ++ii) {
        test_group_start(group_ptr);
        test_group_wait(group_ptr);
        // Here, check if only on thread return 0 when calling cas API
        int num_zero = 0, index;
        for (jj = 0; jj < 4; ++jj) {
            if (retval[jj] == 0) {
                num_zero++;
                index = jj+1;
            }
        }
        ASSERT_MSG(1 == num_zero, "Only one zero should be observed!\n");
        // Load signal value, check if value equal to index
        hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
        hsa_signal_store_release(signal_handle, 0);
        ASSERT_MSG(index == loaded_value, "Failed to perform CAS successfully!\n");
    }
        // Exit current test group
    test_group_exit(group_ptr);
    test_group_destroy(group_ptr);
    // Destroy the resource
    status = hsa_signal_destroy(signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
