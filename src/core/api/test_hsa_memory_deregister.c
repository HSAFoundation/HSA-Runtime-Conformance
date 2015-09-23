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
#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>
#include "test_helper_func.h"

/**
 *
 * Test Name: hsa_memory_deregister
 *
 * Purpose:
 * Verify that if the API works as expected
 *
 * Description:
 *
 * 1) Register a block of memory using hsa_memory_register
 *    and call hsa_memory_deregister; check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Before the runtime is initialized call hsa_memory_deregister and check if the return
 *    value is HSA_STATUS_ERROR_NOT_INITIALIZED
 *
 */

/**
 *
 * @Brief:
 * Implement Description #1
 *
 * @Return:
 * int
 *
 */

int test_hsa_memory_deregister() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    int size = 1024;
    void *addr = 0;

    // Getting a block of memory allocated
    addr = (void*) malloc(sizeof(char) * size);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Registering the allocated memory
    status = hsa_memory_register(addr, size);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_memory_deregister(addr, size);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "The hsa_memory_deregister API failed to properly deregister memory.");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

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

int test_hsa_memory_deregister_not_initialized() {
    hsa_status_t status;
    void *addr;

    status = hsa_memory_deregister(addr, 0);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "The hsa_memory_deregister API failed to return HSA_STATUS_ERROR_NOT_INITIALIZED when called before the runtime was initialized.");

    return 0;
}
