#ifndef _CDD_AES_H_
#define _CDD_AES_H_

#include <stddef.h>
#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef CBC
#define CBC             (1U)
#endif /* CBC */

#ifndef ECB
#define ECB             (1U)
#endif /* ECB */

#ifndef CTR
#define CTR             (1U)
#endif /* CTR */

#define AES128          (1U)
#define AES_BLOCKLEN    (16U)

#if defined(AES256) && (AES256 == 1U)
#define AES_KEYLEN      (32U)
#define AES_keyExpSize  (240U)
#elif defined(AES192) && (AES192 == 1U)
#define AES_KEYLEN      (24U)
#define AES_keyExpSize  (208U)
#else
#define AES_KEYLEN      (16U)
#define AES_keyExpSize  (176U)
#endif /* defined(AES256) && (AES256 == 1U) */

struct AES_ctx
{
    uint8_t RoundKey[AES_keyExpSize];
#if (defined(CBC) && (CBC == 1U)) || (defined(CTR) && (CTR == 1U))
    uint8_t Iv[AES_BLOCKLEN];
#endif /* (defined(CBC) && (CBC == 1U)) || (defined(CTR) && (CTR == 1U)) */
};

/*******************************************************************************
 * API
 ******************************************************************************/
/**
* @brief   Initializes an AES context with a key.
* @details This function expands the AES key into the context round key buffer.
*
* @param[out] ctx - Pointer to AES context.
* @param[in]  key - Pointer to AES key bytes.
*
* @return  void.
*
* @pre     ctx and key shall point to valid buffers.
**/
void AES_init_ctx(struct AES_ctx *ctx, const uint8_t *key);

#if (defined(CBC) && (CBC == 1U)) || (defined(CTR) && (CTR == 1U))
/**
* @brief   Initializes an AES context with key and IV.
* @details This function expands the AES key and stores the IV into the AES context.
*
* @param[out] ctx - Pointer to AES context.
* @param[in]  key - Pointer to AES key bytes.
* @param[in]  iv  - Pointer to initialization vector bytes.
*
* @return  void.
*
* @pre     ctx, key and iv shall point to valid buffers.
**/
void AES_init_ctx_iv(struct AES_ctx *ctx, const uint8_t *key, const uint8_t *iv);

/**
* @brief   Updates the IV in an AES context.
* @details This function copies the IV bytes into the AES context.
*
* @param[in,out] ctx - Pointer to AES context.
* @param[in]     iv  - Pointer to initialization vector bytes.
*
* @return  void.
*
* @pre     ctx and iv shall point to valid buffers.
**/
void AES_ctx_set_iv(struct AES_ctx *ctx, const uint8_t *iv);
#endif /* (defined(CBC) && (CBC == 1U)) || (defined(CTR) && (CTR == 1U)) */

#if defined(ECB) && (ECB == 1U)
/**
* @brief   Encrypts one AES ECB block.
* @details This function encrypts exactly AES_BLOCKLEN bytes in-place.
*
* @param[in]     ctx - Pointer to initialized AES context.
* @param[in,out] buf - Pointer to one AES block.
*
* @return  void.
*
* @pre     ctx and buf shall point to valid buffers.
**/
void AES_ECB_encrypt(const struct AES_ctx *ctx, uint8_t *buf);

/**
* @brief   Decrypts one AES ECB block.
* @details This function decrypts exactly AES_BLOCKLEN bytes in-place.
*
* @param[in]     ctx - Pointer to initialized AES context.
* @param[in,out] buf - Pointer to one AES block.
*
* @return  void.
*
* @pre     ctx and buf shall point to valid buffers.
**/
void AES_ECB_decrypt(const struct AES_ctx *ctx, uint8_t *buf);
#endif /* defined(ECB) && (ECB == 1U) */

#if defined(CBC) && (CBC == 1U)
/**
* @brief   Encrypts an AES CBC buffer.
* @details This function encrypts the buffer in-place. Length shall be a multiple of AES_BLOCKLEN.
*
* @param[in,out] ctx    - Pointer to initialized AES context.
* @param[in,out] buf    - Pointer to buffer.
* @param[in]     length - Buffer length in bytes.
*
* @return  void.
*
* @pre     ctx and buf shall point to valid buffers. length shall be a multiple of AES_BLOCKLEN.
**/
void AES_CBC_encrypt_buffer(struct AES_ctx *ctx, uint8_t *buf, size_t length);

/**
* @brief   Decrypts an AES CBC buffer.
* @details This function decrypts the buffer in-place. Length shall be a multiple of AES_BLOCKLEN.
*
* @param[in,out] ctx    - Pointer to initialized AES context.
* @param[in,out] buf    - Pointer to buffer.
* @param[in]     length - Buffer length in bytes.
*
* @return  void.
*
* @pre     ctx and buf shall point to valid buffers. length shall be a multiple of AES_BLOCKLEN.
**/
void AES_CBC_decrypt_buffer(struct AES_ctx *ctx, uint8_t *buf, size_t length);
#endif /* defined(CBC) && (CBC == 1U) */

#if defined(CTR) && (CTR == 1U)
/**
* @brief   Applies AES CTR operation to a buffer.
* @details This function encrypts or decrypts the buffer in-place and increments the context IV.
*
* @param[in,out] ctx    - Pointer to initialized AES context.
* @param[in,out] buf    - Pointer to buffer.
* @param[in]     length - Buffer length in bytes.
*
* @return  void.
*
* @pre     ctx and buf shall point to valid buffers.
**/
void AES_CTR_xcrypt_buffer(struct AES_ctx *ctx, uint8_t *buf, size_t length);
#endif /* defined(CTR) && (CTR == 1U) */

#endif /* _CDD_AES_H_ */

