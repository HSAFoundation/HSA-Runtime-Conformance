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

#ifndef _AGENT_UTILS_H_
#define _AGENT_UTILS_H_
#include <hsa.h>

// Struct to store agent list information
struct agent_list_s {
    size_t num_agents;
    hsa_agent_t *agents;
};

// Check that the system info reported is consistent with specification
void check_system_info();

// Check that the agent info reported is consistent with specification
void check_agent_info(hsa_agent_t agent);

// Callback that counts the number of agents
hsa_status_t get_num_agents(hsa_agent_t agent, void* data);

// Callback that initializes an agent list
hsa_status_t get_agents(hsa_agent_t agent, void* data);

// Get the list of all agents on the platform
void get_agent_list(struct agent_list_s *agent_list);

// Free an agent list
void free_agent_list(struct agent_list_s *agent_list);

// Get the first gpu agent returned by topology
hsa_status_t get_gpu_agent(hsa_agent_t agent, void* data);

// Get the first cpu agent returned by topology
hsa_status_t get_cpu_agent(hsa_agent_t agent, void* data);

// Get the first agent that supports kernel dispatch
hsa_status_t get_kernel_dispatch_agent(hsa_agent_t agent, void* data);

// Callback that checks an agent's information
hsa_status_t check_agent(hsa_agent_t agent, void* data);

// Struct to store agent memory region list information
struct region_list_s {
    // number of regions in the list
    size_t num_regions;
    // region pointers
    hsa_region_t* regions;
};

// Get all of the memory regions associated with an agent
void get_region_list(hsa_agent_t agent, struct region_list_s* region_list);

// Free a memory region list
void free_region_list(struct region_list_s* region_list);

// Callback to acquire a kernarg memory region associated with the agent
hsa_status_t get_kernarg_memory_region(hsa_region_t region, void* data);

// Callback to acquire a group memory region associated with the agent
hsa_status_t get_group_memory_region(hsa_region_t region, void* data);

// Callback to acquire a global memory region associated with the agent
hsa_status_t get_global_memory_region(hsa_region_t region, void* data);

// Callback to acquire a fine grained global memory region associated
// with the agent
hsa_status_t get_global_memory_region_fine_grained(hsa_region_t region, void* data);

// Callback to acquire a course grained global memory region associated
// with the agent
hsa_status_t get_global_memory_region_coarse_grained(hsa_region_t region, void* data);

#endif  // _AGENT_UTILS_H_
