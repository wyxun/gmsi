/**
 * @file trace_fmt.h
 * @brief Lightweight number-to-string conversion for MCU trace output
 *        (no stdio dependency, pure integer arithmetic)
 */

#ifndef __TRACE_FMT_H__
#define __TRACE_FMT_H__

#include <stdint.h>

/*============================ PROTOTYPES ====================================*/

/**
 * @brief Convert uint32 to hex string "0x12345678"
 * @param wValue   input value
 * @param pchBuf   output buffer (>= 11 bytes)
 * @return pointer to pchBuf
 */
char *trace_fmt_hex32(uint32_t wValue, char *pchBuf);

/**
 * @brief Convert uint16 to hex string "0x1234"
 * @param hwValue  input value
 * @param pchBuf   output buffer (>= 7 bytes)
 * @return pointer to pchBuf
 */
char *trace_fmt_hex16(uint16_t hwValue, char *pchBuf);

/**
 * @brief Convert uint8 to hex string "0x12"
 * @param chValue  input value
 * @param pchBuf   output buffer (>= 5 bytes)
 * @return pointer to pchBuf
 */
char *trace_fmt_hex8(uint8_t chValue, char *pchBuf);

/**
 * @brief Convert int32 to decimal string "-12345"
 * @param nValue   input value
 * @param pchBuf   output buffer (>= 12 bytes)
 * @return pointer to pchBuf
 */
char *trace_fmt_int32(int32_t nValue, char *pchBuf);

/**
 * @brief Convert double/float to decimal string "-3.1415"
 * @param dfValue  input value
 * @param pchBuf   output buffer (>= 24 bytes)
 * @param chDecimals  number of decimal places (1-6)
 * @return pointer to pchBuf
 */
char *trace_fmt_float(double dfValue, char *pchBuf, uint8_t chDecimals);

#endif /* __TRACE_FMT_H__ */
/* EOF */
