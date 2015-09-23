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
#include "hsa_memory.h"

DEFINE_TEST(memory_allocate_max_size);
DEFINE_TEST(memory_allocate_zero_size);
DEFINE_TEST(memory_assign_agent);
DEFINE_TEST(memory_allocated_vector_copy_heap);
DEFINE_TEST(memory_allocated_vector_copy_stack);
DEFINE_TEST(memory_group_dynamic_allocation);
DEFINE_TEST(memory_copy_allocated_to_allocated)
DEFINE_TEST(memory_copy_allocated_to_registered)
DEFINE_TEST(memory_copy_registered_to_allocated)
DEFINE_TEST(memory_copy_registered_to_registered)
DEFINE_TEST(memory_vector_copy_between_stack_and_heap);
DEFINE_TEST(memory_vector_copy_heap_not_registered);
DEFINE_TEST(memory_vector_copy_heap_registered);
DEFINE_TEST(memory_vector_copy_stack_not_registered);
DEFINE_TEST(memory_vector_copy_stack_registered);
DEFINE_TEST(memory_minimum_region);
DEFINE_TEST(memory_region_concurrent_get_info);
DEFINE_TEST(memory_region_alignment);
DEFINE_TEST(memory_register_subrange);
DEFINE_TEST(memory_concurrent_allocate);
DEFINE_TEST(memory_concurrent_free);
DEFINE_TEST(memory_concurrent_register);
DEFINE_TEST(memory_concurrent_deregister);
DEFINE_TEST(memory_basic_allocate_free);
DEFINE_TEST(memory_basic_register_deregister);
DEFINE_TEST(memory_coherence_after_register);
DEFINE_TEST(memory_copy_system_and_global);

int main(int argc, char* argv[]) {
    INITIALIZE_TESTSUITE();
    ADD_TEST(memory_allocate_max_size);
    ADD_TEST(memory_allocate_zero_size);
    ADD_TEST(memory_assign_agent);
    ADD_TEST(memory_allocated_vector_copy_heap);
    ADD_TEST(memory_allocated_vector_copy_stack);
    ADD_TEST(memory_group_dynamic_allocation);
    ADD_TEST(memory_copy_allocated_to_allocated);
    ADD_TEST(memory_copy_allocated_to_registered);
    ADD_TEST(memory_copy_registered_to_allocated);
    ADD_TEST(memory_copy_registered_to_registered);
    ADD_TEST(memory_vector_copy_between_stack_and_heap);
    ADD_TEST(memory_vector_copy_heap_not_registered);
    ADD_TEST(memory_vector_copy_heap_registered);
    ADD_TEST(memory_vector_copy_stack_not_registered);
    ADD_TEST(memory_vector_copy_stack_registered);
    ADD_TEST(memory_minimum_region);
    ADD_TEST(memory_region_concurrent_get_info);
    ADD_TEST(memory_region_alignment);
    ADD_TEST(memory_register_subrange);
    ADD_TEST(memory_concurrent_allocate);
    ADD_TEST(memory_concurrent_free);
    ADD_TEST(memory_concurrent_register);
    ADD_TEST(memory_concurrent_deregister);
    ADD_TEST(memory_basic_allocate_free);
    ADD_TEST(memory_basic_register_deregister);
    ADD_TEST(memory_coherence_after_register);
    ADD_TEST(memory_copy_system_and_global);
    RUN_TESTS();
    return 0;
}
