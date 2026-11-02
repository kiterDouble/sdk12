#!/bin/bash
#
# protobuf v3.20.3 构建脚本
# 1. 先编译本地(x86_64) protoc, 供主机端 .proto 代码生成
# 2. 再交叉编译 ARMv7 (ag35cnt) 版本, 产出动态库(.so) + 头文件
#
# v3.20.3 是最后一个不依赖 abseil-cpp 的版本, 产物干净:
#   只有 libprotobuf.so / libprotobuf-lite.so / libprotoc.so / protoc / google头文件
#
# 用法: 在容器内执行
#   bash /host/c3/tonly/protobuf-3.20.3/build_protobuf.sh
#
# 产物:
#   <PROTOBUF_DIR>/build-native/protoc    # 本地 protoc (x86_64, 可直接运行)
#   <PROTOBUF_DIR>/build/out/
#     ├── protoc                          # ARM 版 protoc
#     ├── include/google/                 # 头文件 (只有 .h 和 .inc)
#     └── lib/
#         ├── libprotobuf.so              -> libprotobuf.so.3.20.3
#         ├── libprotobuf-lite.so         -> libprotobuf-lite.so.3.20.3
#         └── libprotoc.so                -> libprotoc.so.3.20.3
#

# ============================================================
# 配置
# ============================================================
PROTOBUF_VER="3.20.3"
WORK_DIR="/host/c3/tonly"
PROTOBUF_DIR="${WORK_DIR}/protobuf-${PROTOBUF_VER}"
PROTOBUF_TARBALL="${WORK_DIR}/protobuf-${PROTOBUF_VER}.tar.gz"
BUILD_DIR="${PROTOBUF_DIR}/build"
OUT_DIR="${BUILD_DIR}/out"
NATIVE_BUILD_DIR="${PROTOBUF_DIR}/build-native"

TOOLCHAIN_PREFIX="/opt/ql_crosstools/ql-ag35-1806e-gcc-13.3.0-glibc-v1-toolchain"
SYSROOT="/opt/ql_crosstools/ql-ol-extsdk-quectel_ag35-xxt_r01a05_sdk/ql-sysroots"
TOOLCHAIN_FILE="${PROTOBUF_DIR}/arm-toolchain.cmake"

# proto 项目路径 (用于自动拷贝本地 protoc)
PROTO_CANDIDATES=(
    "${WORK_DIR}/tonly-all-ag35cnt/app/app_terminal_manager/terminal_management/proto"
    "/app_work/app/app_terminal_manager/terminal_management/proto"
)
PROTO_DIR=""
for _d in "${PROTO_CANDIDATES[@]}"; do
    if [ -d "${_d}" ]; then PROTO_DIR="${_d}"; break; fi
done

set -e

echo "============================================================"
echo " protobuf ${PROTOBUF_VER} 构建"
echo " 1) 本地 protoc (x86_64)"
echo " 2) ARMv7 交叉编译"
echo " 输出目录: ${OUT_DIR}"
echo "============================================================"

# ============================================================
# Step 0: 编译本地 protoc (x86_64) — 供 .proto 代码生成
# ============================================================
echo ""
echo "[0/5] 编译本地 protoc (x86_64) ..."

if [ -x "${NATIVE_BUILD_DIR}/protoc" ] && file "${NATIVE_BUILD_DIR}/protoc" | grep -q 'x86-64'; then
    echo "  -> ${NATIVE_BUILD_DIR}/protoc 已存在 (x86_64)，跳过"
else
    rm -rf "${NATIVE_BUILD_DIR}"
    mkdir -p "${NATIVE_BUILD_DIR}"
    cd "${NATIVE_BUILD_DIR}"

    echo "  -> cmake 配置 (本地) ..."

    # 清除可能残留的 ARM sysroot 环境变量
    unset CFLAGS CXXFLAGS LDFLAGS

    cmake "${PROTOBUF_DIR}/cmake" \
        -DCMAKE_C_COMPILER=gcc \
        -DCMAKE_CXX_COMPILER=g++ \
        -DCMAKE_SYSROOT="/" \
        -DCMAKE_C_FLAGS="" \
        -DCMAKE_CXX_FLAGS="" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_STANDARD=14 \
        -Dprotobuf_BUILD_SHARED_LIBS=OFF \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_WITH_ZLIB=OFF \
        2>&1 | tail -3

    echo "  -> 编译 protoc (make -j4 protoc) ..."
    make -j4 protoc 2>&1 | tail -5

    if [ ! -x "protoc" ]; then
        echo "  ❌ 本地 protoc 编译失败"
        exit 1
    fi
    echo "  -> 本地 protoc 就绪"
fi

# 自动拷贝到 proto 目录
if [ -n "${PROTO_DIR}" ]; then
    cp -f "${NATIVE_BUILD_DIR}/protoc" "${PROTO_DIR}/protoc"
    echo "  -> 已拷贝到 ${PROTO_DIR}/protoc"
fi

# ============================================================
# Step 1: 创建 toolchain 文件
# ============================================================
echo ""
echo "[1/5] 生成 toolchain 文件..."

cat > "${TOOLCHAIN_FILE}" << 'TOOLCHAIN_EOF'
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7)

set(TOOLCHAIN_PREFIX /opt/ql_crosstools/ql-ag35-1806e-gcc-13.3.0-glibc-v1-toolchain)
set(SYSROOT /opt/ql_crosstools/ql-ol-extsdk-quectel_ag35-xxt_r01a05_sdk/ql-sysroots)

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}/bin/arm-openwrt-linux-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}/bin/arm-openwrt-linux-g++)

set(CMAKE_C_FLAGS   "-march=armv7-a -marm -mfpu=neon -mfloat-abi=hard" CACHE STRING "")
set(CMAKE_CXX_FLAGS "-march=armv7-a -marm -mfpu=neon -mfloat-abi=hard" CACHE STRING "")

set(CMAKE_FIND_ROOT_PATH ${SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
TOOLCHAIN_EOF

echo "  -> ${TOOLCHAIN_FILE}"

# ============================================================
# Step 2: cmake 配置 (ARM 交叉编译)
# ============================================================
echo ""
echo "[2/5] cmake 配置 (ARM 交叉编译)..."

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake "${PROTOBUF_DIR}/cmake" \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=14 \
    -Dprotobuf_BUILD_SHARED_LIBS=ON \
    -Dprotobuf_BUILD_TESTS=OFF \
    -Dprotobuf_WITH_ZLIB=OFF

# ============================================================
# Step 3: 编译 (ARM)
# ============================================================
echo ""
echo "[3/5] 编译 (make -j4) ..."
make -j4

echo ""
echo "  -> 编译完成"

# ============================================================
# Step 4: 收集产物到 build/out/
# ============================================================
echo ""
echo "[4/5] 收集产物到 ${OUT_DIR} ..."

rm -rf "${OUT_DIR}"
mkdir -p "${OUT_DIR}/include" "${OUT_DIR}/lib"

# --- 头文件 ---
echo "  -> 复制头文件..."
cp -r "${PROTOBUF_DIR}/src/google" "${OUT_DIR}/include/"
echo "  -> 清理非头文件 (保留 .h 和 .inc)..."
find "${OUT_DIR}/include" -type f ! -name "*.h" ! -name "*.inc" -delete
find "${OUT_DIR}/include" -type d -empty -delete 2>/dev/null || true

# --- 动态库 ---
echo "  -> 复制 .so..."
cp -a libprotobuf.so* libprotobuf-lite.so* libprotoc.so* "${OUT_DIR}/lib/" 2>/dev/null || true

# --- protoc ---
echo "  -> 复制 protoc..."
cp -a protoc "${OUT_DIR}/" 2>/dev/null || true

# --- 汇总 ---
SO_COUNT=$(ls "${OUT_DIR}/lib/"*.so* 2>/dev/null | wc -l)
echo ""
echo "============================================================"
echo " 编译完成!"
echo "============================================================"
echo ""
echo "本地 protoc (x86_64): ${NATIVE_BUILD_DIR}/protoc"
echo ""
echo "ARM 产物: ${OUT_DIR}"
echo ""
echo "  ${OUT_DIR}/"
echo "  ├── protoc                    (编译器, ARM)"
echo "  ├── include/google/           (头文件)"
echo "  └── lib/                      (${SO_COUNT} 个 .so)"
echo "      ├── libprotobuf.so        -> libprotobuf.so.3.20.3"
echo "      ├── libprotobuf-lite.so   -> libprotobuf-lite.so.3.20.3"
echo "      └── libprotoc.so          -> libprotoc.so.3.20.3"
echo ""
echo "安装到项目:"
echo "  rm -f ${WORK_DIR}/tonly-all-ag35cnt/app-third-lib/lib/libprotobuf*"
echo "  rm -f ${WORK_DIR}/tonly-all-ag35cnt/app-third-lib/lib/libprotoc*"
echo "  cp -a ${OUT_DIR}/lib/* ${WORK_DIR}/tonly-all-ag35cnt/app-third-lib/lib/"
echo "  rm -rf ${WORK_DIR}/tonly-all-ag35cnt/app-third-lib/include/protobuf"
echo "  mkdir -p ${WORK_DIR}/tonly-all-ag35cnt/app-third-lib/include/protobuf"
echo "  cp -r ${OUT_DIR}/include/google ${WORK_DIR}/tonly-all-ag35cnt/app-third-lib/include/protobuf/"
echo ""
echo "============================================================"
