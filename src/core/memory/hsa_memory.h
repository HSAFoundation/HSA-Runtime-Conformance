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

#ifndef _HSA_MEMORY_H_
#define _HSA_MEMORY_H_

extern int test_memory_allocate_max_size();
extern int test_memory_allocate_zero_size();
extern int test_memory_assign_agent();
extern int test_memory_allocated_vector_copy_heap();
extern int test_memory_allocated_vector_copy_stack();
extern int test_memory_group_dynamic_allocation();
extern int test_memory_copy_allocated_to_allocated();
extern int test_memory_copy_allocated_to_registered();
extern int test_memory_copy_registered_to_allocated();
extern int test_memory_copy_registered_to_registered();
extern int test_memory_vector_copy_between_stack_and_heap();
extern int test_memory_vector_copy_heap_not_registered();
extern int test_memory_vector_copy_heap_registered();
extern int test_memory_vector_copy_stack_not_registered();
extern int test_memory_vector_copy_stack_registered();
extern int test_memory_minimum_region();
extern int test_memory_region_concurrent_get_info();
extern int test_memory_region_alignment();
extern int test_memory_register_subrange();
extern int test_memory_concurrent_allocate();
extern int test_memory_concurrent_free();
extern int test_memory_concurrent_register();
extern int test_memory_concurrent_deregister();
extern int test_memory_basic_allocate_free();
extern int test_memory_basic_register_deregister();
extern int test_memory_coherence_after_register();
extern int test_memory_copy_system_and_global();

#endif  // _HSA_MEMORY_H_
