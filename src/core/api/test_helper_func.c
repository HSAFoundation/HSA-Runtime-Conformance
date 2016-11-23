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
#include <finalize_utils.h>
#include <framework.h>
#include <stdlib.h>
#include <unistd.h>
#include "test_helper_func.h"

// Callback function to get the number of agents
hsa_status_t callback_get_num_agents(hsa_agent_t agent, void* data) {
    int *num_agents = (int *)data;
    (*num_agents)++;
    return HSA_STATUS_SUCCESS;
}

// Callback function to get the list of agents
hsa_status_t callback_get_agents(hsa_agent_t agent, void* data) {
    hsa_agent_t **agent_list = (hsa_agent_t **)data;
    **agent_list = agent;
    (*agent_list)++;
    return HSA_STATUS_SUCCESS;
}

// Callback function to get the first agent that supports kernel dispatch
hsa_status_t callback_get_kernel_dispatch_agent(hsa_agent_t agent, void* data) {
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

// Check if the input is the power of two
char isPowerOfTwo_local(uint32_t x) {
    while (((x & 1) == 0) && x > 1) /*  While x is even and > 1 */
        x >>= 1;
    return (x == 1);
}

// Check if every attribute of the agent is valid
void check_attributes(hsa_agent_t agent) {
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
    ASSERT(feature == HSA_AGENT_FEATURE_KERNEL_DISPATCH ||
           feature == HSA_AGENT_FEATURE_AGENT_DISPATCH);

    if (HSA_AGENT_FEATURE_KERNEL_DISPATCH == feature) {
        uint32_t wavefront_size = 0;
        status = hsa_agent_get_info(agent,
                                    HSA_AGENT_INFO_WAVEFRONT_SIZE,
                                    &wavefront_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT_MSG((wavefront_size >= 1) &&
                   (wavefront_size <= 256) &&
                   isPowerOfTwo_local(wavefront_size),
                   "Error: wavefront_size = %u", wavefront_size);

        uint32_t workgroup_max_size;
        status =
             hsa_agent_get_info(agent,
                                HSA_AGENT_INFO_WORKGROUP_MAX_SIZE,
                                &workgroup_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT_MSG(workgroup_max_size > 0,
                  "Error: workgroup_max_size = %u",
                  workgroup_max_size);

        uint16_t workgroup_max_dim[3];
        workgroup_max_dim[0] = 0;
        workgroup_max_dim[1] = 0;
        workgroup_max_dim[2] = 0;
        status =
             hsa_agent_get_info(agent,
                                HSA_AGENT_INFO_WORKGROUP_MAX_DIM,
                                workgroup_max_dim);
        ASSERT(HSA_STATUS_SUCCESS == status);
        for (ii = 0; ii < 3; ii++) {
            ASSERT_MSG((workgroup_max_dim[ii] > 0 &&
                        workgroup_max_dim[ii] <= workgroup_max_size),
                        "Error: workgroup_max_dim[%d] = %u",
                        ii,
                        workgroup_max_dim[ii]);
        }

        uint32_t grid_max_size;
        status =
             hsa_agent_get_info(agent,
                                HSA_AGENT_INFO_GRID_MAX_SIZE,
                                &grid_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(grid_max_size > 0);

        hsa_dim3_t grid_max_dim;
        status = hsa_agent_get_info(agent,
                                    HSA_AGENT_INFO_GRID_MAX_DIM,
                                    &grid_max_dim);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(grid_max_dim.x > 0 &&
               grid_max_dim.x >= workgroup_max_dim[0] &&
               grid_max_dim.x <= grid_max_size);
        ASSERT(grid_max_dim.y > 0 &&
               grid_max_dim.y >= workgroup_max_dim[1] &&
               grid_max_dim.y <= grid_max_size);
        ASSERT(grid_max_dim.z > 0 &&
               grid_max_dim.z >= workgroup_max_dim[2] &&
               grid_max_dim.z <= grid_max_size);

        uint32_t fbarriers_max_size;
        status = hsa_agent_get_info(agent,
                                    HSA_AGENT_INFO_FBARRIER_MAX_SIZE,
                                    &fbarriers_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT_MSG(fbarriers_max_size >= 32,
                   "Error: fbarriers_max_size = %u", fbarriers_max_size);
    }

    uint32_t queues_max;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_QUEUES_MAX, &queues_max);
    ASSERT(HSA_STATUS_SUCCESS == status);

    uint32_t queue_max_size;
    status = hsa_agent_get_info(agent,
                                HSA_AGENT_INFO_QUEUE_MAX_SIZE,
                                &queue_max_size);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT(queue_max_size > 0 && isPowerOfTwo_local(queue_max_size));

    uint32_t node;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_NODE, &node);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_device_type_t device;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &device);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT(device == HSA_DEVICE_TYPE_GPU ||
           device == HSA_DEVICE_TYPE_CPU ||
           device == HSA_DEVICE_TYPE_DSP);

    uint32_t cache_size[4];
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_CACHE_SIZE, cache_size);
    ASSERT(HSA_STATUS_SUCCESS == status);

    if (device == HSA_DEVICE_TYPE_GPU) {
        uint32_t image1d_max_elems;
        status = hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_1D_MAX_ELEMENTS,
                                    &image1d_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image1d_max_elems <= 16384);

        uint32_t image1da_max_elems;
        status = hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_1DA_MAX_ELEMENTS,
                                    &image1da_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image1da_max_elems <= 16384);

        uint32_t image1db_max_elems;
        status = hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_1DB_MAX_ELEMENTS,
                                    &image1db_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image1db_max_elems <= 65536);

        uint32_t image2d_max_elems[2];
        status = hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_2D_MAX_ELEMENTS,
                                    image2d_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image2d_max_elems[0] <= 16384 && image2d_max_elems[1] <= 16384);

        uint32_t image2da_max_elems[2];
        status = hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_2DA_MAX_ELEMENTS,
                                    image2da_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image2da_max_elems[0] <= 16384 &&
               image2da_max_elems[1] <= 16384);

        uint32_t image2dd_max_elems[2];
        status =
             hsa_agent_get_info(agent,
                                HSA_EXT_AGENT_INFO_IMAGE_2DDEPTH_MAX_ELEMENTS,
                                image2dd_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image2dd_max_elems[0] <= 16384 &&
               image2dd_max_elems[1] <= 16384);

        uint32_t image2dad_max_elems[2];
        status =
             hsa_agent_get_info(agent,
                                HSA_EXT_AGENT_INFO_IMAGE_2DADEPTH_MAX_ELEMENTS,
                                image2dad_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image2dad_max_elems[0] <= 16384 &&
               image2dad_max_elems[1] <= 16384);

        uint32_t image3d_max_elems[3];
        status = hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_3D_MAX_ELEMENTS,
                                    image3d_max_elems);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image3d_max_elems[0] <= 2048 &&
               image3d_max_elems[1] <= 2048 &&
               image3d_max_elems[2] <= 2048);


        uint32_t image_array_max_layers;
        status = hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_IMAGE_ARRAY_MAX_LAYERS,
                                    &image_array_max_layers);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image_array_max_layers <= 2048);

        uint32_t image_rd_max;
        status = hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_MAX_IMAGE_RD_HANDLES,
                                    &image_rd_max);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image_rd_max >= 128);

        uint32_t image_rorw_max;
        status = hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_MAX_IMAGE_RORW_HANDLES,
                                    &image_rorw_max);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(image_rorw_max >= 64);

        uint32_t sampler_max;
        status = hsa_agent_get_info(agent,
                                    HSA_EXT_AGENT_INFO_MAX_SAMPLER_HANDLERS,
                                    &sampler_max);
        ASSERT(HSA_STATUS_SUCCESS == status);
        ASSERT(sampler_max >= 16);
    }

    return;
}

hsa_status_t callback_check_agents(hsa_agent_t agent, void* data) {
    // Check attributes of the agent
    check_attributes(agent);

    // Keep iterating
    return HSA_STATUS_SUCCESS;
}

void check_agents() {
    hsa_status_t status;
    const char *err_str;
    status = hsa_iterate_agents(callback_check_agents, NULL);
    hsa_status_string(status, &err_str);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status,
               "\nErr_code: %d Err_Str: %s\n",
               status, err_str);
    return;
}

// Callback function to get the number of regions of an agent
hsa_status_t callback_get_num_regions(hsa_region_t region, void* data) {
    int *num_regions = (int *)data;
    (*num_regions)++;
    return HSA_STATUS_SUCCESS;
}

// Callback function to get the list of regions of an agent
hsa_status_t callback_get_regions(hsa_region_t region, void* data) {
    hsa_region_t **region_list = (hsa_region_t **)data;
    **region_list = region;
    (*region_list)++;
    return HSA_STATUS_SUCCESS;
}

// Callback function to get a global memory region that allow allocation
hsa_status_t callback_get_region_global_allocatable(hsa_region_t region,
                                                    void* data) {
    hsa_status_t status;
    hsa_region_segment_t segment;
    status = hsa_region_get_info(region, HSA_REGION_INFO_SEGMENT, &segment);
    ASSERT(HSA_STATUS_SUCCESS == status);
    bool runtime_alloc_allowed;
    status = hsa_region_get_info(region,
                                 HSA_REGION_INFO_RUNTIME_ALLOC_ALLOWED,
                                 &runtime_alloc_allowed);
    ASSERT(HSA_STATUS_SUCCESS == status);
    if (HSA_REGION_SEGMENT_GLOBAL == segment && runtime_alloc_allowed) {
        hsa_region_t* ret = (hsa_region_t*)data;
        *ret = region;
        return HSA_STATUS_INFO_BREAK;
    }
    return HSA_STATUS_SUCCESS;
}

void launch_no_op_kernels(hsa_agent_t* agent,
                          hsa_queue_t* queue,
                          int num_packets) {
    hsa_status_t status;

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("no_op.brig", &module));

    // Finalize the executable
    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0 , sizeof(hsa_ext_control_directives_t));
    hsa_code_object_t code_object;
    hsa_executable_t executable;

    status = finalize_executable(*agent,
                                 1,
                                 &module,
                                 HSA_MACHINE_MODEL_LARGE,
                                 HSA_PROFILE_FULL,
                                 HSA_DEFAULT_FLOAT_ROUNDING_MODE_ZERO,
                                 HSA_CODE_OBJECT_TYPE_PROGRAM,
                                 0,
                                 control_directives,
                                 &code_object,
                                 &executable);

    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the symbol and the symbol info
    symbol_record_t symbol_record;
    memset(&symbol_record, 0, sizeof(symbol_record_t));

    char* symbol_names[1];
    symbol_names[0] = "&__no_op_kernel";
    status = get_executable_symbols(executable,
                                    *agent,
                                    0,
                                    1,
                                    symbol_names,
                                    &symbol_record);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Signal and dispatch packet
    hsa_signal_t* signals =
                  (hsa_signal_t*) malloc(sizeof(hsa_signal_t) * num_packets);
    hsa_kernel_dispatch_packet_t dispatch_packet;

    int jj;
    for (jj = 0; jj < num_packets; ++jj) {
        status = hsa_signal_create(1, 0, NULL, &signals[jj]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Get size of dispatch_packet
    const size_t packet_size = sizeof(hsa_kernel_dispatch_packet_t);

    // Fill info for the default dispatch_packet
    memset(&dispatch_packet, 0, packet_size);
    dispatch_packet.header |=
                 HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
    dispatch_packet.header |=
                 HSA_FENCE_SCOPE_AGENT << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
    dispatch_packet.setup |=
                 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
    dispatch_packet.workgroup_size_x = 256;
    dispatch_packet.workgroup_size_y = 1;
    dispatch_packet.workgroup_size_z = 1;
    dispatch_packet.grid_size_x = 256;
    dispatch_packet.grid_size_y = 1;
    dispatch_packet.grid_size_z = 1;
    dispatch_packet.group_segment_size = symbol_record.group_segment_size;
    dispatch_packet.private_segment_size = symbol_record.private_segment_size;
    dispatch_packet.kernel_object = symbol_record.kernel_object;
    dispatch_packet.kernarg_address = 0;

    // Enqueue dispatch packets
    hsa_kernel_dispatch_packet_t* queue_packet;
    for (jj = 0; jj < num_packets; ++jj) {
        // Increment the write index of the queue
        uint64_t write_index = hsa_queue_add_write_index_relaxed(queue, 1);
        // Set the value fo the dispatch packet to the correct signal
        dispatch_packet.completion_signal = signals[jj];
        // Obtain the address of the queue packet entry
        queue_packet =
            (hsa_kernel_dispatch_packet_t*) (queue->base_address +
                                             (write_index % queue->size) * packet_size);
        // Copy the initialized packet to the queue packet entry
        memcpy(queue_packet, &dispatch_packet, packet_size);
        // Set the queue packet entries header.type value
        // to HSA_PACKET_TYPE_KERNEL_DISPATCH
        // This allows the command processor to process this packet.
        queue_packet->header |=
                   HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
        // Ring the doorbell
        hsa_signal_store_relaxed(queue->doorbell_signal, write_index);
    }

    // Wait until all dispatch packets finish executing
    for (jj = 0; jj < num_packets; ++jj) {
        hsa_signal_value_t value =
                              hsa_signal_wait_relaxed(signals[jj],
                                                      HSA_SIGNAL_CONDITION_EQ,
                                                      0,
                                                      UINT64_MAX,
                                                      HSA_WAIT_STATE_BLOCKED);
        ASSERT(0 == value);
    }

    // Destroy signals
    for (jj = 0; jj < num_packets; ++jj) {
        status = hsa_signal_destroy(signals[jj]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }
    free(signals);

    // Destroy the loaded module
    destroy_module(module);

    // Destroy the executable
    hsa_executable_destroy(executable);

    // Destroy the code object
    hsa_code_object_destroy(code_object);

    return;
}

hsa_code_object_t load_code_object(hsa_agent_t* agent,
                                   char* file_name,
                                   char* kernel_name) {
    hsa_status_t status;

/*
    // Load the BRIG module for the target kernel.
    hsa_ext_brig_module_t* brig_module;
    brig_utils_status_t brig_status;
    brig_status = create_brig_module_from_brig_file(file_name, &brig_module);
    ASSERT(BRIG_UTILS_STATUS_SUCCESS == brig_status);

    // Create HSA program
    hsa_ext_program_handle_t program;
    status = hsa_ext_program_create(agent, 1, HSA_EXT_BRIG_MACHINE_LARGE, HSA_EXT_BRIG_PROFILE_FULL, &program);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Add the Brig module to HSA program
    hsa_ext_brig_module_handle_t module;
    status = hsa_ext_add_module(program, brig_module, &module);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Construct finalization request list.
    hsa_ext_finalization_request_t finalization_request_list;
    finalization_request_list.module = module;
    finalization_request_list.program_call_convention = 0;
    brig_status = find_symbol_offset(brig_module, kernel_name, &finalization_request_list.symbol);
    ASSERT(BRIG_UTILS_STATUS_SUCCESS == brig_status);

    // Finalize the HSA program
    status = hsa_ext_finalize_program(program, *agent, 1, &finalization_request_list, NULL, NULL, 0, NULL, 0);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Free the BRIG data loaded from the brig file.
    brig_status = destroy_brig_module(brig_module);
    ASSERT(BRIG_UTILS_STATUS_SUCCESS == brig_status);

    // Get the HSA code descriptor address.
    hsa_ext_code_descriptor_t* hsa_code_descriptor;
    status = hsa_ext_query_kernel_descriptor_address(program, module, finalization_request_list.symbol, &hsa_code_descriptor);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_code_object_t code_object;
    code_object.handle = hsa_code_descriptor->code.handle;
*/
    hsa_code_object_t code_object;
    code_object.handle = 0;
    return code_object;
}

hsa_status_t callback_serialize_alloc(size_t size,
                                      hsa_callback_data_t data,
                                      void** address) {
    *address = malloc(size);
    return HSA_STATUS_SUCCESS;
}
