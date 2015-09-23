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
#define CMP_VAL 1

#define N 8
#define M 32 

void signal_create_func(void *data) {
    hsa_status_t status;
    int offset = (*(int *)data);
    int ii;
    const char *err_str;
    for (ii = 0; ii < M; ++ii) {
        status = hsa_signal_create(INI_VAL, 0, NULL, &signals[offset + ii]);
        ASSERT_MSG(status == HSA_STATUS_SUCCESS, "\nErr_code: %d Err_string: %s\n", status, err_str);
    }
    return;
}

void signals_wait_host_func(void *data) {
    int ii;
    hsa_agent_t *agent = (hsa_agent_t *)data;
    for (ii = 0; ii < M*N; ++ii) {
        hsa_signal_wait_acquire(signals[ii], HSA_SIGNAL_CONDITION_EQ, CMP_VAL, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    }
    return;
}

void signals_wait_component_func(void *data) {
    int ii;
    hsa_agent_t *agent = (hsa_agent_t *)data;
    for (ii = 0; ii < M*N; ++ii) {
        // Launch a kernel with signal_wait_func
        hsa_signal_wait_acquire(signals[ii], HSA_SIGNAL_CONDITION_EQ, CMP_VAL, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    }
    return;
}

int test_signal_create_concurrent() {
    int ii;
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    signals = (hsa_signal_t *)malloc(sizeof(hsa_signal_t) * N * M);

    struct test_group *tg_sg_create = test_group_create(N);
    int *offset = (int *) malloc(sizeof(int) * N);

    for (ii = 0; ii < N; ++ii) {
        offset[ii] = ii * M;
        test_group_add(tg_sg_create, &signal_create_func, offset + ii, 1);
    }

    test_group_thread_create(tg_sg_create);
    test_group_start(tg_sg_create);
    test_group_wait(tg_sg_create);
    test_group_exit(tg_sg_create);
    test_group_destroy(tg_sg_create);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    struct test_group *tg_sg_wait = test_group_create(agent_list.num_agents);
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        hsa_device_type_t device_type;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_DEVICE, &device_type);
        ASSERT(status == HSA_STATUS_SUCCESS);
        if (device_type == HSA_DEVICE_TYPE_CPU) {
            test_group_add(tg_sg_wait, &signals_wait_host_func, &(agent_list.agents[ii]), 1);
        } else if (device_type == HSA_DEVICE_TYPE_GPU) {
            test_group_add(tg_sg_wait, &signals_wait_component_func, &(agent_list.agents[ii]), 1);
        } else if (device_type == HSA_DEVICE_TYPE_DSP) {
            ASSERT_MSG(1, "ERROR: DSP_AGENT NOT SUPPORTED\n");
        } else {
            ASSERT_MSG(1, "ERROR: UNKNOWN DEVICE\n");
        }
    }

    test_group_thread_create(tg_sg_wait);
    test_group_start(tg_sg_wait);

    for (ii = 0; ii < N*M; ++ii) {
        hsa_signal_store_relaxed(signals[ii], CMP_VAL);
    }
    test_group_wait(tg_sg_wait);
    test_group_exit(tg_sg_wait);
    test_group_destroy(tg_sg_wait);


    for (ii = 0; ii < N*M; ++ii) {
        status = hsa_signal_destroy(signals[ii]);
        ASSERT(status == HSA_STATUS_SUCCESS);
    }

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free_agent_list(&agent_list);

    free(signals);
    free(offset);

    return 0;
}
