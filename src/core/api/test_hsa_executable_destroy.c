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

/**
 * Test Name: hsa_executable_destroy
 * Scope: Conformance
 *
 * Purpose: destroy HSA executables with various parameter settings.
 *
 * Test Description:
 * 1. Initialize the HSA runtime, then create an executable with correct setting
 *    so that we have a valid executable.
 * 2. Shutdown HSA runtime, then destroy the executable. Check the return error
 *    code (NOT_INITIALIZED).
 * 3. Initialize the HSA runtime again, then create an executable with correct
 *    setting.
 * 4. Destroy the executable, no error should occur.
 * 5. Destroy the executable again, expect to receive an error code
 *    INVALID_EXECUTABLE.
 * 6. Set the executable's handle to an invalid value, i.e., (uint64_t)-1. Then
 *    destroy the executable, and expect to receive an error code
 *    INVALID_EXECUTABLE.
 * 7. Shutdown the HSA runtime and finish.
 *
 */

#include <stdio.h>
#include <hsa.h>
#include <framework.h>

int test_hsa_executable_destroy() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // create an executable with correct setting
    hsa_executable_t exe;
    status = hsa_executable_create(
        HSA_PROFILE_FULL,
        HSA_EXECUTABLE_STATE_UNFROZEN,
        NULL,
        &exe);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // destroy the executable, no error should occur
    status = hsa_executable_destroy(exe);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    return 0;
}

int test_hsa_executable_destroy_not_initialized() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // create an executable with correct setting
    hsa_executable_t exe;
    status = hsa_executable_create(
        HSA_PROFILE_FULL,
        HSA_EXECUTABLE_STATE_UNFROZEN,
        NULL,
        &exe);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // shutdown HSA runtime and then destroy the executable
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_executable_destroy(exe);
    ASSERT(HSA_STATUS_ERROR_NOT_INITIALIZED == status);
    return 0;
}

int test_hsa_executable_destroy_invalid_executable() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // create an executable with correct setting
    hsa_executable_t exe;
    status = hsa_executable_create(
        HSA_PROFILE_FULL,
        HSA_EXECUTABLE_STATE_UNFROZEN,
        NULL,
        &exe);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // destroy the executable, no error should occur
    status = hsa_executable_destroy(exe);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Destroy the executable again, expect to receive an error code
    // INVALID_EXECUTABLE
    status = hsa_executable_destroy(exe);
    ASSERT(HSA_STATUS_ERROR_INVALID_EXECUTABLE == status);

    // Set the executable's handle to an invalid value, then
    // destroy the executable
    exe.handle = (uint64_t)-1;
    status = hsa_executable_destroy(exe);
    ASSERT(HSA_STATUS_ERROR_INVALID_EXECUTABLE == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    return 0;
}

