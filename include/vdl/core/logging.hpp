/**
 * @file logging.hpp
 * @brief VDL 日志接口
 * 
 * 提供简单的日志宏和接口。
 */

#ifndef VDL_CORE_LOGGING_HPP
#define VDL_CORE_LOGGING_HPP

#include <cstdio>
#include <cstdarg>

namespace vdl {

// ============================================================================
// 日志级别
// ============================================================================

enum class log_level_t {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    error = 4,
    critical = 5,
    off = 6
};

// ============================================================================
// 日志配置
// ============================================================================

namespace detail {

// 全局日志级别
inline log_level_t& get_log_level() {
    static log_level_t s_level = log_level_t::info;
    return s_level;
}

// 日志级别名称
inline const char* get_level_name(log_level_t level) {
    switch (level) {
        case log_level_t::trace:    return "TRACE";
        case log_level_t::debug:    return "DEBUG";
        case log_level_t::info:     return "INFO";
        case log_level_t::warn:     return "WARN";
        case log_level_t::error:    return "ERROR";
        case log_level_t::critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

// 简单日志输出
inline void log_output(log_level_t level, const char* file, int line,
                       const char* fmt, ...) {
    if (level < get_log_level()) {
        return;
    }

    std::fprintf(stderr, "[%s] %s:%d: ", get_level_name(level), file, line);

    va_list args;
    va_start(args, fmt);
    std::vfprintf(stderr, fmt, args);
    va_end(args);

    std::fprintf(stderr, "\n");
}

}  // namespace detail

// ============================================================================
// 公共接口
// ============================================================================

/**
 * @brief 设置日志级别
 */
inline void set_log_level(log_level_t level) {
    detail::get_log_level() = level;
}

/**
 * @brief 获取当前日志级别
 */
inline log_level_t get_log_level() {
    return detail::get_log_level();
}

}  // namespace vdl

// ============================================================================
// 日志宏
// ============================================================================

#define VDL_LOG_TRACE(fmt, ...) \
    vdl::detail::log_output(vdl::log_level_t::trace, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define VDL_LOG_DEBUG(fmt, ...) \
    vdl::detail::log_output(vdl::log_level_t::debug, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define VDL_LOG_INFO(fmt, ...) \
    vdl::detail::log_output(vdl::log_level_t::info, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define VDL_LOG_WARN(fmt, ...) \
    vdl::detail::log_output(vdl::log_level_t::warn, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define VDL_LOG_ERROR(fmt, ...) \
    vdl::detail::log_output(vdl::log_level_t::error, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define VDL_LOG_CRITICAL(fmt, ...) \
    vdl::detail::log_output(vdl::log_level_t::critical, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif  // VDL_CORE_LOGGING_HPP
