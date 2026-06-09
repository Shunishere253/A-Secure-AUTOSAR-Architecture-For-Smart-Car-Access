#include "Cdd_Jdy23.h"

#include "CDD_Uart.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CDD_JDY23_RX_TIMEOUT_TICKS            (0xFFFFFFUL)
#define CDD_JDY23_TX_TIMEOUT_TICKS            (0xFFFFFFUL)
#define CDD_JDY23_RX_DEFAULT_TIMEOUT_TICKS    (0x2FFFFUL)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void Cdd_Jdy23_MixEntropy(uint32 Value);
static Std_ReturnType Cdd_Jdy23_StartAsyncReceive(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32 s_CddJdy23EntropyValue = 0xA5A55A5AUL;
static uint8 s_CddJdy23RxByte = 0U;
static boolean s_CddJdy23RxBusy = FALSE;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void Cdd_Jdy23_MixEntropy(uint32 Value)
{
    s_CddJdy23EntropyValue ^= Value;
    s_CddJdy23EntropyValue ^= (s_CddJdy23EntropyValue << 13U);
    s_CddJdy23EntropyValue ^= (s_CddJdy23EntropyValue >> 17U);
    s_CddJdy23EntropyValue ^= (s_CddJdy23EntropyValue << 5U);
}

static Std_ReturnType Cdd_Jdy23_StartAsyncReceive(void)
{
    Std_ReturnType ret = E_NOT_OK;

    ret = Uart_AsyncReceive(
        UART_LPUART_INTERNAL_CHANNEL,
        &s_CddJdy23RxByte,
        1U
    );

    if (ret == E_OK)
    {
        s_CddJdy23RxBusy = TRUE;
    }

    return ret;
}

Std_ReturnType Cdd_Jdy23_Receive(uint8 *Buffer, uint32 Length)
{
    Std_ReturnType ret = E_NOT_OK;

    if ((Buffer != NULL_PTR) && (Length > 0UL))
    {
        ret = Cdd_Jdy23_ReceiveWithTimeout(
            Buffer,
            Length,
            CDD_JDY23_RX_DEFAULT_TIMEOUT_TICKS
        );
    }

    return ret;
}

Std_ReturnType Cdd_Jdy23_ReceiveByte(uint8 *Byte)
{
    Std_ReturnType ret = E_NOT_OK;

    if (Byte != NULL_PTR)
    {
        ret = Cdd_Jdy23_Receive(Byte, 1U);
    }

    return ret;
}

Std_ReturnType Cdd_Jdy23_TryReceiveByte(uint8 *Byte)
{
    volatile Std_ReturnType uartStatus = E_NOT_OK;
    volatile Uart_StatusType uartReceiveStatus = UART_STATUS_OPERATION_ONGOING;
    Std_ReturnType ret = E_NOT_OK;
    uint32 bytesRemaining = 0UL;

    if (Byte != NULL_PTR)
    {
        if (s_CddJdy23RxBusy == FALSE)
        {
            uartStatus = Cdd_Jdy23_StartAsyncReceive();
            (void)uartStatus;
        }
        else
        {
            bytesRemaining = 1UL;
            uartReceiveStatus = Uart_GetStatus(
                UART_LPUART_INTERNAL_CHANNEL,
                &bytesRemaining,
                UART_RECEIVE
            );

            if (uartReceiveStatus == UART_STATUS_NO_ERROR)
            {
                *Byte = s_CddJdy23RxByte;
                s_CddJdy23RxBusy = FALSE;
                Cdd_Jdy23_MixEntropy(((uint32)s_CddJdy23RxByte) ^ bytesRemaining);
                uartStatus = Cdd_Jdy23_StartAsyncReceive();
                (void)uartStatus;
                ret = E_OK;
            }
            else if (uartReceiveStatus != UART_STATUS_OPERATION_ONGOING)
            {
                s_CddJdy23RxBusy = FALSE;
                (void)Uart_Abort(UART_LPUART_INTERNAL_CHANNEL, UART_RECEIVE);
            }
            else
            {
                /* Receive operation is still pending. */
            }
        }
    }

    return ret;
}

Std_ReturnType Cdd_Jdy23_Send(const uint8 *Buffer, uint32 Length)
{
    volatile Std_ReturnType uartStatus = E_NOT_OK;
    volatile Uart_StatusType uartTransmitStatus = UART_STATUS_OPERATION_ONGOING;
    Std_ReturnType ret = E_NOT_OK;
    uint32 bytesRemaining = 0UL;
    uint32 timeout = 0UL;
    uint32 localCounter = 0UL;

    if ((Buffer != NULL_PTR) && (Length > 0UL))
    {
        bytesRemaining = Length;
        timeout = CDD_JDY23_TX_TIMEOUT_TICKS;

        uartStatus = Uart_AsyncSend(
            UART_LPUART_INTERNAL_CHANNEL,
            Buffer,
            Length
        );

        if (uartStatus == E_OK)
        {
            do
            {
                localCounter++;

                uartTransmitStatus = Uart_GetStatus(
                    UART_LPUART_INTERNAL_CHANNEL,
                    &bytesRemaining,
                    UART_SEND
                );

                timeout--;
            }
            while ((uartTransmitStatus != UART_STATUS_NO_ERROR) && (timeout > 0UL));

            Cdd_Jdy23_MixEntropy(localCounter ^ bytesRemaining ^ Length ^ 0x55AA55AAUL);

            if (uartTransmitStatus != UART_STATUS_NO_ERROR)
            {
                (void)Uart_Abort(UART_LPUART_INTERNAL_CHANNEL, UART_SEND);
            }
            else
            {
                ret = E_OK;
            }
        }
    }

    return ret;
}

uint32 Cdd_Jdy23_GetEntropy(void)
{
    uint32 entropyValue = 0UL;

    Cdd_Jdy23_MixEntropy(0x3C6EF372UL);
    entropyValue = s_CddJdy23EntropyValue;

    return entropyValue;
}

Std_ReturnType Cdd_Jdy23_ReceiveWithTimeout(
    uint8 *Buffer,
    uint32 Length,
    uint32 TimeoutTicks
)
{
    volatile Std_ReturnType uartStatus = E_NOT_OK;
    volatile Uart_StatusType uartReceiveStatus = UART_STATUS_OPERATION_ONGOING;
    Std_ReturnType ret = E_NOT_OK;
    uint32 bytesRemaining = 0UL;
    uint32 timeout = 0UL;
    uint32 localCounter = 0UL;

    if ((Buffer != NULL_PTR) && (Length > 0UL) && (TimeoutTicks > 0UL))
    {
        bytesRemaining = Length;
        timeout = TimeoutTicks;

        if (s_CddJdy23RxBusy == TRUE)
        {
            (void)Uart_Abort(UART_LPUART_INTERNAL_CHANNEL, UART_RECEIVE);
            s_CddJdy23RxBusy = FALSE;
        }

        uartStatus = Uart_AsyncReceive(
            UART_LPUART_INTERNAL_CHANNEL,
            Buffer,
            Length
        );

        if (uartStatus == E_OK)
        {
            do
            {
                localCounter++;

                uartReceiveStatus = Uart_GetStatus(
                    UART_LPUART_INTERNAL_CHANNEL,
                    &bytesRemaining,
                    UART_RECEIVE
                );

                timeout--;
            }
            while ((uartReceiveStatus != UART_STATUS_NO_ERROR) && (timeout > 0UL));

            s_CddJdy23EntropyValue ^= localCounter;
            s_CddJdy23EntropyValue ^= bytesRemaining;
            s_CddJdy23EntropyValue ^= Length;

            if (uartReceiveStatus != UART_STATUS_NO_ERROR)
            {
                (void)Uart_Abort(UART_LPUART_INTERNAL_CHANNEL, UART_RECEIVE);
                s_CddJdy23RxBusy = FALSE;
            }
            else
            {
                ret = E_OK;
            }
        }
    }

    return ret;
}

void Cdd_Jdy23_ResetSession(void)
{
    (void)Uart_Abort(UART_LPUART_INTERNAL_CHANNEL, UART_RECEIVE);
    (void)Uart_Abort(UART_LPUART_INTERNAL_CHANNEL, UART_SEND);

    s_CddJdy23RxBusy = FALSE;
    s_CddJdy23EntropyValue ^= 0xA5965A69UL;
}
