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

#ifndef _FRAMEWORK_H_
#define _FRAMEWORK_H_

#include <check.h>
#include <check_stdint.h>

#ifndef EXIT_SUCCESS
    #define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
    #define EXIT_FAILURE -1
#endif

#define DEFINE_TEST(__test_name__) \
START_TEST(__test_name__) { \
    int error = test_##__test_name__(); \
    ck_assert_int_eq(error, 0); \
} \
END_TEST

#define INITIALIZE_TESTSUITE(__test_suite__) \
    int      number_failed = 0; \
    Suite   *suite = suite_create(#__test_suite__); \
    SRunner *runner = srunner_create(suite); \
    TCase   *test_case;

#define ADD_TEST(__test_name__) \
    test_case = tcase_create(#__test_name__); \
    tcase_add_test(test_case, __test_name__); \
    suite_add_tcase(suite, test_case);

#define RUN_TESTS() \
    srunner_run_all(runner, CK_NORMAL); \
    number_failed = srunner_ntests_failed(runner); \
    srunner_free(runner); \
    return(number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

// Convenience Test Function Mappings
#define ASSERT(expr)         ck_assert(expr)
#define ASSERT_MSG(expr, ...) ck_assert_msg(expr, ##__VA_ARGS__)
#define ABORT                ck_abort()
#define ABORT_MSG(...)       ck_abort_msg(##__VA_ARGS__)
#define ASSERT_INT_EQ(X, Y)  ck_assert_int_eq(X, Y)
#define ASSERT_INT_NE(X, Y)  ck_assert_int_ne(X, Y)
#define ASSERT_INT_LT(X, Y)  ck_assert_int_lt(X, Y)
#define ASSERT_INT_LE(X, Y)  ck_assert_int_le(X, Y)
#define ASSERT_INT_GT(X, Y)  ck_assert_int_gt(X, Y)
#define ASSERT_INT_GE(X, Y)  ck_assert_int_ge(X, Y)
#define ASSERT_UINT_EQ(X, Y) ck_assert_uint_eq(X, Y)
#define ASSERT_UINT_NE(X, Y) ck_assert_uint_ne(X, Y)
#define ASSERT_UINT_LT(X, Y) ck_assert_uint_lt(X, Y)
#define ASSERT_UINT_LE(X, Y) ck_assert_uint_le(X, Y)
#define ASSERT_UINT_GT(X, Y) ck_assert_uint_gt(X, Y)
#define ASSERT_UINT_GE(X, Y) ck_assert_uint_ge(X, Y)
#define ASSERT_STR_EQ(X, Y)  ck_assert_str_eq(X, Y)
#define ASSERT_STR_NE(X, Y)  ck_assert_str_ne(X, Y)
#define ASSERT_STR_LT(X, Y)  ck_assert_str_lt(X, Y)
#define ASSERT_STR_LE(X, Y)  ck_assert_str_le(X, Y)
#define ASSERT_STR_GT(X, Y)  ck_assert_str_gt(X, Y)
#define ASSERT_STR_GE(X, Y)  ck_assert_str_ge(X, Y)
#define ASSERT_PTR_EQ(X, Y)  ck_assert_ptr_eq(X, Y)
#define ASSERT_PTR_NE(X, Y)  ck_assert_ptr_ne(X, Y)
#define MARK_SRC_POINT()     mark_point()

#endif  // _FRAMEWORK_H_
