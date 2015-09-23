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
 * Test Name: signal_xor_atomic
 *
 * Purpose:
 * Verify atomicity feature of signal operation
 *
 * Description:
 *
 * 1) Create a signal.
 *     Create 4 threads, that
 *     call hsa_signal_xor_acquire, the first to 0, the second to all bits set,
 *     the third with alternating 1's and 0's with 0 in the first bit and
 *     the fourth with alternating 1's and 0's with 1 in the first bit.
 *     Run the threads for millions of iterations of xors, with no
 *     explicit synchronization between the threads.
 *
 * 2) Create a signal.
 *     Create 4 threads, that
 *     call hsa_signal_xor_release, the first to 0, the second to all bits set,
 *     the third with alternating 1's and 0's with 0 in the first bit and
 *     the fourth with alternating 1's and 0's with 1 in the first bit.
 *     Run the threads for millions of iterations of xors, with no
 *     explicit synchronization between the threads.
 *
 * 3) Create a signal.
 *     Create 4 threads, that
 *     call hsa_signal_xor_relaxed, the first to 0, the second to all bits set,
 *     the third with alternating 1's and 0's with 0 in the first bit and
 *     the fourth with alternating 1's and 0's with 1 in the first bit.
 *     Run the threads for millions of iterations of xors, with no
 *     explicit synchronization between the threads.
 *
 * 4) Create a signal.
 *     Create 4 threads, that
 *     call hsa_signal_xor_acq_rel, the first to 0, the second to all bits set,
 *     the third with alternating 1's and 0's with 0 in the first bit and
 *     the fourth with alternating 1's and 0's with 1 in the first bit.
 *     Run the threads for millions of iterations of xors, with no
 *     explicit synchronization between the threads.
 *
 */

#include <hsa.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>
#include "config.h"

typedef struct test_group test_group;

typedef enum OP_TYPE_T {
    OP_TYPE_ACQUIRE,
    OP_TYPE_ACQ_REL,
    OP_TYPE_RELEASE,
    OP_TYPE_RELAXED
} OP_TYPE_T;

#ifdef HSA_LARGE_MODEL
    #define NO_BITS    0x0000000000000000
    #define ALL_BITS   0xffffffffffffffff
    #define ALT_BITS_1 0x5555555555555555
    #define ALT_BITS_2 0xaaaaaaaaaaaaaaaa
#else
    #define NO_BITS    0x00000000
    #define ALL_BITS   0xffffffff
    #define ALT_BITS_1 0x55555555
    #define ALT_BITS_2 0xaaaaaaaa
#endif

// Define a structure to pass parameter to child function
typedef struct {
    hsa_signal_t signal_handle;
    int num;
    OP_TYPE_T type;
} param;

static void child_func(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    param* param_ptr = (param*)data;
    hsa_signal_t signal_handle = param_ptr->signal_handle;
    int num = param_ptr->num;
    OP_TYPE_T type = param_ptr->type;

    // Different thread behaves differently
    hsa_signal_value_t value, signal_value;
    switch (num) {
    case 0: {
        signal_value = NO_BITS;
        break;
    }
    case 1: {
        signal_value = ALL_BITS;
        break;
    }
    case 2: {
        signal_value = ALT_BITS_1;
        break;
    }
    case 3: {
        signal_value = ALT_BITS_2;
        break;
    }
    default:
            ASSERT(num < 4);
    }

    int ii;
    switch (type) {
    case OP_TYPE_ACQUIRE : {
        for (ii = 0; ii < OP_COUNT; ++ii) {
            hsa_signal_xor_acquire(signal_handle, signal_value);
        }
    }
    case OP_TYPE_ACQ_REL : {
        for (ii = 0; ii < OP_COUNT; ++ii) {
            hsa_signal_xor_acq_rel(signal_handle, signal_value);
        }
    }
    case OP_TYPE_RELEASE : {
        for (ii = 0; ii < OP_COUNT; ++ii) {
            hsa_signal_xor_release(signal_handle, signal_value);
        }
    }
    case OP_TYPE_RELAXED : {
        for (ii = 0; ii < OP_COUNT; ++ii) {
            hsa_signal_xor_relaxed(signal_handle, signal_value);
        }
    }
    }

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

int test_signal_xor_atomic_acquire() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of 4
    test_group* group_ptr = NULL;
    group_ptr = test_group_create(4);
    ASSERT(NULL != group_ptr);

    int ii;
    // Set parameter structure for each thread
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
        (param_ptr+ii)->type = OP_TYPE_ACQUIRE;
    }

    // Add tests
    for (ii = 0; ii < 4; ++ii)
        test_group_add(group_ptr, child_func, param_ptr+ii, 1);
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running
    test_group_start(group_ptr);
    test_group_wait(group_ptr);

    // Here, we can use load acquire or relaxed, but store must be release to make sure signal is set to 0
    hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
    hsa_signal_store_release(signal_handle, 0);
    ASSERT_MSG(NO_BITS == loaded_value ||
               ALL_BITS == loaded_value ||
               ALT_BITS_1 == loaded_value ||
               ALT_BITS_2 == loaded_value,
               "Signal value is not what is expected!\n");

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

int test_signal_xor_atomic_release() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of 4
    test_group* group_ptr = NULL;
    group_ptr = test_group_create(4);
    ASSERT(NULL != group_ptr);

    int ii;
    // Set parameter structure for each thread
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
        (param_ptr+ii)->type = OP_TYPE_RELEASE;
    }

    // Add tests
    for (ii = 0; ii < 4; ++ii)
        test_group_add(group_ptr, child_func, param_ptr+ii, 1);
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running
    test_group_start(group_ptr);
    test_group_wait(group_ptr);

    // Here, we can use load acquire or relaxed, but store must be release to make sure signal is set to 0
    hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
    hsa_signal_store_release(signal_handle, 0);
    ASSERT_MSG(NO_BITS == loaded_value ||
               ALL_BITS == loaded_value ||
               ALT_BITS_1 == loaded_value ||
               ALT_BITS_2 == loaded_value,
               "Signal value is not what is expected!\n");

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

int test_signal_xor_atomic_relaxed() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of 4
    test_group* group_ptr = NULL;
    group_ptr = test_group_create(4);
    ASSERT(NULL != group_ptr);

    int ii;
    // Set parameter structure for each thread
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
        (param_ptr+ii)->type = OP_TYPE_RELAXED;
    }

    // Add tests
    for (ii = 0; ii < 4; ++ii)
        test_group_add(group_ptr, child_func, param_ptr+ii, 1);
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running
    test_group_start(group_ptr);
    test_group_wait(group_ptr);

    // Here, we can use load acquire or relaxed, but store must be release to make sure signal is set to 0
    hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
    hsa_signal_store_release(signal_handle, 0);
    ASSERT_MSG(NO_BITS == loaded_value ||
               ALL_BITS == loaded_value ||
               ALT_BITS_1 == loaded_value ||
               ALT_BITS_2 == loaded_value,
               "Signal value is not what is expected!\n");

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

int test_signal_xor_atomic_acq_rel() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of 4
    test_group* group_ptr = NULL;
    group_ptr = test_group_create(4);
    ASSERT(NULL != group_ptr);

    int ii;
    // Set parameter structure for each thread
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
        (param_ptr+ii)->type = OP_TYPE_ACQ_REL;
    }

    // Add tests
    for (ii = 0; ii < 4; ++ii)
        test_group_add(group_ptr, child_func, param_ptr+ii, 1);
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running
    test_group_start(group_ptr);
    test_group_wait(group_ptr);

    // Here, we can use load acquire or relaxed, but store must be release to make sure signal is set to 0
    hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
    hsa_signal_store_release(signal_handle, 0);
    ASSERT_MSG(NO_BITS == loaded_value ||
               ALL_BITS == loaded_value ||
               ALT_BITS_1 == loaded_value ||
               ALT_BITS_2 == loaded_value,
               "Signal value is not what is expected!\n");

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
