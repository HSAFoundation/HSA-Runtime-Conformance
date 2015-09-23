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
#include "hsa_signals.h"

DEFINE_TEST(signal_create_concurrent);
DEFINE_TEST(signal_create_initial_value);
DEFINE_TEST(signal_create_max_consumers);
DEFINE_TEST(signal_create_one_consumers);
DEFINE_TEST(signal_create_zero_consumers);
DEFINE_TEST(signal_destroy_concurrent);
DEFINE_TEST(signal_kernel_multi_set);
DEFINE_TEST(signal_kernel_multi_wait);
DEFINE_TEST(signal_kernel_set);
DEFINE_TEST(signal_kernel_wait);
DEFINE_TEST(signal_wait_acquire_add);
DEFINE_TEST(signal_wait_relaxed_add);
DEFINE_TEST(signal_wait_acquire_and);
DEFINE_TEST(signal_wait_relaxed_and);
DEFINE_TEST(signal_wait_acquire_cas);
DEFINE_TEST(signal_wait_relaxed_cas);
DEFINE_TEST(signal_wait_conditions);
DEFINE_TEST(signal_wait_satisfied_conditions);
DEFINE_TEST(signal_wait_expectancy);
DEFINE_TEST(signal_wait_acquire_exchange);
DEFINE_TEST(signal_wait_relaxed_exchange);
DEFINE_TEST(signal_wait_acquire_or);
DEFINE_TEST(signal_wait_relaxed_or);
DEFINE_TEST(signal_wait_store_relaxed);
DEFINE_TEST(signal_wait_store_release);
DEFINE_TEST(signal_wait_acquire_subtract);
DEFINE_TEST(signal_wait_relaxed_subtract);
DEFINE_TEST(signal_wait_acquire_xor);
DEFINE_TEST(signal_wait_relaxed_xor);
DEFINE_TEST(signal_wait_acquire_timeout);
DEFINE_TEST(signal_wait_relaxed_timeout);
DEFINE_TEST(signal_store_release_load_acquire_ordering);
DEFINE_TEST(signal_store_release_load_acquire_ordering_transitive);
DEFINE_TEST(signal_load_store_atomic);
DEFINE_TEST(signal_add_acq_rel_ordering);
DEFINE_TEST(signal_add_acq_rel_ordering_transitive);
DEFINE_TEST(signal_add_acquire_release_ordering);
DEFINE_TEST(signal_add_acquire_release_ordering_transitive);
DEFINE_TEST(signal_add_atomic_acq_rel);
DEFINE_TEST(signal_add_atomic_acquire);
DEFINE_TEST(signal_add_atomic_release);
DEFINE_TEST(signal_add_atomic_relaxed);
DEFINE_TEST(signal_and_acq_rel_ordering);
DEFINE_TEST(signal_and_acq_rel_ordering_transitive);
DEFINE_TEST(signal_and_acquire_release_ordering);
DEFINE_TEST(signal_and_acquire_release_ordering_transitive);
DEFINE_TEST(signal_and_atomic_acq_rel);
DEFINE_TEST(signal_and_atomic_acquire);
DEFINE_TEST(signal_and_atomic_release);
DEFINE_TEST(signal_and_atomic_relaxed);
DEFINE_TEST(signal_cas_acq_rel_ordering);
DEFINE_TEST(signal_cas_acq_rel_ordering_transitive);
DEFINE_TEST(signal_cas_acquire_release_ordering);
DEFINE_TEST(signal_cas_acquire_release_ordering_transitive);
DEFINE_TEST(signal_cas_atomic_acq_rel);
DEFINE_TEST(signal_cas_atomic_acquire);
DEFINE_TEST(signal_cas_atomic_release);
DEFINE_TEST(signal_cas_atomic_relaxed);
DEFINE_TEST(signal_exchange_acq_rel_ordering);
DEFINE_TEST(signal_exchange_acq_rel_ordering_transitive);
DEFINE_TEST(signal_exchange_acquire_release_ordering);
DEFINE_TEST(signal_exchange_acquire_release_ordering_transitive);
DEFINE_TEST(signal_exchange_atomic_acq_rel);
DEFINE_TEST(signal_exchange_atomic_acquire);
DEFINE_TEST(signal_exchange_atomic_release);
DEFINE_TEST(signal_exchange_atomic_relaxed);
DEFINE_TEST(signal_or_acq_rel_ordering);
DEFINE_TEST(signal_or_acq_rel_ordering_transitive);
DEFINE_TEST(signal_or_acquire_release_ordering);
DEFINE_TEST(signal_or_acquire_release_ordering_transitive);
DEFINE_TEST(signal_or_atomic_acq_rel);
DEFINE_TEST(signal_or_atomic_acquire);
DEFINE_TEST(signal_or_atomic_release);
DEFINE_TEST(signal_or_atomic_relaxed);
DEFINE_TEST(signal_subtract_acq_rel_ordering);
DEFINE_TEST(signal_subtract_acq_rel_ordering_transitive);
DEFINE_TEST(signal_subtract_acquire_release_ordering_transitive);
DEFINE_TEST(signal_subtract_atomic_acq_rel);
DEFINE_TEST(signal_subtract_atomic_acquire);
DEFINE_TEST(signal_subtract_atomic_release);
DEFINE_TEST(signal_subtract_atomic_relaxed);
DEFINE_TEST(signal_xor_acq_rel_ordering);
DEFINE_TEST(signal_xor_acq_rel_ordering_transitive);
DEFINE_TEST(signal_xor_acquire_release_ordering);
DEFINE_TEST(signal_xor_acquire_release_ordering_transitive);
DEFINE_TEST(signal_xor_atomic_acq_rel);
DEFINE_TEST(signal_xor_atomic_acquire);
DEFINE_TEST(signal_xor_atomic_release);
DEFINE_TEST(signal_xor_atomic_relaxed);

int main(int argc, char* argv[]) {
    INITIALIZE_TESTSUITE();
    ADD_TEST(signal_create_concurrent);
    ADD_TEST(signal_create_initial_value);
    ADD_TEST(signal_create_max_consumers);
    ADD_TEST(signal_create_one_consumers);
    ADD_TEST(signal_create_zero_consumers);
    ADD_TEST(signal_destroy_concurrent);
    ADD_TEST(signal_kernel_multi_set);
    ADD_TEST(signal_kernel_multi_wait);
    ADD_TEST(signal_kernel_set);
    ADD_TEST(signal_kernel_wait);
    ADD_TEST(signal_wait_acquire_add);
    ADD_TEST(signal_wait_relaxed_add);
    ADD_TEST(signal_wait_acquire_and);
    ADD_TEST(signal_wait_relaxed_and);
    ADD_TEST(signal_wait_acquire_cas);
    ADD_TEST(signal_wait_relaxed_cas);
    ADD_TEST(signal_wait_conditions);
    ADD_TEST(signal_wait_satisfied_conditions);
    ADD_TEST(signal_wait_expectancy);
    ADD_TEST(signal_wait_acquire_exchange);
    ADD_TEST(signal_wait_relaxed_exchange);
    ADD_TEST(signal_wait_acquire_or);
    ADD_TEST(signal_wait_relaxed_or);
    ADD_TEST(signal_wait_store_relaxed);
    ADD_TEST(signal_wait_store_release);
    ADD_TEST(signal_wait_acquire_subtract);
    ADD_TEST(signal_wait_relaxed_subtract);
    ADD_TEST(signal_wait_acquire_xor);
    ADD_TEST(signal_wait_relaxed_xor);
    ADD_TEST(signal_wait_acquire_timeout);
    ADD_TEST(signal_wait_relaxed_timeout);
    ADD_TEST(signal_store_release_load_acquire_ordering);
    ADD_TEST(signal_store_release_load_acquire_ordering_transitive);
    ADD_TEST(signal_load_store_atomic);
    ADD_TEST(signal_add_acq_rel_ordering);
    ADD_TEST(signal_add_acq_rel_ordering_transitive);
    ADD_TEST(signal_add_acquire_release_ordering);
    ADD_TEST(signal_add_acquire_release_ordering_transitive);
    ADD_TEST(signal_add_atomic_acq_rel);
    ADD_TEST(signal_add_atomic_acquire);
    ADD_TEST(signal_add_atomic_release);
    ADD_TEST(signal_add_atomic_relaxed);
    ADD_TEST(signal_and_acq_rel_ordering);
    ADD_TEST(signal_and_acq_rel_ordering_transitive);
    ADD_TEST(signal_and_acquire_release_ordering);
    ADD_TEST(signal_and_acquire_release_ordering_transitive);
    ADD_TEST(signal_and_atomic_acq_rel);
    ADD_TEST(signal_and_atomic_acquire);
    ADD_TEST(signal_and_atomic_release);
    ADD_TEST(signal_and_atomic_relaxed);
    ADD_TEST(signal_cas_acq_rel_ordering);
    ADD_TEST(signal_cas_acq_rel_ordering_transitive);
    ADD_TEST(signal_cas_acquire_release_ordering);
    ADD_TEST(signal_cas_acquire_release_ordering_transitive);
    ADD_TEST(signal_cas_atomic_acq_rel);
    ADD_TEST(signal_cas_atomic_acquire);
    ADD_TEST(signal_cas_atomic_release);
    ADD_TEST(signal_cas_atomic_relaxed);
    ADD_TEST(signal_exchange_acq_rel_ordering);
    ADD_TEST(signal_exchange_acq_rel_ordering_transitive);
    ADD_TEST(signal_exchange_acquire_release_ordering);
    ADD_TEST(signal_exchange_acquire_release_ordering_transitive);
    ADD_TEST(signal_exchange_atomic_acq_rel);
    ADD_TEST(signal_exchange_atomic_acquire);
    ADD_TEST(signal_exchange_atomic_release);
    ADD_TEST(signal_exchange_atomic_relaxed);
    ADD_TEST(signal_or_acq_rel_ordering);
    ADD_TEST(signal_or_acq_rel_ordering_transitive);
    ADD_TEST(signal_or_acquire_release_ordering);
    ADD_TEST(signal_or_acquire_release_ordering_transitive);
    ADD_TEST(signal_or_atomic_acq_rel);
    ADD_TEST(signal_or_atomic_acquire);
    ADD_TEST(signal_or_atomic_release);
    ADD_TEST(signal_or_atomic_relaxed);
    ADD_TEST(signal_subtract_acq_rel_ordering);
    ADD_TEST(signal_subtract_acq_rel_ordering_transitive);
    ADD_TEST(signal_subtract_acquire_release_ordering_transitive);
    ADD_TEST(signal_subtract_atomic_acq_rel);
    ADD_TEST(signal_subtract_atomic_acquire);
    ADD_TEST(signal_subtract_atomic_release);
    ADD_TEST(signal_subtract_atomic_relaxed);
    ADD_TEST(signal_xor_acq_rel_ordering);
    ADD_TEST(signal_xor_acq_rel_ordering_transitive);
    ADD_TEST(signal_xor_acquire_release_ordering);
    ADD_TEST(signal_xor_acquire_release_ordering_transitive);
    ADD_TEST(signal_xor_atomic_acq_rel);
    ADD_TEST(signal_xor_atomic_acquire);
    ADD_TEST(signal_xor_atomic_release);
    ADD_TEST(signal_xor_atomic_relaxed);
    RUN_TESTS();
}
