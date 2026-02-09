/**
 * @file blm_port.h
 * @brief BLM Hardware Abstraction Layer Port Interface
 * 
 * This file defines all hardware-dependent interfaces that need to be 
 * implemented for different MCU platforms (STM32, AT32, etc.)
 */
#ifndef __BLM_PORT_H__
#define __BLM_PORT_H__

#include <stdint.h>

/*============================================================================
 * UART Interface
 *===========================================================================*/

/**
 * @brief Initialize UART for bootloader communication
 * @param wBaudrate Baud rate
 * @return 0 on success, negative error code on failure
 */
int blm_port_UartInit(uint32_t wBaudrate);

/**
 * @brief Send data via UART
 * @param pchData Pointer to data buffer
 * @param hwLen Data length
 * @return Number of bytes sent, negative on error
 */
int blm_port_UartSend(const uint8_t *pchData, uint16_t hwLen);

/**
 * @brief Receive data via UART with timeout
 * @param pchData Pointer to receive buffer
 * @param hwLen Expected length
 * @param wTimeoutMs Timeout in milliseconds
 * @return Number of bytes received, negative on error/timeout
 */
int blm_port_UartRecv(uint8_t *pchData, uint16_t hwLen, uint32_t wTimeoutMs);

/**
 * @brief Check if data available in UART RX buffer
 * @return Number of bytes available
 */
int blm_port_UartAvailable(void);

/**
 * @brief Flush UART buffers
 */
void blm_port_UartFlush(void);

/*============================================================================
 * Flash Interface
 *===========================================================================*/

/**
 * @brief Unlock flash for write/erase operations
 * @return 0 on success
 */
int blm_port_FlashUnlock(void);

/**
 * @brief Lock flash after write/erase operations
 * @return 0 on success
 */
int blm_port_FlashLock(void);

/**
 * @brief Erase flash region
 * @param wAddr Start address (must be page/sector aligned)
 * @param wSize Size to erase
 * @return 0 on success
 */
int blm_port_FlashErase(uint32_t wAddr, uint32_t wSize);

/**
 * @brief Write data to flash
 * @param wAddr Destination address
 * @param pchData Source data
 * @param wLen Data length
 * @return 0 on success
 */
int blm_port_FlashWrite(uint32_t wAddr, const uint8_t *pchData, uint32_t wLen);

/**
 * @brief Read data from flash
 * @param wAddr Source address
 * @param pchData Destination buffer
 * @param wLen Data length
 * @return 0 on success
 */
int blm_port_FlashRead(uint32_t wAddr, uint8_t *pchData, uint32_t wLen);

/**
 * @brief Get flash page/sector size
 * @param wAddr Address to query
 * @return Page/sector size in bytes
 */
uint32_t blm_port_FlashGetPageSize(uint32_t wAddr);

/*============================================================================
 * System Interface
 *===========================================================================*/

/**
 * @brief System reset
 */
void blm_port_SystemReset(void);

/**
 * @brief Jump to application
 * @param wAppAddr Application start address
 */
void blm_port_JumpToApp(uint32_t wAppAddr);

/**
 * @brief Get system tick in milliseconds
 * @return Tick count
 */
uint32_t blm_port_GetTickMs(void);

/**
 * @brief Delay in milliseconds
 * @param wMs Delay time
 */
void blm_port_DelayMs(uint32_t wMs);

/**
 * @brief Check if upgrade button is pressed
 * @return 1 if pressed, 0 otherwise
 */
int blm_port_IsUpgradeButtonPressed(void);

#endif  // __BLM_PORT_H__
