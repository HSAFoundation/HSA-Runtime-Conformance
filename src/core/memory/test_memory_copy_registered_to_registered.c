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
 * Test Name: memory_copy_registered_to_registered
 *
 * Purpose: Tests that the hsa_memory_copy API can copy data between memory
 * registered by the hsa runtime registration API's.
 *
 * Test Description:
 *
 * 1. Allocate two buffers from system memory and register them using the
 *    hsa_memory_register API. Denote one as the source buffer and the
 *    other as the destination buffer.
 * 2. Initialize both buffers to different values.
 * 3. Use the hsa_memory_copy API to copy the source values to the
 *    destination buffer.
 *
 * Expected Results: The data from the source should be
 * successfully copied to the destination.
 *
 */

#include <hsa.h>
#include <framework.h>
#include <stdlib.h>

int test_memory_copy_registered_to_registered() {
    hsa_status_t status;
    const uint32_t block_size = 1024;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    uint32_t *src_buffer = (uint32_t *)malloc(block_size* sizeof(uint32_t));
    ASSERT(src_buffer != NULL);

    uint32_t *dst_buffer = (uint32_t *)malloc(block_size* sizeof(uint32_t));
    ASSERT(src_buffer != NULL);

    // Register the memory
    status = hsa_memory_register(src_buffer, block_size * sizeof(uint32_t));
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_memory_register(dst_buffer, block_size * sizeof(uint32_t));
    ASSERT(HSA_STATUS_SUCCESS == status);

    int kk;
    for (kk = 0; kk < block_size; ++kk) {
        src_buffer[kk] = kk;
    }
    memset(dst_buffer, 0, sizeof(uint32_t) * block_size);

    status = hsa_memory_copy(dst_buffer, src_buffer, block_size * sizeof(uint32_t));
    ASSERT(HSA_STATUS_SUCCESS == status);

    for (kk = 0; kk < block_size; ++kk) {
        ASSERT(dst_buffer[kk] == kk);
    }

    // Deregister the memory
    status = hsa_memory_deregister(src_buffer, block_size * sizeof(uint32_t));
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_memory_deregister(dst_buffer, block_size * sizeof(uint32_t));
    ASSERT(HSA_STATUS_SUCCESS == status);

    free(src_buffer);
    free(dst_buffer);

    // Shutdown the runtime
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
