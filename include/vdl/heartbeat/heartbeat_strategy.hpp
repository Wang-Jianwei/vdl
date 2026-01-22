/**
 * @file heartbeat_strategy.hpp
 * @brief 心跳策略接口
 * 
 * 定义心跳检测策略的抽象接口。
 */

#ifndef VDL_HEARTBEAT_HEARTBEAT_STRATEGY_HPP
#define VDL_HEARTBEAT_HEARTBEAT_STRATEGY_HPP

#include "../core/types.hpp"
#include "../core/error.hpp"
#include "../protocol/command.hpp"
#include "../protocol/response.hpp"

#include <memory>

namespace vdl {

// ============================================================================
// i_heartbeat_strategy_t - 心跳策略接口
// ============================================================================

/**
 * @brief 心跳策略接口
 * 
 * 定义心跳检测的策略。实现类需要提供：
 * 1. 生成心跳命令
 * 2. 验证心跳响应
 * 
 * @note 策略对象需要是线程安全的
 */
class i_heartbeat_strategy_t {
public:
    virtual ~i_heartbeat_strategy_t() = default;

    // ========================================================================
    // 策略接口
    // ========================================================================

    /**
     * @brief 生成心跳命令
     * 
     * @return 返回心跳命令，失败返回错误
     * 
     * @note 可能需要设备的引用来获取配置，但为了保持策略的独立性，
     *       实现类应该在构造时存储必要的配置。
     */
    virtual result_t<command_t> make_heartbeat_command() = 0;

    /**
     * @brief 验证心跳响应
     * 
     * @param resp 接收到的响应
     * @return true 表示验证成功，false 表示验证失败
     * 
     * @note 如果解析响应失败，应该返回 false 而非异常
     */
    virtual bool validate_response(const response_t& resp) = 0;

    /**
     * @brief 获取策略名称（用于日志）
     */
    virtual const char* name() const = 0;
};

// ============================================================================
// heartbeat_strategy_ptr_t - 心跳策略智能指针
// ============================================================================

using heartbeat_strategy_ptr_t = std::unique_ptr<i_heartbeat_strategy_t>;

}  // namespace vdl

#endif  // VDL_HEARTBEAT_HEARTBEAT_STRATEGY_HPP
