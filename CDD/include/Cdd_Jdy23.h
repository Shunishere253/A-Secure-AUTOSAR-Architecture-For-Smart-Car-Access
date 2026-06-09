#ifndef _CDD_JDY23_H_
#define _CDD_JDY23_H_

#include "Std_Types.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define UART_LPUART_INTERNAL_CHANNEL    (0U)

/*******************************************************************************
 * API
 ******************************************************************************/
/**
* @brief   Receives bytes from JDY-23 UART with default timeout.
* @details This function starts a blocking receive operation for the requested length.
*
* @param[out] Buffer - Pointer to destination buffer.
* @param[in]  Length - Number of bytes to receive.
*
* @return                  Std_ReturnType.
* @retval E_OK             Reception completed successfully.
* @retval E_NOT_OK         Invalid parameter, UART error or timeout.
*
* @pre     Uart_Init shall be called before this API.
**/
Std_ReturnType Cdd_Jdy23_Receive(uint8 *Buffer, uint32 Length);

/**
* @brief   Sends bytes through JDY-23 UART.
* @details This function starts a blocking send operation for the requested length.
*
* @param[in] Buffer - Pointer to source buffer.
* @param[in] Length - Number of bytes to send.
*
* @return                  Std_ReturnType.
* @retval E_OK             Transmission completed successfully.
* @retval E_NOT_OK         Invalid parameter, UART error or timeout.
*
* @pre     Uart_Init shall be called before this API.
**/
Std_ReturnType Cdd_Jdy23_Send(const uint8 *Buffer, uint32 Length);

/**
* @brief   Receives one byte from JDY-23 UART.
* @details This function receives one byte using the default blocking receive timeout.
*
* @param[out] Byte - Pointer to received byte destination.
*
* @return                  Std_ReturnType.
* @retval E_OK             Reception completed successfully.
* @retval E_NOT_OK         Invalid parameter, UART error or timeout.
*
* @pre     Uart_Init shall be called before this API.
**/
Std_ReturnType Cdd_Jdy23_ReceiveByte(uint8 *Byte);

/**
* @brief   Polls one byte from JDY-23 UART without blocking.
* @details This function services one-byte asynchronous reception and rearms RX after each received byte.
*
* @param[out] Byte - Pointer to received byte destination.
*
* @return                  Std_ReturnType.
* @retval E_OK             One byte is available and copied to Byte.
* @retval E_NOT_OK         No byte available or invalid parameter.
*
* @pre     Uart_Init shall be called before this API.
**/
Std_ReturnType Cdd_Jdy23_TryReceiveByte(uint8 *Byte);

/**
* @brief   Receives bytes from JDY-23 UART with caller-provided timeout.
* @details This function starts a blocking receive operation for the requested length and timeout.
*
* @param[out] Buffer       - Pointer to destination buffer.
* @param[in]  Length       - Number of bytes to receive.
* @param[in]  TimeoutTicks - Timeout counter value.
*
* @return                  Std_ReturnType.
* @retval E_OK             Reception completed successfully.
* @retval E_NOT_OK         Invalid parameter, UART error or timeout.
*
* @pre     Uart_Init shall be called before this API.
**/
Std_ReturnType Cdd_Jdy23_ReceiveWithTimeout(
    uint8 *Buffer,
    uint32 Length,
    uint32 TimeoutTicks
);

/**
* @brief   Resets JDY-23 UART session state.
* @details This function aborts active UART send/receive operations and resets non-blocking RX state.
*
* @param   -
*
* @return  void.
*
* @pre     Uart_Init shall be called before this API.
**/
void Cdd_Jdy23_ResetSession(void);

/**
* @brief   Returns accumulated JDY-23 entropy.
* @details This function mixes and returns the local entropy value used by the crypto service.
*
* @param   -
*
* @return  uint32 entropy value.
*
* @pre     -
**/
uint32 Cdd_Jdy23_GetEntropy(void);

#endif /* _CDD_JDY23_H_ */

