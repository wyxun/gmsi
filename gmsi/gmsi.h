#ifndef __GMSI_H__
#define __GMSI_H__

// gmsi库所有头文件
#include "state_machine.h"
#include "gbase.h"
#include "global_define.h"
#include "utilities/util_debug.h"

#ifdef LINUX_POSIX
#include <assert.h>
#endif

void gmsi_Init(void);
void gmsi_Clock(void);
void gmsi_Run(void);
void gmsi_LogPrintf(const char *pString);

#endif