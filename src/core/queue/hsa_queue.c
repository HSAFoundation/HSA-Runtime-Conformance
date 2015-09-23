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
#include "hsa_queue.h"

DEFINE_TEST(queue_create_concurrent)
DEFINE_TEST(queue_create_parameters)
DEFINE_TEST(queue_callback)
DEFINE_TEST(queue_destroy_concurrent)
DEFINE_TEST(queue_dispatch_concurrent)
DEFINE_TEST(queue_full)
DEFINE_TEST(queue_multiple_dispatch)
DEFINE_TEST(queue_inactivate)
DEFINE_TEST(queue_size_create)
DEFINE_TEST(queue_multiple_queues)
DEFINE_TEST(queue_multi_gap)
DEFINE_TEST(queue_write_index_add_acq_rel_ordering)
DEFINE_TEST(queue_write_index_add_acquire_release_ordering)
DEFINE_TEST(queue_write_index_add_atomic)
DEFINE_TEST(queue_write_index_cas_acq_rel_ordering)
DEFINE_TEST(queue_write_index_cas_acquire_release_ordering)
DEFINE_TEST(queue_write_index_cas_atomic)
DEFINE_TEST(queue_write_index_load_store_atomic)

int main(int argc, char* argv[])
{
    INITIALIZE_TESTSUITE();
    ADD_TEST(queue_create_parameters);
    ADD_TEST(queue_callback);
    ADD_TEST(queue_create_concurrent)
    ADD_TEST(queue_destroy_concurrent);
    ADD_TEST(queue_dispatch_concurrent)
    ADD_TEST(queue_full)
    ADD_TEST(queue_multiple_dispatch);
    ADD_TEST(queue_inactivate);
    ADD_TEST(queue_size_create);
    ADD_TEST(queue_multiple_queues)
    ADD_TEST(queue_multi_gap);
    ADD_TEST(queue_write_index_add_acq_rel_ordering);
    ADD_TEST(queue_write_index_add_acquire_release_ordering);
    ADD_TEST(queue_write_index_add_atomic);
    ADD_TEST(queue_write_index_cas_acq_rel_ordering);
    ADD_TEST(queue_write_index_cas_acquire_release_ordering)
    ADD_TEST(queue_write_index_cas_atomic);
    // This test may not be correct. Doesn't monotonically
    // increment the write index.
    // ADD_TEST(queue_write_index_load_store_atomic);
    RUN_TESTS();
}
