#ifndef _BSW_PLATFORM_H_
#define _BSW_PLATFORM_H_

#include "Std_Types.h"
#include "Mcu.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
/* Extern diagnostic variables are intentionally exposed for S32DS debugger watch. */
extern volatile uint32 g_DebugStep;
extern volatile Mcu_PllStatusType g_McuPllStatus;
extern volatile uint32 g_McuPllTimeout;

/**
* @brief   Initializes the basic software platform.
* @details This function initializes Platform, MCU clock, MCL, Port, UART and Crypto services.
*
* @param   -
*
* @return  void.
*
* @pre     This API shall be called once during ECU startup before application modules.
**/
void Bsw_Platform_Init(void);

#endif /* _BSW_PLATFORM_H_ */

