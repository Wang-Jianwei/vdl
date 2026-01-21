/**
 * @file buffer.hpp
 * @brief VDL 缓冲区管理
 * 
 * 提供缓冲区类和相关工具函数。
 */

#ifndef VDL_CORE_BUFFER_HPP
#define VDL_CORE_BUFFER_HPP

#include <vdl/core/types.hpp>
#include <vdl/core/error.hpp>
#include <vdl/core/noncopyable.hpp>
#include <algorithm>
#include <cstring>

namespace vdl {

// ============================================================================
// 环形缓冲区
// ============================================================================

/**
 * @brief 环形缓冲区
 * 
 * 适用于流式数据的缓冲区实现。
 */
class ring_buffer_t : public noncopyable_t {
public:
    /**
     * @brief 构造指定容量的环形缓冲区
     */
    explicit ring_buffer_t(size_t capacity)
        : m_buffer(capacity)
        , m_capacity(capacity)
        , m_read_pos(0)
        , m_write_pos(0)
        , m_size(0) {}

    /**
     * @brief 获取容量
     */
    size_t capacity() const { return m_capacity; }

    /**
     * @brief 获取当前数据大小
     */
    size_t size() const { return m_size; }

    /**
     * @brief 获取可用空间
     */
    size_t available() const { return m_capacity - m_size; }

    /**
     * @brief 检查是否为空
     */
    bool empty() const { return m_size == 0; }

    /**
     * @brief 检查是否已满
     */
    bool full() const { return m_size == m_capacity; }

    /**
     * @brief 写入数据
     * 
     * @param data 数据指针
     * @param len 数据长度
     * @return 实际写入的字节数
     */
    size_t write(const byte_t* data, size_t len) {
        if (data == nullptr || len == 0) {
            return 0;
        }

        size_t to_write = std::min(len, available());
        if (to_write == 0) {
            return 0;
        }

        // 分两段写入（如果跨越边界）
        size_t first_part = std::min(to_write, m_capacity - m_write_pos);
        std::memcpy(&m_buffer[m_write_pos], data, first_part);

        if (to_write > first_part) {
            std::memcpy(&m_buffer[0], data + first_part, to_write - first_part);
        }

        m_write_pos = (m_write_pos + to_write) % m_capacity;
        m_size += to_write;

        return to_write;
    }

    /**
     * @brief 写入数据（span 版本）
     */
    size_t write(const_byte_span_t data) {
        return write(data.data(), data.size());
    }

    /**
     * @brief 读取数据
     * 
     * @param buffer 接收缓冲区
     * @param len 要读取的长度
     * @return 实际读取的字节数
     */
    size_t read(byte_t* buffer, size_t len) {
        if (buffer == nullptr || len == 0) {
            return 0;
        }

        size_t to_read = std::min(len, m_size);
        if (to_read == 0) {
            return 0;
        }

        // 分两段读取（如果跨越边界）
        size_t first_part = std::min(to_read, m_capacity - m_read_pos);
        std::memcpy(buffer, &m_buffer[m_read_pos], first_part);

        if (to_read > first_part) {
            std::memcpy(buffer + first_part, &m_buffer[0], to_read - first_part);
        }

        m_read_pos = (m_read_pos + to_read) % m_capacity;
        m_size -= to_read;

        return to_read;
    }

    /**
     * @brief 读取数据（span 版本）
     */
    size_t read(byte_span_t buffer) {
        return read(buffer.data(), buffer.size());
    }

    /**
     * @brief 预览数据（不移除）
     */
    size_t peek(byte_t* buffer, size_t len) const {
        if (buffer == nullptr || len == 0) {
            return 0;
        }

        size_t to_peek = std::min(len, m_size);
        if (to_peek == 0) {
            return 0;
        }

        size_t first_part = std::min(to_peek, m_capacity - m_read_pos);
        std::memcpy(buffer, &m_buffer[m_read_pos], first_part);

        if (to_peek > first_part) {
            std::memcpy(buffer + first_part, &m_buffer[0], to_peek - first_part);
        }

        return to_peek;
    }

    /**
     * @brief 跳过指定字节数
     */
    size_t skip(size_t len) {
        size_t to_skip = std::min(len, m_size);
        m_read_pos = (m_read_pos + to_skip) % m_capacity;
        m_size -= to_skip;
        return to_skip;
    }

    /**
     * @brief 消费指定字节数（skip 的别名）
     */
    size_t consume(size_t len) {
        return skip(len);
    }

    /**
     * @brief 获取可读数据大小（size 的别名）
     */
    size_t readable_size() const { return m_size; }

    /**
     * @brief 获取可写空间大小（available 的别名）
     */
    size_t writable_size() const { return available(); }

    /**
     * @brief 写入单个字节
     */
    bool push(byte_t b) {
        if (full()) {
            return false;
        }
        m_buffer[m_write_pos] = b;
        m_write_pos = (m_write_pos + 1) % m_capacity;
        ++m_size;
        return true;
    }

    /**
     * @brief 读取单个字节
     */
    tl::optional<byte_t> pop() {
        if (empty()) {
            return tl::nullopt;
        }
        byte_t b = m_buffer[m_read_pos];
        m_read_pos = (m_read_pos + 1) % m_capacity;
        --m_size;
        return b;
    }

    /**
     * @brief 清空缓冲区
     */
    void clear() {
        m_read_pos = 0;
        m_write_pos = 0;
        m_size = 0;
    }

private:
    bytes_t m_buffer;
    size_t m_capacity;
    size_t m_read_pos;
    size_t m_write_pos;
    size_t m_size;
};

// ============================================================================
// 静态缓冲区
// ============================================================================

/**
 * @brief 固定大小的静态缓冲区
 */
template<size_t N>
class static_buffer_t {
public:
    static_buffer_t() : m_size(0) {}

    byte_t* data() { return m_data; }
    const byte_t* data() const { return m_data; }
    size_t size() const { return m_size; }
    size_t capacity() const { return N; }
    bool empty() const { return m_size == 0; }

    void set_size(size_t s) { m_size = std::min(s, N); }
    void clear() { m_size = 0; }

    byte_t& operator[](size_t idx) { return m_data[idx]; }
    const byte_t& operator[](size_t idx) const { return m_data[idx]; }

    byte_span_t as_span() { return byte_span_t(m_data, m_size); }
    const_byte_span_t as_span() const { return const_byte_span_t(m_data, m_size); }

private:
    byte_t m_data[N];
    size_t m_size;
};

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * @brief 从字节数组创建 bytes_t
 */
inline bytes_t make_bytes(const byte_t* data, size_t len) {
    return bytes_t(data, data + len);
}

/**
 * @brief 从 span 创建 bytes_t
 */
inline bytes_t make_bytes(const_byte_span_t span) {
    return bytes_t(span.begin(), span.end());
}

/**
 * @brief 比较两个 span
 */
inline bool bytes_equal(const_byte_span_t a, const_byte_span_t b) {
    if (a.size() != b.size()) {
        return false;
    }
    return std::equal(a.begin(), a.end(), b.begin());
}

}  // namespace vdl

#endif  // VDL_CORE_BUFFER_HPP
