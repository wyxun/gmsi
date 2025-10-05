#ifndef __USER_PROTOCOL_H__
#define __USER_PROTOCOL_H__
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "userconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

// 主端发送函数指针类型
typedef void (*protocol_send_func_t)(const uint8_t *pchFrame, uint16_t hwFrameLen);

// 注册主端发送函数
void protocol_register_send_func(protocol_send_func_t pfcn);

// 主端协议帧发送接口
int protocol_send_frame(uint8_t chAddr, uint8_t chCmd, const uint8_t *pchData, uint16_t hwDataLen);

// 应答回调函数类型
typedef void (*protocol_response_callback_t)(uint8_t chCmd, const uint8_t *pchData, uint8_t chDataLen);
typedef uint16_t (*protocol_response_func_t)(const uint8_t *pchFrame, uint16_t hwFrameLen);
// 注册应答回调
void protocol_register_response_callback(protocol_response_callback_t pfcn);
void protocol_register_response_func(pfcnWrite pfcn);
// 解析协议帧并处理，支持粘包处理
void protocol_parse_and_handle(const uint8_t *pchBuf, uint16_t hwBufLen);

// 打包应答帧
uint16_t pack_protocol_response_frame(uint8_t chCmd, const uint8_t *pchData,
                                        uint16_t hwDataLen);

#ifdef __cplusplus
}
#endif

#endif // __USER_PROTOCOL_H__