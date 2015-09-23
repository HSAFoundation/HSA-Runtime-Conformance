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
 * Test Name: signal_add_atomic
 *
 * Purpose:
 * Verify atomicity of the add signal operation
 *
 * Description:
 *
 * 1) Create a signal, assigning it an initial value of 0.
 *    Create several threads, which call hsa_signal_add_acquire
 *    in a loop to add 1 to signal repeatedly.
 *    After threads finish, check if the value is correct, and
 *    repeat this process several times.
 *
 * 2) Create a signal, assigning it an initial value of 0.
 *    Create several threads, which call hsa_signal_add_release
 *    in a loop to add 1 to signal repeatedly.
 *    After threads finish, check if the value is correct, and
 *    repeat this process several times.
 *
 * 3) Create a signal, assigning it an initial value of 0.
 *    Create several threads, which call hsa_signal_add_relaxed
 *    in a loop to add 1 to signal repeatedly.
 *    After threads finish, check if the value is correct, and
 *    repeat this process several times.
 *
 * 4) Create a signal, assigning it an initial value of 0.
 *    Create several threads, which call hsa_signal_add_acq_rel
 *    in a loop to add 1 to signal repeatedly.
 *    After threads finish, check if the value is correct, and
 *    repeat this process several times.
 *
 */

#include<hsa.h>
#include<concurrent_utils.h>
#include<framework.h>
#include "config.h"

static void child_func_acquire(void* data) {
// Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    hsa_signal_t* signal_handle = (hsa_signal_t*)data;

    // Call hsa_signal_add_acquire in a loop
    int ii;
    for (ii = 0; ii < OP_COUNT; ++ii) {
        hsa_signal_add_acquire(*signal_handle, 1);
    }

    return;
}

static void child_func_release(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    hsa_signal_t* signal_handle = (hsa_signal_t*)data;

    // Call hsa_signal_add_acquire in a loop
    int ii;
    for (ii = 0; ii < OP_COUNT; ++ii) {
        hsa_signal_add_release(*signal_handle, 1);
    }

    return;
}

static void child_func_relaxed(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    hsa_signal_t* signal_handle = (hsa_signal_t*)data;

    // Call hsa_signal_add_acquire in a loop
    int ii;
    for (ii = 0; ii < OP_COUNT; ++ii) {
        hsa_signal_add_relaxed(*signal_handle, 1);
    }

    return;
}

static void child_func_acq_rel(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    hsa_signal_t* signal_handle = (hsa_signal_t*)data;

    // Call hsa_signal_add_acquire in a loop
    int ii;
    for (ii = 0; ii < OP_COUNT; ++ii) {
        hsa_signal_add_acq_rel(*signal_handle, 1);
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

int test_signal_add_atomic_acquire() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of GROUP_SIZE
    struct test_group* group_ptr = NULL;
    group_ptr = test_group_create(GROUP_SIZE);
    ASSERT(NULL != group_ptr);
    // Add tests
    test_group_add(group_ptr, child_func_acquire, &signal_handle, GROUP_SIZE);
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running for 10 times
    int ii;
    for (ii = 0; ii < TEST_COUNT; ++ii) {
        test_group_start(group_ptr);
        test_group_wait(group_ptr);
        // Here, we can use load acquire or relaxed, but store must be release to make sure signal is set to 0
        hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
        hsa_signal_store_release(signal_handle, 0);
        ASSERT_MSG((GROUP_SIZE * OP_COUNT) == loaded_value, "Signal value is not the expected value.\n");
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

int test_signal_add_atomic_release() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of GROUP_SIZE
    struct test_group* group_ptr = NULL;
    group_ptr = test_group_create(GROUP_SIZE);
    ASSERT(NULL != group_ptr);
    // Add tests
    test_group_add(group_ptr, child_func_release, &signal_handle, GROUP_SIZE);
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running for 10 times
    int ii;
    for (ii = 0; ii < TEST_COUNT; ++ii) {
        test_group_start(group_ptr);
        test_group_wait(group_ptr);
        // Here, we can use load acquire or relaxed, but store must be release to make sure signal is set to 0
        hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
        hsa_signal_store_release(signal_handle, 0);
        ASSERT_MSG((GROUP_SIZE * OP_COUNT) == loaded_value, "Signal value is not the expected value.\n");
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

int test_signal_add_atomic_relaxed() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of GROUP_SIZE
    struct test_group* group_ptr = NULL;
    group_ptr = test_group_create(GROUP_SIZE);
    ASSERT(NULL != group_ptr);
    // Add tests
    test_group_add(group_ptr, child_func_relaxed, &signal_handle, GROUP_SIZE);
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running for 10 times
    int ii;
    for (ii = 0; ii < TEST_COUNT; ++ii) {
        test_group_start(group_ptr);
        test_group_wait(group_ptr);
        // Here, we can use load acquire or relaxed, but store must be release to make sure signal is set to 0
        hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
        hsa_signal_store_release(signal_handle, 0);
        ASSERT_MSG((GROUP_SIZE * OP_COUNT) == loaded_value, "Signal value is not the expected value.\n");
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

int test_signal_add_atomic_acq_rel() {
    hsa_status_t status;

    // Open HsaRt
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_t signal_handle;
    hsa_signal_value_t initial_value = 0;

    // Create an new signal
    status = hsa_signal_create(initial_value, 0, NULL, &signal_handle);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create test group with size of GROUP_SIZE
    struct test_group* group_ptr = NULL;
    group_ptr = test_group_create(GROUP_SIZE);
    ASSERT(NULL != group_ptr);
    // Add tests
    test_group_add(group_ptr, child_func_acq_rel, &signal_handle, GROUP_SIZE);
    // Create threads for tests
    test_group_thread_create(group_ptr);
    // Set run flag to let multi-thread running for 10 times
    int ii;
    for (ii = 0; ii < TEST_COUNT; ++ii) {
        test_group_start(group_ptr);
        test_group_wait(group_ptr);
        // Here, we can use load acquire or relaxed, but store must be release to make sure signal is set to 0
        hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
        hsa_signal_store_release(signal_handle, 0);
        ASSERT_MSG((GROUP_SIZE * OP_COUNT) == loaded_value, "Signal value is not the expected value.\n");
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
