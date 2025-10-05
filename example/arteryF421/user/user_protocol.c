/*
-------------------------------------------------------------
 协议帧格式说明
-------------------------------------------------------------
  - 十六进制制
  - 协议帧数据总长度 = 2 + 1 + 1 + 1 + N + 2 = 7 + N  (N≥1)
  - 应答帧命令: Cmd = Cmd | 0x80

  字段    长度    说明
  ----------------------------------------------------------
  帧头    2      固定为0x53, 0x47
  地址    1      分别对主机箱控制板0x01，二合一手柄0x02，拍摄模块0x03
  命令    1      分别针对不同的板子展开描述
  长度    1      表明数据长度
  数据    N      data
  CRC校验 2      根据CRC16校验
-------------------------------------------------------------
*/
#include "user_protocol.h"
#include "userconfig.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// 使用静态内存作为协议帧缓冲区
#define PROTOCOL_FRAME_MAX_LEN 128
static uint8_t s_protocol_frame_buf[PROTOCOL_FRAME_MAX_LEN];

// 应答回调函数类型
typedef void (*protocol_response_callback_t)(uint8_t chCmd, const uint8_t *pchData, \
                                            uint8_t chDataLen);
static protocol_response_callback_t s_protocol_response_cb = NULL;

// 注册应答回调
void protocol_register_response_callback(protocol_response_callback_t pfcn) 
{
    s_protocol_response_cb = pfcn;
}

uint16_t calculate_crc16(const uint8_t *pchData, uint8_t chLength) {
    uint16_t hwCrc = 0xFFFF;
    for (uint8_t i = 0; i < chLength; i++) {
        hwCrc ^= pchData[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (hwCrc & 0x0001) {
                hwCrc = (hwCrc >> 1) ^ 0xA001;
            } else {
                hwCrc >>= 1;
            }
        }
    }
    return hwCrc;
}

// 解析协议帧并处理，支持粘包处理的协议帧解析
void protocol_parse_and_handle(const uint8_t *pchBuffer, uint16_t hwBufferLen)
{
    uint16_t hwOffset = 0;
    while (hwOffset + 7 <= hwBufferLen) {
        // 查找帧头
        if (pchBuffer[hwOffset] != 0x53 || pchBuffer[hwOffset + 1] != 0x47) {
            hwOffset++;
            continue;
        }
        uint8_t chAddr = pchBuffer[hwOffset + 2];
        if (chAddr != PROTOCOL_BOARD_ADDR) {
            // 不是发给本设备的，跳过本帧
            hwOffset++;
            continue;
        }
        uint8_t chDataLen = pchBuffer[hwOffset + 4];
        uint16_t frame_len = 7 + chDataLen;
        if (hwOffset + frame_len > hwBufferLen) {
            // 数据不完整，等待下次补齐
            break;
        }
        // 校验CRC
        uint16_t crc_recv = pchBuffer[hwOffset + 5 + chDataLen] | (pchBuffer[hwOffset + 6 + chDataLen] << 8);
        uint16_t crc_calc = calculate_crc16(&pchBuffer[hwOffset], 5 + chDataLen);
        #ifdef USER_PROTOCOL_CRC
        if (crc_recv == crc_calc) 
        {
            uint8_t chCmd = pchBuffer[hwOffset + 3];
            if (s_protocol_response_cb) {
                s_protocol_response_cb(chCmd, &pchBuffer[hwOffset + 5], chDataLen);
            }
            hwOffset += frame_len;
        } else {
            // CRC错误，跳过当前帧头，继续查找下一个帧头
            hwOffset++;
        }
        #else
            uint8_t chCmd = pchBuffer[offset + 3];
            if (s_protocol_response_cb) {
                s_protocol_response_cb(chCmd, &pchBuffer[offset + 5], chDataLen);
            }
            hwOffset += chDataLen;
        #endif
    }
}

static pfcnWrite s_protocol_response_func = NULL;
// 打包应答帧
uint16_t pack_protocol_response_frame(uint8_t chCmd, const uint8_t *pchData,
                                        uint16_t hwDataLen)
{
    if (!s_protocol_response_func) return 0;
    
    uint16_t hwFrameLen = 7 + hwDataLen;
    if (hwFrameLen > PROTOCOL_FRAME_MAX_LEN) {
        return 0;
    }
    
    uint8_t *pchFrame = s_protocol_frame_buf;
    pchFrame[0] = 0x53;
    pchFrame[1] = 0x47;
    pchFrame[2] = PROTOCOL_BOARD_ADDR;
    pchFrame[3] = chCmd | 0x80; // 应答命令
    pchFrame[4] = hwDataLen;
    if (hwDataLen > 0 && pchData != NULL) {
        memcpy(&pchFrame[5], pchData, hwDataLen);
    }
    uint16_t crc = calculate_crc16(pchFrame, 5 + hwDataLen);
    pchFrame[5 + hwDataLen] = crc & 0xFF;
    pchFrame[6 + hwDataLen] = (crc >> 8) & 0xFF;
    hwFrameLen = s_protocol_response_func(0x01, pchFrame, hwFrameLen);
    
    return hwFrameLen;
}

void protocol_register_response_func(pfcnWrite pfcn)
{
    s_protocol_response_func = pfcn;
}

/* 注册主端发送函数 */ 

// 主端发送函数指针类型
typedef void (*protocol_send_func_t)(const uint8_t *pchFrame, uint16_t hwFrameLen);
static protocol_send_func_t s_protocol_send_func = NULL;

// 注册主端发送函数
void protocol_register_send_func(protocol_send_func_t pfcn)
{
    s_protocol_send_func = pfcn;
}

// 主端协议帧发送接口
// addr: 目标板地址，chCmd: 命令，pchData: 数据指针，hwDataLen: 数据长度
// 返回0成功，-1失败
int protocol_send_frame(uint8_t chAddr, uint8_t chCmd, const uint8_t *pchData, uint16_t hwDataLen)
{
    if (!s_protocol_send_func) return -1;
    uint16_t hwFrameLen = 7 + hwDataLen;
    if (hwFrameLen > PROTOCOL_FRAME_MAX_LEN) return -1;
    uint8_t *pchFrame = s_protocol_frame_buf;
    pchFrame[0] = 0x53;
    pchFrame[1] = 0x47;
    pchFrame[2] = chAddr;
    pchFrame[3] = chCmd;
    pchFrame[4] = hwDataLen;
    if (hwDataLen > 0 && pchData != NULL) {
        memcpy(&pchFrame[5], pchData, hwDataLen);
    }
    uint16_t crc = calculate_crc16(pchFrame, 5 + hwDataLen);
    pchFrame[5 + hwDataLen] = crc & 0xFF;
    pchFrame[6 + hwDataLen] = (crc >> 8) & 0xFF;
    s_protocol_send_func(pchFrame, hwFrameLen);
    return 0;
}