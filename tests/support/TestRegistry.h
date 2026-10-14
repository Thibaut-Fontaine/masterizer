#pragma once

namespace masterizer::test {

using TestFn = void (*)();

/// Registers a test case. Returns true so it can initialise a static variable.
bool registerTest(const char* name, TestFn fn);

/// Runs every registered test whose name contains `filter` (or all when null).
/// Returns the number of failing tests.
int runAll(const char* filter);

/// Entry point used by main(): parses an optional name filter from argv.
int runAllTests(int argc, char** argv);

}  // namespace masterizer::test

/// Declares a self-registering test case with the given identifier.
#define TEST_CASE(name) \
    static void name(); \
    static const bool name##_registered = \
        ::masterizer::test::registerTest(#name, &name); \
    static void name()
