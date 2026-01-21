/**
 * @file test_logging.cpp
 * @brief 测试日志系统
 */

#include <catch.hpp>
#include <vdl/core/logging.hpp>

// ============================================================================
// log_level_t 测试
// ============================================================================

TEST_CASE("log_level_t values", "[core][logging]") {
    REQUIRE(static_cast<int>(vdl::log_level_t::trace) == 0);
    REQUIRE(static_cast<int>(vdl::log_level_t::debug) == 1);
    REQUIRE(static_cast<int>(vdl::log_level_t::info) == 2);
    REQUIRE(static_cast<int>(vdl::log_level_t::warn) == 3);
    REQUIRE(static_cast<int>(vdl::log_level_t::error) == 4);
    REQUIRE(static_cast<int>(vdl::log_level_t::critical) == 5);
    REQUIRE(static_cast<int>(vdl::log_level_t::off) == 6);
}

TEST_CASE("log_level_t comparison", "[core][logging]") {
    REQUIRE(vdl::log_level_t::trace < vdl::log_level_t::debug);
    REQUIRE(vdl::log_level_t::debug < vdl::log_level_t::info);
    REQUIRE(vdl::log_level_t::info < vdl::log_level_t::warn);
    REQUIRE(vdl::log_level_t::warn < vdl::log_level_t::error);
    REQUIRE(vdl::log_level_t::error < vdl::log_level_t::critical);
    REQUIRE(vdl::log_level_t::critical < vdl::log_level_t::off);
}

// ============================================================================
// 日志级别设置测试
// ============================================================================

TEST_CASE("set_log_level and get_log_level", "[core][logging]") {
    auto original = vdl::get_log_level();

    vdl::set_log_level(vdl::log_level_t::debug);
    REQUIRE(vdl::get_log_level() == vdl::log_level_t::debug);

    vdl::set_log_level(vdl::log_level_t::error);
    REQUIRE(vdl::get_log_level() == vdl::log_level_t::error);

    vdl::set_log_level(original);
}

// ============================================================================
// 日志宏测试
// ============================================================================

TEST_CASE("log macros compile", "[core][logging]") {
    auto original = vdl::get_log_level();
    vdl::set_log_level(vdl::log_level_t::off);

    // 使用带参数的格式以满足 C++11 可变参数要求
    VDL_LOG_TRACE("trace: %d", 1);
    VDL_LOG_DEBUG("debug: %d", 2);
    VDL_LOG_INFO("info: %d", 3);
    VDL_LOG_WARN("warn: %d", 4);
    VDL_LOG_ERROR("error: %d", 5);
    VDL_LOG_CRITICAL("critical: %d", 6);

    vdl::set_log_level(original);
    REQUIRE(true);
}

TEST_CASE("log level filtering", "[core][logging]") {
    auto original = vdl::get_log_level();
    
    vdl::set_log_level(vdl::log_level_t::warn);
    
    // 低于 warn 级别不输出
    VDL_LOG_TRACE("trace: %d", 0);
    VDL_LOG_DEBUG("debug: %d", 0);
    VDL_LOG_INFO("info: %d", 0);
    
    vdl::set_log_level(vdl::log_level_t::off);
    VDL_LOG_WARN("warn: %d", 0);
    VDL_LOG_ERROR("error: %d", 0);
    VDL_LOG_CRITICAL("critical: %d", 0);
    
    vdl::set_log_level(original);
    REQUIRE(true);
}

TEST_CASE("log level iteration", "[core][logging]") {
    auto original = vdl::get_log_level();
    
    for (int i = 0; i <= 6; ++i) {
        vdl::set_log_level(static_cast<vdl::log_level_t>(i));
        REQUIRE(static_cast<int>(vdl::get_log_level()) == i);
    }
    
    vdl::set_log_level(original);
}
