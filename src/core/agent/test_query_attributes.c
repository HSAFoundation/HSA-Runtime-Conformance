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
 * Test Name: query_attributes
 * Scope: Conformance
 *
 * Purpose: Verifies that the agent list can be traversed using the
 * hsa_iterate_agents
 * API, and that every agent attribute can be queried using the
 * hsa_agent_get_info
 * API. 
 *
 * Test Description:
 * 1) Call hsa_iterate_agent with a callback that does the following for each
 * agent,
 *    a) Query all defined attributes using the hsa_agent_get_info API.
 *    b) Checks each of the queried attributes for known constraints.
 * 2) Count the number of agents available.
 *
 * Expected Results: Each attribute should have a reasonable set of
 * values. There should
 * be at least 2 HSA agents, one CPU and one COMPONENT.
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>

int test_query_attributes() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    // Call hsa_iterate_agents traversing all agents and check attributes of
    // each agent
    status = hsa_iterate_agents(check_agent, NULL);
    ASSERT(status == HSA_STATUS_SUCCESS);

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    return 0;
}
