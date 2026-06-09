#include "Bsw_Platform.h"

#include "Cdd_Crypto.h"
#include "Cdd_Jdy23.h"
#include "CDD_Uart.h"
#include "Mcl.h"
#include "Platform.h"
#include "Port.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BSW_PLATFORM_MCU_PLL_LOCK_TIMEOUT    (1000000UL)
#define BSW_PLATFORM_READY_TEXT              ((const uint8 *)"UART READY\r\n")
#define BSW_PLATFORM_READY_TEXT_LENGTH       (12U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void Bsw_Platform_InitMcu(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (MCU_PRECOMPILE_SUPPORT == STD_OFF)
extern const Mcu_ConfigType Mcu_Config; /* Generated RTD configuration object. */
#endif /* MCU_PRECOMPILE_SUPPORT == STD_OFF */

/* Global volatile variables are kept for debugger/watch visibility during board bring-up. */
volatile uint32 g_DebugStep = 0UL;
volatile Mcu_PllStatusType g_McuPllStatus = MCU_PLL_UNLOCKED;
volatile uint32 g_McuPllTimeout = 0UL;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void Bsw_Platform_InitMcu(void)
{
#if (MCU_NO_PLL == STD_OFF)
    uint32 pllTimeout = 0UL;
#endif /* MCU_NO_PLL == STD_OFF */

    g_DebugStep = 2UL;

#if (MCU_PRECOMPILE_SUPPORT == STD_ON)
    Mcu_Init(NULL_PTR);
#else
    Mcu_Init(&Mcu_Config);
#endif /* MCU_PRECOMPILE_SUPPORT == STD_ON */

    g_DebugStep = 3UL;
    Mcu_InitClock(McuModeSettingConf_0);

#if (MCU_NO_PLL == STD_OFF)
    pllTimeout = BSW_PLATFORM_MCU_PLL_LOCK_TIMEOUT;

    do
    {
        g_McuPllStatus = Mcu_GetPllStatus();
        pllTimeout--;
    }
    while ((MCU_PLL_LOCKED != g_McuPllStatus) && (pllTimeout > 0UL));

    g_McuPllTimeout = pllTimeout;

    if (MCU_PLL_LOCKED == g_McuPllStatus)
    {
        Mcu_DistributePllClock();
    }
    else
    {
        g_DebugStep = 91UL;

        while (1)
        {
        }
    }
#endif /* MCU_NO_PLL == STD_OFF */

    g_DebugStep = 4UL;
    Mcu_SetMode(McuModeSettingConf_0);
}

void Bsw_Platform_Init(void)
{
    g_DebugStep = 1UL;
    Platform_Init(NULL_PTR);

    Bsw_Platform_InitMcu();

    g_DebugStep = 5UL;
    Mcl_Init(NULL_PTR);

    g_DebugStep = 6UL;
    Port_Init(NULL_PTR);

    g_DebugStep = 7UL;
    Uart_Init(NULL_PTR);

    g_DebugStep = 8UL;
    Cdd_Crypto_Init();

    (void)Cdd_Jdy23_Send(BSW_PLATFORM_READY_TEXT, BSW_PLATFORM_READY_TEXT_LENGTH);
}

