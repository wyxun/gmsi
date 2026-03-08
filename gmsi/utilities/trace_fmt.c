/**
 * @file trace_fmt.c
 * @brief Lightweight number-to-string conversion for MCU trace output
 *        (no stdio dependency, pure integer arithmetic)
 *
 * This file is compiled to empty when TRACE_USE_LIBC_PRINTF == 1 (PC builds).
 */

#include "trace_fmt.h"

/* Only compile on MCU targets (non-printf path) */
#ifndef TRACE_USE_LIBC_PRINTF
#   if defined(_MSC_VER) || defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#       define TRACE_USE_LIBC_PRINTF    1
#   else
#       define TRACE_USE_LIBC_PRINTF    0
#   endif
#endif

#if !TRACE_USE_LIBC_PRINTF

/*============================ IMPLEMENTATION ================================*/

static const char s_chHexDigits[] = "0123456789ABCDEF";

/*----------------------------------------------------------------------------*
 * Hex conversion (shift + lookup, no division)                               *
 *----------------------------------------------------------------------------*/

char *trace_fmt_hex32(uint32_t wValue, char *pchBuf)
{
    pchBuf[0] = '0';
    pchBuf[1] = 'x';
    for (int_fast8_t i = 7; i >= 0; i--) {
        pchBuf[2 + (7 - i)] = s_chHexDigits[(wValue >> (i * 4)) & 0x0F];
    }
    pchBuf[10] = '\0';
    return pchBuf;
}

char *trace_fmt_hex16(uint16_t hwValue, char *pchBuf)
{
    pchBuf[0] = '0';
    pchBuf[1] = 'x';
    for (int_fast8_t i = 3; i >= 0; i--) {
        pchBuf[2 + (3 - i)] = s_chHexDigits[(hwValue >> (i * 4)) & 0x0F];
    }
    pchBuf[6] = '\0';
    return pchBuf;
}

char *trace_fmt_hex8(uint8_t chValue, char *pchBuf)
{
    pchBuf[0] = '0';
    pchBuf[1] = 'x';
    pchBuf[2] = s_chHexDigits[(chValue >> 4) & 0x0F];
    pchBuf[3] = s_chHexDigits[chValue & 0x0F];
    pchBuf[4] = '\0';
    return pchBuf;
}

/*----------------------------------------------------------------------------*
 * Decimal conversion (mod 10 + reverse)                                      *
 *----------------------------------------------------------------------------*/

char *trace_fmt_int32(int32_t nValue, char *pchBuf)
{
    char *p = pchBuf;
    uint32_t uValue;
    int_fast8_t chLen = 0;

    if (nValue < 0) {
        *p++ = '-';
        /* handle INT32_MIN safely: -(int32_t)(-2147483648) overflows */
        uValue = (uint32_t)(-(nValue + 1)) + 1U;
    } else {
        uValue = (uint32_t)nValue;
    }

    /* generate digits in reverse order */
    char chTmp[10];
    do {
        chTmp[chLen++] = '0' + (char)(uValue % 10U);
        uValue /= 10U;
    } while (uValue > 0U);

    /* reverse copy */
    for (int_fast8_t i = chLen - 1; i >= 0; i--) {
        *p++ = chTmp[i];
    }
    *p = '\0';

    return pchBuf;
}

/*----------------------------------------------------------------------------*
 * Float conversion (integer split method)                                    *
 *----------------------------------------------------------------------------*/

char *trace_fmt_float(double dfValue, char *pchBuf, uint8_t chDecimals)
{
    char *p = pchBuf;

    /* clamp decimals to 1-6 */
    if (chDecimals < 1U) chDecimals = 1U;
    if (chDecimals > 6U) chDecimals = 6U;

    /* handle negative */
    if (dfValue < 0.0) {
        *p++ = '-';
        dfValue = -dfValue;
    }

    /* compute multiplier: 10^chDecimals */
    uint32_t wMul = 1U;
    for (uint8_t i = 0; i < chDecimals; i++) {
        wMul *= 10U;
    }

    /* round: add 0.5 * 10^(-chDecimals) */
    dfValue += 0.5 / (double)wMul;

    /* split integer and fractional parts */
    uint32_t wIntPart  = (uint32_t)dfValue;
    uint32_t wFracPart = (uint32_t)((dfValue - (double)wIntPart) * (double)wMul);

    /* print integer part */
    trace_fmt_int32((int32_t)wIntPart, p);
    while (*p) p++;

    /* decimal point */
    *p++ = '.';

    /* print fractional part with leading zeros */
    for (int_fast8_t i = (int_fast8_t)chDecimals - 1; i >= 0; i--) {
        uint32_t wDivisor = 1U;
        for (int_fast8_t j = 0; j < i; j++) {
            wDivisor *= 10U;
        }
        *p++ = '0' + (char)((wFracPart / wDivisor) % 10U);
    }
    *p = '\0';

    return pchBuf;
}

#endif /* !TRACE_USE_LIBC_PRINTF */
/* EOF */
