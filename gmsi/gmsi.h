#ifndef __GMSI_H__
#define __GMSI_H__

// Include necessary libraries
#include "gbase.h"
#include "global_define.h"
#include "utilities/util_debug.h"
#include "gcoroutine.h"
#include "glog.h"
#include "gstorage.h"

#undef this
#define this    (*ptThis)

// Define types
typedef struct {
    gstorage_data_t *ptData;
}gmsi_t;

#if defined(__clang__) || defined(__ARMCOMPILER_VERSION) || defined(__ARMCC_VERSION)
  #if defined(__has_attribute)
    #if __has_attribute(section)
      #define INIT_SECTION __attribute__((section("init_infos"), used))
    #else
      #define INIT_SECTION __attribute__((section("init_infos"), used))
    #endif
  #else
    /* assume section attribute exists on clang/armclang when __has_attribute is not available */
    #define INIT_SECTION __attribute__((section("init_infos"), used))
  #endif
#elif defined(__GNUC__)
  #define INIT_SECTION __attribute__((section("init_infos"), used))
#else
  #define INIT_SECTION
#endif

typedef int (*init_func_t)(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);
typedef struct {
    init_func_t pfcnInitFunc;
    uintptr_t wObjectAddr;
    uintptr_t wConfigAddr;
} gmsi_init_info_t;

#define GMSI_DECLARE_OBJECT(object_type, object_name, ...)                \
    object_type##_cfg_t t##object_name##Cfg = { __VA_ARGS__ };            \
    object_type##_t t##object_name;                                       \
    INIT_SECTION const gmsi_init_info_t init_info_##object_name = {       \
        .pfcnInitFunc = object_type##_Init,                               \
        .wObjectAddr = (uintptr_t)&t##object_name,                        \
        .wConfigAddr = (uintptr_t)&t##object_name##Cfg                    \
    };

// Function prototypes
void gmsi_Init(gmsi_t *ptGmsi);
void gmsi_Clock(void);
void gmsi_Run(void);

#endif  // __GMSI_H__