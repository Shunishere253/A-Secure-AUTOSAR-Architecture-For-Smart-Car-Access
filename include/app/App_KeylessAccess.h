#ifndef _APP_KEYLESSACCESS_H_
#define _APP_KEYLESSACCESS_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
/**
* @brief   Initializes the keyless access application.
* @details This function resets the keyless access state machine and local counters.
*
* @param   -
*
* @return  void.
*
* @pre     BSW, CAN, BLE link and security access modules shall be initialized before cyclic execution.
**/
void App_KeylessAccess_Init(void);

/**
* @brief   Runs the keyless access application cyclic function.
* @details This function polls BLE bytes, runs security access, updates BLE link state and services CAN transmission.
*
* @param   -
*
* @return  void.
*
* @pre     App_KeylessAccess_Init shall be called before this API.
**/
void App_KeylessAccess_MainFunction(void);

#endif /* _APP_KEYLESSACCESS_H_ */

