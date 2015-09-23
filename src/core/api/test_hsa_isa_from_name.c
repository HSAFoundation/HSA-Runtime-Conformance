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
 * Test Name: hsa_isa_compatible
 * Scope: Conformance
 *
 * Purpose: Verify that the hsa_isa_from_name API works as
 * expected.
 *
 * Description:
 *
 * 1) Query an agent for a supported isa and get the isa's name.
 * Use that isa name in a call to hsa_isa_from_name to obtain
 * another isa. Check that the agent's isa and the named isa
 * are compatible, if not the same.
 *
 * 2) Call hsa_isa_from_name with a null name. Verify the API
 * return HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 * 3) Call hsa_isa_from_name with a null isa. Verify the API
 * return HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 * 4) Call hsa_isa_from_name with a invalid isa name. Verify the API
 * return HSA_STATUS_ERROR_INVALID_ISA_NAME.
 *
 */

#include <stdio.h>
#include <hsa.h>
#include <hsa_ext_image.h>
#include <framework.h>
#include <finalize_utils.h>
#include "test_helper_func.h"

int test_hsa_isa_from_name() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the kernel dispatch agent
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(
        callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Get the ISA from this agent
    hsa_isa_t agent_isa;
    agent_isa.handle = (uint64_t)-1;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &agent_isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != agent_isa.handle);

    // Get the name length of the agent's isa
    size_t length = 0;
    status = hsa_isa_get_info(agent_isa, HSA_ISA_INFO_NAME_LENGTH, 0, &length);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the name of the agent's isa
    char name[length];
    status = hsa_isa_get_info(agent_isa, HSA_ISA_INFO_NAME, 0, &name);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Obtain another reference to the isa using the name
    hsa_isa_t named_isa;
    status = hsa_isa_from_name(name, &named_isa);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Check if these two ISAs are compatible
    bool compatible;
    status = hsa_isa_compatible(named_isa, agent_isa, &compatible);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

int test_hsa_isa_from_name_null_name() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Try to Obtain another reference to the isa using a NULL name
    hsa_isa_t named_isa;
    status = hsa_isa_from_name(NULL, &named_isa);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status,"The hsa_isa_from_name API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when called with a null name.");

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

int test_hsa_isa_from_name_null_isa() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the kernel dispatch agent
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(
        callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Get the ISA from this agent
    hsa_isa_t agent_isa;
    agent_isa.handle = (uint64_t)-1;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &agent_isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != agent_isa.handle);

    // Get the name length of the agent's isa
    size_t length = 0;
    status = hsa_isa_get_info(agent_isa, HSA_ISA_INFO_NAME_LENGTH, 0, &length);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the name of the agent's isa
    char name[length];
    status = hsa_isa_get_info(agent_isa, HSA_ISA_INFO_NAME, 0, &name);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Try to obtain another reference to the isa using a NULL isa
    status = hsa_isa_from_name(name, NULL);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status,"The hsa_isa_from_name API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when called with a null isa.");

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

int test_hsa_isa_from_name_invalid_isa_name() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Make up the name of the agent's isa
    char* name = "Invalid:ISA:Name";

    // Obtain another reference to the isa using the name
    hsa_isa_t named_isa;
    status = hsa_isa_from_name(name, &named_isa);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ISA_NAME == status,"The hsa_isa_from_name API failed to return HSA_STATUS_ERROR_INVALID_ISA_NAME when called with a invalid isa name.");

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
