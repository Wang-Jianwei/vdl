/**
 * @file echo_heartbeat.hpp
 * @brief Echo 心跳策略
 * 
 * 发送数据并验证回显的心跳策略。
 */

#ifndef VDL_HEARTBEAT_STRATEGIES_ECHO_HEARTBEAT_HPP
#define VDL_HEARTBEAT_STRATEGIES_ECHO_HEARTBEAT_HPP

#include "../heartbeat_strategy.hpp"

namespace vdl {

// ============================================================================
// echo_heartbeat_t - Echo 策略
// ============================================================================

/**
 * @brief Echo 心跳策略
 * 
 * 策略：
 * 1. 生成一个包含特定数据的命令
 * 2. 验证响应数据是否与发送数据匹配
 * 
 * 用途：
 * - 验证设备的基本功能
 * - 检测数据完整性
 * - 适用于支持回显的协议
 * 
 * 优点：
 * - 能验证数据传输的完整性
 * - 可以检测设备状态
 * 
 * 缺点：
 * - 比 Ping 更消耗资源
 * - 需要设备支持回显功能
 */
class echo_heartbeat_t : public i_heartbeat_strategy_t {
public:
    /**
     * @brief 构造函数
     * @param function_code 使用的函数码
     * @param echo_data 回显数据内容
     */
    echo_heartbeat_t(uint8_t function_code, const bytes_t& echo_data)
        : m_function_code(function_code)
        , m_echo_data(echo_data) {
    }

    /**
     * @brief 构造函数（使用默认回显数据）
     * @param function_code 使用的函数码
     */
    explicit echo_heartbeat_t(uint8_t function_code = 0x00)
        : m_function_code(function_code)
        , m_echo_data({0x48, 0x65, 0x6C, 0x6C, 0x6F})  // "Hello"
    {
    }

    // i_heartbeat_strategy_t 实现
    result_t<command_t> make_heartbeat_command() override {
        command_t cmd;
        cmd.set_function_code(m_function_code);
        cmd.set_data(m_echo_data);
        return make_ok(std::move(cmd));
    }

    bool validate_response(const response_t& resp) override {
        // 检查响应是否有效且包含相同的数据
        if (resp.is_error()) {
            return false;
        }

        // 获取响应数据并比较
        const auto& resp_data = resp.data();
        if (resp_data.size() != m_echo_data.size()) {
            return false;
        }

        // 比较数据内容
        return std::equal(m_echo_data.begin(), m_echo_data.end(),
                         resp_data.begin());
    }

    const char* name() const override {
        return "echo_heartbeat";
    }

    /**
     * @brief 获取回显数据
     */
    const bytes_t& get_echo_data() const {
        return m_echo_data;
    }

    /**
     * @brief 设置回显数据
     */
    void set_echo_data(const bytes_t& data) {
        m_echo_data = data;
    }

private:
    uint8_t m_function_code;
    bytes_t m_echo_data;
};

}  // namespace vdl

#endif  // VDL_HEARTBEAT_STRATEGIES_ECHO_HEARTBEAT_HPP
