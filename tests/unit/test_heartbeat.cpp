/**
 * @file test_heartbeat.cpp
 * @brief 心跳模块测试
 */

#include <catch.hpp>
#include <vdl/heartbeat/heartbeat_config.hpp>
#include <vdl/heartbeat/heartbeat_strategy.hpp>
#include <vdl/heartbeat/heartbeat_runner.hpp>
#include <vdl/heartbeat/strategies/ping_heartbeat.hpp>
#include <vdl/heartbeat/strategies/echo_heartbeat.hpp>
#include <vdl/heartbeat/strategies/scpi_heartbeat.hpp>
#include <vdl/device/device_impl.hpp>
#include <vdl/transport/mock_transport.hpp>
#include <vdl/codec/binary_codec.hpp>
#include <vdl/core/memory.hpp>

#include <thread>
#include <chrono>

// ============================================================================
// heartbeat_config_t 测试
// ============================================================================

TEST_CASE("heartbeat_config_t default values", "[heartbeat]") {
    vdl::heartbeat_config_t config;

    REQUIRE(config.interval == 1000);
    REQUIRE(config.timeout == 500);
    REQUIRE(config.max_failures == 3);
    REQUIRE(config.pause_during_lock == true);
    REQUIRE(config.auto_reset_failures == true);
}

TEST_CASE("heartbeat_config_t custom values", "[heartbeat]") {
    vdl::heartbeat_config_t config;
    config.interval = 2000;
    config.timeout = 1000;
    config.max_failures = 5;

    REQUIRE(config.interval == 2000);
    REQUIRE(config.timeout == 1000);
    REQUIRE(config.max_failures == 5);
}

// ============================================================================
// ping_heartbeat_t 测试
// ============================================================================

TEST_CASE("ping_heartbeat_t make_heartbeat_command", "[heartbeat]") {
    vdl::ping_heartbeat_t ping;

    auto cmd_result = ping.make_heartbeat_command();
    REQUIRE(cmd_result.has_value());

    auto& cmd = *cmd_result;
    REQUIRE(cmd.function_code() == 0x00);
    REQUIRE(cmd.data().empty());
}

TEST_CASE("ping_heartbeat_t validate_response success", "[heartbeat]") {
    vdl::ping_heartbeat_t ping;

    vdl::response_t resp;
    resp.set_function_code(0x00);
    resp.set_data({0x01, 0x02, 0x03});

    REQUIRE(ping.validate_response(resp) == true);
}

TEST_CASE("ping_heartbeat_t validate_response failure", "[heartbeat]") {
    vdl::ping_heartbeat_t ping;

    vdl::response_t resp;
    resp.set_error_code(0xFF);

    REQUIRE(ping.validate_response(resp) == false);
}

TEST_CASE("ping_heartbeat_t name", "[heartbeat]") {
    vdl::ping_heartbeat_t ping;
    REQUIRE(std::string(ping.name()) == "ping_heartbeat");
}

// ============================================================================
// echo_heartbeat_t 测试
// ============================================================================

TEST_CASE("echo_heartbeat_t make_heartbeat_command", "[heartbeat]") {
    vdl::bytes_t echo_data = {0xAA, 0xBB, 0xCC};
    vdl::echo_heartbeat_t echo(0x01, echo_data);

    auto cmd_result = echo.make_heartbeat_command();
    REQUIRE(cmd_result.has_value());

    auto& cmd = *cmd_result;
    REQUIRE(cmd.function_code() == 0x01);
    REQUIRE(cmd.data() == echo_data);
}

TEST_CASE("echo_heartbeat_t validate_response success", "[heartbeat]") {
    vdl::bytes_t echo_data = {0xAA, 0xBB, 0xCC};
    vdl::echo_heartbeat_t echo(0x01, echo_data);

    vdl::response_t resp;
    resp.set_function_code(0x01);
    resp.set_data(echo_data);

    REQUIRE(echo.validate_response(resp) == true);
}

TEST_CASE("echo_heartbeat_t validate_response mismatch", "[heartbeat]") {
    vdl::bytes_t echo_data = {0xAA, 0xBB, 0xCC};
    vdl::echo_heartbeat_t echo(0x01, echo_data);

    vdl::response_t resp;
    resp.set_function_code(0x01);
    resp.set_data({0xAA, 0xBB, 0xDD});  // 数据不匹配

    REQUIRE(echo.validate_response(resp) == false);
}

TEST_CASE("echo_heartbeat_t validate_response error", "[heartbeat]") {
    vdl::bytes_t echo_data = {0xAA, 0xBB, 0xCC};
    vdl::echo_heartbeat_t echo(0x01, echo_data);

    vdl::response_t resp;
    resp.set_error_code(0xFF);

    REQUIRE(echo.validate_response(resp) == false);
}

TEST_CASE("echo_heartbeat_t name", "[heartbeat]") {
    vdl::echo_heartbeat_t echo;
    REQUIRE(std::string(echo.name()) == "echo_heartbeat");
}

// ============================================================================
// scpi_heartbeat_t 测试
// ============================================================================

TEST_CASE("scpi_heartbeat_t make_heartbeat_command", "[heartbeat]") {
    vdl::scpi_heartbeat_t scpi;

    auto cmd_result = scpi.make_heartbeat_command();
    REQUIRE(cmd_result.has_value());

    auto& cmd = *cmd_result;
    REQUIRE(cmd.function_code() == 0x04);  // Query
}

TEST_CASE("scpi_heartbeat_t validate_response success", "[heartbeat]") {
    vdl::scpi_heartbeat_t scpi;

    vdl::response_t resp;
    resp.set_function_code(0x04);
    resp.set_data({0x4B, 0x65, 0x79, 0x73, 0x69, 0x67, 0x68, 0x74});  // "Keysight"

    REQUIRE(scpi.validate_response(resp) == true);
}

TEST_CASE("scpi_heartbeat_t validate_response empty", "[heartbeat]") {
    vdl::scpi_heartbeat_t scpi;

    vdl::response_t resp;
    resp.set_function_code(0x04);
    resp.set_data({});

    REQUIRE(scpi.validate_response(resp) == false);
}

TEST_CASE("scpi_heartbeat_t validate_response error", "[heartbeat]") {
    vdl::scpi_heartbeat_t scpi;

    vdl::response_t resp;
    resp.set_error_code(0xFF);

    REQUIRE(scpi.validate_response(resp) == false);
}

TEST_CASE("scpi_heartbeat_t name", "[heartbeat]") {
    vdl::scpi_heartbeat_t scpi;
    REQUIRE(std::string(scpi.name()) == "scpi_heartbeat");
}

// ============================================================================
// heartbeat_runner_t 测试
// ============================================================================

TEST_CASE("heartbeat_runner_t creation", "[heartbeat]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();
    vdl::device_impl_t device(std::move(transport), std::move(codec));

    auto strategy = vdl::make_unique<vdl::ping_heartbeat_t>();
    vdl::heartbeat_config_t config;

    vdl::heartbeat_runner_t runner(device, std::move(strategy), config);

    REQUIRE(runner.is_running() == false);
    REQUIRE(runner.failure_count() == 0);
}

TEST_CASE("heartbeat_runner_t start and stop", "[heartbeat]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();
    vdl::device_impl_t device(std::move(transport), std::move(codec));
    device.connect();

    auto strategy = vdl::make_unique<vdl::ping_heartbeat_t>();
    vdl::heartbeat_config_t config;
    config.interval = 100;

    vdl::heartbeat_runner_t runner(device, std::move(strategy), config);

    auto start_result = runner.start();
    REQUIRE(start_result.has_value());
    REQUIRE(runner.is_running() == true);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    runner.stop();
    REQUIRE(runner.is_running() == false);
}

TEST_CASE("heartbeat_runner_t pause and resume", "[heartbeat]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();
    vdl::device_impl_t device(std::move(transport), std::move(codec));
    device.connect();

    auto strategy = vdl::make_unique<vdl::ping_heartbeat_t>();
    vdl::heartbeat_config_t config;
    config.interval = 100;

    vdl::heartbeat_runner_t runner(device, std::move(strategy), config);

    runner.start();
    REQUIRE(runner.is_running() == true);

    runner.pause();
    REQUIRE(runner.is_paused() == true);

    runner.resume();
    REQUIRE(runner.is_paused() == false);

    runner.stop();
}

TEST_CASE("heartbeat_runner_t counters", "[heartbeat]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();
    vdl::device_impl_t device(std::move(transport), std::move(codec));
    device.connect();

    auto strategy = vdl::make_unique<vdl::ping_heartbeat_t>();
    vdl::heartbeat_config_t config;
    config.interval = 100;

    vdl::heartbeat_runner_t runner(device, std::move(strategy), config);

    runner.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    runner.stop();

    REQUIRE(runner.success_count() > 0);
    REQUIRE(runner.failure_count() == 0);
}
