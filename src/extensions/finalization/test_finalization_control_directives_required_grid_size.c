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
 * Test Name: finalization_control_directives_required_grid_size
 * Scope: Extension (Finalization)
 *
 * Test Description:
 *   1) With a value greater than the value specified by the requiredgridsize directive
 *   2) Less than the value specified in the requiredgridsize directive
 *   3) With a value equal to the requiredgridsize directive*
 * Expected Results: The finalization will fail if the value is greater than requiredgridsize,
 * and return corresponding error code. Finalization will success if the value is less or equal
 * to the one specified by requiredgridsize.
 *
 */

#include <stdlib.h>
#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include "test_helper_func.h"

int test_finalization_control_directives_required_grid_size() {
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

    // Load a brig module from a valid source
    char module_name[256] = "control_device.brig";
    char symbol_name[256] = "&__control_device_kernel";
    hsa_ext_module_t module;
    status = load_module_from_file(module_name, &module);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Add the module to the program
    status = pfn.hsa_ext_program_add_module(program, module);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Get the ISA from the current agent
    hsa_isa_t isa;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &isa);
    ASSERT(HSA_STATUS_SUCCESS == status);
    // Set up a (empty) control directive
    hsa_ext_control_directives_t control_directives;
    memset(&control_directives, 0, sizeof(hsa_ext_control_directives_t));

    // Let the required_grid_size to be greater than specified in kernel
    control_directives.required_grid_size[0]= 2000;
    control_directives.required_grid_size[1]= 2000;
    control_directives.required_grid_size[2]= 2000;

    // Finalize the program, expect to fail
    hsa_code_object_t code_object;
    status = pfn.hsa_ext_program_finalize(program, isa,
                                          HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
                                          control_directives,
                                          NULL,
                                          HSA_CODE_OBJECT_TYPE_PROGRAM,
                                          &code_object);
    // Expected to fail and receive the following error message
    ASSERT(HSA_EXT_STATUS_ERROR_DIRECTIVE_MISMATCH == status);

    // Let the required_grid_size to be less than specified in kernel
    control_directives.required_grid_size[0]= 500;
    control_directives.required_grid_size[1]= 500;
    control_directives.required_grid_size[2]= 500;
    
    // Finalize the program, expect to fail
 
    status = pfn.hsa_ext_program_finalize(program, isa,
                                          HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
                                          control_directives,
                                          NULL,
                                          HSA_CODE_OBJECT_TYPE_PROGRAM,
                                          &code_object);
    // Expected to fail and receive the following error message
    ASSERT(HSA_EXT_STATUS_ERROR_DIRECTIVE_MISMATCH == status);


    // Let the maximum dynamic group size to match the value specified in kernel
    control_directives.required_grid_size[0]= 1000;
    control_directives.required_grid_size[1]= 1000;
    control_directives.required_grid_size[2]= 1000;

    status = pfn.hsa_ext_program_finalize(program, isa,
                                          HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
                                          control_directives,
                                          NULL,
                                          HSA_CODE_OBJECT_TYPE_PROGRAM,
                                          &code_object);
    // Expected to be finalized successfully
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Create the empty executable
    hsa_executable_t executable;
    status = hsa_executable_create(HSA_PROFILE_FULL, HSA_EXECUTABLE_STATE_UNFROZEN, NULL, &executable);
    ASSERT(HSA_STATUS_SUCCESS == status);
    // Load the code object
    status = hsa_executable_load_code_object(executable, agent, code_object, NULL);
    ASSERT(HSA_STATUS_SUCCESS == status);
    // Freeze the executable; it can now be queried for symbols
    status = hsa_executable_freeze(executable, NULL);
    ASSERT(HSA_STATUS_SUCCESS == status);
    // Releasing resources
    destroy_module(module);
    pfn.hsa_ext_program_destroy(program);

    // Find the executable symbol
    symbol_record_t symbol_record;
    symbol_record.module_name = module_name;
    symbol_record.symbol.handle = (uint64_t)-1;
    char* symbol_name_ptr = &(symbol_name[0]);
    status = get_executable_symbols(executable,
                                    agent,
                                    HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
                                    1,
                                    &symbol_name_ptr,
                                    &symbol_record);
    ASSERT(HSA_STATUS_SUCCESS == status);
    // Query the kernel object handle
    uint64_t kernel_object;
    status = hsa_executable_symbol_get_info(symbol_record.symbol,
                                            HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT,
                                            &kernel_object);
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Dispatch the kernel
    hsa_queue_t* queue;
    status = hsa_queue_create(agent, 256, HSA_QUEUE_TYPE_SINGLE, NULL, NULL, UINT32_MAX, UINT32_MAX, &queue);
    ASSERT(HSA_STATUS_SUCCESS == status);
    launch_kernel_no_kernarg(queue, kernel_object, 1);

    // Release resources
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
