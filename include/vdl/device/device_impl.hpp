/**
 * @file device_impl.hpp
 * @brief 设备实现基类
 * 
 * 提供设备接口的通用实现。
 */

#ifndef VDL_DEVICE_DEVICE_IMPL_HPP
#define VDL_DEVICE_DEVICE_IMPL_HPP

#include "device.hpp"
#include "../transport/transport.hpp"
#include "../codec/codec.hpp"
#include "../core/buffer.hpp"
#include "../core/logging.hpp"

#include <algorithm>
#include <thread>
#include <chrono>

namespace vdl {

// ============================================================================
// device_impl_t - 设备实现基类
// ============================================================================

/**
 * @brief 设备实现基类
 * 
 * 通过组合 Transport 和 Codec 实现设备接口。
 * 
 * @code
 * // 创建设备
 * auto transport = std::make_unique<tcp_transport_t>("192.168.1.100", 502);
 * auto codec = std::make_unique<modbus_codec_t>();
 * 
 * device_impl_t device(std::move(transport), std::move(codec));
 * device.connect();
 * 
 * auto result = device.execute(make_read_command(0x03, 0x0000, 10));
 * @endcode
 */
class device_impl_t : public i_device_t {
public:
    /**
     * @brief 构造函数
     * @param transport 传输层实现
     * @param codec 编解码器实现
     */
    device_impl_t(transport_ptr_t transport, codec_ptr_t codec)
        : m_transport(std::move(transport))
        , m_codec(std::move(codec))
        , m_state(device_state_t::disconnected) {
    }

    ~device_impl_t() override {
        disconnect();
    }

    // ========================================================================
    // i_device_t 实现 - 连接管理
    // ========================================================================

    result_t<void> connect() override {
        if (m_state == device_state_t::connected) {
            return make_ok();
        }

        m_state = device_state_t::connecting;

        auto result = m_transport->open();
        if (!result) {
            m_state = device_state_t::error;
            return result;
        }

        m_state = device_state_t::connected;
        VDL_LOG_INFO("Device connected via %s", m_transport->type_name());

        return make_ok();
    }

    void disconnect() override {
        if (m_transport) {
            m_transport->close();
        }
        m_state = device_state_t::disconnected;
    }

    device_state_t state() const override {
        return m_state;
    }

    bool is_connected() const override {
        return m_state == device_state_t::connected && 
               m_transport && m_transport->is_open();
    }

    // ========================================================================
    // i_device_t 实现 - 命令执行
    // ========================================================================

    result_t<response_t> execute(const command_t& cmd) override {
        return execute(cmd, m_config.command_timeout);
    }

    result_t<response_t> execute(const command_t& cmd,
                                  milliseconds_t timeout_ms) override {
        if (!is_connected()) {
            return make_error<response_t>(error_code_t::not_connected, 
                                        "Device not connected");
        }

        auto encode_result = m_codec->encode(cmd);
        if (!encode_result) {
            return make_unexpected(encode_result.error());
        }

        const auto& frame = *encode_result;
        const_byte_span_t frame_span(frame.data(), frame.size());

        const uint8_t max_attempts = std::max<uint8_t>(1, m_config.max_retries);
        error_t last_error{error_code_t::ok};

        for (uint8_t attempt = 0; attempt < max_attempts; ++attempt) {
            auto write_result = m_transport->write_all(frame_span, timeout_ms);
            if (!write_result) {
                last_error = write_result.error();
            } else {
                auto read_result = _read_response(timeout_ms, /*handle_error_on_fail=*/false);
                if (read_result) {
                    return read_result;
                }
                last_error = read_result.error();
            }

            const bool has_more = (static_cast<uint8_t>(attempt + 1)) < max_attempts;
            if (!has_more) {
                break;
            }

            if (m_config.retry_delay > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_config.retry_delay));
            }
        }

        _handle_error(last_error);
        return make_unexpected(last_error);
    }

    // ========================================================================
    // i_device_t 实现 - 设备信息
    // ========================================================================

    const device_info_t& info() const override {
        return m_info;
    }

    const device_config_t& config() const override {
        return m_config;
    }

    void set_config(const device_config_t& config) override {
        m_config = config;
    }

    const char* type_name() const override {
        return "device_impl";
    }

    // ========================================================================
    // 扩展接口
    // ========================================================================

    /**
     * @brief 设置设备信息
     */
    void set_info(const device_info_t& info) {
        m_info = info;
    }

    /**
     * @brief 设置重连回调
     * @param callback 回调函数，接收重连事件、当前尝试次数、最大尝试次数和错误信息
     * 
     * @code
     * device.set_reconnect_callback([](reconnect_event_t event, uint8_t attempt, 
     *                                   uint8_t max_attempts, const error_t& err) {
     *     switch (event) {
     *         case reconnect_event_t::started:
     *             std::cout << "开始重连..." << std::endl;
     *             break;
     *         case reconnect_event_t::attempting:
     *             std::cout << "重连尝试 " << (int)attempt << "/" << (int)max_attempts << std::endl;
     *             break;
     *         case reconnect_event_t::success:
     *             std::cout << "重连成功！" << std::endl;
     *             break;
     *         case reconnect_event_t::failed:
     *             std::cout << "重连失败: " << err.message() << std::endl;
     *             break;
     *     }
     * });
     * @endcode
     */
    void set_reconnect_callback(reconnect_callback_t callback) {
        m_reconnect_callback = std::move(callback);
    }

    /**
     * @brief 手动触发重连
     * @return 成功返回 ok，失败返回错误
     */
    result_t<void> reconnect() {
        if (m_state == device_state_t::connected) {
            return make_ok();
        }

        error_t trigger_error(error_code_t::not_connected, "Manual reconnect requested");
        _try_reconnect(trigger_error);

        if (m_state == device_state_t::connected) {
            return make_ok();
        }
        return make_error_void(error_code_t::connection_failed, "Reconnect failed");
    }

    /**
     * @brief 获取传输层
     */
    i_transport_t* transport() {
        return m_transport.get();
    }

    const i_transport_t* transport() const {
        return m_transport.get();
    }

    /**
     * @brief 获取编解码器
     */
    i_codec_t* codec() {
        return m_codec.get();
    }

    const i_codec_t* codec() const {
        return m_codec.get();
    }

    // ========================================================================
    // 便捷接口 - 原始数据读写（低层 API）
    // ========================================================================

    /**
     * @brief 写入原始数据（绕过 codec）
     * @param data 数据
     * @param timeout_ms 超时时间
     * @return 成功返回 ok，失败返回错误
     */
    result_t<void> write_raw(const_byte_span_t data, 
                             milliseconds_t timeout_ms = 0) {
        if (!is_connected()) {
            return make_error_void(error_code_t::not_connected, 
                                  "Device not connected");
        }

        if (timeout_ms == 0) {
            timeout_ms = m_config.command_timeout;
        }

        return m_transport->write_all(data, timeout_ms);
    }

    /**
     * @brief 读取原始数据（绕过 codec）
     * @param max_bytes 最大读取字节数
     * @param timeout_ms 超时时间
     * @return 成功返回数据，失败返回错误
     */
    result_t<bytes_t> read_raw(size_t max_bytes, 
                               milliseconds_t timeout_ms = 0) {
        if (!is_connected()) {
            return make_error<bytes_t>(error_code_t::not_connected);
        }

        if (timeout_ms == 0) {
            timeout_ms = m_config.command_timeout;
        }

        bytes_t buffer(max_bytes);
        byte_span_t span(buffer.data(), buffer.size());
        auto result = m_transport->read(span, timeout_ms);
        if (!result) {
            return make_unexpected(result.error());
        }

        buffer.resize(*result);
        return make_ok(std::move(buffer));
    }

    // ========================================================================
    // 便捷接口 - 字符串读写（适用于文本协议如 SCPI）
    // ========================================================================

    /**
     * @brief 写入文本命令（不等待响应）
     * @param text 命令文本
     * @return 成功返回 ok，失败返回错误
     */
    result_t<void> write(const std::string& text) {
        if (!is_connected()) {
            return make_error_void(error_code_t::not_connected);
        }

        bytes_t data(text.begin(), text.end());
        const_byte_span_t span(data.data(), data.size());
        return write_raw(span, m_config.command_timeout);
    }

    /**
     * @brief 读取文本响应
     * @param timeout_ms 超时时间
     * @return 成功返回响应文本，失败返回错误
     */
    result_t<std::string> read(milliseconds_t timeout_ms = 0) {
        if (!is_connected()) {
            return make_error<std::string>(error_code_t::not_connected);
        }

        if (timeout_ms == 0) {
            timeout_ms = m_config.command_timeout;
        }

        // 读取数据直到看到换行符或超时
        ring_buffer_t buffer(m_codec->max_frame_size());
        bytes_t temp(1024);
        bytes_t data_copy;

        while (true) {
            // 检查是否有完整行（以 \n 结尾）
            size_t data_size = buffer.readable_size();
            if (data_size > 0) {
                data_copy.resize(data_size);
                buffer.peek(data_copy.data(), data_size);
                
                // 查找换行符
                auto* newline_pos = static_cast<byte_t*>(
                    memchr(data_copy.data(), '\n', data_size));
                if (newline_pos != nullptr) {
                    size_t line_len = static_cast<size_t>(
                        newline_pos - data_copy.data()) + 1;
                    
                    // 提取行数据
                    std::string result;
                    for (size_t i = 0; i < line_len; ++i) {
                        byte_t c = data_copy[i];
                        if (c != '\r' && c != '\n') {
                            result.push_back(static_cast<char>(c));
                        }
                    }
                    
                    buffer.consume(line_len);
                    return make_ok(std::move(result));
                }
            }

            // 读取更多数据
            byte_span_t temp_span(temp.data(), temp.size());
            auto read_result = m_transport->read(temp_span, timeout_ms);
            if (!read_result) {
                return make_unexpected(read_result.error());
            }

            size_t bytes_read = *read_result;
            if (bytes_read == 0) {
                return make_error<std::string>(error_code_t::timeout);
            }

            buffer.write(temp.data(), bytes_read);

            // 防止缓冲区溢出
            if (buffer.readable_size() > m_codec->max_frame_size()) {
                return make_error<std::string>(error_code_t::frame_too_large);
            }
        }
    }

    /**
     * @brief 查询（发送命令并读取响应）
     * @param command 命令文本（不需要终止符）
     * @param timeout_ms 超时时间
     * @return 成功返回响应文本，失败返回错误
     */
    result_t<std::string> query(const std::string& command,
                                milliseconds_t timeout_ms = 0) {
        if (!is_connected()) {
            return make_error<std::string>(error_code_t::not_connected);
        }

        if (timeout_ms == 0) {
            timeout_ms = m_config.command_timeout;
        }

        // 发送命令（确保以换行符结尾）
        std::string cmd_with_newline = command;
        if (cmd_with_newline.empty() || cmd_with_newline.back() != '\n') {
            cmd_with_newline.push_back('\n');
        }

        auto write_result = write(cmd_with_newline);
        if (!write_result) {
            return make_unexpected(write_result.error());
        }

        // 读取响应
        return read(timeout_ms);
    }

protected:
    /**
     * @brief 读取响应（私有实现）
     */
    result_t<response_t> _read_response(milliseconds_t timeout_ms,
                                        bool handle_error_on_fail = true) {
        ring_buffer_t buffer(m_codec->max_frame_size());
        bytes_t temp(1024);
        bytes_t data_copy;  // 用于存储 peek 的数据

        // 循环读取直到获得完整帧
        while (true) {
            // 检查是否有完整帧
            size_t data_size = buffer.readable_size();
            if (data_size > 0) {
                data_copy.resize(data_size);
                buffer.peek(data_copy.data(), data_size);
                
                const_byte_span_t data_span(data_copy.data(), data_size);
                size_t frame_len = m_codec->frame_length(data_span);
                
                if (frame_len > 0 && frame_len <= data_size) {
                    // 解码响应
                    size_t consumed = 0;
                    const_byte_span_t frame_span(data_copy.data(), frame_len);
                    auto decode_result = m_codec->decode(frame_span, consumed);
                    
                    buffer.consume(consumed);
                    return decode_result;
                }
            }

            // 读取更多数据
            byte_span_t temp_span(temp.data(), temp.size());
            auto read_result = m_transport->read(temp_span, timeout_ms);
            if (!read_result) {
                if (handle_error_on_fail) {
                    _handle_error(read_result.error());
                }
                return make_unexpected(read_result.error());
            }

            size_t bytes_read = *read_result;
            if (bytes_read == 0) {
                return make_error<response_t>(error_code_t::timeout, "Read timeout");
            }

            buffer.write(temp.data(), bytes_read);

            // 防止缓冲区溢出
            if (buffer.readable_size() > m_codec->max_frame_size()) {
                return make_error<response_t>(error_code_t::frame_too_large, 
                                              "Frame exceeds maximum size");
            }
        }
    }

    /**
     * @brief 处理错误（私有实现）
     * @param error 触发错误
     */
    void _handle_error(const error_t& error) {
        // 判断是否需要自动重连
        if (!m_config.auto_reconnect) {
            disconnect();
            return;
        }

        // 判断错误类型是否可恢复
        if (!_is_recoverable_error(error)) {
            VDL_LOG_WARN("Non-recoverable error, disconnecting: %s", error.message().c_str());
            disconnect();
            return;
        }

        _try_reconnect(error);
    }

    /**
     * @brief 判断错误是否可恢复（值得重连）
     */
    bool _is_recoverable_error(const error_t& error) const {
        switch (error.code()) {
            // 传输层瞬时错误 - 可恢复
            case error_code_t::read_failed:
            case error_code_t::write_failed:
            case error_code_t::connection_closed:
            case error_code_t::not_connected:
            case error_code_t::io_error:
                return true;

            // 超时 - 根据配置决定
            case error_code_t::timeout:
                return m_config.reconnect_on_timeout;

            // 协议错误、参数错误等 - 不可恢复
            case error_code_t::protocol_error:
            case error_code_t::invalid_frame:
            case error_code_t::checksum_error:
            case error_code_t::encode_failed:
            case error_code_t::decode_failed:
            case error_code_t::invalid_argument:
            case error_code_t::device_error:
            default:
                return false;
        }
    }

    /**
     * @brief 尝试重连（带退避策略）
     */
    void _try_reconnect(const error_t& trigger_error) {
        m_state = device_state_t::reconnecting;

        const uint8_t max_attempts = std::max<uint8_t>(1, m_config.max_retries);
        milliseconds_t current_delay = m_config.reconnect_delay;

        // 通知开始重连
        _trigger_reconnect_callback(reconnect_event_t::started, 0, max_attempts, trigger_error);
        VDL_LOG_INFO("Starting auto-reconnect (max %u attempts)", max_attempts);

        for (uint8_t attempt = 0; attempt < max_attempts; ++attempt) {
            // 通知正在尝试
            _trigger_reconnect_callback(reconnect_event_t::attempting, static_cast<uint8_t>(attempt + 1), max_attempts, trigger_error);

            // 关闭当前连接
            if (m_transport->is_open()) {
                m_transport->close();
            }

            // 尝试重新打开
            auto open_result = m_transport->open();
            if (open_result) {
                m_state = device_state_t::connected;
                VDL_LOG_INFO("Auto-reconnect succeeded on attempt %u", static_cast<unsigned>(attempt + 1));
                _trigger_reconnect_callback(reconnect_event_t::success, static_cast<uint8_t>(attempt + 1), max_attempts, 
                                           error_t(error_code_t::ok, "Reconnected"));
                return;
            }

            // 检查是否还有更多尝试
            const bool has_more = (static_cast<uint8_t>(attempt + 1)) < max_attempts;
            if (!has_more) {
                break;
            }

            // 等待（使用退避策略）
            if (current_delay > 0) {
                VDL_LOG_DEBUG("Reconnect attempt %u failed, waiting %u ms before retry",
                             static_cast<unsigned>(attempt + 1), static_cast<unsigned>(current_delay));
                std::this_thread::sleep_for(std::chrono::milliseconds(current_delay));
            }

            // 计算下一次延迟（指数退避）
            milliseconds_t next_delay = static_cast<milliseconds_t>(
                static_cast<float>(current_delay) * m_config.backoff_multiplier);
            current_delay = std::min(next_delay, m_config.max_reconnect_delay);
        }

        // 所有尝试都失败
        m_state = device_state_t::error;
        VDL_LOG_ERROR("Auto-reconnect failed after %u attempts", max_attempts);
        _trigger_reconnect_callback(reconnect_event_t::failed, max_attempts, max_attempts, trigger_error);
    }

    /**
     * @brief 触发重连回调
     */
    void _trigger_reconnect_callback(reconnect_event_t event,
                                     uint8_t attempt,
                                     uint8_t max_attempts,
                                     const error_t& error) {
        if (m_reconnect_callback) {
            try {
                m_reconnect_callback(event, attempt, max_attempts, error);
            } catch (const std::exception& e) {
                VDL_LOG_WARN("Reconnect callback threw exception: %s", e.what());
            }
        }
    }

protected:
    transport_ptr_t m_transport;
    codec_ptr_t m_codec;
    device_state_t m_state;
    device_info_t m_info;
    device_config_t m_config;
    reconnect_callback_t m_reconnect_callback;
};

}  // namespace vdl

#endif  // VDL_DEVICE_DEVICE_IMPL_HPP
