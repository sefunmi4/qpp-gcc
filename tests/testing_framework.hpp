#pragma once

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace qpp::test {

using TestFunc = std::function<void()>;

struct TestCase {
    std::string name;
    TestFunc func;
};

class ExpectationFailure : public std::runtime_error {
  public:
    explicit ExpectationFailure(std::string message);
};

void register_test(std::string name, TestFunc func);
void expect(bool condition, const char *expression, const char *file, int line);
void expect_near(double lhs, double rhs, double tolerance, const char *lhs_expr,
                 const char *rhs_expr, const char *file, int line);

template <typename LHS, typename RHS>
void expect_equal(const LHS &lhs, const RHS &rhs, const char *lhs_expr,
                  const char *rhs_expr, const char *file, int line) {
    if (!(lhs == rhs)) {
        std::ostringstream oss;
        oss << file << ':' << line << ": expected " << lhs_expr << " == " << rhs_expr;
        throw ExpectationFailure(oss.str());
    }
}

template <typename LHS, typename RHS>
void expect_not_equal(const LHS &lhs, const RHS &rhs, const char *lhs_expr,
                      const char *rhs_expr, const char *file, int line) {
    if (lhs == rhs) {
        std::ostringstream oss;
        oss << file << ':' << line << ": expected " << lhs_expr << " != " << rhs_expr;
        throw ExpectationFailure(oss.str());
    }
}

template <typename Exception, typename Callable>
void expect_throw(Callable &&callable, const char *expression, const char *file,
                  int line) {
    try {
        callable();
    } catch (const Exception &) {
        return;
    } catch (const std::exception &e) {
        std::ostringstream oss;
        oss << file << ':' << line << ": expected " << expression << " to throw "
            << typeid(Exception).name() << " but caught " << typeid(e).name()
            << " (" << e.what() << ')';
        throw ExpectationFailure(oss.str());
    } catch (...) {
        std::ostringstream oss;
        oss << file << ':' << line << ": expected " << expression << " to throw "
            << typeid(Exception).name() << " but caught an unknown exception";
        throw ExpectationFailure(oss.str());
    }
    std::ostringstream oss;
    oss << file << ':' << line << ": expected " << expression << " to throw "
        << typeid(Exception).name() << " but no exception was thrown";
    throw ExpectationFailure(oss.str());
}

class TestRegistration {
  public:
    TestRegistration(const char *name, TestFunc func);
};

int run_all_tests();

} // namespace qpp::test

#define QPP_TEST(name)                                                                \
    static void name();                                                               \
    static ::qpp::test::TestRegistration name##_registration{#name, &name};           \
    static void name()

#define QPP_EXPECT(expr) ::qpp::test::expect((expr), #expr, __FILE__, __LINE__)
#define QPP_EXPECT_TRUE(expr) QPP_EXPECT(expr)
#define QPP_EXPECT_FALSE(expr)                                                        \
    ::qpp::test::expect(!(expr), "!(" #expr ")", __FILE__, __LINE__)
#define QPP_EXPECT_EQ(lhs, rhs)                                                       \
    ::qpp::test::expect_equal((lhs), (rhs), #lhs, #rhs, __FILE__, __LINE__)
#define QPP_EXPECT_NE(lhs, rhs)                                                       \
    ::qpp::test::expect_not_equal((lhs), (rhs), #lhs, #rhs, __FILE__, __LINE__)
#define QPP_EXPECT_NEAR(lhs, rhs, tol)                                                \
    ::qpp::test::expect_near((lhs), (rhs), (tol), #lhs, #rhs, __FILE__, __LINE__)
#define QPP_EXPECT_THROW(expr, exception_type)                                        \
    ::qpp::test::expect_throw<exception_type>([&]() { expr; }, #expr, __FILE__, __LINE__)

