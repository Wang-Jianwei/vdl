# VDL 开发计划和架构内部指南

> **注意**: 本文档主要面向 VDL 库的开发者和贡献者，不是库的使用者文档。
> 库的使用者请参考 [README.md](../README.md) 和 [docs/README_CN.md](README_CN.md)

---

## 一、开发计划

### 1.1 阶段总览

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

### 1.2 详细计划

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
  .clang-format
  .gitignore
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
  tests/unit/test_buffer.cpp
  tests/unit/test_error.cpp

================================================================================
Phase 3: URI 模块 (3天)
================================================================================
Day 1: uri.hpp
Day 2: uri_parser.hpp, uri_parser.cpp
Day 3: query_params.hpp, 测试

产出:
  include/vdl/uri/*.hpp
  src/uri/uri_parser.cpp
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
  src/codec/binary_codec.cpp
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
  src/device/device_impl.cpp
  tests/unit/test_device_impl.cpp
  tests/integration/test_device_lifecycle.cpp

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
  src/heartbeat/heartbeat_runner.cpp
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
  README.md 更新
  docs/
```

---

## 二、核心类实现要点

### 2.1 device_impl_t 实现

```cpp
#ifndef VDL_DEVICE_DEVICE_IMPL_HPP
#define VDL_DEVICE_DEVICE_IMPL_HPP

class device_impl_t : public i_device_t {
public:
    device_impl_t(
        std::unique_ptr<i_transport_t> transport,
        std::unique_ptr<i_codec_t> codec,
        const device_config_t& config = device_config_t()
    );

    // IDevice 实现
    result_t<void> open() override;
    result_t<void> close() override;
    device_state_t state() const override;
    result_t<response_t> execute(const command_t& cmd, milliseconds_t timeout = 0) override;
    device_guard_t lock() override;
    bool try_lock() override;
    const device_info_t& info() const override;

private:
    // 内部执行（不加锁）
    result_t<response_t> _execute_unlocked(const command_t& cmd, milliseconds_t timeout);
    result_t<bytes_t> _read_frame(milliseconds_t timeout);
    void _update_state(device_state_t new_state);

    std::unique_ptr<i_transport_t> m_transport;
    std::unique_ptr<i_codec_t> m_codec;
    mutable std::mutex m_mutex;
    device_state_t m_state;
    device_info_t m_info;
    std::unique_ptr<heartbeat_runner_t> m_heartbeat;
};

#endif // VDL_DEVICE_DEVICE_IMPL_HPP
```

### 2.2 命令执行流程

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

### 2.3 并发访问处理

```cpp
// device_impl_t 中的并发保护

// 情况 1: 多线程同时调用 execute
Thread 1: execute() ─┐
                    ├─ m_mutex.lock() ─┐
Thread 2: execute() ─┤                 ├─ 串行执行
                    └─ m_mutex.lock() ─┘
Thread 3: execute() ─┘

// 情况 2: execute 过程中调用 lock
Thread 1: execute()                          (持有 mutex)
Thread 2: lock() ─ 等待 mutex 释放 ─ 获得独占访问

// 情况 3: 心跳线程与 execute 并发
Heartbeat: _do_heartbeat()    (非关键路径)
    ↓ 每 1000ms
Device: execute()             (用户命令)
    ├─ 获得 mutex
    ├─ 执行命令
    └─ 释放 mutex
```

---

## 三、迁移指南（从 v2 升级到 v3）

### 3.1 概念映射

| v2 概念 | v3 概念 | 说明 |
|---------|---------|------|
| Device.read/write | Transport.read/write | 原 Device 的 I/O 接口移到 Transport |
| Driver | Transport | 驱动的通信部分变成 Transport |
| Message | Command + Response | 统一的 Message 改为分离的 Command 和 Response |
| Protocol.handleMessage | Device.execute + Codec.encode/decode | 协议处理拆分到 Device 和 Codec |

### 3.2 代码迁移示例

#### v2 代码示例

```cpp
// v2 (旧)
class my_driver_t : public driver_base_t {
    device_result_t _do_initialize() override;
    device_result_t _do_shutdown() override;
    
    // 混合了 I/O 和业务逻辑
    tl::expected<size_t, std::string> _do_read(byte_span_t buffer) override;
    tl::expected<size_t, std::string> _do_write(const_byte_span_t data) override;
    
    // 协议处理
    device_result_t _do_execute(const message_t& msg) override;
};
```

#### v3 代码示例

```cpp
// v3 (新)
// 步骤 1: 将 I/O 提取到 Transport
class my_transport_t : public i_transport_t {
    result_t<void> open() override {
        // _do_initialize() 的逻辑
        // TCP 连接或串口打开
    }
    
    result_t<void> close() override {
        // _do_shutdown() 的逻辑
    }
    
    result_t<size_t> read(byte_span_t buffer, milliseconds_t timeout) override {
        // _do_read() 的逻辑
    }
    
    result_t<size_t> write(const_byte_span_t data, milliseconds_t timeout) override {
        // _do_write() 的逻辑
    }
};

// 步骤 2: 将协议处理提取到 Codec
class my_codec_t : public i_codec_t {
    result_t<bytes_t> encode(const command_t& cmd) override {
        // command_t → bytes
        // 原 _do_execute() 中的编码部分
    }
    
    result_t<response_t> decode(const_byte_span_t data) override {
        // bytes → response_t
        // 原 _do_execute() 中的解码部分
    }
    
    result_t<size_t> frame_size(const_byte_span_t header) override {
        // 根据头部推断完整帧长度
    }
    
    size_t min_header_size() const override {
        return 4;  // 你的协议最小头部长度
    }
};

// 步骤 3: 使用 device_impl_t 组合
std::unique_ptr<i_device_t> device = std::make_unique<device_impl_t>(
    std::make_unique<my_transport_t>(),
    std::make_unique<my_codec_t>(),
    device_config_t { /* 配置 */ }
);

// 步骤 4: 使用 Device
device->open();
command_t cmd = command_t::query(0x100);
auto resp = device->execute(cmd);
```

### 3.3 迁移检查清单

- [ ] 分析旧代码中 I/O 部分
- [ ] 实现 `i_transport_t` 替代原 Driver 的 I/O
- [ ] 分析旧代码中协议处理部分
- [ ] 实现 `i_codec_t` 替代原 Protocol 的编解码
- [ ] 更新业务代码使用新的 Device 接口
- [ ] 迁移测试用例到新架构
- [ ] 验证功能等价性

---

## 四、架构内部设计决策

### 4.1 为什么 Device 不继承 Transport？

**问题**: 为什么不用继承来实现层次关系？

**答案**:
1. **职责分离**: Device 和 Transport 是不同抽象层次
   - Transport 是底层通信（read/write 字节）
   - Device 是中层业务（execute 命令）
   
2. **接口污染**: 如果 Device 继承 Transport，会暴露不应暴露的接口
   ```cpp
   // 不好的设计
   class device_impl_t : public i_transport_t {
       result_t<size_t> read(...);  // 不应暴露给业务层
       result_t<size_t> write(...); // 不应暴露给业务层
   };
   
   // 好的设计
   class device_impl_t : public i_device_t {
       result_t<response_t> execute(...);  // 只暴露业务接口
   private:
       std::unique_ptr<i_transport_t> m_transport;  // 内部使用
   };
   ```

3. **灵活性**: 组合支持多个 Transport，继承只支持一个
   ```cpp
   // 组合：支持多传输
   device_impl_t(transport1, codec1);
   device_impl_t(transport2, codec1);  // 同样的 codec，不同的 transport
   
   // 继承：不支持
   class device_impl_t : public tcp_transport_t { ... };
   // 无法同时支持其他 transport
   ```

### 4.2 为什么分离 Command 和 Response？

**问题**: 为什么不用单一的 Message 类型？

**答案**:
1. **类型安全**: 编译期检查
   ```cpp
   // 不好：运行时才发现错误
   result_t<message_t> resp = device->execute(msg);
   if (resp->type() == MessageType::Response) {
       // 检查类型...
   }
   
   // 好：编译期检查
   result_t<response_t> resp = device->execute(cmd);
   // 类型已确定
   ```

2. **语义清晰**: 反映实际的通信模型
   - Command: 用户 → 设备
   - Response: 设备 → 用户

3. **接口设计**: 不同的方法处理不同的类型
   ```cpp
   // 与其他库的一致性
   device->execute(cmd);     // 发送命令
   // vs
   device->send(message);    // 泛泛的 send
   ```

### 4.3 为什么选择 tl::expected 而不是异常？

**问题**: 为什么不用异常处理错误？

**答案**:
1. **性能**: 异常有开销（栈展开、RTTI）
2. **可组合**: result_t 支持函数式编程
   ```cpp
   // 可组合
   auto result = device->execute(cmd)
       .and_then([](const response_t& resp) {
           return parse_response(resp);
       })
       .or_else([](const error_t& err) {
           return handle_error(err);
       });
   
   // 异常不支持这样的组合
   ```

3. **显式**: 函数签名表明可能失败
   ```cpp
   result_t<response_t> execute(...);  // 清楚地表明可能失败
   response_t execute(...);             // 不清楚
   ```

---

## 五、完整接口定义参考

### 5.1 类型别名

```cpp
// include/vdl/core/types.hpp

namespace vdl {

// 基础类型
typedef std::uint8_t byte_t;
typedef std::vector<byte_t> bytes_t;
typedef std::size_t size_t;

// span_t 类型 (自定义轻量实现或使用 std::span)
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
```

### 5.2 错误码枚举

```cpp
// include/vdl/core/error.hpp

namespace vdl {

enum class error_code_t : int {
    // 成功 (0-999)
    ok = 0,
    
    // 传输层 (1000-1999)
    connection_failed = 1000,
    connection_timeout = 1001,
    connection_closed = 1002,
    read_timeout = 1010,
    read_error = 1011,
    write_timeout = 1020,
    write_error = 1021,
    
    // 编解码层 (2000-2999)
    invalid_frame = 2000,
    frame_too_large = 2001,
    checksum_error = 2010,
    decode_failed = 2020,
    encode_failed = 2021,
    
    // 设备层 (3000-3999)
    device_not_open = 3000,
    device_busy = 3001,
    device_error_state = 3002,
    lock_conflict = 3010,
    lock_timeout = 3011,
    invalid_state = 3020,
    
    // 协议层 (4000-4999)
    invalid_command = 4000,
    unsupported_command = 4001,
    unsupported_version = 4010,
    
    // 应用层 (5000-5999)
    invalid_argument = 5000,
    resource_not_found = 5001,
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
```

---

## 六、新贡献者指南

### 6.1 代码审查检查清单

在提交 PR 时，确保满足：

- [ ] 代码符合 `.clang-format` 风格
- [ ] 单元测试覆盖率 > 80%
- [ ] 所有测试通过 (`ctest`)
- [ ] 无编译警告 (所有平台)
- [ ] 符合命名规范 (snake_case, m_ 前缀等)
- [ ] 使用 result_t 处理错误，不使用异常
- [ ] 线程安全（如适用）
- [ ] 文档已更新
- [ ] 提交消息清晰

### 6.2 本地开发工作流

```bash
# 1. 构建
./build.sh -D

# 2. 运行测试
cd build && ctest --verbose

# 3. 检查代码风格
clang-format -i include/vdl/**/*.hpp src/**/*.cpp

# 4. 构建文档（如适用）
# make docs

# 5. 提交前验证
make clean && make && ctest
```

### 6.3 常见任务

**添加新的 Transport 实现:**
1. 在 `include/vdl/transport/` 创建 `.hpp` 文件
2. 实现 `i_transport_t` 接口
3. 在 `src/transport/` 创建 `.cpp` 文件（如需）
4. 在 `tests/unit/` 添加测试
5. 在 `transport_factory.hpp` 注册工厂

**添加新的 Codec 实现:**
1. 在 `include/vdl/codec/` 创建 `.hpp` 文件
2. 实现 `i_codec_t` 接口
3. 在 `src/codec/` 创建 `.cpp` 文件（如需）
4. 在 `tests/unit/` 添加测试
5. 在文档中说明支持的协议格式

---

## 七、性能优化笔记

### 7.1 关键路径

1. **device.execute()** - 最频繁的操作
   - 优化：预分配缓冲区，避免频繁 malloc
   - 优化：减少锁持有时间

2. **codec.encode/decode()** - 数据路径
   - 优化：使用 span_t 避免复制
   - 优化：考虑 SIMD 加速（如适用）

3. **transport.read/write()** - I/O 路径
   - 优化：使用非阻塞 I/O（如可能）
   - 优化：缓冲批量操作

### 7.2 内存布局

```cpp
// device_impl_t 内存布局优化

class device_impl_t {
private:
    // 热路径数据（第一缓存行）
    std::atomic<device_state_t> m_state;
    std::mutex m_mutex;
    
    // 冷路径数据
    std::unique_ptr<i_transport_t> m_transport;
    std::unique_ptr<i_codec_t> m_codec;
    device_info_t m_info;
    std::unique_ptr<heartbeat_runner_t> m_heartbeat;
};
```

---

## 八、故障排查指南

### 8.1 常见问题

**Q: 编译错误 `error: no matching function for call to 'device_impl_t::device_impl_t'`**

A: 检查 unique_ptr 的类型是否匹配 Transport/Codec 接口

**Q: 运行时崩溃在 `device->execute()`**

A: 检查：
1. device 是否已 open()
2. transport 和 codec 是否初始化成功
3. 是否有多线程竞态条件

**Q: 心跳线程不工作**

A: 检查：
1. heartbeat_config_t 是否正确配置
2. 心跳策略的响应验证是否过于严格
3. 设备是否在独占访问中（会暂停心跳）

### 8.2 调试技巧

```cpp
// 启用日志
#define VDL_ENABLE_LOGGING 1

// 追踪状态转换
device_state_t old_state = m_state;
device_state_t new_state = ...;
VDL_LOG_DEBUG("State: {} -> {}", (int)old_state, (int)new_state);

// 验证并发访问
std::lock_guard<std::mutex> lock(m_mutex);  // 确保持有锁
```

---

**维护者**: VDL 开发团队  
**最后更新**: 2024年1月27日  
**版本**: 1.0
