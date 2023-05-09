/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_UNITY_ASSERT_TEST_FIXTURE_H
#define SID_UNITY_ASSERT_TEST_FIXTURE_H

#include <unity.h>

#include <setjmp.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Issue an assert that can be caught by test fixture
 *
 * This is mainly intended to be used in the implementation of an assert macro, together with the
 * #SID_TEST_EXPECT_AN_ASSERT fixture
 */
void sid_assert_test_throw_assert(void);

/** Informs fixture that an assert is expected
 *
 * When sid_assert_test_throw_assert is called, it checks this flag. If assertions are not expected
 * the test is aborted.
 * This function is mainly intended for it to be called by the #SID_TEST_EXPECT_AN_ASSERT fixture
 *
 * @param[in] expect_assert If true the fixture will expect an assert to trigger
 */
void sid_assert_test_expect_assert(bool expect_assert);

/** Gets the frame storage allocated by sid_assert_test
 *
 * This is mainly intended to be used by the #SID_TEST_EXPECT_AN_ASSERT fixture since
 * we have to call setjmp directly without any wrapper (otherwise it would be undefined behavior to
 * jump back to a function that already returned)
 */
jmp_buf *sid_assert_test_get_abort_frame(void);

/** A test fixture that can capture if the given expression asserts
 *
 * To properly capture an assert condition, the assert macro or library used by the given
 * expression needs to be redirected to call sid_assert_test_throw_assert() whenever the assertion fails.
 */
#define SID_TEST_EXPECT_AN_ASSERT(x)                                    \
    do {                                                                \
        volatile bool test_asserted = false;                            \
        if (setjmp(*sid_assert_test_get_abort_frame()) == 0) {          \
            sid_assert_test_expect_assert(true);                        \
            /* We expect the given expression to cause an assertion.    \
             * If the assertion triggers, the assert macro should call  \
             * sid_assert_test_throw_assert(), which will cause a       \
             * jump back to the if condition above but the return       \
             * value won't be zero */                                   \
            (x);                                                        \
        } else {                                                        \
            test_asserted = true;                                       \
        }                                                               \
        TEST_ASSERT_TRUE_MESSAGE(test_asserted, "no asserts occurred"); \
        sid_assert_test_expect_assert(false);                           \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // SID_UNITY_ASSERT_TEST_FIXTURE_H
