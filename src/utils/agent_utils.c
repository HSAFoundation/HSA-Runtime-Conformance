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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <hsa.h>
#include <hsa_ext_image.h>
#include "agent_utils.h"
#include "framework.h"

// Helper functions
char isPowerOfTwo(uint32_t x) {
    return (x && ((x & (x-1)) == 0));
}

// Functions that check that reported attributes are expected
void check_system_info() {
    hsa_status_t status;
    uint16_t version_major;
    status = hsa_system_get_info(HSA_SYSTEM_INFO_VERSION_MAJOR, &version_major);
    ASSERT(status == HSA_STATUS_SUCCESS);

    uint16_t version_minor;
    status = hsa_system_get_info(HSA_SYSTEM_INFO_VERSION_MINOR, &version_minor);
    ASSERT(status == HSA_STATUS_SUCCESS);

    uint64_t timestamp1, timestamp2;
    status = hsa_system_get_info(HSA_SYSTEM_INFO_TIMESTAMP, &timestamp1);
    ASSERT(status == HSA_STATUS_SUCCESS);
    sleep(1);
    status = hsa_system_get_info(HSA_SYSTEM_INFO_TIMESTAMP, &timestamp2);
    ASSERT(status == HSA_STATUS_SUCCESS);
    ASSERT_MSG(timestamp2 > timestamp1, "Error: timestamp1: %u is not less than timestamp2: %u\n", timestamp1, timestamp2);

    uint16_t timestamp_freq;
    status = hsa_system_get_info(HSA_SYSTEM_INFO_TIMESTAMP_FREQUENCY, &timestamp_freq);
    ASSERT(status == HSA_STATUS_SUCCESS);
    // Disable value validation for HSA_SYSTEM_INFO_TIMESTAMP_FREQUENCY
    // ASSERT_MSG(timestamp_freq >= 1 && timestamp_freq <= 400, "Invalid timestamp frequency: %u\n", timestamp_freq);

    uint64_t signal_max_wait;
    status = hsa_system_get_info(HSA_SYSTEM_INFO_SIGNAL_MAX_WAIT, &signal_max_wait);
    ASSERT(status == HSA_STATUS_SUCCESS);

    return;
}

void check_agent_info(hsa_agent_t agent) {
    hsa_status_t status;
    char name[64];
    int ii;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_NAME, name);
    ASSERT(HSA_STATUS_SUCCESS == status);

    char vendor_name[64];
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_VENDOR_NAME, vendor_name);
    ASSERT(HSA_STATUS_SUCCESS == status);

    uint32_t feature = 0;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_FEATURE, &feature);
    ASSERT(HSA_STATUS_SUCCESS == status);

    if (HSA_AGENT_FEATURE_KERNEL_DISPATCH == feature) {
        uint32_t wavefront_size = 0;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_WAVEFRONT_SIZE, &wavefront_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT_MSG((wavefront_size >= 1) && (wavefront_size <= 256) && isPowerOfTwo(wavefront_size), "Error: wavefront_size = %u", wavefront_size);

        uint32_t workgroup_max_size;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_WORKGROUP_MAX_SIZE, &workgroup_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT_MSG(workgroup_max_size > 0, "Error: workgroup_max_size = %u", workgroup_max_size);

        uint16_t workgroup_max_dim[3];
        workgroup_max_dim[0] = 0;
        workgroup_max_dim[1] = 0;
        workgroup_max_dim[2] = 0;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_WORKGROUP_MAX_DIM, workgroup_max_dim);
        ASSERT(HSA_STATUS_SUCCESS == status);
        for (ii = 0; ii < 3; ii++) {
            ASSERT_MSG((workgroup_max_dim[ii] > 0 && workgroup_max_dim[ii] <= workgroup_max_size), "Error: workgroup_max_dim[%d] = %u", ii, workgroup_max_dim[ii]);
        }

        uint32_t grid_max_size;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_GRID_MAX_SIZE, &grid_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(grid_max_size > 0);

        hsa_dim3_t grid_max_dim;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_GRID_MAX_DIM, &grid_max_dim);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(grid_max_dim.x > 0 && grid_max_dim.x >= workgroup_max_dim[0] && grid_max_dim.x <= grid_max_size);
        ASSERT(grid_max_dim.y > 0 && grid_max_dim.y >= workgroup_max_dim[1] && grid_max_dim.y <= grid_max_size);
        ASSERT(grid_max_dim.z > 0 && grid_max_dim.z >= workgroup_max_dim[2] && grid_max_dim.z <= grid_max_size);

        uint32_t fbarriers_max_size;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_FBARRIER_MAX_SIZE, &fbarriers_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT_MSG(fbarriers_max_size >= 32, "Error: fbarriers_max_size = %u", fbarriers_max_size);

        uint32_t queues_max;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_QUEUES_MAX, &queues_max);
        ASSERT(HSA_STATUS_SUCCESS == status);

        uint32_t queue_max_size;
        status = hsa_agent_get_info(agent, HSA_AGENT_INFO_QUEUE_MAX_SIZE, &queue_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(queue_max_size > 0 && isPowerOfTwo(queue_max_size));
    }

    uint32_t node;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_NODE, &node);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_device_type_t device;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &device);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT(device == HSA_DEVICE_TYPE_GPU || device == HSA_DEVICE_TYPE_CPU || HSA_DEVICE_TYPE_DSP);

    uint32_t cache_size[4];
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_CACHE_SIZE, cache_size);
    ASSERT(HSA_STATUS_SUCCESS == status);

    if (device == HSA_DEVICE_TYPE_GPU) {
        uint32_t image1d_max_elems;
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_IMAGE_1D_MAX_ELEMENTS, &image1d_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image1d_max_elems <= 16384);

        uint32_t image1da_max_elems;
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_IMAGE_1DA_MAX_ELEMENTS, &image1da_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image1da_max_elems <= 16384);

        uint32_t image1db_max_elems;
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_IMAGE_1DB_MAX_ELEMENTS, &image1db_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image1db_max_elems <= 65536);

        uint32_t image2d_max_elems[2];
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_IMAGE_2D_MAX_ELEMENTS, image2d_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image2d_max_elems[0] <= 16384 && image2d_max_elems[1] <= 16384);

        uint32_t image2da_max_elems[2];
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_IMAGE_2DA_MAX_ELEMENTS, image2da_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image2da_max_elems[0] <= 16384 && image2da_max_elems[1] <= 16384);

        uint32_t image2dd_max_elems[2];
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_IMAGE_2DDEPTH_MAX_ELEMENTS, image2dd_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image2dd_max_elems[0] <= 16384 && image2dd_max_elems[1] <= 16384);

        uint32_t image2dad_max_elems[2];
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_IMAGE_2DADEPTH_MAX_ELEMENTS, image2dad_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image2dad_max_elems[0] <= 16384 && image2dad_max_elems[1] <= 16384);

        uint32_t image3d_max_elems[3];
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_IMAGE_3D_MAX_ELEMENTS, image3d_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image3d_max_elems[0] <= 2048 && image3d_max_elems[1] <= 2048 && image3d_max_elems[2] <= 2048);

        uint32_t image_array_max_layers;
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_IMAGE_ARRAY_MAX_LAYERS, &image_array_max_layers);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image_array_max_layers <= 2048);

        uint32_t image_rd_max;
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_MAX_IMAGE_RD_HANDLES, &image_rd_max);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image_rd_max >= 128);

        uint32_t image_rorw_max;
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_MAX_IMAGE_RORW_HANDLES, &image_rorw_max);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image_rorw_max >= 64);

        uint32_t sampler_max;
        status = hsa_agent_get_info(agent, HSA_EXT_AGENT_INFO_MAX_SAMPLER_HANDLERS, &sampler_max);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(sampler_max >= 16);
    }

    return;
}

// Callback function to get the number of agents
hsa_status_t get_num_agents(hsa_agent_t agent, void* data) {
    int *num_agents = (int *) data;
    (*num_agents)++;

    return HSA_STATUS_SUCCESS;
}

// Callback function to get the list of agents
hsa_status_t get_agents(hsa_agent_t agent, void* data) {
    hsa_agent_t **agent_list = (hsa_agent_t **) data;
    **agent_list = agent;
    (*agent_list)++;

    return HSA_STATUS_SUCCESS;
}

void get_agent_list(struct agent_list_s *agent_list) {
    size_t num_agents = 0;
    int ii;
    hsa_status_t status;

    // Get number of agents
    status = hsa_iterate_agents(get_num_agents, &num_agents);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create a array of size num_agents to store the agent list
    agent_list->agents = (hsa_agent_t*) malloc(sizeof(hsa_agent_t) * num_agents);

    // Get the agent list
    hsa_agent_t *agent_iter = agent_list->agents;
    status = hsa_iterate_agents(get_agents, &agent_iter);
    ASSERT(HSA_STATUS_SUCCESS == status);

    agent_list->num_agents = num_agents;

    return;
}

void free_agent_list(struct agent_list_s *agent_list) {
    agent_list->num_agents = 0;
    free(agent_list->agents);
    return;
}

// Callbacks that get specific agent types
hsa_status_t get_cpu_agent(hsa_agent_t agent, void* data) {
    hsa_status_t status;
    hsa_device_type_t device_type;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &device_type);
    if (HSA_STATUS_SUCCESS == status && HSA_DEVICE_TYPE_CPU == device_type) {
        hsa_agent_t* ret = (hsa_agent_t*)data;
        *ret = agent;
        return HSA_STATUS_INFO_BREAK;
    }

    return HSA_STATUS_SUCCESS;
}

hsa_status_t get_gpu_agent(hsa_agent_t agent, void* data) {
    hsa_status_t status;
    hsa_device_type_t device_type;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &device_type);
    if (HSA_STATUS_SUCCESS == status && HSA_DEVICE_TYPE_GPU == device_type) {
        hsa_agent_t* ret = (hsa_agent_t*)data;
        *ret = agent;
        return HSA_STATUS_INFO_BREAK;
    }

    return HSA_STATUS_SUCCESS;
}

hsa_status_t get_kernel_dispatch_agent(hsa_agent_t agent, void* data) {
    // callback function to get the first agent that supports kernel dispatch
    hsa_status_t status;
    hsa_agent_feature_t feature;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_FEATURE, &feature);
    if (HSA_STATUS_SUCCESS == status &&
        HSA_AGENT_FEATURE_KERNEL_DISPATCH == feature) {
        hsa_agent_t* ret = (hsa_agent_t*)data;
        *ret = agent;
        return HSA_STATUS_INFO_BREAK;
    }

    return HSA_STATUS_SUCCESS;
}

// Callback that checks an agent attributes are valid
hsa_status_t check_agent(hsa_agent_t agent, void* data) {
    // Check the attributes of the agent
    check_agent_info(agent);

    // Keep iterating
    return HSA_STATUS_SUCCESS;
}

// Callback function to get the number of an agent's memory regions
hsa_status_t get_num_regions(hsa_region_t region, void* data) {
    int *num_regions = (int *) data;
    (*num_regions)++;

    return HSA_STATUS_SUCCESS;
}

// Callback function to get the list of an agent's memory regions
hsa_status_t get_regions(hsa_region_t region, void* data) {
    hsa_region_t **region_list = (hsa_region_t **)data;
    **region_list = region;
    (*region_list)++;

    return HSA_STATUS_SUCCESS;
}

void get_region_list(hsa_agent_t agent, struct region_list_s* region_list) {
    size_t num_regions = 0;
    int ii;
    hsa_status_t status;

    // Get number of regions on the agent
    status = hsa_agent_iterate_regions(agent, get_num_regions, &num_regions);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create a array of size num_regions to store the regions list
    region_list->regions = (hsa_region_t*) malloc(sizeof(hsa_region_t) * num_regions);

    // Get region list
    hsa_region_t *region_iter = region_list->regions;
    status = hsa_agent_iterate_regions(agent, get_regions, &region_iter);
    ASSERT(HSA_STATUS_SUCCESS == status);

    region_list->num_regions = num_regions;

    return;
}

void free_region_list(struct region_list_s* region_list) {
    region_list->num_regions = 0;
    free(region_list->regions);
    return;
}

// Callbacks to get specific types of memory regions
hsa_status_t get_kernarg_memory_region(hsa_region_t region, void* data) {
    hsa_region_segment_t segment;
    hsa_region_get_info(region, HSA_REGION_INFO_SEGMENT, &segment);
    if (HSA_REGION_SEGMENT_GLOBAL != segment) {
        return HSA_STATUS_SUCCESS;
    }

    hsa_region_global_flag_t flags;
    hsa_region_get_info(region, HSA_REGION_INFO_GLOBAL_FLAGS, &flags);
    if (flags & HSA_REGION_GLOBAL_FLAG_KERNARG) {
        hsa_region_t* ret = (hsa_region_t*) data;
        *ret = region;
        return HSA_STATUS_INFO_BREAK;
    }

    return HSA_STATUS_SUCCESS;
}

hsa_status_t get_group_memory_region(hsa_region_t region, void* data) {
    hsa_region_segment_t segment;
    hsa_region_get_info(region, HSA_REGION_INFO_SEGMENT, &segment);
    if (HSA_REGION_SEGMENT_GROUP == segment) {
        hsa_region_t* ret = (hsa_region_t*) data;
        *ret = region;
        return HSA_STATUS_INFO_BREAK;
    }

    return HSA_STATUS_SUCCESS;
}

hsa_status_t get_global_memory_region(hsa_region_t region, void* data) {
    hsa_region_segment_t segment;
    hsa_region_get_info(region, HSA_REGION_INFO_SEGMENT, &segment);
    if (HSA_REGION_SEGMENT_GLOBAL == segment) {
        hsa_region_t* ret = (hsa_region_t*) data;
        *ret = region;
        return HSA_STATUS_INFO_BREAK;
    }

    return HSA_STATUS_SUCCESS;
}

hsa_status_t get_global_memory_region_fine_grained(hsa_region_t region, void* data) {
    hsa_region_segment_t segment;
    hsa_region_get_info(region, HSA_REGION_INFO_SEGMENT, &segment);
    if (HSA_REGION_SEGMENT_GLOBAL != segment) {
        return HSA_STATUS_SUCCESS;
    }

    hsa_region_global_flag_t flags;
    hsa_region_get_info(region, HSA_REGION_INFO_GLOBAL_FLAGS, &flags);
    if (flags & HSA_REGION_GLOBAL_FLAG_FINE_GRAINED) {
        hsa_region_t* ret = (hsa_region_t*) data;
        *ret = region;
        return HSA_STATUS_INFO_BREAK;
    }

    return HSA_STATUS_SUCCESS;
}

hsa_status_t get_global_memory_region_coarse_grained(hsa_region_t region, void* data) {
    hsa_region_segment_t segment;
    hsa_region_get_info(region, HSA_REGION_INFO_SEGMENT, &segment);
    if (HSA_REGION_SEGMENT_GLOBAL != segment) {
        return HSA_STATUS_SUCCESS;
    }

    hsa_region_global_flag_t flags;
    hsa_region_get_info(region, HSA_REGION_INFO_GLOBAL_FLAGS, &flags);
    if (flags & HSA_REGION_GLOBAL_FLAG_COARSE_GRAINED) {
        hsa_region_t* ret = (hsa_region_t*) data;
        *ret = region;
        return HSA_STATUS_INFO_BREAK;
    }

    return HSA_STATUS_SUCCESS;
}
