/**
 * @file test_response.cpp
 * @brief 测试 response_t
 */

#include <catch.hpp>
#include <vdl/protocol/response.hpp>

// ============================================================================
// response_t 基本测试
// ============================================================================

TEST_CASE("response_t default construction", "[protocol][response]") {
    vdl::response_t resp;

    REQUIRE(resp.status() == vdl::response_status_t::invalid);
    REQUIRE(resp.function_code() == 0);
    REQUIRE(resp.error_code() == 0);
    REQUIRE_FALSE(resp.has_data());
}

TEST_CASE("response_t builder pattern", "[protocol][response]") {
    vdl::response_t resp;
    
    resp.set_status(vdl::response_status_t::success)
        .set_function_code(0x03)
        .set_data({0x00, 0x64, 0x00, 0xC8});

    REQUIRE(resp.is_success());
    REQUIRE(resp.function_code() == 0x03);
    REQUIRE(resp.has_data());
    REQUIRE(resp.data_size() == 4);
}

// ============================================================================
// response_t 状态检查测试
// ============================================================================

TEST_CASE("response_t status checks", "[protocol][response]") {
    vdl::response_t resp;

    SECTION("success status") {
        resp.set_status(vdl::response_status_t::success);
        REQUIRE(resp.is_success());
        REQUIRE_FALSE(resp.is_error());
        REQUIRE_FALSE(resp.is_busy());
    }

    SECTION("error status") {
        resp.set_status(vdl::response_status_t::error);
        REQUIRE_FALSE(resp.is_success());
        REQUIRE(resp.is_error());
    }

    SECTION("busy status") {
        resp.set_status(vdl::response_status_t::busy);
        REQUIRE(resp.is_busy());
    }
}

// ============================================================================
// response_t 数据访问测试
// ============================================================================

TEST_CASE("response_t get_byte", "[protocol][response]") {
    vdl::response_t resp;
    resp.set_data({0x12, 0x34, 0x56});

    REQUIRE(resp.get_byte(0) == 0x12);
    REQUIRE(resp.get_byte(1) == 0x34);
    REQUIRE(resp.get_byte(2) == 0x56);
    REQUIRE(resp.get_byte(3) == 0);  // 越界返回 0
}

TEST_CASE("response_t get_uint16_be", "[protocol][response]") {
    vdl::response_t resp;
    resp.set_data({0x12, 0x34, 0x56, 0x78});

    REQUIRE(resp.get_uint16_be(0) == 0x1234);
    REQUIRE(resp.get_uint16_be(2) == 0x5678);
    REQUIRE(resp.get_uint16_be(3) == 0);  // 越界
}

TEST_CASE("response_t get_uint16_le", "[protocol][response]") {
    vdl::response_t resp;
    resp.set_data({0x12, 0x34, 0x56, 0x78});

    REQUIRE(resp.get_uint16_le(0) == 0x3412);
    REQUIRE(resp.get_uint16_le(2) == 0x7856);
}

TEST_CASE("response_t get_uint32_be", "[protocol][response]") {
    vdl::response_t resp;
    resp.set_data({0x12, 0x34, 0x56, 0x78});

    REQUIRE(resp.get_uint32_be(0) == 0x12345678);
}

TEST_CASE("response_t get_uint32_le", "[protocol][response]") {
    vdl::response_t resp;
    resp.set_data({0x12, 0x34, 0x56, 0x78});

    REQUIRE(resp.get_uint32_le(0) == 0x78563412);
}

// ============================================================================
// response_t 清空测试
// ============================================================================

TEST_CASE("response_t clear", "[protocol][response]") {
    vdl::response_t resp;
    
    resp.set_status(vdl::response_status_t::success)
        .set_function_code(0x03)
        .set_data({0x01, 0x02})
        .set_raw_frame({0xAA, 0xBB});

    resp.clear();

    REQUIRE(resp.status() == vdl::response_status_t::invalid);
    REQUIRE(resp.function_code() == 0);
    REQUIRE_FALSE(resp.has_data());
    REQUIRE(resp.raw_frame().empty());
}

// ============================================================================
// 工厂函数测试
// ============================================================================

TEST_CASE("make_success_response", "[protocol][response]") {
    auto resp = vdl::make_success_response(0x03, {0x00, 0x64});

    REQUIRE(resp.is_success());
    REQUIRE(resp.function_code() == 0x03);
    REQUIRE(resp.data_size() == 2);
    REQUIRE(resp.data()[0] == 0x00);
    REQUIRE(resp.data()[1] == 0x64);
}

TEST_CASE("make_success_response without data", "[protocol][response]") {
    auto resp = vdl::make_success_response(0x06);

    REQUIRE(resp.is_success());
    REQUIRE(resp.function_code() == 0x06);
    REQUIRE_FALSE(resp.has_data());
}

TEST_CASE("make_error_response", "[protocol][response]") {
    auto resp = vdl::make_error_response(0x83, 0x02);

    REQUIRE(resp.is_error());
    REQUIRE(resp.function_code() == 0x83);
    REQUIRE(resp.error_code() == 0x02);
}

// ============================================================================
// response_status_t 测试
// ============================================================================

TEST_CASE("response_status_t values", "[protocol][response]") {
    REQUIRE(static_cast<uint8_t>(vdl::response_status_t::success) == 0x00);
    REQUIRE(static_cast<uint8_t>(vdl::response_status_t::error) == 0x01);
    REQUIRE(static_cast<uint8_t>(vdl::response_status_t::busy) == 0x02);
    REQUIRE(static_cast<uint8_t>(vdl::response_status_t::timeout) == 0x03);
    REQUIRE(static_cast<uint8_t>(vdl::response_status_t::invalid) == 0xFF);
}
