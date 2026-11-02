# OtaConfigHandle HTTP 转发设计方案

## 1. 背景与问题

- `ota_configuration` 与 `ota_config2` 是两个独立进程，同时运行
- 两者 MQTT 控制互斥：`ota_configuration` 在 `GetMqttControlEnable() == 1` 时工作；`ota_config2` 在 `GetMqttControlEnable() == 2` 时工作
- 两者均接收 0x502 并调用 `OtaConfigHandle()`，其中需要向本地 HTTP Server (`127.0.0.1:20005/setotafield`) 发起 POST
- **需求**：`ota_config2` 中的 `HttpPostClient` 调用必须委托给 `ota_configuration` 执行

## 2. 方案选型：UDP IPC 转发

采用 **UDP 127.0.0.1 IPC** 作为跨进程转发通道，原因：
1. `ota_configuration` 已有 UDP Server 运行在 `127.0.0.1:5010`
2. 无需引入新库或新依赖
3. 本地 loopback UDP 丢包率极低，通信可靠
4. 改动量最小，侵入性低

## 3. 通信协议

### 3.1 JSON 消息格式

**Request**（`ota_config2` -> `ota_configuration`）
```json
{
    "cmd": "http_post",
    "url": "http://127.0.0.1:20005/setotafield",
    "header": "Content-Type: application/json",
    "post_data": "{...}",
    "seqid": "xxx"
}
```

**Response**（`ota_configuration` -> `ota_config2`）
```json
{
    "cmd": "http_post_reply",
    "seqid": "xxx",
    "response": 0,
    "ret": 0
}
```

### 3.2 时序图

```
ota_config2 (MQTT 接收 0x502)
  -> OtaConfigHandle()
    -> UdpHttpForwardClient()
      -> socket(AF_INET, SOCK_DGRAM, 0)
      -> bind(127.0.0.1:0)   // 自动分配端口，用于接收响应
      -> sendto(127.0.0.1:5010, cmd="http_post"...)
      -> recvfrom() [SO_RCVTIMEO = 10s]
      <- Response(cmd="http_post_reply", response=X)
    -> 构造 MQTT 回复

ota_configuration (UdphandleServer 循环)
  <- recvfrom() 得到 JSON cmd="http_post"
  -> 解析 post_data、seqid
  -> HttpPostClient(url, post_data, ...)
  -> sendto(来源 clientAddr, cmd="http_post_reply", ...)
```

## 4. 代码变更清单

| 进程 | 操作 | 文件 | 说明 |
|------|------|------|------|
| ota_config2 | 新增 | `src/udp/udp_ipc_client.cpp` | UDP Client：发送 HTTP 请求 + 接收结果 |
| ota_config2 | 新增 | `inc/udp/udp_ipc_client.h` | 头文件，声明 `UdpHttpForwardClient` |
| ota_config2 | 修改 | `src/mqtt/mqtt_data_handler.cpp` | `OtaConfigHandle` 改用 UDP 转发 |
| ota_config2 | 修改 | `CMakeLists.txt` | 增加 `udp_ipc_client.cpp` |
| ota_configuration | 修改 | `src/udp/udp_server.cpp` | `UdphandleServer` 新增 `cmd="http_post"` 解析分支 |

## 5. 关键实现说明

### 5.1 ota_config2 UDP Client

```cpp
// 伪代码
int UdpHttpForwardClient(const char* post_data, const char* seqid, int* response) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    // bind 本地地址（端口自动分配），用于接收响应
    struct sockaddr_in local = {0};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr("127.0.0.1");
    local.sin_port = htons(0);
    bind(fd, (struct sockaddr*)&local, sizeof(local));

    // 构造 JSON 请求
    cJSON* req = cJSON_CreateObject();
    cJSON_AddStringToObject(req, "cmd", "http_post");
    cJSON_AddStringToObject(req, "url", "http://127.0.0.1:20005/setotafield");
    cJSON_AddStringToObject(req, "header", "Content-Type: application/json");
    cJSON_AddStringToObject(req, "post_data", post_data);
    cJSON_AddStringToObject(req, "seqid", seqid);
    char* json = cJSON_PrintUnformatted(req);

    // 发送到 ota_configuration UDP Server
    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    srv.sin_port = htons(5010);
    sendto(fd, json, strlen(json), 0, (struct sockaddr*)&srv, sizeof(srv));

    // 设置 10 秒接收超时
    struct timeval tv = {10, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // 阻塞等待响应
    char buf[1500];
    struct sockaddr_in from;
    socklen_t fromLen = sizeof(from);
    int n = recvfrom(fd, buf, sizeof(buf)-1, 0, (struct sockaddr*)&from, &fromLen);
    if (n > 0) {
        buf[n] = '\0';
        cJSON* resp = cJSON_Parse(buf);
        *response = cJSON_GetObjectItem(resp, "response")->valueint;
        cJSON_Delete(resp);
    }

    cJSON_Delete(req);
    free(json);
    close(fd);
    return 0;
}
```

### 5.2 ota_configuration UDP Server 扩展

在 `UdphandleServer` 的 `recvfrom` 后增加分支：

```cpp
if (bytesReceived > 0) {
    buffer[bytesReceived] = '\0';
    cJSON* root = cJSON_Parse(buffer);
    cJSON* cmd = cJSON_GetObjectItem(root, "cmd");

    if (cmd && strcmp(cmd->valuestring, "http_post") == 0) {
        // 解析参数
        const char* post_data = cJSON_GetObjectItem(root, "post_data")->valuestring;
        const char* seqid = cJSON_GetObjectItem(root, "seqid")->valuestring;

        // 执行 HTTP POST
        HTTPSClientParams params = {0};
        params.url = "http://127.0.0.1:20005/setotafield";
        params.header1 = "Content-Type: application/json";
        params.post_data = (char*)post_data; // 需要 mutable
        params.httpscallback = otahttpscallback;
        params.response = -1;
        int ret = HttpPostClient(&params);

        // 构造响应 JSON
        cJSON* reply = cJSON_CreateObject();
        cJSON_AddStringToObject(reply, "cmd", "http_post_reply");
        cJSON_AddStringToObject(reply, "seqid", seqid);
        cJSON_AddNumberToObject(reply, "response", params.response);
        cJSON_AddNumberToObject(reply, "ret", ret);
        char* reply_json = cJSON_PrintUnformatted(reply);

        // 通过 sendto 返回给 ota_config2（clientAddr 来自 recvfrom）
        sendto(serversock, reply_json, strlen(reply_json), 0,
               (struct sockaddr*)&clientAddr, sizeof(clientAddr));

        cJSON_Delete(reply);
        free(reply_json);
    } else {
        // 原有逻辑：publish MQTT
        ...
    }
    cJSON_Delete(root);
}
```

## 6. 边界与异常处理

| 场景 | 处理 |
|------|------|
| UDP 发送失败 | `UdpHttpForwardClient` 返回 -1，`OtaConfigHandle` 按失败处理 |
| UDP 响应超时 | 10s 未收到响应，`UdpHttpForwardClient` 返回 -1 |
| HTTP 失败 | `otahttpscallback` 返回 -1，通过 `response` 字段告知 `ota_config2` |
| JSON 解析错误 | 忽略非法数据，不回复 |
| 端口冲突 | `ota_config2` 绑定 `127.0.0.1:0` 自动分配端口，无冲突 |

## 7. 替代方案

- **Unix Domain Socket (AF_UNIX)**：更可靠，双向流式通信，但需要新增完整 socket server/client 代码
- **TCP Socket**：过重，本项目无 TCP IPC 基础设施
- **D-Bus / Signal**：依赖更多，与现有架构不符

## 8. 结论

采用 **UDP IPC JSON 转发** 方案，改动量最小，复用现有 UDP 基础设施，满足功能需求。建议按第 5 节伪代码实现。
