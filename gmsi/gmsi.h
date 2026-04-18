#ifndef __GMSI_H__
#define __GMSI_H__

// Include necessary libraries
#include "gbase.h"
#include "global_define.h"
#include "gdebug/util_debug.h"
#include "gcoroutine.h"
#include "glog.h"
/* NOTE: gstorage.h is intentionally NOT included here.
 * Users who declare gstorage objects must include gstorage.h themselves. */

#undef this
#define this    (*ptThis)

typedef struct {
    /**
     * @brief 默认 Flash 设备，供 gstorage 使用（可选）。
     *
     * 赋值 HW.ptAppFlash 后 gmsi_Init 内部自动绑定到所有 gstorage 实例。
     * 若不使用 gstorage，保持 NULL 即可。
     * 使用 void* 以避免对 gdi_flash_t 类型的强依赖——
     * gstorage.c 内部负责转型。
     */
    void *ptAppFlash;
} gmsi_t;

#define GMSI_DECLARE_OBJECT(object_type, object_name, ...)                \
    object_type##_cfg_t t##object_name##Cfg = { __VA_ARGS__ };            \
    object_type##_t t##object_name;                                       \
    INIT_SECTION const gmsi_init_info_t init_info_##object_name = {       \
        .pfcnInitFunc = (init_func_t)object_type##_Init,                  \
        .wObjectAddr = (uintptr_t)&t##object_name,                        \
        .wConfigAddr = (uintptr_t)&t##object_name##Cfg                    \
    };

// Function prototypes
void gmsi_Init(gmsi_t *ptGmsi);
void gmsi_Clock(void);
void gmsi_Run(void);

#endif  // __GMSI_H__