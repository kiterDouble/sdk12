import sys
import os
import struct
import zlib
import hashlib
from Crypto.Hash import CMAC
from Crypto.Cipher import AES
from intelhex import IntelHex

class SRecord:
    """S-Record 格式生成工具类"""
    @staticmethod
    def checksum(data: bytes) -> int:
        return (~sum(data) & 0xFF)

    @staticmethod
    def create_s3(address: int, data: bytes) -> str:
        """创建 S3 数据记录 (32位地址)"""
        length = len(data) + 4 + 1  # 数据长度 + 4字节地址 + 1字节校验和
        record_bytes = bytearray([length])
        record_bytes.extend(address.to_bytes(4, byteorder='big'))
        record_bytes.extend(data)
        record_bytes.append(SRecord.checksum(record_bytes))
        return f"S3{record_bytes.hex().upper()}"

    @staticmethod
    def create_s7(address: int) -> str:
        """创建 S7 结束记录 (32位地址)"""
        length = 4 + 1
        record_bytes = bytearray([length])
        record_bytes.extend(address.to_bytes(4, byteorder='big'))
        record_bytes.append(SRecord.checksum(record_bytes))
        return f"S7{record_bytes.hex().upper()}"

def sign_app_hex(hex_obj, output_filename="app_signed.hex"):
    """
    计算 app 的大小、SHA256、AES128CMAC，将其按特定结构写入 0x52000，
    并将从 0x52000 开始到文件末尾的所有空白部分强制填充为 0xFF。
    """
    print(f"\n[步骤 3.5] 计算 app 的信息并构造新 Header，注入 0x52000 并全局填充空隙...")
    
    app_start_addr = hex_obj.minaddr() # 原 app 的起始地址，正常是 0x54000
    app_max_addr = hex_obj.maxaddr()
    
    # 1. 提取 app 区域的数据计算 Size 和 Hash (tobinarray 返回 array.array 类型)
    hex_obj.padding = 0xFF
    app_bin_data = hex_obj.tobinarray(start=app_start_addr, end=app_max_addr)
    
    size = len(app_bin_data)
    print(f" -> 提取计算范围: 0x{app_start_addr:X} - 0x{app_max_addr:X}")
    print(f" -> 固件大小 (Size): {size} 字节 (0x{size:X})")
    
    # 2. 计算 SHA256 (32 字节)，保持不变 (hashlib 兼容 array 类型)
    sha256_hash = hashlib.sha256(app_bin_data).digest()
    print(f" -> 固件 SHA256: {sha256_hash.hex().upper()}")

    # 3. 计算 AES-128 CMAC (对 APP 原始数据进行加密生成 16 字节数据)
    aes_key = bytes.fromhex("0102030405060708090A0B0C0D0E0F10") # 16 bytes for AES-128

    cobj = CMAC.new(aes_key, ciphermod=AES)
    cobj.update(bytes(app_bin_data)) 
    cmac_tag = cobj.digest()  # 生成 16 字节

    print(f" -> 固件 AES128-CMAC (16字节): {cmac_tag.hex().upper()}")

    # 4. 组装 Header 数据 (4 + 4 + 4 + 32 + 16 + 4 = 64 字节)
    magic_header = 0x5a6a7a8a
    magic_tail = 0x8a7a6a5a
    
    # 使用 struct 打包前三个 4 字节数据 (小端模式 '<III')
    # 后面直接拼接 32 字节的 SHA256 和 16 字节的 AES 数据
    header_data = (struct.pack('<III', magic_header, app_start_addr, size) 
                    + sha256_hash 
                    + cmac_tag 
                    + struct.pack('<I', magic_tail))
        
    # 5. 创建一个临时的 Hex 对象，把 Header 和 App 拼在一起
    temp_hex = IntelHex()
    temp_hex.frombytes(app_bin_data, offset=app_start_addr) # 放入 app 本体
    temp_hex.puts(0x52000, header_data)                     # 放入 0x52000 新组装的 Header
    
    # 6. 将 temp_hex 作为一个整体导出，范围从 0x52000 到 app 末尾
    # 这一步会自动把 0x52000 Header 之后到 0x54000 的断层强制塞满 0xFF
    temp_hex.padding = 0xFF
    full_bin_data = temp_hex.tobinarray(start=0x52000, end=app_max_addr)
    
    # 7. 把彻底填满的连续二进制数据写回最终对象
    signed_hex = IntelHex()
    signed_hex.frombytes(full_bin_data, offset=0x52000)
    
    # 输出 app_signed.hex
    signed_hex.write_hex_file(output_filename)
    print(f" -> {output_filename} 生成成功，0x52000 至 0x{app_max_addr:X} 的空隙均已填充 0xFF。")
    print(f" -> Header 结构已写入: [Magic(4)][Addr(4)][Size(4)][SHA256(32)][AES(16)][MagicTail(4)]")
    
    return signed_hex

def sign_boot_hex(hex_obj, output_filename="boot_signed.hex"):
    """
    计算 boot 的大小、SHA256、AES128CMAC，将其按特定结构写入 0x3E000，
    并将从 boot 起始位置到 0x3E000 (含 Header) 之间的空白部分强制填充为 0xFF。
    """
    print(f"\n[操作] 计算 boot 的信息并构造新 Header，注入 0x3E000 并全局填充空隙...")
    
    boot_start_addr = hex_obj.minaddr() # 原 boot 的起始地址，通常是 0x0
    boot_max_addr = hex_obj.maxaddr()
    
    # 1. 提取 boot 区域的数据计算 Size 和 Hash
    hex_obj.padding = 0xFF
    boot_bin_data = hex_obj.tobinarray(start=boot_start_addr, end=boot_max_addr)
    
    size = len(boot_bin_data)
    print(f" -> 提取计算范围: 0x{boot_start_addr:X} - 0x{boot_max_addr:X}")
    print(f" -> 固件大小 (Size): {size} 字节 (0x{size:X})")
    
    # 2. 计算 SHA256 (32 字节)，保持不变
    sha256_hash = hashlib.sha256(boot_bin_data).digest()
    print(f" -> 固件 SHA256: {sha256_hash.hex().upper()}")

    # 3. 计算 AES-128 CMAC (对 BOOT 原始数据加密生成 16 字节数据)
    aes_key = bytes.fromhex("0102030405060708090A0B0C0D0E0F10") # 16 bytes for AES-128
    
    cobj = CMAC.new(aes_key, ciphermod=AES)
    # PyCryptodome 不接受 array.array，必须强转为 bytes
    cobj.update(bytes(boot_bin_data)) 
    cmac_tag = cobj.digest()   # 生成 16 字节

    print(f" -> 固件 AES128-CMAC (16字节): {cmac_tag.hex().upper()}")

    # 4. 组装 Header 数据 (4 + 4 + 4 + 32 + 16 + 4 = 64 字节)
    magic_header = 0x5a6a7a8a
    magic_tail = 0x8a7a6a5a
    
    header_data = (struct.pack('<III', magic_header, boot_start_addr, size) 
                    + sha256_hash 
                    + cmac_tag 
                    + struct.pack('<I', magic_tail))
        
    # 5. 创建一个临时的 Hex 对象，把 Header 和 Boot 拼在一起
    temp_hex = IntelHex()
    temp_hex.frombytes(boot_bin_data, offset=boot_start_addr) # 放入 boot 本体
    temp_hex.puts(0x3E000, header_data)                       # 放入 0x3E000 新组装的 Header
    
    # 6. 计算包含 Header 的最大地址，并将 temp_hex 作为一个整体导出
    # 这一步会自动把 boot 本体末尾到 0x3E000 之间的断层强制塞满 0xFF
    temp_hex.padding = 0xFF
    header_end_addr = 0x3E000 + len(header_data) - 1
    final_max_addr = max(boot_max_addr, header_end_addr)
    
    full_bin_data = temp_hex.tobinarray(start=boot_start_addr, end=final_max_addr)
    
    # 7. 把彻底填满的连续二进制数据写回最终对象
    signed_hex = IntelHex()
    signed_hex.frombytes(full_bin_data, offset=boot_start_addr)
    
    # 输出 boot_signed.hex
    signed_hex.write_hex_file(output_filename)
    print(f" -> {output_filename} 生成成功，0x{boot_start_addr:X} 至 0x{final_max_addr:X} 的空隙均已填充 0xFF。")
    print(f" -> Header 结构已写入: [Magic(4)][Addr(4)][Size(4)][SHA256(32)][AES(16)][MagicTail(4)]")
    
    return signed_hex

def hex_to_srec_step(hex_obj, srec_path="app.srec"):
    """
    将内存中的 Hex 对象转换为 SREC 格式
    """
    print(f"\n[步骤 3.6] 将 app_signed.hex 转换为 SREC 格式 ({srec_path})...")
    
    start_addr = hex_obj.minaddr()
    end_addr = hex_obj.maxaddr()

    # 关键校验：对应步骤 3.5，注入 Header 后的起始地址应为 0x52000
    expected_start = 0x52000
    if start_addr != expected_start:
        print(f"[!] 错误：Hex 起始地址不符合预期，无法转 SREC！")
        print(f"    预期地址: 0x{expected_start:08X}")
        print(f"    实际地址: 0x{start_addr:08X}")
        return False
    
    print(f" -> 起始地址校验通过: 0x{start_addr:08X}")
    print(f" -> 数据截止地址: 0x{end_addr:08X}")

    # 获取二进制数据（确保已做 padding）
    hex_obj.padding = 0xFF
    data = hex_obj.tobinarray(start=start_addr, end=end_addr)
    
    record_count = 0
    max_data_len = 32  # 每行 S3 记录携带 32 字节数据

    with open(srec_path, 'w', encoding='utf-8') as f:
        # 1. 写入 S0 标识
        f.write("S00600004844521B\n") 
        
        # 2. 写入 S3 数据记录
        for i in range(0, len(data), max_data_len):
            chunk = data[i : i + max_data_len]
            current_addr = start_addr + i
            f.write(SRecord.create_s3(current_addr, chunk) + '\n')
            record_count += 1
            
        # 3. 写入 S7 结束记录（入口点设为起始地址）
        f.write(SRecord.create_s7(start_addr) + '\n')

    print(f" -> 转换成功！共写入 {record_count} 条 S3 记录。")
    print(f" -> {srec_path} 生成成功。")
    return True

def merge_and_process_hex(hex1_path, hex2_path):
    print("=== 开始执行 Hex 处理脚本 ===")
    
    # 步骤 1: 输入两个hex文件
    print(f"[步骤 1] 加载文件: {hex1_path} 和 {hex2_path}")
    if not os.path.exists(hex1_path) or not os.path.exists(hex2_path):
        print("错误：找不到输入的 Hex 文件，请检查路径。")
        return
        
    hex1 = IntelHex(hex1_path)
    hex2 = IntelHex(hex2_path)

    # 步骤 2: 检查hex1的起始地址是0，结束地址小于0x40000
    print("[步骤 2] 检查 hex1 地址范围...")
    if hex1.minaddr() != 0x0:
        print(f"错误: hex1 的起始地址不是 0x0 (当前为 0x{hex1.minaddr():X})")
        return
    if hex1.maxaddr() >= 0x40000:
        print(f"错误: hex1 的结束地址超出了 0x40000 (当前为 0x{hex1.maxaddr():X})")
        return
    print(" -> hex1 地址检查通过。")

    # 步骤 2.5: 对 Boot (hex1) 进行签名并注入 Header
    boot_signed_hex = sign_boot_hex(hex1, "boot_signed.hex")

    # 步骤 3: 检查hex2的起始地址是0x54000，结束地址小于0x100000
    print("[步骤 3] 检查 hex2 地址范围...")
    if hex2.minaddr() != 0x54000:
        print(f"错误: hex2 的起始地址不是 0x54000 (当前为 0x{hex2.minaddr():X})")
        return
    if hex2.maxaddr() >= 0x100000:
        print(f"错误: hex2 的结束地址超出了 0x100000 (当前为 0x{hex2.maxaddr():X})")
        return
    print(" -> hex2 地址检查通过。")

    # 步骤 3.5: 调用新增的函数处理 app hex 并生成带有 Size 和 CRC 的 app_signed.hex
    app_signed_hex = sign_app_hex(hex2, "app_signed.hex")

    # 步骤 3.6: 将处理后的 app_signed_hex 转换为 app.srec
    if not hex_to_srec_step(app_signed_hex, "app.srec"):
        print("SREC 转换失败，终止后续流程。")
        return

    # 步骤 4: 将两个hex文件合并成一个hex文件，然后空白部分填充全FF，命名是bankA.hex
    print("[步骤 4] 合并 hex1 和 app_signed_hex 0xFF 并生成 bankA.hex ...")
    app_signed_hex.start_addr = None
    bankA_temp = IntelHex()
    bankA_temp.merge(boot_signed_hex)
    bankA_temp.merge(app_signed_hex)
    
    # 为了严格保证空白部分填充 0xFF 且总大小符合 0x0 ~ 0xFFFFF 范围（大小0x100000）
    # 我们将其导出为二进制数组并强制填充 0xFF，然后再转回 Hex
    bankA_bin = bankA_temp.tobinarray(start=0x0, end=0xFFFFF, pad=0xFF)
    
    bankA = IntelHex()
    bankA.frombytes(bankA_bin, offset=0x0)
    bankA.write_hex_file("bankA.hex")
    print(" -> bankA.hex 生成成功。")

    # 步骤 5: 将bankA.hex的全部内容复制到0x100000地址上生成bankB.hex
    print("[步骤 5] 复制 bankA 内容平移至 0x100000 并生成 bankB.hex ...")
    bankB = IntelHex()
    bankB.frombytes(bankA_bin, offset=0x100000)
    bankB.write_hex_file("bankB.hex")
    print(" -> bankB.hex 生成成功。")

    # 步骤 6: 检查bankB.hex的起始地址是0x100000，结束地址小于0x200000
    print("[步骤 6] 检查 bankB 地址范围...")
    if bankB.minaddr() != 0x100000:
        print(f"错误: bankB 的起始地址不是 0x100000 (当前为 0x{bankB.minaddr():X})")
        return
    # 因为大小是0x100000，所以最大地址是 0x100000 + 0x100000 - 1 = 0x1FFFFF < 0x200000
    if bankB.maxaddr() >= 0x200000:
        print(f"错误: bankB 的结束地址超出了 0x100000 (当前为 0x{bankB.maxaddr():X})")
        return
    print(" -> bankB 地址检查通过。")

    # 步骤 7: 将bankA和BankB合并成一个hex文件，命名是boot_app.hex
    print("[步骤 7] 合并 bankA 和 bankB ...")
    final_hex = IntelHex()
    final_hex.merge(bankA)
    # final_hex.merge(bankB)

    # 步骤 8: 在 0x40000 注入魔术头
    # 使用 struct.pack('<4I', ...) 打包成小端模式(Little-Endian)的字节序，这是绝大多数MCU的标准。
    print("[步骤 8] 在 0x40000 位置写入魔术头数据...")
    magic_header_40000 = struct.pack('<8I', 
                                  0x5A5A5A5A, # 头固定5a5a5a5a
                                  0x00000000, # 初始升级版本
                                  0x00000001, # 当前激活区 A 或 B
                                  0x00000000, # 升级标志 0标志无升级
                                  0x55AA5AA5, 
                                  0x00000000, 
                                  0x00000000, 
                                  0x5A5A5A5A) # 尾固定5a5a5a5a
    final_hex.puts(0x40000, magic_header_40000)

    # 步骤 9: 在 0x140000 注入魔术头
    # print("[步骤 9] 在 0x140000 位置写入魔术头数据...")
    # magic_header_140000 = struct.pack('<4I', 0x5a5a5a5a, 0x00000000, 0xBBBBBBBB, 0x5a5a5a5a)
    # final_hex.puts(0x140000, magic_header_140000)

    # 保存最终文件
    final_hex.write_hex_file("boot_app.hex")
    print(f" -> 最终文件 boot_app.hex 生成成功！(包含 0x40000 和 0x140000 处的魔术字)")
    print("=== 全部处理完成 ===")

if __name__ == "__main__":
    input_hex1 = "boot.hex" 
    input_hex2 = "app.hex"
    
    merge_and_process_hex(input_hex1, input_hex2)