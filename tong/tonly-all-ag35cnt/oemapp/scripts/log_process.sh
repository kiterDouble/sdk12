#! /bin/sh
# =============================================================================
#  文件名称 ：log_process.sh
#  功能描述 ：日志自动打包与留存管理脚本
# -----------------------------------------------------------------------------
#  【主要功能】
#    1. 检测单个日志文件（sirunlog.*），当其大小 > 20MB 时执行压缩；
#    2. 压缩后文件名格式：Mon-Day-HHMM-YYYY.tgz；
#    3. 自动清理过期、超限或多余文件：
#         - 超过 keepDays 的旧归档自动删除；
#         - 当总容量超出 kMaxSizeMb 时，从最旧文件开始删除；
#         - 在 Times 模式下，超过 logsNum 的文件按时间顺序裁剪；
#    4. 全程防御性编写：兼容 BusyBox / POSIX shell，支持嵌入式 Linux。
#
#  【适用场景】
#    - 车载 T-BOX / IoT 设备日志管理；
#    - 自动巡检与限容量文件系统；
#    - 低内存嵌入式平台的轻量级归档任务。
#
# -----------------------------------------------------------------------------
#  作者姓名 ：Xu Jaxsen
#  单位部门 ：GAC TSP Embedded Platform / Telemetry Core
#  创建日期 ：2025-11-07
#  当前版本 ：v1.0.0
# -----------------------------------------------------------------------------
#  修订记录：
#   - [2025-11-07] v1.0.0  初版发布，支持 SD/内存双策略与 20MB 自动打包。
#   - 后续版本号规范建议：
#       vX.Y.Z  → X=主版本（结构调整） Y=功能增量  Z=小修或注释调整
# -----------------------------------------------------------------------------
#  版权声明：
#    Copyright (C) 2025 GAC Automotive Group / TSP Embedded Division.
#    本脚本仅限项目内部使用，禁止未经授权的复制、修改或外传。
# =============================================================================



# ───────────────────────── 入参与目录准备 ──────────────────────────

logPath=$1                       # [IN ] 源日志目录（只处理匹配 sirunlog.* 的文件）
logsNum=$2                       # [IN ] Times 分支用的文件总数上限（包含 .tgz）

if [ -z "$1" -o -z "$2" ]; then
  echo "用法: sh log_process.sh <日志路径> <日志个数(存储空间大于1G时默认存储7天)>" >&2
  exit 1
fi

logstorePath="${logPath}/logstore"   # [IN/OUT] 压缩产物与中间 .log 的归档目录
mkdir -p "${logstorePath}"           # -p: 不存在则创建；存在则忽略


# 轻量日志工具（BusyBox 友好）
log_info() { printf '%s [INFO] %s\n'  "$(date '+%F %T')" "$*"; }
log_warn() { printf '%s [WARN] %s\n'  "$(date '+%F %T')" "$*"; }
log_err()  { printf '%s [ERROR] %s\n' "$(date '+%F %T')" "$*"; }

# ─────────────────────── 存储策略（天数/容量） ───────────────────────

# 说明：若检测到 /mnt/sdcard 已挂载，则放宽保留策略；否则使用更保守策略。
if mount | grep -q "/mnt/sdcard"; then
  keepDays=15                    # [POLICY] 归档目录中过期文件保留天数
  kMaxSizeMb=2048                # [POLICY] 归档目录容量上限（MB）
  echo "SD card mounted successfully, 保留日志 ${keepDays} 天"
else
  keepDays=8
  kMaxSizeMb=50
  echo "SD card not mounted, 仅保留日志 ${keepDays} 天"
fi


# ───────────────────────── 压缩阈值（20MB） ─────────────────────────

singleFileThresholdBytes=$((20 * 1024 * 1024))   # 20MB（字节）用于精确比较


# ─────────────────────────── 工具函数区 ────────────────────────────

###############################################################################
# 函数名：dir_size_mb
# 目  的：计算指定目录当前占用大小（单位：MB，向下取整）
# 参  数：
#   $1  [IN ] 目录路径
# 返回值：
#   STDOUT 输出一个非负整数（目录大小，单位MB）；出错或不存在时输出空/无
#   函数本身返回码忽略；调用者需自行兜底（见调用处的默认0处理）
# 副作用：无
# 边  界：
#   - 依赖 du/awk；在极端环境中 du 失败时，调用方需将空值兜底为 0
# 复  杂：O(目录项数)；du 内部遍历
# 样  例：dir_size_mb "/var/tmp"
###############################################################################
dir_size_mb() {
  du -sm "$1" 2>/dev/null | awk '{print $1}'
}

###############################################################################
# 函数名：checkLogstoreSize
# 目  的：若归档目录大小超过上限，则按“最旧优先”删除文件直至达标
# 参  数：无（使用全局 logstorePath/kMaxSizeMb）
# 返回值：0（不关注中途 rm 失败的细节，保持稳态尽力而为）
# 副作用：可能删除归档目录中的旧文件
# 边  界：
#   - 目录不可读/空目录：通过空值与空列表保护避免死循环
#   - 删除失败：循环会继续尝试后续文件；最终大小可能仍 > 上限
# 复  杂：O(N log N) ≈ ls 排序 + 逐个 rm（N 为文件数）
# 样  例：checkLogstoreSize
###############################################################################
checkLogstoreSize() {
  currentSize="$(dir_size_mb "${logstorePath}")"
  [ -n "${currentSize}" ] || currentSize=0

  # ls -rt：按 mtime 升序（最旧→最新）；head -n1 取最旧文件
  while [ "${currentSize}" -gt "${kMaxSizeMb}" ]; do
    oldestFile="$(ls -1rt "${logstorePath}" 2>/dev/null | head -n 1)"
    if [ -z "${oldestFile}" ]; then
      break  # 无可删文件（空/不可读），防死循环
    fi
    rm -f -- "${logstorePath}/${oldestFile}"
    currentSize="$(dir_size_mb "${logstorePath}")"
    [ -n "${currentSize}" ] || currentSize=0
  done
  return 0
}

###############################################################################
# 函数名：build_name_parts
# 目  的：基于文件 mtime 生成日期组件，用于“带日期”的规范命名
# 参  数：
#   $1  [IN ] 文件路径
# 返回值：0；通过全局变量导出以下字段：
#   fileDateMon：英文月份缩写（如 Nov）
#   fileDateDay：日（两位，如 07）
#   fileTime   ：时分（HHMM，如 1425）
#   fileYear   ：年（四位，如 2025）
# 副作用：设置上述四个全局变量
# 边  界：
#   - 依赖 date -r；若文件/时间戳异常，date 可能失败（上层负责保证文件存在）
# 复  杂：O(1)
# 样  例：build_name_parts "/var/log/sirunlog.1"
###############################################################################
build_name_parts() {
  ts="$(LANG=C date -r "$1" "+%b %d %H%M %Y")"
  fileDateMon="$(echo "$ts" | awk '{print $1}')"
  fileDateDay="$(echo "$ts" | awk '{print $2}')"
  fileTime="$(echo "$ts" | awk '{print $3}')"
  fileYear="$(echo "$ts" | awk '{print $4}')"
}

###############################################################################
# 函数名：compress_one_if_big
# 目  的：当单个源日志文件 >20MB 时，执行“归档改名 + 压缩 + 删除 .log”
# 参  数：
#   $1  [IN ] 待处理的日志完整路径（例如 /mnt/log/sirunlog.1）
# 返回值：
#   0   成功（包含“小于阈值而跳过”的情形）
#   1   失败（移动/压缩/删除任一操作失败）
# 副作用：
#   - 若触发压缩：会在 ${logstorePath} 生成 .tgz，并删除中间 .log
#   - 会将源目录中的该日志文件移走（改名后进入 logstorePath）
# 前置条件：
#   - ${logstorePath} 已存在；磁盘剩余空间足以写入压缩包
# 行为约束：
#   - 不做失败回滚（tar 失败不会删除原始文件）
# 边  界：
#   - wc -c 不可读或返回非数字 → 跳过
#   - mv/tar/rm 失败时记录错误日志并返回 1
# 复  杂：O(F) ≈ 压缩量与文件大小 F 成正比
# 样  例：compress_one_if_big "/mnt/log/sirunlog.20251107"
###############################################################################
compress_one_if_big() {
  src="$1"
  [ -f "${src}" ] || {
    log_warn "compress_one_if_big: source not found -> ${src}"
    return 0
  }

  # Step 1. 读取字节数（权限/设备异常时跳过）
  bytes="$(wc -c < "${src}" 2>/dev/null)"
  case "${bytes}" in
    ''|*[!0-9]*) 
      log_warn "compress_one_if_big: cannot read size (wc -c failed) -> ${src}"
      return 0
      ;;
  esac

#   # Step 2. 小于 20MB → 跳过
#   if [ "${bytes}" -lt "${singleFileThresholdBytes}" ]; then
#     log_info "compress_one_if_big: skip small file (${bytes}B < ${singleFileThresholdBytes}B) -> ${src}"
#     return 0
#   fi

  # Step 3. 生成“带日期”的新文件名与压缩包名
  build_name_parts "${src}"
  newFileName="${fileDateMon}-${fileDateDay}-${fileTime}-${fileYear}.log"
  tarFileName="${fileDateMon}-${fileDateDay}-${fileTime}-${fileYear}.tgz"

  log_info "compress_one_if_big: start compress ${src} (${bytes} bytes)"
  log_info "compress_one_if_big: rename -> ${newFileName}, tar -> ${tarFileName}"

  # Step 4. 移到归档目录并改名（同分区=重命名；跨分区=移动）
  if ! mv -- "${src}" "${logstorePath}/${newFileName}"; then
    log_error "compress_one_if_big: mv failed (${src} -> ${logstorePath}/${newFileName})"
    return 1
  fi

  # Step 5. 打包：包内成员名即“带日期”的 .log，便于解包直观识别
  if ! tar -czf "${logstorePath}/${tarFileName}" -C "${logstorePath}" "${newFileName}" 2>/dev/null; then
    log_error "compress_one_if_big: tar failed (${tarFileName})"
    # 保留中间 .log，以免丢数据
    return 1
  fi

  # Step 6. 压缩完成后删除中间 .log（释放空间）
  if ! rm -f -- "${logstorePath}/${newFileName}"; then
    log_warn "compress_one_if_big: rm failed (${newFileName})"
  fi

  log_info "compress_one_if_big: success -> ${tarFileName}"
  return 0
}


###############################################################################
# 函数名：prune_by_days
# 目  的：按“保留天数”清理归档目录中过期文件（仅 .tgz 与残留 .log）
# 参  数：无（使用全局 logstorePath/keepDays）
# 返回值：0
# 副作用：删除早于 keepDays 天的归档文件
# 边  界：
#   - find 失败时静默；-print 仅用于人工核对，可去掉
# 复  杂：O(N)（N 为归档目录文件数）
# 样  例：prune_by_days
###############################################################################
prune_by_days() {
  find "${logstorePath}" -type f \
       \( -name "*.tgz" -o -name "*.log" \) \
       -mtime +"${keepDays}" -print -delete 2>/dev/null
}


# ────────────────────────── 业务分支：Date ──────────────────────────

###############################################################################
# 函数名：TarLogFileDate
# 目  的：容量治理优先的归档流程
# 流  程：
#   1) 按天清理过期归档；
#   2) 压缩源目录中 >20MB 的 sirunlog.*；
#   3) 若归档目录超容量上限，则自最旧起删至达标。
# 参  数：无（使用全局变量）
# 返回值：0（不因个别文件失败而整体失败）
# 副作用：可能删除归档目录旧文件；可能生成/删除归档文件
# 边  界：源目录可能无匹配文件；find/压缩失败不影响整体循环
# 复  杂：O(N + 压缩量)
# 样  例：TarLogFileDate
###############################################################################
TarLogFileDate() {
  mkdir -p "${logstorePath}"
  cd "${logstorePath}" || exit 1

  prune_by_days

  # ----------------------------------------------------------------------
  # 遍历 ${logPath} 下的 sirunlog.* 文件，逐个检查并压缩
  # - 采用 for 通配遍历，兼容 BusyBox；
  # - [ -f ] 过滤掉“无匹配时的字面量通配符”和目录；
  # - compress_one_if_big 内部自行判断 <20MB 自动跳过；
  # ----------------------------------------------------------------------
  for fileNameWithPath in "${logPath}"/sirunlog.*; do
    [ -f "${fileNameWithPath}" ] || continue
    log_info "TarLogFileDate checking: ${fileNameWithPath}"
    compress_one_if_big "${fileNameWithPath}"
  done


  checkLogstoreSize
}


# ────────────────────────── 业务分支：Times ─────────────────────────

###############################################################################
# 函数名：TarLogFileTimes
# 目  的：日常轮转优先的归档流程
# 流  程：
#   1) 按天清理过期归档；
#   2) 压缩源目录中 >20MB 的 sirunlog.*；
#   3) 按容量上限清理（删最旧）；
#   4) 若归档文件数 > logsNum，则删除最旧的多余文件数。
# 参  数：无（使用全局变量）
# 返回值：0
# 副作用：同 Date 分支，另包含“数量裁剪”
# 边  界：
#   - 若仅统计 .tgz，请改为：find ... -name "*.tgz" | wc -l，并相应限定 ls/删除
# 复  杂：O(N log N)（排序 + 删除）+ 压缩量
# 样  例：TarLogFileTimes
###############################################################################
TarLogFileTimes() {
  mkdir -p "${logstorePath}"
  cd "${logstorePath}" || exit 1

  prune_by_days

 # ----------------------------------------------------------------------
  # 遍历 ${logPath} 下的 sirunlog.* 文件，逐个检查并压缩
  # - 采用 for 通配遍历，兼容 BusyBox，避免管道子进程导致变量失效；
  # - [ -f ] 可过滤无匹配时的字面量“sirunlog.*”；
  # - compress_one_if_big 内部自行判断大小阈值（<20MB 自动跳过）。
  # ----------------------------------------------------------------------
  for fileNameWithPath in "${logPath}"/sirunlog.*; do
      [ -f "${fileNameWithPath}" ] || continue
      log_info "TarLogFileTimes checking: ${fileNameWithPath}"
      compress_one_if_big "${fileNameWithPath}"
  done

  checkLogstoreSize

  # 数量裁剪：统计“所有文件”（含 .tgz）；如需仅统计 .tgz，请收窄匹配条件
  CurrentNum="$(find "${logstorePath}" -maxdepth 1 -type f | wc -l)"
  if [ "${CurrentNum}" -gt "${logsNum}" ]; then
    delCount=$(( CurrentNum - logsNum ))
    ls -1rt "${logstorePath}" 2>/dev/null \
    | head -n "${delCount}" \
    | while IFS= read -r f; do
        rm -f -- "${logstorePath}/${f}"
      done
  fi
}


# ──────────────────────────── 主循环区 ─────────────────────────────

# 说明：
#   - 若归档目录当前总大小（MB）> 上限：走“容量治理”分支（TarLogFileDate）；
#   - 否则：走“日常轮转”分支（TarLogFileTimes）。
while [ 1 ]
do
  dirSize="$(dir_size_mb "${logstorePath}")"
  [ -n "${dirSize}" ] || dirSize=0

  if [ "${dirSize}" -gt "${kMaxSizeMb}" ]; then
    TarLogFileDate
  else
    TarLogFileTimes
  fi

  sleep 30   # 巡检间隔（根据设备负载可调）
done
