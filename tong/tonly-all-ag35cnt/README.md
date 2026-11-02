# TONLY-ALL-AG35CNT


# 安装docker

sudo apt-get install docker

# 下载docker镜像

公共用户名：read-images@sirun

公共密码：tbox@2024.sirun

公网环境使用：sirun-registry.cn-hangzhou.cr.aliyuncs.com

私网环境使用(在公司时)：sirun-registry-vpc.cn-hangzhou.cr.aliyuncs.com


docker login --username=read-images@sirun sirun-registry.cn-hangzhou.cr.aliyuncs.com

docker pull sirun-registry.cn-hangzhou.cr.aliyuncs.com/tbox/ag35xxt:r01a05

docker tag  sirun-registry.cn-hangzhou.cr.aliyuncs.com/tbox/ag35xxt:r01a05 ag35xxt:r01a05

# 项目代码仓库

# mpu os
https://code.sirun.net/TBox/SQSYC/zy_s300/sqsyc-zy_s300-ag35cnt-os.git

# mpu server
https://code.sirun.net/TBox/SQSYC/zy_s300/sqsyc-c3server.git

# mpu oemapp 
https://code.sirun.net/TBox/tonly/all/tonly-all-ag35cnt.git

# mcu
https://code.sirun.net/TBox/tonly/all/tonly-all-z20k148ms.git


# 运行 docker 镜像指令 编译 Server 和 App

export TL_MCU_IMAGE_PATH=${HOME}/dev/Ag35/ShanQi/tonly-all-z20k148ms/image

export TL_MPU_IMAGE_PATH=${HOME}/dev/Ag35/ShanQi/sqsyc-zy_s300-ag35cnt-os/image

export TL_MPU_APP_PATH=${HOME}/dev/Ag35/ShanQi/tonly-all-ag35cnt

export TL_MPU_C3_SERVER_PATH=${HOME}/dev/Ag35/ShanQi/sqsyc-c3server

docker run -it --rm -v ${TL_MPU_APP_PATH}:/app_work/ -v ${TL_MPU_C3_SERVER_PATH}/C3_base_line:/app_work/C3_base_line -v ${TL_MPU_IMAGE_PATH}:/mpu_image/ -v ${TL_MCU_IMAGE_PATH}:/mcu_image/  -w /app_work  ag35xxt:r01a05 bash ./build.sh AG35-CNTCA_OS_20260901_1220  V001_20260611 235000001  001


# 单独编译

## 编译 c3_server

docker run -it --rm -v ${PWD}:/app_work/ -v ${TL_MPU_C3_SERVER_PATH}/C3_base_line:/app_work/C3_base_line -w /app_work ag35xxt:r01a05 bash -c "cd /app_work/C3_base_line/c3server/ && ./build_all.sh ag35cet && rm -rf /app_work/C3_base_line/C3BaseLineOut/server-out/* && cp -r -d /app_work/C3_base_line/c3server/server-out/ /app_work/C3_base_line/C3BaseLineOut/"


## 编译 third_lib

docker run -it --rm -v ${PWD}:/app_work/ -v ${TL_MPU_C3_SERVER_PATH}/C3_base_line:/app_work/C3_base_line -w /app_work ag35xxt:r01a05 bash -c "mkdir -p /app_work/oemapp/third_lib/ && cp -r -d /app_work/app-third-lib/lib/* /app_work/oemapp/third_lib/"


## 编译 fota_daemon

docker run -it --rm -v ${PWD}:/app_work/ -v ${TL_MPU_C3_SERVER_PATH}/C3_base_line:/app_work/C3_base_line -w /app_work ag35xxt:r01a05 bash -c "cd /app_work/fota && ./ota_daemon/build_all.sh ag35cet"


## 编译 app

docker run -it --rm -v ${PWD}:/app_work/ -v ${TL_MPU_C3_SERVER_PATH}/C3_base_line:/app_work/C3_base_line -w /app_work ag35xxt:r01a05 bash -c "cd /app_work/app && ./build_all.sh ag35cet"
