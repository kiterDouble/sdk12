# server

## 编译说明
直接运行build_all.sh脚本,生成物默认安装在server-out目录中

```
bash build_all.sh ModemType GnssType WifiType HSMType
```
参数说明:

未传入参数时，采用第一个做为默认参数。

1. 参数说明

| 参数 | 说明 | 取值 |默认值|
| :---- | :------- | :------ | :------ |
|ModemType|NAD模组类型| 见表2 模组类型           |ag35ce|
|GnssType |GNSS定位模组类型,内置在NAD中时,填写模组类型 |ag35ce,ag35na,ec20ce,f9k|ag35ce|
|WifiType | WiFi模组类型|None,FC20|None|
|HSMType  |MPU侧加密芯片类型| None,FM1280,CIU98_B|None|

2. 模组类型

|          模组           |  ModemType取值   |                                                              |
| :---------------------: | :--------------: | :----------------------------------------------------------: |
|         ag35eut         |     ag35eut      |                                                              |
|         ag35cet         |     ag35cet      |                                                              |
|          A7805          |      A7805       |                                                              |
|         ag35ce          |      ag35ce      |                                                              |
|         ag35evd         |     ag35evd      |                                                              |
|          ag519          |      ag519       |                                                              |
| ag35cet（openssl3版本） | ag35cet-openssl3 | ![image-20241112110224723](README.assets/image-20241112110224723.png)<br />目前用法：将openssl3版本的thrid-lib-out替换掉编译出的thrid-lib-out，之后传入ModemType==ag35cet-openssl3进行编译，后续讨论库版本维护后修改此临时方式 |
|          AG591          |      AG591       |                                                              |
|         ZM8201          |      ZM8201      |                                                              |

