/**
 * @file heartbeat_runner.hpp
 * @brief 心跳运行器
 * 
 * 在独立线程中定期执行心跳检测。
 */

#ifndef VDL_HEARTBEAT_HEARTBEAT_RUNNER_HPP
#define VDL_HEARTBEAT_HEARTBEAT_RUNNER_HPP

#include "heartbeat_config.hpp"
#include "heartbeat_strategy.hpp"
#include "../device/device.hpp"
#include "../core/types.hpp"
#include "../core/error.hpp"

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <chrono>

namespace vdl {

// ============================================================================
// Heartbeat 事件回调
// ============================================================================

/**
 * @brief 心跳事件类型
 */
enum class heartbeat_event_t : uint8_t {
    success = 0,         ///< 心跳成功
    failure = 1,         ///< 心跳失败
    max_failures = 2,    ///< 达到最大失败次数
    paused = 3,          ///< 心跳已暂停
    resumed = 4,         ///< 心跳已恢复
    stopped = 5          ///< 心跳已停止
};

/**
 * @brief 心跳事件回调类型
 * 
 * 参数：
 * - event: 事件类型
 * - failure_count: 当前失败次数（仅在 failure 事件时有意义）
 * - error: 错误信息（仅在 failure 事件时有意义）
 */
using heartbeat_callback_t = std::function<void(
    heartbeat_event_t event,
    uint8_t failure_count,
    const error_t& error
)>;

// ============================================================================
// heartbeat_runner_t - 心跳运行器
// ============================================================================

/**
 * @brief 心跳运行器
 * 
 * 在独立线程中定期执行心跳检测，监控设备连接状态。
 * 
 * 功能：
 * 1. 定期发送心跳命令
 * 2. 验证响应
 * 3. 追踪失败次数
 * 4. 支持暂停/恢复
 * 5. 事件通知
 * 
 * 用法：
 * @code
 * // 创建运行器
 * auto runner = std::make_unique<heartbeat_runner_t>(
 *     device,
 *     std::make_unique<ping_heartbeat_t>(),
 *     heartbeat_config_t()
 * );
 * 
 * // 设置回调
 * runner->set_callback([](heartbeat_event_t event, uint8_t failures, const error_t& err) {
 *     if (event == heartbeat_event_t::max_failures) {
 *         std::cerr << "心跳失败次数达到上限，尝试重连\n";
 *         // 执行重连逻辑
 *     }
 * });
 * 
 * // 启动
 * runner->start();
 * 
 * // ... 使用设备 ...
 * 
 * // 停止
 * runner->stop();
 * @endcode
 */
class heartbeat_runner_t {
public:
    /**
     * @brief 构造函数
     * 
     * @param device 设备引用
     * @param strategy 心跳策略
     * @param config 心跳配置
     */
    heartbeat_runner_t(
        i_device_t& device,
        heartbeat_strategy_ptr_t strategy,
        const heartbeat_config_t& config = heartbeat_config_t()
    );

    /**
     * @brief 析构函数
     * 
     * 自动停止心跳线程（如果正在运行）。
     */
    ~heartbeat_runner_t();

    // ========================================================================
    // 控制接口
    // ========================================================================

    /**
     * @brief 启动心跳检测
     * 
     * @return 成功返回 ok，失败返回错误
     * 
     * @note 如果已经在运行，返回 error
     */
    result_t<void> start();

    /**
     * @brief 停止心跳检测
     * 
     * 等待后台线程完成并关闭。
     * 
     * @note 阻塞，直到线程完全停止
     */
    void stop();

    /**
     * @brief 暂停心跳检测
     * 
     * 暂停后不再发送心跳，但线程仍在运行。
     * 调用 resume() 可以恢复。
     */
    void pause();

    /**
     * @brief 恢复心跳检测
     * 
     * 从暂停状态恢复心跳。
     */
    void resume();

    /**
     * @brief 检查心跳是否正在运行
     */
    bool is_running() const {
        return m_running.load();
    }

    /**
     * @brief 检查心跳是否暂停
     */
    bool is_paused() const {
        return m_paused.load();
    }

    // ========================================================================
    // 配置接口
    // ========================================================================

    /**
     * @brief 获取当前配置
     */
    const heartbeat_config_t& config() const {
        return m_config;
    }

    /**
     * @brief 设置配置
     * 
     * @note 某些配置可能在运行中无法改变
     */
    void set_config(const heartbeat_config_t& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config = config;
    }

    /**
     * @brief 设置心跳间隔
     */
    void set_interval(milliseconds_t interval) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config.interval = interval;
    }

    /**
     * @brief 设置心跳超时
     */
    void set_timeout(milliseconds_t timeout) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config.timeout = timeout;
    }

    /**
     * @brief 设置最大失败次数
     */
    void set_max_failures(uint8_t max_failures) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config.max_failures = max_failures;
    }

    // ========================================================================
    // 状态查询
    // ========================================================================

    /**
     * @brief 获取当前失败次数
     */
    uint8_t failure_count() const {
        return m_failure_count.load();
    }

    /**
     * @brief 获取最后一次失败的错误
     */
    error_t last_error() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_last_error;
    }

    /**
     * @brief 获取总成功次数
     */
    uint64_t success_count() const {
        return m_success_count.load();
    }

    /**
     * @brief 获取总失败次数
     */
    uint64_t total_failures() const {
        return m_total_failures.load();
    }

    /**
     * @brief 重置计数器
     */
    void reset_counters() {
        m_failure_count.store(0);
        m_success_count.store(0);
        m_total_failures.store(0);
    }

    /**
     * @brief 重置失败计数器
     */
    void reset_failure_count() {
        m_failure_count.store(0);
    }

    // ========================================================================
    // 回调接口
    // ========================================================================

    /**
     * @brief 设置事件回调
     * 
     * @param callback 回调函数
     * 
     * @note 回调函数在心跳线程中执行，应该快速完成
     */
    void set_callback(heartbeat_callback_t callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callback = callback;
    }

    /**
     * @brief 移除回调
     */
    void clear_callback() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callback = nullptr;
    }

    /**
     * @brief 获取策略名称
     */
    const char* strategy_name() const {
        return m_strategy ? m_strategy->name() : "unknown";
    }

private:
    // ========================================================================
    // 后台线程逻辑
    // ========================================================================

    /**
     * @brief 运行循环（后台线程）
     */
    void _run_loop();

    /**
     * @brief 执行一次心跳检测
     * 
     * @return true 表示成功，false 表示失败
     */
    bool _do_heartbeat();

    /**
     * @brief 触发事件回调
     */
    void _trigger_callback(
        heartbeat_event_t event,
        uint8_t failure_count,
        const error_t& error
    );

    // ========================================================================
    // 成员变量
    // ========================================================================

    i_device_t& m_device;
    heartbeat_strategy_ptr_t m_strategy;
    heartbeat_config_t m_config;
    heartbeat_callback_t m_callback;

    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_paused{false};
    std::atomic<bool> m_stop_requested{false};

    std::atomic<uint8_t> m_failure_count{0};
    std::atomic<uint64_t> m_success_count{0};
    std::atomic<uint64_t> m_total_failures{0};

    error_t m_last_error;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
};

}  // namespace vdl

#endif  // VDL_HEARTBEAT_HEARTBEAT_RUNNER_HPP
