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
 * Test Name: hsa_executable_create
 * Scope: Conformance
 *
 * Purpose: create HSA executables with various parameter settings.
 *
 * Test Description:
 * 1. Create an executable without initializing the HSA runtime.
 * 2. Initialize the HSA runtime, then create executables with correct settings.
 * 3. Create executables with invalid arguments: profile and exe_ptr.
 * 4. Create executables until out-of-resource.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <hsa.h>
#include <framework.h>



int test_hsa_executable_create() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // create executables with correct settings
    hsa_executable_t exe[4];
    status = hsa_executable_create(
        HSA_PROFILE_FULL,
        HSA_EXECUTABLE_STATE_UNFROZEN,
        NULL,
        exe);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_executable_create(
        HSA_PROFILE_BASE,
        HSA_EXECUTABLE_STATE_UNFROZEN,
        NULL,
        exe + 1);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_executable_create(
        HSA_PROFILE_FULL,
        HSA_EXECUTABLE_STATE_FROZEN,
        NULL,
        exe + 2);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_executable_create(
        HSA_PROFILE_BASE,
        HSA_EXECUTABLE_STATE_FROZEN,
        NULL,
        exe + 3);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_executable_destroy(exe[0]);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_executable_destroy(exe[1]);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_executable_destroy(exe[2]);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_executable_destroy(exe[3]);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

// create an executable without initializing the HSA runtime
int test_hsa_executable_create_not_initialized() {
    hsa_status_t status;
    hsa_executable_t exe;

    status = hsa_executable_create(
        HSA_PROFILE_FULL,
        HSA_EXECUTABLE_STATE_UNFROZEN,
        NULL,
        &exe);
    ASSERT(HSA_STATUS_ERROR_NOT_INITIALIZED == status);

    return 0;
}

// create an executable with invalid arguments: null exe_ptr
int test_hsa_executable_create_invalid_argument() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_executable_create(
        HSA_PROFILE_FULL,
        HSA_EXECUTABLE_STATE_UNFROZEN,
        NULL,
        NULL);
    ASSERT(HSA_STATUS_ERROR_INVALID_ARGUMENT == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    return 0;
}

// create executables until out-of-resource
int test_hsa_executable_create_out_of_resources() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // may need a larger "executables_max" (65k x 1k) to reach to an OOR state.
    const uint32_t executables_max = 65536;
    hsa_executable_t* exe_array;
    exe_array = (hsa_executable_t*)malloc(sizeof(hsa_executable_t) * executables_max);
    uint32_t executable_count = 0;
    uint32_t i;
    for (i = 0; i < executables_max; ++i) {
        status = hsa_executable_create(
            HSA_PROFILE_FULL,
            HSA_EXECUTABLE_STATE_UNFROZEN,
            NULL,
            exe_array + i);
        if (HSA_STATUS_SUCCESS == status) {
            ++executable_count;
        } else if (HSA_STATUS_ERROR_OUT_OF_RESOURCES == status) {
            break;
        } else {
            // unexpected error
            ASSERT(0);
        }
    }
    for (i = 0; i < executable_count; ++i) {
        status = hsa_executable_destroy(exe_array[i]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }
    free(exe_array);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    return 0;
}
