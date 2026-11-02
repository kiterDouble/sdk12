[openssl-3.0.5]
source /opt/ql_crosstools/crosstool-env-init
./Configure linux-armv4 --prefix=/app_work/build --openssldir=/app_work/build/ssl   shared  no-zlib
make
make install
  
[paho.mqtt.c-1.3.14]
source /opt/ql_crosstools/crosstool-env-init

[protobuf-3.20.3]
source /opt/ql_crosstools/crosstool-env-init
# 将 /app_work/app-third-lib/include/build_protobuf.sh 放在protobuf-3.20.3库中调度编译
# 1. 先编译本地(x86_64) protoc, 供主机端 .proto 代码生成
# 2. 再交叉编译 ARMv7 (ag35cnt) 版本, 产出动态库(.so) + 头文件
# 将 protobuf-3.20.3/build-native/protoc 作为本地x86_64系统的编译命令拷贝到应用层 /app_work/app/app_terminal_manager/terminal_management/proto
# 3. APP层调度 /app_work/app/app_terminal_manager/terminal_management/proto/generate_proto.sh 编译原始proto文件

