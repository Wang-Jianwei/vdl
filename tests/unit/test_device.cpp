/**
 * @file test_device.cpp
 * @brief 测试设备层
 */

#include <catch.hpp>
#include <vdl/device/device.hpp>
#include <vdl/device/device_impl.hpp>
#include <vdl/device/device_guard.hpp>
#include <vdl/transport/mock_transport.hpp>
#include <vdl/codec/binary_codec.hpp>

// ============================================================================
// device_state_t 测试
// ============================================================================

TEST_CASE("device_state_t values", "[device]") {
    REQUIRE(static_cast<uint8_t>(vdl::device_state_t::disconnected) == 0);
    REQUIRE(static_cast<uint8_t>(vdl::device_state_t::connecting) == 1);
    REQUIRE(static_cast<uint8_t>(vdl::device_state_t::connected) == 2);
    REQUIRE(static_cast<uint8_t>(vdl::device_state_t::error) == 3);
}

TEST_CASE("device_state_name", "[device]") {
    REQUIRE(std::string(vdl::device_state_name(vdl::device_state_t::disconnected)) == "disconnected");
    REQUIRE(std::string(vdl::device_state_name(vdl::device_state_t::connected)) == "connected");
}

// ============================================================================
// device_config_t 测试
// ============================================================================

TEST_CASE("device_config_t default values", "[device]") {
    vdl::device_config_t config;

    REQUIRE(config.command_timeout == 1000);
    REQUIRE(config.retry_delay == 100);
    REQUIRE(config.max_retries == 3);
    REQUIRE(config.auto_reconnect == true);
}

// ============================================================================
// device_impl_t 连接测试
// ============================================================================

TEST_CASE("device_impl_t initial state", "[device]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    REQUIRE(device.state() == vdl::device_state_t::disconnected);
    REQUIRE_FALSE(device.is_connected());
}

TEST_CASE("device_impl_t connect and disconnect", "[device]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    auto result = device.connect();
    REQUIRE(result.has_value());
    REQUIRE(device.state() == vdl::device_state_t::connected);
    REQUIRE(device.is_connected());

    device.disconnect();
    REQUIRE(device.state() == vdl::device_state_t::disconnected);
    REQUIRE_FALSE(device.is_connected());
}

TEST_CASE("device_impl_t connect failure", "[device]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    transport->set_fail_open(true);
    
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    auto result = device.connect();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(device.state() == vdl::device_state_t::error);
}

// ============================================================================
// device_impl_t 配置测试
// ============================================================================

TEST_CASE("device_impl_t config get and set", "[device]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    vdl::device_config_t config;
    config.command_timeout = 5000;
    config.max_retries = 5;

    device.set_config(config);

    REQUIRE(device.config().command_timeout == 5000);
    REQUIRE(device.config().max_retries == 5);
}

TEST_CASE("device_impl_t info get and set", "[device]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    vdl::device_info_t info;
    info.name = "Test Device";
    info.manufacturer = "VDL";
    info.model = "VDL-001";

    device.set_info(info);

    REQUIRE(device.info().name == "Test Device");
    REQUIRE(device.info().manufacturer == "VDL");
    REQUIRE(device.info().model == "VDL-001");
}

// ============================================================================
// device_impl_t 组件访问测试
// ============================================================================

TEST_CASE("device_impl_t component access", "[device]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    auto* transport_ptr = transport.get();
    auto* codec_ptr = codec.get();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    REQUIRE(device.transport() == transport_ptr);
    REQUIRE(device.codec() == codec_ptr);
}

TEST_CASE("device_impl_t type_name", "[device]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    REQUIRE(std::string(device.type_name()) == "device_impl");
}

// ============================================================================
// device_guard_t 测试
// ============================================================================

TEST_CASE("device_guard_t auto connect", "[device][guard]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    {
        vdl::device_guard_t guard(device);

        REQUIRE(guard.is_connected());
        REQUIRE_FALSE(guard.has_error());
    }

    // 守卫销毁后自动断开
    REQUIRE_FALSE(device.is_connected());
}

TEST_CASE("device_guard_t connect failure", "[device][guard]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    transport->set_fail_open(true);

    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    vdl::device_guard_t guard(device);

    REQUIRE_FALSE(guard.is_connected());
    REQUIRE(guard.has_error());
    REQUIRE(guard.connect_error().code() == vdl::error_code_t::connection_failed);
}

TEST_CASE("device_guard_t no auto connect", "[device][guard]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    {
        vdl::device_guard_t guard(device, false);  // 不自动连接
        REQUIRE_FALSE(guard.is_connected());
    }
}

TEST_CASE("device_guard_t release ownership", "[device][guard]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    {
        vdl::device_guard_t guard(device);
        REQUIRE(guard.is_connected());

        guard.release();  // 释放所有权
    }

    // 守卫销毁后保持连接
    REQUIRE(device.is_connected());

    device.disconnect();
}

TEST_CASE("make_device_guard factory function", "[device][guard]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    {
        auto guard = vdl::make_device_guard(device);
        REQUIRE(guard.is_connected());
    }

    REQUIRE_FALSE(device.is_connected());
}
