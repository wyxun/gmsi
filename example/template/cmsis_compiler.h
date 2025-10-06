#ifndef __CMSIS_COMPILER_H  
#define __CMSIS_COMPILER_H  
  
// 编译器检测  
#if defined(__GNUC__)  
    #define __STATIC_INLINE static inline  
    #define __INLINE inline
    #define __WEAK __attribute__((weak))
#elif defined(_MSC_VER)  
    #define __STATIC_INLINE static __inline  
    #define __INLINE __inline
    #define __WEAK __declspec(selectany)
#else  
    #define __STATIC_INLINE static  
    #define __INLINE
    #define __WEAK
#endif  
  
// 空实现的内存屏障(PC 端可能不需要)  
#define __DSB()   
#define __ISB()  
  
// 中断控制(PC 端可以为空或使用线程同步)  
static inline uint32_t __get_PRIMASK(void) { return 0; }  
static inline void __set_PRIMASK(uint32_t priMask) { (void)priMask; }  
static inline void __disable_irq(void) {}  
static inline void __enable_irq(void) {}  
  
#endif