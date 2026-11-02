## 编译zlog说明

1. source交叉编译环境变量，以AG35NAVD举例：

```shell
zz@LAPTOP-EASUPN43:~/c3/thrid-lib$ source /opt/ag35/ql-ol-sdk/ql-ol-crosstool/ql-ol-crosstool-env-init
QUECTEL_PROJECT_NAME      =AG35NAVD
QUECTEL_PROJECT_REV       =AG35NAVDR08A02T4G_OCPU_01.002.04
Done.
```

2. 运行./build.sh进行编译

```shell
zz@LAPTOP-EASUPN43:~/c3/thrid-lib/zlog-1.2.16$ ./build.sh
```

3. 在thrid-lib-out取得编译的头文件和库

```shell
zz@LAPTOP-EASUPN43:~/c3/thrid-lib$ cd thrid-lib-out/
zz@LAPTOP-EASUPN43:~/c3/thrid-lib/thrid-lib-out$ tree
.
├── bin
│   └── zlog-chk-conf
├── include
│   └── zlog.h
└── lib
    ├── libzlog.a
    ├── libzlog.so -> libzlog.so.1
    ├── libzlog.so.1 -> libzlog.so.1.2
    └── libzlog.so.1.2

3 directories, 6 files
```

### zlog官方文档

- 比较详细的文档可以查看doc/UsersGuide-CN.lyx

### 测试demo

- 测试demo是一个循环写入100W条log的测试程序，配置为单个日志文件最大20M，最多保存5个日志文件
  - 源码：test/test.c
  - 配置文件：test/test.conf
  - 可以参考test/makefile_arm进行编译测试
