#ifndef _NVT_CRYPTO_H_
#define _NVT_CRYPTO_H_

#define NVT_CRYPTO_AES_BLOCK_SIZE       16  ///< bytes
#define NVT_CRYPTO_AES_128_KEY_SIZE     16  ///< bytes
#define NVT_CRYPTO_AES_256_KEY_SIZE     32  ///< bytes

typedef enum {
	NVT_CRYPTO_MODE_DES = 0,                ///< Block Size => 64  bits, Key Size => 64  bits, Single Block Cipher
	NVT_CRYPTO_MODE_3DES,                   ///< Block Size => 64  bits, Key Size => 192 bits, Single Block Cipher
	NVT_CRYPTO_MODE_AES_128,                ///< Block Size => 128 bits, Key Size => 128 bits, Single Block Cipher
	NVT_CRYPTO_MODE_AES_256,                ///< Block Size => 128 bits, Key Size => 256 bits, Single Block Cipher
	NVT_CRYPTO_MODE_MAX
} NVT_CRYPTO_MODE_T;

typedef enum {
	NVT_CRYPTO_OPMODE_ECB = 0,              ///< ECB mode
	NVT_CRYPTO_OPMODE_CBC,                  ///< CBC mode, need IV
	NVT_CRYPTO_OPMODE_MAX
} NVT_CRYPTO_OPMODE_T;

typedef enum {
	NVT_CRYPTO_KEY_SRC_EFUSE = 0,           ///< key from efuse
	NVT_CRYPTO_KEY_SRC_DATA,                ///< key from user key data
	NVT_CRYPTO_KEY_SRC_MAX
} NVT_CRYPTO_KEY_SRC_T;

struct nvt_crypto_pio_t {
	uint32_t             encrypt;           ///< 0:decrypt 1:encrypt
	NVT_CRYPTO_OPMODE_T  opmode;            ///< 0:ecb 1:cbc
	uint8_t              *src;              ///< input  data buffer
	uint8_t              *dst;              ///< output data buffer
	uint8_t              *key;              ///< key data buffer, key => NULL means from efuse secure boot key
	uint8_t              *iv;               ///< initial vector data buffer
	NVT_CRYPTO_KEY_SRC_T key_src;           ///< key from efuse or key buffer, 0:from efuse 1:frome key buffer
	uint32_t             key_ofs;           ///< key offset from efuse, word(4bytes) unit, used if key source from efuse
	uint32_t             src_size;          ///< input  data buffer length, bytes
	uint32_t             dst_size;          ///< output data buffer length, bytes
	uint32_t             key_size;          ///< key data buffer length, bytes, 16=>AES-128, 32=>AES-256
	uint32_t             iv_size;           ///< initial vector data buffer length, bytes
};

/*************************************************************************************
 *  Public Function Prototype
 *************************************************************************************/
#ifdef CONFIG_NVT_CRYPTO
void nvt_crypto_close(void);
int  nvt_crypto_open(void);
int  nvt_crypto_pio_aes(struct nvt_crypto_pio_t *p_crypto);
int  nvt_crypto_dma_aes(struct nvt_crypto_pio_t *p_crypto);
#else
static inline void nvt_crypto_close(void)
{
	return;
}

static inline int nvt_crypto_open(void)
{
	return -1;
}

static inline int nvt_crypto_pio_aes(struct nvt_crypto_pio_t *p_crypto)
{
	return -1;
}

static inline int nvt_crypto_dma_aes(struct nvt_crypto_pio_t *p_crypto)
{
	return -1;
}
#endif

#endif /* _NVT_CRYPTO_H_  */
