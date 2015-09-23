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
#include <framework.h>
#include <agent_utils.h>
#include "test_helper_func.h"

/**
 *
 * Test Name: hsa_memory_free
 *
 * Purpose:
 * Verify that if the API works as expected
 *
 * Description:
 *
 * 1) Allocate a block of memory using hsa_memory_allocate and then call hsa_memory_free.
 *    Check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Before init HsaRt, call hsa_memory_free, and check if the return value is HSA_STATUS_ERROR_NOT_INITIALIZED
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

int test_hsa_memory_free() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Work with the first agent
    hsa_agent_t agent = agent_list.agents[0];

    // Get the total number of regions for the agent
    int num_regions = 0;
    status = hsa_agent_iterate_regions(agent, callback_get_num_regions, &num_regions);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_region_t region_list[num_regions];
    hsa_region_t *ptr_reg = region_list;
    status = hsa_agent_iterate_regions(agent, callback_get_regions, &ptr_reg);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_region_t region = region_list[0];

    // Getting information about region's maximum size
    size_t size_r = 0;
    status = hsa_region_get_info(region, HSA_REGION_INFO_ALLOC_MAX_SIZE, (void *)&size_r);
    ASSERT(HSA_STATUS_SUCCESS == status);

    int size = 1024;
    void *addr = 0;

    // Allocating a block of memory
    status = hsa_memory_allocate(region, size, &addr);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Deallocating the memory
    status = hsa_memory_free(addr);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "The hsa_memory_free API failed to free memory correctly.\n");

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

int test_hsa_memory_free_not_initialized() {
    hsa_status_t status;
    void *addr = 0;

    status = hsa_memory_free(addr);
    ASSERT_MSG(status == HSA_STATUS_ERROR_NOT_INITIALIZED, "The hsa_memory_free API failed to return HSA_STATUS_ERROR_NOT_INITIALIZED when called before the runtime was initialized.");

    return 0;
}
