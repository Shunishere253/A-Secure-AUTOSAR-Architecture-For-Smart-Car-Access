#ifndef _SWC_SECACCESS_H_
#define _SWC_SECACCESS_H_

#include "Std_Types.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef enum
{
    SWC_SECACCESS_RESULT_IDLE = 0U,
    SWC_SECACCESS_RESULT_ACTIVE,
    SWC_SECACCESS_RESULT_PASS,
    SWC_SECACCESS_RESULT_FAIL
} Swc_SecAccess_ResultType;

/*******************************************************************************
 * API
 ******************************************************************************/
/**
* @brief   Initializes the security access software component.
* @details This function resets internal buffers, state and debug status.
*
* @param   -
*
* @return  void.
*
* @pre     Cdd_Crypto_Init shall be called before starting authentication.
**/
void Swc_SecAccess_Init(void);

/**
* @brief   Runs the security access cyclic function.
* @details This function supervises challenge verification timeout while authentication is active.
*
* @param   -
*
* @return  void.
*
* @pre     Swc_SecAccess_Init shall be called before this API.
**/
void Swc_SecAccess_MainFunction(void);

/**
* @brief   Processes one received BLE byte for security access.
* @details This function detects the start sequence, sends a challenge and verifies the encrypted response.
*
* @param[in] RxByte - Received byte from BLE UART.
*
* @return                  Swc_SecAccess_ResultType.
* @retval SWC_SECACCESS_RESULT_ACTIVE Authentication is active or waiting for more bytes.
* @retval SWC_SECACCESS_RESULT_PASS   Authentication passed.
* @retval SWC_SECACCESS_RESULT_FAIL   Authentication failed.
*
* @pre     Swc_SecAccess_Init shall be called before this API.
**/
Swc_SecAccess_ResultType Swc_SecAccess_ProcessRxByte(uint8 RxByte);

/**
* @brief   Runs blocking security access processing.
* @details This compatibility API processes BLE bytes until PASS or FAIL is reached.
*
* @param   -
*
* @return                  Std_ReturnType.
* @retval E_OK             Authentication passed.
* @retval E_NOT_OK         Authentication failed.
*
* @pre     Prefer Swc_SecAccess_ProcessRxByte for non-blocking application flow.
**/
Std_ReturnType Swc_SecAccess_Process(void);

#endif /* _SWC_SECACCESS_H_ */

