#pragma once

namespace masterizer::test {

/// Per-test execution state, including the running failure count.
class TestContext {
public:
    static TestContext& current();
    void begin(const char* name);
    void fail(const char* file, int line, const char* expression);
    bool passed() const;
    const char* name() const;

private:
    const char* name_ = "";
    int failures_ = 0;
};

}  // namespace masterizer::test
