# VDL 库 VISA 通信设备适配方案

## 概览

本文档提供了 VDL 库如何优雅地适配 VISA（Virtual Instrument Software Architecture）通信设备的完整解决方案。VISA 是业界标准的仪器编程接口，支持 GPIB、TCP/IP、USB、Serial 等多种传输方式。

**关键优势**：
- 充分利用 VDL 现有的分层架构
- 无需修改库核心代码
- 支持所有 VISA 传输方式
- 代码复用率高
- 易于维护和扩展

---

## 1. VISA 通信基础

### 1.1 VISA 概述

VISA (Virtual Instrument Software Architecture) 是由 IVI 基金会定义的标准仪器编程接口。

**主要特点**：
- 统一的编程接口
- 支持多种物理接口（GPIB、TCP/IP、USB、Serial）
- 自动资源管理
- 内置超时和错误处理

**支持的传输方式**：
```
┌─────────────────────────────────────────┐
│        VISA 应用程序                     │
├─────────────────────────────────────────┤
│        VISA 库 (NI-VISA 或其他)         │
├─────────────────────────────────────────┤
│  GPIB Driver │ TCP/IP │ USB │ Serial   │
├─────────────────────────────────────────┤
│ 仪器硬件                                 │
└─────────────────────────────────────────┘
```

### 1.2 VISA 资源字符串

VISA 使用资源字符串标识仪器：

```
GPIB::192::INSTR              // GPIB 192 地址
GPIB0::1::INSTR               // GPIB 0 接口，地址 1
TCPIP::192.168.1.100::INSTR   // TCP/IP 192.168.1.100
TCPIP::vna.local::5025::SOCKET// TCP/IP 带端口
USB::0x0699::0x0341::INSTR    // USB 设备
ASRL1::INSTR                  // 串口 COM1
```

### 1.3 VISA 操作基础

标准 VISA 操作：
```cpp
viStatus viOpenDefaultRM()              // 打开资源管理器
viStatus viOpen()                       // 打开设备
viStatus viWrite()                      // 写数据
viStatus viRead()                       // 读数据
viStatus viReadToFile()                 // 读到文件
viStatus viWriteFromFile()              // 从文件写
viStatus viClose()                      // 关闭设备
```

---

## 2. VDL 架构回顾

### 2.1 当前分层设计

```
┌──────────────────────────────┐
│   应用程序                    │
├──────────────────────────────┤
│   设备适配器                  │
│  (vna_adapter_t 等)           │
├──────────────────────────────┤
│   协议适配器                  │
│  (scpi_adapter_t)             │
├──────────────────────────────┤
│   Device 实现                 │
│  (device_impl_t)              │
├──────────────────────────────┤
│  Transport  │  Codec          │
│ (抽象基类)  │ (抽象基类)      │
├──────────────────────────────┤
│  TCP/Serial │ SCPI/ModBus    │
│ (具体实现)  │ (具体实现)      │
└──────────────────────────────┘
```

### 2.2 关键接口

**Transport 接口** (`include/vdl/transport/transport.hpp`):
- `open()` - 打开连接
- `close()` - 关闭连接
- `write_all()` - 写数据
- `read()` - 读数据
- `is_open()` - 检查连接状态

**Codec 接口** (`include/vdl/codec/codec.hpp`):
- `encode()` - 编码命令
- `decode()` - 解码响应
- `frame_length()` - 获取帧长度

---

## 3. VISA 适配方案

### 3.1 适配架构设计

```
┌─────────────────────────────────────────┐
│     应用程序                            │
├─────────────────────────────────────────┤
│  visa_device_adapter_t (可选便利层)    │
│  ├─ 简化 VISA 特定操作                 │
│  └─ 内部使用 scpi_adapter_t            │
├─────────────────────────────────────────┤
│  scpi_adapter_t (既有库组件)            │
│  └─ 提供 SCPI 协议支持                 │
├─────────────────────────────────────────┤
│  device_impl_t (既有库组件)             │
│  └─ 组合 Transport + Codec             │
├─────────────────────────────────────────┤
│  visa_transport_t (新增 Transport)     │
│  └─ 包装 VISA 库调用                   │
├─────────────────────────────────────────┤
│  NI-VISA / 其他 VISA 库                │
│  └─ 实际硬件通信                       │
└─────────────────────────────────────────┘
```

### 3.2 设计原理

**为什么这个设计优雅？**

1. **最少修改原则**
   - VDL 库核心保持不变
   - 仅添加新的 Transport 实现
   - 完全复用 Codec 和 Device 层

2. **分离关注点**
   - Transport 层：负责 VISA 通信细节
   - Codec 层：负责 SCPI 协议（已有）
   - Device 层：组合两者
   - Adapter 层：提供便利接口

3. **最大化复用**
   - 复用 scpi_adapter_t 的所有功能
   - 复用 device_impl_t 的便利方法
   - 复用错误处理机制

4. **灵活性**
   - 支持所有 VISA 传输方式
   - 无需修改应用代码
   - 易于扩展

---

## 4. 实现方案

### 4.1 visa_transport_t 实现框架

```cpp
// File: examples/visa_transport.hpp

#ifndef VISA_TRANSPORT_HPP
#define VISA_TRANSPORT_HPP

#include <vdl/transport/transport.hpp>
#include <visa.h>
#include <string>
#include <memory>

/**
 * @brief VISA Transport 实现
 * 
 * 包装 NI-VISA 或其他 VISA 库的功能
 * 提供与 VDL Transport 接口兼容的 VISA 通信
 */
class visa_transport_t : public vdl::transport_base_t {
public:
    /**
     * @brief 构造函数
     * @param resource_string VISA 资源字符串
     *                        例如: "GPIB0::192::INSTR"
     *                               "TCPIP::192.168.1.100::INSTR"
     *                               "USB::0x0699::0x0341::INSTR"
     *                               "ASRL1::INSTR"
     * @param timeout_ms 操作超时时间（毫秒）
     */
    explicit visa_transport_t(
        const std::string& resource_string,
        uint32_t timeout_ms = 2000
    );

    virtual ~visa_transport_t();

    // Transport 接口实现
    vdl::result_t<void> open() override;
    void close() override;
    vdl::result_t<size_t> write_all(
        vdl::const_byte_span_t data,
        vdl::milliseconds_t timeout_ms
    ) override;
    vdl::result_t<vdl::bytes_t> read(
        size_t max_bytes,
        vdl::milliseconds_t timeout_ms
    ) override;
    bool is_open() const override;

    // VISA 特定方法
    vdl::result_t<std::string> get_idn() const;
    vdl::result_t<void> clear();
    vdl::result_t<void> set_timeout(uint32_t timeout_ms);

private:
    ViSession m_resource_manager = VI_NULL;
    ViSession m_instrument = VI_NULL;
    std::string m_resource_string;
    uint32_t m_timeout_ms = 2000;
    bool m_is_open = false;

    // 辅助方法
    std::string get_visa_error_string(ViStatus status) const;
};

#endif // VISA_TRANSPORT_HPP
```

### 4.2 使用示例

#### 基础使用（直接使用 SCPI 适配器）

```cpp
// File: examples/08_visa_scpi_example.cpp

#include "visa_transport.hpp"
#include <vdl/device/scpi_adapter.hpp>
#include <iostream>

int main() {
    try {
        // 1. 创建 VISA Transport
        auto transport = std::make_unique<visa_transport_t>(
            "TCPIP::192.168.1.100::INSTR",
            2000  // 2 秒超时
        );

        // 2. 创建 SCPI Codec（既有）
        auto codec = std::make_unique<vdl::binary_codec_t>();

        // 3. 创建设备
        vdl::device_impl_t device(std::move(transport), std::move(codec));
        device.connect();

        // 4. 使用 SCPI 适配器（既有库组件）
        vdl::scpi_adapter_t scpi(device);

        // 5. 执行 SCPI 命令
        auto idn = scpi.get_idn();
        if (idn) {
            std::cout << "仪器: " << *idn << std::endl;
        }

        scpi.reset();
        scpi.clear_status();

        // 6. 类型转换查询
        auto freq = scpi.query_double(":FREQ:CENT?");
        if (freq) {
            std::cout << "中心频率: " << (*freq / 1e9) << " GHz" << std::endl;
        }

        device.disconnect();
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
}
```

#### 高级使用（带便利适配器）

```cpp
// File: examples/visa_device_adapter.hpp

#include <vdl/device/scpi_adapter.hpp>
#include <string>

/**
 * @brief VISA 设备适配器
 * 
 * 在 scpi_adapter_t 基础上添加 VISA 特定功能
 */
class visa_device_adapter_t {
private:
    vdl::scpi_adapter_t m_scpi;
    
public:
    explicit visa_device_adapter_t(vdl::device_impl_t& device)
        : m_scpi(device) {}

    // VISA 特定操作
    vdl::result_t<std::string> query_idn() {
        return m_scpi.get_idn();
    }

    vdl::result_t<void> reset_device() {
        return m_scpi.reset();
    }

    vdl::result_t<void> clear_errors() {
        return m_scpi.clear_status();
    }

    // 频率控制（VNA 示例）
    vdl::result_t<void> set_frequency(double freq_hz) {
        return m_scpi.write(":FREQ:CENT " + std::to_string(freq_hz));
    }

    vdl::result_t<double> get_frequency() {
        return m_scpi.query_double(":FREQ:CENT?");
    }

    // 访问底层 SCPI 适配器
    vdl::scpi_adapter_t& scpi() { return m_scpi; }
};
```

使用高级适配器：

```cpp
#include "visa_transport.hpp"
#include "visa_device_adapter.hpp"

int main() {
    // 创建 VISA Transport
    auto transport = std::make_unique<visa_transport_t>(
        "GPIB0::192::INSTR"
    );
    auto codec = std::make_unique<vdl::binary_codec_t>();

    vdl::device_impl_t device(std::move(transport), std::move(codec));
    device.connect();

    // 使用高级适配器
    visa_device_adapter_t visa_device(device);

    auto idn = visa_device.query_idn();
    if (idn) {
        std::cout << "设备: " << *idn << std::endl;
    }

    visa_device.reset_device();
    visa_device.set_frequency(1e9);  // 1 GHz
    auto freq = visa_device.get_frequency();

    device.disconnect();
    return 0;
}
```

### 4.3 支持多种 VISA 传输方式

```cpp
// 创建不同类型的 VISA 连接

// GPIB 连接
auto transport1 = std::make_unique<visa_transport_t>("GPIB0::1::INSTR");

// TCP/IP 连接
auto transport2 = std::make_unique<visa_transport_t>(
    "TCPIP::192.168.1.100::INSTR"
);

// USB 连接
auto transport3 = std::make_unique<visa_transport_t>(
    "USB::0x0699::0x0341::INSTR"
);

// Serial 连接
auto transport4 = std::make_unique<visa_transport_t>("ASRL1::INSTR");

// 所有连接方式使用相同的代码：
vdl::device_impl_t device(std::move(transport), codec);
device.connect();
vdl::scpi_adapter_t scpi(device);
// ... 使用 scpi
```

---

## 5. VISA Transport 实现细节

### 5.1 关键方法实现

```cpp
// visa_transport.cpp 中的实现片段

#include "visa_transport.hpp"
#include <stdexcept>
#include <sstream>

visa_transport_t::visa_transport_t(
    const std::string& resource_string,
    uint32_t timeout_ms)
    : m_resource_string(resource_string),
      m_timeout_ms(timeout_ms) {}

visa_transport_t::~visa_transport_t() {
    close();
}

vdl::result_t<void> visa_transport_t::open() {
    try {
        // 打开 VISA 资源管理器
        ViStatus status = viOpenDefaultRM(&m_resource_manager);
        if (status != VI_SUCCESS) {
            return vdl::make_error<void>(
                vdl::error_code_t::communication_error,
                "Failed to open VISA resource manager"
            );
        }

        // 打开仪器
        status = viOpen(
            m_resource_manager,
            const_cast<ViRsrc>(m_resource_string.c_str()),
            VI_NULL,
            VI_NULL,
            &m_instrument
        );

        if (status != VI_SUCCESS) {
            viClose(m_resource_manager);
            return vdl::make_error<void>(
                vdl::error_code_t::communication_error,
                "Failed to open VISA instrument: " + 
                get_visa_error_string(status)
            );
        }

        // 设置超时
        status = viSetAttribute(
            m_instrument,
            VI_ATTR_TMO_VALUE,
            m_timeout_ms
        );

        m_is_open = true;
        return vdl::make_ok();

    } catch (const std::exception& e) {
        return vdl::make_error<void>(
            vdl::error_code_t::communication_error,
            std::string(e.what())
        );
    }
}

void visa_transport_t::close() {
    if (m_instrument != VI_NULL) {
        viClose(m_instrument);
        m_instrument = VI_NULL;
    }
    if (m_resource_manager != VI_NULL) {
        viClose(m_resource_manager);
        m_resource_manager = VI_NULL;
    }
    m_is_open = false;
}

vdl::result_t<size_t> visa_transport_t::write_all(
    vdl::const_byte_span_t data,
    vdl::milliseconds_t timeout_ms) {
    try {
        if (!is_open()) {
            return vdl::make_error<size_t>(
                vdl::error_code_t::communication_error,
                "VISA instrument not open"
            );
        }

        ViUInt32 written = 0;
        ViStatus status = viWrite(
            m_instrument,
            const_cast<ViByte*>(data.data()),
            static_cast<ViUInt32>(data.size()),
            &written
        );

        if (status != VI_SUCCESS) {
            return vdl::make_error<size_t>(
                vdl::error_code_t::communication_error,
                "VISA write failed: " + get_visa_error_string(status)
            );
        }

        return vdl::make_ok(written);

    } catch (const std::exception& e) {
        return vdl::make_error<size_t>(
            vdl::error_code_t::communication_error,
            std::string(e.what())
        );
    }
}

vdl::result_t<vdl::bytes_t> visa_transport_t::read(
    size_t max_bytes,
    vdl::milliseconds_t timeout_ms) {
    try {
        if (!is_open()) {
            return vdl::make_error<vdl::bytes_t>(
                vdl::error_code_t::communication_error,
                "VISA instrument not open"
            );
        }

        vdl::bytes_t buffer(max_bytes);
        ViUInt32 bytes_read = 0;

        ViStatus status = viRead(
            m_instrument,
            buffer.data(),
            static_cast<ViUInt32>(max_bytes),
            &bytes_read
        );

        if (status != VI_SUCCESS && 
            status != VI_ERROR_TMO) {  // 超时是可接受的
            return vdl::make_error<vdl::bytes_t>(
                vdl::error_code_t::communication_error,
                "VISA read failed: " + get_visa_error_string(status)
            );
        }

        buffer.resize(bytes_read);
        return vdl::make_ok(buffer);

    } catch (const std::exception& e) {
        return vdl::make_error<vdl::bytes_t>(
            vdl::error_code_t::communication_error,
            std::string(e.what())
        );
    }
}

bool visa_transport_t::is_open() const {
    return m_is_open && m_instrument != VI_NULL;
}

std::string visa_transport_t::get_visa_error_string(ViStatus status) const {
    // 返回 VISA 错误的可读字符串
    switch (status) {
        case VI_ERROR_CONN_LOST:
            return "Connection lost";
        case VI_ERROR_TIMEOUT:
            return "Operation timeout";
        case VI_ERROR_RSRC_NFOUND:
            return "Resource not found";
        case VI_ERROR_INV_RSRC:
            return "Invalid resource";
        // ... 更多错误代码
        default:
            return "VISA error code: " + std::to_string(status);
    }
}
```

---

## 6. 集成 CMake 配置

### 6.1 查找 VISA 库

```cmake
# cmake/FindVISA.cmake

find_path(VISA_INCLUDE_DIR
    NAMES visa.h
    PATHS
        /usr/include
        /usr/local/include
        "C:/Program Files/IVI Foundation/VISA/Win64/Include"
        "C:/Program Files/National Instruments/VISA/Win64/Include"
)

find_library(VISA_LIBRARY
    NAMES visa visa_lib
    PATHS
        /usr/lib
        /usr/local/lib
        "C:/Program Files/IVI Foundation/VISA/Win64/Lib"
        "C:/Program Files/National Instruments/VISA/Win64/Lib"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VISA
    REQUIRED_VARS VISA_LIBRARY VISA_INCLUDE_DIR
)

if(VISA_FOUND)
    add_library(VISA::VISA SHARED IMPORTED)
    set_target_properties(VISA::VISA PROPERTIES
        IMPORTED_LOCATION "${VISA_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${VISA_INCLUDE_DIR}"
    )
endif()
```

### 6.2 在 CMakeLists.txt 中配置

```cmake
# examples/CMakeLists.txt 中添加

# 尝试查找 VISA
find_package(VISA QUIET)

if(VISA_FOUND)
    # 添加 VISA 相关示例
    add_executable(08_visa_scpi_example 08_visa_scpi_example.cpp)
    target_link_libraries(08_visa_scpi_example
        PRIVATE
            vdl
            VISA::VISA
    )

    add_executable(09_visa_device_example 09_visa_device_example.cpp)
    target_link_libraries(09_visa_device_example
        PRIVATE
            vdl
            VISA::VISA
    )
else()
    message(STATUS "VISA not found - VISA examples will be skipped")
endif()
```

---

## 7. 最佳实践

### 7.1 错误处理

```cpp
// 总是检查 result_t
auto result = visa_device.get_frequency();
if (!result) {
    std::cerr << "Failed to get frequency: " 
              << result.error().message() << std::endl;
    return;
}

double freq = *result;
std::cout << "Frequency: " << freq << " Hz" << std::endl;
```

### 7.2 资源管理

```cpp
// 使用 device_guard_t 确保资源释放
class measurement_controller {
private:
    vdl::device_guard_t m_device_guard;
    vdl::device_impl_t& m_device;

public:
    measurement_controller(std::unique_ptr<vdl::i_transport_t> transport,
                          std::unique_ptr<vdl::i_codec_t> codec)
        : m_device_guard(std::move(transport), std::move(codec)),
          m_device(m_device_guard.device()) {}

    // 在析构时自动释放连接
    ~measurement_controller() = default;
};
```

### 7.3 支持不同的 VISA 配置

```cpp
class visa_config_t {
public:
    struct gpib_config_t {
        uint32_t board;
        uint32_t primary_address;
        uint32_t secondary_address;
    };

    struct tcpip_config_t {
        std::string host;
        uint16_t port;
        uint32_t timeout_ms;
    };

    struct usb_config_t {
        uint16_t vendor_id;
        uint16_t product_id;
        uint32_t timeout_ms;
    };

    // 从配置生成 VISA 资源字符串
    static std::string generate_resource_string(const gpib_config_t& cfg);
    static std::string generate_resource_string(const tcpip_config_t& cfg);
    static std::string generate_resource_string(const usb_config_t& cfg);
};
```

---

## 8. 对比不同的适配方式

### 8.1 方案对比表

| 方案 | 复杂度 | 复用度 | 文件位置 | 说明 |
|------|-------|-------|--------|------|
| **直接 VISA** | 低 | 高 | library | VISA Transport 放在库中，支持所有 SCPI 设备 |
| **VISA + 通用适配器** | 中 | 高 | library | 同上，并添加通用 VISA 适配器 |
| **VISA + 设备特定适配器** | 中 | 中 | examples | VISA Transport 在库，设备适配器在 examples |
| **完全定制** | 高 | 低 | user | 用户完全自定义，不使用库 |

**推荐**：第一种或第二种方案最优雅。

### 8.2 选择建议

**使用"直接 VISA"方案：**
- 只需要基础 SCPI 命令
- 支持多种设备
- 追求最小化库依赖

**使用"VISA + 通用适配器"方案：**
- 需要 VISA 特定功能
- 想要简化 API
- 需要一致的错误处理

---

## 9. 实现步骤

### 步骤 1：创建 VISA Transport

```bash
cp examples/visa_transport.hpp examples/visa_transport.hpp
# 实现 visa_transport_t 类的所有方法
```

### 步骤 2：编写示例

```bash
# 08_visa_scpi_example.cpp - 基础示例
# 09_visa_device_example.cpp - 高级示例
```

### 步骤 3：更新 CMake

添加 VISA 查找和条件编译

### 步骤 4：测试

```bash
cd build
cmake ..
make
./bin/08_visa_scpi_example
./bin/09_visa_device_example
```

### 步骤 5：文档

创建 VISA 相关文档和最佳实践指南

---

## 10. 总结

### 优势

✅ **无需修改库核心**
- 仅需添加一个 Transport 实现
- 库的接口和架构完全不变

✅ **最大化代码复用**
- 复用 scpi_adapter_t
- 复用 device_impl_t
- 复用所有现有功能

✅ **支持所有 VISA 传输**
- GPIB、TCP/IP、USB、Serial
- 用户无需考虑传输方式

✅ **易于维护和扩展**
- Transport 逻辑集中
- 易于添加新的 Transport 类型
- 易于添加新的设备适配器

✅ **符合开源最佳实践**
- 库保持纯净
- 示例展示用法
- 用户可参考编写自己的适配器

### 后续扩展

1. 添加 NI-DAQmx 支持（数据采集）
2. 添加异步操作支持
3. 添加连接池管理
4. 添加性能监控和日志

---

## 参考资源

- [IVI Foundation](https://www.ivifoundation.org/)
- [NI-VISA 文档](https://www.ni.com/en-us/support/documentation/field-installable-software/details/ni-visa.html)
- [SCPI 标准](http://www.ivifoundation.org/docs/scpi-99.pdf)
- VDL 库文档

---

**建议**：选择方案并开始实现。VISA Transport 是完全独立的，可以作为示例或库的一部分。

