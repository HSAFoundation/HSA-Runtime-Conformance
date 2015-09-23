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

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>
#include "test_helper_func.h"

/**
 *
 * Test Name: hsa_memory_allocate
 *
 * Purpose:
 * Verify that if the API works as expected
 *
 * Description:
 *
 * 1) Get the list of agents and their valid regions
 *    Call the hsa_memory_allocate API with the valid region and appropriate size.
 *    Check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Before initializing the HSA runtime, call hsa_memory_allocate,
 *    and check that the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) Check that the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT when
 *    passing a NULL value as the base address parameter.
 *
 * 4) Check that the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT when
 *    passing a 0 for the size parameter.
 *
 * 5) Check that the return value is HSA_STATUS_ERROR_INVALID_ALLOCATION when
 *    passing a size argument greater than the size of region.
 *
 * 6) Check if the return value is HSA_STATUS_ERROR_INVALID_REGION when passing region 0.
 *
 */

/**
 *
 * @Brief:
 * Implement Description #1
 *
 * @Return:
 * int
 *
 */

int test_hsa_memory_allocate() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Work with the first agent
    ASSERT(0 < agent_list.num_agents);
    int i = 0;
    hsa_agent_t agent = agent_list.agents[0];

    // Getting total number of regions for the agent
    int num_regions = 0;
    status = hsa_agent_iterate_regions(agent, callback_get_num_regions, &num_regions);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Allocate memory to hold region list of an agent
    hsa_region_t region_list[num_regions];
    hsa_region_t *ptr_reg= region_list;
    status = hsa_agent_iterate_regions(agent, callback_get_regions, &ptr_reg);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_region_t region = region_list[0];

    // Getting information about region's maximum size
    size_t size_r = 0;
    status = hsa_region_get_info(region, HSA_REGION_INFO_ALLOC_MAX_SIZE, (void *)&size_r);
    ASSERT(HSA_STATUS_SUCCESS == status);

    int size = 1024;
    void *addr = 0;

    status = hsa_memory_allocate(region, size, &addr);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "The hsa_memory_allocate API failed to correctly allocate memory.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

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

int test_hsa_memory_allocate_not_initialized() {
    hsa_status_t status;
    void *addr = 0;

    hsa_region_t invalid_region;
    invalid_region.handle = 0;
    status = hsa_memory_allocate(invalid_region, 10, &addr);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "The hsa_memory_allocate API failed to return HSA_STATUS_ERROR_NOT_INITIALIZED when called before the runtime was initialized.\n");

    return 0;
}

/**
 *
 *@Brief:
 *Implement Description #3
 *
 * @Return:
 * int
 *
 */

int test_hsa_memory_allocate_null_ptr() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Work with the first agent
    ASSERT(0 < agent_list.num_agents);
    int i = 0;
    hsa_agent_t agent = agent_list.agents[0];

    // Get the total number of regions for the agent
    int num_regions = 0;
    status = hsa_agent_iterate_regions(agent, callback_get_num_regions, &num_regions);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Allocate memory to hold region list of an agent
    hsa_region_t region_list[num_regions];
    hsa_region_t *ptr_reg = region_list;
    status = hsa_agent_iterate_regions(agent, callback_get_regions, &ptr_reg);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_region_t region = region_list[0];

    status = hsa_memory_allocate(region, 1024, NULL);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_memory_allocate API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when called with a NULL pointer.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}

/**
 *
 *@Brief:
 *Implement Description #4
 *
 * @Return:
 * int
 *
 */

int test_hsa_memory_allocate_zero_size() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Work with the first agent
    ASSERT(0 < agent_list.num_agents);
    int i = 0;
    hsa_agent_t agent = agent_list.agents[0];

    // Get the total number of regions for the agent
    int num_regions = 0;
    status = hsa_agent_iterate_regions(agent, callback_get_num_regions, &num_regions);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Allocate memory to hold region list of an agent
    hsa_region_t region_list[num_regions];
    hsa_region_t *ptr_reg = region_list;
    status = hsa_agent_iterate_regions(agent, callback_get_regions, &ptr_reg);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_region_t region = region_list[0];

    void *addr = 0;
    status = hsa_memory_allocate(region, 0, &addr);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_memory_allocate API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when called with a size of 9.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}

/**
 *
 * @Brief:
 * Implement Description #5
 *
 * @Return:
 * int
 *
 */

int test_hsa_memory_allocate_invalid_allocation() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Work with the first agent
    ASSERT(0 < agent_list.num_agents);
    int i = 0;
    hsa_agent_t agent = agent_list.agents[0];

    // Get the total number of regions for the agent
    int num_regions = 0;
    status = hsa_agent_iterate_regions(agent, callback_get_num_regions, &num_regions);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Allocate memory to hold the region list of an agent
    hsa_region_t region_list[num_regions];
    hsa_region_t *ptr_reg = region_list;
    status = hsa_agent_iterate_regions(agent, callback_get_regions, &ptr_reg);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_region_t region = region_list[0];

    // Get information about region's maximum size
    size_t size_r = 0;
    status = hsa_region_get_info(region, HSA_REGION_INFO_ALLOC_MAX_SIZE, (void *)&size_r);
    ASSERT(HSA_STATUS_SUCCESS == status);
    size_t size = size_r+10;
    void *addr = 0;

    status = hsa_memory_allocate(region, size, &addr);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ALLOCATION == status, "The hsa_memory_allocate API failed to return HSA_STATUS_ERROR_INVALID_ALLOCATION when called with an invalid size.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}

/**
 *
 * @Brief:
 * Implement Description #6
 *
 * @Return:
 * int
 *
 */

int test_hsa_memory_allocate_invalid_region() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    void *addr;
    hsa_region_t invalid_region;
    invalid_region.handle = 0;
    status = hsa_memory_allocate(invalid_region, 1024, &addr);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_REGION == status, "The hsa_memory_allocate API failed to return HSA_STATUS_ERROR_INVALID_REGION when passed an invalid region.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
