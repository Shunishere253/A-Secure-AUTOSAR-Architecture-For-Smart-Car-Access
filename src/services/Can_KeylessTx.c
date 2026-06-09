#include "Can_KeylessTx.h"

#include "Can_43_FLEXCAN.h"
#include "SchM_Can_43_FLEXCAN.h"

#include <string.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CAN_KEYLESSTX_ID                         (0x123U)
#define CAN_KEYLESSTX_DLC                        (8U)
#define CAN_KEYLESSTX_SW_PDU_HANDLE              (1U)

#define CAN_KEYLESSTX_TX_PERIOD_TICKS            (20000UL)
#define CAN_KEYLESSTX_TX_CONFIRM_TIMEOUT_TICKS   (200000UL)
#define CAN_KEYLESSTX_START_TIMEOUT              (1000000UL)

#define CAN_KEYLESSTX_CONTROLLER_ID              \
    Can_43_FLEXCANConf_CanController_CanController_0

#define CAN_KEYLESSTX_TX_HOH                     \
    Can_43_FLEXCANConf_CanHardwareObject_CanHardwareObject_1

#ifndef CAN_BUSY
#define CAN_BUSY                                 (2U)
#endif /* CAN_BUSY */

typedef enum
{
    CAN_KEYLESSTX_TX_KIND_NONE = 0U,
    CAN_KEYLESSTX_TX_KIND_PERIODIC,
    CAN_KEYLESSTX_TX_KIND_ONESHOT
} Can_KeylessTx_TxKindType;

typedef enum
{
    CAN_KEYLESSTX_ONESHOT_IDLE = 0U,
    CAN_KEYLESSTX_ONESHOT_REQUESTED,
    CAN_KEYLESSTX_ONESHOT_WAIT_CONFIRM
} Can_KeylessTx_OneShotStateType;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void Can_KeylessTx_CopyFrame(
    uint8 *FramePtr,
    uint8 FirstByte,
    const uint8 *InfoPtr
);
static void Can_KeylessTx_PollDriver(void);
static void Can_KeylessTx_StartController(void);
static Std_ReturnType Can_KeylessTx_WriteFrame(const uint8 *FramePtr);
static void Can_KeylessTx_HandleConfirmation(void);
static void Can_KeylessTx_TryTransmit(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (CAN_43_FLEXCAN_PRECOMPILE_SUPPORT == STD_OFF)
extern const Can_43_FLEXCAN_ConfigType Can_43_FLEXCAN_Config; /* Generated RTD configuration object. */
#endif /* CAN_43_FLEXCAN_PRECOMPILE_SUPPORT == STD_OFF */

static uint8 s_CanKeylessTxSdu[CAN_KEYLESSTX_DLC] =
{
    0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U
};

static Can_PduType s_CanKeylessTxPduInfo;

static boolean s_CanKeylessTxPeriodicEnabled = FALSE;
static uint8 s_CanKeylessTxPeriodicFrame[CAN_KEYLESSTX_DLC];
static uint8 s_CanKeylessTxOneShotFrame[CAN_KEYLESSTX_DLC];
static uint8 s_CanKeylessTxPendingOneShotFrame[CAN_KEYLESSTX_DLC];
static boolean s_CanKeylessTxPendingOneShotValid = FALSE;

static Can_KeylessTx_TxKindType s_CanKeylessTxCurrentTxKind =
    CAN_KEYLESSTX_TX_KIND_NONE;

static Can_KeylessTx_OneShotStateType s_CanKeylessTxOneShotState =
    CAN_KEYLESSTX_ONESHOT_IDLE;

static uint32 s_CanKeylessTxPeriodTicks = 0UL;
static uint32 s_CanKeylessTxConfirmTicks = 0UL;

/* Static volatile variables are kept for debugger/watch visibility during CAN bring-up. */
static volatile Std_ReturnType s_CanStartRet = E_NOT_OK;
static volatile Can_ControllerStateType s_CanControllerMode = CAN_CS_UNINIT;
static volatile uint32 s_CanModeTimeout = 0UL;

static volatile uint8 s_CanWriteRet = 0xFFU;
static volatile uint8 s_FirstWriteRet = 0xFFU;
static volatile uint8 s_FirstWriteDone = 0U;

static volatile boolean s_CanTxFlag = FALSE;
static volatile uint32 s_CanTxConfirmCnt = 0UL;
static volatile uint32 s_CanTxTimeout = 0UL;

static volatile uint32 s_CanWriteOkCnt = 0UL;
static volatile uint32 s_CanBusyCnt = 0UL;
static volatile uint32 s_CanWriteFailCnt = 0UL;
static volatile uint32 s_CanNoConfirmCnt = 0UL;

static volatile uint32 s_CanBusOffCnt = 0UL;
static volatile uint8 s_CanBusOffFlag = 0U;

static volatile PduIdType s_CanLastPduId = 0U;
static volatile uint8 s_CanLastControllerId = 0U;
static volatile uint32 s_CanRxIndicationCnt = 0UL;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void Can_KeylessTx_CopyFrame(
    uint8 *FramePtr,
    uint8 FirstByte,
    const uint8 *InfoPtr
)
{
    uint8 index = 0U;

    if (FramePtr != NULL_PTR)
    {
        FramePtr[0U] = FirstByte;

        for (index = 0U; index < CAN_KEYLESSTX_INFO_LENGTH; index++)
        {
            if (InfoPtr != NULL_PTR)
            {
                FramePtr[index + 1U] = InfoPtr[index];
            }
            else
            {
                FramePtr[index + 1U] = 0U;
            }
        }
    }
}

static void Can_KeylessTx_PollDriver(void)
{
    Can_43_FLEXCAN_MainFunction_Write();
    Can_43_FLEXCAN_MainFunction_Mode();
    Can_43_FLEXCAN_MainFunction_BusOff();
}

static void Can_KeylessTx_StartController(void)
{
    uint32 timeout = CAN_KEYLESSTX_START_TIMEOUT;

    s_CanControllerMode = CAN_CS_UNINIT;

    s_CanStartRet = Can_43_FLEXCAN_SetControllerMode(
        CAN_KEYLESSTX_CONTROLLER_ID,
        CAN_CS_STARTED
    );

    while ((s_CanControllerMode != CAN_CS_STARTED) && (timeout > 0UL))
    {
        Can_43_FLEXCAN_MainFunction_Mode();
        timeout--;
    }

    s_CanModeTimeout = timeout;
}

static Std_ReturnType Can_KeylessTx_WriteFrame(const uint8 *FramePtr)
{
    Std_ReturnType ret = E_NOT_OK;

    if (FramePtr != NULL_PTR)
    {
        (void)memcpy(s_CanKeylessTxSdu, FramePtr, CAN_KEYLESSTX_DLC);
        s_CanTxFlag = FALSE;

        s_CanWriteRet = (uint8)Can_43_FLEXCAN_Write(
            CAN_KEYLESSTX_TX_HOH,
            &s_CanKeylessTxPduInfo
        );

        if (s_FirstWriteDone == 0U)
        {
            s_FirstWriteRet = s_CanWriteRet;
            s_FirstWriteDone = 1U;
        }

        if (s_CanWriteRet == E_OK)
        {
            s_CanWriteOkCnt++;
            ret = E_OK;
        }
        else if (s_CanWriteRet == CAN_BUSY)
        {
            s_CanBusyCnt++;
        }
        else
        {
            s_CanWriteFailCnt++;
        }
    }

    return ret;
}

static void Can_KeylessTx_HandleConfirmation(void)
{
    if (s_CanKeylessTxCurrentTxKind != CAN_KEYLESSTX_TX_KIND_NONE)
    {
        s_CanKeylessTxConfirmTicks++;

        if (s_CanTxFlag == TRUE)
        {
            s_CanKeylessTxCurrentTxKind = CAN_KEYLESSTX_TX_KIND_NONE;
            s_CanKeylessTxConfirmTicks = 0UL;

            if (s_CanKeylessTxOneShotState == CAN_KEYLESSTX_ONESHOT_WAIT_CONFIRM)
            {
                if (s_CanKeylessTxPendingOneShotValid == TRUE)
                {
                    (void)memcpy(
                        s_CanKeylessTxOneShotFrame,
                        s_CanKeylessTxPendingOneShotFrame,
                        CAN_KEYLESSTX_DLC
                    );
                    s_CanKeylessTxPendingOneShotValid = FALSE;
                    s_CanKeylessTxOneShotState = CAN_KEYLESSTX_ONESHOT_REQUESTED;
                }
                else
                {
                    s_CanKeylessTxOneShotState = CAN_KEYLESSTX_ONESHOT_IDLE;
                }
            }
        }
        else if (s_CanKeylessTxConfirmTicks > CAN_KEYLESSTX_TX_CONFIRM_TIMEOUT_TICKS)
        {
            s_CanNoConfirmCnt++;
            s_CanTxTimeout = s_CanKeylessTxConfirmTicks;
            s_CanKeylessTxCurrentTxKind = CAN_KEYLESSTX_TX_KIND_NONE;
            s_CanKeylessTxConfirmTicks = 0UL;

            if (s_CanKeylessTxOneShotState == CAN_KEYLESSTX_ONESHOT_WAIT_CONFIRM)
            {
                s_CanKeylessTxOneShotState = CAN_KEYLESSTX_ONESHOT_REQUESTED;
            }
        }
        else
        {
            /* Wait for confirmation in polling mode. */
        }
    }
}

static void Can_KeylessTx_TryTransmit(void)
{
    if (s_CanKeylessTxCurrentTxKind == CAN_KEYLESSTX_TX_KIND_NONE)
    {
        if (s_CanKeylessTxOneShotState == CAN_KEYLESSTX_ONESHOT_REQUESTED)
        {
            if (Can_KeylessTx_WriteFrame(s_CanKeylessTxOneShotFrame) == E_OK)
            {
                s_CanKeylessTxCurrentTxKind = CAN_KEYLESSTX_TX_KIND_ONESHOT;
                s_CanKeylessTxOneShotState = CAN_KEYLESSTX_ONESHOT_WAIT_CONFIRM;
            }
        }
        else if (s_CanKeylessTxPeriodicEnabled == TRUE)
        {
            if (s_CanKeylessTxPeriodTicks < CAN_KEYLESSTX_TX_PERIOD_TICKS)
            {
                s_CanKeylessTxPeriodTicks++;
            }
            else
            {
                s_CanKeylessTxPeriodTicks = 0UL;

                if (Can_KeylessTx_WriteFrame(s_CanKeylessTxPeriodicFrame) == E_OK)
                {
                    s_CanKeylessTxCurrentTxKind = CAN_KEYLESSTX_TX_KIND_PERIODIC;
                }
            }
        }
        else
        {
            s_CanKeylessTxPeriodTicks = CAN_KEYLESSTX_TX_PERIOD_TICKS;
        }
    }
}

void Can_KeylessTx_Init(void)
{
    (void)memset(s_CanKeylessTxPeriodicFrame, 0, CAN_KEYLESSTX_DLC);
    (void)memset(s_CanKeylessTxOneShotFrame, 0, CAN_KEYLESSTX_DLC);
    (void)memset(s_CanKeylessTxPendingOneShotFrame, 0, CAN_KEYLESSTX_DLC);

    s_CanKeylessTxPduInfo.id = CAN_KEYLESSTX_ID;
    s_CanKeylessTxPduInfo.swPduHandle = CAN_KEYLESSTX_SW_PDU_HANDLE;
    s_CanKeylessTxPduInfo.length = CAN_KEYLESSTX_DLC;
    s_CanKeylessTxPduInfo.sdu = s_CanKeylessTxSdu;

#if (CAN_43_FLEXCAN_PRECOMPILE_SUPPORT == STD_ON)
    Can_43_FLEXCAN_Init(NULL_PTR);
#else
    Can_43_FLEXCAN_Init(&Can_43_FLEXCAN_Config);
#endif /* CAN_43_FLEXCAN_PRECOMPILE_SUPPORT == STD_ON */

    Can_KeylessTx_StartController();

    s_CanKeylessTxPeriodicEnabled = FALSE;
    s_CanKeylessTxCurrentTxKind = CAN_KEYLESSTX_TX_KIND_NONE;
    s_CanKeylessTxOneShotState = CAN_KEYLESSTX_ONESHOT_IDLE;
    s_CanKeylessTxPendingOneShotValid = FALSE;
    s_CanKeylessTxPeriodTicks = CAN_KEYLESSTX_TX_PERIOD_TICKS;
    s_CanKeylessTxConfirmTicks = 0UL;
}

void Can_KeylessTx_MainFunction(void)
{
    Can_KeylessTx_PollDriver();
    Can_KeylessTx_HandleConfirmation();
    Can_KeylessTx_TryTransmit();
}

void Can_KeylessTx_SetPeriodicFrame(
    uint8 FirstByte,
    const uint8 *InfoPtr,
    boolean Enable
)
{
    Can_KeylessTx_CopyFrame(s_CanKeylessTxPeriodicFrame, FirstByte, InfoPtr);
    s_CanKeylessTxPeriodicEnabled = Enable;
}

void Can_KeylessTx_RequestOneShotFrame(
    uint8 FirstByte,
    const uint8 *InfoPtr
)
{
    if (s_CanKeylessTxOneShotState == CAN_KEYLESSTX_ONESHOT_IDLE)
    {
        Can_KeylessTx_CopyFrame(
            s_CanKeylessTxOneShotFrame,
            FirstByte,
            InfoPtr
        );
        s_CanKeylessTxOneShotState = CAN_KEYLESSTX_ONESHOT_REQUESTED;
    }
    else
    {
        Can_KeylessTx_CopyFrame(
            s_CanKeylessTxPendingOneShotFrame,
            FirstByte,
            InfoPtr
        );
        s_CanKeylessTxPendingOneShotValid = TRUE;
    }
}

void Can_KeylessTx_DisablePeriodic(void)
{
    s_CanKeylessTxPeriodicEnabled = FALSE;
}

void Can_KeylessTx_NotifyBusOff(uint8 ControllerId)
{
    s_CanBusOffCnt++;
    s_CanBusOffFlag = 1U;
    s_CanLastControllerId = ControllerId;
}

void Can_KeylessTx_NotifyMode(
    uint8 ControllerId,
    Can_ControllerStateType ControllerMode
)
{
    s_CanLastControllerId = ControllerId;
    s_CanControllerMode = ControllerMode;
}

void Can_KeylessTx_NotifyTxConfirmation(PduIdType CanTxPduId)
{
    s_CanTxConfirmCnt++;
    s_CanTxFlag = TRUE;
    s_CanLastPduId = CanTxPduId;
}

void Can_KeylessTx_NotifyRx(
    const Can_HwType *Mailbox,
    const PduInfoType *PduInfoPtr
)
{
    s_CanRxIndicationCnt++;

    /* Rx payload is not consumed by this service yet; parameters are intentionally not dereferenced. */
    (void)Mailbox;
    (void)PduInfoPtr;
}

void CanIf_ControllerBusOff(uint8 ControllerId)
{
    Can_KeylessTx_NotifyBusOff(ControllerId);
}

void CanIf_ControllerModeIndication(
    uint8 ControllerId,
    Can_ControllerStateType ControllerMode
)
{
    Can_KeylessTx_NotifyMode(ControllerId, ControllerMode);
}

void CanIf_TxConfirmation(PduIdType CanTxPduId)
{
    Can_KeylessTx_NotifyTxConfirmation(CanTxPduId);
}

void CanIf_RxIndication(
    const Can_HwType *Mailbox,
    const PduInfoType *PduInfoPtr
)
{
    Can_KeylessTx_NotifyRx(Mailbox, PduInfoPtr);
}

