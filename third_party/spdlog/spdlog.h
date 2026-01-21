// Simplified spdlog wrapper header for VDL
// This provides the essential logging functionality needed for VDL

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <ctime>
#include <iomanip>
#include <map>

// ============================================================================
// Simplified spdlog-like interface for VDL
// ============================================================================

namespace spdlog {

enum class level : int {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    err = 4,
    critical = 5,
    off = 6
};

namespace level_names {
    static constexpr const char* names[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"
    };
}

class logger {
public:
    logger(const std::string& name) : name_(name), level_(level::info) {}
    
    void set_level(spdlog::level l) { level_ = l; }
    level get_level() const { return level_; }
    
    template<typename... Args>
    void trace(const std::string& fmt, Args&&...) { log(level::trace, fmt); }
    
    template<typename... Args>
    void debug(const std::string& fmt, Args&&...) { log(level::debug, fmt); }
    
    template<typename... Args>
    void info(const std::string& fmt, Args&&...) { log(level::info, fmt); }
    
    template<typename... Args>
    void warn(const std::string& fmt, Args&&...) { log(level::warn, fmt); }
    
    template<typename... Args>
    void error(const std::string& fmt, Args&&...) { log(level::err, fmt); }
    
    template<typename... Args>
    void critical(const std::string& fmt, Args&&...) { log(level::critical, fmt); }

private:
    void log(level l, const std::string& msg) {
        if (static_cast<int>(l) >= static_cast<int>(level_)) {
            auto now = std::time(nullptr);
            auto tm = *std::localtime(&now);
            std::stringstream ss;
            ss << "["
               << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
               << "] ["
               << name_
               << "] ["
               << level_names::names[static_cast<int>(l)]
               << "] "
               << msg
               << std::endl;
            std::cout << ss.str();
        }
    }
    
    std::string name_;
    spdlog::level level_;
};

inline std::shared_ptr<logger> stdout_color_mt(const std::string& name) {
    return std::make_shared<logger>(name);
}

inline std::shared_ptr<logger> get(const std::string& name) {
    static std::map<std::string, std::shared_ptr<logger>> loggers;
    if (loggers.find(name) == loggers.end()) {
        loggers[name] = stdout_color_mt(name);
    }
    return loggers[name];
}

} // namespace spdlog

// Macros for VDL logging
#define SPDLOG_TRACE(msg) do {} while(0)
#define SPDLOG_DEBUG(msg) do {} while(0)
#define SPDLOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#define SPDLOG_WARN(msg) std::cerr << "[WARN] " << msg << std::endl
#define SPDLOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl
#define SPDLOG_CRITICAL(msg) std::cerr << "[CRITICAL] " << msg << std::endl
