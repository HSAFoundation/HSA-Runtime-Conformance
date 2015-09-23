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

#ifndef _TEST_SIGNAL_WAIT_UTILS_H_
#define _TEST_SIGNAL_WAIT_UTILS_H_

#include <hsa.h>

typedef struct {
    hsa_signal_t signal_handle;
    volatile int num;
    int* flag;
} param;

void* signal_wait_acquire_test(void* arg);

void* signal_wait_relaxed_test(void* arg);

int signal_wait_test_v1(hsa_signal_value_t (*signal_func)(hsa_signal_t signal,
                     hsa_signal_value_t expected, hsa_signal_value_t value),
                     void* (*wait_test)(void* arg),
                     hsa_signal_value_t (*initial_val)(),
                     hsa_signal_value_t (*wakeup_val)(int index),
                     hsa_signal_value_t (*expect_val)(int index),
                     hsa_signal_value_t (*set_val)(int index),
                     int num_threads);

int signal_wait_test_v2(void (*signal_func)(hsa_signal_t signal, hsa_signal_value_t value),
                     void* (*wait_test)(void* arg),
                     hsa_signal_value_t (*initial_val)(),
                     hsa_signal_value_t (*wakeup_val)(int index),
                     hsa_signal_value_t (*set_val)(int index),
                     int num_threads);

int signal_wait_test_v3(hsa_signal_value_t (*signal_func)(hsa_signal_t signal, hsa_signal_value_t value),
                     void* (*wait_test)(void* arg),
                     hsa_signal_value_t (*initial_val)(),
                     hsa_signal_value_t (*wakeup_val)(int index),
                     hsa_signal_value_t (*expect_val)(int index),
                     hsa_signal_value_t (*set_val)(int index),
                     int num_threads);

#endif  // _TEST_SIGNAL_WAIT_UTILS_H_
