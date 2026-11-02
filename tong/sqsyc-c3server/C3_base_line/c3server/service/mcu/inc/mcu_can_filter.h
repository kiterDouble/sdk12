#ifndef __MCU_CAN_FILTER_H__
#define __MCU_CAN_FILTER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name McuCanFilterInit
 * @brief 启动CAN ID过滤表同步：周期向signalServer请求全量过滤表，
 *        直到收到应答。应答经 McuSrvSendMesg RPC 回送至本进程，
 *        在 mcu_send.c 中截获并调用 McuCanFilterSaveFromRes 全局保存。
 * @return RET_OK 成功，RET_ERR 失败
 */
int32_t McuCanFilterInit(void);

/**
 * @name McuCanFilterSaveFromRes
 * @brief 解析signalServer的CAN ID过滤表一页应答并合并保存到全局（不再下发MCU）
 * @param data reqIndex(2B BE) + respListSize(2B BE) + N×5B(1B 通道<<5 + 4B 29bit CAN ID)
 * @param len 数据长度
 * @return RET_OK 成功，RET_ERR 失败
 */
int32_t McuCanFilterSaveFromRes(const uint8_t *data, uint32_t len);

/**
 * @name McuCanFilterGetList
 * @brief 获取全局保存的CAN ID过滤表（拷贝，条目编码: canId | (channel << 29)）
 * @param buf 输出缓冲区，可为NULL（仅查询条目数）
 * @param num 入参为缓冲区容量（条目数），出参为实际条目数
 * @return RET_OK 成功，RET_ERR 过滤表尚未同步或参数非法
 */
int32_t McuCanFilterGetList(uint32_t *buf, uint32_t *num);

/**
 * @name McuCanFilterIsReady
 * @brief 查询CAN ID过滤表是否已全量同步完成
 * @return 1=已就绪，0=未就绪
 */
uint8_t McuCanFilterIsReady(void);

/**
 * @name McuCanFilterIsMatch
 * @brief 判断指定CAN帧是否命中过滤表
 * @param channel CAN通道号(取值0-7)
 * @param canId   CAN ID(主机序；bit31为扩展帧标志，过滤表为29位ID空间，不参与比对)
 * @return RET_OK 命中，RET_ERR 未命中或过滤表未就绪
 */
int32_t McuCanFilterIsMatch(uint8_t channel, uint32_t canId);

/**
 * @name McuCanFilterForwardRawToData
 * @brief 将CAN_RAW报文按CAN ID过滤表过滤后，重组为CAN_DATA格式广播给signalServer。
 *        过滤表未加载完成时不做任何转发；报文截断/dlc异常时整批丢弃。
 * @param payload CAN_RAW消息体首地址(2B num + 帧数据)
 * @param srvLen  消息体长度
 */
void McuCanFilterForwardRawToData(const uint8_t *payload, uint16_t srvLen);

#ifdef __cplusplus
}
#endif

#endif
