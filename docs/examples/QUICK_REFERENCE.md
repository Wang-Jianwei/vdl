# VNA SCPI 通信快速参考

## 快速开始

### 1. 编译
```bash
cd /workspaces/vdl/build
cmake .. && make 06_vna_scpi_communication
```

### 2. 运行
```bash
# 方式 1: 直接运行
./bin/06_vna_scpi_communication 192.168.1.100 5025

# 方式 2: 使用测试脚本
cd examples
./test_vna.sh 192.168.1.100
```

## 核心代码片段

### 创建 VNA 连接
```cpp
#include <vdl/vdl.hpp>

// 1. 创建 TCP 传输
auto transport = std::unique_ptr<vdl::i_transport_t>(
    new vdl::tcp_transport_t("192.168.1.100", 5025)
);

// 2. 创建 SCPI 编解码器
auto codec = std::unique_ptr<vdl::i_codec_t>(
    new vdl::scpi_codec_t()
);

// 3. 创建设备
auto device = std::unique_ptr<vdl::i_device_t>(
    new vdl::device_impl_t(std::move(transport), std::move(codec))
);

// 4. 连接
device->connect();
```

### 发送命令
```cpp
// 查询命令
auto cmd = vdl::make_scpi_command("*IDN?");
auto result = device->execute(cmd);
if (result.has_value()) {
    std::string response = vdl::get_response_string(*result);
    std::cout << "设备: " << response << "\n";
}

// 设置命令
device->execute(vdl::make_scpi_command("SENS:FREQ:STAR 1E9"));
```

## 常用 SCPI 命令

| 功能 | 命令 | 说明 |
|------|------|------|
| 设备识别 | `*IDN?` | 查询设备制造商、型号等 |
| 复位 | `*RST` | 复位到出厂状态 |
| 等待 | `*WAI` | 等待所有命令完成 |
| 错误查询 | `SYST:ERR?` | 查询错误队列 |
| 设置起始频率 | `SENS:FREQ:STAR 1E9` | 1 GHz |
| 设置终止频率 | `SENS:FREQ:STOP 10E9` | 10 GHz |
| 设置扫描点数 | `SENS:SWE:POIN 201` | 201 点 |
| 设置 IF 带宽 | `SENS:BAND 1000` | 1 kHz |
| 设置 S11 | `CALC:PAR:DEF S11` | 测量 S11 |
| 读取数据 | `CALC:DATA? FDAT` | 格式化数据 |
| 单次扫描 | `INIT:IMM` | 触发单次扫描 |

## 故障排除

### 连接失败
```bash
# 测试网络
ping 192.168.1.100

# 测试端口
telnet 192.168.1.100 5025
# 或
nc -vz 192.168.1.100 5025
```

### 检查 VNA 设置
1. VNA 网络设置 → IP 地址
2. VNA 系统设置 → 远程接口 → SCPI
3. 端口通常是 5025

### 增加超时时间
```cpp
vdl::transport_config_t config;
config.read_timeout = 5000;  // 5秒
// 应用配置（需要在 device 创建前设置）
```

## 完整示例程序结构

```
06_vna_scpi_communication.cpp
├── tcp_transport_t           # TCP 传输层
│   ├── open()               # 连接
│   ├── close()              # 断开
│   ├── read()               # 读取
│   └── write()              # 写入
│
├── scpi_codec_t             # SCPI 编解码器
│   ├── encode()             # 编码命令
│   ├── decode()             # 解码响应
│   └── frame_length()       # 检测帧长
│
├── make_scpi_command()      # 辅助函数
├── get_response_string()    # 辅助函数
│
└── 示例函数
    ├── example_vna_identification()
    ├── example_vna_frequency_control()
    ├── example_vna_sweep_config()
    ├── example_vna_s_parameters()
    └── example_vna_error_checking()
```

## 支持的 VNA 设备

- ✓ Keysight/Agilent PNA 系列
- ✓ Rohde & Schwarz ZVA/ZNB 系列
- ✓ Anritsu VectorStar 系列
- ✓ Copper Mountain 系列
- ✓ 任何支持 SCPI over TCP/IP 的 VNA

## 扩展建议

1. **数据解析** - 解析 CSV 格式的测量数据
2. **保存数据** - 导出 Touchstone (.s2p) 文件
3. **批量测试** - 循环测试多个频点/器件
4. **自动化** - 集成到生产测试系统
5. **GUI** - 使用 Qt/GTK 创建图形界面

## 相关文档

- [VNA_SCPI_GUIDE.md](VNA_SCPI_GUIDE.md) - 完整使用指南
- [examples/README.md](README.md) - 示例编译指南
- VNA 设备的 SCPI 命令参考手册

## 注意事项

⚠️ 运行前确保：
- VNA 已开机并连接到网络
- IP 地址配置正确
- SCPI 服务已启用
- 防火墙允许端口 5025

💡 提示：
- 默认端口是 5025
- 某些 VNA 需要登录认证
- 大数据传输可能需要增加超时时间
- 建议先使用 `*IDN?` 测试连接
