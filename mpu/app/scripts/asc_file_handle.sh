#!/bin/bash

find_files_by_time()
{
	TIME_SPEC1="$1"
	TIME_SPEC2="$2"
	SEARCH_DIR="$3"
	FILE_END="$4"
	# 将时间字符串转换为Unix时间戳
	if [[ "$TIME_SPEC1" =~ ^[0-9]+$ ]]; then
		# 已经是Unix时间戳
		EDN_TIME="$TIME_SPEC1"
	else
		# 尝试将日期字符串转换为Unix时间戳
		if ! EDN_TIME=$(date -d "$TIME_SPEC1" +%s 2>/dev/null); then
			echo "错误: 时间格式无效 '$TIME_SPEC'" >&2
			echo "      请使用Unix时间戳或可识别的日期格式" >&2
			return 1
		fi
	fi
	
	if [[ "$TIME_SPEC2" =~ ^[0-9]+$ ]]; then
		# 已经是Unix时间戳
		START_TIME="$TIME_SPEC2"
	else
		# 尝试将日期字符串转换为Unix时间戳
		if ! START_TIME=$(date -d "$TIME_SPEC2" +%s 2>/dev/null); then
			echo "错误: 时间格式无效 '$TIME_SPEC2'" >&2
			echo "      请使用Unix时间戳或可识别的日期格式" >&2
			return 1
		fi
	fi
	
	# 使用临时文件存储结果
	TEMP_FILE=$(mktemp)
	find "$SEARCH_DIR" -type f -print0 | while IFS= read -r -d $'\0' file; do
    # 获取文件的修改时间(mtime)和状态改变时间(ctime)
    MTIME=$(stat -c %Y "$file")
    CTIME=$(stat -c %Z "$file")
	basename="${file##*/}"  # 提取文件名部分
	timestr="${basename#can_}"  # 移除前缀 "can_"
	timestr="${timestr%${FILE_END}}"   # 移除后缀 ".asc"
	formatted_timestr=$(echo "$timestr" | sed 's/_/-/;s/_/-/;s/_/ /;s/_/:/;s/_/:/')
	CREATE_TIME=`date -d "$formatted_timestr" +%s`
	echo  $formatted_timestr  > a.log
    # 检查条件: 创建时间(ctime)早于结束时间 且 修改时间(mtime)早于开始时间
    if [ $CREATE_TIME -gt $EDN_TIME ] && [ $MTIME -gt $EDN_TIME ]; then
		continue
    fi
    if [ $CREATE_TIME -lt $START_TIME ] && [ $MTIME -gt $START_TIME ]; then
		printf "%d\t%s\n" "$CREATE_TIME" "$file" >> "$TEMP_FILE"
		continue
    fi
	if [ $CREATE_TIME -lt $START_TIME ] && [ $MTIME -gt $EDN_TIME ]; then
		printf "%d\t%s\n" "$CREATE_TIME" "$file" >> "$TEMP_FILE"
		continue
    fi
	if [ $CREATE_TIME -lt $EDN_TIME ] && [ $MTIME -gt $EDN_TIME ]; then
		printf "%d\t%s\n" "$CREATE_TIME" "$file" >> "$TEMP_FILE"
		continue
    fi
	if [ $CREATE_TIME -gt $START_TIME ] && [ $MTIME -gt $EDN_TIME ]; then
		printf "%d\t%s\n" "$CREATE_TIME" "$file" >> "$TEMP_FILE"
		continue
    fi
	if [ $CREATE_TIME -gt $START_TIME ] && [ $MTIME -lt $EDN_TIME ]; then
		printf "%d\t%s\n" "$CREATE_TIME" "$file" >> "$TEMP_FILE"
		continue
    fi
	done
	
	sort -n "$TEMP_FILE" | while IFS=$'\t' read -r ctime file; do
		echo "$file"
	done
	return 0
}


handle_asc_time()
{
    OFFSET="$1"
    file_name="$2"
    
    # 验证偏移量格式（也需要支持负数）
    if [[ ! "$OFFSET" =~ ^[-+]?[0-9]+(\.[0-9]+)?$ ]]; then
        echo "错误: 偏移量格式无效 '$OFFSET'" >&2
        return 1
    fi

    # 检查文件
    if [ ! -f "$file_name" ] || [[ "$file_name" != *.asc ]]; then
        echo "警告: 文件 '$file_name' 无效，跳过" >&2
        return 1
    fi
    
    echo "处理文件: $file_name"
    
    # 使用更高效的mktemp方法
    TMP_FILE=$(mktemp) || return 1
    
    # 使用修复后的AWK处理，支持负时间戳
    awk -v offset="$OFFSET" '
    BEGIN {
        OFMT = "%.6f"
    }
    {
        # 匹配正数、负数和带正号的数
        if ($1 ~ /^[-+]?[0-9]+(\.[0-9]+)?$/) {
            $1 = sprintf("%.6f", $1 + offset)
        }
        print
    }' "$file_name" > "$TMP_FILE"
    
    # 原子性替换文件
    # 原代码：
	# mv "$TMP_FILE" "$file_name"
	if cp --preserve=all "$TMP_FILE" "$file_name"; then
		rm -f "$TMP_FILE"
	else
		rm -f "$TMP_FILE"
		echo "错误: 无法复制临时文件到 '$file_name'" >&2
		return 1
	fi
    
    echo "完成: $file_name"
}

process_tar_file() {    
    # 从函数读取每一行
	tmpTarDir=$4
	mkdir -p ${tmpTarDir}
    while IFS= read -r line; do
		echo "$line"
		case "$line" in
			*.tgz)
				tar -zxf $line -C ${tmpTarDir}
				;;
			*.asc)
				cp $line ${tmpTarDir}
				;;
			*)
				echo "未知文件类型"
				;;
		esac
		#tar -zxf $line -C ${tmpTarDir}
    done < <(find_files_by_time "$1" "$2" "$3" ".tgz")
}

merge_files_without_header() {
    local time_arg1="$1"
    local time_arg2="$2"
    local time_arg3="$3"
    local output_file="$4"
    
    # 创建临时文件用于存储最终合并结果
    local merged_file=$(mktemp) || return 1
    
    # 初始化计数器
    local i=0
	
	if [[ "$time_arg2" =~ ^[0-9]+$ ]]; then
		# 已经是Unix时间戳
		START_TIME="$time_arg2"
	else
		# 尝试将日期字符串转换为Unix时间戳
		if ! START_TIME=$(date -d "$time_arg2" +%s 2>/dev/null); then
			echo "错误: 时间格式无效 '$time_arg2'" >&2
			echo "      请使用Unix时间戳或可识别的日期格式" >&2
			return 1
		fi
	fi
	
	echo "date" `date -d "@$START_TIME" "+%a %b %d %r %Y" | sed 's/\([0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}\)/\1'".000"'/' | sed 's/AM/am/; s/PM/pm/'` > "$merged_file"
	echo "base hex timestamps absolute" >> "$merged_file"
	echo "internal events logged" >> "$merged_file"
	echo "// version 18.3.0" >> "$merged_file"
    
    # 读取文件列表并处理每个文件
    while IFS= read -r file; do
        i=$((i+1))
        echo "处理第${i}个文件: $file"
        
        # 检查文件是否存在
        if [ ! -f "$file" ]; then
            echo "警告: 文件 '$file' 不存在，跳过" >&2
            continue
        fi
        
        # 去除文件的前4行并追加到合并文件
        tail -n +5 "$file" >> "$merged_file"
        
        # 可选：在每个文件内容之间添加分隔行
        echo "" >> "$merged_file"
        
    done < <(find_files_by_time "$time_arg1" "$time_arg2" "$time_arg3" ".asc")
    
    # 移动合并文件到指定位置
    #mv "$merged_file" "$output_file"
	if cp --preserve=all "$merged_file" "$output_file"; then
		rm -f "$merged_file"
	else
		rm -f "$merged_file"
		echo "错误: 无法复制临时文件到 '$output_file'" >&2
		return 1
	fi
    
    echo "已合并 $i 个文件到: $output_file"
}

process_asc_file() {    
    # 从函数读取每一行
	tmpTarDir=$4
	tmpStartTime=$2
	
	if [[ "$tmpStartTime" =~ ^[0-9]+$ ]]; then
		# 已经是Unix时间戳
		startTime="$tmpStartTime"
	else
		# 尝试将日期字符串转换为Unix时间戳
		if ! startTime=$(date -d "$tmpStartTime" +%s 2>/dev/null); then
			echo "错误: 时间格式无效 '$tmpStartTime'" >&2
			echo "      请使用Unix时间戳或可识别的日期格式" >&2
			return 1
		fi
	fi
	
    while IFS= read -r line; do
		echo "$line"
		basename="${line##*/}"  # 提取文件名部分
		timestr="${basename#can_}"  # 移除前缀 "can_"
		timestr="${timestr%.asc}"   # 移除后缀 ".asc"
		formatted_timestr=$(echo "$timestr" | sed 's/_/-/;s/_/-/;s/_/ /;s/_/:/;s/_/:/')
		FILE_CREATE_TIME=`date -d "$formatted_timestr" +%s`
		handle_asc_time $(($FILE_CREATE_TIME - $startTime)) $line
    done < <(find_files_by_time "$1" "$2" "$tmpTarDir" ".asc")
}

filter_asc_timestamp() 
{
    local input_file="$1"
    local TMP_FILE=$(mktemp)
	
	TIME_SPEC1="$2"
	TIME_SPEC2="$3"
	# 将时间字符串转换为Unix时间戳
	if [[ "$TIME_SPEC1" =~ ^[0-9]+$ ]]; then
		# 已经是Unix时间戳
		EDN_TIME="$TIME_SPEC1"
	else
		# 尝试将日期字符串转换为Unix时间戳
		if ! EDN_TIME=$(date -d "$TIME_SPEC1" +%s 2>/dev/null); then
			echo "错误: 时间格式无效 '$TIME_SPEC1'" >&2
			echo "      请使用Unix时间戳或可识别的日期格式" >&2
			return 1
		fi
	fi
	
	if [[ "$TIME_SPEC2" =~ ^[0-9]+$ ]]; then
		# 已经是Unix时间戳
		START_TIME="$TIME_SPEC2"
	else
		# 尝试将日期字符串转换为Unix时间戳
		if ! START_TIME=$(date -d "$TIME_SPEC2" +%s 2>/dev/null); then
			echo "错误: 时间格式无效 '$TIME_SPEC2'" >&2
			echo "      请使用Unix时间戳或可识别的日期格式" >&2
			return 1
		fi
	fi
    
    # 使用 awk 处理
    awk -v start="$START_TIME" -v end="$EDN_TIME" '
    {
        if ($1 ~ /^[-+]?[0-9]+(\.[0-9]+)?$/) {
            if ($1 >= 0 && $1 <= (end - start)) {
                print
            }
        } else {
            print
        }
    }' "$input_file" > "$TMP_FILE"
    
    # 替换原文件
    #mv "$TMP_FILE" "$input_file"
	if cp --preserve=all "$TMP_FILE" "$input_file"; then
		rm -f "$TMP_FILE"
	else
		rm -f "$TMP_FILE"
		echo "错误: 无法复制临时文件到 '$input_file'" >&2
		return 1
	fi
    
    echo "已完成对文件 $input_file 的处理"
}



process_tar_file "$1" "$2" "$3" "$4"

cp "$3"/*.asc "$4"

process_asc_file "$1" "$2" "$3" "$4"

merge_files_without_header "$1" "$2" "$4" "$3/tmp.asc"

filter_asc_timestamp "$3/tmp.asc" "$1" "$2"
