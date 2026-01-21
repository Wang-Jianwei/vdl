/**
 * @file test_buffer.cpp
 * @brief 测试缓冲区类
 */

#include <catch.hpp>
#include <vdl/core/buffer.hpp>

// ============================================================================
// ring_buffer_t 基础测试
// ============================================================================

TEST_CASE("ring_buffer_t construction", "[core][buffer]") {
    vdl::ring_buffer_t buffer(1024);

    REQUIRE(buffer.capacity() == 1024);
    REQUIRE(buffer.size() == 0);
    REQUIRE(buffer.available() == 1024);
    REQUIRE(buffer.empty());
    REQUIRE_FALSE(buffer.full());
}

TEST_CASE("ring_buffer_t write and read", "[core][buffer]") {
    vdl::ring_buffer_t buffer(64);

    vdl::byte_t data[] = {0xAA, 0xBB, 0xCC};
    size_t written = buffer.write(data, 3);

    REQUIRE(written == 3);
    REQUIRE(buffer.size() == 3);
    REQUIRE(buffer.available() == 61);

    vdl::byte_t out[3];
    size_t read = buffer.read(out, 3);

    REQUIRE(read == 3);
    REQUIRE(out[0] == 0xAA);
    REQUIRE(out[1] == 0xBB);
    REQUIRE(out[2] == 0xCC);
    REQUIRE(buffer.empty());
}

TEST_CASE("ring_buffer_t write with const_byte_span_t", "[core][buffer]") {
    vdl::ring_buffer_t buffer(64);

    vdl::byte_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    vdl::const_byte_span_t span(data, 5);
    
    size_t written = buffer.write(span);

    REQUIRE(written == 5);
    REQUIRE(buffer.size() == 5);
}

TEST_CASE("ring_buffer_t read with byte_span_t", "[core][buffer]") {
    vdl::ring_buffer_t buffer(64);

    vdl::byte_t data[] = {0x01, 0x02, 0x03};
    buffer.write(data, 3);

    vdl::byte_t out[3];
    vdl::byte_span_t out_span(out, 3);
    size_t read = buffer.read(out_span);

    REQUIRE(read == 3);
    REQUIRE(out[0] == 0x01);
    REQUIRE(out[1] == 0x02);
    REQUIRE(out[2] == 0x03);
}

TEST_CASE("ring_buffer_t peek", "[core][buffer]") {
    vdl::ring_buffer_t buffer(64);

    vdl::byte_t data[] = {0x01, 0x02, 0x03};
    buffer.write(data, 3);

    vdl::byte_t out[2];
    size_t peeked = buffer.peek(out, 2);

    REQUIRE(peeked == 2);
    REQUIRE(out[0] == 0x01);
    REQUIRE(out[1] == 0x02);
    // peek 不移除数据
    REQUIRE(buffer.size() == 3);
}

TEST_CASE("ring_buffer_t skip", "[core][buffer]") {
    vdl::ring_buffer_t buffer(64);

    vdl::byte_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    buffer.write(data, 5);

    size_t skipped = buffer.skip(2);
    REQUIRE(skipped == 2);
    REQUIRE(buffer.size() == 3);

    vdl::byte_t out[3];
    buffer.read(out, 3);
    REQUIRE(out[0] == 0x03);
    REQUIRE(out[1] == 0x04);
    REQUIRE(out[2] == 0x05);
}

TEST_CASE("ring_buffer_t clear", "[core][buffer]") {
    vdl::ring_buffer_t buffer(64);

    vdl::byte_t data[] = {0x01, 0x02, 0x03};
    buffer.write(data, 3);
    REQUIRE(buffer.size() == 3);

    buffer.clear();
    REQUIRE(buffer.size() == 0);
    REQUIRE(buffer.empty());
    REQUIRE(buffer.available() == 64);
}

TEST_CASE("ring_buffer_t wraparound", "[core][buffer]") {
    vdl::ring_buffer_t buffer(8);

    // 写满
    vdl::byte_t data1[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    size_t written = buffer.write(data1, 8);
    REQUIRE(written == 8);
    REQUIRE(buffer.full());

    // 读取部分
    vdl::byte_t out[4];
    buffer.read(out, 4);
    REQUIRE(buffer.size() == 4);

    // 再写入（会跨越边界）
    vdl::byte_t data2[] = {0xAA, 0xBB, 0xCC, 0xDD};
    written = buffer.write(data2, 4);
    REQUIRE(written == 4);
    REQUIRE(buffer.full());

    // 读取全部验证
    vdl::byte_t result[8];
    size_t read = buffer.read(result, 8);
    REQUIRE(read == 8);
    REQUIRE(result[0] == 0x05);
    REQUIRE(result[1] == 0x06);
    REQUIRE(result[2] == 0x07);
    REQUIRE(result[3] == 0x08);
    REQUIRE(result[4] == 0xAA);
    REQUIRE(result[5] == 0xBB);
    REQUIRE(result[6] == 0xCC);
    REQUIRE(result[7] == 0xDD);
}

TEST_CASE("ring_buffer_t partial write when full", "[core][buffer]") {
    vdl::ring_buffer_t buffer(4);

    vdl::byte_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    size_t written = buffer.write(data, 6);

    // 只能写入 4 字节
    REQUIRE(written == 4);
    REQUIRE(buffer.full());
}

TEST_CASE("ring_buffer_t null pointer handling", "[core][buffer]") {
    vdl::ring_buffer_t buffer(64);

    REQUIRE(buffer.write(nullptr, 10) == 0);
    REQUIRE(buffer.read(nullptr, 10) == 0);
    REQUIRE(buffer.peek(nullptr, 10) == 0);
}
