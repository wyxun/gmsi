#if defined(__riscv)
/*============================================================================
 * RISC-V Bare-Metal Compiler-RT & libc Shims
 *
 * 编译器隐式生成的 builtin (__divdi3 等) 和基础 libc (memset 等),
 * 用于 -nostdlib 的 RISC-V bare-metal 目标。
 *
 * 通过定义 MODUS_NO_RISCV_SHIM 可禁用 (当 target 已有替代实现时)。
 *===========================================================================*/

#if !defined(MODUS_NO_RISCV_SHIM)

#include <stdint.h>
#include <stddef.h>

/*============================ LIBC Shims ======================================*/

void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

size_t strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return p - s;
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (n && (*d++ = *src++)) n--;
    while (n--) *d++ = '\0';
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    if (n == 0) return 0;
    while (n-- && *s1 && *s1 == *s2) {
        if (n == 0 || *s1 == '\0') break;
        s1++; s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

/*============================ COMPILER-RT Shims ================================*/

int64_t __divdi3(int64_t a, int64_t b) {
    if (b == 0) return 0;
    int sign = 1;
    if (a < 0) { a = -a; sign = -sign; }
    if (b < 0) { b = -b; sign = -sign; }
    uint64_t ua = a;
    uint64_t ub = b;
    uint64_t res = 0;
    for (int i = 63; i >= 0; i--) {
        if ((ua >> i) >= ub) {
            res |= (1ULL << i);
            ua -= (ub << i);
        }
    }
    return sign < 0 ? -((int64_t)res) : (int64_t)res;
}

uint64_t __lshrdi3(uint64_t a, int b) {
    if (b <= 0) return a;
    if (b >= 64) return 0;

    union {
        uint64_t val;
        struct { uint32_t low; uint32_t high; } words;
    } src, dst;

    src.val = a;
    if (b >= 32) {
        dst.words.low  = src.words.high >> (b - 32);
        dst.words.high = 0;
    } else {
        dst.words.low  = (src.words.low >> b) | (src.words.high << (32 - b));
        dst.words.high = src.words.high >> b;
    }
    return dst.val;
}

uint64_t __ashldi3(uint64_t a, int b) {
    if (b <= 0) return a;
    if (b >= 64) return 0;

    union {
        uint64_t val;
        struct { uint32_t low; uint32_t high; } words;
    } src, dst;

    src.val = a;
    if (b >= 32) {
        dst.words.high = src.words.low << (b - 32);
        dst.words.low  = 0;
    } else {
        dst.words.high = (src.words.high << b) | (src.words.low >> (32 - b));
        dst.words.low  = src.words.low << b;
    }
    return dst.val;
}

double __extendsfdf2(float a) {
    union { float f; uint32_t u; } src;
    union { double d; uint64_t u; } dst;
    src.f = a;
    uint32_t sign = src.u & 0x80000000UL;
    uint32_t exp  = src.u & 0x7F800000UL;
    uint32_t frac = src.u & 0x007FFFFFUL;

    if (exp == 0 && frac == 0) {
        dst.u = ((uint64_t)sign) << 32;
        return dst.d;
    }

    uint64_t d_sign = ((uint64_t)sign) << 32;
    uint64_t d_exp  = ((((uint64_t)(exp >> 23)) - 127 + 1023) & 0x7FF) << 52;
    uint64_t d_frac = ((uint64_t)frac) << 29;

    dst.u = d_sign | d_exp | d_frac;
    return dst.d;
}

float __truncdfsf2(double a) {
    union { double d; uint64_t u; } src;
    union { float f; uint32_t u; } dst;
    src.d = a;
    uint64_t sign = src.u & 0x8000000000000000ULL;
    uint64_t exp  = src.u & 0x7FF0000000000000ULL;
    uint64_t frac = src.u & 0x000FFFFFFFFFFFFFULL;

    if (exp == 0 && frac == 0) {
        dst.u = (uint32_t)(sign >> 32);
        return dst.f;
    }

    uint32_t f_sign = (uint32_t)(sign >> 32);
    int32_t  real_exp = (int32_t)(exp >> 52) - 1023;
    uint32_t f_exp;
    if (real_exp < -126) {
        f_exp = 0;
    } else if (real_exp > 127) {
        f_exp = 0xFF << 23;
    } else {
        f_exp = (uint32_t)((real_exp + 127) & 0xFF) << 23;
    }
    uint32_t f_frac = (uint32_t)(frac >> 29);

    dst.u = f_sign | f_exp | f_frac;
    return dst.f;
}

float __mulsf3(float a, float b) {
    (void)a; (void)b;
    return 0.0f;
}

int32_t __fixsfsi(float a) {
    (void)a;
    return 0;
}

int __ltdf2(double a, double b) {
    (void)a; (void)b;
    return 0;
}

double __floatunsidf(uint32_t a) {
    (void)a;
    return 0.0;
}

double __divdf3(double a, double b) {
    (void)a; (void)b;
    return 0.0;
}

double __adddf3(double a, double b) {
    (void)a; (void)b;
    return 0.0;
}

uint32_t __fixunsdfsi(double a) {
    (void)a;
    return 0;
}

double __subdf3(double a, double b) {
    (void)a; (void)b;
    return 0.0;
}

double __muldf3(double a, double b) {
    (void)a; (void)b;
    return 0.0;
}

float __floatunsisf(uint32_t a) {
    (void)a;
    return 0.0f;
}

float __divsf3(float a, float b) {
    (void)a; (void)b;
    return 0.0f;
}

__attribute__((optnone)) int __nesf2(float a, float b) {
    union { float f; uint32_t u; } ua, ub;
    ua.f = a;
    ub.f = b;

    uint32_t exp_a = ua.u & 0x7F800000UL;
    uint32_t frac_a = ua.u & 0x007FFFFFUL;
    uint32_t exp_b = ub.u & 0x7F800000UL;
    uint32_t frac_b = ub.u & 0x007FFFFFUL;

    if ((exp_a == 0x7F800000UL && frac_a != 0) ||
        (exp_b == 0x7F800000UL && frac_b != 0)) {
        return 1;
    }

    if (((ua.u & 0x7FFFFFFFUL) == 0) && ((ub.u & 0x7FFFFFFFUL) == 0)) {
        return 0;
    }

    return (ua.u != ub.u) ? 1 : 0;
}

__attribute__((optnone)) int __eqdf2(double a, double b) {
    union { double d; uint64_t u; } ua, ub;
    ua.d = a;
    ub.d = b;

    uint64_t exp_a = ua.u & 0x7FF0000000000000ULL;
    uint64_t frac_a = ua.u & 0x000FFFFFFFFFFFFFULL;
    uint64_t exp_b = ub.u & 0x7FF0000000000000ULL;
    uint64_t frac_b = ub.u & 0x000FFFFFFFFFFFFFULL;

    if ((exp_a == 0x7FF0000000000000ULL && frac_a != 0) ||
        (exp_b == 0x7FF0000000000000ULL && frac_b != 0)) {
        return 1;
    }

    if (((ua.u & 0x7FFFFFFFFFFFFFFFULL) == 0) && ((ub.u & 0x7FFFFFFFFFFFFFFFULL) == 0)) {
        return 0;
    }

    return (ua.u == ub.u) ? 0 : 1;
}

#endif /* !MODUS_NO_RISCV_SHIM */
#endif /* defined(__riscv) */
