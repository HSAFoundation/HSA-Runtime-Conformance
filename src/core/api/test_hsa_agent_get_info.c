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
#include <hsa_ext_image.h>
#include <agent_utils.h>
#include <framework.h>

/**
 *
 * Test Name: agent_get_info
 *
 * Purpose:
 * Verify that if the API works as expected
 *
 * Description:
 *
 * 1) Iterate over all agents and check agent information.
 *
 * 2) Before the runtime is initialized call hsa_agent_get_info and check
 *    that the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) Call hsa_agent_get_info with an invalid agent handle.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_AGENT.
 *
 * 4) Call hsa_agent_get_info with invalid agent attribute.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 * 5) Call hsa_agent_get_info with a NULL value parameter.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
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

int test_hsa_agent_get_info() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;

    get_agent_list(&agent_list);

    // Iterate through the attributes of all of the agents
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        hsa_agent_t agent = agent_list.agents[ii];

        char name[64];

        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_NAME, name);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's name.\n");

        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_VENDOR_NAME, name);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's vendor name.\n");

        hsa_agent_feature_t agent_feature;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_FEATURE, &agent_feature);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get any agent features.\n");

        hsa_machine_model_t machine_model;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_MACHINE_MODEL, &machine_model);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the agent's machine model.\n");

        hsa_profile_t profile;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_PROFILE, &profile);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the agent's profile.\n");

        hsa_default_float_rounding_mode_t rounding_mode;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_DEFAULT_FLOAT_ROUNDING_MODE, &rounding_mode);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the agent's default rounding mode.\n");

        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_BASE_PROFILE_DEFAULT_FLOAT_ROUNDING_MODES, &rounding_mode);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the agent's default rounding mode.\n");

        bool fast_f16_operation;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_FAST_F16_OPERATION, &fast_f16_operation);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the agent's f16 HSAIL operation flag.\n");

        uint32_t qmax = 0;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_QUEUES_MAX, &qmax);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get maximum number of queues supported by the agent.\n");

        uint32_t max = 0;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_QUEUE_MAX_SIZE, &max);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's queue max size.\n");
        // Verify if the max size is power of 2
        if (qmax > 0) {
            ASSERT_MSG(max&&(!(max&(max-1))), "Max queue size is not power of 2!\n");
        }

        uint32_t min = 0;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_QUEUE_MIN_SIZE, &min);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's queue min size.\n");

        hsa_queue_type_t queue_type;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_QUEUE_TYPE, &queue_type);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's queue type.\n");
        ASSERT_MSG(HSA_QUEUE_TYPE_SINGLE == queue_type || HSA_QUEUE_TYPE_MULTI == queue_type, "Neither queue type is supported.\n");

        uint32_t node;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_NODE, &node);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's node info.\n");

        hsa_device_type_t device_type;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &device_type);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's device type.\n");
        ASSERT_MSG(HSA_DEVICE_TYPE_CPU == device_type || HSA_DEVICE_TYPE_GPU == device_type || HSA_DEVICE_TYPE_DSP == device_type, "No device type is supported.\n");

        uint32_t cache_size[4];
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_CACHE_SIZE, cache_size);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's cache size.\n");

        hsa_isa_t isa;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &isa);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's instruction set architecture.\n");

        uint8_t extensions[128];
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_EXTENSIONS, &extensions);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's extensions mask.\n");

        uint16_t major_version;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_VERSION_MAJOR, &major_version);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's major version number.\n");

        uint16_t minor_version;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_VERSION_MINOR, &minor_version);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get agent's minor version number.\n");

        // Attributes that are only supported by agents that support kernel dispatch
        if (HSA_AGENT_FEATURE_KERNEL_DISPATCH == agent_feature) {
            uint32_t size = 0;
            // Verify wavefront size
            status = hsa_agent_get_info(agent, HSA_AGENT_INFO_WAVEFRONT_SIZE, &size);
            ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the agnet's wavefront size.\n");

            // Verify if the size is power of 2 and in the range of [1, 256]
            ASSERT_MSG(size && (!(size & (size-1))) && size <= 256, "Size of the agent's wavefront is not correct.\n");

            size = 0;
            status = hsa_agent_get_info(agent, HSA_AGENT_INFO_WORKGROUP_MAX_SIZE, &size);
            ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the agent's workgroup max size.\n");

            // It imply that workgroup max size can't be 0, verify the value of size
            ASSERT_MSG(size>0, "Faild to get a correct workgroup max size.\n");

            uint16_t max_dim[3];
            status = hsa_agent_get_info(agent, HSA_AGENT_INFO_WORKGROUP_MAX_DIM, max_dim);
            ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the agent's workgroup max dim.\n");

            // Verify each maximum is greater than 0 and not greater than workgroup max size
            ASSERT_MSG(max_dim[0] > 0 &&
                       max_dim[0] <= size &&
                       max_dim[1] > 0 &&
                       max_dim[1] <= size &&
                       max_dim[2] > 0 &&
                       max_dim[2] <= size,
                       "Value of max_dim is not correct.\n");

            uint32_t grid_size = 0;
            status = hsa_agent_get_info(agent, HSA_AGENT_INFO_GRID_MAX_SIZE, &grid_size);
            ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the agent's grid max size.\n");

            // Grid max size must be greater than workgroup max size
            ASSERT_MSG(grid_size >= size, "The value for grid and workgroup size is not correct!\n");

            hsa_dim3_t hsa_dim;
            status = hsa_agent_get_info(agent, HSA_AGENT_INFO_GRID_MAX_DIM, &hsa_dim);
            ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the agent's grid max dim.\n");

            // Verify each maximum is greater than 0, no greater than grid max size, and no smaller than corresponding workgroup max dim
            ASSERT_MSG(hsa_dim.x >0 &&
                       hsa_dim.x <= grid_size &&
                       hsa_dim.x >= max_dim[0] &&
                       hsa_dim.y >0 &&
                       hsa_dim.y <= grid_size &&
                       hsa_dim.y >= max_dim[1] &&
                       hsa_dim.z >0 &&
                       hsa_dim.z <= grid_size &&
                       hsa_dim.z >= max_dim[2],
                       "The value of grid max dim is not correct.\n");

            uint32_t fbarrier_size = 0;
            status = hsa_agent_get_info(agent, HSA_AGENT_INFO_FBARRIER_MAX_SIZE, &fbarrier_size);
            ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the agent's max fbarrier size.\n");
            ASSERT_MSG(fbarrier_size >= 32, "FBARRIER size must be at least 32!\n");
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

int test_hsa_agent_get_info_not_initialized() {
    hsa_status_t status;
    char name[64];

    hsa_agent_t invalid_agent;
    invalid_agent.handle = 10;
    status = hsa_agent_get_info(invalid_agent, HSA_AGENT_INFO_NAME, name);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "The hsa_agent_get_info API didn't return HSA_STATUS_ERROR_NOT_INITIALIZED when it was called with an un-initialized runtime.\n");

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

int test_hsa_agent_get_info_invalid_agent() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    char name[64];
    hsa_agent_t invalid_agent;
    invalid_agent.handle = 0;
    status = hsa_agent_get_info(invalid_agent, HSA_AGENT_INFO_NAME, name);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_AGENT == status, "The hsa_agent_info API didn't return HSA_STATUS_ERROR_INVALID_AGENT when it was called with an invalid agent.\n");

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

int test_hsa_agent_get_info_invalid_attribute() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    char pt[128];

    // Pass in an invalid attribute value
    status = hsa_agent_get_info(agent_list.agents[0], -1, (void*) &pt);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_agent_info API didn't return HSA_STATUS_ERROR_INVALID_ARGUMENT when it was called with an invalid attribute.\n");

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

int test_hsa_agent_get_info_invalid_ptr() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    status = hsa_agent_get_info(agent_list.agents[0], HSA_AGENT_INFO_NAME, NULL);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_agent_info API didn't return HSA_STATUS_ERROR_INVALID_ARGUMENT when it was called with a NULL variable.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    free_agent_list(&agent_list);

    return 0;
}
