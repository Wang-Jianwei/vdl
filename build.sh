#!/usr/bin/env bash
# VDL 项目便捷编译脚本
# 使用方法: ./build.sh [选项]

set -e  # 遇到错误立即退出

# ============================================================================
# 颜色定义
# ============================================================================
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# ============================================================================
# 辅助函数
# ============================================================================

# 打印信息
info() {
    echo -e "${CYAN}[INFO]${NC} $*"
}

# 打印成功
success() {
    echo -e "${GREEN}[✓]${NC} $*"
}

# 打印警告
warn() {
    echo -e "${YELLOW}[!]${NC} $*"
}

# 打印错误
error() {
    echo -e "${RED}[✗]${NC} $*" >&2
}

# 打印标题
title() {
    echo -e "\n${BOLD}${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BOLD}${BLUE}  $*${NC}"
    echo -e "${BOLD}${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}\n"
}

# 显示帮助信息
show_help() {
    cat << EOF
${BOLD}VDL 项目编译脚本${NC}

${BOLD}用法:${NC}
    ./build.sh [选项]

${BOLD}选项:${NC}
    ${GREEN}-h, --help${NC}          显示此帮助信息
    ${GREEN}-c, --clean${NC}         清理构建目录
    ${GREEN}-r, --rebuild${NC}       清理后重新构建
    ${GREEN}-d, --debug${NC}         Debug 模式编译（默认）
    ${GREEN}-R, --release${NC}       Release 模式编译
    ${GREEN}-t, --test${NC}          编译后运行测试
    ${GREEN}-j, --jobs N${NC}        并行编译任务数（默认: CPU核心数）
    ${GREEN}-v, --verbose${NC}       显示详细编译信息
    ${GREEN}-e, --examples${NC}      编译后运行示例程序
    ${GREEN}--no-tests${NC}          跳过测试编译
    ${GREEN}--coverage${NC}          启用代码覆盖率
    ${GREEN}--sanitizer${NC}         启用地址消毒器（ASan）

${BOLD}示例:${NC}
    ./build.sh                    # 默认 Debug 模式编译
    ./build.sh -R -t              # Release 模式编译并运行测试
    ./build.sh -c                 # 清理构建目录
    ./build.sh -r -j 8            # 清理后使用 8 线程重新编译
    ./build.sh --release --test   # Release 模式编译并测试

${BOLD}快捷方式:${NC}
    ./build.sh clean              # 等同于 --clean
    ./build.sh test               # 等同于 --test
    ./build.sh rebuild            # 等同于 --rebuild

EOF
}

# ============================================================================
# 变量初始化
# ============================================================================

BUILD_DIR="build"
BUILD_TYPE="Release"
CLEAN=0
REBUILD=0
RUN_TESTS=0
RUN_EXAMPLES=0
VERBOSE=0
SKIP_TESTS=0
ENABLE_COVERAGE=0
ENABLE_SANITIZER=0

# 检测 CPU 核心数
if [[ -f /proc/cpuinfo ]]; then
    JOBS=$(grep -c ^processor /proc/cpuinfo)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    JOBS=$(sysctl -n hw.ncpu)
else
    JOBS=4
fi

# ============================================================================
# 参数解析
# ============================================================================

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean|clean)
            CLEAN=1
            shift
            ;;
        -r|--rebuild|rebuild)
            REBUILD=1
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -R|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -t|--test|test)
            RUN_TESTS=1
            shift
            ;;
        -e|--examples)
            RUN_EXAMPLES=1
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        --no-tests)
            SKIP_TESTS=1
            shift
            ;;
        --coverage)
            ENABLE_COVERAGE=1
            BUILD_TYPE="Debug"
            shift
            ;;
        --sanitizer)
            ENABLE_SANITIZER=1
            BUILD_TYPE="Debug"
            shift
            ;;
        *)
            error "未知选项: $1"
            echo "使用 --help 查看帮助信息"
            exit 1
            ;;
    esac
done

# ============================================================================
# 主逻辑
# ============================================================================

# 显示构建配置
title "VDL 构建配置"
info "构建模式: ${BOLD}${BUILD_TYPE}${NC}"
info "并行任务: ${BOLD}${JOBS}${NC}"
info "构建目录: ${BOLD}${BUILD_DIR}${NC}"
[[ $SKIP_TESTS -eq 1 ]] && warn "跳过测试编译"
[[ $ENABLE_COVERAGE -eq 1 ]] && info "代码覆盖率: ${BOLD}已启用${NC}"
[[ $ENABLE_SANITIZER -eq 1 ]] && info "地址消毒器: ${BOLD}已启用${NC}"

# 清理构建
if [[ $CLEAN -eq 1 ]] || [[ $REBUILD -eq 1 ]]; then
    title "清理构建目录"
    if [[ -d "$BUILD_DIR" ]]; then
        info "删除 $BUILD_DIR 目录..."
        rm -rf "$BUILD_DIR"
        success "构建目录已清理"
    else
        info "构建目录不存在，跳过清理"
    fi
    
    # 如果只是清理，退出
    if [[ $CLEAN -eq 1 ]] && [[ $REBUILD -eq 0 ]]; then
        success "清理完成"
        exit 0
    fi
fi

# 创建构建目录
if [[ ! -d "$BUILD_DIR" ]]; then
    info "创建构建目录..."
    mkdir -p "$BUILD_DIR"
fi

# 配置项目
title "配置项目 (CMake)"
cd "$BUILD_DIR"

CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
)

if [[ $SKIP_TESTS -eq 1 ]]; then
    CMAKE_ARGS+=("-DBUILD_TESTING=OFF")
fi

if [[ $ENABLE_COVERAGE -eq 1 ]]; then
    CMAKE_ARGS+=(
        "-DCMAKE_CXX_FLAGS=--coverage"
        "-DCMAKE_C_FLAGS=--coverage"
    )
fi

if [[ $ENABLE_SANITIZER -eq 1 ]]; then
    CMAKE_ARGS+=(
        "-DCMAKE_CXX_FLAGS=-fsanitize=address -fno-omit-frame-pointer"
        "-DCMAKE_C_FLAGS=-fsanitize=address -fno-omit-frame-pointer"
    )
fi

info "运行: cmake ${CMAKE_ARGS[*]} .."
if ! cmake "${CMAKE_ARGS[@]}" ..; then
    error "CMake 配置失败"
    exit 1
fi
success "CMake 配置完成"

# 编译项目
title "编译项目"
info "使用 ${JOBS} 个并行任务编译..."

if [[ $VERBOSE -eq 1 ]]; then
    MAKE_ARGS="VERBOSE=1"
else
    MAKE_ARGS=""
fi

if ! make -j"${JOBS}" ${MAKE_ARGS}; then
    error "编译失败"
    exit 1
fi

success "编译成功！"

# 显示编译结果
echo ""
info "可执行文件位置:"
if [[ -d "bin" ]]; then
    ls -lh bin/ | grep -v "^total" | awk '{printf "  %-30s %8s\n", $9, $5}'
fi

if [[ -d "lib" ]]; then
    echo ""
    info "库文件:"
    ls -lh lib/ | grep -v "^total" | awk '{printf "  %-30s %8s\n", $9, $5}'
fi

# 运行测试
if [[ $RUN_TESTS -eq 1 ]]; then
    title "运行测试"
    
    if [[ ! -f "bin/vdl_tests" ]]; then
        warn "测试程序不存在，跳过测试"
    else
        info "运行测试套件..."
        if ./bin/vdl_tests; then
            success "所有测试通过！"
        else
            error "部分测试失败"
            exit 1
        fi
    fi
fi

# 运行示例程序
if [[ $RUN_EXAMPLES -eq 1 ]]; then
    title "运行示例程序"
    
    EXAMPLES=(
        "01_basic_usage"
        "02_error_handling"
        "03_buffer_operations"
        "04_codec_and_protocol"
    )
    
    for example in "${EXAMPLES[@]}"; do
        if [[ -f "bin/${example}" ]]; then
            info "运行 ${example}..."
            echo "----------------------------------------"
            if ./bin/"${example}"; then
                success "${example} 执行成功"
            else
                warn "${example} 执行失败（退出码: $?）"
            fi
            echo ""
        fi
    done
fi

# 代码覆盖率报告
if [[ $ENABLE_COVERAGE -eq 1 ]]; then
    title "生成代码覆盖率报告"
    
    if command -v lcov &> /dev/null; then
        info "使用 lcov 生成覆盖率报告..."
        lcov --capture --directory . --output-file coverage.info
        lcov --remove coverage.info '/usr/*' '*/third_party/*' '*/tests/*' --output-file coverage.info
        
        if command -v genhtml &> /dev/null; then
            genhtml coverage.info --output-directory coverage_html
            success "覆盖率报告已生成: ${BUILD_DIR}/coverage_html/index.html"
        fi
    else
        warn "未安装 lcov，跳过覆盖率报告生成"
    fi
fi

# 完成
cd ..
title "构建完成"
success "VDL 项目编译成功！(${BUILD_TYPE} 模式)"

echo ""
info "快速开始:"
echo "  运行测试:    cd build && ./bin/vdl_tests"
echo "  运行示例:    cd build && ./bin/01_basic_usage"
echo "  查看帮助:    ./build.sh --help"
echo ""
