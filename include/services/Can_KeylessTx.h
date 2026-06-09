#ifndef _CAN_KEYLESSTX_H_
#define _CAN_KEYLESSTX_H_

#include "Std_Types.h"
#include "Can_43_FLEXCAN.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CAN_KEYLESSTX_INFO_LENGTH             (7U)

#define CAN_KEYLESSTX_FRAME_CONNECTED_LOCKED  (0x5AU)
#define CAN_KEYLESSTX_FRAME_AUTHENTICATED     (0xA5U)
#define CAN_KEYLESSTX_FRAME_LOCK              (0xAAU)

/*******************************************************************************
 * API
 ******************************************************************************/
/**
* @brief   Initializes the keyless CAN transmit service.
* @details This function initializes CAN driver state, CAN PDU information and transmission state machine.
*
* @param   -
*
* @return  void.
*
* @pre     Platform, MCU, Port and MCL shall be initialized before this API.
**/
void Can_KeylessTx_Init(void);

/**
* @brief   Runs the keyless CAN transmit cyclic function.
* @details This function polls CAN driver main functions, supervises confirmation and transmits pending frames.
*
* @param   -
*
* @return  void.
*
* @pre     Can_KeylessTx_Init shall be called before this API.
**/
void Can_KeylessTx_MainFunction(void);

/**
* @brief   Configures a periodic CAN frame.
* @details This function copies the first byte and optional information bytes into the periodic frame buffer.
*
* @param[in] FirstByte - Frame command byte.
* @param[in] InfoPtr   - Optional pointer to seven information bytes. NULL_PTR fills information bytes with zero.
* @param[in] Enable    - TRUE enables periodic transmission, FALSE disables it after frame update.
*
* @return  void.
*
* @pre     Can_KeylessTx_Init shall be called before this API.
**/
void Can_KeylessTx_SetPeriodicFrame(
    uint8 FirstByte,
    const uint8 *InfoPtr,
    boolean Enable
);

/**
* @brief   Requests a one-shot CAN frame.
* @details This function queues a one-shot frame and keeps one pending frame if a one-shot transmission is active.
*
* @param[in] FirstByte - Frame command byte.
* @param[in] InfoPtr   - Optional pointer to seven information bytes. NULL_PTR fills information bytes with zero.
*
* @return  void.
*
* @pre     Can_KeylessTx_Init shall be called before this API.
**/
void Can_KeylessTx_RequestOneShotFrame(
    uint8 FirstByte,
    const uint8 *InfoPtr
);

/**
* @brief   Disables periodic CAN transmission.
* @details This function stops periodic frame requests without clearing pending one-shot frames.
*
* @param   -
*
* @return  void.
*
* @pre     Can_KeylessTx_Init shall be called before this API.
**/
void Can_KeylessTx_DisablePeriodic(void);

/**
* @brief   Notifies CAN bus-off event.
* @details This function stores bus-off diagnostic information from the CAN interface callback.
*
* @param[in] ControllerId - CAN controller identifier.
*
* @return  void.
*
* @pre     Called by CAN interface callback context.
**/
void Can_KeylessTx_NotifyBusOff(uint8 ControllerId);

/**
* @brief   Notifies CAN controller mode change.
* @details This function stores controller mode information used by controller start supervision.
*
* @param[in] ControllerId    - CAN controller identifier.
* @param[in] ControllerMode  - New CAN controller mode.
*
* @return  void.
*
* @pre     Called by CAN interface callback context.
**/
void Can_KeylessTx_NotifyMode(
    uint8 ControllerId,
    Can_ControllerStateType ControllerMode
);

/**
* @brief   Notifies CAN transmit confirmation.
* @details This function stores transmit confirmation status for the polling transmit state machine.
*
* @param[in] CanTxPduId - Confirmed CAN PDU identifier.
*
* @return  void.
*
* @pre     Called by CAN interface callback context.
**/
void Can_KeylessTx_NotifyTxConfirmation(PduIdType CanTxPduId);

/**
* @brief   Notifies CAN receive indication.
* @details This function counts receive indications. The payload is not consumed by this service.
*
* @param[in] Mailbox    - Pointer to CAN hardware mailbox information.
* @param[in] PduInfoPtr - Pointer to received PDU information.
*
* @return  void.
*
* @pre     Called by CAN interface callback context.
**/
void Can_KeylessTx_NotifyRx(
    const Can_HwType *Mailbox,
    const PduInfoType *PduInfoPtr
);

#endif /* _CAN_KEYLESSTX_H_ */

