# 虚拟设备层 VDL 设计方案 v3 （Vibe Coding）

## 一、设计理念

### 1.1 核心目标

VDL (Virtual Device Layer) 是一个**分离业务逻辑与设备 I/O** 的通用设备访问框架。

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              设计核心理念                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  🎯 分层隔离                                                                 │
│  ├─ Device（设备层）：面向业务，提供高级命令接口                              │
│  ├─ Codec（编解码层）：负责 Command ↔ 字节流的转换                           │
│  └─ Transport（传输层）：负责底层 I/O，读写原始字节                           │
│                                                                             │
│  📐 关键原则                                                                 │
│  ├─ 组合优于继承：Device 组合 Transport，而非继承                            │
│  ├─ 接口隔离：每层只暴露必要的接口                                           │
│  ├─ 依赖倒置：上层依赖抽象接口，不依赖具体实现                                │
│  ├─ 单一职责：每个类只做一件事                                               │
│  └─ 零开销抽象：不为不使用的功能付出代价                                      │
│                                                                             │
│  🔧 技术选型                                                                 │
│  ├─ C++11 标准 (兼容 MinGW/GCC/Clang/MSVC)                                  │
│  ├─ CMake 构建系统                                                          │
│  ├─ tl::expected<T, error_t> 错误处理                                       │
│  ├─ tl::optional<T> 可选值                                                  │
│  ├─ RAII 资源管理                                                           │
│  └─ 无 Boost 依赖                                                           │
│                                                                             │
│  📝 代码规范                                                                 │
│  ├─ 头文件保护：#ifndef VDL_XXX_HPP / #define / #endif                       │
│  ├─ 命名风格：全部蛇形命名法 (snake_case)                                    │
│  ├─ 成员变量：m_ 前缀 (如 m_buffer)                                          │
│  ├─ 静态变量：s_ 前缀 (如 s_instance)                                        │
│  ├─ 类型定义：_t 后缀 (如 byte_t, size_t, error_t)                           │
│  └─ 私有函数：_ 前缀 (如 _do_read())                                         │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 1.2 层次架构

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                业务应用层                                    │
│                           (Application Layer)                               │
└─────────────────────────────────┬───────────────────────────────────────────┘
                                  │ execute(Command) → Response
                                  ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                                 设备层                                       │
│                              (Device Layer)                                 │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  IDevice 接口                                                          │  │
│  │  ├─ open() / close()         生命周期管理                              │  │
│  │  ├─ execute(Command)         执行业务命令                              │  │
│  │  ├─ state()                  设备状态                                  │  │
│  │  ├─ lock() / tryLock()       独占访问                                  │  │
│  │  └─ DeviceImpl               组合 Transport + Codec                    │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────┬───────────────────────────────────────────┘
                                  │ encode/decode
                                  ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                                编解码层                                      │
│                              (Codec Layer)                                  │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  ICodec 接口                                                           │  │
│  │  ├─ encode(Command) → Bytes    命令编码                                │  │
│  │  ├─ decode(Bytes) → Response   响应解码                                │  │
│  │  └─ frameSize(header) → size   帧长度推断                              │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────┬───────────────────────────────────────────┘
                                  │ read/write bytes
                                  ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                                传输层                                        │
│                            (Transport Layer)                                │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  ITransport 接口                                                       │  │
│  │  ├─ open() / close()         连接管理                                  │  │
│  │  ├─ read(buffer, timeout)    读取字节                                  │  │
│  │  ├─ write(data, timeout)     写入字节                                  │  │
│  │  └─ isOpen()                 连接状态                                  │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  具体实现:                                                                   │
│  ├─ TcpTransport        TCP/IP 通信                                         │
│  ├─ SerialTransport     串口通信                                            │
│  ├─ MockTransport       测试用 Mock                                         │
│  └─ PcieTransport       PCIe 多通道 [Phase 2]                               │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 1.3 关键设计决策

| 决策点 | 选择 | 理由 |
|--------|------|------|
| Device 与 Transport 关系 | **组合** | Device 持有 Transport 指针，可运行时替换 |
| Command/Response vs Message | **分离** | 语义清晰，类型安全 |
| 错误处理 | `tl::expected<T, error_t>` | 不使用异常，可组合 |
| 线程模型 | 调用线程 + 可选心跳线程 | 简单可控 |
| 配置传递 | URI + Config 结构体 | 灵活且类型安全 |

---

## 二、核心接口定义

### 2.1 传输层接口 (i_transport_t)

```cpp
// include/vdl/transport/transport.hpp

#ifndef VDL_TRANSPORT_TRANSPORT_HPP
#define VDL_TRANSPORT_TRANSPORT_HPP

class i_transport_t {
public:
    virtual ~i_transport_t() = default;

    // 连接管理
    virtual result_t<void> open() = 0;
    virtual result_t<void> close() = 0;
    virtual bool is_open() const = 0;

    // 数据传输
    virtual result_t<size_t> write(const_byte_span_t data, milliseconds_t timeout) = 0;
    virtual result_t<size_t> read(byte_span_t buffer, milliseconds_t timeout) = 0;

    // 缓冲控制
    virtual result_t<void> flush() = 0;
    virtual size_t bytes_available() const = 0;
};

#endif // VDL_TRANSPORT_TRANSPORT_HPP
```

**职责边界**：
- ✅ 建立/断开物理连接
- ✅ 读写原始字节流
- ✅ 超时控制
- ❌ 不理解消息格式
- ❌ 不处理业务逻辑

### 2.2 编解码接口 (i_codec_t)

```cpp
// include/vdl/codec/codec.hpp

#ifndef VDL_CODEC_CODEC_HPP
#define VDL_CODEC_CODEC_HPP

class i_codec_t {
public:
    virtual ~i_codec_t() = default;

    // 编码：command_t → bytes_t
    virtual result_t<bytes_t> encode(const command_t& cmd) = 0;

    // 解码：bytes_t → response_t
    virtual result_t<response_t> decode(const_byte_span_t data) = 0;

    // 帧探测：根据已读取的头部，推断完整帧长度
    virtual result_t<size_t> frame_size(const_byte_span_t header) = 0;

    // 最小头部长度
    virtual size_t min_header_size() const = 0;
};

#endif // VDL_CODEC_CODEC_HPP
```

**职责边界**：
- ✅ Command 对象 → 字节流
- ✅ 字节流 → Response 对象
- ✅ 帧边界检测
- ❌ 不知道如何传输
- ❌ 不处理业务逻辑

### 2.3 设备接口 (i_device_t)

```cpp
// include/vdl/device/device.hpp

#ifndef VDL_DEVICE_DEVICE_HPP
#define VDL_DEVICE_DEVICE_HPP

class i_device_t {
public:
    virtual ~i_device_t() = default;

    // 生命周期
    virtual result_t<void> open() = 0;
    virtual result_t<void> close() = 0;
    virtual device_state_t state() const = 0;

    // 命令执行
    virtual result_t<response_t> execute(const command_t& cmd, milliseconds_t timeout) = 0;

    // 独占访问
    virtual optional_t<device_guard_t> try_lock() = 0;
    virtual result_t<device_guard_t> lock(milliseconds_t timeout) = 0;

    // 设备信息
    virtual const device_info_t& info() const = 0;
};

#endif // VDL_DEVICE_DEVICE_HPP
```

**职责边界**：
- ✅ 管理设备生命周期
- ✅ 执行业务命令
- ✅ 线程安全访问
- ✅ 状态管理
- ❌ 不关心传输细节
- ❌ 不关心编码细节

### 2.4 command_t 与 response_t

```cpp
// include/vdl/protocol/command.hpp

#ifndef VDL_PROTOCOL_COMMAND_HPP
#define VDL_PROTOCOL_COMMAND_HPP

struct command_t {
    uint16_t m_code;            // 命令码
    uint32_t m_id;              // 命令ID（用于追踪）
    bytes_t m_payload;          // 命令参数

    // 便捷构造
    static command_t make(uint16_t code, bytes_t payload = {});
    static command_t query(uint16_t code);     // 查询命令
    static command_t control(uint16_t code);   // 控制命令
};

#endif // VDL_PROTOCOL_COMMAND_HPP

// include/vdl/protocol/response.hpp

#ifndef VDL_PROTOCOL_RESPONSE_HPP
#define VDL_PROTOCOL_RESPONSE_HPP

struct response_t {
    uint16_t m_code;            // 响应码
    uint32_t m_command_id;      // 对应的命令ID
    status_t m_status;          // 执行状态 (ok, error, timeout, ...)
    bytes_t m_payload;          // 响应数据

    bool is_success() const { return m_status == status_t::ok; }
    bool is_error() const { return m_status == status_t::error; }

    // 便捷解析
    template<typename T>
    result_t<T> as() const;     // 将 payload 解析为类型 T
};

// 状态枚举
enum class status_t : uint8_t {
    ok = 0,
    error = 1,
    timeout = 2,
    busy = 3,
    invalid_command = 4,
    not_supported = 5
};

#endif // VDL_PROTOCOL_RESPONSE_HPP
```

---

## 三、状态设计

### 3.1 设备状态 (device_state_t)

```
┌──────────────────────────────────────────────────────────────────┐
│  状态          描述               可执行操作                      │
├──────────────────────────────────────────────────────────────────┤
│  closed       初始/已关闭         open                           │
│  opening      打开中              (等待)                          │
│  ready        就绪，可执行命令    execute, lock, close            │
│  busy         命令执行中          (等待完成)                       │
│  error        错误状态            reset, close                    │
└──────────────────────────────────────────────────────────────────┘
```

### 3.2 状态转换图

```
                    ┌────────┐
        ┌──────────►│ Closed │◄──────────┐
        │           └───┬────┘           │
        │               │ open()         │ close()
        │               ▼                │
        │          ┌─────────┐           │
        │          │ Opening │───────────┼───► Error
        │          └────┬────┘ (失败)    │       │
        │               │ (成功)         │       │ reset()
        │               ▼                │       ▼
        │          ┌─────────┐           │  ┌────────┐
   close()         │  Ready  │◄──────────┼──│ Closed │
        │          └────┬────┘           │  └────────┘
        │               │                │
        │   ┌───────────┼───────────┐    │
        │   │ execute() │           │    │
        │   ▼           │           ▼    │
        │ ┌──────┐      │      ┌────────┐│
        └─│ Busy │◄─────┘      │(Locked)││ (逻辑状态)
          └──┬───┘             └───┬────┘│
             │ (完成)              │     │
             └─────────────────────┴─────┘
                       ▼
                  ┌─────────┐
                  │  Ready  │
                  └─────────┘
```

### 3.3 传输层状态 (TransportState)

```cpp
enum class TransportState {
    Closed,         // 未打开
    Opening,        // 正在建立连接
    Open,           // 已打开，可读写
    Error           // 错误状态
};
```

---

## 四、错误处理体系

### 4.1 错误码分段

| 范围 | 类别 | 示例 |
|------|------|------|
| 0 | 成功 | Ok |
| 1000-1999 | 传输层 | ConnectionFailed, Timeout, ReadError, WriteError |
| 2000-2999 | 编解码层 | InvalidFrame, ChecksumError, DecodeFailed |
| 3000-3999 | 设备层 | NotOpen, Busy, LockConflict, InvalidState |
| 4000-4999 | 协议层 | InvalidCommand, UnsupportedVersion |
| 5000-5999 | 应用层 | InvalidArgument, ResourceNotFound |

### 4.2 error_t 类

```cpp
// include/vdl/core/error.hpp

#ifndef VDL_CORE_ERROR_HPP
#define VDL_CORE_ERROR_HPP

class error_t {
public:
    error_t(error_code_t code, std::string message = "");

    error_code_t code() const;
    error_category_t category() const;   // 由 code 推导
    const std::string& message() const;

    // 上下文链式添加
    error_t& with_context(std::string ctx);

    // 转换为字符串
    std::string to_string() const;

    // 便捷构造
    static error_t transport(error_code_t code, std::string msg);
    static error_t codec(error_code_t code, std::string msg);
    static error_t device(error_code_t code, std::string msg);

private:
    error_code_t m_code;
    std::string m_message;
    std::string m_context;
};

// Result 类型别名
template<typename T>
using result_t = tl::expected<T, error_t>;

template<typename T>
using optional_t = tl::optional<T>;

#endif // VDL_CORE_ERROR_HPP
```

---

## 五、线程模型

### 5.1 架构图

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│   User Thread 1 ──┬── execute() ──► ┌──────────┐           │
│                   │                 │          │           │
│   User Thread 2 ──┼── lock() ─────► │  Device  │◄── mutex  │
│                   │                 │          │           │
│   User Thread 3 ──┘                 └────┬─────┘           │
│                                          │                 │
│                                          ▼                 │
│   Heartbeat Thread ◄───── pauseFlag ─────┘ (可选)          │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 5.2 线程安全边界

| 组件 | 线程安全 | 说明 |
|------|----------|------|
| i_device_t | ✅ 是 | 内部 mutex 保护 |
| device_guard_t | ❌ 否 | 单线程持有 |
| i_transport_t | ❌ 否 | 由 device mutex 保护 |
| i_codec_t | ✅ 是 | 无状态，或内部同步 |

### 5.3 独占访问 (device_guard_t)

```cpp
// include/vdl/device/device_guard.hpp

#ifndef VDL_DEVICE_DEVICE_GUARD_HPP
#define VDL_DEVICE_DEVICE_GUARD_HPP

class device_guard_t {
public:
    device_guard_t(device_guard_t&& other) noexcept;
    device_guard_t& operator=(device_guard_t&& other) noexcept;
    ~device_guard_t();  // 自动释放锁

    // 禁止复制
    device_guard_t(const device_guard_t&) = delete;
    device_guard_t& operator=(const device_guard_t&) = delete;

    // 在独占期间执行命令
    result_t<response_t> execute(const command_t& cmd, milliseconds_t timeout);

    // 显式释放
    void release();

    // 检查是否有效
    bool is_valid() const;

private:
    friend class device_impl_t;
    device_guard_t(device_impl_t* device);
    
    device_impl_t* m_device;
};

#endif // VDL_DEVICE_DEVICE_GUARD_HPP
```

---

## 六、心跳设计

### 6.1 配置

```cpp
struct heartbeat_config_t {
    milliseconds_t m_interval{1000};     // 心跳间隔
    milliseconds_t m_timeout{500};       // 单次超时
    int m_max_failures{3};               // 连续失败阈值
    bool m_pause_during_lock{true};      // 独占期间暂停
};
```

### 6.2 策略接口

```cpp
class i_heartbeat_strategy_t {
public:
    virtual ~i_heartbeat_strategy_t() = default;

    // 生成心跳命令
    virtual command_t make_heartbeat_command() = 0;

    // 验证心跳响应
    virtual bool validate_response(const response_t& resp) = 0;
};
```

### 6.3 内置策略

| 策略 | 用途 |
|------|------|
| `ping_heartbeat_t` | 发送空命令，检查任意响应 |
| `echo_heartbeat_t` | 发送数据，验证回显 |
| `scpi_heartbeat_t` | 发送 `*IDN?`，验证响应 |

---

## 七、URI 设计

### 7.1 格式

```
scheme://[user:pass@]host[:port][/path][?query]
```

### 7.2 支持的 Scheme

| Scheme | 格式 | 示例 |
|--------|------|------|
| tcp | `tcp://host:port` | `tcp://192.168.1.100:5025` |
| serial | `serial:///dev/ttyUSB0?baud=115200` | `serial:///dev/ttyUSB0?baud=115200&parity=N` |
| mock | `mock://name` | `mock://test?latency=10` |
| pcie | `pcie://BDF?channel=0` | `pcie://0000:03:00.0?channel=0,1` |

---

## 八、目录结构

```
vdl/
│
├── include/vdl/
│   │
│   ├── core/                           # ══════ 核心基础设施 ══════
│   │   ├── types.hpp                   # Bytes, Size, Span, Milliseconds
│   │   ├── error.hpp                   # Error, ErrorCode, Res<T>
│   │   ├── noncopyable.hpp             # NonCopyable, NonMovable
│   │   ├── scope_guard.hpp             # ScopeGuard
│   │   ├── compat.hpp                  # C++17 兼容
│   │   ├── buffer.hpp                  # 缓冲区管理
│   │   ├── memory.hpp                  # 内存工具
│   │   └── logging.hpp                 # 日志接口
│   │
│   ├── uri/                            # ══════ 资源寻址 ══════
│   │   ├── uri.hpp                     # Uri 类
│   │   ├── uri_parser.hpp              # 解析器
│   │   └── query_params.hpp            # 查询参数
│   │
│   ├── transport/                      # ══════ 传输层 ══════
│   │   ├── transport.hpp               # ITransport 接口
│   │   ├── transport_factory.hpp       # 工厂
│   │   ├── tcp_transport.hpp           # TCP 实现
│   │   ├── serial_transport.hpp        # 串口实现
│   │   └── mock_transport.hpp          # Mock 实现
│   │
│   ├── codec/                          # ══════ 编解码层 ══════
│   │   ├── codec.hpp                   # ICodec 接口
│   │   ├── binary_codec.hpp            # 二进制编解码
│   │   └── scpi_codec.hpp              # SCPI 编解码 (可选)
│   │
│   ├── protocol/                       # ══════ 协议定义 ══════
│   │   ├── command.hpp                 # Command 结构
│   │   ├── response.hpp                # Response 结构
│   │   └── status.hpp                  # Status 枚举
│   │
│   ├── device/                         # ══════ 设备层 ══════
│   │   ├── device.hpp                  # IDevice 接口
│   │   ├── device_impl.hpp             # 设备实现
│   │   ├── device_state.hpp            # 设备状态
│   │   ├── device_guard.hpp            # 独占守卫
│   │   ├── device_config.hpp           # 设备配置
│   │   └── device_info.hpp             # 设备信息
│   │
│   ├── heartbeat/                      # ══════ 心跳 ══════
│   │   ├── heartbeat_config.hpp        # 配置
│   │   ├── heartbeat_strategy.hpp      # 策略接口
│   │   ├── heartbeat_runner.hpp        # 运行器
│   │   └── strategies/                 # 内置策略
│   │       ├── ping_heartbeat.hpp
│   │       ├── echo_heartbeat.hpp
│   │       └── scpi_heartbeat.hpp
│   │
│   ├── manager/                        # ══════ 设备管理 ══════
│   │   ├── device_factory.hpp          # 设备工厂
│   │   ├── device_registry.hpp         # 设备注册表
│   │   └── device_manager.hpp          # 管理器
│   │
│   └── vdl.hpp                         # 统一头文件
│
├── src/
│   ├── core/
│   │   ├── error.cpp
│   │   └── buffer.cpp
│   │
│   ├── uri/
│   │   └── uri_parser.cpp
│   │
│   ├── transport/
│   │   ├── transport_factory.cpp
│   │   ├── tcp_transport.cpp
│   │   ├── serial_transport.cpp
│   │   └── mock_transport.cpp
│   │
│   ├── codec/
│   │   └── binary_codec.cpp
│   │
│   ├── device/
│   │   ├── device_impl.cpp
│   │   └── device_guard.cpp
│   │
│   └── heartbeat/
│       └── heartbeat_runner.cpp
│
├── tests/
│   ├── unit/
│   │   ├── test_error.cpp
│   │   ├── test_buffer.cpp
│   │   ├── test_uri_parser.cpp
│   │   ├── test_tcp_transport.cpp
│   │   ├── test_mock_transport.cpp
│   │   ├── test_binary_codec.cpp
│   │   ├── test_device_impl.cpp
│   │   ├── test_device_guard.cpp
│   │   └── test_heartbeat.cpp
│   │
│   ├── integration/
│   │   ├── test_device_lifecycle.cpp
│   │   ├── test_concurrent_access.cpp
│   │   └── test_heartbeat_recovery.cpp
│   │
│   ├── mocks/
│   │   ├── mock_transport.hpp
│   │   └── mock_codec.hpp
│   │
│   └── test_main.cpp
│
├── examples/
│   ├── 01_basic_usage/
│   ├── 02_device_guard/
│   ├── 03_heartbeat/
│   ├── 04_custom_codec/
│   └── 05_multi_device/
│
├── third_party/
│   ├── tl_expected/
│   ├── tl_optional/
│   ├── catch2/
│   ├── nlohmann_json/
│   └── spdlog/
│
├── cmake/
│   ├── CompilerOptions.cmake
│   ├── FindDependencies.cmake
│   └── VdlConfig.cmake.in
│
├── CMakeLists.txt
├── .clang-format
├── .gitignore
├── LICENSE
└── README.md
```

---

## 九、使用示例

本项目在 `examples/` 目录中提供了多个完整的使用示例，展示了库的各种功能：

### 示例列表

1. **[examples/01_basic_usage.cpp](examples/01_basic_usage.cpp)** - 基础用法
   - 传输层创建
   - 设备创建和初始化
   - 连接管理
   - 基本命令执行
   - 状态查询

2. **[examples/02_error_handling.cpp](examples/02_error_handling.cpp)** - 错误处理
   - 结果值检查
   - 错误链接
   - 传输层错误模拟
   - 编解码错误处理
   - 错误恢复流程

3. **[examples/03_buffer_operations.cpp](examples/03_buffer_operations.cpp)** - 缓冲区操作
   - 基本读写操作
   - 查看和跳过数据
   - 字节操作
   - 环形缓冲区循环
   - Span 使用
   - 缓冲区清除

4. **[examples/04_codec_and_protocol.cpp](examples/04_codec_and_protocol.cpp)** - 编解码和协议
   - 命令和响应创建
   - 二进制编码/解码
   - 帧格式和结构
   - 帧长度检测
   - CRC 校验
   - 错误处理

5. **[examples/05_complete_communication.cpp](examples/05_complete_communication.cpp)** - 完整通信流程
   - 设备初始化和连接
   - 单条命令执行
   - 多条命令顺序执行
   - 错误处理流程
   - 设备状态追踪

### 编译和运行示例

#### 使用 CMake（推荐）

```bash
cd build
cmake ..
make

# 运行示例
./bin/01_basic_usage
./bin/02_error_handling
./bin/03_buffer_operations
./bin/04_codec_and_protocol
./bin/05_complete_communication

# 或运行所有测试和示例
ctest
```

#### 直接编译

```bash
cd examples

# 编译单个示例
g++ -std=c++11 01_basic_usage.cpp -I../include -I../third_party -o example

# 编译所有示例
for file in *.cpp; do
    g++ -std=c++11 "$file" -I../include -I../third_party -o "${file%.cpp}"
done
```

详见 [examples/README.md](examples/README.md) 了解更多编译和运行选项。

### 9.1 基本用法示例

```cpp
#include <vdl/vdl.hpp>
#include <iostream>
#include <memory>

int main() {
    // 创建传输层和编解码器
    auto transport = std::unique_ptr<vdl::i_transport_t>(
        new vdl::mock_transport_t()
    );
    auto codec = std::unique_ptr<vdl::i_codec_t>(
        new vdl::binary_codec_t()
    );

    // 创建设备
    auto device = std::unique_ptr<vdl::i_device_t>(
        new vdl::device_impl_t(std::move(transport), std::move(codec))
    );

    // 连接设备
    auto connect_result = device->connect();
    if (!connect_result.has_value()) {
        std::cerr << "连接失败: " << connect_result.error().to_string() << std::endl;
        return 1;
    }

    // 创建并执行命令
    vdl::command_t cmd;
    cmd.set_function_code(0x03);
    cmd.set_data({0x00, 0x10, 0x00, 0x08});

    auto result = device->execute(cmd);
    if (result.has_value()) {
        auto& response = *result;
        std::cout << "命令执行成功" << std::endl;
        std::cout << "响应函数码: 0x" << std::hex << (int)response.function_code() << std::dec << std::endl;
    } else {
        std::cerr << "命令执行失败: " << result.error().to_string() << std::endl;
        return 1;
    }

    // 断开连接
    device->disconnect();

    return 0;
}
```

### 9.2 错误处理

详见 [examples/02_error_handling.cpp](examples/02_error_handling.cpp)。

### 9.3 缓冲区操作

详见 [examples/03_buffer_operations.cpp](examples/03_buffer_operations.cpp)。

### 9.4 编解码

详见 [examples/04_codec_and_protocol.cpp](examples/04_codec_and_protocol.cpp)。

### 9.5 完整通信流程

详见 [examples/05_complete_communication.cpp](examples/05_complete_communication.cpp)。

更多示例请查看 [examples/ 目录](examples/) 和 [examples/README.md](examples/README.md)。

---

## 十、开发计划

### 10.1 阶段总览

| 阶段 | 内容 | 时间 |
|------|------|------|
| **Phase 1** | 项目骨架 + 第三方库 + 测试框架 | 3天 |
| **Phase 2** | Core 模块 (types, error, buffer, memory) | 5天 |
| **Phase 3** | URI 模块 | 3天 |
| **Phase 4** | Transport 层 (接口 + TCP + Mock) | 5天 |
| **Phase 5** | Codec 层 (接口 + Binary) | 3天 |
| **Phase 6** | Protocol (Command + Response) | 2天 |
| **Phase 7** | Device 核心 (状态 + 接口 + 实现) | 5天 |
| **Phase 8** | DeviceGuard 独占访问 | 3天 |
| **Phase 9** | Heartbeat | 4天 |
| **Phase 10** | Manager + Factory | 3天 |
| **Phase 11** | 集成测试 + 示例 | 4天 |
| **Phase 12** | 文档 + 收尾 | 2天 |
| **总计** | | **42天 (约 8 周)** |

### 10.2 详细计划

```
================================================================================
Phase 1: 项目骨架 (3天)
================================================================================
Day 1: CMakeLists.txt, cmake/, .clang-format, .gitignore
Day 2: third_party/ (tl_expected, tl_optional, catch2)
Day 3: tests/test_main.cpp, 验证编译

产出:
  CMakeLists.txt
  cmake/CompilerOptions.cmake
  cmake/FindDependencies.cmake
  third_party/README.md
  tests/CMakeLists.txt
  tests/test_main.cpp

================================================================================
Phase 2: Core 模块 (5天)
================================================================================
Day 1: types.hpp (Bytes, Size, Span, Milliseconds)
Day 2: error.hpp, error.cpp (Error, ErrorCode, Res<T>)
Day 3: noncopyable.hpp, scope_guard.hpp, compat.hpp
Day 4: buffer.hpp, buffer.cpp
Day 5: memory.hpp, logging.hpp

产出:
  include/vdl/core/*.hpp
  src/core/*.cpp
  tests/unit/test_error.cpp
  tests/unit/test_buffer.cpp

================================================================================
Phase 3: URI 模块 (3天)
================================================================================
Day 1: uri.hpp
Day 2: uri_parser.hpp, uri_parser.cpp
Day 3: query_params.hpp, 测试

产出:
  include/vdl/uri/*.hpp
  src/uri/*.cpp
  tests/unit/test_uri_parser.cpp

================================================================================
Phase 4: Transport 层 (5天)
================================================================================
Day 1: transport.hpp (ITransport 接口)
Day 2: tcp_transport.hpp, tcp_transport.cpp (连接)
Day 3: tcp_transport.cpp (读写)
Day 4: mock_transport.hpp, mock_transport.cpp
Day 5: transport_factory.hpp, 测试

产出:
  include/vdl/transport/*.hpp
  src/transport/*.cpp
  tests/unit/test_tcp_transport.cpp
  tests/unit/test_mock_transport.cpp

================================================================================
Phase 5: Codec 层 (3天)
================================================================================
Day 1: codec.hpp (ICodec 接口)
Day 2: binary_codec.hpp, binary_codec.cpp
Day 3: 测试

产出:
  include/vdl/codec/*.hpp
  src/codec/*.cpp
  tests/unit/test_binary_codec.cpp

================================================================================
Phase 6: Protocol (2天)
================================================================================
Day 1: command.hpp, response.hpp, status.hpp
Day 2: 测试

产出:
  include/vdl/protocol/*.hpp
  tests/unit/test_protocol.cpp

================================================================================
Phase 7: Device 核心 (5天)
================================================================================
Day 1: device_state.hpp
Day 2: device.hpp (IDevice 接口)
Day 3: device_impl.hpp, device_impl.cpp (连接管理)
Day 4: device_impl.cpp (命令执行)
Day 5: device_config.hpp, device_info.hpp, 测试

产出:
  include/vdl/device/*.hpp
  src/device/*.cpp
  tests/unit/test_device_impl.cpp

================================================================================
Phase 8: DeviceGuard (3天)
================================================================================
Day 1: device_guard.hpp
Day 2: device_guard.cpp, device_impl 锁机制
Day 3: 并发测试

产出:
  include/vdl/device/device_guard.hpp
  src/device/device_guard.cpp
  tests/unit/test_device_guard.cpp
  tests/integration/test_concurrent_access.cpp

================================================================================
Phase 9: Heartbeat (4天)
================================================================================
Day 1: heartbeat_config.hpp, heartbeat_strategy.hpp
Day 2: strategies/ (ping, echo, scpi)
Day 3: heartbeat_runner.hpp, heartbeat_runner.cpp
Day 4: 集成测试

产出:
  include/vdl/heartbeat/*.hpp
  src/heartbeat/*.cpp
  tests/unit/test_heartbeat.cpp
  tests/integration/test_heartbeat_recovery.cpp

================================================================================
Phase 10: Manager (3天)
================================================================================
Day 1: device_factory.hpp
Day 2: device_registry.hpp
Day 3: device_manager.hpp, 测试

产出:
  include/vdl/manager/*.hpp
  tests/unit/test_device_factory.cpp

================================================================================
Phase 11: 集成测试 + 示例 (4天)
================================================================================
Day 1: test_device_lifecycle.cpp
Day 2: examples/01_basic_usage, examples/02_device_guard
Day 3: examples/03_heartbeat, examples/04_custom_codec
Day 4: examples/05_multi_device

产出:
  tests/integration/*.cpp
  examples/*

================================================================================
Phase 12: 文档 + 收尾 (2天)
================================================================================
Day 1: vdl.hpp, README 更新
Day 2: API 文档, 最终测试

产出:
  include/vdl/vdl.hpp
  README.md
  docs/
```

---

## 十一、接口对比（新旧）

### 11.1 旧设计问题

| 问题 | 旧设计 | 新设计 |
|------|--------|--------|
| 层次混乱 | Driver 继承 Device | Device 组合 Transport + Codec |
| 职责不清 | Device 即做 I/O 又做业务 | 三层分离 |
| Message 泛化 | 单一 Message 类型 | Command + Response 分离 |
| 耦合紧 | Protocol 绑定 Driver | Codec 独立，可替换 |

### 11.2 新设计优势

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                              新架构优势                                       │
├──────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ✅ 清晰的职责边界                                                            │
│     Transport: 只管读写字节                                                   │
│     Codec: 只管编解码                                                         │
│     Device: 只管业务命令                                                      │
│                                                                              │
│  ✅ 灵活的组合                                                                │
│     相同的 Device 可以使用不同的 Transport (TCP, Serial, Mock)                │
│     相同的 Transport 可以使用不同的 Codec (Binary, SCPI, Custom)              │
│                                                                              │
│  ✅ 易于测试                                                                  │
│     可以 Mock 任意一层                                                        │
│     单元测试可以独立进行                                                      │
│                                                                              │
│  ✅ 易于扩展                                                                  │
│     添加新传输方式：实现 i_transport_t                                        │
│     添加新协议：实现 i_codec_t                                                │
│     不影响现有代码                                                            │
│                                                                              │
│  ✅ 类型安全                                                                  │
│     command_t 和 response_t 类型分离，编译期检查                            │
│     使用 tl::expected 替代异常                                                │
│                                                                              │
└──────────────────────────────────────────────────────────────────────────────┘
```

---

## 十二、核心类实现要点

### 12.1 device_impl_t 实现

```cpp
#ifndef VDL_DEVICE_DEVICE_IMPL_HPP
#define VDL_DEVICE_DEVICE_IMPL_HPP

class device_impl_t : public i_device_t {
public:
    device_impl_t(
        std::unique_ptr<i_transport_t> transport,
        std::unique_ptr<i_codec_t> codec,
        device_config_t config
    );

    // i_device_t 接口
    result_t<void> open() override;
    result_t<void> close() override;
    device_state_t state() const override;
    result_t<response_t> execute(const command_t& cmd, milliseconds_t timeout) override;
    optional_t<device_guard_t> try_lock() override;
    result_t<device_guard_t> lock(milliseconds_t timeout) override;
    const device_info_t& info() const override;

private:
    // 内部执行（不加锁）
    result_t<response_t> _do_execute(const command_t& cmd, milliseconds_t timeout);

    // 读取完整帧
    result_t<bytes_t> _read_frame(milliseconds_t timeout);

    std::unique_ptr<i_transport_t> m_transport;
    std::unique_ptr<i_codec_t> m_codec;
    device_config_t m_config;
    device_info_t m_info;

    mutable std::mutex m_mutex;
    std::atomic<device_state_t> m_state;
    std::atomic<bool> m_locked;

    // 心跳（可选）
    std::unique_ptr<heartbeat_runner_t> m_heartbeat;
};

#endif // VDL_DEVICE_DEVICE_IMPL_HPP
```

### 12.2 命令执行流程

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         execute(Command, timeout) 流程                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  1. 检查状态                                                                 │
│     └─ 非 Ready 状态 → 返回 Error                                            │
│                                                                             │
│  2. 加锁 (std::unique_lock)                                                  │
│     └─ 获取失败 → 返回 Busy                                                  │
│                                                                             │
│  3. 设置状态 → Busy                                                          │
│                                                                             │
│  4. 编码命令                                                                 │
│     └─ m_codec->encode(cmd) → Bytes                                         │
│     └─ 失败 → 恢复状态，返回 Error                                            │
│                                                                             │
│  5. 发送数据                                                                 │
│     └─ m_transport->write(bytes, timeout)                                   │
│     └─ 失败 → 恢复状态，返回 Error                                            │
│                                                                             │
│  6. 接收响应帧                                                               │
│     └─ readFrame(timeout)                                                   │
│     └─ 失败 → 恢复状态，返回 Error                                            │
│                                                                             │
│  7. 解码响应                                                                 │
│     └─ m_codec->decode(frame) → Response                                    │
│     └─ 失败 → 恢复状态，返回 Error                                            │
│                                                                             │
│  8. 恢复状态 → Ready                                                         │
│                                                                             │
│  9. 返回 Response                                                            │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 十三、迁移指南

从 v2 迁移到 v3：

### 13.1 概念映射

| v2 概念 | v3 概念 |
|---------|---------|
| Device.read/write | Transport.read/write |
| Driver | 废弃，功能拆分到 Transport 和 Device |
| Message | Command + Response |
| Protocol.handleMessage | Device.execute + Codec.encode/decode |

### 13.2 代码迁移

```cpp
// v2 (旧)
class my_driver_t : public driver_base_t {
    device_result_t _do_initialize() override;
    device_result_t _do_start() override;
    tl::expected<bytes_t, std::string> _do_read(size_t size) override;
    tl::expected<size_t, std::string> _do_write(const_byte_span_t data) override;
};

// v3 (新)
// 1. transport_t 处理 I/O
class my_transport_t : public i_transport_t {
    result_t<void> open() override;
    result_t<void> close() override;
    result_t<size_t> read(byte_span_t buffer, milliseconds_t timeout) override;
    result_t<size_t> write(const_byte_span_t data, milliseconds_t timeout) override;
};

// 2. codec_t 处理编解码
class my_codec_t : public i_codec_t {
    result_t<bytes_t> encode(const command_t& cmd) override;
    result_t<response_t> decode(const_byte_span_t data) override;
};

// 3. 使用 device_impl_t 组合
std::unique_ptr<i_device_t> device = device_impl_t::create(
    make_unique<my_transport_t>(),
    make_unique<my_codec_t>(),
    config
);
```

---

## 十四、FAQ

### Q1: 为什么 Device 不继承 Transport？

**A**: 遵循"组合优于继承"原则。Device 和 Transport 是不同的抽象层次：
- Transport 是底层 I/O
- Device 是业务逻辑
  
继承会导致 Device 暴露不应该暴露的接口（如 `read`/`write` 字节），破坏封装。

### Q2: 为什么分离 Command 和 Response？

**A**: 类型安全。Command 和 Response 有不同的语义和结构：
- Command: 用户发起，包含命令码和参数
- Response: 设备返回，包含状态和数据

分离后编译器可以检查类型错误。

### Q3: Codec 何时需要自定义？

**A**: 当使用自定义协议时。内置 BinaryCodec 适用于通用二进制协议，如果你的设备使用特殊格式（如 SCPI、Modbus），需要实现对应的 Codec。

### Q4: 心跳失败会自动重连吗？

**A**: 不会。心跳失败只会上报（触发回调或状态变更），由上层决定是否重连。这样更灵活，避免自动重连导致的问题。

---

## 十五、附录：完整接口定义

### A.1 类型别名

```cpp
// include/vdl/core/types.hpp

#ifndef VDL_CORE_TYPES_HPP
#define VDL_CORE_TYPES_HPP

namespace vdl {

// 基础类型
typedef std::uint8_t byte_t;
typedef std::vector<byte_t> bytes_t;
typedef std::size_t size_t;

// span_t 类型 (自定义轻量实现)
template<typename T>
class span_t;

typedef span_t<byte_t> byte_span_t;
typedef span_t<const byte_t> const_byte_span_t;

// 时间类型
typedef std::chrono::milliseconds milliseconds_t;
typedef std::chrono::microseconds microseconds_t;

// 结果类型
template<typename T>
using result_t = tl::expected<T, error_t>;

template<typename T>
using optional_t = tl::optional<T>;

} // namespace vdl

#endif // VDL_CORE_TYPES_HPP
```

### A.2 错误码枚举

```cpp
// include/vdl/core/error.hpp

#ifndef VDL_CORE_ERROR_HPP
#define VDL_CORE_ERROR_HPP

namespace vdl {

enum class error_code_t : int {
    // 成功
    ok = 0,

    // 传输层 (1000-1999)
    transport_error = 1000,
    connection_failed = 1001,
    connection_closed = 1002,
    read_error = 1003,
    write_error = 1004,
    timeout = 1005,
    address_invalid = 1006,

    // 编解码层 (2000-2999)
    codec_error = 2000,
    invalid_frame = 2001,
    checksum_error = 2002,
    encode_failed = 2003,
    decode_failed = 2004,
    frame_too_large = 2005,

    // 设备层 (3000-3999)
    device_error = 3000,
    not_open = 3001,
    already_open = 3002,
    busy = 3003,
    lock_conflict = 3004,
    invalid_state = 3005,
    operation_failed = 3006,

    // 协议层 (4000-4999)
    protocol_error = 4000,
    invalid_command = 4001,
    unsupported_version = 4002,
    command_timeout = 4003,

    // 应用层 (5000-5999)
    application_error = 5000,
    invalid_argument = 5001,
    resource_not_found = 5002,
    configuration_error = 5003
};

enum class error_category_t {
    none,
    transport,
    codec,
    device,
    protocol,
    application
};

} // namespace vdl

#endif // VDL_CORE_ERROR_HPP
```

---

## 十六、许可证

MIT License
