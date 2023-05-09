/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

/**
 * @file integration_test_wrapper.h
 * @brief Contains integration test specific functions and macros that wrap the
 * Unity test harness.
 */
#ifndef INTEGRATION_TEST_WRAPPER_H
#define INTEGRATION_TEST_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

#define RUN_TEST_NO_ARGS

#ifdef RUN_TEST
#undef RUN_TEST
#endif

#define RUN_TEST(TestFunc, TestLineNum, ...)                             \
    {                                                                    \
        if (!isList) {                                                   \
            Unity.CurrentTestName = #TestFunc "(" #__VA_ARGS__ ")";      \
            Unity.CurrentTestLineNumber = TestLineNum;                   \
            if (TEST_PROTECT()) {                                        \
                if (integration_test_wrapper.setup_fp != NULL) {         \
                    (*integration_test_wrapper.setup_fp)(                \
                        integration_test_wrapper.setup_data_p);          \
                }                                                        \
                TestFunc(__VA_ARGS__);                                   \
            }                                                            \
            if (TEST_PROTECT()) {                                        \
                if (integration_test_wrapper.teardown_fp != NULL) {      \
                    (*integration_test_wrapper.teardown_fp)(             \
                        integration_test_wrapper.teardown_data_p);       \
                }                                                        \
            }                                                            \
            UnityConcludeTest();                                         \
        } else {                                                         \
            printf("Test[%u] : %s\n", (unsigned int)Unity.NumberOfTests, \
                   #TestFunc);                                           \
        }                                                                \
        Unity.NumberOfTests++;                                           \
    }

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ace/unity.h"

typedef struct integration_test_wrapper_storage {
    void (*setup_fp)(void*);
    void* setup_data_p;
    void (*teardown_fp)(void*);
    void* teardown_data_p;
} aceIntegrationTestWrapper_storage_t;

extern bool isList;
extern aceIntegrationTestWrapper_storage_t integration_test_wrapper;

int aceIntegrationTestWrapper_begin(int argc, const char* argv[],
                                    const char* msg, void (*TestSetup)(void*),
                                    void (*TestTearDown)(void*),
                                    void* TestSetupDataP,
                                    void* TestTearDownDataP);
int aceIntegrationTestWrapper_end();

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  // INTEGRATION_TEST_WRAPPER_H
