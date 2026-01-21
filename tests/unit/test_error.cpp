#include <catch.hpp>
#include <vdl/core/error.hpp>

// ============================================================================
// 错误码枚举测试
// ============================================================================

TEST_CASE("ErrorCode enumeration", "[error][enum]") {
    SECTION("Error codes are distinct") {
        REQUIRE(static_cast<int>(vdl::ErrorCode::Success) == 0);
        REQUIRE(static_cast<int>(vdl::ErrorCode::Unknown) == 100);
        REQUIRE(static_cast<int>(vdl::ErrorCode::OutOfMemory) == 200);
        REQUIRE(static_cast<int>(vdl::ErrorCode::NullPointer) == 300);
        REQUIRE(static_cast<int>(vdl::ErrorCode::IOError) == 400);
        REQUIRE(static_cast<int>(vdl::ErrorCode::DeviceError) == 500);
        REQUIRE(static_cast<int>(vdl::ErrorCode::Timeout) == 600);
    }
}

// ============================================================================
// VdlException 基本功能测试
// ============================================================================

TEST_CASE("VdlException construction and properties", "[error][exception]") {
    SECTION("Create exception with error code") {
        vdl::VdlException ex(vdl::ErrorCode::Invalid, "test message");
        REQUIRE(ex.getErrorCode() == vdl::ErrorCode::Invalid);
        REQUIRE(ex.getMessage() == "test message");
    }
    
    SECTION("Create exception with full information") {
        vdl::VdlException ex(vdl::ErrorCode::OutOfRange, "value too large",
                            "test_function", "test.cpp", 42);
        REQUIRE(ex.getErrorCode() == vdl::ErrorCode::OutOfRange);
        REQUIRE(ex.getMessage() == "value too large");
        REQUIRE(ex.getFunction() == "test_function");
        REQUIRE(ex.getFile() == "test.cpp");
        REQUIRE(ex.getLine() == 42);
    }
    
    SECTION("what() returns non-null string") {
        vdl::VdlException ex(vdl::ErrorCode::NullPointer, "null");
        REQUIRE(ex.what() != nullptr);
        REQUIRE(std::string(ex.what()).length() > 0);
    }
}

// ============================================================================
// 异常抛出和捕获测试
// ============================================================================

TEST_CASE("Exception throwing and catching", "[error][throw_catch]") {
    SECTION("Catch by VdlException") {
        try {
            throw vdl::VdlException(vdl::ErrorCode::Invalid, "test");
        } catch (const vdl::VdlException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::Invalid);
            REQUIRE(true);
        } catch (...) {
            REQUIRE(false);
        }
    }
    
    SECTION("Catch by std::exception") {
        try {
            throw vdl::VdlException(vdl::ErrorCode::OutOfMemory, "oom");
        } catch (const std::exception& ex) {
            REQUIRE(std::string(ex.what()).length() > 0);
        }
    }
}

// ============================================================================
// 特定异常类测试
// ============================================================================

TEST_CASE("Specific exception classes", "[error][specific]") {
    SECTION("ArgumentException") {
        try {
            throw vdl::ArgumentException("invalid arg");
        } catch (const vdl::ArgumentException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::InvalidArgument);
        }
    }
    
    SECTION("MemoryException") {
        try {
            throw vdl::MemoryException("out of memory");
        } catch (const vdl::MemoryException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::OutOfMemory);
        }
    }
    
    SECTION("StateException") {
        try {
            throw vdl::StateException("invalid state");
        } catch (const vdl::StateException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::InvalidState);
        }
    }
    
    SECTION("IOException") {
        try {
            throw vdl::IOException("read failed");
        } catch (const vdl::IOException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::IOError);
        }
    }
    
    SECTION("DeviceException") {
        try {
            throw vdl::DeviceException("device error");
        } catch (const vdl::DeviceException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::DeviceError);
        }
    }
    
    SECTION("TimeoutException") {
        try {
            throw vdl::TimeoutException("operation timeout");
        } catch (const vdl::TimeoutException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::Timeout);
        }
    }
}

// ============================================================================
// ErrorManager 测试
// ============================================================================

TEST_CASE("ErrorManager operations", "[error][manager]") {
    SECTION("Get error message for known error code") {
        std::string msg = vdl::ErrorManager::getErrorMessage(vdl::ErrorCode::InvalidArgument);
        REQUIRE(msg.length() > 0);
        bool has_invalid = (msg.find("Invalid") != std::string::npos ||
                            msg.find("invalid") != std::string::npos);
        REQUIRE(has_invalid);
    }
    
    SECTION("Get success message") {
        std::string msg = vdl::ErrorManager::getErrorMessage(vdl::ErrorCode::Success);
        REQUIRE(msg == "Success");
    }
    
    SECTION("Set and get last error") {
        vdl::ErrorManager::clearError();
        REQUIRE(vdl::ErrorManager::getLastError() == vdl::ErrorCode::Success);
        
        vdl::ErrorManager::setLastError(vdl::ErrorCode::Timeout);
        REQUIRE(vdl::ErrorManager::getLastError() == vdl::ErrorCode::Timeout);
    }
    
    SECTION("Register custom error message") {
        vdl::ErrorCode custom_code = vdl::ErrorCode::Unknown;
        std::string custom_msg = "Custom error message";
        
        vdl::ErrorManager::registerErrorMessage(custom_code, custom_msg);
        REQUIRE(vdl::ErrorManager::getErrorMessage(custom_code) == custom_msg);
    }
}

// ============================================================================
// 错误宏测试
// ============================================================================

TEST_CASE("Error macros", "[error][macros]") {
    SECTION("VDL_THROW macro") {
        try {
            VDL_THROW(vdl::ErrorCode::Invalid, "macro test");
        } catch (const vdl::VdlException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::Invalid);
            REQUIRE(ex.getMessage() == "macro test");
        }
    }
    
    SECTION("VDL_THROW_ARGUMENT macro") {
        try {
            VDL_THROW_ARGUMENT("bad argument");
        } catch (const vdl::ArgumentException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::InvalidArgument);
        }
    }
    
    SECTION("VDL_THROW_MEMORY macro") {
        try {
            VDL_THROW_MEMORY("memory error");
        } catch (const vdl::MemoryException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::OutOfMemory);
        }
    }
    
    SECTION("VDL_THROW_STATE macro") {
        try {
            VDL_THROW_STATE("state error");
        } catch (const vdl::StateException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::InvalidState);
        }
    }
    
    SECTION("VDL_SET_ERROR macro") {
        VDL_SET_ERROR(vdl::ErrorCode::OperationFailed);
        REQUIRE(vdl::ErrorManager::getLastError() == vdl::ErrorCode::OperationFailed);
    }
}

// ============================================================================
// 检查宏测试
// ============================================================================

TEST_CASE("Check macros", "[error][check]") {
    SECTION("VDL_CHECK passes on true condition") {
        bool condition = true;
        VDL_CHECK(condition, vdl::ErrorCode::Invalid, "should not throw");
        REQUIRE(true);
    }
    
    SECTION("VDL_CHECK throws on false condition") {
        bool condition = false;
        try {
            VDL_CHECK(condition, vdl::ErrorCode::Invalid, "check failed");
            REQUIRE(false);  // Should not reach here
        } catch (const vdl::VdlException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::Invalid);
        }
    }
    
    SECTION("VDL_CHECK_NOT_NULL with non-null pointer") {
        int value = 42;
        int* ptr = &value;
        VDL_CHECK_NOT_NULL(ptr, "pointer is valid");
        REQUIRE(true);
    }
    
    SECTION("VDL_CHECK_NOT_NULL with null pointer") {
        int* ptr = nullptr;
        try {
            VDL_CHECK_NOT_NULL(ptr, "pointer is null");
            REQUIRE(false);
        } catch (const vdl::VdlException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::NullPointer);
        }
    }
    
    SECTION("VDL_CHECK_RANGE valid") {
        int value = 5;
        VDL_CHECK_RANGE(value, 0, 10, "in range");
        REQUIRE(true);
    }
    
    SECTION("VDL_CHECK_RANGE invalid") {
        int value = 15;
        try {
            VDL_CHECK_RANGE(value, 0, 10, "out of range");
            REQUIRE(false);
        } catch (const vdl::VdlException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::OutOfRange);
        }
    }
}

// ============================================================================
// 错误消息格式测试
// ============================================================================

TEST_CASE("Error message formatting", "[error][formatting]") {
    SECTION("Full message includes error code") {
        vdl::VdlException ex(vdl::ErrorCode::InvalidArgument, "test");
        std::string full_msg = ex.getFullMessage();
        bool has_code = (full_msg.find("InvalidArgument") != std::string::npos ||
                         full_msg.find("301") != std::string::npos);
        REQUIRE(has_code);
    }
    
    SECTION("Full message includes custom message") {
        vdl::VdlException ex(vdl::ErrorCode::Invalid, "custom message");
        std::string full_msg = ex.getFullMessage();
        REQUIRE(full_msg.find("custom message") != std::string::npos);
    }
    
    SECTION("Full message includes location info") {
        vdl::VdlException ex(vdl::ErrorCode::Invalid, "msg",
                            "my_function", "my_file.cpp", 123);
        std::string full_msg = ex.getFullMessage();
        REQUIRE(full_msg.find("my_function") != std::string::npos);
        REQUIRE(full_msg.find("my_file.cpp") != std::string::npos);
        REQUIRE(full_msg.find("123") != std::string::npos);
    }
}

// ============================================================================
// 异常链和处理流程测试
// ============================================================================

TEST_CASE("Exception handling workflow", "[error][workflow]") {
    SECTION("Function throws and caller handles") {
        auto throwing_function = []() {
            VDL_CHECK_NOT_NULL(nullptr, "validation failed");
        };
        
        try {
            throwing_function();
            REQUIRE(false);
        } catch (const vdl::VdlException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::NullPointer);
        }
    }
    
    SECTION("Multiple exception types") {
        try {
            int condition = 0;
            if (condition == 0) {
                VDL_THROW_STATE("invalid state");
            }
        } catch (const vdl::StateException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::InvalidState);
        }
    }
}

// ============================================================================
// 错误恢复测试
// ============================================================================

TEST_CASE("Error recovery scenarios", "[error][recovery]") {
    SECTION("Catch and retry") {
        int attempts = 0;
        bool success = false;
        
        for (int i = 0; i < 3; ++i) {
            try {
                attempts++;
                if (attempts < 3) {
                    VDL_THROW(vdl::ErrorCode::Timeout, "retry");
                }
                success = true;
                break;
            } catch (const vdl::VdlException& ex) {
                // Continue trying
            }
        }
        
        REQUIRE(success);
        REQUIRE(attempts == 3);
    }
    
    SECTION("Error logging and continuation") {
        vdl::ErrorCode last_error = vdl::ErrorCode::Success;
        
        try {
            VDL_THROW(vdl::ErrorCode::OperationFailed, "operation error");
        } catch (const vdl::VdlException& ex) {
            last_error = ex.getErrorCode();
            VDL_SET_ERROR(last_error);
        }
        
        REQUIRE(last_error == vdl::ErrorCode::OperationFailed);
        REQUIRE(vdl::ErrorManager::getLastError() == vdl::ErrorCode::OperationFailed);
    }
}

// ============================================================================
// 集成测试
// ============================================================================

TEST_CASE("Error system integration", "[error][integration]") {
    SECTION("Complete error handling workflow") {
        // 清除之前的错误
        vdl::ErrorManager::clearError();
        
        // 模拟操作和错误处理
        try {
            int value = -5;
            VDL_CHECK_RANGE(value, 0, 100, "value must be 0-100");
        } catch (const vdl::VdlException& ex) {
            VDL_SET_ERROR(ex.getErrorCode());
            REQUIRE(vdl::ErrorManager::getLastError() == vdl::ErrorCode::OutOfRange);
        }
        
        // 验证错误信息可用
        std::string error_msg = vdl::ErrorManager::getErrorMessage(
            vdl::ErrorManager::getLastError());
        REQUIRE(error_msg.length() > 0);
    }
    
    SECTION("Multiple exception types in sequence") {
        vdl::ErrorCode final_error = vdl::ErrorCode::Success;
        
        try {
            try {
                VDL_THROW_ARGUMENT("bad arg");
            } catch (const vdl::ArgumentException& ex) {
                final_error = ex.getErrorCode();
                // Re-throw as different type
                VDL_THROW_STATE("cascading error");
            }
        } catch (const vdl::StateException& ex) {
            REQUIRE(ex.getErrorCode() == vdl::ErrorCode::InvalidState);
            REQUIRE(final_error == vdl::ErrorCode::InvalidArgument);
        }
    }
}
