#!/bin/sh
# 用法: ./start_selfupgrade.sh <upgradeFlag>
# upgradeFlag升级标志: 1=只升级MCU，2=只升级MPU，3=MCU+MPU全升级

# 参数检查
if [ $# -ne 1 ]; then
    echo "用法: $0 <upgradeFlag>"
    exit 1
fi

# 升级标志
upgradeFlag=$1
# 升级包目录
export UPGRADE_PACKAGE_DIR=/sdcard/selfUpgrade/
# 解压目录
export DECOMPRESS_DIR=/sdcard/selfUpgrade/decompress

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oemapp/lib/
fota_daemon_ret=0

# Step 1: 确认升级包 TBOX.*.zip 是否存在
pkg=$(ls "$UPGRADE_PACKAGE_DIR"/TBOX.*.zip 2>/dev/null | head -n 1)
if [ -z "$pkg" ]; then
    echo "错误: 未找到升级包 TBOX.*.zip"
    exit 1
fi

# Step 2: 解压升级包
if [ ! -d "$DECOMPRESS_DIR" ]; then
    mkdir -p "$DECOMPRESS_DIR" || {
        echo "错误: 无法创建目录 $DECOMPRESS_DIR"
        exit 1
    }
else
    rm -rf "$DECOMPRESS_DIR"/* || {
        echo "错误: 无法清空目录 $DECOMPRESS_DIR"
        exit 1
    }
fi

unzip -oq "$pkg" -d "$DECOMPRESS_DIR" || {
    echo "错误: 解压升级包失败: $pkg"
    exit 1
}
# 解压完成后清除原始升级压缩包
rm -rf "$UPGRADE_PACKAGE_DIR"/TBOX.*.zip
echo "解压完成: $pkg -> $DECOMPRESS_DIR"

# Step 3: 解压MCU升级包
tar -zxvf "$DECOMPRESS_DIR"/mcu.tgz -C "$DECOMPRESS_DIR" || {
    echo "错误: 解压MCU升级包失败: $pkg"
    exit 1
}

# Step 4: 校验 MD5
md5file="$DECOMPRESS_DIR/md5sum"
if [ ! -f "$md5file" ]; then
    echo "错误: 未找到 md5sum 文件"
    exit 1
fi

check_file_md5() {
    file=$1
    expected=$(grep "^$file:" "$md5file" | cut -d: -f2)
    if [ -z "$expected" ]; then
        echo "错误: md5sum 文件中缺少 $file 的记录"
        exit 1
    fi
    actual=$(md5sum "$DECOMPRESS_DIR/$file" | awk '{print $1}')
    if [ "$actual" != "$expected" ]; then
        echo "错误: $file 校验失败"
        echo "  期望: $expected"
        echo "  实际: $actual"
        exit 1
    else
        echo "$file 校验通过"
    fi
}

check_file_md5 update.zip
check_file_md5 mcu.tgz
check_file_md5 version.txt

echo "所有文件 MD5 校验通过"

# Step 4: 开始执行自升级
echo "升级标志 upgradeFlag = $upgradeFlag "
echo "升级中, MPU预估升级时间: 10 sec, MCU预估升级时间: 1 min, 请等待..."

timeout 600 /oemapp/apps/otaDaemon $upgradeFlag
fota_daemon_ret=$?
# 自升级成功返回0，参数有误返回1，自升级启动失败返回2，自升级失败返回3
if [ "$fota_daemon_ret" -eq 0 ]; then
    echo "自升级成功！"
    reboot
    if [ "$upgradeFlag" -eq 2 ]; then
        echo "仅升级MPU成功,触发重启..."
        reboot
    fi
    exit 0
else
    echo "自升级失败, 原因: $fota_daemon_ret"
    exit 1
fi
