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
 * Test Name: hsa_executable_get_info
 * Scope: Conformance
 *
 * Purpose: get infos from an executable.
 *
 * Test Description:
 * 1. Initialize HSA runtime, then properly create an executable.
 * 2. Query executable infos.
 *
 */

#include <stdio.h>
#include <hsa.h>
#include <framework.h>

int test_hsa_executable_get_info() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_profile_t profiles[2] = {
        HSA_PROFILE_BASE,
        HSA_PROFILE_FULL};
    hsa_executable_state_t states[2] = {
        HSA_EXECUTABLE_STATE_UNFROZEN,
        HSA_EXECUTABLE_STATE_FROZEN};
    int i;
    int j;
    for (i = 0; i < 2; ++i) {
        for (j = 0; j < 2; ++j) {
            hsa_executable_t exe;
            status = hsa_executable_create(profiles[i], states[j], NULL, &exe);
            ASSERT(HSA_STATUS_SUCCESS == status);

            hsa_profile_t profile;
            status = hsa_executable_get_info(exe,
                HSA_EXECUTABLE_INFO_PROFILE, &profile);
            ASSERT(HSA_STATUS_SUCCESS == status);
            ASSERT(profiles[i] == profile);

            hsa_executable_state_t state;
            status = hsa_executable_get_info(exe,
                HSA_EXECUTABLE_INFO_STATE, &state);
            ASSERT(HSA_STATUS_SUCCESS == status);
            ASSERT(states[j] == state);

            hsa_executable_destroy(exe);
            ASSERT(HSA_STATUS_SUCCESS == status);
        }
    }

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);
    return 0;
}


