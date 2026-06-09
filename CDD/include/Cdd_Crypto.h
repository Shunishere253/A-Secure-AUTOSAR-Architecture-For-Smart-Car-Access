#ifndef _CDD_CRYPTO_H_
#define _CDD_CRYPTO_H_

#include "Std_Types.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
/**
* @brief   Initializes the crypto service.
* @details This function initializes the pseudo-random seed and AES-128 context.
*
* @param   -
*
* @return  void.
*
* @pre     BSW platform initialization shall initialize the crypto service before security access.
**/
void Cdd_Crypto_Init(void);

/**
* @brief   Refreshes the pseudo-random seed.
* @details This function refreshes the pseudo-random generator with external entropy.
*
* @param[in] Entropy - Entropy value used as seed.
*
* @return  void.
*
* @pre     Cdd_Crypto_Init should be called before this API.
**/
void Cdd_Crypto_RefreshSeed(uint32 Entropy);

/**
* @brief   Generates pseudo-random bytes.
* @details This function fills the requested buffer with pseudo-random byte values.
*
* @param[out] Buffer - Pointer to the destination buffer.
* @param[in]  Length - Number of bytes to generate.
*
* @return  void.
*
* @pre     Cdd_Crypto_Init should be called before this API.
**/
void Cdd_Crypto_GenerateRandom(uint8 *Buffer, uint32 Length);

/**
* @brief   Encrypts one AES-128 ECB block.
* @details This function copies plaintext to the output buffer and encrypts the first AES block.
*
* @param[in]  PlainText  - Pointer to plaintext input buffer.
* @param[out] CipherText - Pointer to ciphertext output buffer.
* @param[in]  Length     - Number of bytes copied before encryption.
*
* @return  void.
*
* @pre     PlainText and CipherText shall point to valid buffers. Length shall be at least one AES block.
**/
void Cdd_Crypto_Aes128Encrypt(const uint8 *PlainText, uint8 *CipherText, uint32 Length);

#endif /* _CDD_CRYPTO_H_ */

