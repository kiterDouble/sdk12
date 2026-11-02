## 编译IPC库说明

1. 克隆c3的server仓库与third-lib仓库到同一目录

```
zz@LAPTOP-EASUPN43:~/c3$ tree -L 1
.
├── server
└── thrid-lib

2 directories, 0 files
```

2. 在third-lib/目录下使用build_all.sh编译third-lib，编译出sripc库依赖的zlog库以及demo依赖的flatbuffer（flatcc）库

- 注意build_all.sh使用的是/opt/ql-ol-sdk/ql-ol-crosstool/ql-ol-crosstool-env-init环境文件

```shell
zz@LAPTOP-EASUPN43:~/c3/thrid-lib$ ./build_all.sh
...
...

zz@LAPTOP-EASUPN43:~/c3/thrid-lib$ tree thrid-lib-out/lib/ 
thrid-lib-out/lib/
├── libflatccrt.so
├── libzlog.so -> libzlog.so.1
├── libzlog.so.1 -> libzlog.so.1.2
└── libzlog.so.1.2

0 directories, 4 files
```

3. 在server/目录下使用build_all.sh编译，得到pubdemo、subdemo、libsr-ipc.so

```shell
zz@LAPTOP-EASUPN43:~/c3/server$ ./build_all.sh 
...
...

zz@LAPTOP-EASUPN43:~/c3/server$ tree server-out/
server-out/
├── demo
│   ├── pubdemo
│   ├── pubdemo.conf
│   ├── rpcClientDemo
│   ├── rpcClientDemo.conf
│   ├── rpcServerDemo
│   ├── rpcServerDemo.conf
│   ├── subdemo
│   └── subdemo.conf
├── include
│   ├── common_inc
│   │   ├── err_code.h
│   │   └── sr_log.h
│   ├── queue_msg.h
│   └── sr_ipc.h
└── lib
    └── libsr-ipc.so

4 directories, 13 files
```

4. `测试ipc`：将libsr-ipc.so、libflatccrt.so、libzlog.so拷贝到目标lib路径，将pubdemo（发送方）、subdemo（接收方）以及他们的日志配置文件pubdemo.conf、subdemo.conf拷贝到目标运行路径，运行即可开始测试

5. `测试rpc`：将libsr-ipc.so、libflatccrt.so、libzlog.so拷贝到目标lib路径，将rpcClientDemo（客户端）、rpcServerDemo（服务端）以及他们的日志配置文件rpcClientDemo.conf、rpcServerDemo.conf拷贝到目标运行路径，运行即可开始测试

6. 如果需要添加fbs结构进行测试，可以在demo/fbs下增加一个.fbs文件或者在已有的.fbs文件下新增结构，然后运行generate.sh生成flatbuffer C头文件（需要先在编译环境安装flatcc工具）

### sripc依赖

- glib-2.0
- zlog
- rt
- pthread
- flatccrt
