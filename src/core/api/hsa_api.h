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

#ifndef _HSA_API_H_
#define _HSA_API_H_

extern int test_hsa_init();
extern int test_hsa_init_MAX();
extern int test_hsa_shut_down();
extern int test_hsa_shut_down_not_initialized();
extern int test_hsa_shut_down_after_shut_down();
extern int test_hsa_status_string();
extern int test_hsa_status_string_not_initialized();
extern int test_hsa_status_string_invalid_status();
extern int test_hsa_status_string_invalid_ptr();
extern int test_hsa_iterate_agents();
extern int test_hsa_iterate_agents_not_initialized();
extern int test_hsa_iterate_agents_invalid_callback();
extern int test_hsa_agent_get_info();
extern int test_hsa_agent_get_info_not_initialized();
extern int test_hsa_agent_get_info_invalid_agent();
extern int test_hsa_agent_get_info_invalid_attribute();
extern int test_hsa_agent_get_info_invalid_ptr();
extern int test_hsa_agent_extension_supported();
extern int test_hsa_agent_extension_supported_not_initialized();
extern int test_hsa_agent_extension_supported_invalid_agent();
extern int test_hsa_agent_extension_supported_invalid_extension();
extern int test_hsa_agent_extension_supported_null_result_ptr();
extern int test_hsa_agent_get_exception_policies();
extern int test_hsa_agent_get_exception_policies_not_initialized();
extern int test_hsa_agent_get_exception_policies_invalid_agent();
extern int test_hsa_agent_get_exception_policies_null_mask_ptr();
extern int test_hsa_agent_get_exception_policies_invalid_profile();
extern int test_hsa_system_extension_supported();
extern int test_hsa_system_extension_supported_not_initialized();
extern int test_hsa_system_extension_supported_invalid_extension();
extern int test_hsa_system_extension_supported_null_result_ptr();
extern int test_hsa_system_get_extension_table();
extern int test_hsa_system_get_extension_table_not_initialized();
extern int test_hsa_system_get_extension_table_invalid_extension();
extern int test_hsa_system_get_extension_table_null_table_ptr();
extern int test_hsa_system_get_info();
extern int test_hsa_system_get_info_not_initialized();
extern int test_hsa_system_get_info_invalid_attribute();
extern int test_hsa_system_get_info_invalid_ptr();
extern int test_hsa_signal_create();
extern int test_hsa_signal_create_not_initialized();
extern int test_hsa_signal_create_null_signal();
extern int test_hsa_signal_create_invalid_arg();
extern int test_hsa_signal_destroy();
extern int test_hsa_signal_destroy_not_initialized();
extern int test_hsa_signal_destroy_invalid_arg();
extern int test_hsa_signal_destroy_invalid_signal();
extern int test_hsa_signal_load_acquire();
extern int test_hsa_signal_load_relaxed();
extern int test_hsa_signal_store_release();
extern int test_hsa_signal_store_relaxed();
extern int test_hsa_signal_exchange_acq_rel();
extern int test_hsa_signal_exchange_acquire();
extern int test_hsa_signal_exchange_relaxed();
extern int test_hsa_signal_exchange_release();
extern int test_hsa_signal_cas_acq_rel();
extern int test_hsa_signal_cas_acquire();
extern int test_hsa_signal_cas_relaxed();
extern int test_hsa_signal_cas_release();
extern int test_hsa_signal_add_acq_rel();
extern int test_hsa_signal_add_acquire();
extern int test_hsa_signal_add_relaxed();
extern int test_hsa_signal_add_release();
extern int test_hsa_signal_subtract_acq_rel();
extern int test_hsa_signal_subtract_acquire();
extern int test_hsa_signal_subtract_relaxed();
extern int test_hsa_signal_subtract_release();
extern int test_hsa_signal_and_acq_rel();
extern int test_hsa_signal_and_acquire();
extern int test_hsa_signal_and_relaxed();
extern int test_hsa_signal_and_release();
extern int test_hsa_signal_or_acq_rel();
extern int test_hsa_signal_or_acquire();
extern int test_hsa_signal_or_relaxed();
extern int test_hsa_signal_or_release();
extern int test_hsa_signal_xor_acq_rel();
extern int test_hsa_signal_xor_acquire();
extern int test_hsa_signal_xor_relaxed();
extern int test_hsa_signal_xor_release();
extern int test_hsa_queue_create();
extern int test_hsa_queue_create_not_initialized();
extern int test_hsa_queue_create_out_of_resources();
extern int test_hsa_queue_create_invalid_agent();
extern int test_hsa_queue_create_invalid_queue_creation();
extern int test_hsa_queue_create_invalid_argument();
extern int test_hsa_queue_destroy();
extern int test_hsa_queue_destroy_not_initialized();
extern int test_hsa_queue_destroy_invalid_queue();
extern int test_hsa_queue_destroy_invalid_argument();
extern int test_hsa_queue_inactivate();
extern int test_hsa_queue_inactivate_not_initialized();
extern int test_hsa_queue_inactivate_invalid_queue();
extern int test_hsa_queue_inactivate_invalid_argument();
extern int test_hsa_queue_load_read_index_acquire();
extern int test_hsa_queue_load_read_index_relaxed();
extern int test_hsa_queue_load_store_write_index_acquire_relaxed();
extern int test_hsa_queue_load_store_write_index_relaxed_release();
extern int test_hsa_queue_cas_write_index_acq_rel();
extern int test_hsa_queue_cas_write_index_acquire();
extern int test_hsa_queue_cas_write_index_relaxed();
extern int test_hsa_queue_cas_write_index_release();
extern int test_hsa_queue_add_write_index_acq_rel();
extern int test_hsa_queue_add_write_index_acquire();
extern int test_hsa_queue_add_write_index_relaxed();
extern int test_hsa_queue_add_write_index_release();
extern int test_hsa_memory_allocate_not_initialized();
extern int test_hsa_memory_allocate_null_ptr();
extern int test_hsa_memory_allocate_zero_size();
extern int test_hsa_memory_allocate_invalid_allocation();
extern int test_hsa_memory_allocate_invalid_region();
extern int test_hsa_memory_allocate();
extern int test_hsa_memory_free();
extern int test_hsa_memory_free_not_initialized();
extern int test_hsa_memory_register();
extern int test_hsa_memory_register_not_initialized();
extern int test_hsa_memory_register_invalid_argument();
extern int test_hsa_memory_deregister();
extern int test_hsa_memory_deregister_not_initialized();
extern int test_hsa_region_get_info();
extern int test_hsa_region_get_info_not_initialized();
extern int test_hsa_region_get_info_invalid_region();
extern int test_hsa_region_get_info_invalid_argument();
extern int test_hsa_agent_iterate_regions();
extern int test_hsa_agent_iterate_regions_not_initialized();
extern int test_hsa_agent_iterate_regions_invalid_argument();
extern int test_hsa_agent_iterate_regions_invalid_agent();
extern int test_hsa_isa_get_info();
extern int test_hsa_isa_get_info_not_initialized();
extern int test_hsa_isa_get_info_invalid_isa();
extern int test_hsa_isa_get_info_index_out_of_range();
extern int test_hsa_isa_get_info_invalid_attribute();
extern int test_hsa_isa_get_info_invalid_null_value();
extern int test_hsa_code_object_get_info();
extern int test_hsa_code_symbol_get_info();
extern int test_hsa_executable_create();
extern int test_hsa_executable_create_not_initialized();
extern int test_hsa_executable_create_invalid_argument();
extern int test_hsa_executable_create_out_of_resources();
extern int test_hsa_executable_destroy();
extern int test_hsa_executable_destroy_not_initialized();
extern int test_hsa_executable_destroy_invalid_executable();
extern int test_hsa_executable_load_code_object();
extern int test_hsa_executable_load_code_object_not_initialized();
extern int test_hsa_executable_load_code_object_invalid_executable();
extern int test_hsa_executable_load_code_object_invalid_agent();
extern int test_hsa_executable_load_code_object_invalid_code_object();
extern int test_hsa_executable_load_code_object_frozen_executable();
extern int test_hsa_executable_get_info();
extern int test_hsa_executable_symbol_get_info();
extern int test_hsa_soft_queue_create();
extern int test_hsa_isa_from_name();
extern int test_hsa_isa_from_name_null_name();
extern int test_hsa_isa_from_name_null_isa();
extern int test_hsa_isa_from_name_invalid_isa_name();
extern int test_hsa_isa_compatible();
extern int test_hsa_isa_compatible_invalid_isa();
extern int test_hsa_isa_compatible_null_result();

#endif  // _HSA_API_H_
