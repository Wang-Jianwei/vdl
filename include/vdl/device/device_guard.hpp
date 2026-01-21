/**
 * @file device_guard.hpp
 * @brief 设备连接守卫
 * 
 * 提供 RAII 风格的设备连接管理。
 */

#ifndef VDL_DEVICE_DEVICE_GUARD_HPP
#define VDL_DEVICE_DEVICE_GUARD_HPP

#include "device.hpp"
#include "../core/noncopyable.hpp"

namespace vdl {

// ============================================================================
// device_guard_t - 设备连接守卫
// ============================================================================

/**
 * @brief 设备连接守卫，RAII 风格管理设备连接
 * 
 * 在构造时连接设备，析构时自动断开连接。
 * 
 * @code
 * {
 *     device_guard_t guard(device);
 *     if (!guard.is_connected()) {
 *         // 处理连接失败
 *         return;
 *     }
 *     
 *     // 使用设备
 *     device.execute(cmd);
 * }  // 自动断开连接
 * @endcode
 */
class device_guard_t : private noncopyable_t {
public:
    /**
     * @brief 构造函数，自动连接设备
     * @param device 设备引用
     * @param auto_connect 是否自动连接
     */
    explicit device_guard_t(i_device_t& device, bool auto_connect = true)
        : m_device(device)
        , m_owns_connection(false) {
        if (auto_connect && !device.is_connected()) {
            auto result = device.connect();
            m_owns_connection = result.has_value();
            m_connect_error = result.has_value() ? error_t{} : result.error();
        }
    }

    /**
     * @brief 移动构造函数
     */
    device_guard_t(device_guard_t&& other)
        : m_device(other.m_device)
        , m_owns_connection(other.m_owns_connection)
        , m_connect_error(std::move(other.m_connect_error)) {
        other.m_owns_connection = false;
    }

    /**
     * @brief 析构函数，自动断开连接
     */
    ~device_guard_t() {
        if (m_owns_connection) {
            m_device.disconnect();
        }
    }

    /**
     * @brief 检查是否已连接
     */
    bool is_connected() const {
        return m_device.is_connected();
    }

    /**
     * @brief 获取连接错误
     */
    const error_t& connect_error() const {
        return m_connect_error;
    }

    /**
     * @brief 检查连接是否成功
     */
    bool has_error() const {
        return m_connect_error.code() != error_code_t::ok;
    }

    /**
     * @brief 获取设备引用
     */
    i_device_t& device() {
        return m_device;
    }

    const i_device_t& device() const {
        return m_device;
    }

    /**
     * @brief 释放连接所有权（不再自动断开）
     */
    void release() {
        m_owns_connection = false;
    }

private:
    i_device_t& m_device;
    bool m_owns_connection;
    error_t m_connect_error;
};

// ============================================================================
// 工厂函数
// ============================================================================

/**
 * @brief 创建设备守卫（自动连接）
 */
inline device_guard_t make_device_guard(i_device_t& device) {
    return device_guard_t(device, true);
}

/**
 * @brief 创建设备守卫（不自动连接）
 */
inline device_guard_t make_device_guard_no_connect(i_device_t& device) {
    return device_guard_t(device, false);
}

}  // namespace vdl

#endif  // VDL_DEVICE_DEVICE_GUARD_HPP
