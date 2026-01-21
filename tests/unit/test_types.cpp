#include <catch.hpp>
#include <vdl/core/types.hpp>
#include <vector>
#include <cstring>

// 测试用辅助类和函数
namespace {
    template<typename T>
    struct TestStruct {
        T value;
        TestStruct(T v = T()) : value(v) {}
    };
}

// ============================================================================
// Bytes 类型测试
// ============================================================================

TEST_CASE("Bytes type is vector<uint8>", "[types][bytes]") {
    SECTION("Create empty bytes") {
        vdl::Bytes bytes;
        REQUIRE(bytes.empty());
        REQUIRE(bytes.size() == 0);
    }
    
    SECTION("Create bytes with data") {
        vdl::Bytes bytes = {0x01, 0x02, 0x03, 0x04};
        REQUIRE(bytes.size() == 4);
        REQUIRE(bytes[0] == 0x01);
        REQUIRE(bytes[3] == 0x04);
    }
    
    SECTION("Bytes supports standard vector operations") {
        vdl::Bytes bytes;
        bytes.push_back(0xFF);
        bytes.push_back(0xAA);
        REQUIRE(bytes.size() == 2);
        REQUIRE(bytes.back() == 0xAA);
    }
}

// ============================================================================
// 数字类型别名测试
// ============================================================================

TEST_CASE("Integer type aliases", "[types][integers]") {
    SECTION("8-bit integers") {
        vdl::uint8 u8_val = 255;
        vdl::int8 i8_val = -128;
        REQUIRE(u8_val == 255);
        REQUIRE(i8_val == -128);
    }
    
    SECTION("16-bit integers") {
        vdl::uint16 u16_val = 65535;
        vdl::int16 i16_val = -32768;
        REQUIRE(u16_val == 65535);
        REQUIRE(i16_val == -32768);
    }
    
    SECTION("32-bit integers") {
        vdl::uint32 u32_val = 4294967295U;
        vdl::int32 i32_val = -2147483648;
        REQUIRE(u32_val == 4294967295U);
        REQUIRE(i32_val == -2147483648);
    }
    
    SECTION("64-bit integers") {
        vdl::uint64 u64_val = 18446744073709551615ULL;
        vdl::int64 i64_val = -9223372036854775807LL;
        REQUIRE(u64_val == 18446744073709551615ULL);
        REQUIRE(i64_val == -9223372036854775807LL);
    }
}

TEST_CASE("Floating-point type aliases", "[types][floats]") {
    SECTION("32-bit float") {
        vdl::float32 f32_val = 3.14f;
        REQUIRE(f32_val > 3.0f);
        REQUIRE(f32_val < 4.0f);
    }
    
    SECTION("64-bit float") {
        vdl::float64 f64_val = 3.14159265;
        REQUIRE(f64_val > 3.14);
        REQUIRE(f64_val < 3.15);
    }
}

// ============================================================================
// 时间类型测试
// ============================================================================

TEST_CASE("Milliseconds type alias", "[types][time]") {
    SECTION("Create milliseconds duration") {
        vdl::Milliseconds ms(1000);
        REQUIRE(ms.count() == 1000);
    }
    
    SECTION("Milliseconds arithmetic") {
        vdl::Milliseconds ms1(1000);
        vdl::Milliseconds ms2(500);
        vdl::Milliseconds ms_sum = ms1 + ms2;
        REQUIRE(ms_sum.count() == 1500);
    }
}

TEST_CASE("Timestamp and time utilities", "[types][time][timestamp]") {
    SECTION("Get current timestamp") {
        vdl::Timestamp ts1 = vdl::getCurrentTimestamp();
        vdl::Timestamp ts2 = vdl::getCurrentTimestamp();
        // 时间戳应该是正数
        REQUIRE(ts1 > 0);
        REQUIRE(ts2 > 0);
        // ts2 应该 >= ts1
        REQUIRE(ts2 >= ts1);
    }
    
    SECTION("Timestamp is milliseconds precision") {
        vdl::Timestamp ts = vdl::getCurrentTimestamp();
        // 时间戳应该是一个很大的数字（自 UNIX 纪元起）
        REQUIRE(ts > 1704067200000LL);  // 2024-01-01 之后
    }
}

// ============================================================================
// Size/Offset/Capacity 类型测试
// ============================================================================

TEST_CASE("Size, Offset, Capacity type aliases", "[types][size]") {
    SECTION("Size type") {
        vdl::Size size = 256;
        REQUIRE(size == 256);
        REQUIRE(std::is_same<vdl::Size, std::size_t>::value);
    }
    
    SECTION("Offset type") {
        vdl::Offset offset = -10;
        REQUIRE(offset == -10);
        REQUIRE(std::is_same<vdl::Offset, std::ptrdiff_t>::value);
    }
    
    SECTION("Capacity type") {
        vdl::Capacity cap = 1024;
        REQUIRE(cap == 1024);
        REQUIRE(std::is_same<vdl::Capacity, std::size_t>::value);
    }
}

// ============================================================================
// String 类型别名测试
// ============================================================================

TEST_CASE("String type aliases", "[types][string]") {
    SECTION("String type") {
        vdl::String str = "Hello, VDL!";
        REQUIRE(str == "Hello, VDL!");
        REQUIRE(str.size() == 11);
    }
    
    SECTION("WString type") {
        vdl::WString wstr = L"Hello, VDL!";
        REQUIRE(wstr == L"Hello, VDL!");
        REQUIRE(wstr.size() == 11);
    }
}

// ============================================================================
// Span 类型测试
// ============================================================================

TEST_CASE("Span construction and basic operations", "[types][span]") {
    SECTION("Empty span") {
        vdl::Span<int> span;
        REQUIRE(span.empty());
        REQUIRE(span.size() == 0);
        REQUIRE(span.data() == nullptr);
    }
    
    SECTION("Span from pointer and size") {
        int data[] = {1, 2, 3, 4, 5};
        vdl::Span<int> span(data, 5);
        REQUIRE(!span.empty());
        REQUIRE(span.size() == 5);
        REQUIRE(span[0] == 1);
        REQUIRE(span[4] == 5);
    }
    
    SECTION("Span from C-style array") {
        int data[] = {10, 20, 30};
        vdl::Span<int> span(data);
        REQUIRE(span.size() == 3);
        REQUIRE(span[1] == 20);
    }
    
    SECTION("Span from vector") {
        std::vector<int> vec = {100, 200, 300, 400};
        vdl::Span<int> span(vec);
        REQUIRE(span.size() == 4);
        REQUIRE(span.data() == vec.data());
    }
}

TEST_CASE("Span access methods", "[types][span][access]") {
    SECTION("Front and back access") {
        int data[] = {5, 10, 15, 20};
        vdl::Span<int> span(data);
        REQUIRE(span.front() == 5);
        REQUIRE(span.back() == 20);
    }
    
    SECTION("size_bytes calculation") {
        int data[] = {1, 2, 3};
        vdl::Span<int> span(data);
        REQUIRE(span.size_bytes() == 3 * sizeof(int));
    }
}

TEST_CASE("Span iteration", "[types][span][iteration]") {
    SECTION("Forward iteration") {
        int data[] = {1, 2, 3, 4};
        vdl::Span<int> span(data);
        
        int sum = 0;
        for (int val : span) {
            sum += val;
        }
        REQUIRE(sum == 10);
    }
    
    SECTION("Iterator access") {
        int data[] = {10, 20, 30};
        vdl::Span<int> span(data);
        REQUIRE(*span.begin() == 10);
        REQUIRE(*(span.end() - 1) == 30);
    }
}

TEST_CASE("Span subspan operations", "[types][span][subspan]") {
    SECTION("subspan with offset and count") {
        int data[] = {1, 2, 3, 4, 5, 6, 7};
        vdl::Span<int> span(data);
        
        vdl::Span<int> sub = span.subspan(2, 3);
        REQUIRE(sub.size() == 3);
        REQUIRE(sub[0] == 3);
        REQUIRE(sub[2] == 5);
    }
    
    SECTION("first() method") {
        int data[] = {1, 2, 3, 4, 5};
        vdl::Span<int> span(data);
        
        vdl::Span<int> first3 = span.first(3);
        REQUIRE(first3.size() == 3);
        REQUIRE(first3[0] == 1);
        REQUIRE(first3[2] == 3);
    }
    
    SECTION("last() method") {
        int data[] = {1, 2, 3, 4, 5};
        vdl::Span<int> span(data);
        
        vdl::Span<int> last2 = span.last(2);
        REQUIRE(last2.size() == 2);
        REQUIRE(last2[0] == 4);
        REQUIRE(last2[1] == 5);
    }
}

TEST_CASE("ByteSpan and helper functions", "[types][span][bytes]") {
    SECTION("ByteSpan from Bytes") {
        vdl::Bytes bytes = {0xAA, 0xBB, 0xCC, 0xDD};
        vdl::ByteSpan span(bytes);
        REQUIRE(span.size() == 4);
        REQUIRE(span[0] == 0xAA);
    }
    
    SECTION("makeByteSpan helper") {
        uint8_t data[] = {1, 2, 3, 4};
        vdl::ByteSpan span = vdl::makeByteSpan(data, 4);
        REQUIRE(span.size() == 4);
        REQUIRE(span[1] == 2);
    }
    
    SECTION("makeConstByteSpan helper") {
        const uint8_t data[] = {10, 20, 30};
        vdl::ConstByteSpan span = vdl::makeConstByteSpan(data, 3);
        REQUIRE(span.size() == 3);
        REQUIRE(span[2] == 30);
    }
}

// ============================================================================
// 类型特征测试
// ============================================================================

TEST_CASE("Type traits", "[types][traits]") {
    SECTION("is_integer trait") {
        REQUIRE(vdl::is_integer<vdl::int32>::value);
        REQUIRE(vdl::is_integer<vdl::uint64>::value);
        REQUIRE(!vdl::is_integer<vdl::float32>::value);
        // Note: int and double may be aliases for specialized types on some platforms
    }
    
    SECTION("is_floating trait") {
        REQUIRE(vdl::is_floating<vdl::float32>::value);
        REQUIRE(vdl::is_floating<vdl::float64>::value);
        REQUIRE(!vdl::is_floating<vdl::int32>::value);
        // Note: double may be an alias for float64 on some platforms
    }
}

TEST_CASE("Numeric limits utilities", "[types][limits]") {
    SECTION("maxValue and minValue for int32") {
        vdl::int32 max_val = vdl::maxValue<vdl::int32>();
        vdl::int32 min_val = vdl::minValue<vdl::int32>();
        REQUIRE(max_val == 2147483647);
        REQUIRE(min_val == -2147483648);
    }
    
    SECTION("maxValue for uint8") {
        vdl::uint8 max_val = vdl::maxValue<vdl::uint8>();
        REQUIRE(max_val == 255);
    }
}

// ============================================================================
// ByteOrder 枚举测试
// ============================================================================

TEST_CASE("ByteOrder enumeration", "[types][byteorder]") {
    SECTION("ByteOrder values") {
        REQUIRE(static_cast<int>(vdl::ByteOrder::Little) == 0);
        REQUIRE(static_cast<int>(vdl::ByteOrder::Big) == 1);
        REQUIRE(static_cast<int>(vdl::ByteOrder::Native) == 2);
    }
}
