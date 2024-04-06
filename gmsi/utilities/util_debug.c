#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ABS(__N)    ((__N) < 0 ? -(__N) : (__N))
#define _BV(__N)    ((uint32_t)1<<(__N))
#define TOP         (0x0FFF)

void (*pfcnLedSet)(bool bStatus);
/*! \brief set the 16-level led gradation
 *! \param hwLevel gradation
 *! \return none
 */
static void set_led_gradation(uint16_t hwLevel)
{
    static uint16_t s_hwCounter = 0;
    
    if (hwLevel >= s_hwCounter) {
        pfcnLedSet(0);
    } else {
        pfcnLedSet(1);
    }
    
    s_hwCounter++;
    s_hwCounter &= TOP;
}

int utildebug_LedBreathe(void *ptVoid) 
{
    static uint16_t s_hwCounter = 0;
    static int16_t s_nGray = (TOP >> 1);
    
    s_hwCounter++;
    if (!(s_hwCounter & (_BV(11)-1))) {
        s_nGray++; 
        if (s_nGray == TOP) {
            s_nGray = 0;
        }
    }
    
    set_led_gradation(ABS(s_nGray - (TOP >> 1)));
    return 0;
}

void utildebug_LedInit(void (*fcnLedSet)(bool bStatus))
{
    pfcnLedSet = fcnLedSet;
}


