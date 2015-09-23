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
 * Test Name: hsa_isa_get_info
 * Scope: Conformance
 *
 * Purpose: Verify that the hsa_isa_get_info works as expected.
 *
 * Test Description:
 *
 * 1) Query a valid isa for all supported ISA attributes. Verify
 * that the api executes successfully and return valid values.
 *
 * 2) Verify that the hsa_isa_get_info API returns 
 * HSA_STATUS_ERROR_NOT_INITIALIZED if it is called before the
 * runtime is initialized.
 *
 * 3) Verify that the hsa_isa_get_info API returns
 * HSA_STATUS_ERROR_INVALID_ISA if the ISA is NULL.
 *
 * 4) Verify that the hsa_isa_get_info API returns
 * HSA_STATUS_ERROR_INVALID_INDEX if index is out of range.
 *
 * 5) Verify that the hsa_isa_get_info API returns
 * HSA_STATUS_ERROR_INVALID_ARGUMENT if the specified attribute
 * is invalid.
 *
 * 6) Verify that the hsa_isa_get_info API returns
 * HSA_STATUS_ERROR_INVALID_ARGUMENT if the value parameter
 * is null.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <hsa.h>
#include <hsa_ext_image.h>
#include <framework.h>
#include "test_helper_func.h"

int test_hsa_isa_get_info() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get a kernel dispatch agent
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(
        callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Get the ISA from this agent
    hsa_isa_t isa;
    isa.handle = (uint64_t)-1;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != isa.handle);

    // Query ISA's info
    uint32_t name_length;
    status = hsa_isa_get_info(isa, HSA_ISA_INFO_NAME_LENGTH, 0, &name_length);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get ISA's name length.\n");
    ASSERT_MSG(name_length > 0, "ISA's name length is zero.\n");

    char name[name_length];
    status = hsa_isa_get_info(isa, HSA_ISA_INFO_NAME, 0, name);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get ISA's name.\n");

    uint32_t call_conv_count = 0;
    status = hsa_isa_get_info(isa, HSA_ISA_INFO_CALL_CONVENTION_COUNT, 0, &call_conv_count);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get ISA's call convention count.\n");

    uint32_t i;
    for (i = 0; i < call_conv_count; ++i) {
        uint32_t wavefront_size;
        status = hsa_isa_get_info(isa,
            HSA_ISA_INFO_CALL_CONVENTION_INFO_WAVEFRONT_SIZE,
            i,
            &wavefront_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get ISA's call convention wavefront size.\n");
        ASSERT_MSG(wavefront_size >= 1 && wavefront_size <= 256, "ISA's call convention wavefront size must be in the range of [1, 256].\n");
        ASSERT_MSG(!(wavefront_size & (wavefront_size-1)), "ISA's call convention wavefronts per compute unit must be a power of 2.\n");

        uint32_t wavefronts_per_comp_unit;
        status = hsa_isa_get_info(isa,
            HSA_ISA_INFO_CALL_CONVENTION_INFO_WAVEFRONTS_PER_COMPUTE_UNIT,
            i,
            &wavefronts_per_comp_unit);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get ISA's call convention wavefronts per compute unit.\n");
    }

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
 
int test_hsa_isa_get_info_not_initialized() {
    hsa_status_t status;

    uint32_t name_length;
    hsa_isa_t isa;
    status = hsa_isa_get_info(isa, HSA_ISA_INFO_NAME_LENGTH, 0, &name_length);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "The hsa_isa_get_info API failed to return HSA_STATUS_ERROR_NOT_INITIALIZED when called before the runtime was initialized.\n");

    return 0;
}

int test_hsa_isa_get_info_invalid_isa() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Query with a null ISA.
    uint32_t value;
    hsa_isa_t invalid_isa;
    invalid_isa.handle = 0;
    status = hsa_isa_get_info(invalid_isa, HSA_ISA_INFO_NAME_LENGTH, 0, &value);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ISA == status, "The hsa_isa_get_info API failed to return HSA_STATUS_ERROR_INVALID_ISA when it was called with an invalid isa.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

int test_hsa_isa_get_info_index_out_of_range() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get a kernel dispatch agent
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(
        callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Get the ISA from this agent
    hsa_isa_t isa;
    isa.handle = (uint64_t)-1;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != isa.handle);

    // Query the isa with a index greater than the call convention count. 
    uint32_t value;
    status = hsa_isa_get_info(isa, HSA_ISA_INFO_NAME_LENGTH, -1, &value);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_INDEX == status, "The hsa_isa_get_info API failed to return HSA_STATUS_ERROR_INVALID_INDEX when it was called with an invalid attribute.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

int test_hsa_isa_get_info_invalid_attribute() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get a kernel dispatch agent
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(
        callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Get the ISA from this agent
    hsa_isa_t isa;
    isa.handle = (uint64_t)-1;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != isa.handle);

    // Query ISA's info
    uint32_t value;
    status = hsa_isa_get_info(isa, -1, 0, &value);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_isa_get_info API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when it was called with an invalid attribute.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

int test_hsa_isa_get_info_invalid_null_value() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get a kernel dispatch agent
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(
        callback_get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Get the ISA from this agent
    hsa_isa_t isa;
    isa.handle = (uint64_t)-1;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != isa.handle);

    // Query ISA's info
    status = hsa_isa_get_info(isa, HSA_ISA_INFO_NAME_LENGTH, 0, NULL);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_isa_get_info API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when it was called with an invalid attribute.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
