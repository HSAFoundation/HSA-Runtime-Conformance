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

// Structure alignment macro
#ifndef __ALIGNED__
#if defined(__GNUC__)
#define __ALIGNED__(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#define __ALIGNED__(x) __declspec(align(x))
#else
#error \
    "Your compiler is not recognized.  Add an alignment macro to support it "  \
    "or define __ALIGNED__(x) to the proper alignment property."
#endif
#endif

#define HSA_ARGUMENT_ALIGN_BYTES 16
#define HSA_QUEUE_ALIGN_BYTES 64
#define HSA_PACKET_ALIGN_BYTES 64

#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a < _b ? _a : _b; })

typedef struct __attribute__ ((aligned(HSA_ARGUMENT_ALIGN_BYTES))) kernarg_s {
    void* data;
    uint32_t value;
    uint32_t row_pitch;
    uint32_t slice_pitch;
} kernarg_t;


// void callback_queue_error(hsa_status_t status, hsa_queue_t* queue, void* data);

// Clear the data, launch the kernel, and wait for the execution to complete
void launch_kernel(
        hsa_queue_t* queue,
        uint32_t* data,
        uint32_t total_size,
        uint32_t value,
        int dim,
        hsa_dim3_t grid_dim,
        hsa_dim3_t workgroup_dim,
        uint64_t kernel_obj_address,
        void* kernarg_address);

typedef struct __attribute__ ((aligned(16))) kernarg_memory_s {
    void* in;
    void* out;
    int count;
} kernarg_memory_t;

// Launch a memory kernel (private or group), and wait for the execute to complete
void launch_memory_kernel(
        hsa_queue_t* queue,
        void* in,
        void* out,
        uint32_t data_size,
        uint32_t private_memory_size,
        uint32_t group_memory_size,
        uint64_t kernel_obj_address,
        void* kernarg_address);

#endif  // _TEST_HELPER_FUNC_H_
