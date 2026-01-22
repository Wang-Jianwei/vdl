/**
 * @file heartbeat_config.hpp
 * @brief 心跳检测配置
 * 
 * 定义心跳检测的配置参数。
 */

#ifndef VDL_HEARTBEAT_HEARTBEAT_CONFIG_HPP
#define VDL_HEARTBEAT_HEARTBEAT_CONFIG_HPP

#include "../core/types.hpp"

namespace vdl {

// ============================================================================
// heartbeat_config_t - 心跳配置
// ============================================================================

/**
 * @brief 心跳配置结构
 * 
 * 用于控制心跳检测的参数。
 */
struct heartbeat_config_t {
    /**
     * @brief 心跳间隔（毫秒）
     * 
     * 定期发送心跳命令的时间间隔。
     * 默认: 1000ms
     */
    milliseconds_t interval = 1000;

    /**
     * @brief 单次心跳超时（毫秒）
     * 
     * 等待心跳响应的最大时间。
     * 默认: 500ms
     */
    milliseconds_t timeout = 500;

    /**
     * @brief 最大连续失败次数
     * 
     * 超过此阈值，将触发错误回调或状态变更。
     * 默认: 3
     */
    uint8_t max_failures = 3;

    /**
     * @brief 独占期间是否暂停心跳
     * 
     * 当设备被独占锁定（device_guard）时，是否暂停心跳检测。
     * 这避免了心跳与命令执行的冲突。
     * 默认: true
     */
    bool pause_during_lock = true;

    /**
     * @brief 自动重启失败计数器
     * 
     * 一次成功的心跳后是否重置失败计数。
     * 默认: true
     */
    bool auto_reset_failures = true;
};

}  // namespace vdl

#endif  // VDL_HEARTBEAT_HEARTBEAT_CONFIG_HPP
