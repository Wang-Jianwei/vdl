# VDL 项目 Makefile
# 提供便捷的编译和管理命令

.PHONY: all build debug release clean rebuild test examples help install format docs

# 默认目标
all: build

# 显示帮助
help:
	@echo ""
	@echo "VDL 项目便捷命令"
	@echo "==============================================="
	@echo ""
	@echo "编译命令:"
	@echo "  make build      - Debug 模式编译（默认）"
	@echo "  make debug      - Debug 模式编译"
	@echo "  make release    - Release 模式编译"
	@echo "  make fast       - Release 快速编译（跳过测试）"
	@echo ""
	@echo "清理命令:"
	@echo "  make clean      - 清理构建目录"
	@echo "  make rebuild    - 清理后重新编译"
	@echo ""
	@echo "测试命令:"
	@echo "  make test       - 运行所有测试"
	@echo "  make test-unit  - 只运行单元测试"
	@echo "  make test-quick - 快速测试（常用模块）"
	@echo ""
	@echo "示例命令:"
	@echo "  make examples   - 运行示例程序"
	@echo "  make ex1        - 运行示例 1 (basic_usage)"
	@echo "  make ex2        - 运行示例 2 (error_handling)"
	@echo ""
	@echo "其他命令:"
	@echo "  make format     - 格式化代码"
	@echo "  make install    - 安装到系统"
	@echo "  make help       - 显示此帮助"
	@echo ""

# Debug 模式编译
build debug:
	@./build.sh --debug

# Release 模式编译
release:
	@./build.sh --release

# Release 快速编译（跳过测试）
fast:
	@./build.sh --release --no-tests

# 清理
clean:
	@./build.sh --clean

# 重新编译
rebuild:
	@./build.sh --rebuild

# 运行测试
test:
	@if [ -f build/bin/vdl_tests ]; then \
		cd build && ./bin/vdl_tests; \
	else \
		echo "测试程序不存在，请先编译: make build"; \
		exit 1; \
	fi

# 单元测试
test-unit:
	@if [ -f build/bin/vdl_tests ]; then \
		cd build && ./bin/vdl_tests "[unit]"; \
	else \
		echo "测试程序不存在，请先编译: make build"; \
		exit 1; \
	fi

# 快速测试
test-quick:
	@if [ -f build/bin/vdl_tests ]; then \
		cd build && ./bin/vdl_tests "[core]"; \
	else \
		echo "测试程序不存在，请先编译: make build"; \
		exit 1; \
	fi

# 运行示例
examples:
	@./build.sh --examples

# 运行特定示例
ex1:
	@if [ -f build/bin/01_basic_usage ]; then \
		cd build && ./bin/01_basic_usage; \
	else \
		echo "示例程序不存在，请先编译"; \
	fi

ex2:
	@if [ -f build/bin/02_error_handling ]; then \
		cd build && ./bin/02_error_handling; \
	else \
		echo "示例程序不存在，请先编译"; \
	fi

ex3:
	@if [ -f build/bin/03_buffer_operations ]; then \
		cd build && ./bin/03_buffer_operations; \
	else \
		echo "示例程序不存在，请先编译"; \
	fi

ex4:
	@if [ -f build/bin/04_codec_and_protocol ]; then \
		cd build && ./bin/04_codec_and_protocol; \
	else \
		echo "示例程序不存在，请先编译"; \
	fi

# 安装
install:
	@if [ -d build ]; then \
		cd build && sudo make install; \
	else \
		echo "请先编译项目: make build"; \
		exit 1; \
	fi

# 格式化代码（如果有 clang-format）
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		find include src tests examples -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i; \
		echo "代码格式化完成"; \
	else \
		echo "未安装 clang-format"; \
	fi

# 编译并测试（常用组合）
bt: build test

# Release 编译并测试
rt: release test
