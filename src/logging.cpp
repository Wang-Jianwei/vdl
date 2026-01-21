/**
 * @file logging.cpp
 * @brief VDL 日志系统实现
 */

#include <vdl/core/logging.hpp>
#include <iostream>

namespace vdl {

// 静态成员变量定义
std::shared_ptr<spdlog::logger> Logger::logger_;
LogLevel Logger::current_level_ = LogLevel::Info;

// ============================================================================
// Logger 实现
// ============================================================================

void Logger::initialize(const std::string& logger_name, LogLevel level) {
    try {
        // 获取或创建日志记录器
        logger_ = spdlog::get(logger_name);
        
        if (!logger_) {
            // 创建新的日志记录器
            logger_ = spdlog::stdout_color_mt(logger_name);
        }
        
        // 设置日志级别
        logger_->set_level(convertLogLevel(level));
        current_level_ = level;
        
    } catch (...) {
        std::cerr << "Log initialization failed" << std::endl;
    }
}

void Logger::setLevel(LogLevel level) {
    if (logger_) {
        logger_->set_level(convertLogLevel(level));
        current_level_ = level;
    }
}

LogLevel Logger::getLevel() {
    return current_level_;
}

void Logger::setPattern(const std::string& pattern) {
    // 简化版 spdlog 不支持设置模式
    // 此函数在这里是为了 API 兼容性
    (void)pattern;
}

void Logger::flush() {
    if (logger_) {
        std::cout.flush();
    }
}

std::shared_ptr<spdlog::logger> Logger::getInternalLogger() {
    if (!logger_) {
        // 如果未初始化，自动初始化
        initialize("vdl", LogLevel::Info);
    }
    return logger_;
}

// ============================================================================
// ScopedLogger 实现
// ============================================================================

ScopedLogger::ScopedLogger(const char* function_name, 
                          const char* file, 
                          int line)
    : function_name_(function_name) {
    if (file && line > 0) {
        logDebug(">>> Entering {}() at {}:{}", function_name, file, line);
    } else {
        logDebug(">>> Entering {}()", function_name);
    }
}

ScopedLogger::~ScopedLogger() {
    logDebug("<<< Exiting {}()", function_name_);
}

} // namespace vdl
