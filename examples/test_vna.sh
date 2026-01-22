#!/bin/bash
# VNA SCPI 通信快速测试脚本

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  VNA SCPI 通信测试工具"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# 检查是否提供了 IP 地址
if [ $# -eq 0 ]; then
    echo "使用方法: $0 <VNA_IP> [PORT]"
    echo ""
    echo "示例:"
    echo "  $0 192.168.1.100"
    echo "  $0 192.168.1.100 5025"
    echo ""
    echo "提示: 默认端口为 5025 (标准 SCPI 端口)"
    exit 1
fi

VNA_IP=$1
VNA_PORT=${2:-5025}

echo "VNA 设备信息:"
echo "  IP 地址: $VNA_IP"
echo "  端口: $VNA_PORT"
echo ""

# 检查网络连接
echo "正在测试网络连接..."
if ping -c 1 -W 2 $VNA_IP > /dev/null 2>&1; then
    echo "✓ 网络连接正常"
else
    echo "✗ 无法 ping 通设备，请检查:"
    echo "  1. VNA IP 地址是否正确"
    echo "  2. VNA 是否已开机"
    echo "  3. 网络连接是否正常"
    exit 1
fi

# 检查端口
echo "正在检查端口 $VNA_PORT..."
if timeout 2 bash -c "cat < /dev/null > /dev/tcp/$VNA_IP/$VNA_PORT" 2>/dev/null; then
    echo "✓ 端口 $VNA_PORT 可访问"
else
    echo "✗ 端口 $VNA_PORT 无法访问，请检查:"
    echo "  1. VNA 的 SCPI 服务是否已启用"
    echo "  2. 防火墙设置"
    echo "  3. 端口号是否正确 (通常是 5025)"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "启动 VNA SCPI 通信示例..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# 查找可执行文件
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
EXEC_PATH=""

# 尝试几个可能的位置
if [ -f "$SCRIPT_DIR/../build/bin/06_vna_scpi_communication" ]; then
    EXEC_PATH="$SCRIPT_DIR/../build/bin/06_vna_scpi_communication"
elif [ -f "$SCRIPT_DIR/06_vna_scpi_communication" ]; then
    EXEC_PATH="$SCRIPT_DIR/06_vna_scpi_communication"
elif [ -f "$SCRIPT_DIR/vna_scpi_example" ]; then
    EXEC_PATH="$SCRIPT_DIR/vna_scpi_example"
else
    echo "✗ 找不到可执行文件"
    echo "请先编译项目:"
    echo "  cd build && cmake .. && make"
    exit 1
fi

# 运行示例
$EXEC_PATH $VNA_IP $VNA_PORT

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "测试完成"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
