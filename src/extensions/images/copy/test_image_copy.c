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
 * Test Name: image_copy_<type>_<order>_<geometry>
 *
 * Purpose: Verifies that if an image with the format and geometry specified
 * by the channel type, channel order and image geometry is supported on an agent
 * it an be successfully copied from one image to another using the hsa_ext_image_copy API.
 *
 * Test Description:
 * 1) Check each agent on the platform and determine if they support an image with
 * channel type = <type>, channel order = <order> and geometry = <geometry>.
 * Use the hsa_ext_image_get_capability to do this.
 *
 * 2) If the agent supports the target format and geometry, query the agent using
 * hsa_ext_image_data_get_info to determine the maximum size of the image on the agent.
 *
 * 3) Use hsa_ext_image_get_info to determine the size and alignment required for the image's
 * backing buffer. The image permissions should be read/write and the dimensions should
 * be the maximum queried from the previous step.
 *
 * 4) Allocate the backing buffer with hsa_memory_allocate from an appropriate memory region
 * associated with the agent. Do this for both a source and destination image.
 *
 * 5) Create both source and destination images on the agent using the backing buffer
 * allocated in the previous step.
 *
 * 6) Use the hsa_ext_image_clear API to clear the both images with two different data patterns.
 *
 * 7) Use the hsa_ext_image_copy API to copy a region from the source image to the destination
 * image.
 *
 * 8) Use the verify image kernel to verify the region was properly copied.
 *
 * 11) Repeat steps 6 to 8 until for all sub-regions of the images.
 *
 * Expected results: The regions specified by the hsa_ext_image_copy API are the only
 * ones that should be affected.
 *
 */

#include <hsa.h>
#include <hsa_ext_image.h>
#include <hsa_ext_finalize.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include <image_utils.h>
#include <queue_utils.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int test_image_copy(hsa_ext_image_format_t*  image_format,
                    hsa_ext_image_geometry_t image_geometry) {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the images function pointer table
    hsa_ext_image_pfn_t pfn;
    status = get_image_fnc_tbl(&pfn);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load the BRIG module
    hsa_ext_module_t module;
    ASSERT(0 == load_module_from_file("verify_image_region.brig", &module));

    // Get the list of agents
    struct agent_list_s agent_list;
    get_agent_list(&agent_list);

    // Structures for querying format capabilities.
    uint32_t capability_mask;

    // Repeat the test for each agent
    int ii;
    for (ii = 0; ii < agent_list.num_agents; ++ii) {
        uint32_t features = 0;
        status = hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_FEATURE, &features);
        ASSERT(HSA_STATUS_SUCCESS == status);
        if (!(features & HSA_AGENT_FEATURE_KERNEL_DISPATCH)) {
            continue;
        }

        // Get the destination image's format capability mask.
        status = pfn.hsa_ext_image_get_capability(agent_list.agents[ii],
                                                  image_geometry,
                                                  image_format,
                                                  &capability_mask);
        ASSERT(HSA_STATUS_SUCCESS == status);

        if (HSA_EXT_IMAGE_CAPABILITY_NOT_SUPPORTED == capability_mask) {
            printf("The destination image format is not supported.\n");
            continue;
        }

        if (!(HSA_EXT_IMAGE_CAPABILITY_READ_ONLY & capability_mask) &&
           !(HSA_EXT_IMAGE_CAPABILITY_READ_WRITE & capability_mask) &&
           !(HSA_EXT_IMAGE_CAPABILITY_READ_MODIFY_WRITE & capability_mask)) {
            printf("The destination image format cannot be tested.\n");
            continue;
        }

        uint32_t grid_max_size;
        hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_GRID_MAX_SIZE, &grid_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        uint32_t grid_max_dim[3];
        hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_GRID_MAX_DIM, &grid_max_dim);
        ASSERT(HSA_STATUS_SUCCESS == status);

        uint32_t work_group_max_size;
        hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_WORKGROUP_MAX_SIZE, &work_group_max_size);
        ASSERT(HSA_STATUS_SUCCESS == status);

        uint32_t work_group_max_dim[3];
        hsa_agent_get_info(agent_list.agents[ii], HSA_AGENT_INFO_WORKGROUP_MAX_DIM, &work_group_max_dim);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Find a global memory region for the image backing buffers
        hsa_region_t global_region;
        global_region.handle = (uint64_t) -1;
        status = hsa_agent_iterate_regions(agent_list.agents[ii], get_global_memory_region_fine_grained, &global_region);
        ASSERT(global_region.handle != (uint64_t) -1);

        // Find a memory region that supports kernel arguments
        hsa_region_t kernarg_region;
        kernarg_region.handle = (uint64_t) -1;
        hsa_agent_iterate_regions(agent_list.agents[ii], get_kernarg_memory_region, &kernarg_region);
        ASSERT((uint64_t) -1 != kernarg_region.handle);

        // Define the region step array
        uint32_t region_step[3];

        // Get information regarding the images on this agent using the specified
        // geometry.
        int image_dimension = 0;
        uint32_t max_elements[3];
        char* validation_kernel[1];

        get_geometry_info(agent_list.agents[ii],
                          image_format,
                          image_geometry,
                          &image_dimension,
                          max_elements,
                          &validation_kernel[0]);

        // Adjust max_elements values
        max_elements[0] = (max_elements[0] < grid_max_dim[0]) ? max_elements[0] : grid_max_dim[0];
        max_elements[1] = (max_elements[1] < grid_max_dim[1]) ? max_elements[1] : grid_max_dim[1];
        max_elements[2] = (max_elements[2] < grid_max_dim[2]) ? max_elements[2] : grid_max_dim[2];
        max_elements[0] = (max_elements[0] < 1024) ? max_elements[0] : 1024;
        max_elements[1] = (max_elements[1] < 1024) ? max_elements[1] : 1024;
        max_elements[2] = (max_elements[2] < 8) ? max_elements[2] : 8;
        ASSERT((max_elements[0] * max_elements[1] * max_elements[2]) < grid_max_size);

        // Adjust region step size values
        region_step[0] = region_step[1] = 256;
        region_step[2] = 1;
        region_step[0] = (region_step[0] < max_elements[0]) ? region_step[0] : max_elements[0];
        region_step[1] = (region_step[1] < max_elements[1]) ? region_step[1] : max_elements[1];
        region_step[2] = (region_step[2] < max_elements[2]) ? region_step[2] : max_elements[2];

        // Adjust the work_group_max_dim sizes
        work_group_max_dim[0] = (work_group_max_dim[0] < 16) ? work_group_max_dim[0] : 16;
        work_group_max_dim[1] = (work_group_max_dim[1] < 16) ? work_group_max_dim[1] : 16;
        work_group_max_dim[2] = (work_group_max_dim[2] < 1) ? work_group_max_dim[2] : 1;
        work_group_max_dim[0] = (work_group_max_dim[0] < max_elements[0]) ? work_group_max_dim[0] : max_elements[0];
        work_group_max_dim[1] = (work_group_max_dim[1] < max_elements[1]) ? work_group_max_dim[1] : max_elements[1];
        work_group_max_dim[2] = (work_group_max_dim[2] < max_elements[2]) ? work_group_max_dim[2] : max_elements[2];
        ASSERT((work_group_max_dim[0] * work_group_max_dim[1] * work_group_max_dim[2]) <= work_group_max_size);

        // Create a queue to execute validation kernels.
        hsa_queue_t *queue;
        status = hsa_queue_create(agent_list.agents[ii], 1024, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Finalize the executable
        hsa_ext_control_directives_t control_directives;
        memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
        hsa_code_object_t code_object;
        hsa_executable_t executable;

        status = finalize_executable(agent_list.agents[ii],
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

        // Get the symbol and the symbol info for the validation kernel
        symbol_record_t symbol_record;
        memset(&symbol_record, 0, sizeof(symbol_record_t));

        status = get_executable_symbols(executable, agent_list.agents[ii], 0, 1, &validation_kernel[0], &symbol_record);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Define the validation kernel arguments
        typedef struct __attribute__ ((aligned(16))) validate_args_s {
            hsa_ext_image_t image;  // The image handle
            void* rgn_values;  // The floating point pixel pattern in the specified region
            void* bkg_values;  // The floating point pixel pattern in the rest of the image
            uint32_t* start_region;  // The regions starting coords
            uint32_t* end_region;  // The regions ending coords
            uint32_t* bits;  // The channel values to compare
            uint32_t* cmp_mask;  // The channel values to compare
            uint32_t* error;  // An error field representing different rbga channel errors
        } validate_args_t;

        // Allocate the kernel argument buffer from the correct region
        validate_args_t* kernarg_buffer = NULL;
        status = hsa_memory_allocate(kernarg_region,
                                     symbol_record.kernarg_segment_size,
                                     (void**)(&kernarg_buffer));

        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the pattern buffers for the image
        void* bg_pattern;
        void* clr_pattern;

        if (image_format->channel_type == HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT8  ||
           image_format->channel_type == HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT16 ||
           image_format->channel_type == HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT32) {
            status = hsa_memory_allocate(global_region, 4 * sizeof(uint32_t), (void**) &bg_pattern);
            ASSERT(HSA_STATUS_SUCCESS == status);
            uint32_t *bg = (uint32_t*) bg_pattern;
            bg[0] = bg[1] = bg[2] = bg[3] = 0;
            status = hsa_memory_allocate(global_region, 4 * sizeof(uint32_t), (void**) &clr_pattern);
            ASSERT(HSA_STATUS_SUCCESS == status);
            uint32_t *clr = (uint32_t*) clr_pattern;
            clr[0] = clr[1] = clr[2] = clr[3] = 255;
        } else if (image_format->channel_type == HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT8  ||
           image_format->channel_type == HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT16 ||
           image_format->channel_type == HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT32) {
            status = hsa_memory_allocate(global_region, 4 * sizeof(int32_t), (void**) &bg_pattern);
            ASSERT(HSA_STATUS_SUCCESS == status);
            int32_t *bg = (int32_t*) bg_pattern;
            bg[0] = bg[1] = bg[2] = bg[3] = 0;
            status = hsa_memory_allocate(global_region, 4 * sizeof(int32_t), (void**) &clr_pattern);
            ASSERT(HSA_STATUS_SUCCESS == status);
            int32_t *clr = (int32_t*) clr_pattern;
            clr[0] = clr[1] = clr[2] = clr[3] = 127;
        } else {
            status = hsa_memory_allocate(global_region, 4 * sizeof(float), (void**) &bg_pattern);
            ASSERT(HSA_STATUS_SUCCESS == status);
            float *bg = (float*) bg_pattern;
            bg[0] = bg[1] = bg[2] = bg[3] = 0.0f;
            status = hsa_memory_allocate(global_region, 4 * sizeof(float), (void**) &clr_pattern);
            ASSERT(HSA_STATUS_SUCCESS == status);
            float *clr = (float*) clr_pattern;
            clr[0] = clr[1] = clr[2] = clr[3] = 0.5f;
        }

        // Create the start and end region buffers
        uint32_t* start_region;
        uint32_t* end_region;
        status = hsa_memory_allocate(global_region, 3 * sizeof(uint32_t), (void**) &start_region);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_allocate(global_region, 3 * sizeof(uint32_t), (void**) &end_region);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create the error, mask and bits buffers
        uint32_t* error;
        uint32_t* cmp_mask;
        uint32_t* bits;
        status = hsa_memory_allocate(global_region, sizeof(uint32_t), (void**) &error);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_allocate(global_region, sizeof(uint32_t), (void**) &cmp_mask);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_allocate(global_region, sizeof(uint32_t), (void**) &bits);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Add all of the components to the argument buffer
        *cmp_mask = get_cmp_info(image_format->channel_order);
        *bits = get_channel_type_bits(image_format->channel_type);
        kernarg_buffer->rgn_values = clr_pattern;
        kernarg_buffer->bkg_values = bg_pattern;
        kernarg_buffer->start_region = start_region;
        kernarg_buffer->end_region = end_region;
        kernarg_buffer->bits = bits;
        kernarg_buffer->cmp_mask = cmp_mask;
        kernarg_buffer->error = error;

        // Determine the size and alignment for the source image backing buffer
        hsa_ext_image_descriptor_t image_descriptor;
        image_descriptor.geometry = image_geometry;
        image_descriptor.width  = max_elements[0];
        image_descriptor.height = max_elements[1];
        image_descriptor.depth  = max_elements[2];
        image_descriptor.array_size = 1;
        image_descriptor.format.channel_type = image_format->channel_type;
        image_descriptor.format.channel_order = image_format->channel_order;

        hsa_ext_image_data_info_t image_info;
        hsa_access_permission_t access_permissions = HSA_ACCESS_PERMISSION_RW;

        status = hsa_ext_image_data_get_info(agent_list.agents[ii],
                                             &image_descriptor,
                                             access_permissions,
                                             &image_info);

        ASSERT(HSA_STATUS_SUCCESS == status);

        // Verify that the memory region will correctly align the
        // image data.
        size_t region_align;
        status = hsa_region_get_info(global_region, HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT, &region_align);

        ASSERT((region_align >= image_info.alignment) && (region_align % image_info.alignment == 0));

        // Allocate the backing buffer for the source image
        hsa_ext_image_t src_image;
        void* src_image_data;
        status = hsa_memory_allocate(global_region, image_info.size, (void**) &src_image_data);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create an image with the backing buffer.
        status = pfn.hsa_ext_image_create(agent_list.agents[ii],
                                          &image_descriptor,
                                          src_image_data,
                                          access_permissions,
                                          &src_image);


        ASSERT(HSA_STATUS_SUCCESS == status);

        // Allocate the backing buffer for the source image
        hsa_ext_image_t dst_image;
        void* dst_image_data;
        status = hsa_memory_allocate(global_region, image_info.size, (void**) &dst_image_data);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Create an image with the backing buffer.
        status = pfn.hsa_ext_image_create(agent_list.agents[ii],
                                          &image_descriptor,
                                          dst_image_data,
                                          access_permissions,
                                          &dst_image);

        // Create a completion signal
        hsa_signal_t completion_signal;
        status = hsa_signal_create(1, 0, NULL, &completion_signal);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Setup the dispatch packet
        hsa_kernel_dispatch_packet_t dispatch_packet;
        memset(&dispatch_packet, 0, sizeof(hsa_kernel_dispatch_packet_t));
        dispatch_packet.setup |= image_dimension << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
        dispatch_packet.header |= HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
        dispatch_packet.header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;
        dispatch_packet.header |= 1 << HSA_PACKET_HEADER_BARRIER;
        dispatch_packet.kernel_object = symbol_record.kernel_object;
        dispatch_packet.group_segment_size = symbol_record.group_segment_size;
        dispatch_packet.private_segment_size = symbol_record.private_segment_size;
        dispatch_packet.kernarg_address = (void*) kernarg_buffer;
        dispatch_packet.completion_signal = completion_signal;
        dispatch_packet.workgroup_size_x = work_group_max_dim[0];
        dispatch_packet.workgroup_size_y = work_group_max_dim[1];
        dispatch_packet.workgroup_size_z = work_group_max_dim[2];
        dispatch_packet.grid_size_x = max_elements[0];
        dispatch_packet.grid_size_y = max_elements[1];
        dispatch_packet.grid_size_z = max_elements[2];

        // Define the regions
        hsa_ext_image_region_t region_all;
        hsa_ext_image_region_t region_partial;
        region_all.offset.x = 0;
        region_all.offset.y = 0;
        region_all.offset.z = 0;
        region_all.range.x = max_elements[0];
        region_all.range.y = max_elements[1];
        region_all.range.z = max_elements[2];

        size_t x_offset, y_offset, z_offset;

        // Clear various regions of the images, checking for validity each iteration
        for (x_offset = 0; x_offset < max_elements[0]; x_offset += region_step[0]) {
            region_partial.offset.x = x_offset;
            region_partial.range.x = (region_step[0] <= (max_elements[0] - x_offset)) ? region_step[0] : max_elements[0] % region_step[0];
            if (region_partial.range.x <= 0) {
                continue;
            }
            start_region[0] = region_partial.offset.x;
            end_region[0] = start_region[0] + region_partial.range.x;

            for (y_offset = 0; y_offset < max_elements[1]; y_offset += region_step[1]) {
                region_partial.offset.y = y_offset;
                region_partial.range.y = (region_step[1] <= (max_elements[1] - y_offset)) ? region_step[1] : max_elements[1] % region_step[1];
                if (region_partial.range.y <= 0) {
                    continue;
                }
                start_region[1] = region_partial.offset.y;
                end_region[1] = start_region[1] + region_partial.range.y;

                for (z_offset = 0; z_offset < max_elements[2]; z_offset += region_step[2]) {
                    region_partial.offset.z = z_offset;
                    region_partial.range.z = (region_step[2] <= (max_elements[2] - y_offset)) ? region_step[2] : max_elements[1] % region_step[2];
                    if (region_partial.range.z <= 0) {
                        continue;
                    }
                    start_region[2] = region_partial.offset.z;
                    end_region[2] = start_region[2] + region_partial.range.z;

                    // Clear the entire source image to the clr_pattern.
                    status = pfn.hsa_ext_image_clear(agent_list.agents[ii],
                                                     src_image,
                                                     clr_pattern,
                                                     &region_all);

                    ASSERT(HSA_STATUS_SUCCESS == status);

                    // Clear the entire destination image to the bg_pattern.
                    status = pfn.hsa_ext_image_clear(agent_list.agents[ii],
                                                     dst_image,
                                                     bg_pattern,
                                                     &region_all);

                    ASSERT(HSA_STATUS_SUCCESS == status);

                    // Copy the partial region of the source image to the destination image
                    status = pfn.hsa_ext_image_copy(agent_list.agents[ii],
                                                    src_image,
                                                    &region_partial.offset,
                                                    dst_image,
                                                    &region_partial.offset,
                                                    &region_partial.range);

                    ASSERT(HSA_STATUS_SUCCESS == status);

                    kernarg_buffer->image = dst_image;

                    *kernarg_buffer->error = 0;

                    // Dispatch the kernel
                    enqueue_dispatch_packet(queue, &dispatch_packet);

                    // Wait on the completion signal
                    hsa_signal_value_t value;
                    do {
                        value = hsa_signal_wait_relaxed(completion_signal, HSA_SIGNAL_CONDITION_LT, 1, UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
                    } while (0 != value);

                    // Verify that no errors occured.
                    if (0 != *kernarg_buffer->error) {
                        printf("\nRegion: (%d,%d,%d) -> (%d,%d,%d) Error: %d\n",
                                                                    start_region[0],
                                                                    start_region[1],
                                                                    start_region[2],
                                                                    end_region[0],
                                                                    end_region[1],
                                                                    end_region[2],
                                                                    *kernarg_buffer->error);
                        ASSERT(0 == *kernarg_buffer->error);
                    }

                    printf(".");

                    // Reset the signal value
                    hsa_signal_store_release(completion_signal, 1);
                }
            }
        }

        // Destroy the completion signal
        hsa_signal_destroy(completion_signal);

        // Destroy the images
        status = pfn.hsa_ext_image_destroy(agent_list.agents[ii], src_image);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = pfn.hsa_ext_image_destroy(agent_list.agents[ii], dst_image);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free the backing buffers
        status = hsa_memory_free(src_image_data);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_free(dst_image_data);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free the kernarg_buffer
        status = hsa_memory_free(kernarg_buffer);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the executable
        status = hsa_executable_destroy(executable);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the code object
        status = hsa_code_object_destroy(code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free the clear and background patterns
        status = hsa_memory_free(bg_pattern);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_free(clr_pattern);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Free all the utility buffers
        status = hsa_memory_free(start_region);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_free(end_region);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_free(error);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_free(cmp_mask);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_memory_free(bits);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Destroy the queue
        status = hsa_queue_destroy(queue);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    // Shutdown HSA
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
