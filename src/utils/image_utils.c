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
#include <framework.h>
#include <image_utils.h>
#include <stdlib.h>

static char* VERIFY_IMAGE_REGION_KERNEL_1D[3] = {"&__verify_image_region_kernel_s32_1d", "&__verify_image_region_kernel_u32_1d", "&__verify_image_region_kernel_f32_1d"};
static char* VERIFY_IMAGE_REGION_KERNEL_1DA[3] = {"&__verify_image_region_kernel_s32_1da", "&__verify_image_region_kernel_u32_1da", "&__verify_image_region_kernel_f32_1da"};
static char* VERIFY_IMAGE_REGION_KERNEL_1DB[3] = {"&__verify_image_region_kernel_s32_1db", "&__verify_image_region_kernel_u32_1db", "&__verify_image_region_kernel_f32_1db"};
static char* VERIFY_IMAGE_REGION_KERNEL_2D[3] = {"&__verify_image_region_kernel_s32_2d", "&__verify_image_region_kernel_u32_2d", "&__verify_image_region_kernel_f32_2d"};
static char* VERIFY_IMAGE_REGION_KERNEL_2DA[3] = {"&__verify_image_region_kernel_s32_2da", "&__verify_image_region_kernel_u32_2da", "&__verify_image_region_kernel_f32_2da"};
static char* VERIFY_IMAGE_REGION_KERNEL_2DDEPTH[3] = {"&__verify_image_region_kernel_s32_2ddepth", "&__verify_image_region_kernel_u32_2ddepth", "&__verify_image_region_kernel_f32_2ddepth"};
static char* VERIFY_IMAGE_REGION_KERNEL_2DADEPTH[3] = {"&__verify_image_region_kernel_s32_2dadepth", "&__verify_image_region_kernel_u32_2dadepth", "&__verify_image_region_kernel_f32_2dadepth"};
static char* VERIFY_IMAGE_REGION_KERNEL_3D[3] = {"&__verify_image_region_kernel_s32_3d", "&__verify_image_region_kernel_u32_3d", "&__verify_image_region_kernel_f32_3d"};

// Returns the number of bits that are used to represent the scale of the pixel.
uint32_t get_channel_type_bits(hsa_ext_image_channel_type_t channel_type) {
    switch (channel_type) {
        case HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_SHORT_555 :    { return 5; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_SHORT_565 :    { return 5; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_SNORM_INT8 :         { return 7; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_INT8 :         { return 8; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT8 :      { return 8; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT8 :        { return 7; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_SHORT_101010 : { return 10; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_SNORM_INT16 :        { return 15; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_INT16 :        { return 16; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_HALF_FLOAT :         { return 16; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT16 :       { return 15; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_INT24 :        { return 24; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT32 :       { return 31; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT16 :     { return 16; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_FLOAT :              { return 21; }
        case HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT32 :     { return 32; }
        default : { return 0; }
    }

    return 0;
}

int get_kernel_index(hsa_ext_image_channel_type_t channel_type) {
    if (HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT8 == channel_type ||
       HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT16 == channel_type ||
       HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT32 == channel_type) {
        return 0;
    } else if (HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT8 == channel_type ||
              HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT16 == channel_type ||
              HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT32 == channel_type) {
        return 1;
    }

    return 2;
}

// Obtain the image extension function table
hsa_status_t get_image_fnc_tbl(hsa_ext_image_pfn_t* table) {
    bool support;
    hsa_status_t status;

    if (NULL == table) {
        return HSA_STATUS_ERROR_INVALID_ARGUMENT;
    }

    status = hsa_system_extension_supported(HSA_EXTENSION_IMAGES, 1, 0, &support);

    if (HSA_STATUS_SUCCESS != status) {
        goto exit;
    }

    if (!support) {
        status = HSA_STATUS_ERROR;
        goto exit;
    }

    hsa_ext_images_1_00_pfn_t table_1_00;

    status = hsa_system_get_extension_table(HSA_EXTENSION_IMAGES, 1, 0, &table_1_00);

    if (HSA_STATUS_SUCCESS != status) {
        goto exit;
    }

    // Fill in the table.
    table->hsa_ext_image_get_capability = table_1_00.hsa_ext_image_get_capability;
    table->hsa_ext_image_data_get_info = table_1_00.hsa_ext_image_data_get_info;
    table->hsa_ext_image_create = table_1_00.hsa_ext_image_create;
    table->hsa_ext_image_destroy = table_1_00.hsa_ext_image_destroy;
    table->hsa_ext_image_copy = table_1_00.hsa_ext_image_copy;
    table->hsa_ext_image_import = table_1_00.hsa_ext_image_import;
    table->hsa_ext_image_export = table_1_00.hsa_ext_image_export;
    table->hsa_ext_image_clear = table_1_00.hsa_ext_image_clear;
    table->hsa_ext_sampler_create = table_1_00.hsa_ext_sampler_create;
    table->hsa_ext_sampler_destroy = table_1_00.hsa_ext_sampler_destroy;

 exit:

    return status;
}

// Get the geometric information of the image
void get_geometry_info(hsa_agent_t agent,
                       hsa_ext_image_format_t* format,
                       hsa_ext_image_geometry_t geometry,
                       int* image_dimension,
                       uint32_t* max_elements,
                       char** validation_kernel) {
    hsa_agent_info_t max_elements_attribute;
    switch (geometry) {
        case HSA_EXT_IMAGE_GEOMETRY_1D:
            *image_dimension = 1;
            max_elements_attribute = HSA_EXT_AGENT_INFO_IMAGE_1D_MAX_ELEMENTS;
            *validation_kernel = VERIFY_IMAGE_REGION_KERNEL_1D[get_kernel_index(format->channel_type)];
            break;
        case HSA_EXT_IMAGE_GEOMETRY_1DA:
            *image_dimension = 1;
            max_elements_attribute = HSA_EXT_AGENT_INFO_IMAGE_1DA_MAX_ELEMENTS;
            *validation_kernel = VERIFY_IMAGE_REGION_KERNEL_1DA[get_kernel_index(format->channel_type)];
            break;
        case HSA_EXT_IMAGE_GEOMETRY_1DB:
            *image_dimension = 1;
            max_elements_attribute = HSA_EXT_AGENT_INFO_IMAGE_1DB_MAX_ELEMENTS;
            *validation_kernel = VERIFY_IMAGE_REGION_KERNEL_1DB[get_kernel_index(format->channel_type)];
            break;
        case HSA_EXT_IMAGE_GEOMETRY_2D:
            *image_dimension = 2;
            max_elements_attribute = HSA_EXT_AGENT_INFO_IMAGE_2D_MAX_ELEMENTS;
            *validation_kernel = VERIFY_IMAGE_REGION_KERNEL_2D[get_kernel_index(format->channel_type)];
            break;
        case HSA_EXT_IMAGE_GEOMETRY_2DA:
            *image_dimension = 2;
            max_elements_attribute = HSA_EXT_AGENT_INFO_IMAGE_2DA_MAX_ELEMENTS;
            *validation_kernel = VERIFY_IMAGE_REGION_KERNEL_2DA[get_kernel_index(format->channel_type)];
            break;
        case HSA_EXT_IMAGE_GEOMETRY_2DDEPTH:
            *image_dimension = 2;
            max_elements_attribute = HSA_EXT_AGENT_INFO_IMAGE_2DDEPTH_MAX_ELEMENTS;
            *validation_kernel = VERIFY_IMAGE_REGION_KERNEL_2DDEPTH[get_kernel_index(format->channel_type)];
            break;
        case HSA_EXT_IMAGE_GEOMETRY_2DADEPTH:
            *image_dimension = 2;
            max_elements_attribute = HSA_EXT_AGENT_INFO_IMAGE_2DADEPTH_MAX_ELEMENTS;
            *validation_kernel = VERIFY_IMAGE_REGION_KERNEL_2DADEPTH[get_kernel_index(format->channel_type)];
            break;
        case HSA_EXT_IMAGE_GEOMETRY_3D:
            *image_dimension = 3;
            max_elements_attribute = HSA_EXT_AGENT_INFO_IMAGE_3D_MAX_ELEMENTS;
            *validation_kernel = VERIFY_IMAGE_REGION_KERNEL_3D[get_kernel_index(format->channel_type)];
            break;
        default:
            break;
    }

    hsa_status_t status = hsa_agent_get_info(agent,
                                             max_elements_attribute,
                                             max_elements);

    ASSERT(HSA_STATUS_SUCCESS == status);

    max_elements[1] = (2 <= *image_dimension) ? max_elements[1] : 1;
    max_elements[2] = (3 <= *image_dimension) ? max_elements[2] : 1;

    return;
}

// Fill int the comparison data for the compare buffer
uint32_t get_cmp_info(hsa_ext_image_channel_order_t order) {
    switch (order) {
      case HSA_EXT_IMAGE_CHANNEL_ORDER_A: {
          return 0x0001;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_R : {
          return 0x1000;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_RX : {
          return 0x1000;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_RG : {
          return 0x1100;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_RGX : {
          return 0x1100;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_RA : {
          return 0x1001;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_RGB : {
          return 0x1110;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_RGBX : {
          return 0x1110;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_RGBA : {
          return 0x1111;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_BGRA : {
          return 0x1111;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_ARGB : {
          return 0x1111;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_ABGR : {
          return 0x1111;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_SRGB : {
          return 0x1110;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_SRGBX : {
          return 0x1110;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_SRGBA : {
          return 0x1111;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_SBGRA : {
          return 0x1111;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_INTENSITY : {
          return 0x1000;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_LUMINANCE : {
          return 0x1000;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_DEPTH : {
          return 0x1000;
      }
      case HSA_EXT_IMAGE_CHANNEL_ORDER_DEPTH_STENCIL : {
          return 0x1000;
      }
      default: {
      }
    }

    return 0x0000;
}
