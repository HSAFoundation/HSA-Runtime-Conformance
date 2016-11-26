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
 *
 * Test Name: hsa_queue_create
 *
 * Purpose: Verify that API of hsa_queue_create() works as expected
 *
 * Description:
 *
 * 1) Create a queue with valid parameters.
 *    Check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Init the runtime, generate the agent list, shutdown the runtime,
 *    then create queue.
 *    Check if the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) Create queues until the system is running out of resources.
 *    Check if the return value is HSA_STATUS_ERROR_OUT_OF_RESOURCES.
 *
 * 4) Create a queue on an agent that does NOT support dispatch.
 *    Create a queue on an invalid (not_initialized) agent.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_AGENT.
 *
 * 5) Crate a queue of type MULTI on an agent that only supports
 *    SINGLE. Check if the return value is
 *    HSA_STATUS_ERROR_INVALID_QUEUE_CREATION.
 *
 * 6) Create queues with invalid arguments: queue size of a power
 *    of 2, an invalid queue "type", queue pointer being NULL.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>
#include "test_helper_func.h"
#include <stdlib.h>

/**
 *
 * @Brief:
 * Implement Description #1
 *
 * @Return:
 * int
 *
 */

int test_hsa_queue_create() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // The agent must support at least one queue
        uint32_t queues_max = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUES_MAX, &queues_max);
        ASSERT(status == HSA_STATUS_SUCCESS);
        if (queues_max < 1) {
            continue;
        }

        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            // Dispatch is not supported on CPU.
            continue;
        }

        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 4, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Attempting to create a queue with the hsa_queue_create API failed.\n");

        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

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

int test_hsa_queue_create_not_initialized() {
    int ii;
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Get all the dispatch agents before shutting down the runtime
    hsa_agent_t** dispatch_agents = (hsa_agent_t**)malloc(sizeof(hsa_agent_t*) * agent_list.num_agents);
    int num_dispatch_agents = 0;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 != (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            // Dispatch is supported on GPU, not on CPU
            dispatch_agents[num_dispatch_agents] = &(agent_list.agents[ii]);
            ++num_dispatch_agents;
        }
    }

    free_agent_list(&agent_list);

    // Shut down the runtime
    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    for (ii = 0; ii < num_dispatch_agents; ++ii) {
        hsa_queue_t* queue;
        status = hsa_queue_create(*(dispatch_agents[ii]), 4, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        if (HSA_STATUS_ERROR_INVALID_AGENT == status) {
            ASSERT_MSG(0, "The hsa_queue_create API returned HSA_STATUS_ERROR_INVALID_AGENT instead of HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime wasn't initialized.\n");
        } else if (HSA_STATUS_ERROR_NOT_INITIALIZED == status) {
            // This indicates proper behavior
        } else {
            ASSERT_MSG(0, "The hsa_queue_create API returned an error besides HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime wasn't initialized.\n");
        }
    }

    free(dispatch_agents);
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

int test_hsa_queue_create_out_of_resources() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // Get max number of queues
        uint32_t queues_max;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUES_MAX, &queues_max);
        ASSERT(HSA_STATUS_SUCCESS == status);

        if (queues_max < 1) {
            // This agent does not support any queue
            continue;
        }

        // Check if the queue supports dispatch
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (0 == (features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            // Dispatch is not supported on CPU.
            continue;
        }

        uint32_t type = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUE_TYPE, &type);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the queues
        const uint32_t queue_size = 128;
        hsa_queue_t** queues = (hsa_queue_t**)malloc(queues_max * sizeof(hsa_queue_t*));
        int num_queues = 0;
        while (num_queues < queues_max) {
            status = hsa_queue_create(agent_list.agents[ii], queue_size, type, NULL, NULL, UINT32_MAX, UINT32_MAX, queues + num_queues);
            if (HSA_STATUS_ERROR_OUT_OF_RESOURCES == status) {
                break;
            }
            ASSERT_MSG(HSA_STATUS_SUCCESS == status, "The hsa_queue_create API didn't return HSA_STATUS_ERROR_OUT_OF_RESOURCES when more than the maximum number of queues were created.");
            ++num_queues;
        }


        // Destroy queues
        int jj;
        for (jj = 0; jj < num_queues; ++jj) {
            status = hsa_queue_destroy(queues[jj]);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }

        free(queues);
    }

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free_agent_list(&agent_list);

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

int test_hsa_queue_create_invalid_agent() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create a queue with an invalid agent.
    hsa_queue_t* queue;
    hsa_agent_t agent;
    agent.handle = 0;
    status = hsa_queue_create(agent, 4, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
    ASSERT(HSA_STATUS_ERROR_INVALID_AGENT == status);

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

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

int test_hsa_queue_create_invalid_queue_creation() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // The agent must support at least one queue
        uint32_t queues_max = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUES_MAX, &queues_max);
        ASSERT(status == HSA_STATUS_SUCCESS);
        if (queues_max < 1) {
            continue;
        }

        // Find an agent that only supports one producer
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUE_TYPE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if ((hsa_queue_type_t)features == HSA_QUEUE_TYPE_SINGLE) {
            // Create a queue of type MULTI.
            hsa_queue_t* queue;
            status = hsa_queue_create(agent_list.agents[ii], 4, HSA_QUEUE_TYPE_MULTI, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);

            // Expect result of INVALID_QUEUE_CREATION
            ASSERT(HSA_STATUS_ERROR_INVALID_QUEUE_CREATION == status);
        }
    }

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

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

int test_hsa_queue_create_invalid_argument() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        hsa_queue_t* queue;
        char* err_string;

        // The agent must support at least one queue
        uint32_t queues_max = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUES_MAX, &queues_max);
        ASSERT(status == HSA_STATUS_SUCCESS);
        if (queues_max < 1) {
            continue;
        }

        // Create a queue with a size that is not a power of 2
        status = hsa_queue_create(agent_list.agents[ii], 5, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        if (HSA_STATUS_ERROR == status) {
            ASSERT_MSG(1, "Queue create with size not a power of 2: ERROR_INVALID_ARGUMENT expected, ERROR received.\n");
        } else if (HSA_STATUS_ERROR_INVALID_ARGUMENT == status) {
            // This indicates proper behavior
        } else {
            ASSERT(0);
        }

        // Create a queue with an invalid queue type
        status = hsa_queue_create(agent_list.agents[ii], 4, 3, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        if (HSA_STATUS_SUCCESS == status) {
            ASSERT_MSG(0, "Queue created with \"type\" not a valid type: HSA_STATUS_ERROR_INVALID_ARGUMENT expected, HSA_STATUS_SUCCESS received.\n");
        } else if (HSA_STATUS_ERROR_INVALID_ARGUMENT == status) {
            // This indicates proper behavior
        } else {
            ASSERT(0);
        }

        // Create a queue with NULL pointer to the queue
        status = hsa_queue_create(agent_list.agents[ii], 4, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, NULL);
        ASSERT(HSA_STATUS_ERROR_INVALID_ARGUMENT == status);
    }

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    free_agent_list(&agent_list);

    return 0;
}
