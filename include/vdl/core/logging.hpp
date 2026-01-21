#pragma once

/**
 * @file logging.hpp
 * @brief VDL 日志系统封装
 * 
 * 提供统一的日志接口，基于 spdlog 库。
 * 支持日志级别、格式配置、输出目标等功能。
 */

#include <vdl/core/noncopyable.hpp>
#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace vdl {

// ============================================================================
// 日志级别枚举
// ============================================================================

/**
 * @brief VDL 日志级别
 * 
 * 与 spdlog 的日志级别对应。
 */
enum class LogLevel {
    Trace,    ///< 跟踪级别
    Debug,    ///< 调试级别
    Info,     ///< 信息级别
    Warn,     ///< 警告级别
    Error,    ///< 错误级别
    Critical, ///< 临界错误级别
    Off       ///< 关闭日志
};

// ============================================================================
// 日志系统管理
// ============================================================================

/**
 * @brief VDL 日志系统类
 * 
 * 管理全局日志记录器的创建、配置和使用。
 */
class Logger {
public:
    /**
     * @brief 初始化日志系统
     * 
     * 必须在使用任何日志功能前调用。
     * 
     * @param logger_name 日志记录器名称
     * @param level 日志级别
     */
    static void initialize(const std::string& logger_name = "vdl",
                          LogLevel level = LogLevel::Info);
    
    /**
     * @brief 设置日志级别
     * 
     * @param level 新的日志级别
     */
    static void setLevel(LogLevel level);
    
    /**
     * @brief 获取当前日志级别
     * 
     * @return LogLevel 当前日志级别
     */
    static LogLevel getLevel();
    
    /**
     * @brief 设置日志格式
     * 
     * spdlog 格式字符串，例如：
     * "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v"
     * 
     * @param pattern 日志格式字符串
     */
    static void setPattern(const std::string& pattern);
    
    /**
     * @brief 刷新日志（确保所有日志输出）
     */
    static void flush();
    
    /**
     * @brief 获取内部的 spdlog 记录器
     * 
     * @return 指向 spdlog logger 的指针
     */
    static std::shared_ptr<spdlog::logger> getInternalLogger();
    
private:
    static std::shared_ptr<spdlog::logger> logger_;
    static LogLevel current_level_;
};

// ============================================================================
// 日志记录函数
// ============================================================================

/**
 * @brief 记录跟踪级别日志
 * 
 * @param format 格式字符串
 * @param args 格式参数
 */
template<typename... Args>
inline void logTrace(const char* format, const Args&... args) {
    if (Logger::getLevel() <= LogLevel::Trace) {
        Logger::getInternalLogger()->trace(format, args...);
    }
}

/**
 * @brief 记录调试级别日志
 * 
 * @param format 格式字符串
 * @param args 格式参数
 */
template<typename... Args>
inline void logDebug(const char* format, const Args&... args) {
    if (Logger::getLevel() <= LogLevel::Debug) {
        Logger::getInternalLogger()->debug(format, args...);
    }
}

/**
 * @brief 记录信息级别日志
 * 
 * @param format 格式字符串
 * @param args 格式参数
 */
template<typename... Args>
inline void logInfo(const char* format, const Args&... args) {
    if (Logger::getLevel() <= LogLevel::Info) {
        Logger::getInternalLogger()->info(format, args...);
    }
}

/**
 * @brief 记录警告级别日志
 * 
 * @param format 格式字符串
 * @param args 格式参数
 */
template<typename... Args>
inline void logWarn(const char* format, const Args&... args) {
    if (Logger::getLevel() <= LogLevel::Warn) {
        Logger::getInternalLogger()->warn(format, args...);
    }
}

/**
 * @brief 记录错误级别日志
 * 
 * @param format 格式字符串
 * @param args 格式参数
 */
template<typename... Args>
inline void logError(const char* format, const Args&... args) {
    if (Logger::getLevel() <= LogLevel::Error) {
        Logger::getInternalLogger()->error(format, args...);
    }
}

/**
 * @brief 记录临界错误级别日志
 * 
 * @param format 格式字符串
 * @param args 格式参数
 */
template<typename... Args>
inline void logCritical(const char* format, const Args&... args) {
    if (Logger::getLevel() <= LogLevel::Critical) {
        Logger::getInternalLogger()->critical(format, args...);
    }
}

// ============================================================================
// 便捷宏定义
// ============================================================================

/**
 * @brief 初始化日志系统的宏
 * 
 * 使用示例：
 *   VDL_LOG_INIT("myapp", vdl::LogLevel::Debug);
 */
#define VDL_LOG_INIT(name, level) \
    vdl::Logger::initialize(name, level)

/**
 * @brief 跟踪级别日志宏
 * 
 * 使用示例：
 *   VDL_LOG_TRACE("Processing value: {}", 42);
 */
#define VDL_LOG_TRACE(...) \
    vdl::logTrace(__VA_ARGS__)

/**
 * @brief 调试级别日志宏
 * 
 * 使用示例：
 *   VDL_LOG_DEBUG("Debug info: {}", variable);
 */
#define VDL_LOG_DEBUG(...) \
    vdl::logDebug(__VA_ARGS__)

/**
 * @brief 信息级别日志宏
 * 
 * 使用示例：
 *   VDL_LOG_INFO("Application started");
 */
#define VDL_LOG_INFO(...) \
    vdl::logInfo(__VA_ARGS__)

/**
 * @brief 警告级别日志宏
 * 
 * 使用示例：
 *   VDL_LOG_WARN("Memory usage high: {}%", usage);
 */
#define VDL_LOG_WARN(...) \
    vdl::logWarn(__VA_ARGS__)

/**
 * @brief 错误级别日志宏
 * 
 * 使用示例：
 *   VDL_LOG_ERROR("Failed to open file: {}", filename);
 */
#define VDL_LOG_ERROR(...) \
    vdl::logError(__VA_ARGS__)

/**
 * @brief 临界错误级别日志宏
 * 
 * 使用示例：
 *   VDL_LOG_CRITICAL("Fatal error: {}", reason);
 */
#define VDL_LOG_CRITICAL(...) \
    vdl::logCritical(__VA_ARGS__)

/**
 * @brief 刷新日志宏
 * 
 * 确保所有日志消息都被写入。
 */
#define VDL_LOG_FLUSH() \
    vdl::Logger::flush()

/**
 * @brief 设置日志级别宏
 * 
 * 使用示例：
 *   VDL_LOG_SET_LEVEL(vdl::LogLevel::Debug);
 */
#define VDL_LOG_SET_LEVEL(level) \
    vdl::Logger::setLevel(level)

// ============================================================================
// 日志上下文辅助类
// ============================================================================

/**
 * @brief 作用域日志记录器
 * 
 * 在构造时记录进入信息，析构时记录退出信息。
 * 用于追踪函数执行。
 */
class ScopedLogger : public vdl::NonCopyable {
public:
    /**
     * @brief 构造 ScopedLogger
     * 
     * @param function_name 函数名称
     * @param file 文件名
     * @param line 行号
     */
    ScopedLogger(const char* function_name, 
                 const char* file = nullptr, 
                 int line = 0);
    
    /**
     * @brief 析构时记录退出信息
     */
    ~ScopedLogger();
    
private:
    std::string function_name_;
};

/**
 * @brief 函数作用域追踪宏
 * 
 * 在函数开始处使用，会自动在函数进入和退出时记录日志。
 * 
 * 使用示例：
 *   void myFunction() {
 *       VDL_SCOPE_LOG();
 *       // 函数代码...
 *   }
 */
#define VDL_SCOPE_LOG() \
    vdl::ScopedLogger _scope_logger_(__FUNCTION__, __FILE__, __LINE__)

} // namespace vdl

// ============================================================================
// 实现部分
// ============================================================================

namespace vdl {

// 需要在 logging.cpp 中定义的静态成员
// 这里在 .hpp 中提供内联实现以保持 header-only 兼容性

inline spdlog::level convertLogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:    return spdlog::level::trace;
        case LogLevel::Debug:    return spdlog::level::debug;
        case LogLevel::Info:     return spdlog::level::info;
        case LogLevel::Warn:     return spdlog::level::warn;
        case LogLevel::Error:    return spdlog::level::err;
        case LogLevel::Critical: return spdlog::level::critical;
        case LogLevel::Off:      return spdlog::level::off;
        default:                 return spdlog::level::info;
    }
}

inline LogLevel convertFromSpdlogLevel(spdlog::level level) {
    switch (level) {
        case spdlog::level::trace:    return LogLevel::Trace;
        case spdlog::level::debug:    return LogLevel::Debug;
        case spdlog::level::info:     return LogLevel::Info;
        case spdlog::level::warn:     return LogLevel::Warn;
        case spdlog::level::err:      return LogLevel::Error;
        case spdlog::level::critical: return LogLevel::Critical;
        case spdlog::level::off:      return LogLevel::Off;
        default:                      return LogLevel::Info;
    }
}

} // namespace vdl
