#ifndef _SWC_BLELINK_H_
#define _SWC_BLELINK_H_

#include "Std_Types.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SWC_BLELINK_CAN_INFO_LENGTH       (7U)

#define SWC_BLELINK_POSITION_UNKNOWN      (0U)
#define SWC_BLELINK_POSITION_OUTSIDE      (1U)
#define SWC_BLELINK_POSITION_CABIN        (2U)

typedef struct
{
    uint8 PositionZone;
    uint8 RssiLevel;
    uint8 DistanceCm;
    uint8 Flags;
} Swc_BleLink_TelemetryType;

/*******************************************************************************
 * API
 ******************************************************************************/
/**
* @brief   Initializes the BLE link software component.
* @details This function resets BLE connection, authentication, telemetry and diagnostic counters.
*
* @param   -
*
* @return  void.
*
* @pre     -
**/
void Swc_BleLink_Init(void);

/**
* @brief   Runs the BLE link cyclic function.
* @details This function supervises silent ticks and clears link state after link lost timeout.
*
* @param   -
*
* @return  void.
*
* @pre     Swc_BleLink_Init shall be called before this API.
**/
void Swc_BleLink_MainFunction(void);

/**
* @brief   Notifies one received BLE byte.
* @details This function marks BLE link as connected and updates activity diagnostics.
*
* @param[in] RxByte - Received byte from BLE UART.
*
* @return  void.
*
* @pre     Swc_BleLink_Init shall be called before this API.
**/
void Swc_BleLink_OnRxByte(uint8 RxByte);

/**
* @brief   Updates BLE authentication state.
* @details This function stores authentication state and marks link as connected when authenticated.
*
* @param[in] IsAuthenticated - TRUE if security access passed, otherwise FALSE.
*
* @return  void.
*
* @pre     Swc_BleLink_Init shall be called before this API.
**/
void Swc_BleLink_SetAuthenticated(boolean IsAuthenticated);

/**
* @brief   Updates BLE telemetry data.
* @details This function copies telemetry information and marks link telemetry as valid.
*
* @param[in] TelemetryPtr - Pointer to telemetry data.
*
* @return  void.
*
* @pre     Swc_BleLink_Init shall be called before this API.
**/
void Swc_BleLink_UpdateTelemetry(
    const Swc_BleLink_TelemetryType *TelemetryPtr
);

/**
* @brief   Gets BLE information bytes for CAN payload.
* @details This function copies BLE flags, position, RSSI, distance and diagnostic bytes into caller buffer.
*
* @param[out] InfoPtr - Pointer to output buffer.
* @param[in]  Length  - Output buffer length.
*
* @return  void.
*
* @pre     InfoPtr shall point to at least SWC_BLELINK_CAN_INFO_LENGTH bytes.
**/
void Swc_BleLink_GetCanInfo(uint8 *InfoPtr, uint8 Length);

/**
* @brief   Returns BLE connection state.
* @details This function returns TRUE while BLE activity is detected before link lost timeout.
*
* @param   -
*
* @return  boolean connection state.
*
* @pre     Swc_BleLink_Init shall be called before this API.
**/
boolean Swc_BleLink_IsConnected(void);

/**
* @brief   Returns BLE authentication state.
* @details This function returns TRUE after successful security access until state is cleared.
*
* @param   -
*
* @return  boolean authentication state.
*
* @pre     Swc_BleLink_Init shall be called before this API.
**/
boolean Swc_BleLink_IsAuthenticated(void);

/**
* @brief   Returns BLE strong proximity state.
* @details This function returns TRUE when telemetry indicates cabin or near/strong signal.
*
* @param   -
*
* @return  boolean strong proximity state.
*
* @pre     Swc_BleLink_Init shall be called before this API.
**/
boolean Swc_BleLink_IsStrongProximity(void);

#endif /* _SWC_BLELINK_H_ */

