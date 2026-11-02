#!/bin/sh
###############################################################################
# 脚本名称: gnss_tracking_threshold.sh
#
# 脚本作用:
#   用于在调试/测试场景下，通过串口直接与 GNSS 模块交互，
#   配置 GNSS 芯片的 Tracking 信噪比门限（25 dB），并将配置固化到 Flash。
#
#   内置重试机制：
#     - 单次配置失败后等待 1s 重试
#     - 最多重试 3 次
#
###############################################################################

FLAG_FILE="/oemdata/set_gnss_threshold_done.flag"
TTY_DEV="/dev/ttyS3"
LOG_FILE="/data/ttyS3.log"
PWR_NODE="/sys/kernel/debug/gnss_gpio/gnss_power_on_off"

MAX_RETRY=3
RETRY_DELAY=1

set -e

###############################################################################
# 一次 GNSS Tracking 门限设置流程
###############################################################################
do_set_gnss_threshold()
{
    # step1、停止 locationd 进程，释放串口
    /etc/init.d/ql_locationd stop || true

    # step2、GNSS 下电
    echo off > "${PWR_NODE}"

    # step3、设置 ttyS3 通讯速率
    stty -F "${TTY_DEV}" speed 115200

    # step4、配置串口跟 GNSS 交互（后台保存日志）
    cat "${TTY_DEV}" > "${LOG_FILE}" 2>/dev/null &
    CAT_PID=$!

    # step5、GNSS 上电
    echo on > "${PWR_NODE}"

    # step6、设置 25 dB tracking 门限（私有二进制指令）
    echo -en \
'\xBA\xCE\x2C\x00\x06\x07\x08\x00\x00\x00\x00\x30\x00\x00\x19\x00\x01\x00\x00\x02\xF4\x09\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x4D\x32\xFB\x10' \
    > "${TTY_DEV}"

    # step7、修改完成后查询参数，确认是否配置成功
    echo -en '\xBA\xCE\x00\x00\x06\x07\x00\x00\x06\x07' > "${TTY_DEV}"

    # step8、将配置固化到 GNSS 模块 Flash
    echo -en '$PCAS00*01\r\n' > "${TTY_DEV}"

    # 结束后台抓串口日志
    kill "${CAT_PID}" 2>/dev/null || true

    return 0
}

###############################################################################
# 主流程
###############################################################################

# 已配置过则直接退出
if [ -f "${FLAG_FILE}" ]; then
    echo "GNSS tracking threshold already configured, skip. (${FLAG_FILE} exists)"
    exit 0
fi

i=1
while [ "${i}" -le "${MAX_RETRY}" ]; do
    echo "[INFO] GNSS threshold config attempt ${i}/${MAX_RETRY}"

    # 关键点：在函数调用处临时关闭 set -e，捕获失败
    set +e
    do_set_gnss_threshold
    ret=$?
    set -e

    if [ "${ret}" -eq 0 ]; then
        touch "${FLAG_FILE}"
        sync
        echo "GNSS tracking threshold configured successfully, flag created: ${FLAG_FILE}"
        exit 0
    fi

    echo "[WARN] Attempt ${i} failed"

    if [ "${i}" -lt "${MAX_RETRY}" ]; then
        echo "[INFO] Retry after ${RETRY_DELAY}s..."
        sleep "${RETRY_DELAY}"
    fi

    i=$((i + 1))
done

echo "[ERROR] GNSS tracking threshold configuration failed after ${MAX_RETRY} attempts"
exit 1
