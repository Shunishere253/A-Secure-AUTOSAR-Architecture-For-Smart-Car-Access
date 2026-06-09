#include "Swc_BleLink.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SWC_BLELINK_LINK_LOST_TICKS       (100000UL)
#define SWC_BLELINK_STRONG_RSSI_LEVEL     (80U)
#define SWC_BLELINK_NEAR_DISTANCE_CM      (80U)
#define SWC_BLELINK_DISTANCE_UNKNOWN      (0xFFU)

#define SWC_BLELINK_FLAG_CONNECTED        (0x01U)
#define SWC_BLELINK_FLAG_AUTHENTICATED    (0x02U)
#define SWC_BLELINK_FLAG_STRONG           (0x04U)
#define SWC_BLELINK_FLAG_TELEMETRY_VALID  (0x08U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static boolean Swc_BleLink_IsTelemetryStrong(void);
static uint8 Swc_BleLink_BuildFlags(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static boolean s_SwcBleLinkConnected = FALSE;
static boolean s_SwcBleLinkAuthenticated = FALSE;
static boolean s_SwcBleLinkTelemetryValid = FALSE;

static uint32 s_SwcBleLinkSilentTicks = 0UL;
static uint8 s_SwcBleLinkLastRxByte = 0U;
static uint8 s_SwcBleLinkActivityCounter = 0U;
static uint8 s_SwcBleLinkDiagCounter = 0U;

static Swc_BleLink_TelemetryType s_SwcBleLinkTelemetry =
{
    SWC_BLELINK_POSITION_UNKNOWN,
    0U,
    SWC_BLELINK_DISTANCE_UNKNOWN,
    0U
};

/*******************************************************************************
 * Code
 ******************************************************************************/
static boolean Swc_BleLink_IsTelemetryStrong(void)
{
    boolean isStrong = FALSE;

    if (s_SwcBleLinkTelemetryValid == TRUE)
    {
        if ((s_SwcBleLinkTelemetry.PositionZone == SWC_BLELINK_POSITION_CABIN) ||
            (s_SwcBleLinkTelemetry.RssiLevel >= SWC_BLELINK_STRONG_RSSI_LEVEL) ||
            (s_SwcBleLinkTelemetry.DistanceCm <= SWC_BLELINK_NEAR_DISTANCE_CM))
        {
            isStrong = TRUE;
        }
    }

    return isStrong;
}

static uint8 Swc_BleLink_BuildFlags(void)
{
    uint8 flags = 0U;

    if (s_SwcBleLinkConnected == TRUE)
    {
        flags |= SWC_BLELINK_FLAG_CONNECTED;
    }

    if (s_SwcBleLinkAuthenticated == TRUE)
    {
        flags |= SWC_BLELINK_FLAG_AUTHENTICATED;
    }

    if (Swc_BleLink_IsStrongProximity() == TRUE)
    {
        flags |= SWC_BLELINK_FLAG_STRONG;
    }

    if (s_SwcBleLinkTelemetryValid == TRUE)
    {
        flags |= SWC_BLELINK_FLAG_TELEMETRY_VALID;
    }

    return flags;
}

void Swc_BleLink_Init(void)
{
    s_SwcBleLinkConnected = FALSE;
    s_SwcBleLinkAuthenticated = FALSE;
    s_SwcBleLinkTelemetryValid = FALSE;
    s_SwcBleLinkSilentTicks = 0UL;
    s_SwcBleLinkLastRxByte = 0U;
    s_SwcBleLinkActivityCounter = 0U;
    s_SwcBleLinkDiagCounter = 0U;

    s_SwcBleLinkTelemetry.PositionZone = SWC_BLELINK_POSITION_UNKNOWN;
    s_SwcBleLinkTelemetry.RssiLevel = 0U;
    s_SwcBleLinkTelemetry.DistanceCm = SWC_BLELINK_DISTANCE_UNKNOWN;
    s_SwcBleLinkTelemetry.Flags = 0U;
}

void Swc_BleLink_MainFunction(void)
{
    if (s_SwcBleLinkConnected == TRUE)
    {
        if (s_SwcBleLinkSilentTicks < SWC_BLELINK_LINK_LOST_TICKS)
        {
            s_SwcBleLinkSilentTicks++;
        }
        else
        {
            s_SwcBleLinkConnected = FALSE;
            s_SwcBleLinkAuthenticated = FALSE;
            s_SwcBleLinkTelemetryValid = FALSE;
            s_SwcBleLinkDiagCounter++;
        }
    }
}

void Swc_BleLink_OnRxByte(uint8 RxByte)
{
    s_SwcBleLinkConnected = TRUE;
    s_SwcBleLinkSilentTicks = 0UL;
    s_SwcBleLinkLastRxByte = RxByte;
    s_SwcBleLinkActivityCounter++;
}

void Swc_BleLink_SetAuthenticated(boolean IsAuthenticated)
{
    s_SwcBleLinkAuthenticated = IsAuthenticated;

    if (IsAuthenticated == TRUE)
    {
        s_SwcBleLinkConnected = TRUE;
        s_SwcBleLinkSilentTicks = 0UL;
    }
}

void Swc_BleLink_UpdateTelemetry(
    const Swc_BleLink_TelemetryType *TelemetryPtr
)
{
    if (TelemetryPtr != NULL_PTR)
    {
        s_SwcBleLinkTelemetry = *TelemetryPtr;
        s_SwcBleLinkTelemetryValid = TRUE;
        s_SwcBleLinkConnected = TRUE;
        s_SwcBleLinkSilentTicks = 0UL;
    }
}

void Swc_BleLink_GetCanInfo(uint8 *InfoPtr, uint8 Length)
{
    if ((InfoPtr != NULL_PTR) && (Length >= SWC_BLELINK_CAN_INFO_LENGTH))
    {
        InfoPtr[0U] = Swc_BleLink_BuildFlags();
        InfoPtr[1U] = s_SwcBleLinkTelemetry.PositionZone;
        InfoPtr[2U] = s_SwcBleLinkTelemetry.RssiLevel;
        InfoPtr[3U] = s_SwcBleLinkTelemetry.DistanceCm;
        InfoPtr[4U] = s_SwcBleLinkLastRxByte;
        InfoPtr[5U] = s_SwcBleLinkActivityCounter;
        InfoPtr[6U] = s_SwcBleLinkDiagCounter;
    }
}

boolean Swc_BleLink_IsConnected(void)
{
    boolean isConnected = FALSE;

    isConnected = s_SwcBleLinkConnected;

    return isConnected;
}

boolean Swc_BleLink_IsAuthenticated(void)
{
    boolean isAuthenticated = FALSE;

    isAuthenticated = s_SwcBleLinkAuthenticated;

    return isAuthenticated;
}

boolean Swc_BleLink_IsStrongProximity(void)
{
    boolean isStrong = FALSE;

    isStrong = Swc_BleLink_IsTelemetryStrong();

    return isStrong;
}

