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
 * Test Name: signal_create_one_consumers
 * Scope: Conformance
 *
 * Purpose: Verifies that when a signal is created with the num_consumers
 * parameter set to 1 that the consumers array is used to create
 * a signal that can be consumed by that agent.
 *
 * Test Description:
 * 1) Create a signal using the following parameters,
 *    a) A num_consumers value of 1.
 *    b) A consumers list containing 1 agent.
 * 2) After the signal is created, have the specified agent
 * wait on the signal either using the appropriate hsa_signal_wait
 * API or a HSAIL instruction executed in a kernel.
 * 3) Set the signal on another thread such that the waiting
 * threads wait condition is satisfied.
 * 4) Repeat for all agents in the system.
 *
 * Expected Results: All of the agents should be able to properly wait
 * on the signal create for them.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>

#define CMP_VAL 1
#define INI_VAL 0

static void signal_wait_host_func(void *data) {
    hsa_signal_t* signal_ptr = (hsa_signal_t*) data;
    hsa_signal_wait_acquire(*signal_ptr, HSA_SIGNAL_CONDITION_EQ, CMP_VAL, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    return;
}

static void signal_wait_component_func(void *data) {
    hsa_signal_t* signal_ptr = (hsa_signal_t*) data;
    hsa_signal_wait_acquire(*signal_ptr, HSA_SIGNAL_CONDITION_EQ, CMP_VAL, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    return;
}

int test_signal_create_one_consumers() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    hsa_signal_t signal;
    hsa_signal_create(INI_VAL, 1, &(agent_list.agents[0]), &signal);

    struct test_group *tg_sg_wait = test_group_create(agent_list.num_agents);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ii++) {
        hsa_device_type_t device_type;
        hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_DEVICE, &device_type);
        if (device_type == HSA_DEVICE_TYPE_CPU) {
            test_group_add(tg_sg_wait, &signal_wait_host_func, &signal, 1);
        } else if (device_type == HSA_DEVICE_TYPE_GPU) {
            test_group_add(tg_sg_wait, &signal_wait_component_func, &signal, 1);
        } else if (device_type == HSA_DEVICE_TYPE_DSP) {
            ASSERT_MSG(1, "ERROR: DSP_AGENT NOT SUPPORTED\n");
        } else {
            ASSERT_MSG(1, "ERROR: UNKOWN DEIVCE TYPE");
        }
    }

    test_group_thread_create(tg_sg_wait);
    test_group_start(tg_sg_wait);

    hsa_signal_store_relaxed(signal, CMP_VAL);

    test_group_wait(tg_sg_wait);
    test_group_exit(tg_sg_wait);
    test_group_destroy(tg_sg_wait);

    free_agent_list(&agent_list);

    status = hsa_signal_destroy(signal);
    ASSERT(status == HSA_STATUS_SUCCESS);

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    return 0;
}

