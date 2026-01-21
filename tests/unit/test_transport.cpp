/**
 * @file test_transport.cpp
 * @brief 测试传输层
 */

#include <catch.hpp>
#include <vdl/transport/transport.hpp>
#include <vdl/transport/mock_transport.hpp>

// ============================================================================
// mock_transport_t 基础测试
// ============================================================================

TEST_CASE("mock_transport_t construction", "[transport][mock]") {
    vdl::mock_transport_t transport;
    
    REQUIRE_FALSE(transport.is_open());
    REQUIRE(std::string(transport.type_name()) == "mock");
}

TEST_CASE("mock_transport_t open and close", "[transport][mock]") {
    vdl::mock_transport_t transport;
    
    auto result = transport.open();
    REQUIRE(result.has_value());
    REQUIRE(transport.is_open());
    
    transport.close();
    REQUIRE_FALSE(transport.is_open());
}

TEST_CASE("mock_transport_t simulated open failure", "[transport][mock]") {
    vdl::mock_transport_t transport;
    transport.set_fail_open(true);
    
    auto result = transport.open();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::connection_failed);
}

// ============================================================================
// mock_transport_t 读写测试
// ============================================================================

TEST_CASE("mock_transport_t write and get written data", "[transport][mock]") {
    vdl::mock_transport_t transport;
    transport.open();

    vdl::byte_t data[] = {0x01, 0x02, 0x03, 0x04};
    vdl::const_byte_span_t span(data, 4);
    auto result = transport.write(span);

    REQUIRE(result.has_value());
    REQUIRE(*result == 4);

    auto written = transport.get_written_data();
    REQUIRE(written.size() == 4);
    REQUIRE(written[0] == 0x01);
    REQUIRE(written[3] == 0x04);
}

TEST_CASE("mock_transport_t read preset response", "[transport][mock]") {
    vdl::mock_transport_t transport;
    transport.open();

    // 预设响应
    transport.set_response({0xAA, 0xBB, 0xCC});

    // 读取
    vdl::bytes_t buffer(3);
    vdl::byte_span_t read_span(buffer.data(), buffer.size());
    auto result = transport.read(read_span);

    REQUIRE(result.has_value());
    REQUIRE(*result == 3);
    REQUIRE(buffer[0] == 0xAA);
    REQUIRE(buffer[1] == 0xBB);
    REQUIRE(buffer[2] == 0xCC);
}

TEST_CASE("mock_transport_t read without data returns timeout", "[transport][mock]") {
    vdl::mock_transport_t transport;
    transport.open();

    vdl::bytes_t buffer(10);
    vdl::byte_span_t read_span(buffer.data(), buffer.size());
    auto result = transport.read(read_span);

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::timeout);
}

TEST_CASE("mock_transport_t read when not connected", "[transport][mock]") {
    vdl::mock_transport_t transport;

    vdl::bytes_t buffer(10);
    vdl::byte_span_t read_span(buffer.data(), buffer.size());
    auto result = transport.read(read_span);

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::not_connected);
}

// ============================================================================
// mock_transport_t 错误模拟
// ============================================================================

TEST_CASE("mock_transport_t simulated read failure", "[transport][mock]") {
    vdl::mock_transport_t transport;
    transport.open();
    transport.set_response({0x01, 0x02});  // 需要有数据才不会先返回 timeout
    transport.set_fail_read(true);

    vdl::bytes_t buffer(10);
    vdl::byte_span_t read_span(buffer.data(), buffer.size());
    auto result = transport.read(read_span);

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::read_failed);
}

TEST_CASE("mock_transport_t simulated write failure", "[transport][mock]") {
    vdl::mock_transport_t transport;
    transport.open();
    transport.set_fail_write(true);

    vdl::byte_t data[] = {0x01, 0x02};
    vdl::const_byte_span_t span(data, 2);
    auto result = transport.write(span);

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::write_failed);
}

// ============================================================================
// transport_config_t 测试
// ============================================================================

TEST_CASE("transport_config_t defaults", "[transport][config]") {
    vdl::transport_config_t config;
    
    REQUIRE(config.read_timeout == 1000);
    REQUIRE(config.write_timeout == 1000);
    REQUIRE(config.connect_timeout == 5000);
    REQUIRE(config.read_buffer_size == 4096);
    REQUIRE(config.write_buffer_size == 4096);
}

TEST_CASE("transport_config_t custom values", "[transport][config]") {
    vdl::transport_config_t config;
    config.read_timeout = 500;
    config.write_timeout = 250;
    
    REQUIRE(config.read_timeout == 500);
    REQUIRE(config.write_timeout == 250);
}

// ============================================================================
// flush_read 测试
// ============================================================================

TEST_CASE("mock_transport_t flush_read", "[transport][mock]") {
    vdl::mock_transport_t transport;
    transport.open();
    
    transport.set_response({0x01, 0x02, 0x03});
    transport.flush_read();
    
    vdl::bytes_t buffer(10);
    vdl::byte_span_t read_span(buffer.data(), buffer.size());
    auto result = transport.read(read_span);
    
    // 刷新后应该没有数据
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::timeout);
}
