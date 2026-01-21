/**
 * @file error.hpp
 * @brief VDL 错误处理系统
 * 
 * 提供统一的错误码和错误类型定义。
 */

#ifndef VDL_CORE_ERROR_HPP
#define VDL_CORE_ERROR_HPP

#include <vdl/core/types.hpp>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#include <string>
#include <sstream>

namespace vdl {

// ============================================================================
// 错误码定义
// ============================================================================

/**
 * @brief 错误码枚举
 */
enum class error_code_t : int {
    // 成功
    ok = 0,

    // 一般错误 (100-199)
    unknown = 100,
    invalid = 101,
    not_supported = 102,
    not_implemented = 103,

    // 内存错误 (200-299)
    out_of_memory = 200,
    allocation_failed = 201,
    invalid_pointer = 202,

    // 参数错误 (300-399)
    null_pointer = 300,
    invalid_argument = 301,
    out_of_range = 302,
    invalid_size = 303,

    // IO 错误 (400-499)
    io_error = 400,
    file_not_found = 401,
    file_access_denied = 402,
    invalid_format = 403,

    // 设备错误 (500-599)
    device_error = 500,
    device_not_available = 501,
    device_not_initialized = 502,
    device_already_open = 503,
    device_not_open = 504,

    // 超时和竞争 (600-699)
    timeout = 600,
    deadlock = 601,
    concurrency_error = 602,
    lock_conflict = 603,
    busy = 604,

    // 协议和通信 (700-799)
    protocol_error = 700,
    communication_error = 701,
    version_mismatch = 702,
    invalid_frame = 703,
    checksum_error = 704,
    encode_failed = 705,
    decode_failed = 706,
    incomplete_frame = 707,
    frame_too_large = 708,

    // 配置和初始化 (800-899)
    config_error = 800,
    initialization_failed = 801,
    not_initialized = 802,
    already_initialized = 803,

    // 业务逻辑 (900-999)
    invalid_state = 900,
    operation_failed = 901,
    operation_cancelled = 902,
    invalid_command = 903,

    // 传输层 (1000-1099)
    transport_error = 1000,
    connection_failed = 1001,
    connection_closed = 1002,
    read_error = 1003,
    write_error = 1004,
    address_invalid = 1005,
    not_connected = 1006,
    read_failed = 1007,
    write_failed = 1008
};

/**
 * @brief 错误类别枚举
 */
enum class error_category_t {
    none,           ///< 无错误
    general,        ///< 一般错误
    memory,         ///< 内存错误
    argument,       ///< 参数错误
    io,             ///< IO 错误
    device,         ///< 设备错误
    concurrency,    ///< 并发错误
    protocol,       ///< 协议错误
    config,         ///< 配置错误
    logic,          ///< 业务逻辑错误
    transport       ///< 传输层错误
};

// ============================================================================
// 错误码工具函数
// ============================================================================

/**
 * @brief 获取错误码对应的类别
 */
inline error_category_t get_error_category(error_code_t code) {
    int c = static_cast<int>(code);
    if (c == 0) return error_category_t::none;
    if (c >= 100 && c < 200) return error_category_t::general;
    if (c >= 200 && c < 300) return error_category_t::memory;
    if (c >= 300 && c < 400) return error_category_t::argument;
    if (c >= 400 && c < 500) return error_category_t::io;
    if (c >= 500 && c < 600) return error_category_t::device;
    if (c >= 600 && c < 700) return error_category_t::concurrency;
    if (c >= 700 && c < 800) return error_category_t::protocol;
    if (c >= 800 && c < 900) return error_category_t::config;
    if (c >= 900 && c < 1000) return error_category_t::logic;
    if (c >= 1000 && c < 1100) return error_category_t::transport;
    return error_category_t::general;
}

/**
 * @brief 获取错误码的字符串名称
 */
inline const char* get_error_name(error_code_t code) {
    switch (code) {
        case error_code_t::ok: return "ok";
        case error_code_t::unknown: return "unknown";
        case error_code_t::invalid: return "invalid";
        case error_code_t::not_supported: return "not_supported";
        case error_code_t::not_implemented: return "not_implemented";
        case error_code_t::out_of_memory: return "out_of_memory";
        case error_code_t::allocation_failed: return "allocation_failed";
        case error_code_t::invalid_pointer: return "invalid_pointer";
        case error_code_t::null_pointer: return "null_pointer";
        case error_code_t::invalid_argument: return "invalid_argument";
        case error_code_t::out_of_range: return "out_of_range";
        case error_code_t::invalid_size: return "invalid_size";
        case error_code_t::io_error: return "io_error";
        case error_code_t::file_not_found: return "file_not_found";
        case error_code_t::file_access_denied: return "file_access_denied";
        case error_code_t::invalid_format: return "invalid_format";
        case error_code_t::device_error: return "device_error";
        case error_code_t::device_not_available: return "device_not_available";
        case error_code_t::device_not_initialized: return "device_not_initialized";
        case error_code_t::device_already_open: return "device_already_open";
        case error_code_t::device_not_open: return "device_not_open";
        case error_code_t::timeout: return "timeout";
        case error_code_t::deadlock: return "deadlock";
        case error_code_t::concurrency_error: return "concurrency_error";
        case error_code_t::lock_conflict: return "lock_conflict";
        case error_code_t::busy: return "busy";
        case error_code_t::protocol_error: return "protocol_error";
        case error_code_t::communication_error: return "communication_error";
        case error_code_t::version_mismatch: return "version_mismatch";
        case error_code_t::invalid_frame: return "invalid_frame";
        case error_code_t::checksum_error: return "checksum_error";
        case error_code_t::encode_failed: return "encode_failed";
        case error_code_t::decode_failed: return "decode_failed";
        case error_code_t::incomplete_frame: return "incomplete_frame";
        case error_code_t::frame_too_large: return "frame_too_large";
        case error_code_t::config_error: return "config_error";
        case error_code_t::initialization_failed: return "initialization_failed";
        case error_code_t::not_initialized: return "not_initialized";
        case error_code_t::already_initialized: return "already_initialized";
        case error_code_t::invalid_state: return "invalid_state";
        case error_code_t::operation_failed: return "operation_failed";
        case error_code_t::operation_cancelled: return "operation_cancelled";
        case error_code_t::invalid_command: return "invalid_command";
        case error_code_t::transport_error: return "transport_error";
        case error_code_t::connection_failed: return "connection_failed";
        case error_code_t::connection_closed: return "connection_closed";
        case error_code_t::read_error: return "read_error";
        case error_code_t::write_error: return "write_error";
        case error_code_t::address_invalid: return "address_invalid";
        case error_code_t::not_connected: return "not_connected";
        case error_code_t::read_failed: return "read_failed";
        case error_code_t::write_failed: return "write_failed";
        default: return "unknown_error";
    }
}

// ============================================================================
// 错误类
// ============================================================================

/**
 * @brief 错误类
 * 
 * 包含错误码、消息和上下文信息。
 */
class error_t {
public:
    /**
     * @brief 默认构造（成功状态）
     */
    error_t() : m_code(error_code_t::ok) {}

    /**
     * @brief 从错误码构造
     */
    explicit error_t(error_code_t code, const std::string& message = "")
        : m_code(code)
        , m_message(message) {}

    /**
     * @brief 从错误码和消息构造
     */
    error_t(error_code_t code, const char* message)
        : m_code(code)
        , m_message(message ? message : "") {}

    // 访问器
    error_code_t code() const { return m_code; }
    error_category_t category() const { return get_error_category(m_code); }
    const std::string& message() const { return m_message; }
    const std::string& context() const { return m_context; }

    /**
     * @brief 添加上下文信息
     */
    error_t& with_context(const std::string& ctx) {
        if (!m_context.empty()) {
            m_context += " <- ";
        }
        m_context += ctx;
        return *this;
    }

    /**
     * @brief 检查是否为成功状态
     */
    bool is_ok() const { return m_code == error_code_t::ok; }

    /**
     * @brief 检查是否为错误状态
     */
    bool is_error() const { return m_code != error_code_t::ok; }

    /**
     * @brief 转换为布尔值（true 表示有错误）
     */
    explicit operator bool() const { return is_error(); }

    /**
     * @brief 转换为字符串
     */
    std::string to_string() const {
        std::ostringstream oss;
        oss << get_error_name(m_code) << "(" << static_cast<int>(m_code) << ")";
        if (!m_message.empty()) {
            oss << ": " << m_message;
        }
        if (!m_context.empty()) {
            oss << " [" << m_context << "]";
        }
        return oss.str();
    }

    // 便捷构造
    static error_t make(error_code_t code, const std::string& msg = "") {
        return error_t(code, msg);
    }

    static error_t transport(error_code_t code, const std::string& msg) {
        return error_t(code, msg);
    }

    static error_t device(error_code_t code, const std::string& msg) {
        return error_t(code, msg);
    }

    static error_t protocol(error_code_t code, const std::string& msg) {
        return error_t(code, msg);
    }

private:
    error_code_t m_code;
    std::string m_message;
    std::string m_context;
};

// ============================================================================
// Result 类型别名
// ============================================================================

/**
 * @brief 结果类型（成功或错误）
 */
template<typename T>
using result_t = tl::expected<T, error_t>;

/**
 * @brief 可选类型
 */
template<typename T>
using optional_t = tl::optional<T>;

// ============================================================================
// 便捷宏
// ============================================================================

/**
 * @brief 创建成功结果
 */
template<typename T>
inline result_t<T> make_ok(T&& value) {
    return result_t<T>(std::forward<T>(value));
}

inline result_t<void> make_ok() {
    return result_t<void>();
}

/**
 * @brief 创建错误结果
 */
template<typename T>
inline result_t<T> make_error(error_code_t code, const std::string& msg = "") {
    return tl::make_unexpected(error_t(code, msg));
}

/**
 * @brief 创建 void 类型的错误结果
 */
inline result_t<void> make_error_void(error_code_t code, const std::string& msg = "") {
    return tl::make_unexpected(error_t(code, msg));
}

/**
 * @brief 从已有错误创建 void 类型的错误结果
 */
inline result_t<void> make_error_void(const error_t& err) {
    return tl::make_unexpected(err);
}

/**
 * @brief 创建 unexpected 错误对象
 */
inline tl::unexpected<error_t> make_unexpected(error_code_t code, const std::string& msg = "") {
    return tl::make_unexpected(error_t(code, msg));
}

/**
 * @brief 从已有错误创建 unexpected 错误对象
 */
inline tl::unexpected<error_t> make_unexpected(const error_t& err) {
    return tl::make_unexpected(err);
}

}  // namespace vdl

#endif  // VDL_CORE_ERROR_HPP
