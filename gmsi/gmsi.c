#include <stdint.h>
#include "gmsi.h"
//! \name GMSI Interface Type
//! @{
//! general purpose interface for normal MCU applicatoin
#define GENERAL_PURPOSE                 0           
//! @}

#define GMSI_PURPOSE                     GENERAL_PURPOSE

/*! \brief interface version
 *! \note Change this when new generation of interface is released
 */
#define GMSI_INTERFACE_VERSION           1

/*! \brief major version for specified gsf interface
 *! \note major version is rarely changed
 */
#define GMSI_MAJOR_VERSION               1

/*! \brief minor version for normal maintaince
 *! \note update this for function update and bug fixing
 */
#define GMSI_MINOR_VERSION               0

/*! \brief GSF version 
 *         (InterfaceType, InterfaceVersion, MajorVersion, MinorVersion)
 */
#define GMSI_VERSION                 {                                      \
                                        GMSI_PURPOSE,                       \
                                        GMSI_INTERFACE_VERSION,             \
                                        GMSI_MAJOR_VERSION,                 \
                                        GMSI_MINOR_VERSION,                 \
                                    }

const struct {
    uint8_t chPurpose;          //!< software framework purpose
    uint8_t chInterface;        //!< interface version
    uint8_t chMajor;            //!< major version
    uint8_t chMinor;            //!< minor version
} GMSIVersion = GMSI_VERSION;


static int rti_start(void)
{
    return 0;
}
INIT_EXPORT(rti_start, "0");

static int rti_board_start(void)
{
    return 0;
}
INIT_EXPORT(rti_board_start, "0.end");

static int rti_board_end(void)
{
    return 0;
}
INIT_EXPORT(rti_board_end, "1.end");

static int rti_init_end(void)
{
    return 0;
}
INIT_EXPORT(rti_init_end, "6.end");

static int rti_autorun_start(void *ptObject)
{
    return 0;
}
static int rti_autorun_end(void *ptObject)
{
    
    return 0;
}

int gmsi_components_board_init(void)
{
    const gmsi_fn_t *fn_ptr;
 
    for (fn_ptr = &__gmsi_init_rti_board_start; fn_ptr < &__gmsi_init_rti_board_end; fn_ptr++)
    {
        (*fn_ptr)();
    }
    
    return 0;
}
