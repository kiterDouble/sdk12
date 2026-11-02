#!/bin/sh

# 简单的 GPIO 控制脚本
# 参数:
#   $1: GPIO 号，例如 31
#   $2: 方向: in / out
#   $3: 电平: 0 / 1 / low / high （仅在 out 时生效）

show_usage() {
    echo "用法: $0 <gpio_num> <direction> <value>"
    echo "  <gpio_num>   : GPIO 号，例如 31"
    echo "  <direction>  : in 或 out"
    echo "  <value>      :"
    echo "                 - 对于 out: 0 / 1 / low / high"
    echo "                 - 对于 in : 可随便填(不会使用)，例如 0"
    echo
    echo "示例:"
    echo "  $0 31 out 0        # 导出 GPIO31，配置为输出，拉低"
    echo "  $0 31 out 1        # 导出 GPIO31，配置为输出，拉高"
    echo "  $0 31 out high     # 导出 GPIO31，配置为输出，拉高"
    echo "  $0 31 out low      # 导出 GPIO31，配置为输出，拉低"
    echo "  $0 32 in 0         # 导出 GPIO32，配置为输入（忽略 value 参数）"
}

# 参数数量检查
if [ $# -lt 3 ]; then
    echo "错误: 参数数量不足"
    show_usage
    exit 1
fi

GPIO="$1"
DIR="$2"
VAL="$3"

# GPIO 号检查（简单判断是否为数字）
case "$GPIO" in
    ''|*[!0-9]*)
        echo "错误: GPIO 号必须是数字，当前为: $GPIO"
        exit 1
        ;;
esac

# 统一转小写
DIR=$(echo "$DIR" | tr 'A-Z' 'a-z')
VAL=$(echo "$VAL" | tr 'A-Z' 'a-z')

# 方向检查
if [ "$DIR" != "in" ] && [ "$DIR" != "out" ]; then
    echo "错误: direction 只能是 in 或 out，当前为: $DIR"
    show_usage
    exit 1
fi

# 输出时才检查电平参数
if [ "$DIR" = "out" ]; then
    case "$VAL" in
        0|1|low|high)
            ;;
        *)
            echo "错误: 对于 out，value 只能是 0 / 1 / low / high，当前为: $VAL"
            show_usage
            exit 1
            ;;
    esac
fi

# 将 low/high 转为 0/1
if [ "$VAL" = "low" ]; then
    VAL=0
elif [ "$VAL" = "high" ]; then
    VAL=1
fi

GPIO_PATH="/sys/class/gpio/gpio$GPIO"

# 导出 GPIO（如果还没导出）
if [ ! -d "$GPIO_PATH" ]; then
    echo "$GPIO" > /sys/class/gpio/export 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "错误: 无法导出 GPIO $GPIO（可能需要 root 权限或该 GPIO 不支持导出）"
        exit 1
    fi
fi

# 设置方向
echo "$DIR" > "$GPIO_PATH/direction" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "错误: 无法设置 GPIO $GPIO 方向为 $DIR"
    exit 1
fi

# 如果是输出，设置电平
if [ "$DIR" = "out" ]; then
    echo "$VAL" > "$GPIO_PATH/value" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "错误: 无法设置 GPIO $GPIO 电平为 $VAL"
        exit 1
    fi
    echo "成功: GPIO $GPIO 已设置为 OUTPUT，电平=$VAL"
else
    echo "成功: GPIO $GPIO 已设置为 INPUT（value 参数已忽略）"
fi
