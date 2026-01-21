#pragma once

/**
 * @file error.hpp
 * @brief VDL 错误处理系统
 * 
 * 提供统一的异常和错误处理机制，包括：
 * - 错误码枚举
 * - 异常类体系
 * - 错误消息映射
 */

#include <stdexcept>
#include <string>
#include <map>
#include <sstream>

namespace vdl {

// ============================================================================
// 错误码定义
// ============================================================================

/**
 * @brief VDL 错误码枚举
 * 
 * 定义所有可能的错误情况。
 */
enum class ErrorCode {
    // 成功
    Success = 0,
    
    // 一般错误 (100-199)
    Unknown = 100,                    ///< 未知错误
    Invalid = 101,                    ///< 无效的参数或状态
    NotSupported = 102,               ///< 不支持的操作
    Unimplemented = 103,              ///< 功能未实现
    
    // 内存错误 (200-299)
    OutOfMemory = 200,                ///< 内存不足
    AllocationFailed = 201,           ///< 内存分配失败
    InvalidPointer = 202,             ///< 无效的指针
    
    // 参数错误 (300-399)
    NullPointer = 300,                ///< 空指针
    InvalidArgument = 301,            ///< 无效的参数
    OutOfRange = 302,                 ///< 超出范围
    InvalidSize = 303,                ///< 无效的大小
    
    // IO 错误 (400-499)
    IOError = 400,                    ///< 输入输出错误
    FileNotFound = 401,               ///< 文件未找到
    FileAccessDenied = 402,           ///< 文件访问被拒绝
    InvalidFormat = 403,              ///< 无效的格式
    
    // 设备错误 (500-599)
    DeviceError = 500,                ///< 设备错误
    DeviceNotAvailable = 501,         ///< 设备不可用
    DeviceNotInitialized = 502,       ///< 设备未初始化
    DeviceAlreadyOpen = 503,          ///< 设备已打开
    
    // 超时和竞争 (600-699)
    Timeout = 600,                    ///< 超时
    Deadlock = 601,                   ///< 死锁
    ConcurrencyError = 602,           ///< 并发错误
    
    // 协议和通信 (700-799)
    ProtocolError = 700,              ///< 协议错误
    CommunicationError = 701,         ///< 通信错误
    VersionMismatch = 702,            ///< 版本不匹配
    
    // 配置和初始化 (800-899)
    ConfigError = 800,                ///< 配置错误
    InitializationFailed = 801,       ///< 初始化失败
    NotInitialized = 802,             ///< 未初始化
    AlreadyInitialized = 803,         ///< 已初始化
    
    // 业务逻辑 (900-999)
    InvalidState = 900,               ///< 无效状态
    OperationFailed = 901,            ///< 操作失败
    OperationCancelled = 902,         ///< 操作取消
};

// ============================================================================
// 异常基类
// ============================================================================

/**
 * @brief VDL 异常基类
 * 
 * 所有 VDL 异常的基类，继承自 std::exception。
 */
class VdlException : public std::exception {
public:
    /**
     * @brief 构造异常
     * 
     * @param error_code 错误码
     * @param message 错误消息
     * @param function 函数名
     * @param file 文件名
     * @param line 行号
     */
    VdlException(ErrorCode error_code,
                 const std::string& message = "",
                 const char* function = nullptr,
                 const char* file = nullptr,
                 int line = 0);
    
    /**
     * @brief 虚析构函数
     */
    virtual ~VdlException() noexcept = default;
    
    /**
     * @brief 获取异常消息
     * 
     * @return C 字符串形式的异常消息
     */
    const char* what() const noexcept override;
    
    /**
     * @brief 获取错误码
     * 
     * @return ErrorCode 错误码值
     */
    ErrorCode getErrorCode() const noexcept {
        return error_code_;
    }
    
    /**
     * @brief 获取自定义消息
     * 
     * @return std::string 自定义消息
     */
    std::string getMessage() const noexcept {
        return message_;
    }
    
    /**
     * @brief 获取函数名
     * 
     * @return std::string 函数名
     */
    std::string getFunction() const noexcept {
        return function_;
    }
    
    /**
     * @brief 获取文件名
     * 
     * @return std::string 文件名
     */
    std::string getFile() const noexcept {
        return file_;
    }
    
    /**
     * @brief 获取行号
     * 
     * @return int 行号
     */
    int getLine() const noexcept {
        return line_;
    }
    
    /**
     * @brief 获取完整的错误信息（包含位置信息）
     * 
     * @return std::string 完整错误信息
     */
    std::string getFullMessage() const;

private:
    ErrorCode error_code_;
    std::string message_;
    std::string function_;
    std::string file_;
    int line_;
    mutable std::string what_message_;
};

// ============================================================================
// 特定的异常类
// ============================================================================

/**
 * @brief 内存异常
 */
class MemoryException : public VdlException {
public:
    MemoryException(const std::string& message = "",
                   const char* function = nullptr,
                   const char* file = nullptr,
                   int line = 0)
        : VdlException(ErrorCode::OutOfMemory, message, function, file, line) {}
};

/**
 * @brief 参数错误异常
 */
class ArgumentException : public VdlException {
public:
    ArgumentException(const std::string& message = "",
                     const char* function = nullptr,
                     const char* file = nullptr,
                     int line = 0)
        : VdlException(ErrorCode::InvalidArgument, message, function, file, line) {}
};

/**
 * @brief 状态错误异常
 */
class StateException : public VdlException {
public:
    StateException(const std::string& message = "",
                  const char* function = nullptr,
                  const char* file = nullptr,
                  int line = 0)
        : VdlException(ErrorCode::InvalidState, message, function, file, line) {}
};

/**
 * @brief IO 异常
 */
class IOException : public VdlException {
public:
    IOException(const std::string& message = "",
               const char* function = nullptr,
               const char* file = nullptr,
               int line = 0)
        : VdlException(ErrorCode::IOError, message, function, file, line) {}
};

/**
 * @brief 设备异常
 */
class DeviceException : public VdlException {
public:
    DeviceException(const std::string& message = "",
                   const char* function = nullptr,
                   const char* file = nullptr,
                   int line = 0)
        : VdlException(ErrorCode::DeviceError, message, function, file, line) {}
};

/**
 * @brief 超时异常
 */
class TimeoutException : public VdlException {
public:
    TimeoutException(const std::string& message = "",
                    const char* function = nullptr,
                    const char* file = nullptr,
                    int line = 0)
        : VdlException(ErrorCode::Timeout, message, function, file, line) {}
};

// ============================================================================
// 错误管理类
// ============================================================================

/**
 * @brief 错误管理器
 * 
 * 提供错误码到消息的映射和全局错误状态管理。
 */
class ErrorManager {
public:
    /**
     * @brief 获取错误代码的描述信息
     * 
     * @param code 错误码
     * @return std::string 错误描述
     */
    static std::string getErrorMessage(ErrorCode code);
    
    /**
     * @brief 注册自定义错误消息
     * 
     * @param code 错误码
     * @param message 错误消息
     */
    static void registerErrorMessage(ErrorCode code, const std::string& message);
    
    /**
     * @brief 获取最后一个错误码
     * 
     * @return ErrorCode 最后的错误码
     */
    static ErrorCode getLastError();
    
    /**
     * @brief 设置最后一个错误码
     * 
     * @param code 错误码
     */
    static void setLastError(ErrorCode code);
    
    /**
     * @brief 清除最后一个错误
     */
    static void clearError();
    
private:
    static std::map<ErrorCode, std::string> error_messages_;
    static ErrorCode last_error_;
    
    static void initializeErrorMessages();
};

// ============================================================================
// 便捷宏定义
// ============================================================================

/**
 * @brief 抛出 VdlException 的宏
 * 
 * 使用示例：
 *   VDL_THROW(vdl::ErrorCode::Invalid, "value out of range");
 */
#define VDL_THROW(code, message) \
    throw vdl::VdlException(code, message, __FUNCTION__, __FILE__, __LINE__)

/**
 * @brief 抛出特定类型异常的宏
 * 
 * 使用示例：
 *   VDL_THROW_ARGUMENT("invalid parameter");
 */
#define VDL_THROW_ARGUMENT(message) \
    throw vdl::ArgumentException(message, __FUNCTION__, __FILE__, __LINE__)

/**
 * @brief 抛出内存异常的宏
 */
#define VDL_THROW_MEMORY(message) \
    throw vdl::MemoryException(message, __FUNCTION__, __FILE__, __LINE__)

/**
 * @brief 抛出状态异常的宏
 */
#define VDL_THROW_STATE(message) \
    throw vdl::StateException(message, __FUNCTION__, __FILE__, __LINE__)

/**
 * @brief 抛出 IO 异常的宏
 */
#define VDL_THROW_IO(message) \
    throw vdl::IOException(message, __FUNCTION__, __FILE__, __LINE__)

/**
 * @brief 抛出设备异常的宏
 */
#define VDL_THROW_DEVICE(message) \
    throw vdl::DeviceException(message, __FUNCTION__, __FILE__, __LINE__)

/**
 * @brief 抛出超时异常的宏
 */
#define VDL_THROW_TIMEOUT(message) \
    throw vdl::TimeoutException(message, __FUNCTION__, __FILE__, __LINE__)

/**
 * @brief 设置最后的错误
 * 
 * 使用示例：
 *   VDL_SET_ERROR(vdl::ErrorCode::Invalid);
 */
#define VDL_SET_ERROR(code) \
    vdl::ErrorManager::setLastError(code)

/**
 * @brief 获取最后的错误消息
 */
#define VDL_GET_ERROR_MESSAGE() \
    vdl::ErrorManager::getErrorMessage(vdl::ErrorManager::getLastError())

/**
 * @brief 检查条件，如果不满足则抛出异常
 * 
 * 使用示例：
 *   VDL_CHECK(ptr != nullptr, vdl::ErrorCode::NullPointer, "pointer is null");
 */
#define VDL_CHECK(condition, code, message) \
    do { \
        if (!(condition)) { \
            VDL_THROW(code, message); \
        } \
    } while(0)

/**
 * @brief 检查指针不为空
 */
#define VDL_CHECK_NOT_NULL(ptr, message) \
    VDL_CHECK((ptr) != nullptr, vdl::ErrorCode::NullPointer, message)

/**
 * @brief 检查值在范围内
 */
#define VDL_CHECK_RANGE(value, min, max, message) \
    VDL_CHECK((value) >= (min) && (value) <= (max), \
              vdl::ErrorCode::OutOfRange, message)

} // namespace vdl
