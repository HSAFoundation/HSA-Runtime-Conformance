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

#ifndef _FINALIZE_UTILS_H_
#define _FINALIZE_UTILS_H_

#include <hsa.h>
#include <hsa_ext_finalize.h>

typedef struct symbol_record_s {
    hsa_executable_symbol_t symbol;
    hsa_symbol_kind_t type;
    uint32_t name_length;
    char* name;
    uint32_t module_name_length;
    char* module_name;
    hsa_agent_t agent;
    uint64_t variable_address;
    hsa_symbol_linkage_t linkage;
    hsa_variable_allocation_t variable_allocation;
    hsa_variable_segment_t variable_segment;
    uint32_t variable_alignment;
    uint32_t variable_size;
    bool is_const;
    uint64_t kernel_object;
    uint32_t kernarg_segment_size;
    uint32_t kernarg_segment_alignment;
    uint32_t group_segment_size;
    uint32_t private_segment_size;
    bool dynamic_callstack;
    uint64_t indirect_function_object;
    uint32_t indirect_function_call_convention;
} symbol_record_t;

typedef struct hsa_ext_finalizer_pfn_s {
    hsa_status_t (*hsa_ext_program_create)(hsa_machine_model_t machine_model,
                                           hsa_profile_t profile,
                                           hsa_default_float_rounding_mode_t default_float_rounding_mode,
                                           const char *options,
                                           hsa_ext_program_t *program);

    hsa_status_t (*hsa_ext_program_destroy)(hsa_ext_program_t program);

    hsa_status_t (*hsa_ext_program_add_module)(hsa_ext_program_t program,
                                               hsa_ext_module_t module);

    hsa_status_t (*hsa_ext_program_iterate_modules)(
        hsa_ext_program_t program,
        hsa_status_t (*callback)(hsa_ext_program_t program,
                                 hsa_ext_module_t module, void *data),
                                 void *data);

    hsa_status_t (*hsa_ext_program_get_info)(hsa_ext_program_t program,
                                             hsa_ext_program_info_t attribute,
                                             void *value);

    hsa_status_t (*hsa_ext_program_finalize)(hsa_ext_program_t program,
                                             hsa_isa_t isa,
                                             int32_t call_convention,
                                             hsa_ext_control_directives_t control_directives,
                                             const char *options,
                                             hsa_code_object_type_t code_object_type,
                                             hsa_code_object_t *code_object);
} hsa_ext_finalizer_pfn_t;

hsa_status_t get_finalization_fnc_tbl(hsa_ext_finalizer_pfn_t *table);

int load_module_from_file(const char* file, hsa_ext_module_t* module);
int load_base_or_full_module_from_file(hsa_agent_t agent,
                                       const char* base_file_name,
                                       const char* full_file_name,
                                       hsa_ext_module_t* module);

void destroy_module(hsa_ext_module_t module);

hsa_status_t finalize_executable(hsa_agent_t agent,
                                 uint32_t module_count,
                                 hsa_ext_module_t *modules,
                                 hsa_machine_model_t machine_model,
                                 hsa_profile_t profile,
                                 hsa_default_float_rounding_mode_t default_float_rounding_mode,
                                 hsa_code_object_type_t code_object_type,
                                 int32_t call_convention,
                                 hsa_ext_control_directives_t control_directives,
                                 hsa_code_object_t* code_object,
                                 hsa_executable_t* executable);

hsa_status_t get_executable_symbols(hsa_executable_t executable,
                                    hsa_agent_t agent,
                                    uint32_t call_convention,
                                    uint32_t symbol_count,
                                    char** symbol_names,
                                    symbol_record_t* symbol_record_list);

#endif  // _FINALIZE_UTILS_H_
