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

/**
 *
 * Test Name: hsa_system_get_extension_table
 *
 * Purpose:
 * Verify that if the API hsa_system_get_extension_table API
 * works as expected.
 *
 * Description:
 *
 * 1) After initializing the hsa Runtime, call
 *    hsa_system_get_extension_table API  using a known
 *    extension. Check if the return value is HSA_STATUS_SUCCESS.
 *
 * 2) Before init hsa Runtime, call hsa_system_get_extension_table API.
 *    Check if the return value is HSA_STATUS_ERROR_NOT_INITIALIZED.
 *
 * 3) Call hsa_system_extension_supported API with an invalid
 *    extension. Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT.
 *
 * 4) Call hsa_system_extension_supported API with a NULL result
 *    parameter. Check if the return value is HSA_STATUS_ERROR_INVALID_ARGUMENT
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

int test_hsa_system_get_extension_table() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);


    hsa_ext_images_1_00_pfn_t images_table;
    status = hsa_system_get_extension_table(HSA_EXTENSION_IMAGES, 1, 0, &images_table);
    ASSERT_MSG(HSA_STATUS_SUCCESS == status, "Failed to get the hsa_ext_images extension's function table.\n");

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

int test_hsa_system_get_extension_table_not_initialized() {
    bool result;
    hsa_status_t status;

    hsa_ext_images_1_00_pfn_t images_table;
    status = hsa_system_get_extension_table(HSA_EXTENSION_IMAGES, 1, 0, &images_table);
    ASSERT_MSG(HSA_STATUS_ERROR_NOT_INITIALIZED == status, "The hsa_system_get_extension_table API failed to return HSA_STATUS_ERROR_NOT_INITIALIZED when the runtime was not initialized.\n");

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

int test_hsa_system_get_extension_table_invalid_extension() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    int table;
    status = hsa_system_extension_supported(-1, 1, 0, (void*) &table);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_system_get_extension_table API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when an invalid extension was specified.\n");

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

int test_hsa_system_get_extension_table_null_table_ptr() {
    hsa_status_t status;

    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_system_get_extension_table(HSA_EXTENSION_IMAGES, 1, 0, NULL);
    ASSERT_MSG(HSA_STATUS_ERROR_INVALID_ARGUMENT == status, "The hsa_system_get_extension_table API failed to return HSA_STATUS_ERROR_INVALID_ARGUMENT when a NULL table pointer was used.\n");

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
