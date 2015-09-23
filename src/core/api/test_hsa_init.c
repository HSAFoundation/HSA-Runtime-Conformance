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

#include <stdio.h>
#include <hsa.h>
#include <framework.h>

/**
 *
 * Test Name: hsa_init
 *
 * Purpose: Verify that API of hsa_init works as expected
 *
 * Description:
 *
 * 1) Open an new instance of HSA runtime;
 *    Check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Keep opening instance of HSA runtime until it reaches MAX number
 *    Check if the return value is HSA_STATUS_ERROR_REFCOUNT_OVERFLOW.
 *
 */

/**
 *
 * @Brief:
 * Simply open one instance of HSA runtime.
 *
 * @Return:
 * int
 *
 */

int test_hsa_init() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "The call to hsa_init() failed.\n");

    return 0;
}

/**
 *
 * @Brief:
 * Simply open instances of HSA runtime as much as possible until it reaches MAX number.
 *
 * @Return:
 * int
 *
 */

int test_hsa_init_MAX() {
    hsa_status_t status;

    int i;
    for (i = 0; i < INT32_MAX; ++i) {
        status = hsa_init();
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    status = hsa_init();
    ASSERT_MSG(HSA_STATUS_ERROR_REFCOUNT_OVERFLOW == status, "The HSA_STATUS_ERROR_REFCOUNT_OVERFLOW error code wasn't returned on the INT32_MAX+1 call to hsa_init().\n");

    return 0;
}
