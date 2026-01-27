# VNA SCPI 通信示例使用指南

本示例展示如何使用 VDL 库通过 TCP/IP 与 VNA（矢量网络分析仪）设备进行 SCPI（Standard Commands for Programmable Instruments）通信。

## 功能特性

### 1. TCP 传输层 (`tcp_transport_t`)
- 支持 TCP/IP 网络连接
- 跨平台实现（Linux/macOS/Windows）
- 带超时的读写操作
- 自动资源管理

### 2. SCPI 编解码器 (`scpi_codec_t`)
- 文本命令编码
- 换行符分隔的帧解析
- 自动命令终止符处理

### 3. 完整的 VNA 通信示例
包含 5 个实用示例：
- 设备识别
- 频率控制
- 扫描配置
- S 参数测量
- 错误处理

## 编译

### Linux/macOS
```bash
cd /workspaces/vdl/build
cmake ..
make 06_vna_scpi_communication
```

或者直接编译：
```bash
g++ -std=c++11 06_vna_scpi_communication.cpp -I../include -I../third_party -o vna_scpi_example
```

### Windows (MinGW)
```bash
g++ -std=c++11 06_vna_scpi_communication.cpp -I../include -I../third_party -lws2_32 -o vna_scpi_example.exe
```

## 运行

### 基本用法
```bash
# 使用默认 IP (192.168.1.100) 和端口 (5025)
./bin/06_vna_scpi_communication

# 指定自定义 IP 和端口
./bin/06_vna_scpi_communication 192.168.1.100 5025
```

### 预期输出
```
╔═══════════════════════════════════════════════════╗
║       VNA SCPI 通信示例 - VDL 库                 ║
╚═══════════════════════════════════════════════════╝

VNA 设备配置:
  IP 地址: 192.168.1.100
  端口: 5025

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
示例1: VNA 设备识别
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
正在连接到 VNA @ 192.168.1.100:5025...
✓ 连接成功

发送命令: *IDN?
设备响应: Keysight Technologies,N5222B,MY12345678,A.09.90.08
✓ 已断开连接

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
示例2: VNA 频率控制
...
```

## 代码示例

### 连接到 VNA 设备
```cpp
// 创建 TCP 传输层
auto transport = std::unique_ptr<i_transport_t>(
    new tcp_transport_t("192.168.1.100", 5025)
);

// 创建 SCPI 编解码器
auto codec = std::unique_ptr<i_codec_t>(
    new scpi_codec_t()
);

// 创建设备对象
auto device = std::unique_ptr<i_device_t>(
    new device_impl_t(std::move(transport), std::move(codec))
);

// 连接
device->connect();
```

### 发送 SCPI 命令
```cpp
// 查询设备 ID
auto cmd = make_scpi_command("*IDN?");
auto result = device->execute(cmd);
if (result.has_value()) {
    std::string response = get_response_string(*result);
    std::cout << "设备: " << response << "\n";
}
```

### 常用 SCPI 命令

#### 设备识别和状态
```cpp
make_scpi_command("*IDN?");          // 查询设备标识
make_scpi_command("*RST");           // 复位设备
make_scpi_command("*CLS");           // 清除状态
make_scpi_command("*OPC?");          // 操作完成查询
make_scpi_command("SYST:ERR?");      // 查询错误
```

#### 频率设置
```cpp
make_scpi_command("SENS:FREQ:STAR 1E9");      // 设置起始频率 1 GHz
make_scpi_command("SENS:FREQ:STOP 10E9");     // 设置终止频率 10 GHz
make_scpi_command("SENS:FREQ:CENT 5.5E9");    // 设置中心频率
make_scpi_command("SENS:FREQ:SPAN 9E9");      // 设置频率跨度
make_scpi_command("SENS:FREQ:STAR?");         // 查询起始频率
```

#### 扫描配置
```cpp
make_scpi_command("SENS:SWE:POIN 201");       // 设置扫描点数
make_scpi_command("SENS:SWE:POIN?");          // 查询扫描点数
make_scpi_command("SENS:BAND 1000");          // 设置 IF 带宽
make_scpi_command("SENS:SWE:TIME 0.5");       // 设置扫描时间
```

#### 测量和数据
```cpp
make_scpi_command("CALC:PAR:DEF S11");        // 定义 S11 参数
make_scpi_command("CALC:PAR:DEF S21");        // 定义 S21 参数
make_scpi_command("CALC:FORM MLOG");          // 设置格式为对数幅度
make_scpi_command("CALC:DATA? FDAT");         // 读取格式化数据
make_scpi_command("CALC:DATA? SDAT");         // 读取复数数据
```

#### 触发和扫描
```cpp
make_scpi_command("INIT:CONT ON");            // 连续扫描
make_scpi_command("INIT:CONT OFF");           // 单次扫描模式
make_scpi_command("INIT:IMM");                // 立即触发扫描
make_scpi_command("*WAI");                    // 等待操作完成
```

## 设备配置要求

### VNA 网络设置
1. 确保 VNA 已连接到网络
2. 配置 VNA 的 IP 地址（通常在设备的系统设置中）
3. 启用 SCPI 远程控制功能
4. 默认 SCPI 端口通常是 **5025**

### 常见 VNA 设备
- Keysight/Agilent PNA 系列（E8361A, N5222A 等）
- Rohde & Schwarz ZVA/ZNB 系列
- Anritsu VectorStar 系列
- Copper Mountain 系列

## 故障排除

### 连接失败
```
✗ 连接失败: connection_failed(1001): Failed to connect to device
```
**解决方案：**
1. 检查 VNA IP 地址是否正确
2. 确认 VNA 和电脑在同一网络
3. 使用 ping 命令测试连接：`ping 192.168.1.100`
4. 检查防火墙设置
5. 验证端口号（通常是 5025）

### 地址无效
```
✗ 连接失败: address_invalid(1005): Invalid IP address
```
**解决方案：**
- 确认 IP 地址格式正确（例如：192.168.1.100）
- 不要使用主机名（除非您的系统支持 DNS 解析）

### 超时错误
```
✗ 命令执行失败: timeout(600)
```
**解决方案：**
1. 增加超时时间：
```cpp
transport_config_t config;
config.read_timeout = 5000;  // 5 秒
device->transport().set_config(config);
```
2. 检查 VNA 是否正在执行长时间操作
3. 确认网络连接稳定

### 命令错误
如果 VNA 返回错误，查询错误队列：
```cpp
auto result = device->execute(make_scpi_command("SYST:ERR?"));
if (result.has_value()) {
    std::cout << "错误: " << get_response_string(*result) << "\n";
}
```

## 进阶应用

### 1. 数据采集和绘图
```cpp
// 读取 S11 数据
auto result = device->execute(make_scpi_command("CALC:DATA? FDAT"));
if (result.has_value()) {
    std::string data = get_response_string(*result);
    
    // 解析逗号分隔的数据
    std::istringstream iss(data);
    std::vector<double> values;
    std::string token;
    while (std::getline(iss, token, ',')) {
        values.push_back(std::stod(token));
    }
    
    // 处理数据...
}
```

### 2. 自动化测试
```cpp
// 配置频率扫描
device->execute(make_scpi_command("SENS:FREQ:STAR 1E9"));
device->execute(make_scpi_command("SENS:FREQ:STOP 10E9"));
device->execute(make_scpi_command("SENS:SWE:POIN 201"));

// 循环测试多个器件
for (int i = 0; i < num_duts; i++) {
    std::cout << "测试器件 " << i << "\n";
    
    // 触发扫描
    device->execute(make_scpi_command("INIT:IMM"));
    device->execute(make_scpi_command("*WAI"));
    
    // 读取数据
    auto result = device->execute(make_scpi_command("CALC:DATA? FDAT"));
    
    // 保存结果...
}
```

### 3. 校准状态检查
```cpp
// 查询校准状态
auto result = device->execute(make_scpi_command("SENS:CORR:STAT?"));
if (result.has_value()) {
    std::string cal_status = get_response_string(*result);
    if (cal_status == "1") {
        std::cout << "校准已启用\n";
    } else {
        std::cout << "警告: 校准未启用\n";
    }
}
```

## 参考资料

- [SCPI 标准文档](https://www.ivifoundation.org/specifications.html)
- Keysight VNA 编程指南
- 您的 VNA 设备的 SCPI 命令参考手册

## 扩展建议

1. **添加数据解析** - 实现完整的 CSV/数组数据解析
2. **保存到文件** - 将测量数据保存为 Touchstone (.s2p) 格式
3. **图形界面** - 使用 Qt/GTK 创建 GUI
4. **多端口测试** - 同时控制多台 VNA
5. **自动校准** - 实现自动化校准流程

## 许可证

本示例遵循与 VDL 库相同的许可证。
