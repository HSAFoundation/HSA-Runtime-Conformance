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

#include <framework.h>
#include "hsa_api.h"

DEFINE_TEST(hsa_init);
DEFINE_TEST(hsa_init_MAX);
DEFINE_TEST(hsa_shut_down);
DEFINE_TEST(hsa_shut_down_not_initialized);
DEFINE_TEST(hsa_shut_down_after_shut_down);
DEFINE_TEST(hsa_status_string);
DEFINE_TEST(hsa_status_string_not_initialized);
DEFINE_TEST(hsa_status_string_invalid_status);
DEFINE_TEST(hsa_status_string_invalid_ptr);
DEFINE_TEST(hsa_iterate_agents);
DEFINE_TEST(hsa_iterate_agents_not_initialized);
DEFINE_TEST(hsa_iterate_agents_invalid_callback);
DEFINE_TEST(hsa_agent_get_info);
DEFINE_TEST(hsa_agent_get_info_not_initialized);
DEFINE_TEST(hsa_agent_get_info_invalid_agent);
DEFINE_TEST(hsa_agent_get_info_invalid_attribute);
DEFINE_TEST(hsa_agent_get_info_invalid_ptr);
DEFINE_TEST(hsa_agent_extension_supported);
DEFINE_TEST(hsa_agent_extension_supported_not_initialized);
DEFINE_TEST(hsa_agent_extension_supported_invalid_agent);
DEFINE_TEST(hsa_agent_extension_supported_invalid_extension);
DEFINE_TEST(hsa_agent_extension_supported_null_result_ptr);
DEFINE_TEST(hsa_agent_get_exception_policies);
DEFINE_TEST(hsa_agent_get_exception_policies_not_initialized);
DEFINE_TEST(hsa_agent_get_exception_policies_invalid_agent);
DEFINE_TEST(hsa_agent_get_exception_policies_null_mask_ptr);
DEFINE_TEST(hsa_agent_get_exception_policies_invalid_profile);
DEFINE_TEST(hsa_system_extension_supported);
DEFINE_TEST(hsa_system_extension_supported_not_initialized);
DEFINE_TEST(hsa_system_extension_supported_invalid_extension);
DEFINE_TEST(hsa_system_extension_supported_null_result_ptr);
DEFINE_TEST(hsa_system_get_extension_table);
DEFINE_TEST(hsa_system_get_extension_table_not_initialized);
DEFINE_TEST(hsa_system_get_extension_table_invalid_extension);
DEFINE_TEST(hsa_system_get_extension_table_null_table_ptr);
DEFINE_TEST(hsa_system_get_info);
DEFINE_TEST(hsa_system_get_info_not_initialized);
DEFINE_TEST(hsa_system_get_info_invalid_attribute);
DEFINE_TEST(hsa_system_get_info_invalid_ptr);
DEFINE_TEST(hsa_signal_create);
DEFINE_TEST(hsa_signal_create_not_initialized);
DEFINE_TEST(hsa_signal_create_null_signal);
DEFINE_TEST(hsa_signal_create_invalid_arg);
DEFINE_TEST(hsa_signal_destroy);
DEFINE_TEST(hsa_signal_destroy_not_initialized);
DEFINE_TEST(hsa_signal_destroy_invalid_arg);
DEFINE_TEST(hsa_signal_destroy_invalid_signal);
DEFINE_TEST(hsa_signal_load_acquire);
DEFINE_TEST(hsa_signal_load_relaxed);
DEFINE_TEST(hsa_signal_store_release);
DEFINE_TEST(hsa_signal_store_relaxed);
DEFINE_TEST(hsa_signal_exchange_acq_rel);
DEFINE_TEST(hsa_signal_exchange_acquire);
DEFINE_TEST(hsa_signal_exchange_relaxed);
DEFINE_TEST(hsa_signal_exchange_release);
DEFINE_TEST(hsa_signal_cas_acq_rel);
DEFINE_TEST(hsa_signal_cas_acquire);
DEFINE_TEST(hsa_signal_cas_relaxed);
DEFINE_TEST(hsa_signal_cas_release);
DEFINE_TEST(hsa_signal_add_acq_rel);
DEFINE_TEST(hsa_signal_add_acquire);
DEFINE_TEST(hsa_signal_add_relaxed);
DEFINE_TEST(hsa_signal_add_release);
DEFINE_TEST(hsa_signal_subtract_acq_rel);
DEFINE_TEST(hsa_signal_subtract_acquire);
DEFINE_TEST(hsa_signal_subtract_relaxed);
DEFINE_TEST(hsa_signal_subtract_release);
DEFINE_TEST(hsa_signal_and_acq_rel);
DEFINE_TEST(hsa_signal_and_acquire);
DEFINE_TEST(hsa_signal_and_relaxed);
DEFINE_TEST(hsa_signal_and_release);
DEFINE_TEST(hsa_signal_or_acq_rel);
DEFINE_TEST(hsa_signal_or_acquire);
DEFINE_TEST(hsa_signal_or_relaxed);
DEFINE_TEST(hsa_signal_or_release);
DEFINE_TEST(hsa_signal_xor_acq_rel);
DEFINE_TEST(hsa_signal_xor_acquire);
DEFINE_TEST(hsa_signal_xor_relaxed);
DEFINE_TEST(hsa_signal_xor_release);
DEFINE_TEST(hsa_queue_create);
DEFINE_TEST(hsa_queue_create_not_initialized);
DEFINE_TEST(hsa_queue_create_out_of_resources);
DEFINE_TEST(hsa_queue_create_invalid_agent);
DEFINE_TEST(hsa_queue_create_invalid_queue_creation);
DEFINE_TEST(hsa_queue_create_invalid_argument);
DEFINE_TEST(hsa_queue_destroy);
DEFINE_TEST(hsa_queue_destroy_not_initialized);
DEFINE_TEST(hsa_queue_destroy_invalid_queue);
DEFINE_TEST(hsa_queue_destroy_invalid_argument);
DEFINE_TEST(hsa_queue_inactivate);
DEFINE_TEST(hsa_queue_inactivate_not_initialized);
DEFINE_TEST(hsa_queue_inactivate_invalid_queue);
DEFINE_TEST(hsa_queue_inactivate_invalid_argument);
DEFINE_TEST(hsa_queue_load_read_index_acquire);
DEFINE_TEST(hsa_queue_load_read_index_relaxed);
DEFINE_TEST(hsa_queue_load_store_write_index_acquire_relaxed);
DEFINE_TEST(hsa_queue_load_store_write_index_relaxed_release);
DEFINE_TEST(hsa_queue_cas_write_index_acq_rel);
DEFINE_TEST(hsa_queue_cas_write_index_acquire);
DEFINE_TEST(hsa_queue_cas_write_index_relaxed);
DEFINE_TEST(hsa_queue_cas_write_index_release);
DEFINE_TEST(hsa_queue_add_write_index_acq_rel);
DEFINE_TEST(hsa_queue_add_write_index_acquire);
DEFINE_TEST(hsa_queue_add_write_index_relaxed);
DEFINE_TEST(hsa_queue_add_write_index_release);
DEFINE_TEST(hsa_memory_allocate);
DEFINE_TEST(hsa_memory_allocate_not_initialized);
DEFINE_TEST(hsa_memory_allocate_null_ptr);
DEFINE_TEST(hsa_memory_allocate_zero_size);
DEFINE_TEST(hsa_memory_allocate_invalid_allocation);
DEFINE_TEST(hsa_memory_allocate_invalid_region);
DEFINE_TEST(hsa_memory_free);
DEFINE_TEST(hsa_memory_free_not_initialized);
DEFINE_TEST(hsa_memory_register);
DEFINE_TEST(hsa_memory_register_not_initialized);
DEFINE_TEST(hsa_memory_register_invalid_argument);
DEFINE_TEST(hsa_memory_deregister);
DEFINE_TEST(hsa_memory_deregister_not_initialized);
DEFINE_TEST(hsa_region_get_info);
DEFINE_TEST(hsa_region_get_info_not_initialized);
DEFINE_TEST(hsa_region_get_info_invalid_region);
DEFINE_TEST(hsa_region_get_info_invalid_argument);
DEFINE_TEST(hsa_agent_iterate_regions);
DEFINE_TEST(hsa_agent_iterate_regions_not_initialized);
DEFINE_TEST(hsa_agent_iterate_regions_invalid_argument);
DEFINE_TEST(hsa_agent_iterate_regions_invalid_agent);
DEFINE_TEST(hsa_isa_get_info);
DEFINE_TEST(hsa_isa_get_info_not_initialized);
DEFINE_TEST(hsa_isa_get_info_invalid_isa);
DEFINE_TEST(hsa_isa_get_info_index_out_of_range);
DEFINE_TEST(hsa_isa_get_info_invalid_attribute);
DEFINE_TEST(hsa_isa_get_info_invalid_null_value);
DEFINE_TEST(hsa_code_object_get_info);
DEFINE_TEST(hsa_code_symbol_get_info);
DEFINE_TEST(hsa_executable_create);
DEFINE_TEST(hsa_executable_create_not_initialized);
DEFINE_TEST(hsa_executable_create_invalid_argument);
DEFINE_TEST(hsa_executable_create_out_of_resources);
DEFINE_TEST(hsa_executable_destroy);
DEFINE_TEST(hsa_executable_destroy_not_initialized);
DEFINE_TEST(hsa_executable_destroy_invalid_executable);
DEFINE_TEST(hsa_executable_load_code_object);
DEFINE_TEST(hsa_executable_load_code_object_not_initialized);
DEFINE_TEST(hsa_executable_load_code_object_invalid_executable);
DEFINE_TEST(hsa_executable_load_code_object_invalid_agent);
DEFINE_TEST(hsa_executable_load_code_object_invalid_code_object);
DEFINE_TEST(hsa_executable_load_code_object_frozen_executable);
DEFINE_TEST(hsa_executable_get_info);
DEFINE_TEST(hsa_executable_symbol_get_info);
DEFINE_TEST(hsa_soft_queue_create);
DEFINE_TEST(hsa_isa_from_name);
DEFINE_TEST(hsa_isa_from_name_null_name);
DEFINE_TEST(hsa_isa_from_name_null_isa);
DEFINE_TEST(hsa_isa_from_name_invalid_isa_name);
DEFINE_TEST(hsa_isa_compatible);
DEFINE_TEST(hsa_isa_compatible_invalid_isa);
DEFINE_TEST(hsa_isa_compatible_null_result);

int main(int argc, char* argv[]) {
    INITIALIZE_TESTSUITE();
    ADD_TEST(hsa_init);
    // ADD_TEST(hsa_init_MAX);
    ADD_TEST(hsa_shut_down);
    ADD_TEST(hsa_shut_down_not_initialized);
    ADD_TEST(hsa_shut_down_after_shut_down);
    ADD_TEST(hsa_status_string);
    ADD_TEST(hsa_status_string_not_initialized);
    ADD_TEST(hsa_status_string_invalid_status);
    ADD_TEST(hsa_status_string_invalid_ptr);
    ADD_TEST(hsa_iterate_agents);
    ADD_TEST(hsa_iterate_agents_not_initialized);
    ADD_TEST(hsa_iterate_agents_invalid_callback);
    ADD_TEST(hsa_agent_get_info);
    ADD_TEST(hsa_agent_get_info_not_initialized);
    ADD_TEST(hsa_agent_get_info_invalid_agent);
    ADD_TEST(hsa_agent_get_info_invalid_attribute);
    ADD_TEST(hsa_agent_get_info_invalid_ptr);
    ADD_TEST(hsa_agent_extension_supported);
    ADD_TEST(hsa_agent_extension_supported_not_initialized);
    ADD_TEST(hsa_agent_extension_supported_invalid_agent);
    ADD_TEST(hsa_agent_extension_supported_invalid_extension);
    ADD_TEST(hsa_agent_extension_supported_null_result_ptr);
    ADD_TEST(hsa_agent_get_exception_policies);
    ADD_TEST(hsa_agent_get_exception_policies_not_initialized);
    ADD_TEST(hsa_agent_get_exception_policies_invalid_agent);
    ADD_TEST(hsa_agent_get_exception_policies_null_mask_ptr);
    ADD_TEST(hsa_agent_get_exception_policies_invalid_profile);
    ADD_TEST(hsa_system_extension_supported);
    ADD_TEST(hsa_system_extension_supported_not_initialized);
    ADD_TEST(hsa_system_extension_supported_invalid_extension);
    ADD_TEST(hsa_system_extension_supported_null_result_ptr);
    ADD_TEST(hsa_system_get_extension_table);
    ADD_TEST(hsa_system_get_extension_table_not_initialized);
    ADD_TEST(hsa_system_get_extension_table_invalid_extension);
    ADD_TEST(hsa_system_get_extension_table_null_table_ptr);
    ADD_TEST(hsa_system_get_info);
    ADD_TEST(hsa_system_get_info_not_initialized);
    ADD_TEST(hsa_system_get_info_invalid_attribute);
    ADD_TEST(hsa_system_get_info_invalid_ptr);
    ADD_TEST(hsa_signal_create);
    ADD_TEST(hsa_signal_create_not_initialized);
    ADD_TEST(hsa_signal_create_null_signal);
    ADD_TEST(hsa_signal_create_invalid_arg);
    ADD_TEST(hsa_signal_destroy);
    ADD_TEST(hsa_signal_destroy_not_initialized);
    ADD_TEST(hsa_signal_destroy_invalid_arg);
    ADD_TEST(hsa_signal_destroy_invalid_signal);
    ADD_TEST(hsa_signal_load_acquire);
    ADD_TEST(hsa_signal_load_relaxed);
    ADD_TEST(hsa_signal_store_release);
    ADD_TEST(hsa_signal_store_relaxed);
    ADD_TEST(hsa_signal_exchange_acq_rel);
    ADD_TEST(hsa_signal_exchange_acquire);
    ADD_TEST(hsa_signal_exchange_relaxed);
    ADD_TEST(hsa_signal_exchange_release);
    ADD_TEST(hsa_signal_cas_acq_rel);
    ADD_TEST(hsa_signal_cas_acquire);
    ADD_TEST(hsa_signal_cas_relaxed);
    ADD_TEST(hsa_signal_cas_release);
    ADD_TEST(hsa_signal_add_acq_rel);
    ADD_TEST(hsa_signal_add_acquire);
    ADD_TEST(hsa_signal_add_relaxed);
    ADD_TEST(hsa_signal_add_release);
    ADD_TEST(hsa_signal_subtract_acq_rel);
    ADD_TEST(hsa_signal_subtract_acquire);
    ADD_TEST(hsa_signal_subtract_relaxed);
    ADD_TEST(hsa_signal_subtract_release);
    ADD_TEST(hsa_signal_and_acq_rel);
    ADD_TEST(hsa_signal_and_acquire);
    ADD_TEST(hsa_signal_and_relaxed);
    ADD_TEST(hsa_signal_and_release);
    ADD_TEST(hsa_signal_or_acq_rel);
    ADD_TEST(hsa_signal_or_acquire);
    ADD_TEST(hsa_signal_or_relaxed);
    ADD_TEST(hsa_signal_or_release);
    ADD_TEST(hsa_signal_xor_acq_rel);
    ADD_TEST(hsa_signal_xor_acquire);
    ADD_TEST(hsa_signal_xor_relaxed);
    ADD_TEST(hsa_signal_xor_release);
    ADD_TEST(hsa_queue_create);
    ADD_TEST(hsa_queue_create_not_initialized);
    ADD_TEST(hsa_queue_create_out_of_resources);
    ADD_TEST(hsa_queue_create_invalid_agent);
    ADD_TEST(hsa_queue_create_invalid_queue_creation);
    ADD_TEST(hsa_queue_create_invalid_argument);
    ADD_TEST(hsa_queue_destroy);
    ADD_TEST(hsa_queue_destroy_not_initialized);
    ADD_TEST(hsa_queue_destroy_invalid_queue);
    ADD_TEST(hsa_queue_destroy_invalid_argument);
    ADD_TEST(hsa_queue_inactivate);
    ADD_TEST(hsa_queue_inactivate_not_initialized);
    ADD_TEST(hsa_queue_inactivate_invalid_queue);
    ADD_TEST(hsa_queue_inactivate_invalid_argument);
    ADD_TEST(hsa_queue_load_read_index_acquire);
    ADD_TEST(hsa_queue_load_read_index_relaxed);
    ADD_TEST(hsa_queue_load_store_write_index_acquire_relaxed);
    ADD_TEST(hsa_queue_load_store_write_index_relaxed_release);
    ADD_TEST(hsa_queue_cas_write_index_acq_rel);
    ADD_TEST(hsa_queue_cas_write_index_acquire);
    ADD_TEST(hsa_queue_cas_write_index_relaxed);
    ADD_TEST(hsa_queue_cas_write_index_release);
    ADD_TEST(hsa_queue_add_write_index_acq_rel);
    ADD_TEST(hsa_queue_add_write_index_acquire);
    ADD_TEST(hsa_queue_add_write_index_relaxed);
    ADD_TEST(hsa_queue_add_write_index_release);
    ADD_TEST(hsa_memory_allocate);
    ADD_TEST(hsa_memory_allocate_not_initialized);
    ADD_TEST(hsa_memory_allocate_null_ptr);
    ADD_TEST(hsa_memory_allocate_zero_size);
    ADD_TEST(hsa_memory_allocate_invalid_allocation);
    ADD_TEST(hsa_memory_allocate_invalid_region);
    ADD_TEST(hsa_memory_free);
    ADD_TEST(hsa_memory_free_not_initialized);
    ADD_TEST(hsa_memory_register);
    ADD_TEST(hsa_memory_register_not_initialized);
    ADD_TEST(hsa_memory_register_invalid_argument);
    ADD_TEST(hsa_memory_deregister);
    ADD_TEST(hsa_memory_deregister_not_initialized);
    ADD_TEST(hsa_region_get_info);
    ADD_TEST(hsa_region_get_info_not_initialized);
    ADD_TEST(hsa_region_get_info_invalid_region);
    ADD_TEST(hsa_region_get_info_invalid_argument);
    ADD_TEST(hsa_agent_iterate_regions);
    ADD_TEST(hsa_agent_iterate_regions_not_initialized);
    ADD_TEST(hsa_agent_iterate_regions_invalid_argument);
    ADD_TEST(hsa_agent_iterate_regions_invalid_agent);
    ADD_TEST(hsa_isa_get_info);
    ADD_TEST(hsa_isa_get_info_not_initialized);
    ADD_TEST(hsa_isa_get_info_invalid_isa);
    ADD_TEST(hsa_isa_get_info_index_out_of_range);
    ADD_TEST(hsa_isa_get_info_invalid_attribute);
    ADD_TEST(hsa_isa_get_info_invalid_null_value);
    ADD_TEST(hsa_code_object_get_info);
    ADD_TEST(hsa_code_symbol_get_info);
    ADD_TEST(hsa_executable_create);
    ADD_TEST(hsa_executable_create_not_initialized);
    ADD_TEST(hsa_executable_create_invalid_argument);
    ADD_TEST(hsa_executable_create_out_of_resources);
    ADD_TEST(hsa_executable_destroy);
    ADD_TEST(hsa_executable_destroy_not_initialized);
    ADD_TEST(hsa_executable_destroy_invalid_executable);
    ADD_TEST(hsa_executable_load_code_object);
    ADD_TEST(hsa_executable_load_code_object_not_initialized);
    ADD_TEST(hsa_executable_load_code_object_invalid_executable);
    ADD_TEST(hsa_executable_load_code_object_invalid_agent);
    ADD_TEST(hsa_executable_load_code_object_invalid_code_object);
    ADD_TEST(hsa_executable_load_code_object_frozen_executable);
    ADD_TEST(hsa_executable_get_info);
    ADD_TEST(hsa_executable_symbol_get_info);
    ADD_TEST(hsa_soft_queue_create);
    ADD_TEST(hsa_isa_from_name);
    ADD_TEST(hsa_isa_from_name_null_name);
    ADD_TEST(hsa_isa_from_name_null_isa);
    ADD_TEST(hsa_isa_from_name_invalid_isa_name);
    ADD_TEST(hsa_isa_compatible);
    ADD_TEST(hsa_isa_compatible_invalid_isa);
    ADD_TEST(hsa_isa_compatible_null_result);
    RUN_TESTS();
}
