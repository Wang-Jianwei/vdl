/**
 * @file memory.hpp
 * @brief 内存管理工具
 * 
 * 提供内存池和智能指针相关工具。
 */

#ifndef VDL_CORE_MEMORY_HPP
#define VDL_CORE_MEMORY_HPP

#include "compat.hpp"
#include "noncopyable.hpp"
#include "types.hpp"

#include <memory>
#include <vector>
#include <cstring>

namespace vdl {

// ============================================================================
// buffer_pool_t - 缓冲区池
// ============================================================================

/**
 * @brief 缓冲区池，用于复用缓冲区减少内存分配
 * 
 * @code
 * buffer_pool_t pool(1024);  // 每个缓冲区 1024 字节
 * 
 * auto buf = pool.acquire();
 * // ... 使用 buf ...
 * pool.release(std::move(buf));
 * @endcode
 */
class buffer_pool_t : private noncopyable_t {
public:
    /**
     * @brief 构造函数
     * @param buffer_size 每个缓冲区的大小
     * @param initial_count 初始缓冲区数量
     */
    explicit buffer_pool_t(size_t buffer_size, size_t initial_count = 4)
        : m_buffer_size(buffer_size) {
        for (size_t i = 0; i < initial_count; ++i) {
            m_pool.push_back(bytes_t(buffer_size));
        }
    }

    /**
     * @brief 获取一个缓冲区
     * @return 缓冲区
     */
    bytes_t acquire() {
        if (!m_pool.empty()) {
            bytes_t buf = std::move(m_pool.back());
            m_pool.pop_back();
            return buf;
        }
        return bytes_t(m_buffer_size);
    }

    /**
     * @brief 归还缓冲区
     * @param buf 要归还的缓冲区
     */
    void release(bytes_t buf) {
        if (buf.size() == m_buffer_size && m_pool.size() < m_max_pool_size) {
            m_pool.push_back(std::move(buf));
        }
    }

    /**
     * @brief 获取缓冲区大小
     */
    size_t buffer_size() const {
        return m_buffer_size;
    }

    /**
     * @brief 获取池中缓冲区数量
     */
    size_t pool_size() const {
        return m_pool.size();
    }

    /**
     * @brief 设置最大池大小
     */
    void set_max_pool_size(size_t max_size) {
        m_max_pool_size = max_size;
    }

    /**
     * @brief 清空池
     */
    void clear() {
        m_pool.clear();
    }

private:
    size_t m_buffer_size;
    size_t m_max_pool_size = 64;
    std::vector<bytes_t> m_pool;
};

// ============================================================================
// pooled_buffer_t - 自动归还的缓冲区
// ============================================================================

/**
 * @brief 自动归还的缓冲区，析构时自动归还到池
 */
class pooled_buffer_t : private noncopyable_t {
public:
    /**
     * @brief 构造函数
     */
    pooled_buffer_t(buffer_pool_t& pool, bytes_t buf)
        : m_pool(&pool)
        , m_buffer(std::move(buf)) {
    }

    /**
     * @brief 移动构造函数
     */
    pooled_buffer_t(pooled_buffer_t&& other)
        : m_pool(other.m_pool)
        , m_buffer(std::move(other.m_buffer)) {
        other.m_pool = nullptr;
    }

    /**
     * @brief 移动赋值
     */
    pooled_buffer_t& operator=(pooled_buffer_t&& other) {
        if (this != &other) {
            _release();
            m_pool = other.m_pool;
            m_buffer = std::move(other.m_buffer);
            other.m_pool = nullptr;
        }
        return *this;
    }

    /**
     * @brief 析构函数，自动归还缓冲区
     */
    ~pooled_buffer_t() {
        _release();
    }

    /**
     * @brief 获取数据指针
     */
    byte_t* data() {
        return m_buffer.data();
    }

    const byte_t* data() const {
        return m_buffer.data();
    }

    /**
     * @brief 获取大小
     */
    size_t size() const {
        return m_buffer.size();
    }

    /**
     * @brief 获取span视图
     */
    byte_span_t span() {
        return make_span(m_buffer);
    }

    const_byte_span_t span() const {
        return make_span(m_buffer);
    }

    /**
     * @brief 释放缓冲区（不归还到池）
     */
    bytes_t release() {
        m_pool = nullptr;
        return std::move(m_buffer);
    }

private:
    void _release() {
        if (m_pool && !m_buffer.empty()) {
            m_pool->release(std::move(m_buffer));
        }
    }

    buffer_pool_t* m_pool;
    bytes_t m_buffer;
};

/**
 * @brief 从池中获取自动归还的缓冲区
 */
inline pooled_buffer_t acquire_buffer(buffer_pool_t& pool) {
    return pooled_buffer_t(pool, pool.acquire());
}

// ============================================================================
// 内存操作工具
// ============================================================================

/**
 * @brief 安全内存复制
 * @param dest 目标
 * @param src 源
 * @param count 字节数
 */
inline void mem_copy(void* dest, const void* src, size_t count) {
    if (dest && src && count > 0) {
        std::memcpy(dest, src, count);
    }
}

/**
 * @brief 安全内存设置
 */
inline void mem_set(void* dest, int value, size_t count) {
    if (dest && count > 0) {
        std::memset(dest, value, count);
    }
}

/**
 * @brief 安全内存清零
 */
inline void mem_zero(void* dest, size_t count) {
    mem_set(dest, 0, count);
}

/**
 * @brief 内存比较
 */
inline int mem_compare(const void* lhs, const void* rhs, size_t count) {
    if (!lhs || !rhs || count == 0) {
        return 0;
    }
    return std::memcmp(lhs, rhs, count);
}

}  // namespace vdl

#endif  // VDL_CORE_MEMORY_HPP
