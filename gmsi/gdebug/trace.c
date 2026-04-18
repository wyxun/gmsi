/*****************************************************************************
 *   Copyright(C)2009-2019 by GorgonMeducer<embedded_zhuoran@hotmail.com>    *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*============================ INCLUDES ======================================*/
#include "trace.h"
#include <string.h>
#include <stdint.h>

/*============================ PLATFORM DETECTION ============================*/
#ifndef TRACE_USE_LIBC_PRINTF
#   if defined(_MSC_VER) || defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#       define TRACE_USE_LIBC_PRINTF    1
#   else
#       define TRACE_USE_LIBC_PRINTF    0
#   endif
#endif

#if TRACE_USE_LIBC_PRINTF
#   include <stdio.h>
#else
#   include "trace_fmt.h"
#   ifndef TRACE_MCU_WRITE_STRING
#       error "MCU mode requires TRACE_MCU_WRITE_STRING(str) macro, e.g. -DTRACE_MCU_WRITE_STRING(s)=SEGGER_RTT_WriteString(0,s)"
#   endif
#endif

/*============================ MACROS ========================================*/
#ifndef TRACE_DISPLAY_WIDTH
#   define TRACE_DISPLAY_WIDTH          16
#endif

#undef this
#define this        (*ptThis)

/*============================ MACROFIED FUNCTIONS ===========================*/
#ifndef ASSERT
#   define ASSERT(...)
#endif

/*============================ HELPER MACROS =================================*/
#if TRACE_USE_LIBC_PRINTF
#   define TRACE_OUTPUT(buf)            printf("%s", (buf))
#   define TRACE_OUTPUT_CHAR(c)         printf("%c", (c))
#else
#   define TRACE_OUTPUT(buf)            TRACE_MCU_WRITE_STRING(buf)
#   define TRACE_OUTPUT_CHAR(c)         do {        \
        char __chTmp[2] = {(c), '\0'};              \
        TRACE_MCU_WRITE_STRING(__chTmp);             \
    } while(0)
#endif

/*============================ TYPES =========================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
static void __trace_init(trace_cfg_t *ptCFG);
static void __trace_string(const char * pchString);
static void __trace_buffer(const char * pchBuffer, uint_fast16_t hwSize);
static void __trace_uint32_to_string(uint32_t);
static void __trace_int32_to_string(int32_t);
static void __trace_uint16_to_string(uint16_t);
static void __trace_int16_to_string(int16_t);
static void __trace_uint8_to_string(uint8_t);
static void __trace_int8_to_string(int8_t);
static void __trace_float_to_string(float fValue);
static void __trace_double_to_string(double fValue);
static void __trace_byte_stream(uint8_t *, uint_fast16_t);
static void __trace_hword_stream(uint16_t *, uint_fast16_t);
static void __trace_word_stream(uint32_t *, uint_fast16_t);

/*============================ GLOBAL VARIABLES ==============================*/
const i_trace_t TRACE = {
    .Init =             &__trace_init,
    .ToString = {
        .String =       &__trace_string,
        .Buffer =       &__trace_buffer,
        .UInt32 =       &__trace_uint32_to_string,
        .Int32 =        &__trace_int32_to_string,
        .UInt16 =       &__trace_uint16_to_string,
        .Int16 =        &__trace_int16_to_string,
        .UInt8 =        &__trace_uint8_to_string,
        .Int8 =         &__trace_int8_to_string,
        .Float =        &__trace_float_to_string,
        .Double =       &__trace_double_to_string,
        .Bytes =        &__trace_byte_stream,
        .HWords =       &__trace_hword_stream,
        .Words =        &__trace_word_stream,
    },
};

/*============================ IMPLEMENTATION ================================*/

static void __trace_init(trace_cfg_t *ptCFG)
{
    (void)ptCFG;
}

static void __trace_buffer(const char *pchBuffer, uint_fast16_t hwSize)
{
    if (NULL == pchBuffer || 0 == hwSize) {
        return;
    }
#if TRACE_USE_LIBC_PRINTF
    fwrite(pchBuffer, 1, hwSize, stdout);
#else
    /* Use a wrapper to print segment since TRACE_MCU_WRITE_STRING typically expects NULL-term */
    /* However, SEGGER_RTT_Write can take length. 
       If our platform macro supports length, we should use it. */
    #ifdef TRACE_MCU_WRITE_BUFFER
        TRACE_MCU_WRITE_BUFFER(pchBuffer, hwSize);
    #else
        /* Fallback: print char by char if no buffer write available */
        for (uint_fast16_t n = 0; n < hwSize; n++) {
            char __chTmp[2] = {pchBuffer[n], '\0'};
            TRACE_MCU_WRITE_STRING(__chTmp);
        }
    #endif
#endif
}

static void __trace_string(const char * pchString)
{
#if TRACE_USE_LIBC_PRINTF
    printf("%s", pchString);
#else
    TRACE_MCU_WRITE_STRING(pchString);
#endif
}

static void __trace_uint32_to_string(uint32_t wValue)
{
#if TRACE_USE_LIBC_PRINTF
    printf("%08x", wValue);
#else
    char buf[12];
    trace_fmt_hex32(wValue, buf);
    TRACE_MCU_WRITE_STRING(buf);
#endif
}

static void __trace_int32_to_string(int32_t nValue)
{
#if TRACE_USE_LIBC_PRINTF
    printf("%d", nValue);
#else
    char buf[12];
    trace_fmt_int32(nValue, buf);
    TRACE_MCU_WRITE_STRING(buf);
#endif
}

static void __trace_uint16_to_string(uint16_t hwValue)
{
#if TRACE_USE_LIBC_PRINTF
    printf("%04x", hwValue);
#else
    char buf[8];
    trace_fmt_hex16(hwValue, buf);
    TRACE_MCU_WRITE_STRING(buf);
#endif
}

static void __trace_int16_to_string(int16_t iValue)
{
#if TRACE_USE_LIBC_PRINTF
    printf("%d", iValue);
#else
    char buf[8];
    trace_fmt_int32((int32_t)iValue, buf);
    TRACE_MCU_WRITE_STRING(buf);
#endif
}

static void __trace_uint8_to_string(uint8_t chValue)
{
#if TRACE_USE_LIBC_PRINTF
    printf("%02x", chValue);
#else
    char buf[6];
    trace_fmt_hex8(chValue, buf);
    TRACE_MCU_WRITE_STRING(buf);
#endif
}

static void __trace_int8_to_string(int8_t cValue)
{
#if TRACE_USE_LIBC_PRINTF
    printf("%d", cValue);
#else
    char buf[6];
    trace_fmt_int32((int32_t)cValue, buf);
    TRACE_MCU_WRITE_STRING(buf);
#endif
}

static void __trace_float_to_string(float fValue)
{
#if TRACE_USE_LIBC_PRINTF
    printf("%f", fValue);
#else
    char buf[24];
    trace_fmt_float((double)fValue, buf, 4);
    TRACE_MCU_WRITE_STRING(buf);
#endif
}

static void __trace_double_to_string(double dfValue)
{
#if TRACE_USE_LIBC_PRINTF
    printf("%f", dfValue);
#else
    char buf[24];
    trace_fmt_float(dfValue, buf, 4);
    TRACE_MCU_WRITE_STRING(buf);
#endif
}

/*============================ STREAM OUTPUT =================================*/

#if TRACE_USE_LIBC_PRINTF

#define __OUTPUT_STREAM(__TYPE, __ADDR, __SIZE, __FORMAT_STR, __BLANK)          \
    do {                                                                        \
        int_fast8_t n = 0;                                                      \
        uint_fast16_t __Size = (__SIZE);                                        \
        __TYPE *pSrc = (__TYPE *)(__ADDR);                                      \
        uint8_t *pchSrc;                                                        \
        while(__Size > (TRACE_DISPLAY_WIDTH / sizeof(__TYPE))) {                \
            for (n = 0; n < (TRACE_DISPLAY_WIDTH / sizeof(__TYPE)); n++) {      \
                printf(__FORMAT_STR, pSrc[n]);                                  \
            }                                                                   \
                                                                                \
            printf("\t");                                                       \
            pchSrc = (uint8_t *)pSrc;                                           \
                                                                                \
            for (n = 0; n < TRACE_DISPLAY_WIDTH; n++) {                         \
                char c = pchSrc[n];                                             \
                if (c >= 127 || c < 32) {                                       \
                    c = '.';                                                    \
                }                                                               \
                printf("%c", c);                                                \
            }                                                                   \
                                                                                \
            printf("\r\n");                                                     \
                                                                                \
            pSrc += (TRACE_DISPLAY_WIDTH / sizeof(__TYPE));                     \
            __Size -= (TRACE_DISPLAY_WIDTH / sizeof(__TYPE));                   \
        }                                                                       \
                                                                                \
        if (__Size > 0) {                                                       \
            for (n = 0; n < __Size; n++) {                                      \
                printf(__FORMAT_STR, pSrc[n]);                                  \
            }                                                                   \
            for (   n = 0;                                                      \
                    n < (TRACE_DISPLAY_WIDTH/sizeof(__TYPE) - __Size);          \
                    n++) {                                                      \
                printf(__BLANK);                                                \
            }                                                                   \
            printf("\t");                                                       \
            pchSrc = (uint8_t *)pSrc;                                           \
            for (n = 0; n < __Size * sizeof(__TYPE); n++) {                     \
                char c = pchSrc[n];                                             \
                if (c > 127 || c < 32) {                                        \
                    c = '.';                                                    \
                }                                                               \
                printf("%c", c);                                                \
            }                                                                   \
                                                                                \
            printf("\r\n");                                                     \
        }                                                                       \
    } while(0)

#else /* MCU lightweight path */

#define __OUTPUT_STREAM_MCU(__TYPE, __ADDR, __SIZE, __FMT_FN)                  \
    do {                                                                        \
        uint_fast16_t __Size = (__SIZE);                                        \
        __TYPE *pSrc = (__TYPE *)(__ADDR);                                      \
        char buf[12];                                                           \
        for (uint_fast16_t i = 0; i < __Size; i++) {                           \
            __FMT_FN(pSrc[i], buf);                                            \
            TRACE_MCU_WRITE_STRING(buf);                                       \
            TRACE_MCU_WRITE_STRING(" ");                                       \
        }                                                                       \
        TRACE_MCU_WRITE_STRING("\r\n");                                        \
    } while(0)

#endif /* TRACE_USE_LIBC_PRINTF */

static void __trace_word_stream(uint32_t *pwStream, uint_fast16_t hwSize)
{
#if TRACE_USE_LIBC_PRINTF
    __OUTPUT_STREAM(uint32_t, pwStream, hwSize, "%08X ", "         ");
#else
    __OUTPUT_STREAM_MCU(uint32_t, pwStream, hwSize, trace_fmt_hex32);
#endif
}

static void __trace_hword_stream(uint16_t *phwStream, uint_fast16_t hwSize)
{
#if TRACE_USE_LIBC_PRINTF
    __OUTPUT_STREAM(uint16_t, phwStream, hwSize, "%04X ", "     ");
#else
    __OUTPUT_STREAM_MCU(uint16_t, phwStream, hwSize, trace_fmt_hex16);
#endif
}

static void __trace_byte_stream(uint8_t *pchStream, uint_fast16_t hwSize)
{
#if TRACE_USE_LIBC_PRINTF
    __OUTPUT_STREAM(uint8_t, pchStream, hwSize, "%02X ", "   ");
#else
    __OUTPUT_STREAM_MCU(uint8_t, pchStream, hwSize, trace_fmt_hex8);
#endif
}

/* EOF */