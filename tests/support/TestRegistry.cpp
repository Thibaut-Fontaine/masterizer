#include "support/TestRegistry.h"

#include "support/TestContext.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace masterizer::test {
namespace {

struct Registration {
    const char* name;
    TestFn fn;
};

std::vector<Registration>& registrations() {
    static std::vector<Registration> all;
    return all;
}

}  // namespace

bool registerTest(const char* name, TestFn fn) {
    registrations().push_back({name, fn});
    return true;
}

int runAll(const char* filter) {
    int failures = 0;
    int run = 0;
    for (const Registration& reg : registrations()) {
        if (filter != nullptr && std::strstr(reg.name, filter) == nullptr) {
            continue;
        }
        ++run;
        TestContext::current().begin(reg.name);
        reg.fn();
        const bool ok = TestContext::current().passed();
        std::printf("%s %s\n", ok ? "PASS" : "FAIL", reg.name);
        if (!ok) {
            ++failures;
        }
    }
    std::printf("%d test(s) run, %d failed\n", run, failures);
    return failures;
}

int runAllTests(int argc, char** argv) {
    const char* filter = argc > 1 ? argv[1] : nullptr;
    return runAll(filter) == 0 ? 0 : 1;
}

}  // namespace masterizer::test
