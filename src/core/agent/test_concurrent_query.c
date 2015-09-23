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
 * Test Name: concurrent_query
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_agent_get_info API is thread safe.
 *
 * Test Description:
 * 1) Use hsa_iterate_agents to obtain the list of valid agents in the
 * system.
 * 2) For each agent, query all of the agent's attributes and cache
 * those values.
 * 3) For each attribute create several threads that,
 *    a) Concurrently query the attribute.
 *    b) Compares the attribute to the originally cached value.
 * 4) Repeat this several times for each attribute.
 *
 * Expected Results: The concurrent queries should obtain the same results
 * as the initial query.
 */

#include <hsa.h>
#include <hsa_ext_image.h>
#include <agent_utils.h>
#include <concurrent_utils.h>
#include <framework.h>
#include <stdlib.h>

#define NUM_ITER 10
#define NUM_TESTS 10

struct attribute_s {
    hsa_agent_t agent;
    hsa_agent_info_t attr;
    size_t size_attr;
    void *value;
};

void check_attribute(void *data) {
    hsa_status_t status;
    struct attribute_s *attr_orig = (struct attribute_s *)data;
    void *attr_val = (void*) malloc(attr_orig->size_attr);
    memset(attr_val, 0, attr_orig->size_attr);
    // Query a new attribute
    status = hsa_agent_get_info(attr_orig->agent, attr_orig->attr, attr_val);
    ASSERT(status == HSA_STATUS_SUCCESS);
    // Compare the new attribute to the given attribute
    ASSERT(memcmp(attr_val, attr_orig->value, attr_orig->size_attr) == 0);
    free(attr_val);
    return;
}

int check_attribute_concurr(hsa_agent_t agent,
                            hsa_agent_info_t attr,
                            size_t size_attr, void *value) {
    struct attribute_s attr_orig;
    // Fill attribute information
    attr_orig.attr = attr;
    attr_orig.agent = agent;
    attr_orig.size_attr = size_attr;
    attr_orig.value = value;

    // Create threads to query the attribute concurrently
    struct test_group *tg_attr = test_group_create(NUM_TESTS);
    test_group_add(tg_attr, &check_attribute, &attr_orig, NUM_TESTS);
    test_group_thread_create(tg_attr);
    test_group_start(tg_attr);
    test_group_wait(tg_attr);
    test_group_exit(tg_attr);
    test_group_destroy(tg_attr);

    return 0;
}

void check_attributes_concurr(hsa_agent_t agent) {
    hsa_status_t status;
    int ii;
    for (ii = 0; ii < NUM_ITER; ++ii) {
        char name[64];
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_NAME, name);
        check_attribute_concurr(agent, HSA_AGENT_INFO_NAME, sizeof(name), name);
    }

    for (ii = 0; ii < NUM_ITER; ++ii) {
        char vendor_name[64];
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_VENDOR_NAME,
                                    vendor_name);

        check_attribute_concurr(agent, HSA_AGENT_INFO_VENDOR_NAME,
                                sizeof(vendor_name), vendor_name);
    }

    uint32_t feature = 0;
    for (ii = 0; ii < NUM_ITER; ++ii) {
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_FEATURE, &feature);
        check_attribute_concurr(agent, HSA_AGENT_INFO_FEATURE,
                                sizeof(feature), &feature);
    }

    if (HSA_AGENT_FEATURE_KERNEL_DISPATCH == feature) {
        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint32_t wavefront_size = 0;
            status = hsa_agent_get_info(agent, HSA_AGENT_INFO_WAVEFRONT_SIZE,
                                        &wavefront_size);
            check_attribute_concurr(agent, HSA_AGENT_INFO_WAVEFRONT_SIZE,
                                    sizeof(wavefront_size), &wavefront_size);
        }

        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint32_t workgroup_max_size;
            status = hsa_agent_get_info(agent,
                                        HSA_AGENT_INFO_WORKGROUP_MAX_SIZE,
                                        &workgroup_max_size);
            check_attribute_concurr(agent,
                                    HSA_AGENT_INFO_WORKGROUP_MAX_SIZE,
                                    sizeof(workgroup_max_size),
                                    &workgroup_max_size);
        }

        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint16_t workgroup_max_dim[3];
            workgroup_max_dim[0] = 0;
            workgroup_max_dim[1] = 0;
            workgroup_max_dim[2] = 0;
            status = hsa_agent_get_info(agent,
                                        HSA_AGENT_INFO_WORKGROUP_MAX_DIM,
                                        workgroup_max_dim);
            check_attribute_concurr(agent,
                                    HSA_AGENT_INFO_WORKGROUP_MAX_DIM,
                                    3 * sizeof(uint16_t),
                                    workgroup_max_dim);
        }

        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint32_t grid_max_size;
            status = hsa_agent_get_info(agent,
                                        HSA_AGENT_INFO_GRID_MAX_SIZE,
                                        &grid_max_size);
            check_attribute_concurr(agent,
                                    HSA_AGENT_INFO_GRID_MAX_SIZE,
                                    sizeof(grid_max_size),
                                    &grid_max_size);
        }

        for (ii = 0; ii < NUM_ITER; ++ii) {
            hsa_dim3_t grid_max_dim;
            grid_max_dim.x = 0;
            grid_max_dim.y = 0;
            grid_max_dim.z = 0;
            status = hsa_agent_get_info(agent,
                                        HSA_AGENT_INFO_GRID_MAX_DIM,
                                        &grid_max_dim);
            check_attribute_concurr(agent,
                                    HSA_AGENT_INFO_GRID_MAX_DIM,
                                    sizeof(hsa_dim3_t),
                                    &grid_max_dim);
        }

        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint32_t fbarriers_max_size;
            status = hsa_agent_get_info(agent,
                                        HSA_AGENT_INFO_FBARRIER_MAX_SIZE,
                                        &fbarriers_max_size);
            check_attribute_concurr(agent,
                                    HSA_AGENT_INFO_FBARRIER_MAX_SIZE,
                                    sizeof(fbarriers_max_size),
                                    &fbarriers_max_size);
        }
    }

    for (ii = 0; ii < NUM_ITER; ++ii) {
        uint32_t queues_max;
        status = hsa_agent_get_info(agent,
                                    HSA_AGENT_INFO_QUEUES_MAX,
                                    &queues_max);
        check_attribute_concurr(agent,
                                HSA_AGENT_INFO_QUEUES_MAX,
                                sizeof(queues_max),
                                &queues_max);
    }

    for (ii = 0; ii < NUM_ITER; ++ii) {
        uint32_t queue_max_size;
        status = hsa_agent_get_info(agent,
                                    HSA_AGENT_INFO_QUEUE_MAX_SIZE,
                                    &queue_max_size);
        check_attribute_concurr(agent,
                                HSA_AGENT_INFO_QUEUE_MAX_SIZE,
                                sizeof(queue_max_size),
                                &queue_max_size);
    }

    for (ii = 0; ii < NUM_ITER; ++ii) {
        uint32_t node;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_NODE, &node);
        check_attribute_concurr(agent,
                                HSA_AGENT_INFO_NODE,
                                sizeof(node),
                                &node);
    }

    hsa_device_type_t device;
    for (ii = 0; ii < NUM_ITER; ++ii) {
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &device);
        check_attribute_concurr(agent,
                                HSA_AGENT_INFO_DEVICE,
                                sizeof(device),
                                &device);
    }

    for (ii = 0; ii < NUM_ITER; ++ii) {
        uint32_t cache_size[4];
        cache_size[0] = 0;
        cache_size[1] = 0;
        cache_size[2] = 0;
        cache_size[3] = 0;
        status = hsa_agent_get_info(agent,
                                    HSA_AGENT_INFO_CACHE_SIZE,
                                    cache_size);
        check_attribute_concurr(agent,
                                HSA_AGENT_INFO_CACHE_SIZE,
                                4 * sizeof(uint32_t),
                                cache_size);
    }

    if (device == HSA_DEVICE_TYPE_GPU) {
        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint32_t image1d_max_elems;
            status =
                 hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_1D_MAX_ELEMENTS,
                                    &image1d_max_elems);
            check_attribute_concurr(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_1D_MAX_ELEMENTS,
                                    sizeof(uint32_t), &image1d_max_elems);
        }

        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint32_t image2d_max_elems[2];
            status =
                 hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_2D_MAX_ELEMENTS,
                                    image2d_max_elems);
            check_attribute_concurr(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_2D_MAX_ELEMENTS,
                                    2 * sizeof(uint32_t),
                                    image2d_max_elems);
        }

        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint32_t image3d_max_elems[3];
            status =
                 hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_3D_MAX_ELEMENTS,
                                    &image3d_max_elems);
            check_attribute_concurr(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_3D_MAX_ELEMENTS,
                                    3 * sizeof(uint32_t),
                                    image3d_max_elems);
        }

        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint32_t image_array_max_layers;
            status =
                 hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_ARRAY_MAX_LAYERS,
                                    &image_array_max_layers);
            check_attribute_concurr(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_ARRAY_MAX_LAYERS,
                                    sizeof(uint32_t),
                                    &image_array_max_layers);
        }

        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint32_t image_rd_max;
            status =
                 hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_MAX_IMAGE_RD_HANDLES,
                                    &image_rd_max);
            check_attribute_concurr(agent,
                                    HSA_EXT_AGENT_INFO_MAX_IMAGE_RD_HANDLES,
                                    sizeof(uint32_t),
                                    &image_rd_max);
        }

        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint32_t image_rorw_max;
            status =
                 hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_MAX_IMAGE_RORW_HANDLES,
                                    &image_rorw_max);
            check_attribute_concurr(agent,
                                    HSA_EXT_AGENT_INFO_MAX_IMAGE_RORW_HANDLES,
                                    sizeof(uint32_t),
                                    &image_rorw_max);
        }

        for (ii = 0; ii < NUM_ITER; ++ii) {
            uint32_t sampler_max;
            status =
                 hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_MAX_SAMPLER_HANDLERS,
                                    &sampler_max);
            check_attribute_concurr(agent,
                                    HSA_EXT_AGENT_INFO_MAX_SAMPLER_HANDLERS,
                                    sizeof(uint32_t),
                                    &sampler_max);
        }
    }

    return;
}

hsa_status_t check_agents_concurr(hsa_agent_t agent, void* data) {
    hsa_agent_t *ret = (hsa_agent_t *)data;
    *ret = agent;

    // Check attributes of the agent
    check_attributes_concurr(agent);

    // Keep iterating
    return HSA_STATUS_SUCCESS;
}

int test_concurrent_query() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(status == HSA_STATUS_SUCCESS);

    hsa_agent_t agent;
    status = hsa_iterate_agents(check_agents_concurr, &agent);
    ASSERT(status == HSA_STATUS_SUCCESS);

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    return 0;
}
