/**
 * @file types.hpp
 * @brief VDL 基础类型定义
 * 
 * 提供 VDL 项目的核心类型定义。
 */

#ifndef VDL_CORE_TYPES_HPP
#define VDL_CORE_TYPES_HPP

#include <vdl/core/compat.hpp>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <string>

namespace vdl {

// ============================================================================
// 基础类型别名
// ============================================================================

typedef std::uint8_t   uint8_t;
typedef std::uint16_t  uint16_t;
typedef std::uint32_t  uint32_t;
typedef std::uint64_t  uint64_t;
typedef std::int8_t    int8_t;
typedef std::int16_t   int16_t;
typedef std::int32_t   int32_t;
typedef std::int64_t   int64_t;

// ============================================================================
// 字节类型
// ============================================================================

/**
 * @brief 字节类型
 */
typedef std::uint8_t byte_t;

/**
 * @brief 字节序列类型
 */
typedef std::vector<byte_t> bytes_t;

/**
 * @brief 大小类型
 */
typedef std::size_t size_t;

/**
 * @brief 偏移量类型
 */
typedef std::ptrdiff_t offset_t;

// ============================================================================
// 时间类型（使用简单整数避免 chrono 复杂性）
// ============================================================================

/**
 * @brief 毫秒类型（简单整数）
 */
typedef std::int64_t milliseconds_t;

/**
 * @brief 微秒类型
 */
typedef std::int64_t microseconds_t;

/**
 * @brief 时间戳类型（毫秒）
 */
typedef std::int64_t timestamp_t;

/**
 * @brief 获取当前时间戳（毫秒）
 */
inline timestamp_t get_current_timestamp() {
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::system_clock::duration duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// ============================================================================
// 字节序
// ============================================================================

enum class byte_order_t {
    little,     ///< 小端序
    big,        ///< 大端序
    native      ///< 本地字节序
};

// ============================================================================
// Span 类型（轻量级非所有权视图）
// ============================================================================

/**
 * @brief 连续内存视图（非所有权）
 * 
 * 类似于 C++20 std::span 的轻量级实现。
 */
template<typename T>
class span_t {
public:
    typedef T              element_type;
    typedef T*             pointer;
    typedef const T*       const_pointer;
    typedef T&             reference;
    typedef const T&       const_reference;
    typedef pointer        iterator;
    typedef const_pointer  const_iterator;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    /**
     * @brief 默认构造空 span
     */
    span_t() : m_data(nullptr), m_size(0) {}

    /**
     * @brief 从指针和大小构造
     */
    span_t(pointer data, size_type size) : m_data(data), m_size(size) {}

    /**
     * @brief 从数组构造
     */
    template<std::size_t N>
    span_t(element_type (&arr)[N]) : m_data(arr), m_size(N) {}

    /**
     * @brief 从 vector 构造
     */
    template<typename Alloc>
    span_t(std::vector<T, Alloc>& vec)
        : m_data(vec.empty() ? nullptr : vec.data())
        , m_size(vec.size()) {}

    // 访问
    pointer data() const { return m_data; }
    size_type size() const { return m_size; }
    bool empty() const { return m_size == 0; }

    reference operator[](size_type idx) const { return m_data[idx]; }
    reference front() const { return m_data[0]; }
    reference back() const { return m_data[m_size - 1]; }

    // 迭代器
    iterator begin() const { return m_data; }
    iterator end() const { return m_data + m_size; }
    const_iterator cbegin() const { return m_data; }
    const_iterator cend() const { return m_data + m_size; }

    // 子视图
    span_t subspan(size_type offset, size_type count = static_cast<size_type>(-1)) const {
        if (offset >= m_size) {
            return span_t();
        }
        size_type actual_count = (count == static_cast<size_type>(-1) || offset + count > m_size)
                                     ? (m_size - offset)
                                     : count;
        return span_t(m_data + offset, actual_count);
    }

    span_t first(size_type count) const {
        return span_t(m_data, count > m_size ? m_size : count);
    }

    span_t last(size_type count) const {
        if (count >= m_size) {
            return *this;
        }
        return span_t(m_data + m_size - count, count);
    }

private:
    pointer m_data;
    size_type m_size;
};

// 常用 span 类型别名
typedef span_t<byte_t>       byte_span_t;
typedef span_t<const byte_t> const_byte_span_t;

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * @brief 从指针和大小创建 span
 */
template<typename T>
inline span_t<T> make_span(T* data, std::size_t size) {
    return span_t<T>(data, size);
}

/**
 * @brief 从 vector 创建 span
 */
template<typename T, typename Alloc>
inline span_t<T> make_span(std::vector<T, Alloc>& vec) {
    return span_t<T>(vec);
}

template<typename T, typename Alloc>
inline span_t<const T> make_span(const std::vector<T, Alloc>& vec) {
    return span_t<const T>(vec.data(), vec.size());
}

/**
 * @brief 从数组创建 span
 */
template<typename T, std::size_t N>
inline span_t<T> make_span(T (&arr)[N]) {
    return span_t<T>(arr, N);
}

}  // namespace vdl

#endif  // VDL_CORE_TYPES_HPP
