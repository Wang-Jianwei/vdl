# VDL 适配器快速参考

## 三层架构快速选择表

```
┌─────────────────────────────────────────────────────────────┐
│ 应用需求                    │ 推荐使用         │ 优点      │
├─────────────────────────────────────────────────────────────┤
│ 跟任何 SCPI 设备通信         │ scpi_adapter_t  │ 通用、标准 │
│ 跟 VNA 设备通信               │ vna_adapter_t   │ 便捷、强类型│
│ 需要最小依赖                  │ device_impl_t   │ 简洁、兼容 │
│ 需要原始字节级控制            │ write_raw/read_raw│ 完全控制  │
└─────────────────────────────────────────────────────────────┘
```

## 常见任务示例

### 任务 1：查询 VNA 设备ID

```cpp
// ✅ 最推荐：使用设备便捷接口
device_impl_t device(transport, codec);
auto response = device.query("*IDN?");
std::cout << *response << "\n";

// ✅ 也可以：使用 SCPI 适配器
scpi_adapter_t scpi(device);
auto idn = scpi.get_idn();
```

### 任务 2：配置 VNA 频率

```cpp
// ✅ 最推荐：使用 VNA 适配器
vna_adapter_t vna(device);
vna.set_start_freq(1e9);       // 1 GHz
vna.set_stop_freq(6e9);        // 6 GHz

// ✅ 或者：使用设备接口
device.write("SENS:FREQ:STAR 1E9\n");
device.write("SENS:FREQ:STOP 6E9\n");
```

### 任务 3：执行 VNA 扫描

```cpp
// ✅ 最推荐：使用 VNA 适配器
vna_adapter_t vna(device);
vna.set_sweep_points(401);
vna.trigger_sweep();

// ✅ 或者：使用设备接口
device.write("SENS:SWE:POIN 401\n");
device.write("INIT:IMM\n");
```

### 任务 4：读取数据

```cpp
// ✅ 最推荐：使用 VNA 适配器 + SCPI 解析工具
vna_adapter_t vna(device);
auto data = vna.get_formatted_data_parsed();
if (data) {
    for (double value : *data) {
        std::cout << value << "\n";
    }
}

// ✅ 或者：使用手动解析
auto data_str = device.query("CALC:DATA? FDAT");
auto parsed = scpi_adapter_t::parse_data_doubles(*data_str);
```

### 任务 5：处理错误

```cpp
// ✅ 使用 SCPI 适配器的错误处理
scpi_adapter_t scpi(device);
auto errors = scpi.clear_all_errors();
if (errors && !errors->empty()) {
    for (const auto& err : *errors) {
        std::cerr << "Error: " << err << "\n";
    }
}
```

## 包含文件指南

```cpp
// 基础用法（总是需要）
#include <vdl/vdl.hpp>

// 如果使用 SCPI 适配器
#include <vdl/device/scpi_adapter.hpp>

// 如果使用 VNA 适配器
#include <vdl/device/vna_adapter.hpp>

// 完整的三层都用
#include <vdl/vdl.hpp>
#include <vdl/device/scpi_adapter.hpp>
#include <vdl/device/vna_adapter.hpp>
```

## API 速查表

### device_impl_t（设备基础接口）

```cpp
// 连接/断开
result_t<void> connect();
void disconnect();
bool is_connected() const;

// 低级 I/O
result_t<void> write_raw(const_byte_span_t data, milliseconds_t timeout = 0);
result_t<bytes_t> read_raw(size_t max_bytes, milliseconds_t timeout = 0);

// 中级 I/O
result_t<void> write(const std::string& text);
result_t<std::string> read(milliseconds_t timeout = 0);

// 高级 I/O
result_t<std::string> query(const std::string& command, milliseconds_t timeout = 0);
```

### scpi_adapter_t（协议适配器）

```cpp
// 连接
result_t<void> connect();
void disconnect();

// 命令执行
result_t<void> command(const std::string& cmd);
result_t<std::string> query(const std::string& cmd);

// 类型转换查询
result_t<double> query_double(const std::string& cmd);
result_t<int> query_int(const std::string& cmd);
result_t<bool> query_bool(const std::string& cmd);

// 标准 SCPI 命令
result_t<std::string> get_idn();
result_t<void> reset();
result_t<void> clear_status();
result_t<void> wait();
result_t<std::string> get_error();
result_t<std::vector<std::string>> clear_all_errors();

// 数据解析（静态方法）
static result_t<std::vector<double>> parse_data_doubles(const std::string& data);
static result_t<std::vector<std::pair<double,double>>> parse_complex_data(const std::string& data);
```

### vna_adapter_t（VNA 专用适配器）

```cpp
// 连接
result_t<void> connect();
void disconnect();

// 频率控制
result_t<void> set_start_freq(double freq_hz);
result_t<double> get_start_freq();
result_t<void> set_stop_freq(double freq_hz);
result_t<double> get_stop_freq();
result_t<void> set_center_freq(double freq_hz);
result_t<double> get_center_freq();

// 扫描参数
result_t<void> set_sweep_points(int points);
result_t<int> get_sweep_points();
result_t<void> set_if_bandwidth(double bw_hz);
result_t<double> get_if_bandwidth();

// 扫描控制
result_t<void> enable_continuous_sweep();
result_t<void> disable_continuous_sweep();
result_t<void> trigger_sweep();

// 测量配置
result_t<void> set_measurement_param(const std::string& param);
result_t<void> set_data_format(const std::string& format);

// 数据读取
result_t<std::string> get_formatted_data();
result_t<std::string> get_complex_data();
result_t<std::vector<double>> get_formatted_data_parsed();
result_t<std::vector<std::pair<double,double>>> get_complex_data_parsed();

// 设备管理
result_t<std::string> get_idn();
result_t<void> reset();
result_t<void> clear_status();
result_t<std::string> get_error();

// 底层访问
scpi_adapter_t& scpi();
```

## 错误处理模式

```cpp
// 模式 1：检查 has_value()
auto result = device.query("*IDN?");
if (result.has_value()) {
    std::cout << *result << "\n";
} else {
    std::cerr << result.error().to_string() << "\n";
}

// 模式 2：使用 if 转换（C++17）
if (auto response = device.query("*IDN?")) {
    std::cout << *response << "\n";
}

// 模式 3：直接访问（可能抛出异常）
try {
    auto response = device.query("*IDN?");
    std::cout << *response << "\n";
} catch (...) {
    std::cerr << "Query failed\n";
}
```

## 常见问题

### Q：何时使用 vna_adapter vs. device.write/query？

**A：**
- 如果需要类型转换和验证 → 用 vna_adapter
- 如果只是发送原始命令 → 用 device.write/query
- 如果在库级代码中 → 用 device (最兼容)

### Q：SCPI 适配器能用于其他设备吗？

**A：** 是的。任何使用 SCPI 协议的设备都可以用 scpi_adapter：
```cpp
scpi_adapter_t scpi(device);
scpi.query("*IDN?");  // 任何 SCPI 设备都有这个命令
```

### Q：如何添加自定义命令？

**A：** 直接使用 device 或 scpi_adapter 的 command/query：
```cpp
// 这个命令不在 vna_adapter 中
auto result = device.query("CUSTOM:COMMAND?");

// 或
auto result = scpi.command("CUSTOM:COMMAND");
```

### Q：vna_adapter 需要 TCP 传输吗？

**A：** 不需要。vna_adapter 可以和任何传输层配合（TCP、USB、GPIB 等）：
```cpp
// 可以是任何传输实现
vna_adapter_t vna(device_with_usb_transport);
vna_adapter_t vna(device_with_gpib_transport);
vna_adapter_t vna(device_with_any_transport);
```

## 最佳实践

1. **优先使用高层适配器** - 更便捷、更安全
2. **遵循库的分层** - 不要跳过中间层直接调用底层
3. **处理所有返回值** - 使用 result_t 的错误处理
4. **保持适配器专一** - VNA 特定功能在 vna_adapter
5. **定期检查错误队列** - 使用 scpi.clear_all_errors()

## 相关文档

- 详细设计文档：[ADAPTER_DESIGN.md](ADAPTER_DESIGN.md)
- 优化总结：[ADAPTER_DESIGN_SUMMARY.md](ADAPTER_DESIGN_SUMMARY.md)
- VNA 使用指南：[VNA_SCPI_GUIDE.md](examples/VNA_SCPI_GUIDE.md)
