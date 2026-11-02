# otamodem1 测试指南

## 功能测试

### 1. 编译测试

```bash
cd /home/caojingye/work/jiushi/kiter/mpu/app/build
cmake ..
make modem_a
```

### 2. 运行测试

```bash
# 启动modem_a服务
./modem_a
```

预期输出：
```
modem_a start
modem_a start 2 
modem_a starts running..... 
Device 1 (Server) listening on port 8080...
Waiting for device connection.....
```

### 3. 发送测试数据

使用Python脚本测试HTTP监听功能：

```python
#!/usr/bin/env python3
import socket
import struct
import time

def test_send_file_size_and_data():
    """测试发送文件大小和数据"""
    
    # 连接到服务器
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect(('127.0.0.1', 8080))
    
    try:
        # 准备测试数据
        test_data = b"This is test upgrade data. " * 1000  # 约27KB测试数据
        file_size_bytes = len(test_data)
        file_size_mb = file_size_bytes / (1024.0 * 1024.0)
        
        print(f"Sending file size: {file_size_mb:.6f} MB ({file_size_bytes} bytes)")
        
        # 方式1：旧协议 - 只发送MB（8字节double）
        size_mb_packed = struct.pack('d', file_size_mb)
        client_socket.sendall(size_mb_packed)
        
        # 等待一下让服务器处理
        time.sleep(0.1)
        
        # 发送测试数据
        print("Sending test data...")
        client_socket.sendall(test_data)
        
        # 发送结束标记
        time.sleep(0.1)
        client_socket.sendall(b"end")
        
        # 接收升级结果
        response = client_socket.recv(1024)
        print(f"Server response: {response.decode()}")
        
    finally:
        client_socket.close()
        print("Test completed")

if __name__ == '__main__':
    test_send_file_size_and_data()
```

### 4. 验证文件接收

```bash
# 检查升级文件是否创建
ls -lh /oemdata/upgrade/update.zip

# 检查文件大小是否正确
stat /oemdata/upgrade/update.zip

# 查看升级状态
cat /oemdata/parameters/SlaveModemUpgradeStatus

# 查看升级进度
cat /oemdata/parameters/SlaveModemUpProgress
```

### 5. 日志检查

```bash
# 查看modem_a的日志
tail -f /usrdata/log/sr.log | grep modem_a
```

## 常见问题

### Q1: 端口被占用
**问题**: bind failed  
**解决**: 检查是否有其他进程占用8080端口
```bash
netstat -tlnp | grep 8080
kill -9 <PID>
```

### Q2: 权限不足
**问题**: 无法创建文件或目录  
**解决**: 确保有足够的权限
```bash
chmod 777 /oemdata/upgrade
chmod 777 /oemdata/parameters
```

### Q3: 磁盘空间不足
**问题**: Spatial Processing Function fail  
**解决**: 清理磁盘空间或调整日志清理策略

## 性能测试

### 大文件传输测试

```python
#!/usr/bin/env python3
import socket
import struct
import time
import os

def test_large_file_transfer():
    """测试大文件传输（10MB）"""
    
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect(('127.0.0.1', 8080))
    
    try:
        # 生成10MB测试数据
        file_size_mb = 10.0
        file_size_bytes = int(file_size_mb * 1024 * 1024)
        test_data = os.urandom(file_size_bytes)  # 随机数据
        
        print(f"Sending {file_size_mb} MB file...")
        
        # 发送文件大小（新协议格式）
        size_mb_packed = struct.pack('d', file_size_mb)
        type_byte = struct.pack('B', 0x01)  # type = 1
        size_bytes_packed = struct.pack('d', float(file_size_bytes))
        
        client_socket.sendall(size_mb_packed + type_byte + size_bytes_packed)
        time.sleep(0.1)
        
        # 分块发送数据（避免一次性发送太多）
        chunk_size = 20480  # 20KB
        sent = 0
        start_time = time.time()
        
        while sent < file_size_bytes:
            end = min(sent + chunk_size, file_size_bytes)
            client_socket.sendall(test_data[sent:end])
            sent = end
            
            if sent % (1024 * 1024) == 0:  # 每1MB打印一次
                print(f"Sent: {sent / (1024*1024):.1f} MB")
        
        elapsed = time.time() - start_time
        print(f"Data sent in {elapsed:.2f} seconds")
        print(f"Transfer rate: {file_size_bytes / elapsed / 1024 / 1024:.2f} MB/s")
        
        # 发送结束标记
        time.sleep(0.1)
        client_socket.sendall(b"end")
        
        # 接收结果
        response = client_socket.recv(1024)
        print(f"Server response: {response.decode()}")
        
    finally:
        client_socket.close()

if __name__ == '__main__':
    test_large_file_transfer()
```

## 调试技巧

### 1. 启用详细日志
在代码中添加更多LOG_Debug语句来跟踪执行流程

### 2. 使用tcpdump抓包
```bash
tcpdump -i lo port 8080 -w capture.pcap
```

### 3. 检查socket连接状态
```bash
netstat -an | grep 8080
```

### 4. 监控文件写入
```bash
inotifywait -m /oemdata/upgrade/
```
