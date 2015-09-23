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

#ifndef _TEST_HELPER_FUNC_H_
#define _TEST_HELPER_FUNC_H_

#include <hsa.h>

hsa_status_t callback_get_num_agents(hsa_agent_t agent, void* data);

hsa_status_t callback_get_agents(hsa_agent_t agent, void* data);

hsa_status_t callback_check_agents(hsa_agent_t agent, void* data);

hsa_status_t callback_get_kernel_dispatch_agent(hsa_agent_t agent, void* data);

hsa_status_t callback_get_num_regions(hsa_region_t region, void* data);

hsa_status_t callback_get_regions(hsa_region_t region, void* data);

hsa_status_t callback_get_region_global_allocatable(hsa_region_t region,
                                                    void* data);

void check_agents();

void check_system_info();

void launch_no_op_kernels(hsa_agent_t* agent,
                          hsa_queue_t* queue,
                          int num_packets);

hsa_code_object_t load_code_object(hsa_agent_t* agent,
                                   char* file_name,
                                   char* kernel_name);

hsa_status_t callback_serialize_alloc(size_t size,
                                      hsa_callback_data_t data,
                                      void** address);

#endif  // _TEST_HELPER_FUNC_H_
