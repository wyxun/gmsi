#ifndef __GMSI_H__
#define __GMSI_H__

// Include necessary libraries
#include "gbase.h"
#include "global_define.h"
#include "utilities/util_debug.h"
#include "gcoroutine.h"
#include "glog.h"
#include "gstorage.h"

// Define types
typedef struct {
    gstorage_data_t *ptData;
}gmsi_t;

// Function prototypes
void gmsi_Init(gmsi_t *ptGmsi);
void gmsi_Clock(void);
void gmsi_Run(void);

#endif  // __GMSI_H__