/**
 * @file mock_transport.hpp
 * @brief 模拟传输层（用于测试）
 */

#ifndef VDL_TRANSPORT_MOCK_TRANSPORT_HPP
#define VDL_TRANSPORT_MOCK_TRANSPORT_HPP

#include "transport.hpp"
#include "../core/buffer.hpp"

#include <algorithm>

namespace vdl {

// ============================================================================
// mock_transport_t - 模拟传输层
// ============================================================================

/**
 * @brief 模拟传输层，用于单元测试
 * 
 * @code
 * mock_transport_t transport;
 * 
 * // 预设响应数据
 * transport.set_response({0x01, 0x02, 0x03});
 * 
 * // 读取预设数据
 * bytes_t buffer(3);
 * auto result = transport.read(make_span(buffer));
 * @endcode
 */
class mock_transport_t : public transport_base_t {
public:
    mock_transport_t() = default;

    // ========================================================================
    // i_transport_t 实现
    // ========================================================================

    result_t<void> open() override {
        ++m_open_count;

        if (m_fail_open_times > 0) {
            --m_fail_open_times;
            return make_error_void(error_code_t::connection_failed,
                                   "Mock: simulated transient open failure");
        }

        if (m_should_fail_open) {
            return make_error_void(error_code_t::connection_failed, 
                                   "Mock: simulated open failure");
        }
        m_is_open = true;
        return make_ok();
    }

    void close() override {
        m_is_open = false;
    }

    bool is_open() const override {
        return m_is_open;
    }

    result_t<size_t> read(byte_span_t buffer, 
                          milliseconds_t /*timeout_ms*/ = 0) override {
        if (!m_is_open) {
            return make_error<size_t>(error_code_t::not_connected, 
                                      "Mock: not connected");
        }
        
        if (m_fail_read_times > 0) {
            --m_fail_read_times;
            return make_error<size_t>(error_code_t::read_failed,
                                      "Mock: simulated transient read failure");
        }

        if (m_should_fail_read) {
            return make_error<size_t>(error_code_t::read_failed,
                                      "Mock: simulated read failure");
        }

        if (m_read_buffer.empty()) {
            return make_error<size_t>(error_code_t::timeout,
                                      "Mock: no data available");
        }

        // 从读缓冲区复制数据
        size_t to_read = std::min(buffer.size(), m_read_buffer.readable_size());
        size_t read_count = m_read_buffer.read(buffer.data(), to_read);

        return read_count;
    }

    result_t<size_t> write(const_byte_span_t data,
                           milliseconds_t /*timeout_ms*/ = 0) override {
        if (!m_is_open) {
            return make_error<size_t>(error_code_t::not_connected,
                                      "Mock: not connected");
        }

        if (m_fail_write_times > 0) {
            --m_fail_write_times;
            return make_error<size_t>(error_code_t::write_failed,
                                      "Mock: simulated transient write failure");
        }

        if (m_should_fail_write) {
            return make_error<size_t>(error_code_t::write_failed,
                                      "Mock: simulated write failure");
        }

        // 保存写入的数据
        for (size_t i = 0; i < data.size(); ++i) {
            m_write_buffer.push_back(data[i]);
        }

        return data.size();
    }

    void flush_read() override {
        m_read_buffer.clear();
    }

    const char* type_name() const override {
        return "mock";
    }

    // ========================================================================
    // 测试辅助方法
    // ========================================================================

    /**
     * @brief 设置响应数据（将被读取）
     */
    void set_response(const bytes_t& data) {
        m_read_buffer.write(data.data(), data.size());
    }

    void set_response(std::initializer_list<byte_t> data) {
        for (auto b : data) {
            m_read_buffer.push(b);
        }
    }

    /**
     * @brief 获取已写入的数据
     */
    bytes_t get_written_data() const {
        return m_write_buffer;
    }

    /**
     * @brief 清空已写入的数据
     */
    void clear_written_data() {
        m_write_buffer.clear();
    }

    /**
     * @brief 设置打开时是否失败
     */
    void set_fail_open(bool fail) {
        m_should_fail_open = fail;
    }

    /**
     * @brief 设置打开失败的次数（用于模拟瞬时失败）
     */
    void set_fail_open_times(uint32_t times) {
        m_fail_open_times = times;
    }

    /**
     * @brief 获取 open 调用次数
     */
    uint32_t open_count() const {
        return m_open_count;
    }

    /**
     * @brief 设置读取时是否失败
     */
    void set_fail_read(bool fail) {
        m_should_fail_read = fail;
    }

    /**
     * @brief 设置读取失败的次数（用于模拟瞬时失败）
     */
    void set_fail_read_times(uint32_t times) {
        m_fail_read_times = times;
    }

    /**
     * @brief 设置写入时是否失败
     */
    void set_fail_write(bool fail) {
        m_should_fail_write = fail;
    }

    /**
     * @brief 设置写入失败的次数（用于模拟瞬时失败）
     */
    void set_fail_write_times(uint32_t times) {
        m_fail_write_times = times;
    }

private:
    bool m_is_open = false;
    bool m_should_fail_open = false;
    bool m_should_fail_read = false;
    bool m_should_fail_write = false;

    uint32_t m_open_count = 0;
    uint32_t m_fail_open_times = 0;

    uint32_t m_fail_read_times = 0;
    uint32_t m_fail_write_times = 0;

    ring_buffer_t m_read_buffer{4096};
    bytes_t m_write_buffer;
};

}  // namespace vdl

#endif  // VDL_TRANSPORT_MOCK_TRANSPORT_HPP
