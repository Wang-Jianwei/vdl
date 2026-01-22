/**
 * @file scpi_heartbeat.hpp
 * @brief SCPI 心跳策略
 * 
 * 使用 SCPI *IDN? 命令的心跳策略。
 */

#ifndef VDL_HEARTBEAT_STRATEGIES_SCPI_HEARTBEAT_HPP
#define VDL_HEARTBEAT_STRATEGIES_SCPI_HEARTBEAT_HPP

#include "../heartbeat_strategy.hpp"

namespace vdl {

// ============================================================================
// scpi_heartbeat_t - SCPI 策略
// ============================================================================

/**
 * @brief SCPI 心跳策略
 * 
 * 策略：
 * 1. 生成 SCPI *IDN? 命令
 * 2. 验证响应中是否包含有效的 IDN 信息
 * 
 * 用途：
 * - 针对 SCPI 设备的标准心跳
 * - 查询设备标识信息
 * - 验证设备功能完整性
 * 
 * 优点：
 * - SCPI 标准命令，通用性强
 * - 能验证设备的完整功能
 * - 获得设备标识信息
 * 
 * 缺点：
 * - 仅适用于 SCPI 设备
 * - 比其他策略消耗更多资源
 * 
 * SCPI IDN 格式（通常）：
 * - "Manufacturer,Model,SerialNumber,FirmwareVersion"
 * - 例如："Keysight,VNA_E5071C,US123456,A.01.01"
 */
class scpi_heartbeat_t : public i_heartbeat_strategy_t {
public:
    scpi_heartbeat_t() = default;

    // i_heartbeat_strategy_t 实现
    result_t<command_t> make_heartbeat_command() override {
        // 创建 SCPI *IDN? 命令
        command_t cmd;
        // SCPI 命令通常使用函数码 0x04 (Query)
        cmd.set_function_code(0x04);
        
        // 设置命令数据为 "*IDN?"
        const bytes_t scpi_cmd = {
            0x2A,  // '*'
            0x49,  // 'I'
            0x44,  // 'D'
            0x4E,  // 'N'
            0x3F   // '?'
        };
        cmd.set_data(scpi_cmd);
        
        return make_ok(std::move(cmd));
    }

    bool validate_response(const response_t& resp) override {
        // 检查响应是否有效
        if (resp.is_error()) {
            return false;
        }

        // 检查响应数据是否非空
        const auto& data = resp.data();
        if (data.empty()) {
            return false;
        }

        // 检查响应是否包含有效的 IDN 信息
        // SCPI IDN 响应通常包含逗号分隔的字段
        // 至少应该包含制造商信息
        
        // 简单检查：响应长度应该合理（至少有制造商名）
        if (data.size() < 2) {
            return false;
        }

        // 检查是否包含有效的 ASCII 字符（IDN 通常为 ASCII）
        for (const auto& byte : data) {
            // IDN 应该是可打印的 ASCII 或逗号
            if (byte < 0x20 || (byte > 0x7E && byte != 0x2C)) {
                // 0x2C 是逗号
                return false;
            }
        }

        return true;
    }

    const char* name() const override {
        return "scpi_heartbeat";
    }
};

}  // namespace vdl

#endif  // VDL_HEARTBEAT_STRATEGIES_SCPI_HEARTBEAT_HPP
