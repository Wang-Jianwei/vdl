/**
 * @file test_types.cpp
 * @brief 测试核心类型定义
 */

#include <catch.hpp>
#include <vdl/core/types.hpp>

#include <vector>

// ============================================================================
// 基本类型测试
// ============================================================================

TEST_CASE("byte_t is uint8_t", "[core][types]") {
    vdl::byte_t b = 0xFF;
    REQUIRE(b == 255);
    REQUIRE(sizeof(b) == 1);
}

TEST_CASE("bytes_t is vector<byte_t>", "[core][types]") {
    vdl::bytes_t bytes;
    REQUIRE(bytes.empty());

    bytes = {0x01, 0x02, 0x03};
    REQUIRE(bytes.size() == 3);
    REQUIRE(bytes[0] == 0x01);
    REQUIRE(bytes[2] == 0x03);
}

TEST_CASE("size_t and offset_t", "[core][types]") {
    vdl::size_t size = 1024;
    vdl::offset_t offset = 100;

    REQUIRE(size == 1024);
    REQUIRE(offset == 100);
}

TEST_CASE("milliseconds_t", "[core][types]") {
    vdl::milliseconds_t timeout = 5000;
    REQUIRE(timeout == 5000);
}

// ============================================================================
// span_t 测试
// ============================================================================

TEST_CASE("span_t from vector", "[core][types][span]") {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    vdl::span_t<int> span(vec.data(), vec.size());

    REQUIRE(span.size() == 5);
    REQUIRE(span[0] == 1);
    REQUIRE(span[4] == 5);
}

TEST_CASE("span_t from array", "[core][types][span]") {
    int arr[] = {10, 20, 30};
    vdl::span_t<int> span(arr, 3);

    REQUIRE(span.size() == 3);
    REQUIRE(span.data() == arr);
}

TEST_CASE("span_t empty check", "[core][types][span]") {
    vdl::span_t<int> empty_span(nullptr, 0);
    REQUIRE(empty_span.empty());
    REQUIRE(empty_span.size() == 0);
}

TEST_CASE("span_t iteration", "[core][types][span]") {
    std::vector<int> vec = {1, 2, 3};
    vdl::span_t<int> span(vec.data(), vec.size());

    int sum = 0;
    for (auto it = span.begin(); it != span.end(); ++it) {
        sum += *it;
    }
    REQUIRE(sum == 6);
}

TEST_CASE("span_t subspan", "[core][types][span]") {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    vdl::span_t<int> span(vec.data(), vec.size());

    auto sub = span.subspan(2);
    REQUIRE(sub.size() == 3);
    REQUIRE(sub[0] == 3);

    auto sub2 = span.subspan(1, 2);
    REQUIRE(sub2.size() == 2);
    REQUIRE(sub2[0] == 2);
    REQUIRE(sub2[1] == 3);
}

TEST_CASE("span_t first and last", "[core][types][span]") {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    vdl::span_t<int> span(vec.data(), vec.size());

    auto first3 = span.first(3);
    REQUIRE(first3.size() == 3);
    REQUIRE(first3[2] == 3);

    auto last2 = span.last(2);
    REQUIRE(last2.size() == 2);
    REQUIRE(last2[0] == 4);
    REQUIRE(last2[1] == 5);
}

// ============================================================================
// byte_span_t 测试
// ============================================================================

TEST_CASE("byte_span_t from bytes_t", "[core][types][span]") {
    vdl::bytes_t bytes = {0xAA, 0xBB, 0xCC};
    vdl::byte_span_t span(bytes.data(), bytes.size());

    REQUIRE(span.size() == 3);
    REQUIRE(span[0] == 0xAA);
}

TEST_CASE("const_byte_span_t from const bytes_t", "[core][types][span]") {
    const vdl::bytes_t bytes = {0x11, 0x22, 0x33};
    vdl::const_byte_span_t span(bytes.data(), bytes.size());

    REQUIRE(span.size() == 3);
    REQUIRE(span[2] == 0x33);
}

// ============================================================================
// make_span 辅助函数测试
// ============================================================================

TEST_CASE("make_span from pointer and size", "[core][types][span]") {
    int arr[] = {1, 2, 3};
    auto span = vdl::make_span(arr, 3);

    REQUIRE(span.size() == 3);
    REQUIRE(span[0] == 1);
}

TEST_CASE("make_span from vector", "[core][types][span]") {
    std::vector<int> vec = {4, 5, 6};
    auto span = vdl::make_span(vec);

    REQUIRE(span.size() == 3);
    REQUIRE(span[2] == 6);
}

TEST_CASE("make_span from const vector", "[core][types][span]") {
    const std::vector<double> vec = {1.1, 2.2, 3.3};
    auto span = vdl::make_span(vec);

    REQUIRE(span.size() == 3);
}

// ============================================================================
// span_t 修改测试
// ============================================================================

TEST_CASE("span_t allows modification", "[core][types][span]") {
    std::vector<int> vec = {1, 2, 3};
    vdl::span_t<int> span(vec.data(), vec.size());

    span[0] = 100;
    span[2] = 300;

    REQUIRE(vec[0] == 100);
    REQUIRE(vec[2] == 300);
}

TEST_CASE("const span_t is read-only", "[core][types][span]") {
    std::vector<int> vec = {1, 2, 3};
    vdl::span_t<const int> span(vec.data(), vec.size());

    // span[0] = 100;  // 编译错误
    REQUIRE(span[0] == 1);
}
