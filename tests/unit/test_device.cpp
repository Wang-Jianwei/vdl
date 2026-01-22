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
    REQUIRE(static_cast<uint8_t>(vdl::device_state_t::reconnecting) == 3);
    REQUIRE(static_cast<uint8_t>(vdl::device_state_t::error) == 4);
}

TEST_CASE("device_state_name", "[device]") {
    REQUIRE(std::string(vdl::device_state_name(vdl::device_state_t::disconnected)) == "disconnected");
    REQUIRE(std::string(vdl::device_state_name(vdl::device_state_t::connected)) == "connected");
    REQUIRE(std::string(vdl::device_state_name(vdl::device_state_t::reconnecting)) == "reconnecting");
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

// ============================================================================
// 重试机制测试
// ============================================================================

TEST_CASE("device_impl retries on write failure", "[device][retry]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    transport->set_fail_write_times(1);  // 第一次写失败，第二次成功
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    vdl::device_config_t cfg;
    cfg.max_retries = 2;
    cfg.retry_delay = 0;  // 测试中不等待
    device.set_config(cfg);

    REQUIRE(device.connect().has_value());

    vdl::command_t cmd;
    cmd.set_function_code(0x03);
    cmd.set_data({0x01, 0x02});

    // 预置响应帧（使用同一编码作为回环）
    vdl::binary_codec_t codec_helper;
    auto frame = codec_helper.encode(cmd);
    REQUIRE(frame.has_value());
    device.transport()->flush_read();
    static_cast<vdl::mock_transport_t*>(device.transport())->set_response(*frame);

    auto result = device.execute(cmd);
    REQUIRE(result.has_value());
    REQUIRE(result->function_code() == cmd.function_code());
}

TEST_CASE("device_impl retries on read failure", "[device][retry]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    transport->set_fail_read_times(1);  // 第一次读失败
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    vdl::device_config_t cfg;
    cfg.max_retries = 2;
    cfg.retry_delay = 0;
    device.set_config(cfg);

    REQUIRE(device.connect().has_value());

    vdl::command_t cmd;
    cmd.set_function_code(0x02);
    cmd.set_data({0x0A});

    vdl::binary_codec_t codec_helper;
    auto frame = codec_helper.encode(cmd);
    REQUIRE(frame.has_value());
    static_cast<vdl::mock_transport_t*>(device.transport())->set_response(*frame);

    auto result = device.execute(cmd);
    REQUIRE(result.has_value());
    REQUIRE(result->function_code() == cmd.function_code());
}

TEST_CASE("device_impl gives up after max retries", "[device][retry]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    transport->set_fail_write_times(3);  // 超过重试次数
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    vdl::device_config_t cfg;
    cfg.max_retries = 2;
    cfg.retry_delay = 0;
    device.set_config(cfg);

    REQUIRE(device.connect().has_value());

    vdl::command_t cmd;
    cmd.set_function_code(0x05);

    auto result = device.execute(cmd);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::write_failed);
}

// ============================================================================
// 自动重连机制测试
// ============================================================================

TEST_CASE("device_impl auto-reconnect restores connection", "[device][reconnect]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto* transport_ptr = transport.get();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    vdl::device_config_t cfg;
    cfg.max_retries = 2;
    cfg.retry_delay = 0;
    device.set_config(cfg);

    REQUIRE(device.connect().has_value());
    REQUIRE(transport_ptr->open_count() == 1u);

    // 强制写入失败以触发自动重连
    transport_ptr->set_fail_write_times(2);

    vdl::command_t cmd;
    cmd.set_function_code(0x06);

    auto result = device.execute(cmd);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::write_failed);

    // 自动重连应已完成，保持连接状态
    REQUIRE(device.is_connected());
    REQUIRE(device.state() == vdl::device_state_t::connected);
    REQUIRE(transport_ptr->open_count() == 2u);  // 1 次连接 + 1 次重连
}

TEST_CASE("device_impl auto-reconnect stops after failures", "[device][reconnect]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto* transport_ptr = transport.get();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    vdl::device_config_t cfg;
    cfg.max_retries = 2;
    cfg.retry_delay = 0;
    cfg.reconnect_delay = 0;
    device.set_config(cfg);

    REQUIRE(device.connect().has_value());
    REQUIRE(transport_ptr->open_count() == 1u);

    // 写入失败触发错误，随后打开也会连续失败
    transport_ptr->set_fail_write_times(cfg.max_retries);
    transport_ptr->set_fail_open_times(cfg.max_retries);

    vdl::command_t cmd;
    cmd.set_function_code(0x07);

    auto result = device.execute(cmd);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::write_failed);

    REQUIRE_FALSE(device.is_connected());
    REQUIRE(device.state() == vdl::device_state_t::error);
    REQUIRE(transport_ptr->open_count() == static_cast<uint32_t>(1 + cfg.max_retries));
}

TEST_CASE("device_impl reconnect callback is invoked", "[device][reconnect]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto* transport_ptr = transport.get();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    vdl::device_config_t cfg;
    cfg.max_retries = 2;
    cfg.retry_delay = 0;
    cfg.reconnect_delay = 0;
    device.set_config(cfg);

    // 记录回调事件
    std::vector<vdl::reconnect_event_t> events;
    device.set_reconnect_callback([&events](vdl::reconnect_event_t event, 
                                             uint8_t /*attempt*/, 
                                             uint8_t /*max_attempts*/,
                                             const vdl::error_t& /*error*/) {
        events.push_back(event);
    });

    REQUIRE(device.connect().has_value());

    // 写入失败后会触发重连
    transport_ptr->set_fail_write_times(cfg.max_retries);

    vdl::command_t cmd;
    cmd.set_function_code(0x08);

    auto result = device.execute(cmd);
    REQUIRE_FALSE(result.has_value());

    // 应该有: started, attempting(x2), success
    REQUIRE(events.size() >= 3);
    REQUIRE(events[0] == vdl::reconnect_event_t::started);
    REQUIRE(events[1] == vdl::reconnect_event_t::attempting);
    REQUIRE(events.back() == vdl::reconnect_event_t::success);
}

TEST_CASE("device_impl does not reconnect on protocol error", "[device][reconnect]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto* transport_ptr = transport.get();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    vdl::device_config_t cfg;
    cfg.max_retries = 2;
    cfg.retry_delay = 0;
    cfg.reconnect_delay = 0;
    device.set_config(cfg);

    REQUIRE(device.connect().has_value());
    uint32_t initial_open_count = transport_ptr->open_count();

    // 设置无效响应（会导致协议解析错误而非传输错误）
    transport_ptr->set_response({0xFF, 0xFF});  // 无效帧

    vdl::command_t cmd;
    cmd.set_function_code(0x09);

    auto result = device.execute(cmd);
    // 协议错误不应触发重连
    REQUIRE(transport_ptr->open_count() == initial_open_count);
}

TEST_CASE("device_impl manual reconnect", "[device][reconnect]") {
    auto transport = vdl::make_unique<vdl::mock_transport_t>();
    auto* transport_ptr = transport.get();
    auto codec = vdl::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));

    vdl::device_config_t cfg;
    cfg.max_retries = 2;
    cfg.reconnect_delay = 0;
    device.set_config(cfg);

    REQUIRE(device.connect().has_value());
    device.disconnect();
    REQUIRE_FALSE(device.is_connected());

    auto reconnect_result = device.reconnect();
    REQUIRE(reconnect_result.has_value());
    REQUIRE(device.is_connected());
    REQUIRE(transport_ptr->open_count() == 2u);
}
