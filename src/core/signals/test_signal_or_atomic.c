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
 * Test Name: signal_or_atomic
 *
 * Purpose:
 * Verify atomicity feature of signal operation
 *
 * Description:
 *
 * 1) Create a signal, set every bit of signal to be 1, create
 *    4 threads
 *    a) Each thread applies a hsa_signal_or_acquire operation on the signal value.
 *    b) Thread 0 uses a rotating mask of ...0001, ..0001., .0001.., 0001..., shifting
 *     the 0 16 bits per application.
 *    c) Thread 1 uses a rotating mask of ...0010, ..0010., .0010.., 0010..., shifting
 *     the 0 16 bits per application.
 *    d) Thread 2 uses a rotating mask of ...0100, ..0100., .0100.., 0100..., shifting
 *     the 0 16 bits per application.
 *    e) Thread 3 uses a rotating mask of ...1000, ..1000., .1000.., 1000..., shifting
 *     the 0 16 bits per application.
 *    After all threads finish, check if the final value is 0 and repeat OP_COUNT times.
 *
 * 2) Create a signal, set every bit of signal to be 1, create
 *    4 threads
 *    a) Each thread applies a hsa_signal_or_release operation on the signal value.
 *    b) Thread 0 uses a rotating mask of ...0001, ..0001., .0001.., 0001..., shifting
 *     the 0 16 bits per application.
 *    c) Thread 1 uses a rotating mask of ...0010, ..0010., .0010.., 0010..., shifting
 *     the 0 16 bits per application.
 *    d) Thread 2 uses a rotating mask of ...0100, ..0100., .0100.., 0100..., shifting
 *     the 0 16 bits per application.
 *    e) Thread 3 uses a rotating mask of ...1000, ..1000., .1000.., 1000..., shifting
 *     the 0 16 bits per application.
 *    After all threads finish, check if the final value is 0 and repeat OP_COUNT times.
 *
 * 3) Create a signal, set every bit of signal to be 1, create
 *    4 threads
 *    a) Each thread applies a hsa_signal_or_relaxed operation on the signal value.
 *    b) Thread 0 uses a rotating mask of ...0001, ..0001., .0001.., 0001..., shifting
 *     the 0 16 bits per application.
 *    c) Thread 1 uses a rotating mask of ...0010, ..0010., .0010.., 0010..., shifting
 *     the 0 16 bits per application.
 *    d) Thread 2 uses a rotating mask of ...0100, ..0100., .0100.., 0100..., shifting
 *     the 0 16 bits per application.
 *    e) Thread 3 uses a rotating mask of ...1000, ..1000., .1000.., 1000..., shifting
 *     the 0 16 bits per application.
 *    After all threads finish, check if the final value is 0 and repeat OP_COUNT times.
 *
 * 4) Create a signal, set every bit of signal to be 1, create
 *    4 threads
 *    a) Each thread applies a hsa_signal_or_acq_rel operation on the signal value.
 *    b) Thread 0 uses a rotating mask of ...0001, ..0001., .0001.., 0001..., shifting
 *     the 0 16 bits per application.
 *    c) Thread 1 uses a rotating mask of ...0010, ..0010., .0010.., 0010..., shifting
 *     the 0 16 bits per application.
 *    d) Thread 2 uses a rotating mask of ...0100, ..0100., .0100.., 0100..., shifting
 *     the 0 16 bits per application.
 *    e) Thread 3 uses a rotating mask of ...1000, ..1000., .1000.., 1000..., shifting
 *     the 0 16 bits per application.
 *    After all threads finish, check if the final value is 0 and repeat OP_COUNT times.
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
} param;

static void child_func_acquire(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    param* param_ptr = (param*)data;
    hsa_signal_t signal_handle = param_ptr->signal_handle;
    int num = param_ptr->num;

    // Different thread behaves differently
    switch (num) {
    case 0:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x000000000000000f;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_acquire(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x0000fff0;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_acquire(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 1:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x00000000000000f0;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_acquire(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x000000f0;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_acquire(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 2:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x0000000000000f00;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_acquire(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x00000f00;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_acquire(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 3:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x000000000000f000;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_acquire(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0xffff0fff;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_acquire(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    default:
            break;
    }

    return;
}

static void child_func_release(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    param* param_ptr = (param*)data;
    hsa_signal_t signal_handle = param_ptr->signal_handle;
    int num = param_ptr->num;

    // Different thread behaves differently
    switch (num) {
    case 0:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x000000000000000f;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_release(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x0000fff0;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_release(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 1:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x00000000000000f0;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_release(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x000000f0;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_release(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 2:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x0000000000000f00;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_release(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x00000f00;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_release(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 3:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x000000000000f000;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_release(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0xffff0fff;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_release(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    default:
            break;
    }
    return;
}

static void child_func_relaxed(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    param* param_ptr = (param*)data;
    hsa_signal_t signal_handle = param_ptr->signal_handle;
    int num = param_ptr->num;

    // Different thread behaves differently
    switch (num) {
    case 0:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x000000000000000f;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_relaxed(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x0000fff0;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_relaxed(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 1:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x00000000000000f0;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_relaxed(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x000000f0;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_relaxed(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 2:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x0000000000000f00;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_relaxed(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x00000f00;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_relaxed(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 3:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x000000000000f000;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_relaxed(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0xffff0fff;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_relaxed(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    default:
            break;
    }
    return;
}

static void child_func_acq_rel(void* data) {
    // Here, main thread will make sure runtime is open before thread creation and close after thread func finish properly,
    // within thread, we will not open runtime
    param* param_ptr = (param*)data;
    hsa_signal_t signal_handle = param_ptr->signal_handle;
    int num = param_ptr->num;

    // Different thread behaves differently
    switch (num) {
    case 0:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x000000000000000f;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_acq_rel(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x0000fff0;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_acq_rel(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 1:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x00000000000000f0;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_acq_rel(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x000000f0;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_acq_rel(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 2:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x0000000000000f00;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_acq_rel(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0x00000f00;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_acq_rel(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    case 3:
        {
            hsa_signal_value_t signal_value;
            int ii;
            #ifdef HSA_LARGE_MODEL
                signal_value = 0x000000000000f000;
                for (ii = 0; ii < 4; ++ii) {
                    hsa_signal_or_acq_rel(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #else
                signal_value = 0xffff0fff;
                for (ii = 0; ii < 2; ++ii) {
                    hsa_signal_or_acq_rel(signal_handle, signal_value);
                    signal_value = signal_value << 16;
                }
            #endif
            break;
        }
    default:
            break;
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

int test_signal_or_atomic_acquire() {
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
    // Set parameter structure for each thread
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
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
        // Here, we can use load acquire or relaxed, but store must be release to make sure every bit of signal is set before next loop
        hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
        hsa_signal_store_release(signal_handle, 0);
        ASSERT_MSG(-1 == loaded_value, "Signal value is not -1 which is expected!\n");
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

int test_signal_or_atomic_release() {
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
    // Set parameter structure for each thread
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
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
        // Here, we can use load acquire or relaxed, but store must be release to make sure every bit of signal is set before next loop
        hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
        hsa_signal_store_release(signal_handle, 0);
        ASSERT_MSG(-1 == loaded_value, "Signal value is not -1 which is expected!\n");
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

int test_signal_or_atomic_relaxed() {
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
    // Set parameter structure for each thread
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
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
        // Here, we can use load acquire or relaxed, but store must be release to make sure every bit of signal is set before next loop
        hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
        hsa_signal_store_release(signal_handle, 0);
        ASSERT_MSG(-1 == loaded_value, "Signal value is not -1 which is expected!\n");
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

int test_signal_or_atomic_acq_rel() {
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
    // Set parameter structure for each thread
    param* param_ptr = (param*)malloc(sizeof(param)*4);
    for (ii = 0; ii < 4; ++ii) {
        (param_ptr+ii)->signal_handle = signal_handle;
        (param_ptr+ii)->num = ii;
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
        // Here, we can use load acquire or relaxed, but store must be release to make sure every bit of signal is set before next loop
        hsa_signal_value_t loaded_value = hsa_signal_load_relaxed(signal_handle);
        hsa_signal_store_release(signal_handle, 0);
        ASSERT_MSG(-1 == loaded_value, "Signal value is not -1 which is expected!\n");
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
