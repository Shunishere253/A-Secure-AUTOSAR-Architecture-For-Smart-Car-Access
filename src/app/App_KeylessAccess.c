#include "App_KeylessAccess.h"

#include "Can_KeylessTx.h"
#include "Cdd_Jdy23.h"
#include "Swc_BleLink.h"
#include "Swc_SecAccess.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_KEYLESS_ACCESS_USER_IN_CAR_LEN            (11U)
#define APP_KEYLESS_ACCESS_AUTO_LOCK_TIMEOUT_TICKS    (6000000UL)

typedef enum
{
    APP_KEYLESS_ACCESS_STATE_LOCKED = 0U,
    APP_KEYLESS_ACCESS_STATE_WAIT_ENTRY,
    APP_KEYLESS_ACCESS_STATE_COMPLETE
} App_KeylessAccess_StateType;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static boolean App_KeylessAccess_ProcessUserInCarByte(uint8 RxByte);
static void App_KeylessAccess_RequestLockFrame(void);
static void App_KeylessAccess_ProcessBleRx(void);
static void App_KeylessAccess_UpdateCanState(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static App_KeylessAccess_StateType s_AppKeylessAccessState =
    APP_KEYLESS_ACCESS_STATE_LOCKED;

static uint8 s_AppKeylessAccessUserInCarIndex = 0U;
static uint32 s_AppKeylessAccessAutoLockTicks = 0UL;

static const uint8 s_AppKeylessAccessEmptyCanInfo[SWC_BLELINK_CAN_INFO_LENGTH] =
{
    0U, 0U, 0U, 0U, 0U, 0U, 0U
};

static const uint8 s_AppKeylessAccessUserInCarCommand[APP_KEYLESS_ACCESS_USER_IN_CAR_LEN] =
{
    0x55U, 0x53U, 0x45U, 0x52U, 0x5FU, 0x49U,
    0x4EU, 0x5FU, 0x43U, 0x41U, 0x52U
};

/*******************************************************************************
 * Code
 ******************************************************************************/
static boolean App_KeylessAccess_ProcessUserInCarByte(uint8 RxByte)
{
    boolean isMatched = FALSE;

    if (RxByte == s_AppKeylessAccessUserInCarCommand[s_AppKeylessAccessUserInCarIndex])
    {
        s_AppKeylessAccessUserInCarIndex++;

        if (s_AppKeylessAccessUserInCarIndex >= APP_KEYLESS_ACCESS_USER_IN_CAR_LEN)
        {
            s_AppKeylessAccessUserInCarIndex = 0U;
            isMatched = TRUE;
        }
    }
    else if (RxByte == s_AppKeylessAccessUserInCarCommand[0U])
    {
        s_AppKeylessAccessUserInCarIndex = 1U;
    }
    else
    {
        s_AppKeylessAccessUserInCarIndex = 0U;
    }

    return isMatched;
}

static void App_KeylessAccess_RequestLockFrame(void)
{
    Can_KeylessTx_DisablePeriodic();
    Can_KeylessTx_RequestOneShotFrame(
        CAN_KEYLESSTX_FRAME_LOCK,
        s_AppKeylessAccessEmptyCanInfo
    );

    Swc_BleLink_SetAuthenticated(FALSE);
    s_AppKeylessAccessState = APP_KEYLESS_ACCESS_STATE_COMPLETE;
    s_AppKeylessAccessUserInCarIndex = 0U;
    s_AppKeylessAccessAutoLockTicks = 0UL;
}

static void App_KeylessAccess_ProcessBleRx(void)
{
    uint8 rxByte = 0U;
    Swc_SecAccess_ResultType secResult = SWC_SECACCESS_RESULT_IDLE;

    if (Cdd_Jdy23_TryReceiveByte(&rxByte) == E_OK)
    {
        Swc_BleLink_OnRxByte(rxByte);

        if (s_AppKeylessAccessState == APP_KEYLESS_ACCESS_STATE_WAIT_ENTRY)
        {
            if (App_KeylessAccess_ProcessUserInCarByte(rxByte) == TRUE)
            {
                App_KeylessAccess_RequestLockFrame();
            }
        }
        else if (s_AppKeylessAccessState == APP_KEYLESS_ACCESS_STATE_LOCKED)
        {
            secResult = Swc_SecAccess_ProcessRxByte(rxByte);

            if (secResult == SWC_SECACCESS_RESULT_PASS)
            {
                Swc_BleLink_SetAuthenticated(TRUE);

                Can_KeylessTx_DisablePeriodic();
                Can_KeylessTx_RequestOneShotFrame(
                    CAN_KEYLESSTX_FRAME_AUTHENTICATED,
                    s_AppKeylessAccessEmptyCanInfo
                );

                s_AppKeylessAccessState = APP_KEYLESS_ACCESS_STATE_WAIT_ENTRY;
                s_AppKeylessAccessUserInCarIndex = 0U;
                s_AppKeylessAccessAutoLockTicks = 0UL;
            }
            else if (secResult == SWC_SECACCESS_RESULT_FAIL)
            {
                Swc_BleLink_SetAuthenticated(FALSE);
                s_AppKeylessAccessState = APP_KEYLESS_ACCESS_STATE_LOCKED;
                s_AppKeylessAccessUserInCarIndex = 0U;
                s_AppKeylessAccessAutoLockTicks = 0UL;
            }
            else
            {
                /* Security access is still running or waiting for start bytes. */
            }
        }
        else
        {
            /* Access cycle is complete; wait for BLE link lost before restart. */
        }
    }
}

static void App_KeylessAccess_UpdateCanState(void)
{
    boolean isConnected = FALSE;
    boolean isAuthenticated = FALSE;

    isConnected = Swc_BleLink_IsConnected();
    isAuthenticated = Swc_BleLink_IsAuthenticated();

    if (s_AppKeylessAccessState == APP_KEYLESS_ACCESS_STATE_LOCKED)
    {
        if ((isConnected == TRUE) && (isAuthenticated == FALSE))
        {
            Can_KeylessTx_SetPeriodicFrame(
                CAN_KEYLESSTX_FRAME_CONNECTED_LOCKED,
                s_AppKeylessAccessEmptyCanInfo,
                TRUE
            );
        }
        else
        {
            Can_KeylessTx_DisablePeriodic();
        }
    }
    else if (s_AppKeylessAccessState == APP_KEYLESS_ACCESS_STATE_WAIT_ENTRY)
    {
        Can_KeylessTx_DisablePeriodic();

        if (s_AppKeylessAccessAutoLockTicks < APP_KEYLESS_ACCESS_AUTO_LOCK_TIMEOUT_TICKS)
        {
            s_AppKeylessAccessAutoLockTicks++;
        }
        else
        {
            App_KeylessAccess_RequestLockFrame();
        }
    }
    else
    {
        Can_KeylessTx_DisablePeriodic();

        if (isConnected == FALSE)
        {
            s_AppKeylessAccessState = APP_KEYLESS_ACCESS_STATE_LOCKED;
        }
    }
}

void App_KeylessAccess_Init(void)
{
    s_AppKeylessAccessState = APP_KEYLESS_ACCESS_STATE_LOCKED;
    s_AppKeylessAccessUserInCarIndex = 0U;
    s_AppKeylessAccessAutoLockTicks = 0UL;
}

void App_KeylessAccess_MainFunction(void)
{
    App_KeylessAccess_ProcessBleRx();
    Swc_SecAccess_MainFunction();
    Swc_BleLink_MainFunction();
    App_KeylessAccess_UpdateCanState();
    Can_KeylessTx_MainFunction();
}

