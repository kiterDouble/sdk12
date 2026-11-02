## 编译说明

1. source交叉编译环境变量，以AG35NAVD举例：

```shell
zz@LAPTOP-EASUPN43:~/c3/thrid-lib$ source /opt/ag35/ql-ol-sdk/ql-ol-crosstool/ql-ol-crosstool-env-init
QUECTEL_PROJECT_NAME      =AG35NAVD
QUECTEL_PROJECT_REV       =AG35NAVDR08A02T4G_OCPU_01.002.04
Done.
```

2. 运行./build.sh进行编译

```shell
zz@LAPTOP-EASUPN43:~/c3/thrid-lib/flatcc_0.6.1$ ./build.sh
```

3. 在thrid-lib-out取得编译的头文件和库

```shell
zz@LAPTOP-EASUPN43:~/c3/thrid-lib/thrid-lib-out$ tree -L 2
.
├── include
│   └── flatcc
└── lib
    └── libflatccrt.so
```
