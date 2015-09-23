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

#ifndef _TEST_HELPER_FUNC_H_
#define _TEST_HELPER_FUNC_H_

#include <hsa.h>
#include <hsa_ext_finalize.h>

#define ARGUMENT_ALIGN_BYTES 16

// vector_copy kernarg
typedef struct __attribute__ ((aligned(ARGUMENT_ALIGN_BYTES))) kernarg_vector_copy_s {
    void* in;
    void* out;
} kernarg_vector_copy_t;

// launch the vector_copy kernel, and wait for the kernel to finish
void launch_vector_copy_kernel(
        hsa_queue_t* queue,
        uint32_t data_size,
        uint64_t kernel_obj_address,
        void*    kernarg_address);

// struct to store memory region info
typedef struct region_info_s {
    hsa_region_segment_t segment;
    hsa_region_global_flag_t flags;
    size_t size;
    size_t alloc_max_size;
    bool alloc_allowed;
    size_t alloc_granule;
    size_t alloc_alignemnt;
} region_info_t;


// get memory region info
void get_region_info(hsa_region_t region, region_info_t* info);

// init_data kernarg
typedef struct __attribute__ ((aligned(ARGUMENT_ALIGN_BYTES))) kernarg_init_data_s {
    void* data;
    uint32_t value;
    uint32_t row_pitch;
    uint32_t slice_pitch;
} kernarg_init_data_t;

// clear the data, launch the init_data kernel, and wait for the execution to complete.
void launch_init_data_kernel(
        hsa_queue_t* queue,
        uint32_t* data,
        uint32_t total_size,
        uint32_t value,
        int dim,
        hsa_dim3_t grid_dim,
        hsa_dim3_t workgroup_dim,
        uint64_t kernel_obj_address,
        void* kernarg_address);

// The kernarg data structure for group_memory_dynamic_allocation
typedef struct __attribute__ ((aligned(ARGUMENT_ALIGN_BYTES))) kernarg_group_memory_dynamic_alloc_s {
    uint32_t* data_in;
    uint32_t* data_out;
    uint32_t grp_offset;
    uint32_t count;
} kernarg_group_memory_dynamic_alloc_t;

#endif  // _TEST_HELPER_FUNC_H_
