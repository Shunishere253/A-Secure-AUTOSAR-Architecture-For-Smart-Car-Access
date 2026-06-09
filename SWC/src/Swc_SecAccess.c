#include "Swc_SecAccess.h"

#include "Cdd_Crypto.h"
#include "Cdd_Jdy23.h"

#include <string.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SWC_SECACCESS_START_RX_LEN              (3U)
#define SWC_SECACCESS_DATA_LEN                  (16U)

#define SWC_SECACCESS_START_BYTE_0              (0x01U)
#define SWC_SECACCESS_START_BYTE_1              (0x02U)
#define SWC_SECACCESS_START_BYTE_2              (0x03U)

#define SWC_SECACCESS_VERIFY_TIMEOUT_TICKS      (500000UL)
#define SWC_SECACCESS_PASS_TEXT                 ((const uint8 *)"PASS!\r\n")
#define SWC_SECACCESS_FAIL_TEXT                 ((const uint8 *)"FAIL!\r\n")
#define SWC_SECACCESS_RESULT_TEXT_LENGTH        (7U)

typedef enum
{
    SWC_SECACCESS_STATE_WAIT_START = 0U,
    SWC_SECACCESS_STATE_WAIT_VERIFY
} Swc_SecAccess_StateType;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void Swc_SecAccess_ClearBuffers(void);
static void Swc_SecAccess_ResetStartBuffer(void);
static void Swc_SecAccess_PushStartByte(uint8 NewByte);
static boolean Swc_SecAccess_IsStartMatched(void);
static void Swc_SecAccess_ResetState(void);
static Swc_SecAccess_ResultType Swc_SecAccess_StartSession(void);
static Swc_SecAccess_ResultType Swc_SecAccess_CheckVerify(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8 s_SwcSecAccessStartBuffer[SWC_SECACCESS_START_RX_LEN];
static uint8 s_SwcSecAccessChallengeBuffer[SWC_SECACCESS_DATA_LEN];
static uint8 s_SwcSecAccessEncryptedBuffer[SWC_SECACCESS_DATA_LEN];
static uint8 s_SwcSecAccessVerifyBuffer[SWC_SECACCESS_DATA_LEN];

static Swc_SecAccess_StateType s_SwcSecAccessState =
    SWC_SECACCESS_STATE_WAIT_START;

static uint8 s_SwcSecAccessVerifyIndex = 0U;
static uint32 s_SwcSecAccessVerifyTimeout = 0UL;

static volatile uint8 s_SwcSecAccessDebugState = 0U;
static volatile uint8 s_SwcSecAccessLastByte = 0U;
static volatile uint8 s_SwcSecAccessSendResult = 0U;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void Swc_SecAccess_ClearBuffers(void)
{
    uint32 index = 0UL;

    for (index = 0U; index < SWC_SECACCESS_START_RX_LEN; index++)
    {
        s_SwcSecAccessStartBuffer[index] = 0x00U;
    }

    for (index = 0U; index < SWC_SECACCESS_DATA_LEN; index++)
    {
        s_SwcSecAccessChallengeBuffer[index] = 0x00U;
        s_SwcSecAccessEncryptedBuffer[index] = 0x00U;
        s_SwcSecAccessVerifyBuffer[index] = 0x00U;
    }

    s_SwcSecAccessVerifyIndex = 0U;
    s_SwcSecAccessVerifyTimeout = 0UL;
}

static void Swc_SecAccess_ResetStartBuffer(void)
{
    s_SwcSecAccessStartBuffer[0U] = 0x00U;
    s_SwcSecAccessStartBuffer[1U] = 0x00U;
    s_SwcSecAccessStartBuffer[2U] = 0x00U;
}

static void Swc_SecAccess_PushStartByte(uint8 NewByte)
{
    s_SwcSecAccessStartBuffer[0U] = s_SwcSecAccessStartBuffer[1U];
    s_SwcSecAccessStartBuffer[1U] = s_SwcSecAccessStartBuffer[2U];
    s_SwcSecAccessStartBuffer[2U] = NewByte;
}

static boolean Swc_SecAccess_IsStartMatched(void)
{
    boolean isMatched = FALSE;

    if ((s_SwcSecAccessStartBuffer[0U] == SWC_SECACCESS_START_BYTE_0) &&
        (s_SwcSecAccessStartBuffer[1U] == SWC_SECACCESS_START_BYTE_1) &&
        (s_SwcSecAccessStartBuffer[2U] == SWC_SECACCESS_START_BYTE_2))
    {
        isMatched = TRUE;
    }

    return isMatched;
}

static void Swc_SecAccess_ResetState(void)
{
    Swc_SecAccess_ClearBuffers();
    s_SwcSecAccessState = SWC_SECACCESS_STATE_WAIT_START;
    s_SwcSecAccessDebugState = 0U;
}

static Swc_SecAccess_ResultType Swc_SecAccess_StartSession(void)
{
    uint32 harvestedEntropy = 0UL;
    Swc_SecAccess_ResultType result = SWC_SECACCESS_RESULT_ACTIVE;

    Swc_SecAccess_ClearBuffers();
    s_SwcSecAccessState = SWC_SECACCESS_STATE_WAIT_VERIFY;

    harvestedEntropy = Cdd_Jdy23_GetEntropy();
    Cdd_Crypto_RefreshSeed(harvestedEntropy);
    s_SwcSecAccessDebugState = 3U;

    Cdd_Crypto_GenerateRandom(
        s_SwcSecAccessChallengeBuffer,
        SWC_SECACCESS_DATA_LEN
    );
    s_SwcSecAccessDebugState = 4U;

    Cdd_Crypto_Aes128Encrypt(
        s_SwcSecAccessChallengeBuffer,
        s_SwcSecAccessEncryptedBuffer,
        SWC_SECACCESS_DATA_LEN
    );
    s_SwcSecAccessDebugState = 5U;

    if (Cdd_Jdy23_Send(
            s_SwcSecAccessChallengeBuffer,
            SWC_SECACCESS_DATA_LEN
        ) == E_OK)
    {
        s_SwcSecAccessSendResult = 2U;
        s_SwcSecAccessDebugState = 6U;
        s_SwcSecAccessVerifyTimeout = 0UL;
        s_SwcSecAccessVerifyIndex = 0U;
        Swc_SecAccess_ResetStartBuffer();
    }
    else
    {
        s_SwcSecAccessSendResult = 1U;
        s_SwcSecAccessDebugState = 101U;
        Swc_SecAccess_ResetState();
        result = SWC_SECACCESS_RESULT_FAIL;
    }

    return result;
}

static Swc_SecAccess_ResultType Swc_SecAccess_CheckVerify(void)
{
    Swc_SecAccess_ResultType result = SWC_SECACCESS_RESULT_FAIL;

    s_SwcSecAccessDebugState = 9U;

    if (memcmp(
            s_SwcSecAccessEncryptedBuffer,
            s_SwcSecAccessVerifyBuffer,
            SWC_SECACCESS_DATA_LEN
        ) == 0)
    {
        s_SwcSecAccessDebugState = 10U;
        (void)Cdd_Jdy23_Send(SWC_SECACCESS_PASS_TEXT, SWC_SECACCESS_RESULT_TEXT_LENGTH);
        result = SWC_SECACCESS_RESULT_PASS;
    }
    else
    {
        s_SwcSecAccessDebugState = 11U;
        (void)Cdd_Jdy23_Send(SWC_SECACCESS_FAIL_TEXT, SWC_SECACCESS_RESULT_TEXT_LENGTH);
    }

    Swc_SecAccess_ResetState();

    return result;
}

void Swc_SecAccess_Init(void)
{
    Swc_SecAccess_ResetState();
}

void Swc_SecAccess_MainFunction(void)
{
    if (s_SwcSecAccessState == SWC_SECACCESS_STATE_WAIT_VERIFY)
    {
        s_SwcSecAccessVerifyTimeout++;

        if (s_SwcSecAccessVerifyTimeout > SWC_SECACCESS_VERIFY_TIMEOUT_TICKS)
        {
            s_SwcSecAccessDebugState = 102U;
            Swc_SecAccess_ResetState();
        }
    }
}

Swc_SecAccess_ResultType Swc_SecAccess_ProcessRxByte(uint8 RxByte)
{
    Swc_SecAccess_ResultType result = SWC_SECACCESS_RESULT_ACTIVE;

    s_SwcSecAccessLastByte = RxByte;
    Swc_SecAccess_PushStartByte(RxByte);

    if (s_SwcSecAccessState == SWC_SECACCESS_STATE_WAIT_START)
    {
        s_SwcSecAccessDebugState = 1U;

        if (Swc_SecAccess_IsStartMatched() == TRUE)
        {
            s_SwcSecAccessDebugState = 2U;
            result = Swc_SecAccess_StartSession();
        }
    }
    else
    {
        s_SwcSecAccessDebugState = 8U;
        s_SwcSecAccessVerifyTimeout = 0UL;

        s_SwcSecAccessVerifyBuffer[s_SwcSecAccessVerifyIndex] = RxByte;
        s_SwcSecAccessVerifyIndex++;

        if (Swc_SecAccess_IsStartMatched() == TRUE)
        {
            s_SwcSecAccessDebugState = 103U;
            result = Swc_SecAccess_StartSession();
        }
        else if (s_SwcSecAccessVerifyIndex >= SWC_SECACCESS_DATA_LEN)
        {
            result = Swc_SecAccess_CheckVerify();
        }
        else
        {
            result = SWC_SECACCESS_RESULT_ACTIVE;
        }
    }

    return result;
}

Std_ReturnType Swc_SecAccess_Process(void)
{
    uint8 rxByte = 0U;
    Swc_SecAccess_ResultType result = SWC_SECACCESS_RESULT_IDLE;
    Std_ReturnType ret = E_NOT_OK;

    while ((result != SWC_SECACCESS_RESULT_PASS) &&
           (result != SWC_SECACCESS_RESULT_FAIL))
    {
        if (Cdd_Jdy23_TryReceiveByte(&rxByte) == E_OK)
        {
            result = Swc_SecAccess_ProcessRxByte(rxByte);
        }

        Swc_SecAccess_MainFunction();
    }

    if (result == SWC_SECACCESS_RESULT_PASS)
    {
        ret = E_OK;
    }

    return ret;
}

