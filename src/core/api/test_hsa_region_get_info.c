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
 * Test Name: region_get_info
 *
 * Purpose:
 * Verify that if the API works as expected
 *
 * Description:
 *
 * 1) Iterate through all of the regions of an agent and get all region info.
 *
 * 2) Before the hsa runtime is initialized call hsa_region_get_info and
 *   check that the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) Call hsa_region_get_info with an invalid region handle.
 *    Check that the return value is HSA_STATUS_ERROR_INVALID_REGION.
 *
 * 4) Call hsa_region_get_info with a NULL value for the parameter.
 *    Check that the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT
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

int test_hsa_region_get_info() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Work with the first agent
    hsa_agent_t agent = agent_list.agents[0];

    // Get the total num_regions of regions for the agent
    int num_regions = 0;
    status = hsa_agent_iterate_regions(agent, callback_get_num_regions, &num_regions);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_region_t region_list[num_regions];
    hsa_region_t *ptr_regions = region_list;
    status = hsa_agent_iterate_regions(agent, callback_get_regions, &ptr_regions);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get region list.\n");

    // Iterate through the attributes of all of the region
    int ii;
    for (ii = 0; ii < num_regions; ++ii) {
        hsa_region_t region = *(region_list+ii);

        void *addr;

        size_t size;
        status = hsa_region_get_info(region, HSA_REGION_INFO_SIZE, &size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get region size.\n");

        hsa_region_segment_t segment_info;
        status = hsa_region_get_info(region, HSA_REGION_INFO_SEGMENT, &segment_info);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get region segment info.\n");

        if (HSA_REGION_SEGMENT_GLOBAL == segment_info) {
            uint32_t flag_info = 0;
            status = hsa_region_get_info(region, HSA_REGION_INFO_GLOBAL_FLAGS, &flag_info);
            ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get region flag info.\n");
        }

        size_t max_size;
        status = hsa_region_get_info(region, HSA_REGION_INFO_ALLOC_MAX_SIZE, &max_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get region's maximum size.\n");
        ASSERT_MSG((HSA_REGION_SEGMENT_GLOBAL != segment_info && max_size == 0)||(HSA_REGION_SEGMENT_GLOBAL == segment_info), "The region's maximum allocation size is not correct.\n");

        size_t granule_size;
        status = hsa_region_get_info(region, HSA_REGION_INFO_RUNTIME_ALLOC_GRANULE, &granule_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get region's allocation granularity.\n");
        if (max_size == 0) {
            ASSERT_MSG(granule_size == 0, "The region's granule size is wrong.\n");
        }

        size_t alignment_size;
        status = hsa_region_get_info(region, HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT, &alignment_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get region's allocation alignment size!\n");
        if (max_size == 0) {
           ASSERT_MSG(alignment_size == 0, "The region's allocation alignment size is wrong\n");
        } else {
           ASSERT_MSG(alignment_size&&(!(alignment_size&(alignment_size-1))), "The region's alignment size is not power of 2.\n");
        }
    }

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

int test_hsa_region_get_info_not_initialized() {
    hsa_status_t status;
    size_t size;

    hsa_region_t invalid_region;
    invalid_region.handle = 0;

    status = hsa_region_get_info(invalid_region, HSA_REGION_INFO_SIZE, &size);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "The hsa_region_get_info API failed to return HSA_STATUS_ERROR_NOT_INITIALIZED when called before the runtime was initialized.\n");

    return 0;
}

/**
*
* @Brief:
* Implement Description #3
*
* @Return:
* int
*
*/

int test_hsa_region_get_info_invalid_region() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    size_t size;
    hsa_region_t invalid_region;
    invalid_region.handle = 0;
    status = hsa_region_get_info(invalid_region, HSA_REGION_INFO_SIZE, &size);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_REGION == status, "The hsa_region_get_info API failed to return HSA_STATUS_ERROR_INVALID_REGION when passed an invalid region.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

/**
*
* @Brief:
* Implement Description #4
*
* @Return:
* int
*
*/

int test_hsa_region_get_info_invalid_argument() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Work with the first agent
    hsa_agent_t agent = agent_list.agents[0];

    // Getting total num_regions of regions for the agent
    int num_regions = 0;
    status = hsa_agent_iterate_regions(agent, callback_get_num_regions, &num_regions);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Malloc memory to hold region list of an agent
    hsa_region_t region_list[num_regions];
    hsa_region_t *ptr_regions = region_list;
    status = hsa_agent_iterate_regions(agent, callback_get_regions, &ptr_regions);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_region_t region = region_list[0];

    status = hsa_region_get_info(region, HSA_REGION_INFO_SIZE, NULL);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_region_get_info API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when passed a NULL parameter.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
