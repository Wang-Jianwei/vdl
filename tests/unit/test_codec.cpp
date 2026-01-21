/**
 * @file test_codec.cpp
 * @brief 测试编解码器
 */

#include <catch.hpp>
#include <vdl/codec/codec.hpp>
#include <vdl/codec/binary_codec.hpp>

// ============================================================================
// binary_codec_t 编码测试
// ============================================================================

TEST_CASE("binary_codec_t encode basic command", "[codec][binary]") {
    vdl::binary_codec_t codec;
    
    vdl::command_t cmd;
    cmd.set_function_code(0x03);
    cmd.set_data({0x00, 0x01, 0x00, 0x0A});
    
    auto result = codec.encode(cmd);
    
    REQUIRE(result.has_value());
    
    auto& frame = *result;
    // 帧结构: SOF(1) + LEN(2) + FUNC(1) + DATA(4) + CRC(2) = 10 字节
    REQUIRE(frame.size() == 10);
    REQUIRE(frame[0] == 0xAA);  // SOF
    REQUIRE(frame[1] == 0x04);  // LEN low byte
    REQUIRE(frame[2] == 0x00);  // LEN high byte
    REQUIRE(frame[3] == 0x03);  // FUNC
    REQUIRE(frame[4] == 0x00);  // DATA[0]
    REQUIRE(frame[5] == 0x01);  // DATA[1]
    REQUIRE(frame[6] == 0x00);  // DATA[2]
    REQUIRE(frame[7] == 0x0A);  // DATA[3]
}

TEST_CASE("binary_codec_t encode empty data", "[codec][binary]") {
    vdl::binary_codec_t codec;
    
    vdl::command_t cmd;
    cmd.set_function_code(0x01);
    
    auto result = codec.encode(cmd);
    
    REQUIRE(result.has_value());
    
    auto& frame = *result;
    // 帧结构: SOF(1) + LEN(2) + FUNC(1) + CRC(2) = 6 字节
    REQUIRE(frame.size() == 6);
    REQUIRE(frame[0] == 0xAA);
    REQUIRE(frame[1] == 0x00);  // LEN = 0
    REQUIRE(frame[2] == 0x00);
    REQUIRE(frame[3] == 0x01);  // FUNC
}

// ============================================================================
// binary_codec_t 解码测试
// ============================================================================

TEST_CASE("binary_codec_t decode valid frame", "[codec][binary]") {
    vdl::binary_codec_t codec;
    
    // 先编码一个命令
    vdl::command_t cmd;
    cmd.set_function_code(0x03);
    cmd.set_data({0x01, 0x02, 0x03});
    
    auto encode_result = codec.encode(cmd);
    REQUIRE(encode_result.has_value());
    
    // 解码
    vdl::size_t consumed = 0;
    vdl::const_byte_span_t frame_span(encode_result->data(), 
                                       encode_result->size());
    auto decode_result = codec.decode(frame_span, consumed);
    
    REQUIRE(decode_result.has_value());
    REQUIRE(consumed == encode_result->size());
    
    auto& response = *decode_result;
    REQUIRE(response.function_code() == 0x03);
    REQUIRE(response.data().size() == 3);
    REQUIRE(response.data()[0] == 0x01);
    REQUIRE(response.data()[1] == 0x02);
    REQUIRE(response.data()[2] == 0x03);
}

TEST_CASE("binary_codec_t decode incomplete frame", "[codec][binary]") {
    vdl::binary_codec_t codec;
    
    // 不完整的帧（只有头部）
    vdl::byte_t partial[] = {0xAA, 0x05, 0x00, 0x01};
    vdl::const_byte_span_t span(partial, 4);
    
    vdl::size_t consumed = 0;
    auto result = codec.decode(span, consumed);
    
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().code() == vdl::error_code_t::incomplete_frame);
}

TEST_CASE("binary_codec_t decode with garbage before SOF", "[codec][binary]") {
    vdl::binary_codec_t codec;
    
    // 垃圾数据后面跟着有效帧
    vdl::byte_t data[] = {0x00, 0x01, 0x02};  // 没有 SOF
    vdl::const_byte_span_t span(data, 3);
    
    vdl::size_t consumed = 0;
    auto result = codec.decode(span, consumed);
    
    // 应该跳过这些数据
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("binary_codec_t decode checksum error", "[codec][binary]") {
    vdl::binary_codec_t codec;
    
    // 先编码一个命令
    vdl::command_t cmd;
    cmd.set_function_code(0x01);
    
    auto encode_result = codec.encode(cmd);
    REQUIRE(encode_result.has_value());
    
    // 破坏 CRC
    auto& frame = *encode_result;
    frame[frame.size() - 1] ^= 0xFF;
    
    vdl::const_byte_span_t frame_span(frame.data(), frame.size());
    vdl::size_t consumed = 0;
    auto decode_result = codec.decode(frame_span, consumed);
    
    REQUIRE_FALSE(decode_result.has_value());
    REQUIRE(decode_result.error().code() == vdl::error_code_t::checksum_error);
}

// ============================================================================
// frame_length 测试
// ============================================================================

TEST_CASE("binary_codec_t frame_length", "[codec][binary]") {
    vdl::binary_codec_t codec;
    
    // 数据长度为 5 的帧
    vdl::byte_t header[] = {0xAA, 0x05, 0x00, 0x01};
    vdl::const_byte_span_t span(header, 4);
    
    // 帧长度 = HEADER(4) + DATA(5) + CRC(2) = 11
    REQUIRE(codec.frame_length(span) == 11);
}

TEST_CASE("binary_codec_t frame_length with insufficient data", "[codec][binary]") {
    vdl::binary_codec_t codec;
    
    // 不足以确定长度
    vdl::byte_t short_data[] = {0xAA, 0x05};
    vdl::const_byte_span_t span(short_data, 2);
    
    REQUIRE(codec.frame_length(span) == 0);
}

// ============================================================================
// max_frame_size 测试
// ============================================================================

TEST_CASE("binary_codec_t max_frame_size", "[codec][binary]") {
    vdl::binary_codec_t codec;
    
    // 默认最大帧大小
    size_t default_max = codec.max_frame_size();
    REQUIRE(default_max > 0);
    
    // 设置新的最大帧大小
    codec.set_max_frame_size(256);
    REQUIRE(codec.max_frame_size() == 256);
}
