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
#include <hsa_ext_image.h>
#include <hsa_ext_finalize.h>
#include <framework.h>

#define STATUS_STRING_TEST(status_code) \
{\
    hsa_status_t s; \
    const char* status_string = NULL; \
    s = hsa_status_string(HSA_STATUS_SUCCESS, &status_string); \
    ASSERT_MSG(HSA_STATUS_SUCCESS == s && NULL != status_string, "Failed to return proper value for #status_code!\n"); \
}\

/**
 *
 * Test Name: hsa_status_string
 *
 * Purpose:
 * Verify that if the API hsa_status_string works as expected
 *
 * Description:
 *
 * 1) After init hsa Runtime, call hsa_status_string with different status value
 *    Check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Before init hsa Runtime, call hsa_status_string
 *    Check if the return value is HSA_STATUS_ERROR_NOT_INITIALIZED
 *
 * 3) After init hsa Runtime, call hsa_status_string with invalid hsa_status_t
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 * 4) After init hsa Runtime, call hsa_status_string with NULL pointer
 *    Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 */

/**
 *
 * @Brief:
 * Implement Description #1
 *
 * @Return
 * int
 *
 */

int test_hsa_status_string() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    STATUS_STRING_TEST(HSA_STATUS_SUCCESS);
    STATUS_STRING_TEST(HSA_STATUS_INFO_BREAK);
    STATUS_STRING_TEST(HSA_STATUS_ERROR);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_ARGUMENT);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_QUEUE_CREATION);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_ALLOCATION);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_AGENT);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_REGION);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_SIGNAL);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_QUEUE);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_OUT_OF_RESOURCES);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_PACKET_FORMAT);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_RESOURCE_FREE);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_NOT_INITIALIZED);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_REFCOUNT_OVERFLOW);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INCOMPATIBLE_ARGUMENTS);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_INDEX);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_ISA);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_ISA_NAME);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_CODE_OBJECT);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_EXECUTABLE);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_FROZEN_EXECUTABLE);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_INVALID_SYMBOL_NAME);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_VARIABLE_ALREADY_DEFINED);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_VARIABLE_UNDEFINED);
    STATUS_STRING_TEST(HSA_STATUS_ERROR_EXCEPTION);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}

/**
 *
 * @Brief:
 * Implement Description #2
 *
 * @Return
 * int
 *
 */

int test_hsa_status_string_not_initialized() {
    hsa_status_t status;
    const char* status_string = NULL;

    status = hsa_status_string(HSA_STATUS_SUCCESS, &status_string);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "The hsa_status_string API failed to return proper value when the runtime was not initialized.\n");
    return 0;
}

/**
 *
 * @Brief:
 * Implement Description #3
 *
 * @Return
 * int
 *
 */

int test_hsa_status_string_invalid_status() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    const char* status_string = NULL;

    status = hsa_status_string((uint32_t) -1, &status_string);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "Calling hsa_status_string() with an invalid status code didn't return HSA_STATUS_ERROR_INVALID_ARGUMENT.\n");

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

int test_hsa_status_string_invalid_ptr() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_status_string(HSA_STATUS_SUCCESS, NULL);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "Calling hsa_status_string() with an NULL string pointer didn't return HSA_STATUS_ERROR_INVALID_ARGUMENT.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
