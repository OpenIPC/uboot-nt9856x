#ifndef _NVT_HASH_H_
#define _NVT_HASH_H_

typedef enum {
	NVT_HASH_MODE_SHA1 = 0,             ///< block size=> 64 bytes, digest size 20 bytes
	NVT_HASH_MODE_SHA256,               ///< block size=> 64 bytes, digest size 32 bytes
	NVT_HASH_MODE_MAX
} NVT_HASH_MODE_T;

#define NVT_HASH_SHA1_DIGEST_SIZE       20
#define NVT_HASH_SHA1_BLOCK_SIZE        64
#define NVT_HASH_SHA256_DIGEST_SIZE     32
#define NVT_HASH_SHA256_BLOCK_SIZE      64

struct nvt_hash_pio_t {
	uint32_t  mode;                     ///< hash mode
	uint8_t   *src;                     ///< input  data buffer
	uint8_t   *digest;                  ///< digest data buffer
	uint32_t  src_size;                 ///< input  data buffer length
	uint32_t  digest_size;              ///< digest data buffer length
};

/*************************************************************************************
 *  Public Function Prototype
 *************************************************************************************/
#ifdef CONFIG_NVT_HASH
void nvt_hash_close(void);
int  nvt_hash_open(void);
int  nvt_hash_pio_sha(struct nvt_hash_pio_t *p_hash);
int  nvt_hash_dma_sha(struct nvt_hash_pio_t *p_hash);
#else
static inline void nvt_hash_close(void)
{
	return;
}

static inline int nvt_hash_open(void)
{
	return -1;
}

static inline int nvt_hash_pio_sha(struct nvt_hash_pio_t *p_hash)
{
	return -1;
}

static inline int nvt_hash_dma_sha(struct nvt_hash_pio_t *p_hash)
{
	return -1;
}
#endif

#endif /* _NVT_HASH_H_  */
