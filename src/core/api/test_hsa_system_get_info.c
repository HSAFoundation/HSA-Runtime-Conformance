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
#include <framework.h>
#include <stdint.h>

/**
 *
 * Test Name: hsa_system_get_info
 *
 * Purpose:
 * Verify that if the API works as expected
 *
 * Description:
 *
 * 1) After init HsaRt, call hsa_system_get_info to exam every system attribute,
 *    Check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Before init HsaRt, call hsa_system_get_info, and check if the return value
 *    is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) After init HsaRt, call hsa_system_get_info, and pass a invalid hsa_system_info_t value
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 * 4) After init HsaRt, call hsa_system_get_info, and pass a NULL pointer to void*
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
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

int test_hsa_system_get_info() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get info of HSA_SYSTEM_INFO_VERSION_MAJOR
    {
        uint16_t major_version = 0;
        status = hsa_system_get_info(HSA_SYSTEM_INFO_VERSION_MAJOR, &major_version);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get major version info.\n");
    }

    // Get info of HSA_SYSTEM_INFO_VERSION_MINOR
    {
        uint16_t minor_version = 0;
        status = hsa_system_get_info(HSA_SYSTEM_INFO_VERSION_MINOR, &minor_version);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get minor version info\n");
    }

    // Get info of HSA_SYSTEM_INFO_TIMESTAMP
    {
        uint64_t time_stamp = 0;
        status = hsa_system_get_info(HSA_SYSTEM_INFO_TIMESTAMP, &time_stamp);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get time stamp.\n");
    }

    // Get info of HSA_SYSTEM_INFO_TIMESTAMP_FREQUENCY
    {
        uint16_t frequency = 0;
        status = hsa_system_get_info(HSA_SYSTEM_INFO_TIMESTAMP_FREQUENCY, &frequency);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get time stamp frequency.\n");
    }

    // Get info of HSA_SYSTEM_INFO_SIGNAL_MAX_WAIT
    {
        uint64_t max_wait = 0;
        status = hsa_system_get_info(HSA_SYSTEM_INFO_SIGNAL_MAX_WAIT, &max_wait);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get signal max wait.\n");
    }

    // Get info of HSA_SYSTEM_INFO_ENDIANNESS
    {
        hsa_endianness_t endianness;
        status = hsa_system_get_info(HSA_SYSTEM_INFO_ENDIANNESS, &endianness);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get endianness.\n");
    }

    // Get info of HSA_SYSTEM_INFO_MACHINE_MODEL
    {
        hsa_machine_model_t machine_model;
        status = hsa_system_get_info(HSA_SYSTEM_INFO_MACHINE_MODEL, &machine_model);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get machine_model.\n");
    }

    // Get info of HSA_SYSTEM_INFO_EXTENSIONS
    {
        uint8_t extensions[128];
        status = hsa_system_get_info(HSA_SYSTEM_INFO_EXTENSIONS, &extensions);
        ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get extensions.\n");
    }

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

int test_hsa_system_get_info_not_initialized() {
    hsa_status_t status;

    uint16_t major_version = 0;
    status = hsa_system_get_info(HSA_SYSTEM_INFO_VERSION_MAJOR, &major_version);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "Failed to return proper value when calling hsa_system_get_info before initialization.\n");

    return 0;
}

/**
 *
 * @Brief:
 * Implement description #3
 *
 * @Return:
 * int
 *
 */

int test_hsa_system_get_info_invalid_attribute() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // here, pass integer 10 to attribute

    uint64_t test = 0;
    status = hsa_system_get_info(10, &test);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "Failed to return proper value when passing invalid attribute to hsa_system_get_info.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

/**
 *
 * @Brief:
 * Implement Description #4
 *
 * @Return:
 * int
 *
 */

int test_hsa_system_get_info_invalid_ptr() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_system_get_info(HSA_SYSTEM_INFO_VERSION_MAJOR, NULL);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "Failed to return proper value when passing NULL.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
