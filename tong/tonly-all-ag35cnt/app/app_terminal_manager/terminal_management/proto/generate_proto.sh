#!/bin/bash
#
# protobuf .proto -> .pb.h/.pb.cc 生成脚本
#
# 用法:
#   bash generate_proto.sh
#
# 输入:  proto/asms-protocol/net/sirun/tboxs3/connect/*.proto
# 输出:  tspProto/net/sirun/tboxs3/connect/*.pb.h
#        tspProto/net/sirun/tboxs3/connect/*.pb.cc
#

set -e

# ============================================================
# 配置 (按需修改)
# ============================================================
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROTO_SRC_DIR="${SCRIPT_DIR}/asms-protocol"
PROTO_FILES_DIR="${PROTO_SRC_DIR}/net/sirun/tboxs3/connect"
TSPROTO_DIR="${SCRIPT_DIR}/../tspProto"

# 使用脚本旁边的 protoc
PROTOC="${PROTOC:-${SCRIPT_DIR}/protoc}"

OUT_DIR="${TSPROTO_DIR}/net/sirun/tboxs3/connect"

echo "============================================================"
echo " protobuf .proto 代码生成"
echo " 输入: ${PROTO_FILES_DIR}"
echo " 输出: ${OUT_DIR}"
echo " protoc: ${PROTOC}"
echo "============================================================"

# ============================================================
# Step 1: 检查
# ============================================================
echo ""
echo "[1/2] 检查环境..."

if [ ! -x "${PROTOC}" ]; then
    echo "❌ protoc 不存在或不可执行: ${PROTOC}"
    echo ""
    echo "请把 protoc 放到脚本同级目录, 或指定路径:"
    echo "  PROTOC=/path/to/protoc bash $0"
    exit 1
fi

PROTOC_VER=$("${PROTOC}" --version 2>&1 || true)
echo "  ✔ protoc: ${PROTOC_VER}"

if [ ! -d "${PROTO_FILES_DIR}" ]; then
    echo "❌ proto 目录不存在: ${PROTO_FILES_DIR}"
    exit 1
fi

PROTO_COUNT=$(ls "${PROTO_FILES_DIR}"/*.proto 2>/dev/null | wc -l)
echo "  ✔ proto 文件: ${PROTO_COUNT} 个"

# ============================================================
# Step 2: 编译 .proto 文件
# ============================================================
echo ""
echo "[2/2] 编译 .proto -> .pb.h/.pb.cc..."

rm -rf "${OUT_DIR}"
mkdir -p "${OUT_DIR}"

# 清理旧的平铺目录
rm -rf "${TSPROTO_DIR}/inc" "${TSPROTO_DIR}/src"

echo "  proto 文件列表:"
for proto_file in "${PROTO_FILES_DIR}"/*.proto; do
    echo "    - $(basename "${proto_file}")"
done

cd "${PROTO_SRC_DIR}"

"${PROTOC}" \
    --proto_path="${PROTO_SRC_DIR}" \
    --cpp_out="${TSPROTO_DIR}" \
    "${PROTO_FILES_DIR}"/*.proto

H_COUNT=$(find "${OUT_DIR}" -name "*.pb.h" 2>/dev/null | wc -l)
CC_COUNT=$(find "${OUT_DIR}" -name "*.pb.cc" 2>/dev/null | wc -l)

echo ""
echo "============================================================"
echo " 生成完成!"
echo "============================================================"
echo ""
echo "产物目录: ${OUT_DIR}/"
echo "  ├── *.pb.h  × ${H_COUNT}"
echo "  └── *.pb.cc × ${CC_COUNT}"
echo ""

for f in "${OUT_DIR}"/*.pb.h; do
    basename "$f"
done | sort | while read f; do echo "  ${f}"; done

echo ""
echo "CMakeLists.txt 已需要更新 (见下方说明)"
echo "============================================================"
