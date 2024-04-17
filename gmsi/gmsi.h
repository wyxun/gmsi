#ifndef __GMSI_H__
#define __GMSI_H__

// gmsi库所有头文件
#include "gbase.h"
#include "global_define.h"
#include "utilities/util_debug.h"
#include "gcoroutine.h"
#include "glog.h"
#include "gstorage.h"

void gmsi_Init(void);
void gmsi_Clock(void);
void gmsi_Run(void);

#endif