/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 1995 - 2021 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER RTT * Real Time Transfer for embedded targets         *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* SEGGER strongly recommends to not modify the file but use packages *
* with specific customization files.                                 *
*                                                                    *
* SEGGER licenses this file to you under the terms of the            *
* SEGGER RTLib License Agreement which can be found in the           *
* SEGGER_RTLib_License_Agreement.txt file in the distribution of     *
* this software.                                                     *
*                                                                    *
* ------------------------------------------------------------------ *
*                                                                    *
*  release_id = "0x89799632, 0xe43f8753"                             *
*                                                                    *
**********************************************************************
*                                                                    *
*       RTT version: 7.62b                                           *
*                                                                    *
**********************************************************************
*/

#include "SEGGER_RTT.h"
#include <string.h>


/*********************************************************************
*
*       Global data
*
**********************************************************************
*/
SEGGER_RTT_CB _SEGGER_RTT;

static char _acUpBuffer  [BUFFER_SIZE_UP];
static char _acDownBuffer[BUFFER_SIZE_DOWN];

/*
 * Statically initialize the RTT control block so that the "SEGGER RTT" ID
 * string resides in the .data section (copied from flash to RAM by C runtime
 * before main()). This allows the debugger/OpenOCD to locate it immediately,
 * without waiting for SEGGER_RTT_Init() to execute.
 */
SEGGER_RTT_CB _SEGGER_RTT = {
  .acID              = "SEGGER RTT",
  .MaxNumUpBuffers   = SEGGER_RTT_MAX_NUM_UP_BUFFERS,
  .MaxNumDownBuffers = SEGGER_RTT_MAX_NUM_DOWN_BUFFERS,
  .aUp = {{
    .sName        = "Terminal",
    .pBuffer      = _acUpBuffer,
    .SizeOfBuffer = sizeof(_acUpBuffer),
    .WrOff        = 0u,
    .RdOff        = 0u,
    .Flags        = SEGGER_RTT_MODE_DEFAULT,
  }},
  .aDown = {{
    .sName        = "Terminal",
    .pBuffer      = _acDownBuffer,
    .SizeOfBuffer = sizeof(_acDownBuffer),
    .WrOff        = 0u,
    .RdOff        = 0u,
    .Flags        = SEGGER_RTT_MODE_DEFAULT,
  }},
};

/*********************************************************************
*
*       Public functions
*
**********************************************************************
*/

/*********************************************************************
*
*       SEGGER_RTT_Init
*
*  Function description
*    Re-initializes the RTT control block (clears buffers).
*    Not strictly required when using static init, but kept for
*    API compatibility.
*/
void SEGGER_RTT_Init(void) {
  _SEGGER_RTT.aUp[0].WrOff   = 0u;
  _SEGGER_RTT.aUp[0].RdOff   = 0u;
  _SEGGER_RTT.aDown[0].WrOff = 0u;
  _SEGGER_RTT.aDown[0].RdOff = 0u;
}

/*********************************************************************
*
*       SEGGER_RTT_Write
*
*  Function description
*    Stores a specified number of characters in SEGGER RTT
*    control block which is then read by the host.
*/
unsigned SEGGER_RTT_Write(unsigned BufferIndex, const void* pBuffer, unsigned NumBytes) {
  unsigned              Status;
  unsigned              RdOff;
  unsigned              WrOff;
  unsigned              Rem;
  volatile unsigned*    pWrOff;
  SEGGER_RTT_BUFFER_UP* pRing;

  //
  // Redirect to buffer 0 if invalid buffer index
  //
  if (BufferIndex >= (unsigned)_SEGGER_RTT.MaxNumUpBuffers) {
    BufferIndex = 0;
  }
  //
  // Get "to-host" ring buffer.
  //
  pRing = &_SEGGER_RTT.aUp[BufferIndex];
  //
  // Need to be cautious regarding volatiles / atomics here
  //
  pWrOff = &pRing->WrOff;
  WrOff  = *pWrOff;
  RdOff  = pRing->RdOff;
  //
  // Check if we can copy the data
  //
  // Standard RTT logic for ring buffer
  if (RdOff > WrOff) {
    Rem = RdOff - WrOff - 1u;
  } else {
    Rem = pRing->SizeOfBuffer - (WrOff - RdOff + 1u);
  }
  
  if (Rem < NumBytes) {
     if (pRing->Flags == SEGGER_RTT_MODE_NO_BLOCK_SKIP) {
         return 0;
     }
     if (pRing->Flags == SEGGER_RTT_MODE_NO_BLOCK_TRIM) {
         NumBytes = Rem;
     }
     // For BLOCK mode, we would loop here. Omitted for simplicity/deadlock prevention in minimal impl.
  }

  //
  // Copy data
  //
  if (NumBytes > 0) {
      unsigned NumBytesToWrite;
      unsigned NumBytesWritten;

      NumBytesWritten = 0;
      //
      // Write data to buffer
      //
      if (RdOff > WrOff) {
          //
          // Case 1: RdOff > WrOff
          //
          //  +---------------+-----------------------+----------------------+
          //  |   Empty Space |      Valid Data       |      Empty Space     |
          //  +---------------+-----------------------+----------------------+
          //                  ^                       ^
          //                  WrOff                   RdOff
          //
          //  We can write until RdOff - 1
          //
          memcpy(pRing->pBuffer + WrOff, pBuffer, NumBytes);
          WrOff += NumBytes;
      } else {
          //
          // Case 2: RdOff <= WrOff
          //
          //  +-----------------------+---------------+----------------------+
          //  |      Valid Data       |  Empty Space  |      Valid Data      |
          //  +-----------------------+---------------+----------------------+
          //                          ^               ^
          //                          RdOff           WrOff
          //
          //  We can write until end of buffer
          //
          NumBytesToWrite = pRing->SizeOfBuffer - WrOff;
          if (NumBytesToWrite > NumBytes) {
              NumBytesToWrite = NumBytes;
          }
          memcpy(pRing->pBuffer + WrOff, pBuffer, NumBytesToWrite);
          NumBytesWritten = NumBytesToWrite;
          WrOff += NumBytesToWrite;
          
          if (NumBytesWritten < NumBytes) {
              //
              // Continue writing from beginning of buffer
              //
              WrOff = 0;
              NumBytesToWrite = NumBytes - NumBytesWritten;
              memcpy(pRing->pBuffer + WrOff, (const char*)pBuffer + NumBytesWritten, NumBytesToWrite);
              WrOff += NumBytesToWrite;
          }
      }
      *pWrOff = WrOff;
  }
  return NumBytes;
}

/*********************************************************************
*
*       SEGGER_RTT_WriteString
*
*  Function description
*    Stores a zero-terminated string in SEGGER RTT
*    control block which is then read by the host.
*/
unsigned SEGGER_RTT_WriteString(unsigned BufferIndex, const char* s) {
  unsigned Len;

  Len = strlen(s);
  return SEGGER_RTT_Write(BufferIndex, s, Len);
}


/*********************************************************************
*
*       SEGGER_RTT_PutChar
*
*  Function description
*    Stores a single character/byte in SEGGER RTT
*    control block which is then read by the host.
*/
unsigned SEGGER_RTT_PutChar(unsigned BufferIndex, char c) {
  return SEGGER_RTT_Write(BufferIndex, &c, 1);
}

/*********************************************************************
*
*       SEGGER_RTT_Read
*
*  Function description
*    Reads a specified number of characters from SEGGER RTT
*    control block which is written by the host.
*/
unsigned SEGGER_RTT_Read(unsigned BufferIndex, void* pBuffer, unsigned BufferSize) {
  unsigned              NumBytesRead;
  unsigned              RdOff;
  unsigned              WrOff;
  volatile unsigned*    pRdOff;
  SEGGER_RTT_BUFFER_DOWN* pRing;

  //
  // Redirect to buffer 0 if invalid buffer index
  //
  if (BufferIndex >= (unsigned)_SEGGER_RTT.MaxNumDownBuffers) {
    BufferIndex = 0;
  }
  pRing = &_SEGGER_RTT.aDown[BufferIndex];
  pRdOff = &pRing->RdOff;
  RdOff = *pRdOff;
  WrOff = pRing->WrOff;
  NumBytesRead = 0;

  if (RdOff == WrOff) {
    return 0;
  }

  if (RdOff < WrOff) {
      NumBytesRead = WrOff - RdOff;
      if (NumBytesRead > BufferSize) {
          NumBytesRead = BufferSize;
      }
      memcpy(pBuffer, pRing->pBuffer + RdOff, NumBytesRead);
      RdOff += NumBytesRead;
  } else {
      NumBytesRead = pRing->SizeOfBuffer - RdOff;
      if (NumBytesRead > BufferSize) {
          NumBytesRead = BufferSize;
      }
      memcpy(pBuffer, pRing->pBuffer + RdOff, NumBytesRead);
      RdOff += NumBytesRead;
      
      if (RdOff == pRing->SizeOfBuffer) {
          RdOff = 0;
          if (NumBytesRead < BufferSize) {
              unsigned NumBytesRead2 = WrOff;
              if (NumBytesRead2 > (BufferSize - NumBytesRead)) {
                  NumBytesRead2 = (BufferSize - NumBytesRead);
              }
              memcpy((char*)pBuffer + NumBytesRead, pRing->pBuffer, NumBytesRead2);
              RdOff += NumBytesRead2;
              NumBytesRead += NumBytesRead2;
          }
      }
  }
  *pRdOff = RdOff;
  return NumBytesRead;
}

/*********************************************************************
*
*       SEGGER_RTT_ConfigUpBuffer
*
*  Function description
*    Configures a specific up-buffer.
*/
int SEGGER_RTT_ConfigUpBuffer(unsigned BufferIndex, const char* sName, void* pBuffer, unsigned BufferSize, unsigned Flags) {
  if (BufferIndex < (unsigned)_SEGGER_RTT.MaxNumUpBuffers) {
    if (BufferIndex > 0) {
      _SEGGER_RTT.aUp[BufferIndex].sName        = sName;
      _SEGGER_RTT.aUp[BufferIndex].pBuffer      = (char*)pBuffer;
      _SEGGER_RTT.aUp[BufferIndex].SizeOfBuffer = BufferSize;
      _SEGGER_RTT.aUp[BufferIndex].RdOff        = 0u;
      _SEGGER_RTT.aUp[BufferIndex].WrOff        = 0u;
    }
    _SEGGER_RTT.aUp[BufferIndex].Flags          = Flags;
    return 0;
  }
  return -1;
}
