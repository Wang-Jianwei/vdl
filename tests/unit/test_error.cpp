/**
 * @file test_error.cpp
 * @brief 测试错误处理
 */

#include <catch.hpp>
#include <vdl/core/error.hpp>

// ============================================================================
// error_code_t 测试
// ============================================================================

TEST_CASE("error_code_t values", "[core][error]") {
    REQUIRE(static_cast<int>(vdl::error_code_t::ok) == 0);
    REQUIRE(static_cast<int>(vdl::error_code_t::unknown) == 100);
    REQUIRE(static_cast<int>(vdl::error_code_t::timeout) == 600);
    REQUIRE(static_cast<int>(vdl::error_code_t::invalid_argument) == 301);
    REQUIRE(static_cast<int>(vdl::error_code_t::invalid_frame) == 703);
    REQUIRE(static_cast<int>(vdl::error_code_t::device_error) == 500);
}

// ============================================================================
// error_category_t 测试
// ============================================================================

TEST_CASE("error_category_t values", "[core][error]") {
    REQUIRE(static_cast<int>(vdl::error_category_t::none) == 0);
    REQUIRE(static_cast<int>(vdl::error_category_t::general) == 1);
    REQUIRE(static_cast<int>(vdl::error_category_t::transport) == 10);
}

TEST_CASE("get_error_category", "[core][error]") {
    REQUIRE(vdl::get_error_category(vdl::error_code_t::ok) == vdl::error_category_t::none);
    REQUIRE(vdl::get_error_category(vdl::error_code_t::unknown) == vdl::error_category_t::general);
    REQUIRE(vdl::get_error_category(vdl::error_code_t::out_of_memory) == vdl::error_category_t::memory);
    REQUIRE(vdl::get_error_category(vdl::error_code_t::invalid_argument) == vdl::error_category_t::argument);
    REQUIRE(vdl::get_error_category(vdl::error_code_t::io_error) == vdl::error_category_t::io);
    REQUIRE(vdl::get_error_category(vdl::error_code_t::device_error) == vdl::error_category_t::device);
    REQUIRE(vdl::get_error_category(vdl::error_code_t::timeout) == vdl::error_category_t::concurrency);
    REQUIRE(vdl::get_error_category(vdl::error_code_t::protocol_error) == vdl::error_category_t::protocol);
    REQUIRE(vdl::get_error_category(vdl::error_code_t::config_error) == vdl::error_category_t::config);
    REQUIRE(vdl::get_error_category(vdl::error_code_t::invalid_state) == vdl::error_category_t::logic);
    REQUIRE(vdl::get_error_category(vdl::error_code_t::transport_error) == vdl::error_category_t::transport);
}

// ============================================================================
// error_t 测试
// ============================================================================

TEST_CASE("error_t default construction", "[core][error]") {
    vdl::error_t err;

    REQUIRE(err.code() == vdl::error_code_t::ok);
    REQUIRE(err.message().empty());
    REQUIRE(err.category() == vdl::error_category_t::none);
    REQUIRE(err.is_ok());
    REQUIRE_FALSE(err.is_error());
}

TEST_CASE("error_t construction with code", "[core][error]") {
    vdl::error_t err(vdl::error_code_t::timeout);

    REQUIRE(err.code() == vdl::error_code_t::timeout);
    REQUIRE(err.is_error());
    REQUIRE_FALSE(err.is_ok());
}

TEST_CASE("error_t construction with code and message", "[core][error]") {
    vdl::error_t err(vdl::error_code_t::connection_failed, "Cannot connect");

    REQUIRE(err.code() == vdl::error_code_t::connection_failed);
    REQUIRE(err.message() == "Cannot connect");
}

TEST_CASE("error_t bool conversion", "[core][error]") {
    vdl::error_t ok_err;
    vdl::error_t bad_err(vdl::error_code_t::unknown);

    // ok 返回 false (无错误)
    REQUIRE_FALSE(static_cast<bool>(ok_err));
    
    // 有错误返回 true
    REQUIRE(static_cast<bool>(bad_err));
}

TEST_CASE("error_t with_context", "[core][error]") {
    vdl::error_t err(vdl::error_code_t::read_error, "Read failed");
    err.with_context("in file A").with_context("at line 10");

    REQUIRE(err.context() == "in file A <- at line 10");
}

TEST_CASE("error_t to_string", "[core][error]") {
    vdl::error_t err(vdl::error_code_t::timeout, "Operation timed out");
    auto str = err.to_string();

    REQUIRE(str.find("timeout") != std::string::npos);
    REQUIRE(str.find("600") != std::string::npos);
    REQUIRE(str.find("Operation timed out") != std::string::npos);
}

TEST_CASE("get_error_name", "[core][error]") {
    REQUIRE(std::string(vdl::get_error_name(vdl::error_code_t::ok)) == "ok");
    REQUIRE(std::string(vdl::get_error_name(vdl::error_code_t::timeout)) == "timeout");
    REQUIRE(std::string(vdl::get_error_name(vdl::error_code_t::device_error)) == "device_error");
}

// ============================================================================
// result_t 测试
// ============================================================================

TEST_CASE("result_t with value", "[core][error]") {
    vdl::result_t<int> result = 42;

    REQUIRE(result.has_value());
    REQUIRE(*result == 42);
}

TEST_CASE("result_t with error", "[core][error]") {
    vdl::result_t<int> result = tl::make_unexpected(
        vdl::error_t(vdl::error_code_t::invalid_argument, "Bad arg"));

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::invalid_argument);
    REQUIRE(result.error().message() == "Bad arg");
}

TEST_CASE("result_t<void> success", "[core][error]") {
    vdl::result_t<void> result;

    REQUIRE(result.has_value());
}

TEST_CASE("result_t<void> error", "[core][error]") {
    vdl::result_t<void> result = tl::make_unexpected(
        vdl::error_t(vdl::error_code_t::timeout));

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::timeout);
}

// ============================================================================
// 工厂函数测试
// ============================================================================

TEST_CASE("make_ok", "[core][error]") {
    auto result = vdl::make_ok();

    REQUIRE(result.has_value());
}

TEST_CASE("make_error with code", "[core][error]") {
    auto result = vdl::make_error<int>(vdl::error_code_t::read_error);

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::read_error);
}

TEST_CASE("make_error with code and message", "[core][error]") {
    auto result = vdl::make_error<int>(vdl::error_code_t::write_error, 
                                        "Write operation failed");

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::write_error);
    REQUIRE(result.error().message() == "Write operation failed");
}

TEST_CASE("make_unexpected", "[core][error]") {
    auto unexpected = vdl::make_unexpected(vdl::error_code_t::busy, "Resource busy");
    vdl::result_t<int> result = unexpected;

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::busy);
}

// ============================================================================
// result_t 使用模式测试
// ============================================================================

vdl::result_t<int> divide(int a, int b) {
    if (b == 0) {
        return vdl::make_error<int>(vdl::error_code_t::invalid_argument,
                                     "Division by zero");
    }
    return a / b;
}

TEST_CASE("result_t function return value", "[core][error]") {
    SECTION("success case") {
        auto result = divide(10, 2);
        REQUIRE(result.has_value());
        REQUIRE(*result == 5);
    }

    SECTION("error case") {
        auto result = divide(10, 0);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error().code() == vdl::error_code_t::invalid_argument);
    }
}

TEST_CASE("result_t value_or", "[core][error]") {
    auto success = divide(10, 2);
    auto failure = divide(10, 0);

    REQUIRE(success.value_or(-1) == 5);
    REQUIRE(failure.value_or(-1) == -1);
}
