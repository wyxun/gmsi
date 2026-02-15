/**
 * @file trace_stub.c
 * @brief Empty implementation of plooc Trace interface
 */

#include "utilities/trace.h"

/* SEGGER RTT Implementation */
#include "utilities/segger_rtt/SEGGER_RTT.h"
#include <stdio.h>

static void trace_init_stub(trace_cfg_t *ptCFG) { 
    (void)ptCFG; 
    SEGGER_RTT_Init();
}

static void trace_double_stub(double dfValue) { 
    char buf[64];
    snprintf(buf, sizeof(buf), "%f", dfValue);
    SEGGER_RTT_WriteString(0, buf);
}

static void trace_float_stub(float fValue) { 
    char buf[64];
    snprintf(buf, sizeof(buf), "%f", (double)fValue);
    SEGGER_RTT_WriteString(0, buf);
}

static void trace_uint32_stub(uint32_t wValue) { 
    char buf[32];
    snprintf(buf, sizeof(buf), "0x%08X", wValue);
    SEGGER_RTT_WriteString(0, buf);
}

static void trace_int32_stub(int32_t nValue) { 
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", nValue);
    SEGGER_RTT_WriteString(0, buf);
}

static void trace_uint16_stub(uint16_t hwValue) { 
    char buf[16];
    snprintf(buf, sizeof(buf), "0x%04X", hwValue);
    SEGGER_RTT_WriteString(0, buf);
}

static void trace_int16_stub(int16_t iValue) { 
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", iValue);
    SEGGER_RTT_WriteString(0, buf);
}

static void trace_uint8_stub(uint8_t chValue) { 
    char buf[8];
    snprintf(buf, sizeof(buf), "0x%02X", chValue);
    SEGGER_RTT_WriteString(0, buf);
}

static void trace_int8_stub(int8_t cValue) { 
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", cValue);
    SEGGER_RTT_WriteString(0, buf);
}

static void trace_string_stub(const char * pchString) { 
    SEGGER_RTT_WriteString(0, pchString);
}

static void trace_words_stub(uint32_t *pwStream, uint_fast16_t hwSize) {
    if (NULL == pwStream || 0 == hwSize) return;
    for (uint_fast16_t i = 0; i < hwSize; i++) {
        trace_uint32_stub(pwStream[i]);
        SEGGER_RTT_WriteString(0, " ");
    }
}

static void trace_hwords_stub(uint16_t *phwStream, uint_fast16_t hwSize) {
    if (NULL == phwStream || 0 == hwSize) return;
    for (uint_fast16_t i = 0; i < hwSize; i++) {
        trace_uint16_stub(phwStream[i]);
        SEGGER_RTT_WriteString(0, " ");
    }
}

static void trace_bytes_stub(uint8_t *pchStream, uint_fast16_t hwSize) {
    if (NULL == pchStream || 0 == hwSize) return;
    for (uint_fast16_t i = 0; i < hwSize; i++) {
        trace_uint8_stub(pchStream[i]);
        SEGGER_RTT_WriteString(0, " ");
    }
}

/* TRACE instance */
const i_trace_t TRACE = {
    .Init = trace_init_stub,
    .ToString = {
        .Double = trace_double_stub,
        .Float = trace_float_stub,
        .UInt32 = trace_uint32_stub,
        .Int32 = trace_int32_stub,
        .UInt16 = trace_uint16_stub,
        .Int16 = trace_int16_stub,
        .UInt8 = trace_uint8_stub,
        .Int8 = trace_int8_stub,
        .String = trace_string_stub,
        .Words = trace_words_stub,
        .HWords = trace_hwords_stub,
        .Bytes = trace_bytes_stub,
    },
};
