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
 * Test Name: signal_wait_multiple_agents
 * Scope: Conformance
 *
 * Purpose: Verifies that multiple agents can wait on a signal signal.
 *
 * Test Description:
 * 1) Generate a list of agents on the system. If there
 * are less than two, pass the test and finish executing.
 *
 * 2) Create a signal.
 *
 * 3) On each agent, wait on the signal value. This can be done either
 * with an HSA runtime API or a kernel, whichever is appropriate.
 *
 * 4) Set the signals value such that the conditions for all waiters
 * become satisfied.
 *
 * Expected Results: All the agents should be able to wait on the signal,
 * and all of the waiters should wake up once the condition on the signal
 * is met.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>

int test_signal_wait_multiple_agents() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);
    struct agent_list_s agent_list;

    // Generate a list of agents
    get_agent_list(&agent_list);

    // If there are less than two, pass the test and finish executing.
    if (agent_list.num_agents < 2) {
        return 0;
    }

    // Create a signal
    hsa_signal_t signal;
    status = hsa_signal_create(0x0000, agent_list.num_agents, *(agent_list.agents), &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_value_t value;

    hsa_signal_store_relaxed(signal, 0x1000);

    hsa_signal_wait_relaxed(signal, HSA_EQ, 0x1000, UINT64_MAX, HSA_WAIT_EXPECTANCY_UNKNOWN);

    status = hsa_signal_destroy(signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
