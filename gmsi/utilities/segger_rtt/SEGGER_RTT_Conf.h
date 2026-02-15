#ifndef SEGGER_RTT_CONF_H
#define SEGGER_RTT_CONF_H

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   SEGGER_RTT_MAX_NUM_UP_BUFFERS
  #define SEGGER_RTT_MAX_NUM_UP_BUFFERS             (3)     // Max. number of up-buffers (T->H) available on this target
#endif

#ifndef   SEGGER_RTT_MAX_NUM_DOWN_BUFFERS
  #define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS           (3)     // Max. number of down-buffers (H->T) available on this target
#endif

#ifndef   BUFFER_SIZE_UP
  #define BUFFER_SIZE_UP                            (1024)  // Size of the buffer for terminal output of target, up to host
#endif

#ifndef   BUFFER_SIZE_DOWN
  #define BUFFER_SIZE_DOWN                          (16)    // Size of the buffer for terminal input to target from host (Usually keyboard input)
#endif

#ifndef   SEGGER_RTT_PRINTF_BUFFER_SIZE
  #define SEGGER_RTT_PRINTF_BUFFER_SIZE             (64u)    // Size of buffer for RTT printf to bulk-send chars via RTT
#endif

#ifndef   SEGGER_RTT_MODE_DEFAULT
  #define SEGGER_RTT_MODE_DEFAULT                   SEGGER_RTT_MODE_NO_BLOCK_SKIP
#endif

#endif // SEGGER_RTT_CONF_H
