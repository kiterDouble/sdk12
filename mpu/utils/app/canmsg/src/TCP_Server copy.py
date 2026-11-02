import socket
import threading
import binascii
import re
from datetime import datetime

# --- 配置区 ---
HOST = '192.168.1.102'
PORT = 5052

# --- 目标消息和对应响应 ---

# 目标消息1: 完整匹配
TARGET_MSG1_HEX = '55 AA 05 02 03 00 13 00 00 00 00 00 03 00 00 07 02 00 00 07 11 00 00 07 72 5B 08'
# 响应消息1
RESPONSE_MSG1_HEX = '55 AA 05 02 04 00 2A 00 00 00 00 00 00 07 02 08 00 00 00 00 00 00 00 00 00 00 07 11 08 00 00 00 00 00 00 00 00 00 00 07 72 08 00 00 00 00 00 00 00 00 05'

# 目标消息2: 前缀匹配 (以这个开头)
TARGET_MSG2_PREFIX_HEX = '55 a0 50 20 10 00'
# 假设目标消息2的总长度也是27字节
TARGET_MSG2_LENGTH = 27 
# 响应消息2
RESPONSE_MSG2_HEX = '55 aa 05 02 02 00 03 00 00 00 45 0D'
# --- 配置区结束 ---

# 转换为字节串 (bytes)
TARGET_MSG1_BYTES = binascii.unhexlify(TARGET_MSG1_HEX.replace(' ', ''))
TARGET_MSG2_PREFIX_BYTES = binascii.unhexlify(TARGET_MSG2_PREFIX_HEX.replace(' ', ''))
RESPONSE_MSG1_BYTES = binascii.unhexlify(RESPONSE_MSG1_HEX.replace(' ', ''))
RESPONSE_MSG2_BYTES = binascii.unhexlify(RESPONSE_MSG2_HEX.replace(' ', ''))

def get_timestamp():
    """获取当前时间的格式化字符串（包含毫秒）"""
    return datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]

def format_hex(binary_data):
    """将字节数据格式化为带空格的十六进制字符串"""
    hex_str = binascii.hexlify(binary_data).decode('utf-8')
    return re.sub('(..)', r'\1 ', hex_str).strip()

def handle_client(conn, addr):
    """处理单个客户端连接"""
    timestamp = get_timestamp()
    print(f"[{timestamp}] 新连接: {addr}")
    try:
        buffer = bytearray()
        while True:
            data = conn.recv(1024)
            if not data:
                timestamp = get_timestamp()
                print(f"[{timestamp}] 客户端断开: {addr}")
                break
                
            timestamp = get_timestamp()
            print(f"[{timestamp}] 收到数据: {format_hex(data)}")

            buffer.extend(data)
            
            # --- 修改点: 循环处理缓冲区中的所有匹配消息 ---
            while True:
                msg1_index = buffer.find(TARGET_MSG1_BYTES)
                msg2_prefix_index = buffer.find(TARGET_MSG2_PREFIX_BYTES)

                # 找到最早出现的匹配
                min_index = min(msg1_index, msg2_prefix_index)

                if min_index == -1:
                    # 缓冲区中没有任何匹配的消息，退出内层循环，继续接收数据
                    break

                if msg1_index != -1 and (msg1_index <= msg2_prefix_index or msg2_prefix_index == -1):
                    # 情况1: 找到完整匹配的消息1，发送响应1
                    received_msg = buffer[msg1_index:msg1_index+len(TARGET_MSG1_BYTES)]
                    timestamp = get_timestamp()
                    print(f"[{timestamp}] 找到目标消息1: {format_hex(received_msg)}")
                    
                    conn.sendall(RESPONSE_MSG1_BYTES)
                    timestamp = get_timestamp()
                    print(f"[{timestamp}] 已发送响应1: {format_hex(RESPONSE_MSG1_BYTES)}")
                    
                    # 从缓冲区中移除已处理的部分
                    del buffer[:msg1_index+len(TARGET_MSG1_BYTES)]

                else: # msg2_prefix_index != -1
                    # 情况2: 找到前缀匹配的消息2，发送响应2
                    # 检查缓冲区是否有足够的数据来构成完整的消息
                    if len(buffer) >= msg2_prefix_index + TARGET_MSG2_LENGTH:
                        received_msg = buffer[msg2_prefix_index:msg2_prefix_index+TARGET_MSG2_LENGTH]
                        timestamp = get_timestamp()
                        print(f"[{timestamp}] 找到目标消息2 (前缀匹配): {format_hex(received_msg)}")
                        
                        # --- 修改点: 发送新的响应2 ---
                        conn.sendall(RESPONSE_MSG2_BYTES)
                        timestamp = get_timestamp()
                        print(f"[{timestamp}] 已发送响应2: {format_hex(RESPONSE_MSG2_BYTES)}")
                        
                        # 从缓冲区中移除已处理的部分
                        del buffer[:msg2_prefix_index+TARGET_MSG2_LENGTH]
                    else:
                        # 数据不完整，等待下一次recv
                        break

            # 如果缓冲区过大，清理一下，防止内存溢出
            if len(buffer) > max(len(TARGET_MSG1_BYTES), TARGET_MSG2_LENGTH) * 2:
                buffer = buffer[-max(len(TARGET_MSG1_BYTES), TARGET_MSG2_LENGTH):]
                    
    except Exception as e:
        timestamp = get_timestamp()
        print(f"[{timestamp}] 处理客户端 {addr} 时出错: {e}")
    finally:
        conn.close()
        timestamp = get_timestamp()
        print(f"[{timestamp}] 连接关闭: {addr}")

def main():
    """主函数，启动TCP服务"""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((HOST, PORT))
            s.listen()
            timestamp = get_timestamp()
            print(f"[{timestamp}] TCP服务已启动，监听 {HOST}:{PORT}")
            
            while True:
                conn, addr = s.accept()
                client_thread = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
                client_thread.start()
                
    except Exception as e:
        timestamp = get_timestamp()
        print(f"[{timestamp}] 服务启动失败: {e}")

if __name__ == "__main__":
    main()