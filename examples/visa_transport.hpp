/**
 * @file visa_transport.hpp
 * @brief VISA (Virtual Instrument Software Architecture) Transport 实现
 * 
 * 这个实现将 NI-VISA 或其他 VISA 库包装为 VDL Transport 接口。
 * 支持 GPIB、TCP/IP、USB、Serial 等多种传输方式。
 * 
 * 用法：
 *   auto transport = std::make_unique<visa_transport_t>("TCPIP::192.168.1.100::INSTR");
 *   auto codec = std::make_unique<vdl::binary_codec_t>();
 *   vdl::device_impl_t device(std::move(transport), std::move(codec));
 *   device.connect();
 *   
 *   vdl::scpi_adapter_t scpi(device);
 *   auto idn = scpi.get_idn();
 */

#ifndef VISA_TRANSPORT_HPP
#define VISA_TRANSPORT_HPP

#include <vdl/transport/transport.hpp>
#include <string>
#include <memory>
#include <iostream>

// ============================================================================
// VISA 库模拟（当 VISA 库不可用时用于测试）
// 实际部署时应使用真实的 VISA 库：#include <visa.h>
// ============================================================================

// VISA 类型定义
typedef unsigned int ViUInt32;
typedef int ViInt32;
typedef unsigned char ViByte;
typedef void* ViSession;
typedef int ViStatus;

// VISA 常量
const ViSession VI_NULL = 0;
const ViStatus VI_SUCCESS = 0;
const ViStatus VI_ERROR_TMO = -1073807339;
const ViStatus VI_ERROR_CONN_LOST = -1073807234;
const ViStatus VI_ERROR_TIMEOUT = -1073807339;
const ViStatus VI_ERROR_RSRC_NFOUND = -1073807343;
const ViStatus VI_ERROR_INV_RSRC = -1073807301;

const ViInt32 VI_ATTR_TMO_VALUE = 1073676312;

// 模拟的 VISA 函数声明
// 在实际使用中，这些应该来自真实的 VISA 库
extern "C" {
    ViStatus viOpenDefaultRM(ViSession* sesn);
    ViStatus viOpen(ViSession sesn, const char* resource_id, 
                    ViInt32 access_mode, ViInt32 open_timeout, 
                    ViSession* vi);
    ViStatus viClose(ViSession vi);
    ViStatus viWrite(ViSession vi, ViByte* buf, ViUInt32 count, 
                     ViUInt32* ret_count);
    ViStatus viRead(ViSession vi, ViByte* buf, ViUInt32 count, 
                    ViUInt32* ret_count);
    ViStatus viSetAttribute(ViSession vi, ViInt32 attr, 
                            ViUInt32 attr_state);
    ViStatus viClear(ViSession vi);
}

// ============================================================================
// VISA Transport 实现
// ============================================================================

/**
 * @class visa_transport_t
 * @brief VISA 传输层实现
 * 
 * 将 VISA 库调用包装为 VDL Transport 接口
 * 支持所有 VISA 资源类型
 */
class visa_transport_t : public vdl::transport_base_t {
public:
    /**
     * @brief 构造函数
     * @param resource_string VISA 资源字符串
     *   - "GPIB0::192::INSTR" - GPIB 192 地址
     *   - "TCPIP::192.168.1.100::INSTR" - TCP/IP
     *   - "USB::0x0699::0x0341::INSTR" - USB
     *   - "ASRL1::INSTR" - Serial
     * @param timeout_ms 默认操作超时（毫秒）
     */
    explicit visa_transport_t(
        const std::string& resource_string,
        uint32_t timeout_ms = 2000
    );

    virtual ~visa_transport_t();

    // Transport 接口实现
    vdl::result_t<void> open() override;
    void close() override;
    
    vdl::result_t<void> write_all(
        vdl::const_byte_span_t data,
        vdl::milliseconds_t timeout_ms
    ) override;

    vdl::result_t<size_t> write(
        vdl::const_byte_span_t data,
        vdl::milliseconds_t timeout_ms = 0
    ) override;

    vdl::result_t<size_t> read(
        vdl::byte_span_t buffer,
        vdl::milliseconds_t timeout_ms = 0
    ) override;

    bool is_open() const override;

    const char* type_name() const override;

    // VISA 特定方法
    
    /**
     * @brief 查询仪器 IDN
     * @return IDN 字符串或错误
     */
    vdl::result_t<std::string> get_idn() const;

    /**
     * @brief 清除设备缓冲区
     */
    vdl::result_t<void> clear();

    /**
     * @brief 设置操作超时
     * @param timeout_ms 超时时间（毫秒）
     */
    vdl::result_t<void> set_timeout(uint32_t timeout_ms);

    /**
     * @brief 获取 VISA 资源字符串
     */
    const std::string& get_resource_string() const {
        return m_resource_string;
    }

    /**
     * @brief 检查连接状态
     */
    std::string get_connection_info() const;

private:
    ViSession m_resource_manager = VI_NULL;
    ViSession m_instrument = VI_NULL;
    std::string m_resource_string;
    uint32_t m_timeout_ms = 2000;
    bool m_is_open = false;

    // 辅助方法
    std::string visa_error_to_string(ViStatus status) const;
    
    /**
     * @brief 发送命令并接收响应
     */
    vdl::result_t<std::string> query(const std::string& command) const;
};

// ============================================================================
// VISA 配置辅助类
// ============================================================================

/**
 * @class visa_config_builder_t
 * @brief 帮助构建 VISA 资源字符串
 */
class visa_config_builder_t {
public:
    // GPIB 连接
    static std::string gpib_resource(
        uint32_t board,
        uint32_t primary_address,
        uint32_t secondary_address = 0
    ) {
        return "GPIB" + std::to_string(board) + "::" + 
               std::to_string(primary_address) + "::" +
               (secondary_address > 0 ? std::to_string(secondary_address) : "") +
               "::INSTR";
    }

    // TCP/IP 连接
    static std::string tcpip_resource(
        const std::string& host,
        uint16_t port = 5025
    ) {
        return "TCPIP::" + host + "::" + std::to_string(port) + "::SOCKET";
    }

    // TCP/IP 标准连接
    static std::string tcpip_standard(const std::string& host) {
        return "TCPIP::" + host + "::INSTR";
    }

    // USB 连接
    static std::string usb_resource(
        uint16_t vendor_id,
        uint16_t product_id,
        uint32_t serial = 0
    ) {
        return "USB::" + to_hex(vendor_id) + "::" + 
               to_hex(product_id) + "::" +
               (serial > 0 ? std::to_string(serial) : "") +
               "::INSTR";
    }

    // Serial 连接
    static std::string serial_resource(uint32_t port) {
        return "ASRL" + std::to_string(port) + "::INSTR";
    }

private:
    static std::string to_hex(uint16_t value) {
        char buf[10];
        snprintf(buf, sizeof(buf), "0x%04X", value);
        return buf;
    }
};

// ============================================================================
// VISA Transport 工厂函数
// ============================================================================

/**
 * @brief 创建 VISA Transport 的便利函数
 */
inline std::unique_ptr<visa_transport_t> make_visa_transport(
    const std::string& resource_string,
    uint32_t timeout_ms = 2000
) {
    return std::make_unique<visa_transport_t>(resource_string, timeout_ms);
}

#endif // VISA_TRANSPORT_HPP
