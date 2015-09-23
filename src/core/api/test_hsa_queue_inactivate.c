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
 * Test Name: hsa_queue_destroy
 *
 * Purpose: Verify that API of hsa_queue_destroy() works as expected
 *
 * Description:
 *
 * 1) Inactivate a valid, normal queue.
 *    Check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Inactivate a queue after the runtime has been shutdown. The queue
 *    has NOT been destroyed before inactivating it.
 *    Check if the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) Inactivate a queue that has already been destroyed.
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_QUEUE.
 *
 * 4) Inactivate a queue through NULL queue pointer
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 */

#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>
#include "test_helper_func.h"

/**
 *
 * @Brief:
 * Implement Description #1
 *
 * @Return:
 * int
 *
 */

int test_hsa_queue_inactivate() {
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

        hsa_queue_t* queue;
        status = hsa_queue_create(agent_list.agents[ii], 4, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        status = hsa_queue_inactivate(queue);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "The hsa_queue_inactivate API failed when called on a valid queue.");

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
int test_hsa_queue_inactivate_not_initialized() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    hsa_agent_t agent;
    agent.handle = 0;
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        // The agent must support at least one queue
        uint32_t queues_max = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_QUEUES_MAX, &queues_max);
        ASSERT(status == HSA_STATUS_SUCCESS);
        if (queues_max < 1) {
            continue;
        }
        agent = agent_list.agents[ii];
        break;
    }

    if (agent.handle != 0) {
        // Create a queue
        hsa_queue_t* queue;
        status = hsa_queue_create(agent, 4, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Then shutdown the runtime
        status = hsa_shut_down();
        ASSERT(status == HSA_STATUS_SUCCESS);

        // Attempt to inactivate the queue after the runtime is shutdown
        status = hsa_queue_inactivate(queue);
        if (HSA_STATUS_SUCCESS == status) {
            ASSERT_MSG(0, "The hsa_queue_inactivate did not return HSA_STATUS_ERROR_NOT_INITIALIZED when called with a runtime that isn't initialized. HSA_STATUS_SUCCESS received instead.\n");
        } else if (HSA_STATUS_ERROR_NOT_INITIALIZED == status) {
            // This indicates the expected behavior
        } else {
            ASSERT_MSG(0, "The hsa_queue_inactivate did not return HSA_STATUS_ERROR_NOT_INITIALIZED when called with a runtime that isn't initialized.\n");
        }
    } else {
        status = hsa_shut_down();
        ASSERT(status == HSA_STATUS_SUCCESS);
    }

    free_agent_list(&agent_list);

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
int test_hsa_queue_inactivate_invalid_queue() {
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

        hsa_queue_t* queue;
        // Create a queue on the first agent.
        status = hsa_queue_create(agent_list.agents[ii], 4, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Attempt to inactivate the queue after the queue has been destroyed
        status = hsa_queue_inactivate(queue);
        ASSERT_MSG(HSA_STATUS_ERROR_INVALID_QUEUE == status, "The hsa_queue_inactivate API did not return HSA_STATUS_INVALID_QUEUE when called with an invalid queue.");
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

int test_hsa_queue_inactivate_invalid_argument() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_queue_inactivate(NULL);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_QUEUE == status || HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_queue_inactivate API did not return an expected value when called on a NULL queue.\n");

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    return 0;
}
