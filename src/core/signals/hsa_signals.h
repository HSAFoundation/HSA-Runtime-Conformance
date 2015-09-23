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

#ifndef _HSA_SIGNALS_H_
#define _HSA_SIGNALS_H_
extern int test_signal_create_concurrent();
extern int test_signal_create_initial_value();
extern int test_signal_create_max_consumers();
extern int test_signal_create_one_consumers();
extern int test_signal_create_zero_consumers();
extern int test_signal_destroy_concurrent();
extern int test_signal_kernel_multi_set();
extern int test_signal_kernel_multi_wait();
extern int test_signal_kernel_set();
extern int test_signal_kernel_wait();
extern int test_signal_wait_acquire_add();
extern int test_signal_wait_acquire_and();
extern int test_signal_wait_acquire_cas();
extern int test_signal_wait_acquire_exchange();
extern int test_signal_wait_acquire_or();
extern int test_signal_wait_acquire_subtract();
extern int test_signal_wait_acquire_xor();
extern int test_signal_wait_relaxed_add();
extern int test_signal_wait_relaxed_and();
extern int test_signal_wait_relaxed_cas();
extern int test_signal_wait_relaxed_exchange();
extern int test_signal_wait_relaxed_or();
extern int test_signal_wait_relaxed_subtract();
extern int test_signal_wait_relaxed_xor();
extern int test_signal_wait_conditions();
extern int test_signal_wait_expectancy();
extern int test_signal_wait_satisfied_conditions();
extern int test_signal_wait_store_release();
extern int test_signal_wait_store_relaxed();
extern int test_signal_wait_acquire_timeout();
extern int test_signal_wait_relaxed_timeout();
extern int test_signal_store_release_load_acquire_ordering();
extern int test_signal_store_release_load_acquire_ordering_transitive();
extern int test_signal_load_store_atomic();
extern int test_signal_add_acq_rel_ordering();
extern int test_signal_add_acq_rel_ordering_transitive();
extern int test_signal_add_acquire_release_ordering();
extern int test_signal_add_acquire_release_ordering_transitive();
extern int test_signal_add_atomic_acq_rel();
extern int test_signal_add_atomic_acquire();
extern int test_signal_add_atomic_release();
extern int test_signal_add_atomic_relaxed();
extern int test_signal_and_acq_rel_ordering();
extern int test_signal_and_acq_rel_ordering_transitive();
extern int test_signal_and_acquire_release_ordering();
extern int test_signal_and_acquire_release_ordering_transitive();
extern int test_signal_and_atomic_acq_rel();
extern int test_signal_and_atomic_acquire();
extern int test_signal_and_atomic_release();
extern int test_signal_and_atomic_relaxed();
extern int test_signal_cas_acq_rel_ordering();
extern int test_signal_cas_acq_rel_ordering_transitive();
extern int test_signal_cas_acquire_release_ordering();
extern int test_signal_cas_acquire_release_ordering_transitive();
extern int test_signal_cas_atomic_acq_rel();
extern int test_signal_cas_atomic_acquire();
extern int test_signal_cas_atomic_release();
extern int test_signal_cas_atomic_relaxed();
extern int test_signal_exchange_acq_rel_ordering();
extern int test_signal_exchange_acq_rel_ordering_transitive();
extern int test_signal_exchange_acquire_release_ordering();
extern int test_signal_exchange_acquire_release_ordering_transitive();
extern int test_signal_exchange_atomic_acq_rel();
extern int test_signal_exchange_atomic_acquire();
extern int test_signal_exchange_atomic_release();
extern int test_signal_exchange_atomic_relaxed();
extern int test_signal_or_acq_rel_ordering();
extern int test_signal_or_acq_rel_ordering_transitive();
extern int test_signal_or_acquire_release_ordering();
extern int test_signal_or_acquire_release_ordering_transitive();
extern int test_signal_or_atomic_acq_rel();
extern int test_signal_or_atomic_acquire();
extern int test_signal_or_atomic_release();
extern int test_signal_or_atomic_relaxed();
extern int test_signal_subtract_acq_rel_ordering();
extern int test_signal_subtract_acq_rel_ordering_transitive();
extern int test_signal_subtract_acquire_release_ordering_transitive();
extern int test_signal_subtract_atomic_acq_rel();
extern int test_signal_subtract_atomic_acquire();
extern int test_signal_subtract_atomic_release();
extern int test_signal_subtract_atomic_relaxed();
extern int test_signal_xor_acq_rel_ordering();
extern int test_signal_xor_acq_rel_ordering_transitive();
extern int test_signal_xor_acquire_release_ordering();
extern int test_signal_xor_acquire_release_ordering_transitive();
extern int test_signal_xor_atomic_acq_rel();
extern int test_signal_xor_atomic_acquire();
extern int test_signal_xor_atomic_release();
extern int test_signal_xor_atomic_relaxed();
#endif  // _HSA_SIGNALS_H_
