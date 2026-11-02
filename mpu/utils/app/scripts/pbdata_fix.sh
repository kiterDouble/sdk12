#!/bin/bash

# 定义路径
APP_DB="/oemapp/parameters/paramsDb"
DATA_DB="/oemdata/parameters/paramsDb"  # 目标数据库路径（保持不变）
DB_R_FILE="/oemdata/parameters/db_r"
TARGET_TABLE="paramstbl"  # 已替换为实际表名
DEBUG=1  # 调试模式

# 调试输出
debug_echo() {
    if [ $DEBUG -eq 1 ]; then
        echo "[DEBUG] $1"
    fi
}

# 检查并初始化标志文件
check_and_init_db_r() {
    if [ ! -f "$DB_R_FILE" ]; then
        debug_echo "Flag file $DB_R_FILE not found. Creating with 0."
        echo "0" > "$DB_R_FILE" || { echo "Error: Create $DB_R_FILE failed"; exit 1; }
        return 0
    fi
    local content=$(cat "$DB_R_FILE" | tr -d '[:space:]')
    [ "$content" = "1" ] && { echo "Process completed. Exiting."; exit 0; }
    [ "$content" = "0" ] && { debug_echo "Flag is 0. Proceeding."; return 0; }
    echo "Warning: $DB_R_FILE has invalid content. Treating as first run."
}

# 标记完成
mark_completed() {
    echo "1" > "$DB_R_FILE" || { echo "Error: Update $DB_R_FILE failed"; exit 1; }
    echo "Updated $DB_R_FILE to 1."
}

# 检查数据库存在
check_db_exists() {
    [ -f "$1" ] || { echo "Error: Database $1 not found"; exit 1; }
}

# 备份数据库
backup_data_db() {
    local backup_path="${DATA_DB}.backup_$(date +%Y%m%d%H%M%S)"
    echo "Backing up data DB to: $backup_path"
    cp "$DATA_DB" "$backup_path" || { echo "Error: Backup failed"; exit 1; }
}

# 检查目标表是否存在
check_table_exists() {
    local db_path=$1
    local table=$2
    local exists=$(sqlite3 "$db_path" "SELECT 1 FROM sqlite_master WHERE type='table' AND name='$table';")
    [ -z "$exists" ] && { echo "Error: Table $table not found in $db_path"; exit 1; }
}

# 获取表中最大的id值
get_max_id() {
    local db_path=$1
    local table=$2
    local max_id=$(sqlite3 "$db_path" "SELECT MAX(id) FROM $table;")
    [ -z "$max_id" ] && echo 0 || echo "$max_id"
}

# 同步缺失的记录（基于id）
sync_records() {
    local app_db=$1
    local data_db=$2
    local table=$3

    # 获取双方最大id
    local app_max_id=$(get_max_id "$app_db" "$table")
    local data_max_id=$(get_max_id "$data_db" "$table")
    echo "Max ID - APP: $app_max_id, DATA: $data_max_id"

    # 无需同步的情况
    if [ "$app_max_id" -le "$data_max_id" ]; then
        echo "No missing records (APP max ID <= DATA max ID)."
        return 0
    fi

    # 计算需要同步的id范围
    local start_id=$((data_max_id + 1))
    local end_id=$app_max_id
    echo "Need to sync records with ID from $start_id to $end_id."

    # 从APP_DB查询需要同步的记录（字段顺序与你的表结构匹配）
    local records=$(sqlite3 "$app_db" "SELECT id, name, value, type, min, max, paramSources, rpcWriteEnable FROM $table WHERE id BETWEEN $start_id AND $end_id;")

    # 逐条插入到DATA_DB
    local count=0
    while IFS='|' read -r id name value type min max paramSources rpcWriteEnable; do
        # 转义字符串中的单引号
        name=$(echo "$name" | sed "s/'/''/g")
        value=$(echo "$value" | sed "s/'/''/g")
        paramSources=$(echo "$paramSources" | sed "s/'/''/g")

        # 插入SQL（字段顺序与表结构一致）
        insert_sql="INSERT INTO $table (id, name, value, type, min, max, paramSources, rpcWriteEnable) VALUES ($id, '$name', '$value', $type, $min, $max, '$paramSources', $rpcWriteEnable);"
        debug_echo "Executing: $insert_sql"

        # 执行插入
        if sqlite3 "$data_db" "$insert_sql"; then
            ((count++))
            echo "Inserted record with ID: $id"
        else
            echo "Error: Failed to insert record with ID: $id"
        fi
    done <<< "$records"

    echo "Synced $count records successfully."
}

# 主流程
main() {
    check_and_init_db_r
    check_db_exists "$APP_DB"
    check_db_exists "$DATA_DB"
    backup_data_db

    # 检查目标表是否存在
    check_table_exists "$APP_DB" "$TARGET_TABLE"
    check_table_exists "$DATA_DB" "$TARGET_TABLE"

    # 同步记录
    echo -e "\nStarting record sync for table: $TARGET_TABLE"
    sync_records "$APP_DB" "$DATA_DB" "$TARGET_TABLE"

    echo -e "\nRecord sync completed."
    mark_completed
}

main
