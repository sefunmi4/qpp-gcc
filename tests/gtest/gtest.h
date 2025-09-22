#pragma once

#include <cmath>
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace testing {

namespace detail {
template <typename T>
std::string to_string(const T &value) {
    if constexpr (std::is_enum_v<T>) {
        using Underlying = std::underlying_type_t<T>;
        return to_string(static_cast<Underlying>(value));
    } else {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
}

inline std::string comparison_message(const std::string &lhs_name, const std::string &lhs_value,
                                      const std::string &rhs_name, const std::string &rhs_value) {
    std::ostringstream oss;
    oss << lhs_name << ": " << lhs_value << ", " << rhs_name << ": " << rhs_value;
    return oss.str();
}

inline std::string near_message(double lhs, double rhs, double eps) {
    std::ostringstream oss;
    oss << "lhs: " << lhs << ", rhs: " << rhs << ", abs_error: " << eps;
    return oss.str();
}
} // namespace detail

struct TestFailure : public std::exception {
    const char *what() const noexcept override { return "test failure"; }
};

using TestFunction = void (*)();

struct TestInfo {
    std::string suite;
    std::string name;
    TestFunction function;
};

inline std::vector<TestInfo> &registry() {
    static std::vector<TestInfo> tests;
    return tests;
}

struct TestRegistrar {
    TestRegistrar(const char *suite, const char *name, TestFunction func) {
        registry().push_back({suite, name, func});
    }
};

inline void InitGoogleTest(int *, char **) {}

inline void report_failure(const char *expr, const char *file, int line, const std::string &msg) {
    std::cerr << file << ":" << line << ": Failure\n";
    std::cerr << "  Expected: " << expr << "\n";
    if (!msg.empty())
        std::cerr << "  " << msg << "\n";
    throw TestFailure{};
}

inline int RunAllTests() {
    int failed = 0;
    for (const auto &test : registry()) {
        std::cout << "[ RUN      ] " << test.suite << "." << test.name << '\n';
        try {
            test.function();
            std::cout << "[       OK ] " << test.suite << "." << test.name << '\n';
        } catch (const TestFailure &) {
            ++failed;
            std::cout << "[  FAILED  ] " << test.suite << "." << test.name << '\n';
        } catch (const std::exception &ex) {
            ++failed;
            std::cout << "[  FAILED  ] " << test.suite << "." << test.name
                      << " (unexpected exception: " << ex.what() << ")\n";
        } catch (...) {
            ++failed;
            std::cout << "[  FAILED  ] " << test.suite << "." << test.name
                      << " (unknown exception)\n";
        }
    }
    std::size_t total = registry().size();
    if (failed == 0)
        std::cout << "[  PASSED  ] " << total << " test" << (total == 1 ? "" : "s") << "\n";
    else
        std::cout << "[  FAILED  ] " << failed << " test" << (failed == 1 ? "" : "s") << "\n";
    return failed == 0 ? 0 : 1;
}

} // namespace testing

#define TEST(suite, name)                                                                     \
    static void suite##_##name##_Test();                                                      \
    static ::testing::TestRegistrar suite##_##name##_registrar(#suite, #name,                 \
                                                              &suite##_##name##_Test);        \
    static void suite##_##name##_Test()

#define EXPECT_TRUE(condition)                                                                \
    do {                                                                                      \
        if (!(condition))                                                                     \
            ::testing::report_failure(#condition, __FILE__, __LINE__, "condition is false"); \
    } while (false)

#define EXPECT_FALSE(condition)                                                               \
    do {                                                                                      \
        if (condition)                                                                        \
            ::testing::report_failure(#condition, __FILE__, __LINE__, "condition is true");  \
    } while (false)

#define EXPECT_EQ(expected, actual)                                                                             \
    do {                                                                                                        \
        auto _expected = (expected);                                                                            \
        auto _actual = (actual);                                                                                \
        if (!(_expected == _actual))                                                                            \
            ::testing::report_failure(#expected " == " #actual, __FILE__, __LINE__,                             \
                                      ::testing::detail::comparison_message("expected",                        \
                                                                            ::testing::detail::to_string(_expected), \
                                                                            "actual",                          \
                                                                            ::testing::detail::to_string(_actual))); \
    } while (false)

#define EXPECT_NE(val1, val2)                                                                                    \
    do {                                                                                                         \
        auto _v1 = (val1);                                                                                       \
        auto _v2 = (val2);                                                                                       \
        if (_v1 == _v2)                                                                                          \
            ::testing::report_failure(#val1 " != " #val2, __FILE__, __LINE__,                                   \
                                      ::testing::detail::comparison_message("val1",                             \
                                                                            ::testing::detail::to_string(_v1),   \
                                                                            "val2",                             \
                                                                            ::testing::detail::to_string(_v2))); \
    } while (false)

#define EXPECT_LT(val1, val2)                                                                                    \
    do {                                                                                                         \
        auto _v1 = (val1);                                                                                       \
        auto _v2 = (val2);                                                                                       \
        if (!(_v1 < _v2))                                                                                        \
            ::testing::report_failure(#val1 " < " #val2, __FILE__, __LINE__,                                    \
                                      ::testing::detail::comparison_message("lhs",                              \
                                                                            ::testing::detail::to_string(_v1),   \
                                                                            "rhs",                              \
                                                                            ::testing::detail::to_string(_v2))); \
    } while (false)

#define EXPECT_LE(val1, val2)                                                                                    \
    do {                                                                                                         \
        auto _v1 = (val1);                                                                                       \
        auto _v2 = (val2);                                                                                       \
        if (!(_v1 <= _v2))                                                                                       \
            ::testing::report_failure(#val1 " <= " #val2, __FILE__, __LINE__,                                   \
                                      ::testing::detail::comparison_message("lhs",                              \
                                                                            ::testing::detail::to_string(_v1),   \
                                                                            "rhs",                              \
                                                                            ::testing::detail::to_string(_v2))); \
    } while (false)

#define EXPECT_GT(val1, val2)                                                                                    \
    do {                                                                                                         \
        auto _v1 = (val1);                                                                                       \
        auto _v2 = (val2);                                                                                       \
        if (!(_v1 > _v2))                                                                                        \
            ::testing::report_failure(#val1 " > " #val2, __FILE__, __LINE__,                                    \
                                      ::testing::detail::comparison_message("lhs",                              \
                                                                            ::testing::detail::to_string(_v1),   \
                                                                            "rhs",                              \
                                                                            ::testing::detail::to_string(_v2))); \
    } while (false)

#define EXPECT_GE(val1, val2)                                                                                    \
    do {                                                                                                         \
        auto _v1 = (val1);                                                                                       \
        auto _v2 = (val2);                                                                                       \
        if (!(_v1 >= _v2))                                                                                       \
            ::testing::report_failure(#val1 " >= " #val2, __FILE__, __LINE__,                                   \
                                      ::testing::detail::comparison_message("lhs",                              \
                                                                            ::testing::detail::to_string(_v1),   \
                                                                            "rhs",                              \
                                                                            ::testing::detail::to_string(_v2))); \
    } while (false)

#define EXPECT_NEAR(val1, val2, abs_error)                                                                     \
    do {                                                                                                       \
        auto _v1 = (val1);                                                                                      \
        auto _v2 = (val2);                                                                                      \
        auto _eps = (abs_error);                                                                                \
        if (std::fabs(_v1 - _v2) > _eps)                                                                        \
            ::testing::report_failure(#val1 " ~= " #val2, __FILE__, __LINE__,                                   \
                                      ::testing::detail::near_message(_v1, _v2, _eps));                         \
    } while (false)

#define RUN_ALL_TESTS() ::testing::RunAllTests()

