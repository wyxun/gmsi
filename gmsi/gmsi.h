#ifndef __GMSI_H__
#define __GMSI_H__

// gmsi库所有头文件
#include "gbase.h"
#include "global_define.h"
#include "utilities/util_debug.h"
#include "gcoroutine.h"
#include "glog.h"
#include "gstorage.h"

typedef struct {
    gstorage_data_t *ptData;
}gmsi_t;

void gmsi_Init(gmsi_t *ptGmsi);

void gmsi_Clock(void);
void gmsi_Run(void);

#endif