# VNA SCPI 通信示例

这是一个完整的 VNA（矢量网络分析仪）SCPI 通信示例，展示了如何使用 VDL 库通过 TCP/IP 与 VNA 设备进行通信和测试。

## 特点

✅ **完整实现**
- TCP 传输层（`tcp_transport_t`）
- SCPI 编解码器（`scpi_codec_t`）
- 跨平台支持（Linux/macOS/Windows）

✅ **实用示例**
- 设备识别查询
- 频率范围设置
- 扫描参数配置
- S 参数测量
- 错误检查

✅ **即用型代码**
- 可以直接复制到您的项目
- 详细的注释和说明
- 完整的错误处理

## 快速开始

### 1️⃣ 编译
```bash
cd /workspaces/vdl/build
cmake .. && make
```

### 2️⃣ 运行
```bash
# 指定 VNA IP 地址
./bin/06_vna_scpi_communication 192.168.1.100 5025

# 或使用测试脚本
cd examples
./test_vna.sh 192.168.1.100
```

### 3️⃣ 示例输出
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
...
```

## 核心代码

```cpp
#include <vdl/vdl.hpp>

// 创建连接
auto transport = std::unique_ptr<vdl::i_transport_t>(
    new vdl::tcp_transport_t("192.168.1.100", 5025));
auto codec = std::unique_ptr<vdl::i_codec_t>(
    new vdl::scpi_codec_t());
auto device = std::unique_ptr<vdl::i_device_t>(
    new vdl::device_impl_t(std::move(transport), std::move(codec)));

device->connect();

// 发送 SCPI 命令
auto cmd = vdl::make_scpi_command("*IDN?");
auto result = device->execute(cmd);
if (result.has_value()) {
    std::string response = vdl::get_response_string(*result);
    std::cout << response << "\n";
}
```

## 常用命令

| 功能 | SCPI 命令 |
|------|----------|
| 识别设备 | `*IDN?` |
| 设置频率 | `SENS:FREQ:STAR 1E9` |
| 扫描点数 | `SENS:SWE:POIN 201` |
| 读取数据 | `CALC:DATA? FDAT` |
| 查询错误 | `SYST:ERR?` |

## 文档

📖 **详细指南：** [VNA_SCPI_GUIDE.md](VNA_SCPI_GUIDE.md)
📋 **快速参考：** [QUICK_REFERENCE.md](QUICK_REFERENCE.md)
🔧 **测试脚本：** [test_vna.sh](test_vna.sh)

## 支持的设备

- Keysight/Agilent PNA 系列
- Rohde & Schwarz ZVA/ZNB 系列
- Anritsu VectorStar 系列
- Copper Mountain 系列
- 任何支持 SCPI over TCP/IP 的 VNA

## 注意事项

⚠️ 确保：
- VNA IP 地址正确
- SCPI 服务已启用
- 端口号正确（通常是 5025）
- 网络连接正常

💡 提示：先运行 `test_vna.sh` 脚本验证网络连接！

## 许可证

遵循 VDL 库的许可证条款。
