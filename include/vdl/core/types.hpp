#pragma once

/**
 * @file types.hpp
 * @brief VDL 基础类型定义
 * 
 * 提供 VDL 项目的核心类型，包括：
 * - Bytes: 字节序列容器
 * - Span: 非所有权的连续内存视图
 * - Duration: 时间长度表示
 * - Result: 计算结果或错误
 * - Optional: 可选值
 */

#include <vdl/core/compat.hpp>
#include <vector>
#include <cstdint>
#include <chrono>
#include <string>
#include <type_traits>

namespace vdl {

// ============================================================================
// 字节类型
// ============================================================================

/**
 * @brief 字节序列类型
 * 
 * 表示动态字节数组，用于存储二进制数据。
 * 使用 std::vector<uint8_t> 实现。
 */
using Bytes = std::vector<uint8_t>;

/**
 * @brief 字节顺序枚举
 */
enum class ByteOrder {
    Little,      ///< 小端序
    Big,         ///< 大端序
    Native       ///< 本地字节序
};

// ============================================================================
// 时间类型
// ============================================================================

/**
 * @brief 毫秒时间长度
 * 
 * 用于表示时间间隔，精度为毫秒。
 */
using Milliseconds = std::chrono::milliseconds;

/**
 * @brief 微秒时间长度
 */
using Microseconds = std::chrono::microseconds;

/**
 * @brief 纳秒时间长度
 */
using Nanoseconds = std::chrono::nanoseconds;

/**
 * @brief 秒时间长度
 */
using Seconds = std::chrono::seconds;

/**
 * @brief 时间戳类型（自 UNIX 纪元起的时间）
 * 
 * 使用 int64_t 毫秒精度。
 */
using Timestamp = int64_t;

/**
 * @brief 获取当前系统时间戳（毫秒）
 * 
 * @return Timestamp 自 UNIX 纪元起的毫秒数
 */
inline Timestamp getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<Milliseconds>(duration).count();
}

// ============================================================================
// 数字类型别名
// ============================================================================

// 无符号整数
using uint8 = std::uint8_t;       ///< 8 位无符号整数
using uint16 = std::uint16_t;      ///< 16 位无符号整数
using uint32 = std::uint32_t;      ///< 32 位无符号整数
using uint64 = std::uint64_t;      ///< 64 位无符号整数

// 有符号整数
using int8 = std::int8_t;         ///< 8 位有符号整数
using int16 = std::int16_t;        ///< 16 位有符号整数
using int32 = std::int32_t;        ///< 32 位有符号整数
using int64 = std::int64_t;        ///< 64 位有符号整数

// 浮点数
using float32 = float;              ///< 32 位浮点数
using float64 = double;             ///< 64 位浮点数

// ============================================================================
// 大小和容量类型
// ============================================================================

/**
 * @brief 大小类型（字节数）
 * 
 * 用于表示数据的大小，如缓冲区大小、内存大小等。
 */
using Size = std::size_t;

/**
 * @brief 偏移量类型
 * 
 * 用于表示在序列中的位置偏移。
 */
using Offset = std::ptrdiff_t;

/**
 * @brief 容量类型
 * 
 * 用于表示容器的容量。
 */
using Capacity = std::size_t;

// ============================================================================
// 字符串类型
// ============================================================================

/**
 * @brief 标准字符串类型
 */
using String = std::string;

/**
 * @brief 标准宽字符串类型
 */
using WString = std::wstring;

// ============================================================================
// 非所有权的内存视图
// ============================================================================

/**
 * @brief 字节范围视图（不拥有数据）
 * 
 * 提供对连续字节序列的非所有权视图。
 * 仅在 C++20 中提供 std::span，此处提供简化实现。
 */
template<typename T>
class Span {
public:
    /// @brief 指向数据的指针类型
    using pointer = T*;
    
    /// @brief 元素引用类型
    using reference = T&;
    
    /// @brief 元素类型
    using element_type = T;
    
    /// @brief 大小类型
    using size_type = std::size_t;
    
    /**
     * @brief 空 span 构造函数
     */
    constexpr Span() noexcept : data_(nullptr), size_(0) {}
    
    /**
     * @brief 从指针和大小构造 span
     * 
     * @param data 指向数据的指针
     * @param size 元素个数
     */
    constexpr Span(pointer data, size_type size) noexcept
        : data_(data), size_(size) {}
    
    /**
     * @brief 从数组构造 span
     * 
     * @tparam N 数组大小
     * @param data 数组指针
     */
    template<std::size_t N>
    constexpr Span(element_type (&data)[N]) noexcept
        : data_(data), size_(N) {}
    
    /**
     * @brief 从容器构造 span（如 std::vector）
     * 
     * @tparam Container 容器类型
     * @param container 容器对象
     */
    template<typename Container,
             typename = decltype(
                 static_cast<pointer>(const_cast<element_type*>(
                     std::addressof(*(std::declval<Container>().data()))
                 )),
                 std::declval<Container>().size()
             )>
    Span(Container& container) noexcept
        : data_(container.data()), size_(container.size()) {}
    
    /**
     * @brief const 容器构造 span
     */
    template<typename Container,
             typename = typename std::enable_if<
                 std::is_const<Container>::value
             >::type,
             typename = decltype(
                 static_cast<pointer>(std::addressof(
                     *(std::declval<Container>().data())
                 )),
                 std::declval<Container>().size()
             )>
    Span(const Container& container) noexcept
        : data_(container.data()), size_(container.size()) {}
    
    // ========================================================================
    // 访问方法
    // ========================================================================
    
    /**
     * @brief 获取指向数据的指针
     */
    constexpr pointer data() const noexcept {
        return data_;
    }
    
    /**
     * @brief 获取元素个数
     */
    constexpr size_type size() const noexcept {
        return size_;
    }
    
    /**
     * @brief 获取大小（字节数）
     */
    constexpr size_type size_bytes() const noexcept {
        return size_ * sizeof(element_type);
    }
    
    /**
     * @brief 检查 span 是否为空
     */
    constexpr bool empty() const noexcept {
        return size_ == 0;
    }
    
    /**
     * @brief 访问指定索引的元素
     */
    constexpr reference operator[](size_type index) const {
        return data_[index];
    }
    
    /**
     * @brief 获取首个元素
     */
    constexpr reference front() const {
        return data_[0];
    }
    
    /**
     * @brief 获取最后一个元素
     */
    constexpr reference back() const {
        return data_[size_ - 1];
    }
    
    // ========================================================================
    // 迭代器
    // ========================================================================
    
    /**
     * @brief 获取起始迭代器
     */
    constexpr pointer begin() const noexcept {
        return data_;
    }
    
    /**
     * @brief 获取末尾迭代器
     */
    constexpr pointer end() const noexcept {
        return data_ + size_;
    }
    
    /**
     * @brief 获取反向起始迭代器
     */
    pointer rbegin() const noexcept {
        return data_ + size_ - 1;
    }
    
    /**
     * @brief 获取反向末尾迭代器
     */
    pointer rend() const noexcept {
        return data_ - 1;
    }
    
    // ========================================================================
    // 子 span
    // ========================================================================
    
    /**
     * @brief 获取子 span
     * 
     * @param offset 起始偏移
     * @param count 元素个数（如果为 0，表示到末尾）
     * @return 子 span
     */
    constexpr Span subspan(size_type offset, size_type count = 0) const {
        if (offset > size_) return Span();
        size_type remaining = size_ - offset;
        size_type take = (count == 0) ? remaining : 
                        (count > remaining ? remaining : count);
        return Span(data_ + offset, take);
    }
    
    /**
     * @brief 获取前 count 个元素的 span
     */
    constexpr Span first(size_type count) const {
        return Span(data_, count > size_ ? size_ : count);
    }
    
    /**
     * @brief 获取后 count 个元素的 span
     */
    constexpr Span last(size_type count) const {
        return Span(data_ + (size_ > count ? size_ - count : 0), 
                   count > size_ ? size_ : count);
    }

private:
    pointer data_;        ///< 指向数据的指针
    size_type size_;      ///< 元素个数
};

/**
 * @brief 字节 span 类型别名
 * 
 * 用于创建字节序列的非所有权视图。
 */
using ByteSpan = Span<uint8>;

/**
 * @brief 常字节 span 类型别名
 */
using ConstByteSpan = Span<const uint8>;

/**
 * @brief 创建字节 span 的辅助函数
 * 
 * @param data 字节数据指针
 * @param size 字节数
 * @return ByteSpan 字节 span 对象
 */
inline ByteSpan makeByteSpan(void* data, Size size) {
    return ByteSpan(static_cast<uint8*>(data), size);
}

/**
 * @brief 创建常字节 span 的辅助函数
 * 
 * @param data 字节数据指针
 * @param size 字节数
 * @return ConstByteSpan 常字节 span 对象
 */
inline ConstByteSpan makeConstByteSpan(const void* data, Size size) {
    return ConstByteSpan(static_cast<const uint8*>(data), size);
}

// ============================================================================
// 类型特征和工具
// ============================================================================

/**
 * @brief 检查类型是否为整数类型
 */
template<typename T>
struct is_integer : std::false_type {};

template<>
struct is_integer<int8> : std::true_type {};

template<>
struct is_integer<int16> : std::true_type {};

template<>
struct is_integer<int32> : std::true_type {};

template<>
struct is_integer<int64> : std::true_type {};

template<>
struct is_integer<uint8> : std::true_type {};

template<>
struct is_integer<uint16> : std::true_type {};

template<>
struct is_integer<uint32> : std::true_type {};

template<>
struct is_integer<uint64> : std::true_type {};

/**
 * @brief 检查类型是否为浮点类型
 */
template<typename T>
struct is_floating : std::false_type {};

template<>
struct is_floating<float32> : std::true_type {};

template<>
struct is_floating<float64> : std::true_type {};

/**
 * @brief 获取数值类型的最大值
 */
template<typename T>
inline T maxValue() {
    return std::numeric_limits<T>::max();
}

/**
 * @brief 获取数值类型的最小值
 */
template<typename T>
inline T minValue() {
    return std::numeric_limits<T>::min();
}

} // namespace vdl

// 需要的头文件
#include <limits>
