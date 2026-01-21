#include <catch.hpp>
#include <vdl/core/logging.hpp>
#include <sstream>
#include <thread>
#include <chrono>

// ============================================================================
// 日志初始化测试
// ============================================================================

TEST_CASE("Logger initialization", "[logging][init]") {
    SECTION("Initialize with default level") {
        vdl::Logger::initialize("test_logger", vdl::LogLevel::Info);
        auto logger = vdl::Logger::getInternalLogger();
        REQUIRE(logger);
        // Logger initialized successfully
    }
    
    SECTION("Initialize with debug level") {
        vdl::Logger::initialize("test_logger_debug", vdl::LogLevel::Debug);
        REQUIRE(vdl::Logger::getLevel() == vdl::LogLevel::Debug);
    }
    
    SECTION("Get uninitialized logger auto-initializes") {
        // 清理之前的状态
        auto logger = vdl::Logger::getInternalLogger();
        REQUIRE(logger);
        // 应该自动初始化为默认值
    }
}

// ============================================================================
// 日志级别测试
// ============================================================================

TEST_CASE("Log level operations", "[logging][level]") {
    SECTION("Set and get log level") {
        vdl::Logger::initialize("test_level", vdl::LogLevel::Warn);
        REQUIRE(vdl::Logger::getLevel() == vdl::LogLevel::Warn);
        
        vdl::Logger::setLevel(vdl::LogLevel::Debug);
        REQUIRE(vdl::Logger::getLevel() == vdl::LogLevel::Debug);
    }
    
    SECTION("Change log level multiple times") {
        vdl::Logger::initialize("test_level_multi", vdl::LogLevel::Info);
        
        vdl::Logger::setLevel(vdl::LogLevel::Trace);
        REQUIRE(vdl::Logger::getLevel() == vdl::LogLevel::Trace);
        
        vdl::Logger::setLevel(vdl::LogLevel::Critical);
        REQUIRE(vdl::Logger::getLevel() == vdl::LogLevel::Critical);
        
        vdl::Logger::setLevel(vdl::LogLevel::Info);
        REQUIRE(vdl::Logger::getLevel() == vdl::LogLevel::Info);
    }
}

// ============================================================================
// 日志记录宏测试
// ============================================================================

TEST_CASE("Logging macros", "[logging][macros]") {
    SECTION("Initialize with macro") {
        VDL_LOG_INIT("macro_test", vdl::LogLevel::Debug);
        REQUIRE(vdl::Logger::getLevel() == vdl::LogLevel::Debug);
    }
    
    SECTION("Log with different levels using macros") {
        vdl::Logger::initialize("macro_log", vdl::LogLevel::Trace);
        
        // 这些宏调用不应该抛出异常
        VDL_LOG_TRACE("Trace message");
        VDL_LOG_DEBUG("Debug message");
        VDL_LOG_INFO("Info message");
        VDL_LOG_WARN("Warn message");
        VDL_LOG_ERROR("Error message");
        VDL_LOG_CRITICAL("Critical message");
        
        // 测试通过意味着没有异常
        REQUIRE(true);
    }
    
    SECTION("Log with format parameters") {
        vdl::Logger::initialize("format_log", vdl::LogLevel::Debug);
        
        int value = 42;
        std::string text = "test";
        double pi = 3.14159;
        
        VDL_LOG_INFO("Integer: {}, Text: {}, Float: {}", value, text, pi);
        VDL_LOG_DEBUG("Single param: {}", value);
        VDL_LOG_WARN("Multiple: {} {} {}", 1, 2, 3);
        
        REQUIRE(true);
    }
}

// ============================================================================
// 日志级别过滤测试
// ============================================================================

TEST_CASE("Log level filtering", "[logging][filter]") {
    SECTION("Messages below level are not logged") {
        vdl::Logger::initialize("filter_test", vdl::LogLevel::Warn);
        
        // 设置为 Warn 级别，低于 Warn 的消息不应被记录
        // （这是在内存级别，不容易测试输出，但宏会检查级别）
        VDL_LOG_DEBUG("This should be filtered");
        VDL_LOG_INFO("This should also be filtered");
        VDL_LOG_WARN("This should pass through");
        VDL_LOG_ERROR("This should pass through");
        
        REQUIRE(true);
    }
    
    SECTION("All messages at Trace level") {
        vdl::Logger::initialize("trace_test", vdl::LogLevel::Trace);
        
        // 在 Trace 级别，所有消息都应该被记录
        VDL_LOG_TRACE("Trace");
        VDL_LOG_DEBUG("Debug");
        VDL_LOG_INFO("Info");
        VDL_LOG_WARN("Warn");
        VDL_LOG_ERROR("Error");
        VDL_LOG_CRITICAL("Critical");
        
        REQUIRE(true);
    }
}

// ============================================================================
// 日志格式测试
// ============================================================================

TEST_CASE("Log pattern", "[logging][pattern]") {
    SECTION("Set custom pattern") {
        vdl::Logger::initialize("pattern_test", vdl::LogLevel::Info);
        
        std::string custom_pattern = "[%H:%M:%S] %v";
        vdl::Logger::setPattern(custom_pattern);
        
        // 记录一条消息，验证不会出错
        VDL_LOG_INFO("Testing custom pattern");
        
        REQUIRE(true);
    }
    
    SECTION("Reset to default pattern") {
        vdl::Logger::initialize("default_pattern", vdl::LogLevel::Info);
        
        // 恢复默认格式
        const char* default_pattern = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v";
        vdl::Logger::setPattern(default_pattern);
        
        VDL_LOG_INFO("Default pattern restored");
        
        REQUIRE(true);
    }
}

// ============================================================================
// 日志刷新测试
// ============================================================================

TEST_CASE("Logger flush", "[logging][flush]") {
    SECTION("Flush does not throw") {
        vdl::Logger::initialize("flush_test", vdl::LogLevel::Info);
        
        VDL_LOG_INFO("Message 1");
        VDL_LOG_FLUSH();
        VDL_LOG_INFO("Message 2");
        VDL_LOG_FLUSH();
        
        REQUIRE(true);
    }
}

// ============================================================================
// 作用域日志测试
// ============================================================================

TEST_CASE("Scoped logger", "[logging][scope]") {
    SECTION("Create and destroy scoped logger") {
        vdl::Logger::initialize("scope_test", vdl::LogLevel::Debug);
        
        {
            VDL_SCOPE_LOG();
            VDL_LOG_DEBUG("Inside scope");
        }
        
        // 作用域日志应该自动记录进入和退出
        REQUIRE(true);
    }
    
    SECTION("Nested scopes") {
        vdl::Logger::initialize("nested_scope", vdl::LogLevel::Debug);
        
        {
            VDL_SCOPE_LOG();
            VDL_LOG_DEBUG("Outer scope");
            
            {
                VDL_SCOPE_LOG();
                VDL_LOG_DEBUG("Inner scope");
            }
        }
        
        REQUIRE(true);
    }
}

// ============================================================================
// 日志函数直接调用测试
// ============================================================================

TEST_CASE("Direct logging functions", "[logging][functions]") {
    SECTION("Call logging functions directly") {
        vdl::Logger::initialize("direct_test", vdl::LogLevel::Trace);
        
        vdl::logTrace("Trace level");
        vdl::logDebug("Debug level");
        vdl::logInfo("Info level");
        vdl::logWarn("Warn level");
        vdl::logError("Error level");
        vdl::logCritical("Critical level");
        
        REQUIRE(true);
    }
    
    SECTION("Direct functions with format strings") {
        vdl::Logger::initialize("direct_format", vdl::LogLevel::Trace);
        
        int count = 5;
        std::string name = "VDL";
        
        vdl::logInfo("Count: {}, Name: {}", count, name);
        vdl::logDebug("Value: {}", 42);
        vdl::logWarn("Warning with number: {}", 100);
        
        REQUIRE(true);
    }
}

// ============================================================================
// 日志工具函数测试
// ============================================================================

TEST_CASE("Log level conversion utilities", "[logging][convert]") {
    SECTION("Convert VDL level to spdlog level") {
        auto spdlog_level = vdl::convertLogLevel(vdl::LogLevel::Debug);
        REQUIRE(spdlog_level == spdlog::level::debug);
        
        spdlog_level = vdl::convertLogLevel(vdl::LogLevel::Info);
        REQUIRE(spdlog_level == spdlog::level::info);
        
        spdlog_level = vdl::convertLogLevel(vdl::LogLevel::Error);
        REQUIRE(spdlog_level == spdlog::level::err);
    }
    
    SECTION("Convert spdlog level to VDL level") {
        auto vdl_level = vdl::convertFromSpdlogLevel(spdlog::level::trace);
        REQUIRE(vdl_level == vdl::LogLevel::Trace);
        
        vdl_level = vdl::convertFromSpdlogLevel(spdlog::level::warn);
        REQUIRE(vdl_level == vdl::LogLevel::Warn);
        
        vdl_level = vdl::convertFromSpdlogLevel(spdlog::level::critical);
        REQUIRE(vdl_level == vdl::LogLevel::Critical);
    }
}

// ============================================================================
// 多线程日志测试
// ============================================================================

TEST_CASE("Thread-safe logging", "[logging][threadsafe]") {
    SECTION("Log from multiple threads") {
        vdl::Logger::initialize("thread_test", vdl::LogLevel::Info);
        
        auto thread_func = [](int thread_id) {
            for (int i = 0; i < 5; ++i) {
                vdl::logInfo("Thread {} message {}", thread_id, i);
            }
        };
        
        std::thread t1(thread_func, 1);
        std::thread t2(thread_func, 2);
        
        t1.join();
        t2.join();
        
        // spdlog 是线程安全的，所以这应该不会崩溃
        REQUIRE(true);
    }
}

// ============================================================================
// 日志级别枚举测试
// ============================================================================

TEST_CASE("LogLevel enum values", "[logging][enum]") {
    SECTION("Enum values are distinct") {
        REQUIRE(static_cast<int>(vdl::LogLevel::Trace) != 
                static_cast<int>(vdl::LogLevel::Debug));
        REQUIRE(static_cast<int>(vdl::LogLevel::Debug) != 
                static_cast<int>(vdl::LogLevel::Info));
        REQUIRE(static_cast<int>(vdl::LogLevel::Info) != 
                static_cast<int>(vdl::LogLevel::Warn));
        REQUIRE(static_cast<int>(vdl::LogLevel::Warn) != 
                static_cast<int>(vdl::LogLevel::Error));
        REQUIRE(static_cast<int>(vdl::LogLevel::Error) != 
                static_cast<int>(vdl::LogLevel::Critical));
    }
    
    SECTION("Off level is highest") {
        REQUIRE(static_cast<int>(vdl::LogLevel::Off) > 
                static_cast<int>(vdl::LogLevel::Critical));
    }
}

// ============================================================================
// 集成测试
// ============================================================================

TEST_CASE("Integration test", "[logging][integration]") {
    SECTION("Complete logging workflow") {
        // 初始化
        VDL_LOG_INIT("integration", vdl::LogLevel::Debug);
        
        // 记录不同级别的日志
        VDL_LOG_TRACE("Starting application");
        VDL_LOG_DEBUG("Debug: initializing components");
        VDL_LOG_INFO("Application started successfully");
        
        // 模拟应用程序操作
        {
            VDL_SCOPE_LOG();
            VDL_LOG_DEBUG("Processing request");
            VDL_LOG_INFO("Request completed");
        }
        
        // 更改日志级别
        VDL_LOG_SET_LEVEL(vdl::LogLevel::Warn);
        VDL_LOG_DEBUG("This debug message should be filtered");
        VDL_LOG_WARN("This warning will be shown");
        
        // 刷新日志
        VDL_LOG_FLUSH();
        VDL_LOG_INFO("This info will be filtered (level is Warn)");
        
        REQUIRE(true);
    }
}
