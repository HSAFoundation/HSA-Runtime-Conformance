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
 * Test Name: refcount
 * Scope: Conformance
 *
 * Purpose: Verifies that the hsa_init and hsa_shutdown APIs properly increment
 * and decrement reference counting.
 *
 * Test Description:
 * 1) Initialize the HSA runtime with hsa_init by calling that API N times, (N
 * should be large).
 * 2) Verify that the runtime is operational by querying the agent list.
 * 3) Call hsa_shutdown N-1 times.
 * 4) Again, verify the runtime is operational by querying the agent list.
 *
 * Expected Results: The runtime should remain operational when the reference
 * count is positive. Repeated calls to hsa_init should not cause undefined behavior.
 *
 */

#include <hsa.h>
#include <framework.h>

#define N 1000

int test_refcount() {
    hsa_status_t status;

    // Initialize hsa runtime N times
    int ii;
    for (ii = 0; ii < N; ++ii) {
        status = hsa_init();
        ASSERT(status == HSA_STATUS_SUCCESS);
    }

    // Shutdown hsa runtime N - 1 times
    for (ii = 0; ii < N-1; ++ii) {
        status = hsa_shut_down();
        ASSERT(status == HSA_STATUS_SUCCESS);
    }

    status = hsa_shut_down();
    ASSERT(status == HSA_STATUS_SUCCESS);

    return 0;
}
