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
 * Test Name: iterate_terminate
 * Scope: Conformance
 *
 * Purpose: Verifies that if the callback function passed to hsa_iterate_agents
 * returns a status code other than HSA_STATUS_SUCCESS, iteration terminates.
 *
 * Test Description:
 * 1) Call the hsa_iterate_agents API using a callback that returns a valid
 * status code other than HSA_STATUS_SUCCESS.
 * 2) Count the number of times the callback is invoked. This can be done
 * using the data parameter of the callback function. Note that this implies
 * that invocation of each callback isn't concurrent (otherwise iteration
 * wouldn't terminate). 
 *
 * Expected Results: The callback should be invoked only once, regardless of
 * the number of agents.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>

hsa_status_t test_callback(hsa_agent_t agent, void *data) {
    (*(unsigned int *)data)++;
    return HSA_STATUS_INFO_BREAK;
}

int test_iterate_terminate() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    unsigned int count = 0;

    status = hsa_iterate_agents(get_num_agents, &count);
    ASSERT(status == HSA_STATUS_SUCCESS && 1 <= count);

    count = 0;

    status = hsa_iterate_agents(test_callback, &count);
    ASSERT(status == HSA_STATUS_INFO_BREAK && 1 == count);

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    return 0;
}
