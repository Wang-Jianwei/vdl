/**
 * @file ping_heartbeat.hpp
 * @brief Ping 心跳策略
 * 
 * 最简单的心跳策略：发送空命令，检查任意非错误响应。
 */

#ifndef VDL_HEARTBEAT_STRATEGIES_PING_HEARTBEAT_HPP
#define VDL_HEARTBEAT_STRATEGIES_PING_HEARTBEAT_HPP

#include "../heartbeat_strategy.hpp"

namespace vdl {

// ============================================================================
// ping_heartbeat_t - Ping 策略
// ============================================================================

/**
 * @brief Ping 心跳策略
 * 
 * 策略：
 * 1. 生成一个空命令（只有函数码，无数据）
 * 2. 验证响应：检查响应是否有效（非错误码）
 * 
 * 用途：
 * - 简单的可用性检查
 * - 通用的保活命令
 * 
 * 优点：
 * - 实现简单
 * - 适用于大多数协议
 * 
 * 缺点：
 * - 不能验证设备功能
 * - 只检查连接是否活跃
 */
class ping_heartbeat_t : public i_heartbeat_strategy_t {
public:
    /**
     * @brief 构造函数
     * @param function_code 使用的函数码（默认 0x00 = 空操作）
     */
    explicit ping_heartbeat_t(uint8_t function_code = 0x00)
        : m_function_code(function_code) {
    }

    // i_heartbeat_strategy_t 实现
    result_t<command_t> make_heartbeat_command() override {
        command_t cmd;
        cmd.set_function_code(m_function_code);
        // 无数据载荷
        return make_ok(std::move(cmd));
    }

    bool validate_response(const response_t& resp) override {
        // 只检查响应有效性（无 error_code）
        // 任何有效的响应都被认为是成功的心跳
        return !resp.is_error();
    }

    const char* name() const override {
        return "ping_heartbeat";
    }

private:
    uint8_t m_function_code;
};

}  // namespace vdl

#endif  // VDL_HEARTBEAT_STRATEGIES_PING_HEARTBEAT_HPP
