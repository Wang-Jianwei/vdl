/**
 * @file 06_vna_scpi_communication.cpp
 * @brief VNA 设备 SCPI 通信示例
 * 
 * 本示例展示如何使用 VDL 库与 VNA 设备进行 SCPI 命令通信。
 * SCPI (Standard Commands for Programmable Instruments) 是一种基于文本的命令协议。
 * 
 * 编译: 
 * g++ -std=c++11 06_vna_scpi_communication.cpp -I../include -I../third_party -o vna_scpi_example
 * 
 * 运行:
 * ./vna_scpi_example <VNA_IP> <VNA_PORT>
 * 例如: ./vna_scpi_example 192.168.1.100 5025
 */

#include <vdl/vdl.hpp>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "ws2_32.lib")
    #endif
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/select.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    typedef int SOCKET;
#endif

namespace vdl {

// ============================================================================
// TCP 传输层实现
// ============================================================================

/**
 * @brief TCP 传输层
 * 
 * 用于基于 TCP/IP 的设备通信（如以太网 VNA）
 */
class tcp_transport_t : public transport_base_t {
public:
    /**
     * @brief 构造函数
     * @param host 主机地址（IP 或主机名）
     * @param port 端口号
     */
    tcp_transport_t(const std::string& host, uint16_t port)
        : m_host(host), m_port(port), m_socket(INVALID_SOCKET) {
#ifdef _WIN32
        // Windows 需要初始化 Winsock
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    }

    ~tcp_transport_t() {
        close();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    // ========================================================================
    // 连接管理
    // ========================================================================

    result_t<void> open() override {
        if (is_open()) {
            return make_ok();
        }

        // 创建 socket
        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_socket == INVALID_SOCKET) {
            return make_error_void(error_code_t::communication_error, "Failed to create socket");
        }

        // 设置地址
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(m_port);
        
#ifdef _WIN32
        server_addr.sin_addr.s_addr = inet_addr(m_host.c_str());
#else
        if (inet_pton(AF_INET, m_host.c_str(), &server_addr.sin_addr) <= 0) {
            close();
            return make_error_void(error_code_t::address_invalid, "Invalid IP address");
        }
#endif

        // 连接
        if (connect(m_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            close();
            return make_error_void(error_code_t::connection_failed, "Failed to connect to device");
        }

        return make_ok();
    }

    void close() override {
        if (m_socket != INVALID_SOCKET) {
#ifdef _WIN32
            closesocket(m_socket);
#else
            ::close(m_socket);
#endif
            m_socket = INVALID_SOCKET;
        }
    }

    bool is_open() const override {
        return m_socket != INVALID_SOCKET;
    }

    // ========================================================================
    // 读写操作
    // ========================================================================

    result_t<size_t> read(byte_span_t buffer, milliseconds_t timeout_ms = 0) override {
        if (!is_open()) {
            return make_error<size_t>(error_code_t::not_connected);
        }

        // 设置超时
        if (timeout_ms == 0) {
            timeout_ms = m_config.read_timeout;
        }

        // 使用 select 实现超时
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(m_socket, &read_fds);

        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        int ret = select(m_socket + 1, &read_fds, NULL, NULL, &tv);
        if (ret == 0) {
            return make_error<size_t>(error_code_t::timeout);
        } else if (ret < 0) {
            return make_error<size_t>(error_code_t::io_error);
        }

        // 读取数据
        ssize_t n = recv(m_socket, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0);
        if (n < 0) {
            return make_error<size_t>(error_code_t::io_error);
        } else if (n == 0) {
            return make_error<size_t>(error_code_t::connection_closed);
        }

        return make_ok<size_t>(static_cast<size_t>(n));
    }

    result_t<size_t> write(const_byte_span_t data, milliseconds_t timeout_ms = 0) override {
        (void)timeout_ms;  // 暂时未使用
        if (!is_open()) {
            return make_error<size_t>(error_code_t::not_connected);
        }

        ssize_t n = send(m_socket, reinterpret_cast<const char*>(data.data()), data.size(), 0);
        if (n < 0) {
            return make_error<size_t>(error_code_t::io_error);
        }

        return make_ok<size_t>(static_cast<size_t>(n));
    }

    const char* type_name() const override {
        return "TCP";
    }

private:
    std::string m_host;
    uint16_t m_port;
    SOCKET m_socket;
};

// ============================================================================
// SCPI 编解码器实现
// ============================================================================

/**
 * @brief SCPI 编解码器
 * 
 * SCPI 协议特点：
 * - 基于文本的命令
 * - 命令以换行符结束（\n）
 * - 响应以换行符结束（\n）
 * - 支持查询命令（以 ? 结尾）
 */
class scpi_codec_t : public codec_base_t {
public:
    scpi_codec_t() = default;

    // ========================================================================
    // 编码
    // ========================================================================

    /**
     * @brief 编码 SCPI 命令
     * 
     * 对于 SCPI，我们使用 command_t 的 data 字段存储命令字符串
     */
    result_t<bytes_t> encode(const command_t& cmd) override {
        bytes_t result;
        
        // 获取命令字符串
        const bytes_t& cmd_data = cmd.data();
        
        // 复制命令数据
        result.insert(result.end(), cmd_data.begin(), cmd_data.end());
        
        // SCPI 命令以换行符结束
        result.push_back('\n');
        
        return make_ok(std::move(result));
    }

    // ========================================================================
    // 解码
    // ========================================================================

    /**
     * @brief 解码 SCPI 响应
     * 
     * SCPI 响应以换行符结束
     */
    result_t<response_t> decode(const_byte_span_t buffer, size_t& consumed) override {
        // 查找换行符
        size_t frame_len = frame_length(buffer);
        
        if (frame_len == 0) {
            // 数据不完整
            return make_error<response_t>(error_code_t::incomplete_frame);
        }
        
        // 创建响应对象
        response_t response;
        response.set_status(response_status_t::success);
        
        // 提取响应数据（去除换行符）
        bytes_t data;
        for (size_t i = 0; i < frame_len - 1; ++i) {
            if (buffer[i] == '\r') continue;  // 跳过回车符
            data.push_back(buffer[i]);
        }
        
        response.set_data(std::move(data));
        
        // 设置消耗的字节数
        consumed = frame_len;
        
        return make_ok(std::move(response));
    }

    /**
     * @brief 检查是否有完整的 SCPI 帧
     * 
     * SCPI 帧以换行符结束
     */
    size_t frame_length(const_byte_span_t buffer) const override {
        for (size_t i = 0; i < buffer.size(); ++i) {
            if (buffer[i] == '\n') {
                return i + 1;
            }
        }
        return 0;  // 没有找到完整帧
    }

    const char* name() const override {
        return "SCPI";
    }
};

// ============================================================================
// SCPI 命令辅助函数
// ============================================================================

/**
 * @brief 创建 SCPI 命令
 * @param scpi_string SCPI 命令字符串
 */
inline command_t make_scpi_command(const std::string& scpi_string) {
    command_t cmd;
    bytes_t data(scpi_string.begin(), scpi_string.end());
    cmd.set_data(std::move(data));
    return cmd;
}

/**
 * @brief 从响应中提取字符串
 */
inline std::string get_response_string(const response_t& response) {
    const bytes_t& data = response.data();
    return std::string(data.begin(), data.end());
}

}  // namespace vdl

// ============================================================================
// 使用示例
// ============================================================================

using namespace vdl;

/**
 * @brief 示例1: 基础 VNA 连接和识别查询
 */
void example_vna_identification(const std::string& host, uint16_t port) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "示例1: VNA 设备识别 (使用便捷接口)\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    // 创建传输层和编解码器
    auto transport = std::unique_ptr<i_transport_t>(
        new tcp_transport_t(host, port)
    );
    auto codec = std::unique_ptr<i_codec_t>(
        new scpi_codec_t()
    );

    // 创建设备对象
    device_impl_t device(std::move(transport), std::move(codec));

    // 连接设备
    std::cout << "正在连接到 VNA @ " << host << ":" << port << "...\n";
    auto conn_result = device.connect();
    if (!conn_result.has_value()) {
        std::cerr << "✗ 连接失败: " << conn_result.error().to_string() << "\n";
        return;
    }
    std::cout << "✓ 连接成功\n\n";

    // 使用便捷接口查询设备ID
    std::cout << "查询设备 ID...\n";
    auto idn_result = device.query("*IDN?");
    if (idn_result.has_value()) {
        std::cout << "设备标识: " << *idn_result << "\n";
    } else {
        std::cerr << "✗ 查询失败: " << idn_result.error().to_string() << "\n";
    }

    // 断开连接
    device.disconnect();
    std::cout << "✓ 已断开连接\n";
}

/**
 * @brief 示例2: VNA 频率设置和查询
 */
void example_vna_frequency_control(const std::string& host, uint16_t port) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "示例2: VNA 频率控制 (使用便捷接口)\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    auto transport = std::unique_ptr<i_transport_t>(new tcp_transport_t(host, port));
    auto codec = std::unique_ptr<i_codec_t>(new scpi_codec_t());
    device_impl_t device(std::move(transport), std::move(codec));

    if (!device.connect().has_value()) {
        std::cerr << "✗ 连接失败\n";
        return;
    }
    std::cout << "✓ 已连接到 VNA\n\n";

    // 使用便捷接口设置和查询频率
    // 设置起始频率
    std::cout << "设置起始频率: 1 GHz\n";
    device.write("SENS:FREQ:STAR 1E9\n");

    // 设置终止频率
    std::cout << "设置终止频率: 10 GHz\n";
    device.write("SENS:FREQ:STOP 10E9\n");

    // 查询起始频率
    std::cout << "\n查询起始频率...\n";
    auto freq_result = device.query("SENS:FREQ:STAR?");
    if (freq_result.has_value()) {
        std::cout << "起始频率: " << *freq_result << " Hz\n";
    } else {
        std::cerr << "✗ 查询失败\n";
    }

    // 查询终止频率
    std::cout << "查询终止频率...\n";
    freq_result = device.query("SENS:FREQ:STOP?");
    if (freq_result.has_value()) {
        std::cout << "终止频率: " << *freq_result << " Hz\n";
    } else {
        std::cerr << "✗ 查询失败\n";
    }

    device.disconnect();
    std::cout << "\n✓ 完成\n";
}

/**
 * @brief 示例3: VNA 扫描参数配置
 */
void example_vna_sweep_config(const std::string& host, uint16_t port) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "示例3: VNA 扫描配置 (使用便捷接口)\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    auto transport = std::unique_ptr<i_transport_t>(new tcp_transport_t(host, port));
    auto codec = std::unique_ptr<i_codec_t>(new scpi_codec_t());
    device_impl_t device(std::move(transport), std::move(codec));

    if (!device.connect().has_value()) {
        std::cerr << "✗ 连接失败\n";
        return;
    }
    std::cout << "✓ 已连接到 VNA\n\n";

    // 设置扫描点数
    std::cout << "设置扫描点数: 201\n";
    device.write("SENS:SWE:POIN 201\n");

    // 查询扫描点数
    auto result = device.query("SENS:SWE:POIN?");
    if (result.has_value()) {
        std::cout << "当前扫描点数: " << *result << "\n";
    }

    // 设置中频带宽
    std::cout << "\n设置中频带宽: 1 kHz\n";
    device.write("SENS:BAND 1000\n");

    // 查询中频带宽
    result = device.query("SENS:BAND?");
    if (result.has_value()) {
        std::cout << "当前中频带宽: " << *result << " Hz\n";
    }

    // 触发单次扫描
    std::cout << "\n触发单次扫描...\n";
    device.write("INIT:IMM\n");
    
    // 等待扫描完成
    std::cout << "等待扫描完成...\n";
    device.write("*WAI\n");
    std::cout << "✓ 扫描完成\n";

    device.disconnect();
    std::cout << "\n✓ 完成\n";
}

/**
 * @brief 示例4: VNA S参数测量
 */
void example_vna_s_parameters(const std::string& host, uint16_t port) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "示例4: S 参数测量 (使用便捷接口)\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    auto transport = std::unique_ptr<i_transport_t>(new tcp_transport_t(host, port));
    auto codec = std::unique_ptr<i_codec_t>(new scpi_codec_t());
    device_impl_t device(std::move(transport), std::move(codec));

    if (!device.connect().has_value()) {
        std::cerr << "✗ 连接失败\n";
        return;
    }
    std::cout << "✓ 已连接到 VNA\n\n";

    // 复位设备
    std::cout << "复位设备...\n";
    device.write("*RST\n");

    // 选择 S11 参数
    std::cout << "选择 S11 参数测量...\n";
    device.write("CALC:PAR:DEF S11\n");

    // 设置数据格式为对数幅度
    std::cout << "设置数据格式: MLOG (对数幅度)\n";
    device.write("CALC:FORM MLOG\n");

    // 触发扫描
    std::cout << "\n开始扫描...\n";
    device.write("INIT:CONT OFF\n");  // 关闭连续扫描
    device.write("INIT:IMM\n");       // 触发单次扫描
    device.write("*WAI\n");           // 等待完成

    // 读取数据
    std::cout << "读取测量数据...\n";
    auto data_result = device.query("CALC:DATA? FDAT");
    if (data_result.has_value()) {
        std::string data = *data_result;
        std::cout << "数据长度: " << data.length() << " 字节\n";
        
        // 显示前100个字符作为示例
        if (data.length() > 100) {
            std::cout << "数据预览: " << data.substr(0, 100) << "...\n";
        } else {
            std::cout << "数据: " << data << "\n";
        }
    }

    device.disconnect();
    std::cout << "\n✓ 完成\n";
}

/**
 * @brief 示例5: 错误检查
 */
void example_vna_error_checking(const std::string& host, uint16_t port) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "示例5: 错误检查 (使用便捷接口)\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    auto transport = std::unique_ptr<i_transport_t>(new tcp_transport_t(host, port));
    auto codec = std::unique_ptr<i_codec_t>(new scpi_codec_t());
    device_impl_t device(std::move(transport), std::move(codec));

    if (!device.connect().has_value()) {
        std::cerr << "✗ 连接失败\n";
        return;
    }
    std::cout << "✓ 已连接到 VNA\n\n";

    // 发送一个可能的错误命令
    std::cout << "发送无效命令（测试错误处理）...\n";
    device.write("INVALID:COMMAND\n");

    // 检查错误队列
    std::cout << "检查设备错误队列...\n";
    for (int i = 0; i < 5; ++i) {
        auto result = device.query("SYST:ERR?");
        if (result.has_value()) {
            std::string error_msg = *result;
            std::cout << "  错误 " << (i+1) << ": " << error_msg << "\n";
            
            // 如果是 "0, No error"，则停止
            if (error_msg.find("0,") == 0 || error_msg.find("+0,") == 0) {
                std::cout << "✓ 错误队列已清空\n";
                break;
            }
        }
    }

    device.disconnect();
    std::cout << "\n✓ 完成\n";
}
// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char* argv[]) {
    std::cout << "╔═══════════════════════════════════════════════════╗\n";
    std::cout << "║       VNA SCPI 通信示例 - VDL 库                 ║\n";
    std::cout << "╚═══════════════════════════════════════════════════╝\n";

    // 检查命令行参数
    std::string host = "192.168.1.100";  // 默认 IP
    uint16_t port = 5025;                 // 默认 SCPI 端口

    if (argc >= 2) {
        host = argv[1];
    }
    if (argc >= 3) {
        port = static_cast<uint16_t>(std::atoi(argv[2]));
    }

    std::cout << "\nVNA 设备配置:\n";
    std::cout << "  IP 地址: " << host << "\n";
    std::cout << "  端口: " << port << "\n";

    // 运行示例
    try {
        example_vna_identification(host, port);
        example_vna_frequency_control(host, port);
        example_vna_sweep_config(host, port);
        example_vna_s_parameters(host, port);
        example_vna_error_checking(host, port);

        std::cout << "\n╔═══════════════════════════════════════════════════╗\n";
        std::cout << "║           所有示例执行完成！                      ║\n";
        std::cout << "╚═══════════════════════════════════════════════════╝\n";
    } catch (const std::exception& e) {
        std::cerr << "\n✗ 异常: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
