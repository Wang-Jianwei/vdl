/**
 * @file heartbeat_runner.cpp
 * @brief 心跳运行器实现
 */

#include "vdl/heartbeat/heartbeat_runner.hpp"
#include "vdl/core/logging.hpp"
#include "vdl/core/memory.hpp"

namespace vdl {

// ============================================================================
// heartbeat_runner_t 实现
// ============================================================================

heartbeat_runner_t::heartbeat_runner_t(
    i_device_t& device,
    heartbeat_strategy_ptr_t strategy,
    const heartbeat_config_t& config
)
    : m_device(device)
    , m_strategy(std::move(strategy))
    , m_config(config)
    , m_last_error(error_code_t::ok, "")
{
}

heartbeat_runner_t::~heartbeat_runner_t() {
    stop();
}

result_t<void> heartbeat_runner_t::start() {
    if (m_running.load()) {
        return make_error_void(error_code_t::device_error, 
                              "Heartbeat already running");
    }

    if (!m_strategy) {
        return make_error_void(error_code_t::device_error, 
                              "No heartbeat strategy configured");
    }

    m_running.store(true);
    m_paused.store(false);
    m_stop_requested.store(false);
    
    m_thread = make_unique<std::thread>(&heartbeat_runner_t::_run_loop, this);
    
    VDL_LOG_INFO("Heartbeat started with strategy: %s", strategy_name());
    
    return make_ok();
}

void heartbeat_runner_t::stop() {
    if (!m_running.load()) {
        return;
    }

    m_stop_requested.store(true);
    m_cv.notify_all();

    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }

    m_running.store(false);
    m_paused.store(false);
    m_thread.reset();

    VDL_LOG_INFO("Heartbeat stopped (strategy: %s)", strategy_name());
    
    _trigger_callback(heartbeat_event_t::stopped, m_failure_count.load(), 
                     error_t(error_code_t::ok, "Heartbeat stopped"));
}

void heartbeat_runner_t::pause() {
    if (!m_running.load() || m_paused.load()) {
        return;
    }

    m_paused.store(true);

    VDL_LOG_INFO("Heartbeat paused (strategy: %s)", strategy_name());
    _trigger_callback(heartbeat_event_t::paused, m_failure_count.load(),
                     error_t(error_code_t::ok, "Heartbeat paused"));
}

void heartbeat_runner_t::resume() {
    if (!m_running.load() || !m_paused.load()) {
        return;
    }

    m_paused.store(false);
    m_cv.notify_all();

    VDL_LOG_INFO("Heartbeat resumed (strategy: %s)", strategy_name());
    _trigger_callback(heartbeat_event_t::resumed, m_failure_count.load(),
                     error_t(error_code_t::ok, "Heartbeat resumed"));
}

void heartbeat_runner_t::_run_loop() {
    while (!m_stop_requested.load()) {
        // 检查是否暂停
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] { return !m_paused.load() || m_stop_requested.load(); });
        }

        if (m_stop_requested.load()) {
            break;
        }

        // 获取当前配置的副本
        heartbeat_config_t config;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            config = m_config;
        }

        // 执行心跳
        bool success = _do_heartbeat();

        if (success) {
            m_success_count.fetch_add(1);
            
            // 重置失败计数
            if (config.auto_reset_failures) {
                m_failure_count.store(0);
            }

            _trigger_callback(heartbeat_event_t::success, 0,
                             error_t(error_code_t::ok, "Heartbeat success"));
        } else {
            m_failure_count.fetch_add(1);
            m_total_failures.fetch_add(1);

            uint8_t failures = m_failure_count.load();

            // 检查是否达到最大失败次数
            if (failures >= config.max_failures) {
                _trigger_callback(heartbeat_event_t::max_failures, failures, m_last_error);
                
                // 重置计数以允许新一轮重试
                if (config.auto_reset_failures) {
                    m_failure_count.store(0);
                }
            } else {
                _trigger_callback(heartbeat_event_t::failure, failures, m_last_error);
            }
        }

        // 等待直到下一个心跳时间或被停止
        if (!m_stop_requested.load()) {
            std::unique_lock<std::mutex> lock(m_mutex);
            // 使用 wait_for 等待指定时间或被唤醒
            m_cv.wait_for(lock, std::chrono::milliseconds(config.interval), 
                         [this] { return m_stop_requested.load(); });
        }
    }

    VDL_LOG_INFO("Heartbeat thread exiting (strategy: %s)", strategy_name());
}

bool heartbeat_runner_t::_do_heartbeat() {
    // 检查设备是否已连接
    if (!m_device.is_connected()) {
        m_last_error = error_t(error_code_t::not_connected, 
                              "Device not connected");
        return false;
    }

    // 获取配置的副本
    heartbeat_config_t config;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        config = m_config;
    }

    // 生成心跳命令
    auto cmd_result = m_strategy->make_heartbeat_command();
    if (!cmd_result) {
        m_last_error = cmd_result.error();
        return false;
    }

    // 执行命令
    auto exec_result = m_device.execute(*cmd_result, config.timeout);
    if (!exec_result) {
        m_last_error = exec_result.error();
        return false;
    }

    // 验证响应
    bool validated = m_strategy->validate_response(*exec_result);
    if (!validated) {
        m_last_error = error_t(error_code_t::device_error, 
                              "Heartbeat response validation failed");
        return false;
    }

    return true;
}

void heartbeat_runner_t::_trigger_callback(
    heartbeat_event_t event,
    uint8_t failure_count,
    const error_t& error
) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_callback) {
        try {
            m_callback(event, failure_count, error);
        } catch (const std::exception& e) {
            VDL_LOG_WARN("Heartbeat callback threw exception: %s", e.what());
        }
    }
}

}  // namespace vdl
