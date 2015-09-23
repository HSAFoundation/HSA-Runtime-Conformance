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

#ifndef _IMAGE_UTILS_H_
#define _IMAGE_UTILS_H_

#include <hsa.h>
#include <hsa_ext_image.h>

typedef struct hsa_ext_image_pfn_s {
  hsa_status_t (*hsa_ext_image_get_capability)(
      hsa_agent_t agent, hsa_ext_image_geometry_t geometry,
      const hsa_ext_image_format_t *image_format, uint32_t *capability_mask);

  hsa_status_t (*hsa_ext_image_data_get_info)(
      hsa_agent_t agent, const hsa_ext_image_descriptor_t *image_descriptor,
      hsa_access_permission_t access_permission,
      hsa_ext_image_data_info_t *image_data_info);

  hsa_status_t (*hsa_ext_image_create)(
      hsa_agent_t agent, const hsa_ext_image_descriptor_t *image_descriptor,
      const void *image_data, hsa_access_permission_t access_permission,
      hsa_ext_image_t *image);

  hsa_status_t (*hsa_ext_image_destroy)(hsa_agent_t agent,
                                        hsa_ext_image_t image);

  hsa_status_t (*hsa_ext_image_copy)(hsa_agent_t agent,
                                     hsa_ext_image_t src_image,
                                     const hsa_dim3_t *src_offset,
                                     hsa_ext_image_t dst_image,
                                     const hsa_dim3_t *dst_offset,
                                     const hsa_dim3_t *range);

  hsa_status_t (*hsa_ext_image_import)(
      hsa_agent_t agent, const void *src_memory, size_t src_row_pitch,
      size_t src_slice_pitch, hsa_ext_image_t dst_image,
      const hsa_ext_image_region_t *image_region);

  hsa_status_t (*hsa_ext_image_export)(
      hsa_agent_t agent, hsa_ext_image_t src_image, void *dst_memory,
      size_t dst_row_pitch, size_t dst_slice_pitch,
      const hsa_ext_image_region_t *image_region);

  hsa_status_t (*hsa_ext_image_clear)(
      hsa_agent_t agent, hsa_ext_image_t image, const void *data,
      const hsa_ext_image_region_t *image_region);

  hsa_status_t (*hsa_ext_sampler_create)(
      hsa_agent_t agent, const hsa_ext_sampler_descriptor_t *sampler_descriptor,
      hsa_ext_sampler_t *sampler);

  hsa_status_t (*hsa_ext_sampler_destroy)(hsa_agent_t agent,
                                          hsa_ext_sampler_t sampler);
} hsa_ext_image_pfn_t;

// Obtain the image extension function table
hsa_status_t get_image_fnc_tbl(hsa_ext_image_pfn_t* table);

// Get information about a specific image geometry on target agent.
void get_geometry_info(hsa_agent_t agent,
                       hsa_ext_image_format_t* format,
                       hsa_ext_image_geometry_t geometry,
                       int* image_dimension,
                       uint32_t* max_elements,
                       char** validation_kernel);

// Get information about pixel comparison operations.
uint32_t get_cmp_info(hsa_ext_image_channel_order_t order);

// Get the number of bits per pixel in for the specified channel type.
uint32_t get_channel_type_bits(hsa_ext_image_channel_type_t channel_type);

#endif  // _IMAGE_UTILS_H_
