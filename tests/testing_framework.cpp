#include "testing_framework.hpp"

#include <cmath>
#include <exception>
#include <iostream>
#include <mutex>
#include <sstream>
#include <utility>
#include <vector>

namespace qpp::test {

namespace {
std::vector<TestCase> &registry() {
    static std::vector<TestCase> tests;
    return tests;
}

std::mutex &registry_mutex() {
    static std::mutex mtx;
    return mtx;
}
} // namespace

ExpectationFailure::ExpectationFailure(std::string message)
    : std::runtime_error(std::move(message)) {}

void register_test(std::string name, TestFunc func) {
    std::lock_guard<std::mutex> lock(registry_mutex());
    registry().push_back(TestCase{std::move(name), std::move(func)});
}

TestRegistration::TestRegistration(const char *name, TestFunc func) {
    register_test(name, std::move(func));
}

void expect(bool condition, const char *expression, const char *file, int line) {
    if (!condition) {
        std::ostringstream oss;
        oss << file << ':' << line << ": expectation failed: " << expression;
        throw ExpectationFailure(oss.str());
    }
}

void expect_near(double lhs, double rhs, double tolerance, const char *lhs_expr,
                 const char *rhs_expr, const char *file, int line) {
    if (tolerance < 0.0) {
        std::ostringstream oss;
        oss << file << ':' << line
            << ": tolerance must be non-negative for expectation involving " << lhs_expr
            << " and " << rhs_expr;
        throw ExpectationFailure(oss.str());
    }
    if (std::fabs(lhs - rhs) > tolerance) {
        std::ostringstream oss;
        oss << file << ':' << line << ": expected " << lhs_expr << " ≈ " << rhs_expr
            << " within " << tolerance << " but difference was " << std::fabs(lhs - rhs);
        throw ExpectationFailure(oss.str());
    }
}

int run_all_tests() {
    std::vector<TestCase> tests_copy;
    {
        std::lock_guard<std::mutex> lock(registry_mutex());
        tests_copy = registry();
    }

    std::size_t total = tests_copy.size();
    std::size_t failed = 0;

    std::cout << "[==========] Running " << total << " test(s)." << std::endl;

    for (const auto &test : tests_copy) {
        std::cout << "[ RUN      ] " << test.name << std::endl;
        try {
            test.func();
            std::cout << "[       OK ] " << test.name << std::endl;
        } catch (const ExpectationFailure &e) {
            ++failed;
            std::cout << "[  FAILED  ] " << test.name << std::endl;
            std::cerr << "  " << e.what() << std::endl;
        } catch (const std::exception &e) {
            ++failed;
            std::cout << "[  FAILED  ] " << test.name << std::endl;
            std::cerr << "  unexpected exception: " << e.what() << std::endl;
        } catch (...) {
            ++failed;
            std::cout << "[  FAILED  ] " << test.name << std::endl;
            std::cerr << "  unknown exception" << std::endl;
        }
    }

    std::cout << "[==========] " << total << " test(s) ran." << std::endl;
    std::cout << "[  PASSED  ] " << (total - failed) << " test(s)." << std::endl;
    if (failed != 0) {
        std::cout << "[  FAILED  ] " << failed << " test(s)." << std::endl;
    }

    return failed == 0 ? 0 : 1;
}

} // namespace qpp::test

int main() { return qpp::test::run_all_tests(); }

