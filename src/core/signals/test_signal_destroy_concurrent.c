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
 * Test Name: signal_create_concurrent
 * Scope: Conformance
 *
 * Purpose: Verifies that signals can be created concurrently in different
 * threads.
 *
 * Test Description:
 * 1) Start N threads that each
 *   a) Create M signals, that are maintained in a global list.
 *   b) When creating the symbols specify all agents as consumers.
 * 2) After the signals have been created, have each agent wait on
 *    each of the signals. All agents should wait on a signal concurrently
 *    and all signals in the signal list should be waited on one at a time.
 * 3) Set the signal values in another thread so the waiting agents wake
 *    up, as expected.
 * 4) Destroy all of the signals in the main thread.
 *
 *   Expected Results: All of the signals should be created successfully.
 *   All
 *   agents should be able to wait on all of the N*M threads successfully.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>

hsa_signal_t *signals;

#define INI_VAL 0

#define N 8
#define M 32

void signal_destroy_func(void *data) {
    hsa_status_t status;
    int offset = (*(int *)data);
    int ii;
    const char *err_str;
    for (ii = 0; ii < M; ii++) {
        status = hsa_signal_destroy(signals[offset + ii]);
        ASSERT_MSG(status == HSA_STATUS_SUCCESS, "\nErr_code: %d Err_string: %s\n", status, err_str);
    }
    return;
}

int test_signal_destroy_concurrent() {
    int ii;
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    signals = (hsa_signal_t *)malloc(sizeof(hsa_signal_t) * N * M);

    struct test_group *tg_sg_destroy = test_group_create(N);
    int *offset = (int *)malloc(sizeof(int) * N);

    for (ii = 0; ii < N; ++ii) {
        int jj;
        offset[ii] = ii * M;
        for (jj = 0; jj < M; ++jj) {
            status = hsa_signal_create(INI_VAL, 0, NULL, &signals[ii * M + jj]);
            ASSERT(status == HSA_STATUS_SUCCESS);
        }
    }

    for (ii = 0; ii < N; ++ii) {
        test_group_add(tg_sg_destroy, &signal_destroy_func, offset + ii, 1);
    }

    test_group_thread_create(tg_sg_destroy);
    test_group_start(tg_sg_destroy);
    test_group_wait(tg_sg_destroy);
    test_group_exit(tg_sg_destroy);
    test_group_destroy(tg_sg_destroy);

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free(signals);
    free(offset);

    return 0;
}
