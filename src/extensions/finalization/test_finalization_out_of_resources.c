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

#include <stdlib.h>
#include <stdio.h>
#include <hsa.h>
#include <agent_utils.h>
#include <finalize_utils.h>
#include <framework.h>
#include "test_helper_func.h"

int test_finalization_out_of_resources() {
    // Initialize the HSA Runtime
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    // Find the agent that supports kernel dispatch
    hsa_agent_t agent;
    agent.handle = (uint64_t)-1;
    status = hsa_iterate_agents(get_kernel_dispatch_agent, &agent);
    ASSERT((uint64_t)-1 != agent.handle);

    // Get the finalization function pointer table
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
    char module_name[256] = "no_op.brig";
    char symbol_name[256] = "&__no_op_kernel";
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

    // Finalize the program and extract the code object
    const uint32_t OBJECT_MAX = 4096;
    int object_count = 0;
    hsa_code_object_t code_object[OBJECT_MAX];
    while(object_count < OBJECT_MAX) {
        status = pfn.hsa_ext_program_finalize(program, isa,
                                           HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO,
                                           control_directives,
                                           NULL,
                                           HSA_CODE_OBJECT_TYPE_PROGRAM,
                                           &code_object[object_count]);
        if(HSA_STATUS_ERROR_OUT_OF_RESOURCES == status) {
            break;
        } else if(HSA_STATUS_SUCCESS == status) {
            object_count++;
            continue;
        } else {
            ASSERT(0);
        }
    }

    // Releasing resources
    destroy_module(module);
    pfn.hsa_ext_program_destroy(program);
    uint32_t i;
    for (i = 0; i < object_count; i++) {
        status = hsa_code_object_destroy(code_object[i]);
        ASSERT(HSA_STATUS_SUCCESS == status);
    }
    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
