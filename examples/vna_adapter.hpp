/**
 * @file vna_adapter.hpp
 * @brief VNA (矢量网络分析仪) 特定适配器 - 示例实现
 * 
 * 这是一个用户自定义的设备适配器示例。
 * 展示了如何为特定的 VNA 设备编写高级便捷接口。
 * 
 * 用户应该根据自己的设备类型编写类似的适配器。
 */

#ifndef VNA_ADAPTER_HPP
#define VNA_ADAPTER_HPP

#include <vdl/device/scpi_adapter.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace vdl {

// ============================================================================
// VNA 适配器 - 针对矢量网络分析仪的高级接口
// ============================================================================

/**
 * @brief VNA (矢量网络分析仪) 适配器
 * 
 * 提供针对 VNA 设备的便捷接口，包括：
 * - 频率设置和查询
 * - 扫描参数配置
 * - 测量参数设置
 * - 数据读取
 * 
 * @note 此适配器继承自 scpi_adapter_t，提供 VNA 特定的功能
 * 
 * @code
 * auto transport = std::make_unique<tcp_transport_t>("192.168.1.100", 5025);
 * auto codec = std::make_unique<scpi_codec_t>();
 * device_impl_t device(std::move(transport), std::move(codec));
 * 
 * vna_adapter_t vna(device);
 * vna.connect();
 * 
 * // 设置频率范围
 * vna.set_start_freq(1e9);      // 1 GHz
 * vna.set_stop_freq(6e9);       // 6 GHz
 * vna.set_sweep_points(401);    // 401 点扫描
 * 
 * // 触发测量
 * vna.trigger_sweep();
 * auto data = vna.get_formatted_data();
 * @endcode
 */
class vna_adapter_t {
public:
    /**
     * @brief 构造函数
     * @param device 设备实现对象引用
     */
    explicit vna_adapter_t(device_impl_t& device)
        : m_scpi(device) {
    }

    /**
     * @brief 获取底层 SCPI 适配器
     */
    scpi_adapter_t& scpi() {
        return m_scpi;
    }

    /**
     * @brief 连接设备
     */
    result_t<void> connect() {
        return m_scpi.connect();
    }

    /**
     * @brief 断开连接
     */
    void disconnect() {
        m_scpi.disconnect();
    }

    /**
     * @brief 检查是否已连接
     */
    bool is_connected() const {
        return m_scpi.is_connected();
    }

    // ========================================================================
    // 频率相关命令
    // ========================================================================

    /**
     * @brief 设置起始频率
     * @param freq_hz 频率（Hz）
     */
    result_t<void> set_start_freq(double freq_hz) {
        std::ostringstream oss;
        oss << std::scientific << std::setprecision(10) 
            << "SENS:FREQ:STAR " << freq_hz;
        return m_scpi.command(oss.str());
    }

    /**
     * @brief 查询起始频率
     */
    result_t<double> get_start_freq() {
        return m_scpi.query_double("SENS:FREQ:STAR?");
    }

    /**
     * @brief 设置终止频率
     * @param freq_hz 频率（Hz）
     */
    result_t<void> set_stop_freq(double freq_hz) {
        std::ostringstream oss;
        oss << std::scientific << std::setprecision(10) 
            << "SENS:FREQ:STOP " << freq_hz;
        return m_scpi.command(oss.str());
    }

    /**
     * @brief 查询终止频率
     */
    result_t<double> get_stop_freq() {
        return m_scpi.query_double("SENS:FREQ:STOP?");
    }

    /**
     * @brief 设置中心频率
     * @param freq_hz 频率（Hz）
     */
    result_t<void> set_center_freq(double freq_hz) {
        std::ostringstream oss;
        oss << std::scientific << std::setprecision(10) 
            << "SENS:FREQ:CENT " << freq_hz;
        return m_scpi.command(oss.str());
    }

    /**
     * @brief 查询中心频率
     */
    result_t<double> get_center_freq() {
        return m_scpi.query_double("SENS:FREQ:CENT?");
    }

    /**
     * @brief 设置频率跨度
     * @param span_hz 跨度（Hz）
     */
    result_t<void> set_freq_span(double span_hz) {
        std::ostringstream oss;
        oss << std::scientific << std::setprecision(10) 
            << "SENS:FREQ:SPAN " << span_hz;
        return m_scpi.command(oss.str());
    }

    /**
     * @brief 查询频率跨度
     */
    result_t<double> get_freq_span() {
        return m_scpi.query_double("SENS:FREQ:SPAN?");
    }

    // ========================================================================
    // 扫描相关命令
    // ========================================================================

    /**
     * @brief 设置扫描点数
     * @param points 点数
     */
    result_t<void> set_sweep_points(int points) {
        std::ostringstream oss;
        oss << "SENS:SWE:POIN " << points;
        return m_scpi.command(oss.str());
    }

    /**
     * @brief 查询扫描点数
     */
    result_t<int> get_sweep_points() {
        return m_scpi.query_int("SENS:SWE:POIN?");
    }

    /**
     * @brief 设置中频带宽
     * @param bw_hz 带宽（Hz）
     */
    result_t<void> set_if_bandwidth(double bw_hz) {
        std::ostringstream oss;
        oss << std::scientific << std::setprecision(10) 
            << "SENS:BAND " << bw_hz;
        return m_scpi.command(oss.str());
    }

    /**
     * @brief 查询中频带宽
     */
    result_t<double> get_if_bandwidth() {
        return m_scpi.query_double("SENS:BAND?");
    }

    /**
     * @brief 启用连续扫描
     */
    result_t<void> enable_continuous_sweep() {
        return m_scpi.command("INIT:CONT ON");
    }

    /**
     * @brief 禁用连续扫描（单次模式）
     */
    result_t<void> disable_continuous_sweep() {
        return m_scpi.command("INIT:CONT OFF");
    }

    /**
     * @brief 触发单次扫描
     */
    result_t<void> trigger_sweep() {
        return m_scpi.command("INIT:IMM");
    }

    // ========================================================================
    // 测量相关命令
    // ========================================================================

    /**
     * @brief 定义参数（如 S11, S21 等）
     * @param param 参数名称（如 "S11", "S21" 等）
     */
    result_t<void> set_measurement_param(const std::string& param) {
        std::string cmd = "CALC:PAR:DEF " + param;
        return m_scpi.command(cmd);
    }

    /**
     * @brief 设置数据格式
     * @param format 格式名称（MLOG, PHAS, REAL, IMAG 等）
     */
    result_t<void> set_data_format(const std::string& format) {
        std::string cmd = "CALC:FORM " + format;
        return m_scpi.command(cmd);
    }

    /**
     * @brief 读取格式化数据
     * @return 逗号分隔的数据字符串
     */
    result_t<std::string> get_formatted_data() {
        return m_scpi.query("CALC:DATA? FDAT");
    }

    /**
     * @brief 读取复数数据
     * @return 逗号分隔的复数数据字符串 (real1,imag1,real2,imag2,...)
     */
    result_t<std::string> get_complex_data() {
        return m_scpi.query("CALC:DATA? SDAT");
    }

    /**
     * @brief 读取格式化数据并解析为 double 向量
     */
    result_t<std::vector<double>> get_formatted_data_parsed() {
        auto result = get_formatted_data();
        if (!result) {
            return make_unexpected(result.error());
        }
        return scpi_adapter_t::parse_data_doubles(*result);
    }

    /**
     * @brief 读取复数数据并解析为复数向量对
     */
    result_t<std::vector<std::pair<double, double>>> get_complex_data_parsed() {
        auto result = get_complex_data();
        if (!result) {
            return make_unexpected(result.error());
        }
        return scpi_adapter_t::parse_complex_data(*result);
    }

    // ========================================================================
    // 设备管理（通过 SCPI 适配器）
    // ========================================================================

    /**
     * @brief 查询设备 ID
     */
    result_t<std::string> get_idn() {
        return m_scpi.get_idn();
    }

    /**
     * @brief 复位设备
     */
    result_t<void> reset() {
        return m_scpi.reset();
    }

    /**
     * @brief 清除状态
     */
    result_t<void> clear_status() {
        return m_scpi.clear_status();
    }

    /**
     * @brief 查询错误
     */
    result_t<std::string> get_error() {
        return m_scpi.get_error();
    }

private:
    scpi_adapter_t m_scpi;
};

}  // namespace vdl

#endif  // VNA_ADAPTER_HPP
