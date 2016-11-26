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

#include "finalize_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define EXIT_IF(_cond_) if (_cond_) { goto exit; }

#define GET_SYMBOL_INFO(_symbol_record_, _symbol_attribute_, _data_field_) { \
       status = hsa_executable_symbol_get_info(_symbol_record_.symbol, _symbol_attribute_, &(_symbol_record_._data_field_)); \
       if (HSA_STATUS_SUCCESS != status) { goto exit; } \
}

hsa_status_t get_finalization_fnc_tbl(hsa_ext_finalizer_pfn_t *table) {
    bool support;
    hsa_status_t status;

    if (NULL == table) {
        return HSA_STATUS_ERROR_INVALID_ARGUMENT;
    }

    status = hsa_system_extension_supported(HSA_EXTENSION_FINALIZER, 1, 0, &support);

    if (HSA_STATUS_SUCCESS != status) {
        goto exit;
    }

    if (!support) {
        status = HSA_STATUS_ERROR;
        goto exit;
    }

    hsa_ext_finalizer_1_00_pfn_t table_1_00;

    status = hsa_system_get_extension_table(HSA_EXTENSION_FINALIZER, 1, 0, &table_1_00);

    if (HSA_STATUS_SUCCESS != status) {
        goto exit;
    }

    // Fill in the table.
    table->hsa_ext_program_create = table_1_00.hsa_ext_program_create;
    table->hsa_ext_program_destroy = table_1_00.hsa_ext_program_destroy;
    table->hsa_ext_program_add_module = table_1_00.hsa_ext_program_add_module;
    table->hsa_ext_program_iterate_modules = table_1_00.hsa_ext_program_iterate_modules;
    table->hsa_ext_program_get_info = table_1_00.hsa_ext_program_get_info;
    table->hsa_ext_program_finalize = table_1_00.hsa_ext_program_finalize;

 exit:

    return status;
}

int load_module_from_file(const char* file_name, hsa_ext_module_t* module) {
    int rc = -1;

    FILE *fp = fopen(file_name, "rb");

    EXIT_IF(fp == NULL);

    EXIT_IF((rc = fseek(fp, 0, SEEK_END)) == -1);

    size_t file_size = (size_t) (ftell(fp) * sizeof(char));

    EXIT_IF((rc = fseek(fp, 0, SEEK_SET)) == -1);

    char* buf = (char*) malloc(file_size);

    EXIT_IF(buf == NULL);

    memset(buf, 0, file_size);

    size_t read_size = fread(buf, sizeof(char), file_size, fp);

    if (read_size != file_size) {
        free(buf);
    } else {
        rc = 0;
        *module = (void*) buf;
    }

 exit:

    fclose(fp);

    return rc;
}

// Loads file 'base_file_name' to the given module in case the given
// agent supports finalization of the BASE profile, otherwise
// loads the file 'full_file_name'.
int load_base_or_full_module_from_file(hsa_agent_t agent,
                                       const char* base_file_name,
                                       const char* full_file_name,
                                       hsa_ext_module_t* module) {
    hsa_profile_t profile;
    hsa_status_t status;
    const char *file_name;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_PROFILE, &profile);
    if (HSA_STATUS_SUCCESS != status)
      return -1;

    file_name = (profile == HSA_PROFILE_BASE) ?
      base_file_name : full_file_name;
    return load_module_from_file(file_name, module);
}

void destroy_module(hsa_ext_module_t module) {
    char* buf = (char*) module;

    if (buf) {
        free(buf);
    }

    return;
}

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
                              hsa_executable_t* executable) {
    int i;
    int rc;
    hsa_status_t status;

    // Create the program
    hsa_ext_program_t program;
    memset(&program, 0, sizeof(hsa_ext_program_t));
    status = hsa_ext_program_create(machine_model, profile, default_float_rounding_mode, NULL, &program);
    EXIT_IF(HSA_STATUS_SUCCESS != status);

    // Add the brig modules to the program
    for (i = 0; i < module_count; ++i) {
        status = hsa_ext_program_add_module(program, modules[i]);
        EXIT_IF(HSA_STATUS_SUCCESS != status);
    }

    // Determine the agents ISA
    hsa_isa_t isa;
    status = hsa_agent_get_info(agent, HSA_AGENT_INFO_ISA, &isa);
    EXIT_IF(HSA_STATUS_SUCCESS != status);

    // Finalize the program and extract the code object
    status = hsa_ext_program_finalize(program, isa, call_convention, control_directives, "", code_object_type, code_object);
    EXIT_IF(HSA_STATUS_SUCCESS != status);

    // Create the empty executable
    status = hsa_executable_create(profile, HSA_EXECUTABLE_STATE_UNFROZEN, "", executable);
    EXIT_IF(HSA_STATUS_SUCCESS != status);

    // Load the code object
    status = hsa_executable_load_code_object(*executable, agent, *code_object, "");
    EXIT_IF(HSA_STATUS_SUCCESS != status);

    // Freeze the executable; it can now be queried for symbols
    status = hsa_executable_freeze(*executable, "");
    EXIT_IF(HSA_STATUS_SUCCESS != status);

 exit:
    // Releasing these resources should not affect the executable
    hsa_ext_program_destroy(program);

    return status;
}

hsa_status_t get_executable_symbols(hsa_executable_t executable,
                                    hsa_agent_t agent,
                                    uint32_t call_convention,
                                    uint32_t symbol_count,
                                    char** symbol_names,
                                    symbol_record_t* symbol_record_list) {
    int i;
    hsa_status_t status;

    for (i = 0; i < symbol_count; ++i) {
        status = hsa_executable_get_symbol(executable,
                                           NULL,
                                           symbol_names[i],
                                           agent,
                                           call_convention,
                                           &(symbol_record_list[i].symbol));

        EXIT_IF(HSA_STATUS_SUCCESS != status);

        // Get all off the symbols relevant information
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_TYPE, type);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_NAME_LENGTH, name_length);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_NAME, name);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME_LENGTH, module_name_length);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME, module_name)
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_AGENT, agent);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ADDRESS, variable_address);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_LINKAGE, linkage);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ALLOCATION, variable_allocation);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_SEGMENT, variable_segment);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ALIGNMENT, variable_alignment);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_SIZE, variable_size);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_IS_CONST, is_const);
        GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT, kernel_object);
        GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, kernarg_segment_size);
        GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_ALIGNMENT, kernarg_segment_alignment);
        GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, group_segment_size);
        GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, private_segment_size);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_DYNAMIC_CALLSTACK, dynamic_callstack);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_INDIRECT_FUNCTION_OBJECT, indirect_function_object);
        // GET_SYMBOL_INFO(symbol_record_list[i], HSA_EXECUTABLE_SYMBOL_INFO_INDIRECT_FUNCTION_CALL_CONVENTION, indirect_function_call_convention);
    }

 exit:

    return status;
}
