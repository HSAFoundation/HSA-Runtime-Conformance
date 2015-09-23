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

/*
 * Test Name: signal_update_basic
 * Scope: Conformance
 *
 * Purpose: Verifies that all of the signal modification APIs are functional
 * in a single thread.
 *
 * Test Description:
 * 1) Create a signal.
 * 2) For each of the signal modification APIs
 *    a) Change the signal value to a new value.
 *    b) Read back the signal value with hsa_signal_load
 * 3) The modification APIs under test are:
 *    - hsa_signal_store_(acquire|relaxed)
 *    - hsa_signal_exchange_(acq_rel|acquire|relaxed|release)
 *    - hsa_signal_cas_(acq_rel|acquire|relaxed|release)
 *    - hsa_signal_add_(acq_rel|acquire|relaxed|release)
 *    - hsa_signal_subtract_(acq_rel|acquire|relaxed|release)
 *    - hsa_signal_and_(acq_rel|acquire|relaxed|release)
 *    - hsa_signal_or_(acq_rel|acquire|relaxed|release)
 *    - hsa_signal_xor_(acq_rel|acquire|relaxed|release)
 *
 * Expected Results: The hsa_signal_load should return the expected value
 * after each modification.
 *
 */

#include<hsa.h>
#include<framework.h>

void hsa_signal_store_update_basic() {
    hsa_signal_t signal;
    hsa_status_t status = hsa_signal_create(0, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_value_t value;

    hsa_signal_store_release(signal, 1);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(1 == value);

    hsa_signal_store_relaxed(signal, 0);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0 == value);

    status = hsa_signal_destroy(signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}

void hsa_signal_exchange_update_basic() {
    hsa_signal_t signal;
    hsa_status_t status = hsa_signal_create(0, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_value_t value;

    value = hsa_signal_exchange_acquire(signal, 1);
    ASSERT(0 == value);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(1 == value);

    value = hsa_signal_exchange_acq_rel(signal, 0);
    ASSERT(1 == value);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0 == value);

    value = hsa_signal_exchange_release(signal, 1);
    ASSERT(0 == value);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(1 == value);

    value = hsa_signal_exchange_relaxed(signal, 0);
    ASSERT(1 == value);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0 == value);

    status = hsa_signal_destroy(signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}

void hsa_signal_cas_update_basic() {
    hsa_signal_t signal;
    hsa_status_t status = hsa_signal_create(0, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_value_t value;

    value = hsa_signal_cas_acquire(signal, 0, 1);
    ASSERT(0 == value);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(1 == value);

    value = hsa_signal_cas_acq_rel(signal, 1, 0);
    ASSERT(1 == value);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0 == value);

    value = hsa_signal_cas_release(signal, 0, 1);
    ASSERT(0 == value);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(1 == value);

    value = hsa_signal_cas_relaxed(signal, 1, 0);
    ASSERT(1 == value);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0 == value);

    status = hsa_signal_destroy(signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}

void hsa_signal_add_update_basic() {
    hsa_signal_t signal;
    hsa_status_t status = hsa_signal_create(0, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_value_t value;

    hsa_signal_add_acquire(signal, 1);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(1 == value);

    hsa_signal_add_acq_rel(signal, 1);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(2 == value);

    hsa_signal_add_release(signal, 1);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(3 == value);

    hsa_signal_add_relaxed(signal, 1);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(4 == value);

    status = hsa_signal_destroy(signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}

void hsa_signal_subtract_update_basic() {
    hsa_signal_t signal;
    hsa_status_t status = hsa_signal_create(4, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_value_t value;

    hsa_signal_subtract_acquire(signal, 1);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(3 == value);

    hsa_signal_subtract_acq_rel(signal, 1);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(2 == value);

    hsa_signal_subtract_release(signal, 1);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(1 == value);

    hsa_signal_subtract_relaxed(signal, 1);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0 == value);

    status = hsa_signal_destroy(signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}

void hsa_signal_and_update_basic() {
    hsa_signal_t signal;
    hsa_status_t status = hsa_signal_create(0x1111, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_value_t value;

    hsa_signal_and_acquire(signal, 0x0111);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x0111 == value);

    hsa_signal_and_acq_rel(signal, 0x0011);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x0011 == value);

    hsa_signal_and_release(signal, 0x0001);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x0001 == value);

    hsa_signal_and_relaxed(signal, 0x0000);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x0000 == value);

    status = hsa_signal_destroy(signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}

void hsa_signal_or_update_basic() {
    hsa_signal_t signal;
    hsa_status_t status = hsa_signal_create(0x0000, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_value_t value;

    hsa_signal_or_acquire(signal, 0x0001);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x0001 == value);

    hsa_signal_or_acq_rel(signal, 0x0010);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x0011 == value);

    hsa_signal_or_release(signal, 0x0100);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x0111 == value);

    hsa_signal_or_relaxed(signal, 0x1000);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x1111 == value);

    status = hsa_signal_destroy(signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}

void hsa_signal_xor_update_basic() {
    hsa_signal_t signal;
    hsa_status_t status = hsa_signal_create(0x0000, 0, NULL, &signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_value_t value;

    hsa_signal_xor_acquire(signal, 0x0001);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x0001 == value);

    hsa_signal_xor_acq_rel(signal, 0x0010);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x0011 == value);

    hsa_signal_xor_release(signal, 0x0100);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x0111 == value);

    hsa_signal_xor_relaxed(signal, 0x1000);
    value = hsa_signal_load_relaxed(signal);
    ASSERT(0x1111 == value);

    status = hsa_signal_destroy(signal);
    ASSERT(HSA_STATUS_SUCCESS == status);

    return;
}

int test_signal_update_basic() {
    hsa_status_t status;
    status = hsa_init();
    ASSERT(HSA_STATUS_SUCCESS == status);

    hsa_signal_store_update_basic();
    hsa_signal_exchange_update_basic();
    hsa_signal_cas_update_basic();
    hsa_signal_add_update_basic();
    hsa_signal_subtract_update_basic();
    hsa_signal_and_update_basic();
    hsa_signal_or_update_basic();
    hsa_signal_xor_update_basic();

    status = hsa_shut_down();
    ASSERT(HSA_STATUS_SUCCESS == status);

    return 0;
}
