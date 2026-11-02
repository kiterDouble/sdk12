# import socket
# import threading
# import binascii
# import re
# from datetime import datetime


# HOST = '192.168.1.102'
# PORT = 5052

# # -------------------------- 消息1：完整匹配（原配置不变）--------------------------
# TARGET_MSG1_HEX = '55 aa 05 02 03 00 13 00 00 00 00 00 03 00 00 07 02 00 00 07 11 00 00 07 72 00 0a 8d cc'
# RESPONSE_MSG1_HEX = '55 AA 05 02 04 00 2A 00 00 00 00 00 00 07 02 08 00 00 00 00 00 00 00 00 00 00 07 11 08 00 00 00 00 00 00 00 00 00 00 07 72 08 00 00 00 00 00 00 00 00 05 ED'
# TARGET_MSG2_HEX = '55 aa 05 02 03 00 13 00 00 00 00 00 03 00 00 07 02 00 00 07 11 00 00 07 72 00 00 22 96'
# RESPONSE_MSG2_HEX = '55 AA 05 02 04 00 2A 00 00 00 00 00 00 07 02 08 00 00 00 00 00 00 00 00 00 00 07 11 08 00 00 00 00 00 00 00 00 00 00 07 72 08 00 00 00 00 00 00 00 00 05 ED'
# # -------------------------- 消息3：前缀55aa05020100（保留）--------------------------
# TARGET_MSG3_PREFIX_HEX = '55 aa 05 02 01 00'  # 新消息的固定前缀
# MSG3_FIXED_PART_LEN = 5          # 前缀后的固定填充（00000，可根据日志微调）
# MSG3_LEN_FIELD_OFFSET = 6        # 长度字段偏移（前缀6字节后，即索引6）
# # RESPONSE_MSG3_HEX = '55 AA 05 02 02 00 03 00 00 00 45 0D'  # 消息3的响应（可按需修改）
# RESPONSE_MSG3_HEX = '55 AA 05 02 02 00 03 00 00 00 00 DB A0' 
                     
# # --- 转换为字节串（仅保留消息1和消息3，删除消息2相关）---
# TARGET_MSG1_BYTES = binascii.unhexlify(TARGET_MSG1_HEX.replace(' ', ''))
# TARGET_MSG2_BYTES = binascii.unhexlify(TARGET_MSG2_HEX.replace(' ', ''))
# TARGET_MSG3_PREFIX_BYTES = binascii.unhexlify(TARGET_MSG3_PREFIX_HEX.replace(' ', ''))
# RESPONSE_MSG1_BYTES = binascii.unhexlify(RESPONSE_MSG1_HEX.replace(' ', ''))
# RESPONSE_MSG2_BYTES = binascii.unhexlify(RESPONSE_MSG2_HEX.replace(' ', ''))
# RESPONSE_MSG3_BYTES = binascii.unhexlify(RESPONSE_MSG3_HEX.replace(' ', ''))

# # --- 工具函数（无需修改）---
# def get_timestamp():
#     return datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]

# def format_hex(binary_data):
#     hex_str = binascii.hexlify(binary_data).decode('utf-8')
#     return re.sub('(..)', r'\1 ', hex_str).strip()


# def handle_client(conn, addr):
#     """处理单个客户端连接（仅支持消息1和消息3）"""
#     try:
#         buffer = bytearray()
#         while True:
#             # 1. 接收数据
#             data = conn.recv(4096)
#             if not data:
#                 break
            
#             # 2. 更新缓冲区
#             buffer.extend(data)
            
#             # 打印接收到的原始数据
#             timestamp = get_timestamp()
#             print(f"[{timestamp}] 收到数据: {format_hex(data)}")
            
#             # 3. 循环处理缓冲区中的消息
#             while True:
#                 msg1_index = buffer.find(TARGET_MSG1_BYTES)
#                 msg2_index = buffer.find(TARGET_MSG2_BYTES)
#                 msg3_index = buffer.find(TARGET_MSG3_PREFIX_BYTES)
                
#                 if msg1_index == -1 and msg3_index == -1 and msg2_index == -1:
#                     break
                
#                 # -------------------------- 处理消息1（完整匹配，优先）--------------------------
#                 if msg1_index >= 0:
#                     msg1_len = len(TARGET_MSG1_BYTES)
#                     msg1_end = msg1_index + msg1_len
                    
#                     if len(buffer) >= msg1_end:
#                         conn.sendall(RESPONSE_MSG1_BYTES)
#                         print(f"[{get_timestamp()}] 匹配到消息1，已发送响应。")
#                         del buffer[:msg1_end]
#                     else:
#                         break
#                     continue

#                 # -------------------------- 处理消息1（完整匹配，优先）--------------------------
#                 if msg2_index >= 0:
#                     msg2_len = len(TARGET_MSG2_BYTES)
#                     msg2_end = msg2_index + msg2_len
                    
#                     if len(buffer) >= msg2_end:
#                         conn.sendall(RESPONSE_MSG2_BYTES)
#                         print(f"[{get_timestamp()}] 匹配到消息2，已发送响应。")
#                         del buffer[:msg2_end]
#                     else:
#                         break
#                     continue
                
#                 # -------------------------- 处理消息3（前缀匹配，最终修正版）--------------------------
#                 if msg3_index >= 0:
#                     # 根据协议，最小长度 = 帧头(2) + 命令(3) + 长度字段(2) = 7字节
#                     min_frame_len = 7 
                    
#                     # 检查缓冲区是否足够提取长度字段
#                     if len(buffer) < msg3_index + min_frame_len:
#                         break
                    
#                     # --- 关键修正 ---
#                     # 1. 提取2字节的长度字段 (从偏移5和6的位置)
#                     len_bytes = buffer[msg3_index + 5 : msg3_index + 7]
#                     # 2. 按大端序 (Big-Endian) 转换为整数
#                     data_len = int.from_bytes(len_bytes, byteorder='big')
                    
#                     # 计算完整消息3的总长度
#                     # 总长度 = 帧头(2) + 命令(3) + 长度(2) + 数据(data_len) + 校验(2)
#                     msg3_total_len = 7 + data_len + 2
                    
#                     # 检查缓冲区是否足够包含完整消息3
#                     if len(buffer) < msg3_index + msg3_total_len:
#                         break
                    
#                     # 提取完整消息3并发送响应
#                     msg3_end = msg3_index + msg3_total_len
#                     conn.sendall(RESPONSE_MSG3_BYTES)
#                     print(f"[{get_timestamp()}] 匹配到消息3 (长度: {msg3_total_len}字节)，已发送响应。")
                    
#                     # 移除已处理数据
#                     del buffer[:msg3_end]
#                     continue
            
#             # 4. 缓冲区过大时清理
#             max_possible_len = max(len(TARGET_MSG1_BYTES), 7 + 65535 + 2)  # 消息1 + 消息3最大可能长度
#             if len(buffer) > max_possible_len * 2:
#                 print(f"[{get_timestamp()}] 缓冲区过大，清理中...")
#                 buffer = buffer[-max_possible_len:]
                
#     except Exception as e:
#         timestamp = get_timestamp()
#         print(f"[{timestamp}] 处理客户端 {addr} 出错: {str(e)}")
#     finally:
#         conn.close()




# def main():
#     try:
#         with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#             s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
#             s.bind((HOST, PORT))
#             s.listen()
            
#             # 循环接收客户端连接（多线程并发）
#             while True:
#                 conn, addr = s.accept()
#                 client_thread = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
#                 client_thread.start()
                
#     except Exception as e:
#         # 静默捕获服务启动异常（如需调试可取消注释打印）
#         # timestamp = get_timestamp()
#         # print(f"[{timestamp}] 服务启动失败: {str(e)}")
#         pass

# if __name__ == "__main__":
#     main()


import socket
import threading
import binascii
import re
import time  # 新增：用于持续发送的间隔等待
from datetime import datetime


HOST = '192.168.1.102'
PORT = 5052

# -------------------------- 消息配置（保留原结构，新增时间字节偏移计算）--------------------------
# 目标报文1：55 aa 05 02 03 00 13 00 00 00 00 00 03 00 00 07 02 00 00 07 11 00 00 07 72 00 0a 8d cc
# TARGET_MSG1_HEX = '55 aa 05 02 03 00 15 00 00 00 00 00 03 00 00 07 02 00 00 07 11 00 00 07 72 03 e8 d0 d0'
TARGET_MSG1_HEX = '55 aa 05 02 03 00 1d 00 00 00 00 00 05 00 00 07 02 00 00 07 11 00 00 07 72 00 00 07 6a 00 00 07 32 00 0a 18 5c'
# RESPONSE_MSG1_HEX = '55 AA 05 02 04 00 2A 00 00 00 00 00 00 07 02 08 00 00 00 00 00 00 00 00 00 00 07 11 08 00 00 00 00 00 00 00 00 00 00 07 72 08 00 00 00 00 00 00 00 00 05 ED'
RESPONSE_MSG1_HEX = '55 aa 05 02 04 00 44 00 00 00 00 00 00 07 02 08 00 00 00 00 00 00 00 00 00 00 07 11 08 00 00 00 00 00 00 00 00 00 00 07 72 08 00 00 00 00 00 00 00 00 00 00 07 6a 08 00 00 00 00 00 00 00 00 00 00 07 32 08 00 00 00 00 00 00 00 00 35 eb'

# 目标报文2：55 aa 05 02 03 00 13 00 00 00 00 00 03 00 00 07 02 00 00 07 11 00 00 07 72 00 00 22 96
# TARGET_MSG2_HEX = '55 aa 05 02 03 00 15 00 00 00 00 00 03 00 00 07 02 00 00 07 11 00 00 07 72 00 00 91 fe'
TARGET_MSG2_HEX = '55 aa 05 02 03 00 1d 00 00 00 00 00 05 00 00 07 02 00 00 07 11 00 00 07 72 00 00 07 6a 00 00 07 32 00 00 B7 06'
RESPONSE_MSG2_HEX = '55 AA 05 02 04 00 2A 00 00 00 00 00 00 07 02 08 00 00 00 00 00 00 00 00 00 00 07 11 08 00 00 00 00 00 00 00 00 00 00 07 72 08 00 00 00 00 00 00 00 00 05 ED'

# 消息3：保留原配置
TARGET_MSG3_PREFIX_HEX = '55 aa 05 02 01 00'
RESPONSE_MSG3_HEX = '55 AA 05 02 02 00 03 00 00 00 00 DB A0'

# --- 转换为字节串 + 计算时间间隔字节的固定偏移（核心新增）---
TARGET_MSG1_BYTES = binascii.unhexlify(TARGET_MSG1_HEX.replace(' ', ''))
TARGET_MSG2_BYTES = binascii.unhexlify(TARGET_MSG2_HEX.replace(' ', ''))
TARGET_MSG3_PREFIX_BYTES = binascii.unhexlify(TARGET_MSG3_PREFIX_HEX.replace(' ', ''))
RESPONSE_MSG1_BYTES = binascii.unhexlify(RESPONSE_MSG1_HEX.replace(' ', ''))
RESPONSE_MSG2_BYTES = binascii.unhexlify(RESPONSE_MSG2_HEX.replace(' ', ''))
RESPONSE_MSG3_BYTES = binascii.unhexlify(RESPONSE_MSG3_HEX.replace(' ', ''))

# 计算“07 72”在目标报文中的索引，后续+2字节即为时间间隔字节（固定偏移，一次计算）
# 07 72的字节值：0x07, 0x72 → 对应字节串b'\x07r'
TIME_MARKER = b'\x07r'
# 报文1中时间间隔字节的起始索引（MARKER索引 + 2）
MSG1_TIME_OFFSET = TARGET_MSG1_BYTES.find(TIME_MARKER) + 2
# 报文2中时间间隔字节的起始索引（与报文1结构一致，偏移相同）
MSG2_TIME_OFFSET = TARGET_MSG2_BYTES.find(TIME_MARKER) + 2


# --- 工具函数（保留原功能）---
def get_timestamp():
    return datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]

def format_hex(binary_data):
    hex_str = binascii.hexlify(binary_data).decode('utf-8')
    return re.sub('(..)', r'\1 ', hex_str).strip()


# --- 新增：持续发送响应的线程函数 ---
def continuous_send(conn, interval_ms, stop_flag):
    """
    按指定间隔持续发送响应
    :param conn: 客户端连接
    :param interval_ms: 发送间隔（ms）
    :param stop_flag: 停止标志（threading.Event，set()时停止）
    """
    interval_sec = interval_ms / 1000  # 转换为秒（time.sleep参数单位）
    while not stop_flag.is_set():
        try:
            conn.sendall(RESPONSE_MSG1_BYTES)  # 报文1和2的响应相同，复用一个
            print(f"[{get_timestamp()}] 持续发送响应（间隔{interval_ms}ms）")
            time.sleep(interval_sec)
        except Exception as e:
            print(f"[{get_timestamp()}] 持续发送出错: {str(e)}")
            break


def handle_client(conn, addr):
    """处理单个客户端连接（新增持续发送逻辑）"""
    # 初始化持续发送控制：停止标志（默认停止）、线程对象
    stop_continuous_send = threading.Event()
    continuous_thread = None
    try:
        buffer = bytearray()
        while True:
            # 1. 接收数据
            data = conn.recv(4096)
            if not data:
                print(f"[{get_timestamp()}] 客户端 {addr} 断开连接")
                break
            
            # 2. 更新缓冲区并打印接收数据
            buffer.extend(data)
            print(f"[{get_timestamp()}] 收到数据: {format_hex(data)}")
            
            # 3. 循环处理缓冲区中的消息
            while True:
                msg1_index = buffer.find(TARGET_MSG1_BYTES)
                msg2_index = buffer.find(TARGET_MSG2_BYTES)
                msg3_index = buffer.find(TARGET_MSG3_PREFIX_BYTES)
                
                # 无匹配消息时退出循环，等待后续数据
                if msg1_index == -1 and msg2_index == -1 and msg3_index == -1:
                    break

                # -------------------------- 处理消息1（完整匹配 + 时间间隔识别）--------------------------
                if msg1_index >= 0:
                    msg1_len = len(TARGET_MSG1_BYTES)
                    msg1_end = msg1_index + msg1_len
                    
                    # 确保缓冲区包含完整报文
                    if len(buffer) < msg1_end:
                        break
                    
                    # 提取完整报文
                    full_msg1 = buffer[msg1_index:msg1_end]
                    # 提取时间间隔字节（2字节），并转换为10进制（大端序）
                    time_bytes = full_msg1[MSG1_TIME_OFFSET : MSG1_TIME_OFFSET + 2]
                    interval_ms = 10 #int.from_bytes(time_bytes, byteorder='big')
                    print(f"[{get_timestamp()}] 识别消息1，时间间隔: {interval_ms}ms")
                    
                    # 第一步：必须先发送一次响应（无论是否持续）
                    conn.sendall(RESPONSE_MSG1_BYTES)
                    print(f"[{get_timestamp()}] 已发送消息1初始响应")
                    
                    # 第二步：根据间隔值控制持续发送
                    if interval_ms > 0:
                        # 非0间隔：停止现有线程（若存在），启动新线程
                        if continuous_thread and continuous_thread.is_alive():
                            stop_continuous_send.set()
                            continuous_thread.join()  # 等待旧线程结束
                        # 重置停止标志，启动新线程
                        stop_continuous_send.clear()
                        continuous_thread = threading.Thread(
                            target=continuous_send,
                            args=(conn, interval_ms, stop_continuous_send),
                            daemon=True  # 随主线程退出
                        )
                        continuous_thread.start()
                        print(f"[{get_timestamp()}] 已启动持续发送线程（间隔{interval_ms}ms）")
                    else:
                        # 0间隔：停止持续发送线程
                        if continuous_thread and continuous_thread.is_alive():
                            stop_continuous_send.set()
                            continuous_thread.join()
                            print(f"[{get_timestamp()}] 已停止持续发送线程")
                    
                    # 移除缓冲区中已处理的消息1
                    del buffer[:msg1_end]
                    continue

                # -------------------------- 处理消息2（与消息1逻辑一致）--------------------------
                if msg2_index >= 0:
                    msg2_len = len(TARGET_MSG2_BYTES)
                    msg2_end = msg2_index + msg2_len
                    
                    if len(buffer) < msg2_end:
                        break
                    
                    # 提取完整报文和时间间隔
                    full_msg2 = buffer[msg2_index:msg2_end]
                    time_bytes = full_msg2[MSG2_TIME_OFFSET : MSG2_TIME_OFFSET + 2]
                    interval_ms = int.from_bytes(time_bytes, byteorder='big')
                    print(f"[{get_timestamp()}] 识别消息2，时间间隔: {interval_ms}ms")
                    
                    # 第一步：先发送一次响应
                    conn.sendall(RESPONSE_MSG2_BYTES)
                    print(f"[{get_timestamp()}] 已发送消息2初始响应")
                    
                    # 第二步：控制持续发送（逻辑与消息1一致）
                    if interval_ms > 0:
                        if continuous_thread and continuous_thread.is_alive():
                            stop_continuous_send.set()
                            continuous_thread.join()
                        stop_continuous_send.clear()
                        continuous_thread = threading.Thread(
                            target=continuous_send,
                            args=(conn, interval_ms, stop_continuous_send),
                            daemon=True
                        )
                        continuous_thread.start()
                        print(f"[{get_timestamp()}] 已启动持续发送线程（间隔{interval_ms}ms）")
                    else:
                        if continuous_thread and continuous_thread.is_alive():
                            stop_continuous_send.set()
                            continuous_thread.join()
                            print(f"[{get_timestamp()}] 已停止持续发送线程")
                    
                    # 移除已处理的消息2
                    del buffer[:msg2_end]
                    continue
                
                # -------------------------- 处理消息3（保留原逻辑，无修改）--------------------------
                if msg3_index >= 0:
                    min_frame_len = 7  # 帧头(2)+命令(3)+长度字段(2)
                    if len(buffer) < msg3_index + min_frame_len:
                        break
                    
                    # 提取长度字段并计算总长度
                    len_bytes = buffer[msg3_index + 5 : msg3_index + 7]
                    data_len = int.from_bytes(len_bytes, byteorder='big')
                    msg3_total_len = 7 + data_len + 2  # 总长度=帧头+命令+长度+数据+校验
                    
                    if len(buffer) < msg3_index + msg3_total_len:
                        break
                    
                    # 发送响应并清理缓冲区
                    conn.sendall(RESPONSE_MSG3_BYTES)
                    print(f"[{get_timestamp()}] 匹配到消息3 (长度: {msg3_total_len}字节)，已发送响应。")
                    del buffer[:msg3_index + msg3_total_len]
                    continue
            
            # 4. 缓冲区过大时清理（保留原逻辑）
            max_possible_len = max(len(TARGET_MSG1_BYTES), 7 + 65535 + 2)
            if len(buffer) > max_possible_len * 2:
                print(f"[{get_timestamp()}] 缓冲区过大，清理中...")
                buffer = buffer[-max_possible_len:]
                
    except Exception as e:
        print(f"[{get_timestamp()}] 处理客户端 {addr} 出错: {str(e)}")
    finally:
        # 客户端断开时，确保持续发送线程停止
        if continuous_thread and continuous_thread.is_alive():
            stop_continuous_send.set()
            continuous_thread.join()
        conn.close()
        print(f"[{get_timestamp()}] 客户端 {addr} 连接已关闭")


def main():
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((HOST, PORT))
            s.listen()
            print(f"[{get_timestamp()}] 服务已启动，监听 {HOST}:{PORT}")
            
            # 循环接收客户端连接（多线程并发）
            while True:
                conn, addr = s.accept()
                print(f"[{get_timestamp()}] 新客户端连接: {addr}")
                client_thread = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
                client_thread.start()
                
    except Exception as e:
        print(f"[{get_timestamp()}] 服务启动失败: {str(e)}")
        raise  # 取消静默，方便调试（如需静默可注释此行）

if __name__ == "__main__":
    main()