/**
 * @file trace_stub.c
 * @brief Empty implementation of plooc Trace interface
 */

#include "utilities/trace.h"

/* Empty implementation functions */
static void trace_init_stub(trace_cfg_t *ptCFG) { (void)ptCFG; }
static void trace_double_stub(double dfValue) { (void)dfValue; }
static void trace_float_stub(float fValue) { (void)fValue; }
static void trace_uint32_stub(uint32_t wValue) { (void)wValue; }
static void trace_int32_stub(int32_t nValue) { (void)nValue; }
static void trace_uint16_stub(uint16_t hwValue) { (void)hwValue; }
static void trace_int16_stub(int16_t iValue) { (void)iValue; }
static void trace_uint8_stub(uint8_t chValue) { (void)chValue; }
static void trace_int8_stub(int8_t cValue) { (void)cValue; }
static void trace_string_stub(const char * pchString) { (void)pchString; }
static void trace_words_stub(uint32_t *pwStream, uint_fast16_t hwSize) { (void)pwStream; (void)hwSize; }
static void trace_hwords_stub(uint16_t *phwStream, uint_fast16_t hwSize) { (void)phwStream; (void)hwSize; }
static void trace_bytes_stub(uint8_t *pchStream, uint_fast16_t hwSize) { (void)pchStream; (void)hwSize; }

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
