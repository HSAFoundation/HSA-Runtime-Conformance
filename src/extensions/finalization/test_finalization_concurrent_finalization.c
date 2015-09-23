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
 * Test Name: finalization_concurrent_finalization
 * Scope: Extension (Finalization)
 * Support: This test assumes that the system supports the finalization
 * extension and that a viable agent that supports that extension
 * can be found.
 *
 * Purpose: Verifies that concurrent program creation and finalization
 * can occur.
 *
 * Test Description:
 * 1) Load a module from a valid source, i.e. brig file.
 * 2) Create several threads that:
 *    a) Create a program object.
 *    b) Add the module to the program.
 *    c) Finalize the program.
 *    d) Extract a kernel symbol from the program.
 *    e) Release all associated resources (not the module).
 * 3) Free the module.
 *
 * Expected Results: All threads should be able to successfully create a
 * program and finalize it.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <hsa.h>
#include <agent_utils.h>
#include <framework.h>
#include <concurrent_utils.h>
#include <finalize_utils.h>

typedef struct concurrent_finalization_params_s {
    char* module_name;
    char* symbol_name;
    hsa_ext_finalizer_pfn_t* pfn;
    hsa_ext_module_t module;
    hsa_agent_t agent;
    hsa_isa_t isa;
    hsa_executable_t executable;
    hsa_code_object_t code_object;
    uint64_t kernel_object;
} concurrent_finalization_params_t;

void thread_proc_finalize(void* data) {
    hsa_status_t status;
    concurrent_finalization_params_t* param = (concurrent_finalization_params_t*)data;

    // Create a program object
    hsa_ext_program_t program;
    program.handle = (uint64_t)-1;
    status = param->pfn->hsa_ext_program_create(
        HSA_MACHINE_MODEL_LARGE,
        HSA_PROFILE_FULL,
        HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT,
        NULL,
        &program);
    ASSERT(HSA_STATUS_SUCCESS == status);
    ASSERT((uint64_t)-1 != program.handle);

    // Add the module to the program
    status = param->pfn->hsa_ext_program_add_module(program, param->module);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Set up a (empty) control directive
    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));
    // Finalize the program and extract the code object
    status = param->pfn->hsa_ext_program_finalize(program,
                                           param->isa,
                                           HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
                                           control_directives,
                                           NULL,
                                           HSA_CODE_OBJECT_TYPE_PROGRAM,
                                           &param->code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create the empty executable
    status = hsa_executable_create(HSA_PROFILE_FULL,
                                   HSA_EXECUTABLE_STATE_UNFROZEN,
                                   NULL,
                                   &param->executable);
    ASSERT(HSA_STATUS_SUCCESS == status);
    // Load the code object
    status = hsa_executable_load_code_object(param->executable, param->agent, param->code_object, NULL);
    ASSERT(HSA_STATUS_SUCCESS == status);
    // Freeze the executable; it can now be queried for symbols
    status = hsa_executable_freeze(param->executable, NULL);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Releasing resources
    param->pfn->hsa_ext_program_destroy(program);

    // Find the executable symbol
    symbol_record_t symbol_record;
    symbol_record.module_name = param->module_name;
    symbol_record.symbol.handle = (uint64_t)-1;
    status = get_executable_symbols(param->executable,
                                    param->agent,
                                    HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
                                    1,
                                    &param->symbol_name,
                                    &symbol_record);
    ASSERT(HSA_STATUS_SUCCESS == status);
    // Query the kernel object handle
    uint64_t kernel_object;
    status = hsa_executable_symbol_get_info(symbol_record.symbol,
                                            HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT,
                                            &param->kernel_object);
    ASSERT(HSA_STATUS_SUCCESS == status);
}

int test_finalization_concurrent_finalization() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Find the agent that supports kernel dispatch
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Get the finalization funtion pointer table
    hsa_ext_finalizer_pfn_t pfn;
    status = get_finalization_fnc_tbl(&pfn);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Load a brig module from a valid source
    char module_name[256] = "no_op.brig";
    char symbol_name[256] = "&__no_op_kernel";
    hsa_ext_module_t module;
    // module.handle = (uint64_t)-1;
    status = load_module_from_file(module_name, &module);
    ASSERT(HSA_STATUS_SUCCESS == status);
    // ASSERT((uint64_t)-1 != module.handle);

    // Get the ISA from the current agent
    hsa_isa_t isa;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &isa);
    ASSERT(HSA_STATUS_SUCCESS == status);

    const int num_threads = 16;
    concurrent_finalization_params_t params[num_threads];
    int ii;
    for (ii = 0; ii < num_threads; ++ii) {
        params[ii].module_name = module_name;
        params[ii].symbol_name = symbol_name;
        params[ii].pfn = &pfn;
        params[ii].module = module;
        params[ii].agent = agent;
        params[ii].isa = isa;
        params[ii].executable.handle = (uint64_t)-1;
        params[ii].kernel_object = 0;
    }

    struct test_group *tg_concurrent_finalization = test_group_create(num_threads);

    for (ii = 0; ii < num_threads; ++ii) {
        test_group_add(tg_concurrent_finalization, &thread_proc_finalize, &params[ii], 1);
    }

    test_group_thread_create(tg_concurrent_finalization);
    test_group_start(tg_concurrent_finalization);
    test_group_wait(tg_concurrent_finalization);
    test_group_exit(tg_concurrent_finalization);
    test_group_destroy(tg_concurrent_finalization);

    // Releasing resources
    destroy_module(module);
    for (ii = 0; ii < num_threads; ++ii) {
        status = hsa_executable_destroy(params[ii].executable);
        ASSERT(HSA_STATUS_SUCCESS == status);
        status = hsa_code_object_destroy(params[ii].code_object);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
