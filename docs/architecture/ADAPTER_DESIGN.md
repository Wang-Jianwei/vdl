# VDL 适配器设计文档

## 设计原则

VDL 库采用**分层适配器模式**来保持库的通用性和可扩展性。

### 核心设计理念

1. **库级通用性** - VDL 核心库保持协议和设备无关，只提供通用的传输和编解码接口
2. **协议特定性** - 通过适配器层为特定协议（如 SCPI）提供便捷接口
3. **设备特定性** - 为特定设备类型（如 VNA）提供进一步的高级接口
4. **组合优于继承** - 使用组合模式而非继承来实现功能扩展

## 适配器层次

### 第 1 层：设备基础接口 (device_impl_t)

**职责**：提供通用的低级和中级通信接口

**API**：
- `write_raw()` / `read_raw()` - 原始字节 I/O（绕过编解码器）
- `write()` / `read()` / `query()` - 文本协议支持

**特点**：
- 完全通用，不关心具体协议
- 支持任何基于文本的协议（SCPI, AT 命令, 自定义协议等）

```cpp
device_impl_t device(transport, codec);
device.connect();

// 低级 API
auto bytes = device.read_raw(100);

// 中级 API  
auto response = device.query("*IDN?");
device.write("RESET\n");
```

### 第 2 层：协议适配器 (scpi_adapter_t)

**文件**：`include/vdl/device/scpi_adapter.hpp`

**职责**：为 SCPI 协议提供通用便捷接口

**API**：
- `command()` / `query()` - 基本 SCPI 命令执行
- `query_double()` / `query_int()` / `query_bool()` - 类型转换查询
- `get_idn()` / `reset()` / `clear_status()` - 标准 SCPI 命令
- `get_error()` / `clear_all_errors()` - 错误处理
- `parse_data_doubles()` / `parse_complex_data()` - 数据解析工具

**特点**：
- 完全通用，适用于任何 SCPI 设备
- 不包含任何特定于某个设备的命令
- 支持数据解析和错误管理

```cpp
scpi_adapter_t scpi(device);

auto idn = scpi.get_idn();
auto freq = scpi.query_double("SENS:FREQ:STAR?");
scpi.command("*RST");
```

### 第 3 层：设备特定适配器 (vna_adapter_t)

**文件**：`include/vdl/device/vna_adapter.hpp`

**职责**：为 VNA 设备提供高级特定接口

**API**：
- `set_start_freq()` / `get_start_freq()` - 频率设置
- `set_sweep_points()` / `get_sweep_points()` - 扫描参数
- `set_if_bandwidth()` / `get_if_bandwidth()` - 中频带宽
- `trigger_sweep()` / `disable_continuous_sweep()` - 扫描控制
- `set_measurement_param()` / `set_data_format()` - 测量配置
- `get_formatted_data()` / `get_complex_data()` - 数据读取

**特点**：
- 仅包含 VNA 特定的命令和操作
- 内部使用 SCPI 适配器实现
- 提供强类型的便捷方法

```cpp
vna_adapter_t vna(device);

vna.set_start_freq(1e9);        // 1 GHz
vna.set_stop_freq(6e9);         // 6 GHz
vna.set_sweep_points(401);
vna.trigger_sweep();

auto data = vna.get_formatted_data();
```

## 使用示例

### 场景 1：通用 SCPI 设备通信

```cpp
#include <vdl/vdl.hpp>
#include <vdl/device/scpi_adapter.hpp>

// 创建设备
device_impl_t device(transport, codec);
device.connect();

// 使用 SCPI 适配器
scpi_adapter_t scpi(device);
scpi.reset();
scpi.clear_status();

auto response = scpi.query("*IDN?");
auto error = scpi.get_error();
```

### 场景 2：VNA 特定通信

```cpp
#include <vdl/vdl.hpp>
#include <vdl/device/vna_adapter.hpp>

// 创建设备
device_impl_t device(tcp_transport, scpi_codec);
device.connect();

// 使用 VNA 适配器
vna_adapter_t vna(device);

// 设置测量频率
vna.set_start_freq(100e6);      // 100 MHz
vna.set_stop_freq(1e9);         // 1 GHz

// 设置扫描参数
vna.set_sweep_points(201);
vna.set_if_bandwidth(1000);     // 1 kHz

// 执行测量
vna.trigger_sweep();

// 读取数据
auto data = vna.get_formatted_data_parsed();
if (data) {
    for (double value : *data) {
        std::cout << value << "\n";
    }
}
```

### 场景 3：原始 SCPI 命令

```cpp
// 当需要特定的 VNA 命令时，可以直接使用 SCPI 适配器或设备
device_impl_t device(transport, codec);

// 通过设备接口使用原始接口
device.write("CALC:PAR:DEF S21\n");      // 定义参数
device.write("CALC:FORM MLOG\n");        // 设置格式

// 或通过 SCPI 适配器的通用命令方法
scpi_adapter_t scpi(device);
scpi.command("DISP:ENAB 1");
```

## 适配器选择指南

| 场景 | 推荐使用 | 原因 |
|------|--------|------|
| 与任何 SCPI 设备通信 | `scpi_adapter_t` | 通用、标准化 |
| 与特定 VNA 设备通信 | `vna_adapter_t` | 类型安全、便捷 |
| 跨不同设备类型操作 | `device_impl_t` | 最大兼容性 |
| 需要原始字节控制 | `device_impl_t::write_raw/read_raw()` | 最低级别控制 |

## 扩展指南

### 创建新的协议适配器

例如为 Modbus 协议创建适配器：

```cpp
class modbus_adapter_t {
public:
    explicit modbus_adapter_t(device_impl_t& device)
        : m_device(device) {}
    
    result_t<uint16_t> read_holding_register(uint16_t addr);
    result_t<void> write_single_register(uint16_t addr, uint16_t value);
    // ...
    
private:
    device_impl_t& m_device;
};
```

### 创建新的设备特定适配器

例如为其他 VNA 创建特定适配器（如 HP8753）：

```cpp
class hp8753_adapter_t {
public:
    explicit hp8753_adapter_t(device_impl_t& device)
        : m_scpi(device) {}
    
    // HP8753 特定的功能
    result_t<void> set_cal_type(const std::string& type);
    result_t<std::string> get_measurement_status();
    
    // 访问底层 SCPI 适配器
    scpi_adapter_t& scpi() { return m_scpi; }
    
private:
    scpi_adapter_t m_scpi;
};
```

## 架构优势

1. **关注点分离** - 每层只负责自己的职责
2. **代码重用** - VNA 适配器重用 SCPI 适配器
3. **库兼容性** - 核心库保持通用，无特定设备依赖
4. **易于测试** - 每层可独立测试
5. **易于扩展** - 添加新适配器不影响现有代码
6. **类型安全** - 高层适配器提供类型检查

## 编译注意事项

- `scpi_adapter.hpp` 和 `vna_adapter.hpp` 是头文件，需要包含在使用处
- 使用时避免在 `vdl.hpp` 中直接包含这些适配器（可能导致模板实例化问题）
- 在需要的源文件中明确包含所需的适配器

```cpp
#include <vdl/vdl.hpp>
#include <vdl/device/scpi_adapter.hpp>  // 根据需要包含
#include <vdl/device/vna_adapter.hpp>
```

## 总结

VDL 的适配器设计遵循**分层、组合、通用**的原则：

```
应用代码
    ↓
设备特定适配器 (vna_adapter_t) ← VNA 特定功能
    ↓
协议适配器 (scpi_adapter_t) ← SCPI 通用功能
    ↓
设备通用接口 (device_impl_t) ← 传输+编解码
    ↓
传输层 + 编解码器
```

这种设计确保了库的通用性，同时为具体应用提供了便捷的高级接口。
