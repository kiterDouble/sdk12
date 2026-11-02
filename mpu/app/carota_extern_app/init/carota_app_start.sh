#!/bin/bash

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oemapp/lib/
mkdir -p /mnt/carota/fota
mkdir -p /mnt/carota/dota
sleep 1

# 定义需要检查的文件列表（使用普通数组，按顺序对应）
targets=(
    "/mnt/carota/fota/tp_client"
    "/mnt/carota/fota/uds_shell"
    "/mnt/carota/fota/otamaster"
    "/mnt/carota/fota/mdasublauncher"
    "/mnt/carota/fota/otamda"
    "/mnt/carota/dota/diagmaster"
    "/mnt/carota/dota/rds_client"
)

sources=(
    "/oemapp/apps/tp_client"
    "/oemapp/apps/uds_shell"
    "/oemapp/apps/otamaster"
    "/oemapp/apps/mdasublauncher"
    "/oemapp/apps/otamda"
    "/oemapp/apps/diagmaster"
    "/oemapp/apps/rds_client"
)

# 先删除目标路径下的所有软件
echo "Cleaning up target directories..."
rm -rf /mnt/carota/fota/*
rm -rf /mnt/carota/dota/*
echo "Cleanup completed."

# 遍历检查并复制文件（现在目标路径为空，所有文件都会复制）
for i in "${!targets[@]}"; do
    dest="${targets[$i]}"
    src="${sources[$i]}"
    echo "Copying from $src to $dest..."
    
    # 获取目标文件所在的目录
    dest_dir=$(dirname "$dest")
    
    # 检查目录是否存在，不存在则创建（包括必要的父目录）
    if [ ! -d "$dest_dir" ]; then
        echo "Directory $dest_dir does not exist. Creating..."
        mkdir -p "$dest_dir"
    fi
    
    # 复制文件
    cp -r "$src" "$dest"
    
    # 检查复制是否成功
    if [ $? -eq 0 ]; then
        echo "Successfully copied $dest"
    else
        echo "Error: Failed to copy $dest"
    fi
done

sleep 1

# 进入守护进程循环：每5分钟检查一次进程状态
while true; do
    # 检查otamaster和mdasublauncher是否存在
    if ! pgrep otamaster > /dev/null || ! pgrep mdasublauncher > /dev/null; then
        echo "otamaster or mdasublauncher is not running. Killing both and restarting otamaster..."
        
        # 杀掉otamaster进程（如果存在）
        if pgrep otamaster > /dev/null; then
            pkill -9 otamaster
            echo "Killed otamaster."
        fi
        
        # 杀掉mdasublauncher进程（如果存在）
        if pgrep mdasublauncher > /dev/null; then
            pkill -9 mdasublauncher
            echo "Killed mdasublauncher."
            sleep 2
        fi
        
        # 重新启动otamaster
        echo "Restarting otamaster..."
        /mnt/carota/fota/otamaster &
    else
        echo "otamaster and mdasublauncher is already running."   # 可替换为实际需要的操作
    fi

    # 检查diagmaster是否需要启动
    if ! pgrep diagmaster > /dev/null; then
        echo "diagmaster is not running. Restarting..."
        /mnt/carota/dota/diagmaster &
    else
        echo "diagmaster is already running."   # 可替换为实际需要的操作
    fi

    # 休眠5分钟（300秒）
    echo "sleep 100"
    sleep 100
done