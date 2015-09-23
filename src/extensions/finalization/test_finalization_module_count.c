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
 * Test Name: finalization_module_count
 * Scope: Extension (Finalization)
 * Support: This test assumes that the system supports the finalization
 * extension and that a viable agent that supports that extension
 * can be found.
 *
 * Purpose: Verify that as modules are added to a program, the
 * hsa_ext_program_iterate_modules API will properly count the
 * number of modules added.
 *
 * Test Description:
 * 1) Create a hsa_ext_program_t object.
 * 2) Load several hsa_ext_module_t objects.
 * 3) Add a module to the program.
 * 4) Use the hsa_ext_program_iterate_modules to count the number
 *    of modules in the program, and verify the count.
 * 5) Repeat 3 and 4 several times.
 *
 * Expected Results: The hsa_ext_program_iterate_modules API should
 * properly iterate over all added modules.
 *
 */

#include <stdlib.h>
#include <hsa.h>
#include <finalize_utils.h>
#include <framework.h>
#include "test_helper_func.h"

hsa_status_t callback_count_modules(hsa_ext_program_t program,
                                    hsa_ext_module_t module,
                                    void* data) {
    int* count_modules = (int*)data;
    ++(*count_modules);
    return HSA_STATUS_SUCCESS;
}

int test_finalization_module_count() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the finalization funtion pointer table
    hsa_ext_finalizer_pfn_t pfn;
    status = get_finalization_fnc_tbl(&pfn);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create a program object
    hsa_ext_program_t program;
    program.handle = (uint64_t)-1;
    status = pfn.hsa_ext_program_create(
        HSA_MACHINE_MODEL_LARGE,
        HSA_PROFILE_FULL,
        HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT,
        NULL,
        &program);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != program.handle);

    const int num_modules = 3;
    char* module_names[] = {
        "no_op.brig",
        "init_data.brig",
        "vector_copy.brig"};
    hsa_ext_module_t modules[num_modules];

    int ii;
    for (ii = 0; ii < num_modules; ++ii) {
        // Load the brig module from a file
        status = load_module_from_file(module_names[ii], &modules[ii]);
        ASSERT(HSA_STATUS_SUCCESS == status);

        // Add the module to the program
        status = pfn.hsa_ext_program_add_module(program, modules[ii]);
        ASSERT(HSA_STATUS_SUCCESS == status);

        int count_modules = 0;
        status = pfn.hsa_ext_program_iterate_modules(program,
                                                     callback_count_modules,
                                                     &count_modules);
        ASSERT(ii+1 == count_modules);
    }

    // Releasing resources
    for (ii = 0; ii < num_modules; ++ii) {
        destroy_module(modules[ii]);
    }
    pfn.hsa_ext_program_destroy(program);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
