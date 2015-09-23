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
 * Test Name: finalization_multiple_modules
 * Scope: Extension (Finalization)
 * Support: This test assumes that the system supports the finalization
 * extension and that a viable agent that supports that extension
 * can be found.
 *
 * Purpose: Verify that a program that has several independent modules
 * added to it can be finalized, and that the kernels from each
 * of the modules can be executed correctly.
 *
 * Test Description:
 * 1) Create a hsa_ext_program_t object.
 * 2) Load several hsa_ext_module_t objects and add them to
 *    the program. The modules should not have interdependencies.
 * 3) Finalize the program.
 * 4) Extract symbols (kernels) associated with each module from the finalized
 *    program.
 * 5) Dispatch each of the kernels on a valid agent.
 *
 * Expected Results: The program should be properly finalized and all kernels
 * should execute successfully.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <hsa.h>
#include <framework.h>
#include <finalize_utils.h>
#include <agent_utils.h>
#include "test_helper_func.h"

int test_finalization_multiple_modules() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    //  Find the agent that supports kernel dispatch
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    //  Get the finalization funtion pointer table
    hsa_ext_finalizer_pfn_t pfn;
    status = get_finalization_fnc_tbl(&pfn);
    ASSERT(HSA_STATUS_SUCCESS == status);

    //  Create a program object
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

    const int num_modules = 2;
    char* module_names[] = {
        "no_op.brig",
        "no_op2.brig"};
    char* symbol_names[] = {
        "&__no_op_kernel",
        "&__no_op2_kernel"};
    hsa_ext_module_t modules[num_modules];

    int ii;
    for (ii = 0; ii < num_modules; ++ii) {
        //  Load the brig module from a file
        //  modules[ii].handle = (uint64_t)-1;
        int load_status = load_module_from_file(module_names[ii], &modules[ii]);
        ASSERT(0 == load_status);
        //  ASSERT((uint64_t)-1 != modules[ii].handle);

        //  Add the module to the program
        status = pfn.hsa_ext_program_add_module(program, modules[ii]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }

    //  Get the ISA from the current agent
    hsa_isa_t isa;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &isa);
    ASSERT(HSA_STATUS_SUCCESS == status);

    //  Set up a (empty) control directive
    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));

    //  Finalize the program and extract the code object
    hsa_code_object_t code_object;
    status = pfn.hsa_ext_program_finalize(program, isa,
                                           HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
                                           control_directives,
                                           NULL,
                                           HSA_CODE_OBJECT_TYPE_PROGRAM,
                                           &code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);


    //  Create the empty executable
    hsa_executable_t executable;
    status = hsa_executable_create(HSA_PROFILE_FULL, HSA_EXECUTABLE_STATE_UNFROZEN, NULL, &executable);
    ASSERT(HSA_STATUS_SUCCESS == status);
    //  Load the code object
    status = hsa_executable_load_code_object(executable, agent, code_object, NULL);
    ASSERT(HSA_STATUS_SUCCESS == status);
    //  Freeze the executable; it can now be queried for symbols
    status = hsa_executable_freeze(executable, NULL);
    ASSERT(HSA_STATUS_SUCCESS == status);
    //  Releasing resources
    for (ii = 0; ii < num_modules; ++ii) {
        destroy_module(modules[ii]);
    }
    pfn.hsa_ext_program_destroy(program);

    //  Find the executable symbols for dispatch
    symbol_record_t symbol_records[num_modules];
    for (ii = 0; ii < num_modules; ++ii) {
        symbol_records[ii].module_name = module_names[ii];
        symbol_records[ii].symbol.handle = (uint64_t)-1;
    }
    char** symbol_names_ptr = (char**)(&symbol_names);
    status = get_executable_symbols(executable,
                                    agent,
                                    HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
                                    num_modules,
                                    symbol_names_ptr,
                                    symbol_records);
    ASSERT(HSA_STATUS_SUCCESS == status);

    //  Create a queue for dispatch
    hsa_queue_t* queue;
    status = hsa_queue_create(agent, 256, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
    ASSERT(HSA_STATUS_SUCCESS == status);

    for (ii = 0; ii < num_modules; ++ii) {
        //  Query the kernel object handle
        uint64_t kernel_object;
        status = hsa_executable_symbol_get_info(symbol_records[ii].symbol,
                                                HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT,
                                                &kernel_object);
        ASSERT(HSA_STATUS_SUCCESS == status);
        //  Dispatch the kernel
        launch_kernel_no_kernarg(queue, kernel_object, 1);
    }

    //  Release resources
    status = hsa_executable_destroy(executable);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_code_object_destroy(code_object);
    ASSERT(HSA_STATUS_SUCCESS == status);
    status = hsa_queue_destroy(queue);
    ASSERT(HSA_STATUS_SUCCESS == status);

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
