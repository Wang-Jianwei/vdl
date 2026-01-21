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

        // 编码命令
        auto encode_result = m_codec->encode(cmd);
        if (!encode_result) {
            return make_unexpected(encode_result.error());
        }

        // 发送数据
        auto& frame = *encode_result;
        const_byte_span_t frame_span(frame.data(), frame.size());
        auto write_result = m_transport->write_all(frame_span, timeout_ms);
        if (!write_result) {
            _handle_error();
            return make_unexpected(write_result.error());
        }

        // 接收响应
        return _read_response(timeout_ms);
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

protected:
    /**
     * @brief 读取响应（私有实现）
     */
    result_t<response_t> _read_response(milliseconds_t timeout_ms) {
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
                _handle_error();
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
     */
    void _handle_error() {
        if (m_config.auto_reconnect) {
            m_state = device_state_t::error;
        } else {
            disconnect();
        }
    }

protected:
    transport_ptr_t m_transport;
    codec_ptr_t m_codec;
    device_state_t m_state;
    device_info_t m_info;
    device_config_t m_config;
};

}  // namespace vdl

#endif  // VDL_DEVICE_DEVICE_IMPL_HPP
