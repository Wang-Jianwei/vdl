/**
 * @file command.hpp
 * @brief 命令定义
 * 
 * 定义设备命令的数据结构。
 */

#ifndef VDL_PROTOCOL_COMMAND_HPP
#define VDL_PROTOCOL_COMMAND_HPP

#include "../core/types.hpp"

#include <string>

namespace vdl {

// ============================================================================
// 命令类型
// ============================================================================

/**
 * @brief 命令类型枚举
 */
enum class command_type_t : uint8_t {
    read = 0x01,        ///< 读取命令
    write = 0x02,       ///< 写入命令
    execute = 0x03,     ///< 执行命令
    query = 0x04,       ///< 查询命令
    custom = 0xFF       ///< 自定义命令
};

// ============================================================================
// command_t - 命令类
// ============================================================================

/**
 * @brief 设备命令
 * 
 * 表示发送给设备的命令，包含：
 * - 命令类型
 * - 功能码
 * - 参数数据
 * 
 * @code
 * // 创建读取命令
 * command_t cmd;
 * cmd.set_type(command_type_t::read)
 *    .set_function_code(0x03)
 *    .set_address(0x0000)
 *    .set_count(10);
 * 
 * // 创建写入命令
 * command_t write_cmd;
 * write_cmd.set_type(command_type_t::write)
 *          .set_function_code(0x06)
 *          .set_address(0x0100)
 *          .set_data({0x12, 0x34});
 * @endcode
 */
class command_t {
public:
    command_t() = default;

    // ========================================================================
    // 构建器模式
    // ========================================================================

    command_t& set_type(command_type_t type) {
        m_type = type;
        return *this;
    }

    command_t& set_function_code(uint8_t code) {
        m_function_code = code;
        return *this;
    }

    command_t& set_address(uint16_t address) {
        m_address = address;
        return *this;
    }

    command_t& set_count(uint16_t count) {
        m_count = count;
        return *this;
    }

    command_t& set_data(const bytes_t& data) {
        m_data = data;
        return *this;
    }

    command_t& set_data(bytes_t&& data) {
        m_data = std::move(data);
        return *this;
    }

    command_t& set_data(std::initializer_list<byte_t> data) {
        m_data.assign(data.begin(), data.end());
        return *this;
    }

    command_t& set_tag(const std::string& tag) {
        m_tag = tag;
        return *this;
    }

    // ========================================================================
    // 访问器
    // ========================================================================

    command_type_t type() const { return m_type; }
    uint8_t function_code() const { return m_function_code; }
    uint16_t address() const { return m_address; }
    uint16_t count() const { return m_count; }
    const bytes_t& data() const { return m_data; }
    bytes_t& data() { return m_data; }
    const std::string& tag() const { return m_tag; }

    // ========================================================================
    // 便捷方法
    // ========================================================================

    /**
     * @brief 检查是否有数据
     */
    bool has_data() const {
        return !m_data.empty();
    }

    /**
     * @brief 清空命令
     */
    void clear() {
        m_type = command_type_t::read;
        m_function_code = 0;
        m_address = 0;
        m_count = 0;
        m_data.clear();
        m_tag.clear();
    }

private:
    command_type_t m_type = command_type_t::read;
    uint8_t m_function_code = 0;
    uint16_t m_address = 0;
    uint16_t m_count = 0;
    bytes_t m_data;
    std::string m_tag;  // 用于调试/追踪
};

// ============================================================================
// 工厂函数
// ============================================================================

/**
 * @brief 创建读取命令
 */
inline command_t make_read_command(uint8_t function_code, 
                                    uint16_t address, 
                                    uint16_t count) {
    return command_t()
        .set_type(command_type_t::read)
        .set_function_code(function_code)
        .set_address(address)
        .set_count(count);
}

/**
 * @brief 创建写入命令
 */
inline command_t make_write_command(uint8_t function_code,
                                     uint16_t address,
                                     const bytes_t& data) {
    return command_t()
        .set_type(command_type_t::write)
        .set_function_code(function_code)
        .set_address(address)
        .set_data(data);
}

/**
 * @brief 创建执行命令
 */
inline command_t make_execute_command(uint8_t function_code) {
    return command_t()
        .set_type(command_type_t::execute)
        .set_function_code(function_code);
}

}  // namespace vdl

#endif  // VDL_PROTOCOL_COMMAND_HPP
