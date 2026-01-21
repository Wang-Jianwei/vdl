/**
 * @file test_command.cpp
 * @brief 测试 command_t
 */

#include <catch.hpp>
#include <vdl/protocol/command.hpp>

// ============================================================================
// command_t 基本测试
// ============================================================================

TEST_CASE("command_t default construction", "[protocol][command]") {
    vdl::command_t cmd;

    REQUIRE(cmd.type() == vdl::command_type_t::read);
    REQUIRE(cmd.function_code() == 0);
    REQUIRE(cmd.address() == 0);
    REQUIRE(cmd.count() == 0);
    REQUIRE_FALSE(cmd.has_data());
    REQUIRE(cmd.tag().empty());
}

TEST_CASE("command_t builder pattern", "[protocol][command]") {
    vdl::command_t cmd;
    
    cmd.set_type(vdl::command_type_t::write)
       .set_function_code(0x06)
       .set_address(0x0100)
       .set_count(1)
       .set_data({0x12, 0x34})
       .set_tag("test_cmd");

    REQUIRE(cmd.type() == vdl::command_type_t::write);
    REQUIRE(cmd.function_code() == 0x06);
    REQUIRE(cmd.address() == 0x0100);
    REQUIRE(cmd.count() == 1);
    REQUIRE(cmd.has_data());
    REQUIRE(cmd.data().size() == 2);
    REQUIRE(cmd.data()[0] == 0x12);
    REQUIRE(cmd.data()[1] == 0x34);
    REQUIRE(cmd.tag() == "test_cmd");
}

// ============================================================================
// command_t 数据设置测试
// ============================================================================

TEST_CASE("command_t set_data with vector", "[protocol][command]") {
    vdl::command_t cmd;
    
    vdl::bytes_t data = {0x01, 0x02, 0x03};
    cmd.set_data(data);

    REQUIRE(cmd.data().size() == 3);
    REQUIRE(cmd.data()[0] == 0x01);
}

TEST_CASE("command_t set_data with move", "[protocol][command]") {
    vdl::command_t cmd;
    
    vdl::bytes_t data = {0x04, 0x05, 0x06};
    cmd.set_data(std::move(data));

    REQUIRE(cmd.data().size() == 3);
    REQUIRE(cmd.data()[2] == 0x06);
}

TEST_CASE("command_t set_data with initializer_list", "[protocol][command]") {
    vdl::command_t cmd;
    
    cmd.set_data({0x07, 0x08, 0x09, 0x0A});

    REQUIRE(cmd.data().size() == 4);
    REQUIRE(cmd.data()[3] == 0x0A);
}

// ============================================================================
// command_t 清空测试
// ============================================================================

TEST_CASE("command_t clear", "[protocol][command]") {
    vdl::command_t cmd;
    
    cmd.set_type(vdl::command_type_t::execute)
       .set_function_code(0xFF)
       .set_address(0x1234)
       .set_data({0x01, 0x02})
       .set_tag("test");

    cmd.clear();

    REQUIRE(cmd.type() == vdl::command_type_t::read);
    REQUIRE(cmd.function_code() == 0);
    REQUIRE(cmd.address() == 0);
    REQUIRE_FALSE(cmd.has_data());
    REQUIRE(cmd.tag().empty());
}

// ============================================================================
// 工厂函数测试
// ============================================================================

TEST_CASE("make_read_command", "[protocol][command]") {
    auto cmd = vdl::make_read_command(0x03, 0x0000, 10);

    REQUIRE(cmd.type() == vdl::command_type_t::read);
    REQUIRE(cmd.function_code() == 0x03);
    REQUIRE(cmd.address() == 0x0000);
    REQUIRE(cmd.count() == 10);
}

TEST_CASE("make_write_command", "[protocol][command]") {
    vdl::bytes_t data = {0x00, 0x64};
    auto cmd = vdl::make_write_command(0x06, 0x0100, data);

    REQUIRE(cmd.type() == vdl::command_type_t::write);
    REQUIRE(cmd.function_code() == 0x06);
    REQUIRE(cmd.address() == 0x0100);
    REQUIRE(cmd.data().size() == 2);
}

TEST_CASE("make_execute_command", "[protocol][command]") {
    auto cmd = vdl::make_execute_command(0x08);

    REQUIRE(cmd.type() == vdl::command_type_t::execute);
    REQUIRE(cmd.function_code() == 0x08);
}

// ============================================================================
// command_type_t 测试
// ============================================================================

TEST_CASE("command_type_t values", "[protocol][command]") {
    REQUIRE(static_cast<uint8_t>(vdl::command_type_t::read) == 0x01);
    REQUIRE(static_cast<uint8_t>(vdl::command_type_t::write) == 0x02);
    REQUIRE(static_cast<uint8_t>(vdl::command_type_t::execute) == 0x03);
    REQUIRE(static_cast<uint8_t>(vdl::command_type_t::query) == 0x04);
    REQUIRE(static_cast<uint8_t>(vdl::command_type_t::custom) == 0xFF);
}
