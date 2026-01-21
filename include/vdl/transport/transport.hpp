/**
 * @file transport.hpp
 * @brief 传输层接口
 * 
 * 定义传输层抽象接口，用于底层 I/O 操作。
 */

#ifndef VDL_TRANSPORT_TRANSPORT_HPP
#define VDL_TRANSPORT_TRANSPORT_HPP

#include "../core/types.hpp"
#include "../core/error.hpp"
#include "../core/noncopyable.hpp"

#include <string>

namespace vdl {

// ============================================================================
// 传输层配置
// ============================================================================

/**
 * @brief 传输层配置
 */
struct transport_config_t {
    milliseconds_t connect_timeout = 5000;     ///< 连接超时
    milliseconds_t read_timeout = 1000;        ///< 读取超时
    milliseconds_t write_timeout = 1000;       ///< 写入超时
    size_t read_buffer_size = 4096;            ///< 读缓冲区大小
    size_t write_buffer_size = 4096;           ///< 写缓冲区大小
};

// ============================================================================
// i_transport_t - 传输层接口
// ============================================================================

/**
 * @brief 传输层抽象接口
 * 
 * 定义底层 I/O 操作的抽象接口。
 * 
 * 实现类：
 * - tcp_transport_t: TCP 传输
 * - serial_transport_t: 串口传输
 * - mock_transport_t: 模拟传输（测试用）
 */
class i_transport_t : private noncopyable_t {
public:
    virtual ~i_transport_t() = default;

    // ========================================================================
    // 连接管理
    // ========================================================================

    /**
     * @brief 打开连接
     * @return 成功返回 ok，失败返回错误
     */
    virtual result_t<void> open() = 0;

    /**
     * @brief 关闭连接
     */
    virtual void close() = 0;

    /**
     * @brief 检查是否已连接
     */
    virtual bool is_open() const = 0;

    // ========================================================================
    // 读写操作
    // ========================================================================

    /**
     * @brief 读取数据
     * @param buffer 接收缓冲区
     * @param timeout_ms 超时时间（毫秒），0 表示使用默认超时
     * @return 成功返回实际读取的字节数，失败返回错误
     */
    virtual result_t<size_t> read(byte_span_t buffer, 
                                   milliseconds_t timeout_ms = 0) = 0;

    /**
     * @brief 写入数据
     * @param data 要发送的数据
     * @param timeout_ms 超时时间（毫秒），0 表示使用默认超时
     * @return 成功返回实际写入的字节数，失败返回错误
     */
    virtual result_t<size_t> write(const_byte_span_t data,
                                    milliseconds_t timeout_ms = 0) = 0;

    /**
     * @brief 写入所有数据
     * @param data 要发送的数据
     * @param timeout_ms 超时时间
     * @return 成功返回 ok，失败返回错误
     */
    virtual result_t<void> write_all(const_byte_span_t data,
                                      milliseconds_t timeout_ms = 0) {
        size_t total_written = 0;
        const size_t total = data.size();
        
        while (total_written < total) {
            auto result = write(data.subspan(total_written), timeout_ms);
            if (!result) {
                return make_error_void(result.error());
            }
            total_written += *result;
        }
        
        return make_ok();
    }

    /**
     * @brief 清空接收缓冲区
     */
    virtual void flush_read() {}

    /**
     * @brief 清空发送缓冲区
     */
    virtual void flush_write() {}

    // ========================================================================
    // 配置
    // ========================================================================

    /**
     * @brief 获取配置
     */
    virtual const transport_config_t& config() const = 0;

    /**
     * @brief 设置配置
     */
    virtual void set_config(const transport_config_t& config) = 0;

    /**
     * @brief 获取传输类型名称（用于日志）
     */
    virtual const char* type_name() const = 0;
};

// ============================================================================
// transport_ptr_t - 传输层智能指针
// ============================================================================

using transport_ptr_t = std::unique_ptr<i_transport_t>;

// ============================================================================
// 传输层基类 - 提供通用实现
// ============================================================================

/**
 * @brief 传输层基类，提供通用功能
 */
class transport_base_t : public i_transport_t {
public:
    const transport_config_t& config() const override {
        return m_config;
    }

    void set_config(const transport_config_t& config) override {
        m_config = config;
    }

protected:
    transport_config_t m_config;
};

}  // namespace vdl

#endif  // VDL_TRANSPORT_TRANSPORT_HPP
