#include "support/TestContext.h"

#include <cstdio>

namespace masterizer::test {

TestContext& TestContext::current() {
    static TestContext instance;
    return instance;
}

void TestContext::begin(const char* name) {
    name_ = name;
    failures_ = 0;
}

void TestContext::fail(const char* file, int line, const char* expression) {
    ++failures_;
    std::fprintf(stderr, "  %s:%d: CHECK failed: %s\n", file, line, expression);
}

bool TestContext::passed() const {
    return failures_ == 0;
}

const char* TestContext::name() const {
    return name_;
}

}  // namespace masterizer::test
