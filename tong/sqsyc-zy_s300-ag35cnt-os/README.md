# SQSYC-ZY_S300-AG35CNT-OS

# 安装docker

sudo apt-get install docker

# 下载docker镜像

公共用户名：read-images@sirun

公共密码：tbox@2024.sirun

公网环境使用：sirun-registry.cn-hangzhou.cr.aliyuncs.com

私网环境使用(在公司时)：sirun-registry-vpc.cn-hangzhou.cr.aliyuncs.com


docker login --username=read-images@sirun sirun-registry.cn-hangzhou.cr.aliyuncs.com

docker pull sirun-registry.cn-hangzhou.cr.aliyuncs.com/tbox/ag35xxt:r01a04
docker tag  sirun-registry.cn-hangzhou.cr.aliyuncs.com/tbox/ag35xxt:r01a04 ag35xxt:r01a04


# 运行 docker 镜像指令 编译SDK OS
docker run -it --rm -v `pwd`:/work/ -w /opt/ql_crosstools/ql-ol-extsdk-quectel_ag35-xxt_r01a04_sdk --name ag35_r01a04_shanqi_os ag35xxt:r01a04 bash /work/build_os_images.sh AG35-CNTCA

