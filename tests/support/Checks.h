#pragma once

#include "support/TestContext.h"

#include <cmath>

/// Records a failure when `expr` is false and continues the test.
#define CHECK(expr) \
    do { \
        if (!(expr)) { \
            ::masterizer::test::TestContext::current().fail(__FILE__, __LINE__, #expr); \
        } \
    } while (false)

/// Records a failure when `expr` is false and aborts the current test.
#define REQUIRE(expr) \
    do { \
        if (!(expr)) { \
            ::masterizer::test::TestContext::current().fail(__FILE__, __LINE__, #expr); \
            return; \
        } \
    } while (false)

/// Records a failure when |actual - expected| exceeds `tolerance`.
#define CHECK_NEAR(actual, expected, tolerance) \
    do { \
        const double a_ = (actual); \
        const double e_ = (expected); \
        if (!(std::fabs(a_ - e_) <= (tolerance))) { \
            ::masterizer::test::TestContext::current().fail(__FILE__, __LINE__, #actual); \
        } \
    } while (false)
