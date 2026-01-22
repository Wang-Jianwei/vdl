/**
 * @file scpi_adapter.hpp
 * @brief SCPI 协议适配器
 * 
 * 为 SCPI 协议提供便捷的高级接口。
 */

#ifndef VDL_DEVICE_SCPI_ADAPTER_HPP
#define VDL_DEVICE_SCPI_ADAPTER_HPP

#include "device_impl.hpp"
#include "../core/error.hpp"
#include "../core/logging.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

namespace vdl {

// ============================================================================
// SCPI 适配器 - 高级 SCPI 通信接口
// ============================================================================

/**
 * @brief SCPI 协议适配器
 * 
 * 提供针对 SCPI 协议的便捷接口，包括：
 * - 简化的命令发送
 * - 响应解析
 * - 错误检查
 * 
 * @note 此适配器要求设备使用支持文本命令的传输层和编解码器
 * 
 * @code
 * auto transport = std::make_unique<tcp_transport_t>("192.168.1.100", 5025);
 * auto codec = std::make_unique<scpi_codec_t>();
 * device_impl_t device(std::move(transport), std::move(codec));
 * 
 * scpi_adapter_t scpi(&device);
 * scpi.connect();
 * 
 * // 简单查询
 * auto response = scpi.query("*IDN?");
 * if (response) {
 *     std::cout << "设备: " << *response << "\n";
 * }
 * 
 * // 设置参数
 * scpi.command("SENS:FREQ:STAR 1E9");
 * 
 * // 查询参数并转换为数字
 * auto freq = scpi.query_double("SENS:FREQ:STAR?");
 * @endcode
 */
class scpi_adapter_t {
public:
    /**
     * @brief 构造函数
     * @param device 设备实现对象引用
     */
    explicit scpi_adapter_t(device_impl_t& device)
        : m_device(device) {
    }

    /**
     * @brief 连接设备
     */
    result_t<void> connect() {
        return m_device.connect();
    }

    /**
     * @brief 断开连接
     */
    void disconnect() {
        m_device.disconnect();
    }

    /**
     * @brief 检查是否已连接
     */
    bool is_connected() const {
        return m_device.is_connected();
    }

    // ========================================================================
    // 基础命令接口
    // ========================================================================

    /**
     * @brief 发送命令（不期望响应）
     * @param command SCPI 命令字符串
     * @return 成功返回 ok，失败返回错误
     */
    result_t<void> command(const std::string& command) {
        VDL_LOG_DEBUG("SCPI CMD: %s", command.c_str());
        auto result = m_device.write(command);
        return result;
    }

    /**
     * @brief 查询命令（期望响应）
     * @param command SCPI 查询命令字符串
     * @return 成功返回响应字符串，失败返回错误
     */
    result_t<std::string> query(const std::string& command) {
        VDL_LOG_DEBUG("SCPI QUERY: %s", command.c_str());
        auto result = m_device.query(command);
        if (result) {
            VDL_LOG_DEBUG("SCPI RESP: %s", result->c_str());
        }
        return result;
    }

    /**
     * @brief 查询并获取第一个值（逗号分隔的数据）
     * @param command SCPI 查询命令
     * @param index 值的索引（默认 0）
     * @return 成功返回第一个值，失败返回错误
     */
    result_t<std::string> query_value(const std::string& command, 
                                      size_t index = 0) {
        auto result = query(command);
        if (!result) {
            return result;
        }

        // 分割逗号分隔的值
        std::istringstream iss(*result);
        std::string token;
        size_t current_index = 0;

        while (std::getline(iss, token, ',')) {
            if (current_index == index) {
                // 去除前后空格
                token.erase(0, token.find_first_not_of(" \t\r\n"));
                token.erase(token.find_last_not_of(" \t\r\n") + 1);
                return make_ok(std::move(token));
            }
            ++current_index;
        }

        return make_error<std::string>(error_code_t::invalid_format,
                                      "Value index out of range");
    }

    // ========================================================================
    // 类型转换查询
    // ========================================================================

    /**
     * @brief 查询并转换为 double
     * @param command SCPI 查询命令
     * @return 成功返回数值，失败返回错误
     */
    result_t<double> query_double(const std::string& command) {
        auto result = query(command);
        if (!result) {
            return make_unexpected(result.error());
        }

        // 获取第一个值（逗号分隔的情况下）
        std::string value = *result;
        size_t comma_pos = value.find(',');
        if (comma_pos != std::string::npos) {
            value = value.substr(0, comma_pos);
        }

        // 去除前后空格
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        try {
            double parsed_value = std::stod(value);
            return make_ok(parsed_value);
        } catch (const std::exception& e) {
            return make_error<double>(error_code_t::invalid_format,
                                     "Cannot convert to double");
        }
    }

    /**
     * @brief 查询并转换为 int
     * @param command SCPI 查询命令
     * @return 成功返回整数，失败返回错误
     */
    result_t<int> query_int(const std::string& command) {
        auto result = query(command);
        if (!result) {
            return make_unexpected(result.error());
        }

        // 获取第一个值（逗号分隔的情况下）
        std::string value = *result;
        size_t comma_pos = value.find(',');
        if (comma_pos != std::string::npos) {
            value = value.substr(0, comma_pos);
        }

        // 去除前后空格
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        try {
            int parsed_value = std::stoi(value);
            return make_ok(parsed_value);
        } catch (const std::exception& e) {
            return make_error<int>(error_code_t::invalid_format,
                                  "Cannot convert to int");
        }
    }

    /**
     * @brief 查询并转换为 bool
     * @param command SCPI 查询命令
     * @return 成功返回布尔值，失败返回错误
     */
    result_t<bool> query_bool(const std::string& command) {
        auto result = query(command);
        if (!result) {
            return make_unexpected(result.error());
        }

        std::string value = *result;
        // 转换为小写
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        if (value == "1" || value == "on" || value == "true") {
            return make_ok(true);
        } else if (value == "0" || value == "off" || value == "false") {
            return make_ok(false);
        } else {
            return make_error<bool>(error_code_t::invalid_format,
                                   "Cannot convert to bool");
        }
    }

    // ========================================================================
    // 设备管理命令
    // ========================================================================

    /**
     * @brief 查询设备 ID
     */
    result_t<std::string> get_idn() {
        return query("*IDN?");
    }

    /**
     * @brief 复位设备
     */
    result_t<void> reset() {
        return command("*RST");
    }

    /**
     * @brief 清除状态
     */
    result_t<void> clear_status() {
        return command("*CLS");
    }

    /**
     * @brief 等待操作完成
     */
    result_t<void> wait() {
        return command("*WAI");
    }

    /**
     * @brief 查询是否操作完成
     */
    result_t<bool> is_operation_complete() {
        return query_bool("*OPC?");
    }

    /**
     * @brief 查询错误队列
     * @return 成功返回错误消息，失败返回错误
     */
    result_t<std::string> get_error() {
        return query("SYST:ERR?");
    }

    /**
     * @brief 检查并清除所有错误
     * @return 包含所有错误的向量，清除完成返回空向量
     */
    result_t<std::vector<std::string>> clear_all_errors() {
        std::vector<std::string> errors;

        for (int i = 0; i < 100; ++i) {
            auto result = get_error();
            if (!result) {
                return make_unexpected(result.error());
            }

            // 检查是否是 "0, No error"
            if (result->find("+0,") == 0 || result->find("0,") == 0) {
                break;
            }

            errors.push_back(*result);
        }

        return make_ok(std::move(errors));
    }

    // ========================================================================
    // 工具方法
    // ========================================================================

    /**
     * @brief 将逗号分隔的数据转换为 double 向量
     */
    static result_t<std::vector<double>> parse_data_doubles(
        const std::string& data_str) {
        std::vector<double> result;
        std::istringstream iss(data_str);
        std::string token;

        while (std::getline(iss, token, ',')) {
            // 去除前后空格
            token.erase(0, token.find_first_not_of(" \t\r\n"));
            token.erase(token.find_last_not_of(" \t\r\n") + 1);

            if (token.empty()) continue;

            try {
                result.push_back(std::stod(token));
            } catch (const std::exception& e) {
                return make_error<std::vector<double>>(
                    error_code_t::invalid_format,
                    "Failed to parse double value");
            }
        }

        return make_ok(std::move(result));
    }

    /**
     * @brief 将复数数据转换为向量对
     * 格式：实部1,虚部1,实部2,虚部2,...
     */
    static result_t<std::vector<std::pair<double, double>>> 
    parse_complex_data(const std::string& data_str) {
        std::vector<std::pair<double, double>> result;
        auto values_result = parse_data_doubles(data_str);
        if (!values_result) {
            return make_unexpected(values_result.error());
        }

        const auto& values = *values_result;
        if (values.size() % 2 != 0) {
            return make_error<std::vector<std::pair<double, double>>>(
                error_code_t::invalid_format,
                "Complex data must have even number of values");
        }

        for (size_t i = 0; i < values.size(); i += 2) {
            result.push_back({values[i], values[i+1]});
        }

        return make_ok(std::move(result));
    }

private:
    device_impl_t& m_device;
};

}  // namespace vdl

#endif  // VDL_DEVICE_SCPI_ADAPTER_HPP
