/**
 * @file error.cpp
 * @brief VDL 错误处理系统实现
 */

#include <vdl/core/error.hpp>
#include <sstream>

namespace vdl {

// ============================================================================
// VdlException 实现
// ============================================================================

VdlException::VdlException(ErrorCode error_code,
                          const std::string& message,
                          const char* function,
                          const char* file,
                          int line)
    : error_code_(error_code),
      message_(message),
      function_(function ? function : ""),
      file_(file ? file : ""),
      line_(line) {
}

const char* VdlException::what() const noexcept {
    if (what_message_.empty()) {
        try {
            what_message_ = getFullMessage();
        } catch (...) {
            what_message_ = "VDL Exception";
        }
    }
    return what_message_.c_str();
}

std::string VdlException::getFullMessage() const {
    std::stringstream ss;
    
    // 错误码
    ss << "[Error " << static_cast<int>(error_code_) << "] ";
    
    // 错误描述
    ss << ErrorManager::getErrorMessage(error_code_);
    
    // 自定义消息
    if (!message_.empty()) {
        ss << ": " << message_;
    }
    
    // 位置信息
    if (!function_.empty()) {
        ss << " (in " << function_;
        if (!file_.empty()) {
            ss << " at " << file_;
            if (line_ > 0) {
                ss << ":" << line_;
            }
        }
        ss << ")";
    }
    
    return ss.str();
}

// ============================================================================
// ErrorManager 实现
// ============================================================================

// 静态成员变量初始化
std::map<ErrorCode, std::string> ErrorManager::error_messages_;
ErrorCode ErrorManager::last_error_ = ErrorCode::Success;

std::string ErrorManager::getErrorMessage(ErrorCode code) {
    // 初始化错误消息映射
    if (error_messages_.empty()) {
        initializeErrorMessages();
    }
    
    auto it = error_messages_.find(code);
    if (it != error_messages_.end()) {
        return it->second;
    }
    
    return "Unknown error";
}

void ErrorManager::registerErrorMessage(ErrorCode code, const std::string& message) {
    error_messages_[code] = message;
}

ErrorCode ErrorManager::getLastError() {
    return last_error_;
}

void ErrorManager::setLastError(ErrorCode code) {
    last_error_ = code;
}

void ErrorManager::clearError() {
    last_error_ = ErrorCode::Success;
}

void ErrorManager::initializeErrorMessages() {
    // 成功
    error_messages_[ErrorCode::Success] = "Success";
    
    // 一般错误
    error_messages_[ErrorCode::Unknown] = "Unknown error";
    error_messages_[ErrorCode::Invalid] = "Invalid parameter or state";
    error_messages_[ErrorCode::NotSupported] = "Operation not supported";
    error_messages_[ErrorCode::Unimplemented] = "Feature not implemented";
    
    // 内存错误
    error_messages_[ErrorCode::OutOfMemory] = "Out of memory";
    error_messages_[ErrorCode::AllocationFailed] = "Memory allocation failed";
    error_messages_[ErrorCode::InvalidPointer] = "Invalid pointer";
    
    // 参数错误
    error_messages_[ErrorCode::NullPointer] = "Null pointer";
    error_messages_[ErrorCode::InvalidArgument] = "Invalid argument";
    error_messages_[ErrorCode::OutOfRange] = "Out of range";
    error_messages_[ErrorCode::InvalidSize] = "Invalid size";
    
    // IO 错误
    error_messages_[ErrorCode::IOError] = "I/O error";
    error_messages_[ErrorCode::FileNotFound] = "File not found";
    error_messages_[ErrorCode::FileAccessDenied] = "File access denied";
    error_messages_[ErrorCode::InvalidFormat] = "Invalid format";
    
    // 设备错误
    error_messages_[ErrorCode::DeviceError] = "Device error";
    error_messages_[ErrorCode::DeviceNotAvailable] = "Device not available";
    error_messages_[ErrorCode::DeviceNotInitialized] = "Device not initialized";
    error_messages_[ErrorCode::DeviceAlreadyOpen] = "Device already open";
    
    // 超时和竞争
    error_messages_[ErrorCode::Timeout] = "Operation timeout";
    error_messages_[ErrorCode::Deadlock] = "Deadlock detected";
    error_messages_[ErrorCode::ConcurrencyError] = "Concurrency error";
    
    // 协议和通信
    error_messages_[ErrorCode::ProtocolError] = "Protocol error";
    error_messages_[ErrorCode::CommunicationError] = "Communication error";
    error_messages_[ErrorCode::VersionMismatch] = "Version mismatch";
    
    // 配置和初始化
    error_messages_[ErrorCode::ConfigError] = "Configuration error";
    error_messages_[ErrorCode::InitializationFailed] = "Initialization failed";
    error_messages_[ErrorCode::NotInitialized] = "Not initialized";
    error_messages_[ErrorCode::AlreadyInitialized] = "Already initialized";
    
    // 业务逻辑
    error_messages_[ErrorCode::InvalidState] = "Invalid state";
    error_messages_[ErrorCode::OperationFailed] = "Operation failed";
    error_messages_[ErrorCode::OperationCancelled] = "Operation cancelled";
}

} // namespace vdl
