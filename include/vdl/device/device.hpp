/**
 * @file device.hpp
 * @brief 设备层接口
 * 
 * 定义设备抽象接口和相关类型。
 */

#ifndef VDL_DEVICE_DEVICE_HPP
#define VDL_DEVICE_DEVICE_HPP

#include "../core/types.hpp"
#include "../core/error.hpp"
#include "../core/noncopyable.hpp"
#include "../protocol/command.hpp"
#include "../protocol/response.hpp"

#include <memory>
#include <string>

namespace vdl {

// ============================================================================
// 设备状态
// ============================================================================

/**
 * @brief 设备连接状态
 */
enum class device_state_t : uint8_t {
    disconnected = 0,   ///< 未连接
    connecting = 1,     ///< 连接中
    connected = 2,      ///< 已连接
    error = 3           ///< 错误状态
};

/**
 * @brief 获取设备状态名称
 */
inline const char* device_state_name(device_state_t state) {
    switch (state) {
        case device_state_t::disconnected: return "disconnected";
        case device_state_t::connecting:   return "connecting";
        case device_state_t::connected:    return "connected";
        case device_state_t::error:        return "error";
        default: return "unknown";
    }
}

// ============================================================================
// 设备信息
// ============================================================================

/**
 * @brief 设备信息结构
 */
struct device_info_t {
    std::string name;           ///< 设备名称
    std::string manufacturer;   ///< 制造商
    std::string model;          ///< 型号
    std::string serial_number;  ///< 序列号
    std::string firmware_version; ///< 固件版本
};

// ============================================================================
// 设备配置
// ============================================================================

/**
 * @brief 设备配置
 */
struct device_config_t {
    milliseconds_t command_timeout = 1000;  ///< 命令超时
    milliseconds_t retry_delay = 100;       ///< 重试延迟
    uint8_t max_retries = 3;                ///< 最大重试次数
    bool auto_reconnect = true;             ///< 自动重连
};

// ============================================================================
// i_device_t - 设备接口
// ============================================================================

/**
 * @brief 设备抽象接口
 * 
 * 定义设备的核心操作接口：
 * - 连接管理
 * - 命令执行
 * - 状态查询
 * 
 * @note 设备通过组合 Transport 和 Codec 实现，而非继承
 */
class i_device_t : private noncopyable_t {
public:
    virtual ~i_device_t() = default;

    // ========================================================================
    // 连接管理
    // ========================================================================

    /**
     * @brief 连接设备
     * @return 成功返回 ok，失败返回错误
     */
    virtual result_t<void> connect() = 0;

    /**
     * @brief 断开连接
     */
    virtual void disconnect() = 0;

    /**
     * @brief 获取连接状态
     */
    virtual device_state_t state() const = 0;

    /**
     * @brief 检查是否已连接
     */
    virtual bool is_connected() const = 0;

    // ========================================================================
    // 命令执行
    // ========================================================================

    /**
     * @brief 执行命令
     * @param cmd 命令对象
     * @return 成功返回响应，失败返回错误
     */
    virtual result_t<response_t> execute(const command_t& cmd) = 0;

    /**
     * @brief 执行命令（带超时）
     * @param cmd 命令对象
     * @param timeout_ms 超时时间
     * @return 成功返回响应，失败返回错误
     */
    virtual result_t<response_t> execute(const command_t& cmd,
                                          milliseconds_t timeout_ms) = 0;

    // ========================================================================
    // 设备信息
    // ========================================================================

    /**
     * @brief 获取设备信息
     */
    virtual const device_info_t& info() const = 0;

    /**
     * @brief 获取设备配置
     */
    virtual const device_config_t& config() const = 0;

    /**
     * @brief 设置设备配置
     */
    virtual void set_config(const device_config_t& config) = 0;

    /**
     * @brief 获取设备类型名称
     */
    virtual const char* type_name() const = 0;
};

// ============================================================================
// device_ptr_t - 设备智能指针
// ============================================================================

using device_ptr_t = std::unique_ptr<i_device_t>;
using device_shared_ptr_t = std::shared_ptr<i_device_t>;

}  // namespace vdl

#endif  // VDL_DEVICE_DEVICE_HPP
