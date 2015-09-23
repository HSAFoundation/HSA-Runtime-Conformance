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

#ifndef _HSA_QUEUE_H_
#define _HSA_QUEUE_H_
extern int test_concurrent_queue_create();
extern int test_queue_callback();
extern int test_queue_create_concurrent();
extern int test_queue_create_parameters();
extern int test_queue_destroy_concurrent();
extern int test_queue_dispatch_concurrent();
extern int test_queue_multi_gap();
extern int test_queue_multiple_queues();
extern int test_queue_multiple_dispatch();
extern int test_queue_full();
extern int test_queue_inactivate();
extern int test_queue_size_create();
extern int test_queue_write_index_add_acq_rel_ordering();
extern int test_queue_write_index_add_acquire_release_ordering();
extern int test_queue_write_index_add_atomic();
extern int test_queue_write_index_cas_acq_rel_ordering();
extern int test_queue_write_index_cas_acquire_release_ordering();
extern int test_queue_write_index_cas_atomic();
extern int test_queue_write_index_load_store_atomic();
#endif  // _HSA_QUEUE_H_
