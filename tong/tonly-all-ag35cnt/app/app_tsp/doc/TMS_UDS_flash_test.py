"""
TMS (热管理系统) UDS 刷写流程监控测试

参考文档:
  - app_tsp/doc/TMS升级流程.txt
  - app_tsp/doc/TCU_UDS_flash_test.py (模板)

ECU 配置:
  - TMS_A: phy=0x67C, res=0x674, func=0x0 (无功能寻址)
  - CAN1 整车, ISO-15765 标准帧 11-bit

刷写流程 (对接 ota_master_fota_ecu.cpp: FotaEcuExecTms):
  一、预编程
    1. 10 03    扩展会话
    2. 31 01 02 03 检查预编程条件
    3. 85 02    禁用DTC
    4. 28 03 03 禁用非诊断通讯
    5. 22 F1 87 读取零件号
  二、主编程
    1. 10 02    主编程会话
    2. 27 09/0A TMS安全访问
    3. 驱动下载 34/36/37 + 31 01 02 02 完整性
    4. 2E F1 5A 写入指纹 (16字节)
    5. 31 01 FF00 擦除内存(循环)
    6. 下载数据 34/36/37
    7. 31 01 02 02 检查编程完整性
    8. 31 01 FF01 检查编程依赖性
    9. 11 01    ECU硬件复位
  三、后编程
    1. 10 03    扩展会话
    2. 28 00 03 恢复通讯
    3. 85 01    使能DTC
    4. 10 01    默认会话
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
# CAN ID 配置 (OTA Master -> TMS)
# ==============================================================================

TMS_PHYSICAL_ID = 0x67C
TMS_FUNCTIONAL_ID = 0x7DF  # VCU广播ID, TMS回应此广播ID
TMS_RESPONSE_ID = 0x674

OTA_MASTER_REQUEST_ID = TMS_PHYSICAL_ID
TBOX_RESPONSE_ID = TMS_RESPONSE_ID
OTA_MASTER_FUNCTIONAL_ID = TMS_FUNCTIONAL_ID

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
    用例名称='监控OTA Master TMS UDS刷写流程',
    主功能='UDS诊断',
    功能='诊断刷写监控',
    子功能='TMS流程监控',
    前置条件='1.KL30 On;\n2.KL15 On;\n3.CAN1总线连接正常;\n4.OTA Master已启动TMS刷写流程;\n',
    用例描述="""
        TMS UDS刷写流程 (CAN1, 标准帧11-bit):
        一、预编程
        1. 10 03   扩展会话
        2. 31 01 02 03 检查预编程条件
        3. 85 02   禁用DTC
        4. 28 03 03 禁用通讯
        5. 22 F1 87 读取零件号

        二、主编程
        1. 10 02   进入编程会话
        2. 27 09   请求种子
           27 0A   下发密钥(TMS算法)
        3. 下载驱动 34/36/37, 31 01 02 02完整性
        4. 2E F1 5A 写入指纹(16字节)
        5. 31 01 FF00 擦除内存(循环)
        6. 34/36/37 下载数据
        7. 31 01 02 02 检查完整性
        8. 31 01 FF01 检查依赖性
        9. 11 01 ECU硬件复位

        三、后编程
        1. 10 03   扩展会话
        2. 28 00 03 恢复通讯
        3. 85 01   开启DTC
        4. 10 01   默认会话
        5. 14 FF FF FF 清除DTC
    """,
    优先级=Priority.P0,
)
def test_tms_ota_flash(tcan: CanBus, modelConfig: ModelConfig):
    """监听OTA刷写报文并自动回复TMS标准应答"""
    with allure_step(1, "初始化UDS监控应答器", "应答器初始化成功"):
        responder = TMSUDSResponder(
            can_bus=tcan,
            request_id=TMS_PHYSICAL_ID,
            response_id=TMS_RESPONSE_ID,
            functional_id=TMS_FUNCTIONAL_ID,
        )
        print("  TMS UDS 监控应答器已初始化")
        print(f"  物理寻址ID: 0x{TMS_PHYSICAL_ID:03X} (11-bit标准帧)")
        print(f"  功能寻址ID: 0x{TMS_FUNCTIONAL_ID:03X}")
        print(f"  响应ID:     0x{TMS_RESPONSE_ID:03X}")

    with allure_step(2, "启动UDS监控", "监控已启动"):
        responder.start_monitoring()
        print("  等待 OTA Master 发起 TMS 刷写流程...")

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
                    print(f"Timeout: 全程未收到任何UDS报文，等待{max_total_wait}s达到上限，退出监听")
                    break
                if time.time() - start_idle >= idle_exit_sec and has_received_msg:
                    print(f"已收到刷写报文，连续{idle_exit_sec}秒无新请求，自动退出监听")
                    break
                time.sleep(0.5)
    finally:
        with allure_step(4, "停止监控", "监控已停止"):
            responder.stop_monitoring()


# ==============================================================================
# TMS 安全算法: 27 09/0A
# 对齐C函数 UdsCalcKeyTms (ota_master_security_algorithm.cpp)
# ==============================================================================

class TMSSecurityAlgo:
    """
    TMS 安全算法: 27 09(请求种子) / 27 0A(校验密钥)
    ring step 4轮: 奇偶交换 → 加常数 → XOR+左旋7 → 异或种子化常数
    """
    SEC_LEVEL = 0x09
    SEC_KEY_LEVEL = 0x0A
    XOR_VAL = 0x5A5A5A5A
    ADD_VAL = 0x12345678
    MUL_CONST = 0x9E3779B9
    XOR_CONST = 0x85EBCA77
    UNLOCKSEED = 0xFFFFFFFF
    UNDEFINESEED = 0x00000000
    UNLOCKKEY = 0xAAAAAAAA

    def __init__(self):
        self._seed_4b: bytes = b""
        self._unlocked: bool = False

    def generate_seed(self, subfunc: int) -> bytes:
        if subfunc == self.SEC_LEVEL:
            self._seed_4b = random.randbytes(4)
            return self._seed_4b
        return b"\x00\x00\x00\x00"

    def calc_key(self, seed_4b: bytes) -> bytes:
        s = (seed_4b[0] << 24) | (seed_4b[1] << 16) | (seed_4b[2] << 8) | seed_4b[3]
        if s == self.UNLOCKSEED or s == self.UNDEFINESEED:
            return self.UNLOCKKEY.to_bytes(4, byteorder="big")

        t1 = ((s & 0xAAAAAAAA) >> 1) | ((s & 0x55555555) << 1)
        t2 = (t1 + self.ADD_VAL) & 0xFFFFFFFF
        t3 = ((t2 ^ self.XOR_VAL) << 7) | ((t2 ^ self.XOR_VAL) >> 25)
        key = (t3 ^ ((s * self.MUL_CONST + self.XOR_CONST) & 0xFFFFFFFF)) & 0xFFFFFFFF
        return key.to_bytes(4, byteorder="big", signed=False)

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
# UDS - 模拟 TMS ECU 应答
# ==============================================================================

class TMSUDSResponder:
    """模拟 TMS ECU UDS 应答器"""

    def __init__(
        self,
        can_bus: CanBus,
        request_id: int = OTA_MASTER_REQUEST_ID,
        response_id: int = TBOX_RESPONSE_ID,
        functional_id: int = OTA_MASTER_FUNCTIONAL_ID,
        security_algo: Optional[TMSSecurityAlgo] = None,
    ):
        self.can_bus = can_bus
        self.request_id = request_id
        self.functional_id = functional_id
        self.response_id = response_id
        self.security_algo = security_algo or TMSSecurityAlgo()

        self._listen_ids = {request_id, functional_id}

        self._current_session: int = 0x01
        self._programming_active: bool = False
        self._transfer_active: bool = False
        self._transfer_block_number: int = 0
        self._transfer_address: int = 0
        self._transfer_size: int = 0
        self._transfer_data: bytearray = bytearray()

        self._software_version: str = "0002"
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
    def send(self, canid: int, data: bytes):
        self.can_bus.send(Message(arbitration_id=canid, is_extended_id=False, is_fd=False, data=data))

    def start_monitoring(self):
        if self._monitoring:
            return
        self._monitoring = True
        self.security_algo.reset()
        self._monitor_thread = threading.Thread(target=self._monitor_loop, daemon=True)
        self._monitor_thread.start()
        print("  [监控] TMS UDS 监控已启动")

    def stop_monitoring(self):
        self._monitoring = False
        if self._monitor_thread:
            self._monitor_thread.join(timeout=5)
            self._monitor_thread = None
        print("  [监控] TMS UDS 监控已停止")

    def _monitor_loop(self):
        while self._monitoring:
            try:
                msg = self.can_bus.recv(timeout=0.1)
                if msg is None:
                    continue
                if msg.arbitration_id == self.response_id:
                    continue
                if msg.arbitration_id not in self._listen_ids:
                    continue
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
                    fc_frame = bytes.fromhex("3000FF") + b"\x00" * 5
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
    # 服务处理 (对齐 TMS 刷写流程)
    # --------------------------------------------------------------------------
    def _handle_session_control(self, request: bytes) -> bytes:
        """0x10 诊断会话控制"""
        if len(request) < 2:
            return bytes([0x7F, 0x10, 0x13])
        sub = request[1]
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
        """0x27 安全访问: 09=TMS请求种子 | 0A=TMS校验密钥"""
        if len(request) < 2:
            return bytes([0x7F, 0x27, 0x13])
        sub = request[1]
        if sub == 0x09:
            seed = self.security_algo.generate_seed(sub)
            print(f"    [安全] TMS 种子: {seed.hex(' ').upper()}")
            return bytes([0x67, sub]) + seed
        elif sub == 0x0A:
            if len(request) < 6:
                return bytes([0x7F, 0x27, 0x13])
            key = request[2:6]
            if self.security_algo.verify_key(sub, key):
                print("    [安全] TMS 密钥校验通过")
                return bytes([0x67, sub])
            else:
                print("    [安全] TMS 密钥校验失败")
                return bytes([0x7F, 0x27, 0x33])
        return bytes([0x7F, 0x27, 0x12])

    def _handle_read_did(self, request: bytes) -> bytes:
        """0x22 读DID (F187=零件号, F190=VIN)"""
        if len(request) < 3:
            return bytes([0x7F, 0x22, 0x13])
        did = (request[1] << 8) | request[2]
        if did == 0xF187:
            return bytes([0x62, 0xF1, 0x87]) + self._part_no
        elif did == 0xF190:
            vin = self._vin.encode('ascii').ljust(17, b'\x00')
            return bytes([0x62, 0xF1, 0x90]) + vin
        return bytes([0x7F, 0x22, 0x31])

    def _handle_write_did(self, request: bytes) -> bytes:
        """0x2E 写DID (F1 5A=写入指纹, 16字节)"""
        if len(request) < 4:
            return bytes([0x7F, 0x2E, 0x13])
        did = (request[1] << 8) | request[2]
        data = request[3:]
        if did == 0xF15A:
            self._fingerprint = data
            print(f"    [指纹] TMS 写入指纹: {data.hex(' ').upper()}")
            return bytes([0x6E, 0xF1, 0x5A])
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
        """0x31 例程控制: 0203=预编程, 0202=完整性, FF00=擦除, FF01=依赖性"""
        if len(request) < 4:
            return bytes([0x7F, 0x31, 0x13])
        sub = request[1]
        rid = (request[2] << 8) | request[3]
        if rid == 0xFF00:
            time.sleep(0.3)
            return bytes.fromhex("7101FF0000")
        elif rid in (0x0203, 0x0202, 0xFF01):
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
