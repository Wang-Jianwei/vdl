/**
 * @file response.hpp
 * @brief 响应定义
 * 
 * 定义设备响应的数据结构。
 */

#ifndef VDL_PROTOCOL_RESPONSE_HPP
#define VDL_PROTOCOL_RESPONSE_HPP

#include "../core/types.hpp"
#include "../core/error.hpp"

#include <string>

namespace vdl {

// ============================================================================
// 响应状态
// ============================================================================

/**
 * @brief 响应状态
 */
enum class response_status_t : uint8_t {
    success = 0x00,     ///< 成功
    error = 0x01,       ///< 错误
    busy = 0x02,        ///< 设备忙
    timeout = 0x03,     ///< 超时
    invalid = 0xFF      ///< 无效响应
};

// ============================================================================
// response_t - 响应类
// ============================================================================

/**
 * @brief 设备响应
 * 
 * 表示从设备接收的响应，包含：
 * - 响应状态
 * - 功能码
 * - 响应数据
 * 
 * @code
 * // 检查响应
 * auto result = device.execute(cmd);
 * if (result) {
 *     response_t& resp = *result;
 *     if (resp.is_success()) {
 *         process_data(resp.data());
 *     }
 * }
 * @endcode
 */
class response_t {
public:
    response_t() = default;

    // ========================================================================
    // 构建器模式
    // ========================================================================

    response_t& set_status(response_status_t status) {
        m_status = status;
        return *this;
    }

    response_t& set_function_code(uint8_t code) {
        m_function_code = code;
        return *this;
    }

    response_t& set_error_code(uint8_t code) {
        m_error_code = code;
        return *this;
    }

    response_t& set_data(const bytes_t& data) {
        m_data = data;
        return *this;
    }

    response_t& set_data(bytes_t&& data) {
        m_data = std::move(data);
        return *this;
    }

    response_t& set_raw_frame(const bytes_t& frame) {
        m_raw_frame = frame;
        return *this;
    }

    response_t& set_raw_frame(bytes_t&& frame) {
        m_raw_frame = std::move(frame);
        return *this;
    }

    // ========================================================================
    // 访问器
    // ========================================================================

    response_status_t status() const { return m_status; }
    uint8_t function_code() const { return m_function_code; }
    uint8_t error_code() const { return m_error_code; }
    const bytes_t& data() const { return m_data; }
    bytes_t& data() { return m_data; }
    const bytes_t& raw_frame() const { return m_raw_frame; }

    // ========================================================================
    // 状态检查
    // ========================================================================

    /**
     * @brief 是否成功
     */
    bool is_success() const {
        return m_status == response_status_t::success;
    }

    /**
     * @brief 是否错误
     */
    bool is_error() const {
        return m_status == response_status_t::error;
    }

    /**
     * @brief 是否设备忙
     */
    bool is_busy() const {
        return m_status == response_status_t::busy;
    }

    /**
     * @brief 是否有数据
     */
    bool has_data() const {
        return !m_data.empty();
    }

    // ========================================================================
    // 数据访问便捷方法
    // ========================================================================

    /**
     * @brief 获取数据大小
     */
    size_t data_size() const {
        return m_data.size();
    }

    /**
     * @brief 获取单个字节
     */
    byte_t get_byte(size_t index) const {
        return (index < m_data.size()) ? m_data[index] : 0;
    }

    /**
     * @brief 获取16位无符号整数（大端序）
     */
    uint16_t get_uint16_be(size_t offset) const {
        if (offset + 2 > m_data.size()) return 0;
        return static_cast<uint16_t>(
            (static_cast<uint16_t>(m_data[offset]) << 8) |
            static_cast<uint16_t>(m_data[offset + 1])
        );
    }

    /**
     * @brief 获取16位无符号整数（小端序）
     */
    uint16_t get_uint16_le(size_t offset) const {
        if (offset + 2 > m_data.size()) return 0;
        return static_cast<uint16_t>(
            static_cast<uint16_t>(m_data[offset]) |
            (static_cast<uint16_t>(m_data[offset + 1]) << 8)
        );
    }

    /**
     * @brief 获取32位无符号整数（大端序）
     */
    uint32_t get_uint32_be(size_t offset) const {
        if (offset + 4 > m_data.size()) return 0;
        return (static_cast<uint32_t>(m_data[offset]) << 24) |
               (static_cast<uint32_t>(m_data[offset + 1]) << 16) |
               (static_cast<uint32_t>(m_data[offset + 2]) << 8) |
               static_cast<uint32_t>(m_data[offset + 3]);
    }

    /**
     * @brief 获取32位无符号整数（小端序）
     */
    uint32_t get_uint32_le(size_t offset) const {
        if (offset + 4 > m_data.size()) return 0;
        return static_cast<uint32_t>(m_data[offset]) |
               (static_cast<uint32_t>(m_data[offset + 1]) << 8) |
               (static_cast<uint32_t>(m_data[offset + 2]) << 16) |
               (static_cast<uint32_t>(m_data[offset + 3]) << 24);
    }

    /**
     * @brief 清空响应
     */
    void clear() {
        m_status = response_status_t::invalid;
        m_function_code = 0;
        m_error_code = 0;
        m_data.clear();
        m_raw_frame.clear();
    }

private:
    response_status_t m_status = response_status_t::invalid;
    uint8_t m_function_code = 0;
    uint8_t m_error_code = 0;
    bytes_t m_data;
    bytes_t m_raw_frame;  // 原始帧（用于调试）
};

// ============================================================================
// 工厂函数
// ============================================================================

/**
 * @brief 创建成功响应
 */
inline response_t make_success_response(uint8_t function_code, 
                                         const bytes_t& data = {}) {
    return response_t()
        .set_status(response_status_t::success)
        .set_function_code(function_code)
        .set_data(data);
}

/**
 * @brief 创建错误响应
 */
inline response_t make_error_response(uint8_t function_code,
                                       uint8_t error_code) {
    return response_t()
        .set_status(response_status_t::error)
        .set_function_code(function_code)
        .set_error_code(error_code);
}

}  // namespace vdl

#endif  // VDL_PROTOCOL_RESPONSE_HPP
