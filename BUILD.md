# VDL 项目构建指南

本项目提供了多种便捷的构建方式，适用于不同的平台和使用习惯。

## 🚀 快速开始

### Linux/macOS

```bash
# 方式 1: 使用构建脚本（推荐）
./build.sh              # Debug 模式编译
./build.sh -R -t        # Release 模式编译并测试

# 方式 2: 使用 Makefile
make                    # Debug 模式编译
make release            # Release 模式编译
make test               # 运行测试

# 方式 3: 直接使用 CMake
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows

```batch
REM 使用批处理脚本
build.bat              REM Debug 模式编译
build.bat -R -t        REM Release 模式编译并测试

REM 直接使用 CMake
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## 📋 构建脚本详细说明

### build.sh (Linux/macOS)

功能最完善的构建脚本，支持：

- ✅ 彩色输出，友好的用户界面
- ✅ Debug/Release 模式切换
- ✅ 并行编译（自动检测 CPU 核心数）
- ✅ 自动运行测试
- ✅ 代码覆盖率支持
- ✅ AddressSanitizer 支持
- ✅ 详细的帮助信息

#### 常用命令

```bash
# 查看帮助
./build.sh --help

# 编译
./build.sh              # Debug 模式
./build.sh -R           # Release 模式
./build.sh -v           # 详细输出

# 清理和重建
./build.sh clean        # 清理构建目录
./build.sh rebuild      # 清理后重新编译

# 测试
./build.sh -t           # 编译并运行测试
./build.sh -R -t        # Release 模式编译并测试

# 高级选项
./build.sh --coverage   # 启用代码覆盖率
./build.sh --sanitizer  # 启用 AddressSanitizer
./build.sh -j 8         # 使用 8 线程编译
./build.sh --no-tests   # 跳过测试编译（加快编译速度）
```

### Makefile

简洁的 make 命令，适合习惯 Makefile 的开发者：

```bash
# 查看所有命令
make help

# 编译
make                    # Debug 模式（默认）
make build              # Debug 模式
make release            # Release 模式
make fast               # Release 快速编译（跳过测试）

# 清理
make clean              # 清理
make rebuild            # 重建

# 测试
make test               # 运行所有测试
make test-unit          # 只运行单元测试
make test-quick         # 快速测试（常用模块）

# 示例
make examples           # 运行所有示例
make ex1                # 运行示例 1 (basic_usage)
make ex2                # 运行示例 2 (error_handling)

# 组合命令
make bt                 # build + test
make rt                 # release + test
```

### build.bat (Windows)

Windows 平台的构建脚本：

```batch
REM 查看帮助
build.bat --help

REM 编译
build.bat              REM Debug 模式
build.bat -R           REM Release 模式

REM 清理和重建
build.bat clean        REM 清理
build.bat rebuild      REM 重建

REM 测试
build.bat -t           REM 编译并测试
build.bat -R -t        REM Release 模式编译并测试
```

## 🔧 构建选项说明

| 选项 | build.sh | Makefile | build.bat | 说明 |
|------|----------|----------|-----------|------|
| 帮助 | `--help` | `help` | `--help` | 显示帮助信息 |
| 清理 | `--clean` | `clean` | `--clean` | 清理构建目录 |
| 重建 | `--rebuild` | `rebuild` | `--rebuild` | 清理后重新编译 |
| Debug | `--debug` | `debug` | `--debug` | Debug 模式（默认） |
| Release | `--release` | `release` | `--release` | Release 模式 |
| 测试 | `--test` | `test` | `--test` | 运行测试 |
| 并行 | `-j N` | - | - | N 线程并行编译 |
| 详细 | `--verbose` | - | - | 显示详细信息 |

## 📦 构建产物

编译完成后，产物位于 `build/` 目录：

```
build/
├── bin/                    # 可执行文件
│   ├── vdl_tests          # 测试程序
│   ├── 01_basic_usage     # 示例 1
│   ├── 02_error_handling  # 示例 2
│   └── ...                # 其他示例
├── lib/                    # 库文件
│   └── libvdl.a           # 静态库
└── ...                     # 其他 CMake 生成文件
```

## ⚡ 推荐工作流

### 日常开发

```bash
# 初次克隆后
./build.sh              # 编译项目

# 修改代码后
make                    # 快速编译（使用缓存）

# 运行测试
make test               # 运行所有测试
make test-quick         # 快速测试

# 运行示例验证
make ex1                # 运行示例程序
```

### 提交前检查

```bash
# 完整测试
./build.sh -R -t        # Release 模式编译和测试

# 代码覆盖率
./build.sh --coverage -t

# 内存检查
./build.sh --sanitizer -t
```

### 发布版本

```bash
# 清理并构建 Release 版本
./build.sh -r -R -t

# 或使用 Makefile
make rebuild release test
```

## 🐛 故障排除

### 编译失败

1. 确保安装了必要的依赖：
   ```bash
   # Ubuntu/Debian
   sudo apt-get install build-essential cmake
   
   # macOS
   brew install cmake
   ```

2. 清理后重新编译：
   ```bash
   ./build.sh rebuild
   ```

### 测试失败

1. 只运行失败的测试：
   ```bash
   cd build
   ./bin/vdl_tests "[测试名称]"
   ```

2. 查看详细日志：
   ```bash
   ./build.sh -v -t
   ```

## 💡 提示

- 首次编译建议使用 `./build.sh -v` 查看详细信息
- 日常开发使用 `make` 命令更快捷
- Release 模式性能更好，但编译时间更长
- 使用 `-j` 参数可以加速编译（但会占用更多内存）
- 修改 CMakeLists.txt 后需要重新配置：`./build.sh rebuild`

## 📚 更多信息

- [CMakeLists.txt](CMakeLists.txt) - CMake 配置文件
- [README.md](README.md) - 项目主文档
- [examples/](examples/) - 示例程序代码
- [tests/](tests/) - 测试代码
