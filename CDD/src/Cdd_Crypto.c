#include <Cdd_Aes.h>
#include "Cdd_Crypto.h"

#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CDD_CRYPTO_AES128_KEY_LENGTH       (16U)
#define CDD_CRYPTO_RANDOM_BYTE_RANGE       (256U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const uint8 s_CddCryptoAesKey[CDD_CRYPTO_AES128_KEY_LENGTH] =
{
    0x2BU, 0x7EU, 0x15U, 0x16U,
    0x28U, 0xAEU, 0xD2U, 0xA6U,
    0xABU, 0xF7U, 0x15U, 0x88U,
    0x09U, 0xCFU, 0x4FU, 0x3CU
};

static struct AES_ctx s_CddCryptoAesCtx;

/*******************************************************************************
 * Code
 ******************************************************************************/
void Cdd_Crypto_Init(void)
{
    srand((unsigned int)0U);
    AES_init_ctx(&s_CddCryptoAesCtx, s_CddCryptoAesKey);
}

void Cdd_Crypto_RefreshSeed(uint32 Entropy)
{
    srand((unsigned int)Entropy);
}

void Cdd_Crypto_GenerateRandom(uint8 *Buffer, uint32 Length)
{
    uint32 index = 0UL;

    if ((Buffer != NULL_PTR) && (Length > 0UL))
    {
        for (index = 0U; index < Length; index++)
        {
            Buffer[index] = (uint8)(rand() % (int)CDD_CRYPTO_RANDOM_BYTE_RANGE);
        }
    }
}

void Cdd_Crypto_Aes128Encrypt(const uint8 *PlainText, uint8 *CipherText, uint32 Length)
{
    if ((PlainText != NULL_PTR) && (CipherText != NULL_PTR) && (Length >= AES_BLOCKLEN))
    {
        (void)memcpy(CipherText, PlainText, Length);

        AES_init_ctx(&s_CddCryptoAesCtx, s_CddCryptoAesKey);
        AES_ECB_encrypt(&s_CddCryptoAesCtx, CipherText);
    }
}

