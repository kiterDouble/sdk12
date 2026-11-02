"""
MCU2_DCDC (汇川) UDS 刷写流程监控测试

参考文档:
  - app_tsp/doc/MCU升级流程图-UDS.txt
  - app_tsp/doc/MCU-汇川UDS刷新规范_20230714.docx
  - app_tsp/doc/TCU_UDS_flash_test.py (模板)
  - app_tsp/doc/系统升级.docx

ECU 配置:
  - MCU1_TM1: phy=0x18DAD2FA, res=0x18DAFAD2, func=0x18DBFFFA
  - 站号2, CAN3动力, ISO-15765 扩展帧 29-bit

刷写流程 (对接 ota_master_fota_ecu.cpp: FotaEcuExecMcu1):
  一、预编程
    1. 22 F193  读取ECU硬件版本 (物理)
    2. 22 F195  读取ECU软件版本 (物理)
    3. 10 03    扩展会话 (功能)
    4. 31 01 FF02 预编程条件 (物理)
    5. 85 02    禁用DTC (功能)
    6. 28 03 03 禁用非诊断通讯 (功能)
  二、主编程
    1. 10 02    主编程会话
    2. 27 03/04 MCU1安全访问(汇川算法)
    3. 2E F184  写入指纹
    4. 驱动下载 + 31 01 F1A0 完整性
    5. 循环 { 31 01 FF00 -> 34/36/37 -> 31 01 F1A0 }
    6. 31 01 FF01 程序依赖性
    7. 11 01    ECU硬件复位
  三、后编程
    1. 10 03    扩展会话 (功能)
    2. 28 00 01 恢复通讯 (功能)
    3. 85 01    使能DTC (功能)
    4. 10 01    默认会话 (功能)
    5. 14 FF FF FF 清除故障码
"""

import os
import time
import struct
import pytest
import allure
import threading
import random
from enum import Enum, auto
from typing import Optional, List, Dict, Callable, Any
from queue import Queue, Empty
from srtest.device.modelconfig import ModelConfig
from srtest.device.canbus import CanBus
from srtest.allure.allure_testcase import Priority
from srtest.allure import allure_testcase, allure_step
from can import Bus, Notifier, Message

# ==============================================================================
# CAN ID 配置 (OTA Master -> MCU1_TM1)
# ==============================================================================

MCU2_DCDC_PHYSICAL_ID = 0x18DAD7FB
MCU2_DCDC_FUNCTIONAL_ID = 0x18DBFFFB
MCU2_DCDC_RESPONSE_ID = 0x18DAFBD7

OTA_MASTER_REQUEST_ID = MCU2_DCDC_PHYSICAL_ID
TBOX_RESPONSE_ID = MCU2_DCDC_RESPONSE_ID
OTA_MASTER_FUNCTIONAL_ID = MCU2_DCDC_FUNCTIONAL_ID

# ==============================================================================
# 枚举定义
# ==============================================================================

class FlashPhase(Enum):
    """刷写阶段枚举"""
    PRE_PROGRAMMING = auto()
    PROGRAMMING = auto()
    POST_PROGRAMMING = auto()

class UDSService(Enum):
    """UDS 服务枚举"""
    DIAGNOSTIC_SESSION_CONTROL = 0x10
    ECU_RESET = 0x11
    SECURITY_ACCESS = 0x27
    COMMUNICATION_CONTROL = 0x28
    TESTER_PRESENT = 0x3E
    CONTROL_DTC_SETTING = 0x85
    READ_DATA_BY_IDENTIFIER = 0x22
    WRITE_DATA_BY_IDENTIFIER = 0x2E
    ROUTINE_CONTROL = 0x31
    REQUEST_DOWNLOAD = 0x34
    REQUEST_UPLOAD = 0x35
    TRANSFER_DATA = 0x36
    REQUEST_TRANSFER_EXIT = 0x37


@allure_testcase(
    需求ID="",
    用例ID='',
    用例名称='监控OTA Master MCU2_DCDC(汇川) UDS刷写流程',
    主功能='UDS诊断',
    功能='诊断刷写监控',
    子功能='MCU1_TM1UDS流程监控',
    前置条件='1.KL30 On;\n2.KL15 On;\n3.CAN3总线连接正常;\n4.OTA Master已启动MCU2_DCDC刷写流程;\n',
    用例描述="""
        MCU2_DCDC(汇川) UDS刷写流程 (CAN3, 扩展帧):
        一、预编程
        1. 22 F193 读取硬件版本(物理)
        2. 22 F195 读取软件版本(物理)
        3. 10 03   扩展会话(功能)
        4. 31 01 FF02 检查预编程条件(物理)
        5. 85 02   禁用DTC(功能)
        6. 28 03 03 禁用通讯(功能)

        二、主编程
        1. 10 02   进入编程会话
        2. 27 03   请求种子
           27 04   下发密钥(汇川位混合算法)
        3. 2E F184 写入指纹
        4. 下载驱动 34/36/37, 31 01 F1A0完整性
        5. 循环{ 31 01 FF00擦除 -> 34/36/37下载 -> 31 01 F1A0 }
        6. 31 01 FF01 检查依赖性
        7. 11 01 ECU硬件复位

        三、后编程
        1. 10 03   扩展会话
        2. 28 00 01 恢复通讯
        3. 85 01   开启DTC
        4. 10 01   默认会话
        5. 14 FF FF FF 清除DTC
    """,
    优先级=Priority.P0,
)
def test_mcu2_dcdc_ota_flash(tcan: CanBus, modelConfig: ModelConfig):
    """监听OTA刷写报文并自动回复MCU1_TM1(汇川)标准应答"""
    with allure_step(1, "初始化UDS监控应答器", "应答器初始化成功"):
        responder = MCU1UDSResponder(
            can_bus=tcan,
            request_id=MCU2_DCDC_PHYSICAL_ID,
            response_id=MCU2_DCDC_RESPONSE_ID,
            functional_id=MCU2_DCDC_FUNCTIONAL_ID,
        )
        print("  MCU2_DCDC UDS 监控应答器已初始化 (汇川)")
        print(f"  物理寻址ID: 0x{MCU2_DCDC_PHYSICAL_ID:08X} (29-bit扩展帧)")
        print(f"  功能寻址ID: 0x{MCU2_DCDC_FUNCTIONAL_ID:08X}")
        print(f"  响应ID:     0x{MCU2_DCDC_RESPONSE_ID:08X}")

    with allure_step(2, "启动UDS监控", "监控已启动"):
        responder.start_monitoring()
        print("  等待 OTA Master 发起 MCU1_TM1 刷写流程...")
        print("  (请确保 OTA Master 已启动 MCU1_TM1 刷写)")

    try:
        with allure_step(3, "持续监听刷写报文，超时自动退出", "监听结束"):
            print("刷写监控持续监听：\n1. 从未收到报文则等待最大等待3000s超时；\n2. 收到过报文后连续10s无新帧自动退出")
            max_total_wait = 3000
            idle_exit_sec = 10
            start_total = time.time()
            start_idle = time.time()
            has_received_msg = False

            while True:
                if time.time() - start_total >= max_total_wait:
                    if not has_received_msg:
                        print(f"Timeout: 全程未收到任何UDS报文，等待{max_total_wait}s达到上限，退出监听")
                    else:
                        print(f"Timeout: 已达到最大等待{max_total_wait}s，强制退出监听")
                    break
                if time.time() - start_idle >= idle_exit_sec and has_received_msg:
                    print(f"已收到刷写报文，连续{idle_exit_sec}秒无新请求，自动退出监听")
                    break
                time.sleep(0.5)
    finally:
        with allure_step(4, "停止监控", "监控已停止"):
            responder.stop_monitoring()


# ==============================================================================
# MCU1 (汇川) 安全算法: 27 03/04, 4字节位混合
# 对齐C函数 UdsCalcKeyMcu1 (ota_master_security_algorithm.cpp)
# ==============================================================================

class MCU1SecurityAlgo:
    """
    MCU1 (汇川) 安全算法: 27 03(请求种子) / 27 04(校验密钥)
    XOR常量: {0x1A, 0x2B, 0x3C, 0x4D} + 位重排
    """
    SEC_LEVEL = 0x03
    SEC_KEY_LEVEL = 0x04
    XOR_ARRAY = [0x1A, 0x2B, 0x3C, 0x4D]

    def __init__(self):
        self._seed_4b: bytes = b""
        self._unlocked: bool = False

    def generate_seed(self, subfunc: int) -> bytes:
        if subfunc == self.SEC_LEVEL:
            self._seed_4b = random.randbytes(4)
            return self._seed_4b
        return b"\x00\x00\x00\x00"

    def calc_key(self, seed_4b: bytes) -> bytes:
        seedArray = [seed_4b[0], seed_4b[1], seed_4b[2], seed_4b[3]]
        calData = [seedArray[i] ^ self.XOR_ARRAY[i] for i in range(4)]
        getKey = [0] * 4
        getKey[0] = ((calData[0] & 0xF0) >> 4) | (calData[3] & 0xF0)
        getKey[1] = ((calData[1] & 0x0F) << 4) | ((calData[2] & 0xF0) >> 4)
        getKey[2] = ((calData[2] & 0x1C) << 3) | ((calData[0] & 0x3E) >> 1)
        getKey[3] = (calData[3] & 0xF0) | ((calData[1] & 0x0F) >> 4)
        temp = ((getKey[0] & 0xFF) << 24) | ((getKey[1] & 0xFF) << 16) | \
               ((getKey[2] & 0xFF) << 8) | (getKey[3] & 0xFF)
        return temp.to_bytes(4, byteorder="big", signed=False)

    def verify_key(self, subfunc: int, key: bytes) -> bool:
        if len(key) != 4:
            return False
        if subfunc == self.SEC_KEY_LEVEL:
            expect = self.calc_key(self._seed_4b)
            if key == expect:
                self._unlocked = True
                return True
        return False

    def reset(self):
        self._unlocked = False
        self._seed_4b = b''


# ==============================================================================
# UDS - 模拟 MCU1_TM1 ECU 应答 (基于 TCU 模板)
# ==============================================================================

class MCU1UDSResponder:
    """模拟 MCU1_TM1 (汇川) ECU UDS 应答器"""

    def __init__(
        self,
        can_bus: CanBus,
        request_id: int = OTA_MASTER_REQUEST_ID,
        response_id: int = TBOX_RESPONSE_ID,
        functional_id: int = OTA_MASTER_FUNCTIONAL_ID,
        security_algo: Optional[MCU1SecurityAlgo] = None,
    ):
        self.can_bus = can_bus
        self.request_id = request_id
        self.functional_id = functional_id
        self.response_id = response_id
        self.security_algo = security_algo or MCU1SecurityAlgo()

        self._listen_ids = {request_id, functional_id}

        self._current_session: int = 0x01
        self._programming_active: bool = False
        self._transfer_active: bool = False
        self._transfer_block_number: int = 0
        self._transfer_address: int = 0
        self._transfer_size: int = 0
        self._transfer_data: bytearray = bytearray()

        # DID 模拟数据
        self._hardware_version: bytes = b'\x01\x00\x02\x03'  # F193
        self._software_version: bytes = b'\x00\x00\x00\x01'  # F195
        self._vin: str = "LGWSIRUNTESTWX343"
        self._fingerprint: bytes = b''
        self._part_no = bytes.fromhex("31303033323031393034323931")

        self._monitoring: bool = False
        self._monitor_thread: Optional[threading.Thread] = None
        self._message_queue: Queue = Queue()

        self._service_handlers: Dict[int, Callable] = {
            0x10: self._handle_session_control,
            0x11: self._handle_ecu_reset,
            0x27: self._handle_security_access,
            0x22: self._handle_read_did,
            0x2E: self._handle_write_did,
            0x28: self._handle_communication_control,
            0x85: self._handle_dtc_control,
            0x31: self._handle_routine_control,
            0x34: self._handle_request_download,
            0x36: self._handle_transfer_data,
            0x37: self._handle_transfer_exit,
            0x3E: self._handle_tester_present,
            0x14: self._handle_clear_dtc,
        }

        self._send_lock = threading.Lock()

    # --------------------------------------------------------------------------
    # 监控控制 (与TCU一致)
    # --------------------------------------------------------------------------
    def send(self, canid: int, data: bytes):
        self.can_bus.send(Message(arbitration_id=canid, is_extended_id=True, is_fd=False, data=data))

    def start_monitoring(self):
        """启动监控"""
        if self._monitoring:
            return
        self._monitoring = True
        self.security_algo.reset()
        self._monitor_thread = threading.Thread(target=self._monitor_loop, daemon=True)
        self._monitor_thread.start()
        print("  [监控] MCU2_DCDC UDS 监控已启动")

    def stop_monitoring(self):
        """停止监控"""
        self._monitoring = False
        if self._monitor_thread:
            self._monitor_thread.join(timeout=5)
            self._monitor_thread = None
        print("  [监控] MCU2_DCDC UDS 监控已停止")

    def _monitor_loop(self):
        while self._monitoring:
            try:
                msg = self.can_bus.recv(timeout=0.1)
                if msg is None:
                    continue
                if msg.arbitration_id == self.response_id:
                    print(f"[DBG] 丢弃自身应答帧 ID:0x{msg.arbitration_id:X}")
                    continue
                if msg.arbitration_id not in self._listen_ids:
                    print(f"[DBG] 未监听帧 ID:0x{msg.arbitration_id:X}")
                    continue
                addr_type = "物理寻址" if msg.arbitration_id == self.request_id else "功能寻址"
                if len(msg.data) < 1:
                    continue
                pci = msg.data[0]
                pci_type = (pci & 0xF0) >> 4
                if pci_type == 0x00:
                    if hasattr(self, '_multi_frame_data'):
                        del self._multi_frame_data
                    if hasattr(self, '_multi_frame_expected'):
                        del self._multi_frame_expected
                    data_length = pci & 0x0F
                    if data_length > 0 and len(msg.data) > 1:
                        request_data = msg.data[1:1 + data_length]
                    else:
                        request_data = msg.data[1:] if len(msg.data) > 1 else msg.data
                    self._process_request(request_data)
                elif pci_type == 0x01:
                    length = ((pci & 0x0F) << 8) | msg.data[1]
                    self._multi_frame_data = msg.data[2:]
                    self._multi_frame_expected = length - 6
                    fc_frame = bytes.fromhex("30000A") + b"\x00" * 5
                    self.send(self.response_id, fc_frame)
                    print("下发ISO-TP流控帧: 30 00 0A")
                    blk_no = 0
                    if len(self._multi_frame_data) >= 1:
                        sid = self._multi_frame_data[0]
                        if sid == 0x36:
                            blk_no = self._multi_frame_data[1]
                            resp_pending = bytes.fromhex("037F3678") + b'\xCC' * 4
                            self.send(self.response_id, resp_pending)
                            print(f"36先发挂起78 {blk_no:02X}，延后回76")
                elif pci_type == 0x02:
                    if hasattr(self, '_multi_frame_data'):
                        seq_num = pci & 0x0F
                        data_payload = msg.data[1:]
                        self._multi_frame_data += data_payload
                        self._multi_frame_expected -= len(data_payload)
                        if self._multi_frame_expected <= 0:
                            self._process_request(self._multi_frame_data)
                            del self._multi_frame_data
                            del self._multi_frame_expected
            except Exception as e:
                print(f"  [监控错误] {e}")
                continue

    def _process_request(self, request: bytes):
        """处理UDS请求"""
        if len(request) < 1:
            return
        service_id = request[0]
        print(f"  [请求] {request.hex(' ').upper()}")
        handler = self._service_handlers.get(service_id)
        if handler:
            try:
                response = handler(request)
                print(f"  [响应] {response.hex(' ').upper()}")
                self._send_response(response)
            except Exception as e:
                print(f"处理失败: {e}")
                nrc_response = bytes([0x7F, service_id, 0x22])
                self._send_response(nrc_response)
                raise Exception(f"_process_request error: {str(e)}")
        else:
            print(f"不支持的服务: 0x{service_id:02X}")
            nrc_response = bytes([0x7F, service_id, 0x11])
            self._send_response(nrc_response)

    def _send_response(self, response: bytes):
        if not self._send_lock.acquire(blocking=False):
            return
        try:
            if len(response) <= 7:
                pci = len(response)
                frame = bytes([pci]) + response + bytes(7 - len(response))
                self.send(self.response_id, frame)
            else:
                length = len(response)
                ff_pci = 0x10 | ((length >> 8) & 0x0F)
                frame = bytes([ff_pci, length & 0xFF]) + response[:6]
                self.send(self.response_id, frame)
                remaining = response[6:]
                seq_num = 1
                while remaining:
                    chunk = remaining[:7]
                    cf_pci = 0x20 | (seq_num & 0x0F)
                    frame = bytes([cf_pci]) + chunk + bytes(7 - len(chunk))
                    self.send(self.response_id, frame)
                    remaining = remaining[7:]
                    seq_num = (seq_num + 1) & 0x0F
        except ValueError as e:
            raise Exception(f"_send_response error: {str(e)}")
        finally:
            self._send_lock.release()

    # --------------------------------------------------------------------------
    # 服务处理 (对齐 MCU1 刷写流程)
    # --------------------------------------------------------------------------
    def _handle_session_control(self, request: bytes) -> bytes:
        """0x10 诊断会话控制, 支持NOACK(0x81/0x83抑制响应)"""
        if len(request) < 2:
            return bytes([0x7F, 0x10, 0x13])
        sub = request[1]
        if sub & 0x80:
            self._current_session = sub & 0x7F
            return b''
        self._current_session = sub
        if sub == 0x01:
            return bytes.fromhex("5001003207D0FF")
        elif sub == 0x03:
            return bytes.fromhex("5003003207D0FF")
        elif sub == 0x02:
            return bytes.fromhex("5002003201F4CC")
        return bytes([0x7F, 0x10, 0x12])

    def _handle_ecu_reset(self, request: bytes) -> bytes:
        """0x11 ECU复位"""
        if len(request) < 2:
            return bytes([0x7F, 0x11, 0x13])
        sub = request[1]
        if sub == 0x01:
            return bytes.fromhex("5101")
        elif sub == 0x02:
            self._programming_active = True
            return bytes.fromhex("5102")
        return bytes([0x7F, 0x11, 0x12])

    def _handle_security_access(self, request: bytes) -> bytes:
        """0x27 安全访问: 03=MCU1请求种子 | 04=MCU1校验密钥"""
        if len(request) < 2:
            return bytes([0x7F, 0x27, 0x13])
        sub = request[1]
        if sub == 0x03:
            seed = self.security_algo.generate_seed(sub)
            print(f"    [安全] MCU2_DCDC 种子: {seed.hex(' ').upper()}")
            return bytes([0x67, sub]) + seed
        elif sub == 0x04:
            if len(request) < 6:
                return bytes([0x7F, 0x27, 0x13])
            key = request[2:6]
            if self.security_algo.verify_key(sub, key):
                print(f"    [安全] MCU2_DCDC 密钥校验通过")
                return bytes([0x67, sub])
            else:
                print(f"    [安全] MCU2_DCDC 密钥校验失败")
                return bytes([0x7F, 0x27, 0x33])
        return bytes([0x7F, 0x27, 0x12])

    def _handle_read_did(self, request: bytes) -> bytes:
        """0x22 读DID (F193=硬件, F195=软件, F184=指纹, F190=VIN)"""
        if len(request) < 3:
            return bytes([0x7F, 0x22, 0x13])
        did = (request[1] << 8) | request[2]
        if did == 0xF193:
            return bytes([0x62, 0xF1, 0x93]) + self._hardware_version
        elif did == 0xF195:
            return bytes([0x62, 0xF1, 0x95]) + self._software_version
        elif did == 0xF190:
            vin = self._vin.encode('ascii').ljust(17, b'\x00')
            return bytes([0x62, 0xF1, 0x90]) + vin
        elif did == 0xF187:
            return bytes([0x62, 0xF1, 0x87]) + self._part_no
        elif did == 0xF189:
            return bytes([0x62, 0xF1, 0x89]) + self._software_version
        return bytes([0x7F, 0x22, 0x31])

    def _handle_write_did(self, request: bytes) -> bytes:
        """0x2E 写DID (F184=写入指纹)"""
        if len(request) < 4:
            return bytes([0x7F, 0x2E, 0x13])
        did = (request[1] << 8) | request[2]
        data = request[3:]
        if did == 0xF184:
            self._fingerprint = data
            print(f"    [指纹] MCU2_DCDC 写入指纹: {data.hex(' ').upper()}")
            return bytes([0x6E, 0xF1, 0x84])
        return bytes([0x7F, 0x2E, 0x31])

    def _handle_communication_control(self, request: bytes) -> bytes:
        """0x28 通信控制"""
        if len(request) < 3:
            return bytes([0x7F, 0x28, 0x13])
        sub = request[1]
        return bytes([0x68, sub])

    def _handle_dtc_control(self, request: bytes) -> bytes:
        """0x85 DTC控制"""
        if len(request) < 2:
            return bytes([0x7F, 0x85, 0x13])
        sub = request[1]
        return bytes([0xC5, sub])

    def _handle_routine_control(self, request: bytes) -> bytes:
        """0x31 例程控制: FF02=预编程, F1A0=完整性, FF00=擦除, FF01=依赖性"""
        if len(request) < 4:
            return bytes([0x7F, 0x31, 0x13])
        sub = request[1]
        rid = (request[2] << 8) | request[3]
        if rid == 0xFF00:
            time.sleep(0.3)
            return bytes.fromhex("7101FF0000")
        elif rid in (0xFF02, 0xF1A0, 0xFF01):
            return bytes([0x71, sub, request[2], request[3], 0x00])
        return bytes([0x7F, 0x31, 0x31])

    def _handle_request_download(self, request: bytes) -> bytes:
        """0x34 请求下载"""
        if len(request) < 5:
            return bytes([0x7F, 0x34, 0x13])
        self._transfer_active = True
        self._transfer_block_number = 1
        return bytes.fromhex("74200FFF")

    def _handle_transfer_data(self, request: bytes) -> bytes:
        """0x36 传输数据"""
        if not self._transfer_active or len(request) < 2:
            return bytes([0x7F, 0x36, 0x13])
        blk = request[1]
        data = request[2:]
        self._transfer_data.extend(data)
        self._transfer_block_number += 1
        return bytes([0x76, blk])

    def _handle_transfer_exit(self, request: bytes) -> bytes:
        """0x37 退出传输"""
        self._transfer_active = False
        return bytes.fromhex("77")

    def _handle_tester_present(self, request: bytes) -> bytes:
        """0x3E 心跳"""
        if len(request) < 2:
            return bytes([0x7F, 0x3E, 0x13])
        sub = request[1]
        if sub & 0x80:
            return b''
        return bytes([0x7E, sub & 0x7F])

    def _handle_clear_dtc(self, request: bytes) -> bytes:
        """0x14 清除DTC"""
        if len(request) < 4:
            return bytes.fromhex("7F1413")
        return bytes.fromhex("54")
