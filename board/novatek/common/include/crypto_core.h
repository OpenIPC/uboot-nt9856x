/*
*  crypto_core.h
*
*  Author:	Leo Lin
*  Created:	November 27, 2015
*  Copyright:	Novatek Inc.
*
*/

#ifndef _CRYPTO_CORE_H_
#define _CRYPTO_CORE_H_

//#include <ntreg.h>

#define BIT_30	0x40000000

#define	COPY_SECURE_TO_CRYPT_CMD		0x01
#define	COPY_SECURE_TO_CRYPT			0x02
#define COPY_MASTER_TO_CRYPT 	 		0x01

#define UUIDLEN 	16
#define SHA256LEN 	32

#define                         CPECMD_WRITE(data)  {\
				unsigned long					gulXXX;\
                                 *((volatile unsigned long *)(0xFC1B7008))=data;\
                                 gulXXX=*((volatile unsigned long *)(0xFC1B7008));\
                                }


enum {
   CRYPTO_OK=0,             ///< Result OK 
   CRYPTO_ERROR,            ///< Generic Error 
   CRYPTO_NOP,              ///< Not a failure but no operation was performed 
   CRYPTO_INVALID_ARG,      ///< Generic invalid argument 
   CRYPTO_ERROR_READPRNG,   ///< Could not read enough from PRNG 

   CRYPTO_MEM,              ///< Out of memory */
};

//=========================================
#define DEC56CBC_DECRYPT		0x00000000
#define DEC56CBC_ENCRYPT		0x00000001
#define	DEC56ECB_DECRYPT		0x00010000
#define	DEC56ECB_ENCRYPT		0x00010001

#define	TDESECB_DECRYPT			0x00000004
#define	TDESECB_ENCRYPT			0x00000005

#define	AES128CBC_DECRYPT		0x00000008
#define	AES128CBC_ENCRYPT		0x00000009
#define	AES128ECB_DECRYPT		0x00010008
#define	AES128ECB_ENCRYPT		0x00010009
#define	AES128CTR_DECRYPT		0x00020008
#define	AES128CTR_ENCRYPT		0x00020009

#define	MULTI2CBC_DECRYPT		0x0000000C
#define	MULTI2CBC_ENCRYPT		0x0000000D
#define	MULTI2ECB_DECRYPT		0x0001000C
#define	MULTI2ECB_ENCRYPT		0x0001000D

#define	AES128XCBC_MAC			0x00000013
#define	AES128XCBC_MAC_NONPAD	0x00000113

#define	SHA256					0x00000022
#define	SHA1					0x0000002A

#define	PRNG					0x00100000

#define TERM_SOL_MODE			0x20000000
#define TS_MODE					0x10000000
#define TS_ODDKEY_MODE			0x03000000
#define TS_EVENKEY_MODE			0x02000000

//=========================================
#define	SHA256_DIGEST_SIZE		0x00000020	//bytes

//=========================================
#define CRYPT_STAT_TIMEOUT		0x00000001
#define CRYPT_STAT_DONE			0x00000004

#define NVTSFWUP_KEY_LEN 		16
#define NVTSFWUP_VTBL_MAX_SZ 		(32*1024)
#define NVTSFWUP_HW_BUF 		0x5A00000//90MB as physical buffer

// Cryptograph register
typedef struct _CRYPTO_REG 
{
	volatile unsigned long ulCLR_INT_ST;
	volatile unsigned long ulINTStatus;
	volatile unsigned long ulTimeOut;
	volatile unsigned long ulReserve0;
	
	volatile unsigned long ulSrcAddr;
	volatile unsigned long ulSrcLen;

	volatile unsigned long ulDstAddr;
	volatile unsigned long ulDstLen;
	
	volatile unsigned long ulIV[4];
	volatile unsigned long ulKEY[8];
	
	volatile unsigned long ulFuncType;
										
	volatile unsigned long ulNoUse1[3];

	volatile unsigned long ulFuncFire;
	volatile unsigned long ulAXIBusID;
	volatile unsigned long ulNoUse2[2];
	volatile unsigned long ulSecondary_key[4];
	volatile unsigned long ulM2DKey[2];
	volatile unsigned long ulNoUse3[2];
	volatile unsigned long ulRO_DLY_SEL;
	volatile unsigned long ulEntropyCtrl;
	volatile unsigned long ulEntropyVal;
	volatile unsigned long ul0x9C_undefined;
	volatile unsigned long ulIVfPRNG[4];
	volatile unsigned long ulPRNGHold;
	volatile unsigned long ulNoUse4[3]; 
	volatile unsigned long ulM2OutMsg;
	volatile unsigned long ulNoUse5[7];
	volatile unsigned long ulKeySelect;										
										
	volatile unsigned long ulIVSelect;												
									
	volatile unsigned long ul0xE8_undefined;

	volatile unsigned long ulNoUse6[6];

	volatile unsigned long ulLock_rout;
	
	volatile unsigned long ulLockKey;
	volatile unsigned long ulLockIV;

	volatile unsigned long ul0x10C_undefined[60];

	volatile unsigned long ulTS_IV_DEF[4];

	volatile unsigned long ulTS_IV_EVN[4];

	volatile unsigned long ulTS_IV_ODD[4];

	volatile unsigned long ulTS_KEY_DEF[8];

	volatile unsigned long ulTS_KEY_EVN[8];

	volatile unsigned long ulTS_KEY_ODD[8];

	volatile unsigned long ulTS_KEY2_DEF[2];

	volatile unsigned long ulTS_KEY2_EVN[2];

	volatile unsigned long ulTS_KEY2_ODD[2];

	volatile unsigned long ul0x2A8_undefined[150];

	volatile unsigned long ulROUT_VAL[8];


} CRYPTO_REG, *PCRYPTO_REG;


/* 
  * 32-bit integer manipulation macros (big endian) 
  */
#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)				\
{ 										\
	(n) = ( (unsigned long) (b)[(i)    ] << 24 )		\
		| ( (unsigned long) (b)[(i) + 1] << 16 )	\
		| ( (unsigned long) (b)[(i) + 2] <<  8 )	\
		| ( (unsigned long) (b)[(i) + 3]       );		\
}
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)				\
{										\
	(b)[(i)] = (unsigned char) ( (n & 0xFF000000) >> 24 );		\
	(b)[(i) + 1] = (unsigned char) ( (n & 0x00FF0000) >> 16 );	\
	(b)[(i) + 2] = (unsigned char) ( (n & 0x0000FF00) >>  8 );		\
	(b)[(i) + 3] = (unsigned char) ( (n & 0x000000FF)	);		\
}
#endif

/*
  * 32-bit integer manipulation macros (little endian)
  */
#ifndef GET_ULONG_LE
#define GET_ULONG_LE(n,b,i)				\
{										\
	(n) = ( (unsigned long) (b)[(i)    ]       )		\
		| ( (unsigned long) (b)[(i) + 1] <<  8 )	\
		| ( (unsigned long) (b)[(i) + 2] << 16 )	\
		| ( (unsigned long) (b)[(i) + 3] << 24 );	\
}
#endif

#ifndef PUT_ULONG_LE
#define PUT_ULONG_LE(n,b,i)				\
{										\
	(b)[(i)    ] = (unsigned char) ( (n & 0x000000FF)       );		\
	(b)[(i) + 1] = (unsigned char) ( (n & 0x0000FF00) >>  8 );		\
	(b)[(i) + 2] = (unsigned char) ( (n & 0x00FF0000) >> 16 );	\
	(b)[(i) + 3] = (unsigned char) ( (n & 0xFF000000) >> 24 );	\
}
#endif

typedef struct
{
	unsigned long ulType;
	unsigned long total[2]; /*!< number of bytes processed  */
	unsigned char *pu8Buffer;	/*!< data block being processed */
	//unsigned char *pbuffer; /*!< data block being processed */
	int			offset;
} SHA2_CONTEXT, *pSHA2_CONTEXT;


int CryptoCoreInit(void);

/**
 * @brief		Encrypt messages by AES ECB master key in one call
 * @param		[in] ibuf
 *				pointer to buffer where raw data is
 * @param		[out] obuf
 *				pointer to buffer where encrypted data will be stored
 * @param		[in] len
 *				size of message to be encrypted
 * @return	    Returns zero on success and value other than zero on error.
 */
int enc(unsigned char *ibuf, unsigned char *obuf, unsigned int len);

/**
 * @brief		Decrypt messages by AES ECB master key in one call
 * @param		[in] ibuf
 *				pointer to buffer where raw data is
 * @param		[out] obuf
 *				pointer to buffer where decrypted data will be stored
 * @param		[in] len
 *				size of message to be decrypted
 * @return	    Returns zero on success and value other than zero on error.
 */
int dec(unsigned char *ibuf, unsigned char *obuf, unsigned int len);

/**
 * @brief		Encrypt messages by AES ECB secure key in one call
 * @param		[in] ibuf
 *				pointer to buffer where raw data is
 * @param		[out] obuf
 *				pointer to buffer where encrypted data will be stored
 * @param		[in] len
 *				size of message to be encrypted
 * @return	    Returns zero on success and value other than zero on error.
 */
int cyp_enc(unsigned char *ibuf, unsigned char *obuf, unsigned int len);
int cyp_enc_uuid(unsigned char *ibuf, unsigned char *obuf, unsigned int len, unsigned char *uuid);


/**
 * @brief		Encrypt messages by AES ECB in one call
 * @param		[in] ibuf
 *				pointer to physical buffer where raw data is
 * @param		[out] obuf
 *				pointer to physical buffer where encrypted data will be stored
 * @param		[in] len
 *				size of message to be encrypted
 * @param		[in] key
 *				key data
 * @return	    Returns zero on success and value other than zero on error.
 */
int AES_ecb_enc(unsigned char *ibuf, unsigned char *obuf, unsigned int len, unsigned char *key);


/**
 * @brief		Decrypt messages by AES ECB in one call
 * @param		[in] ibuf
 *				pointer to physical buffer where raw data is
 * @param		[out] obuf
 *				pointer to physical buffer where decrypted data will be stored
 * @param		[in] len
 *				size of message to be decrypted
 * @param		[in] key
 *				key data
 * @return	    Returns zero on success and value other than zero on error.
 */
int AES_ecb_dec(unsigned char *ibuf, unsigned char *obuf, unsigned int len, unsigned char *key);


/**
 * @brief		SHA256 update function
 */
void SHA2_Update(SHA2_CONTEXT *ctx, unsigned char *input, int ilen, int itype);

/**
 * @brief		SHA256 Padding function
 */
int SHA2_Padding(SHA2_CONTEXT *ctx);

/**
 * @brief		process the messages by SHA256
 * @param		[in] ctx
 *				sha context information
 * @param		[in] input
 *				pointer to physical buffer where raw data is
 * @param		[out] digest
 *				pointer to physical buffer where hash data will be stored
 */
int SHA2_GetDigest(SHA2_CONTEXT *ctx, unsigned char *input, unsigned char *digest);


/**
 * @brief			SHA256 calculate with dcache writeback and invalid
 * @param		[in] u32InBuf
 *				pointer to physical buffer where raw data is
 * @param		[in] u32Size
 *				size of message to be SHA
 * @param		[out] u32OutBuf
 *				pointer to physical buffer where hash data will be stored
 * @return	    Returns zero on success and value other than zero on error.
 */
unsigned int  MDrv_SHA256_Calculate(unsigned long u32InBuf, unsigned long u32Size, unsigned long *u32OutBuf);


int cyp_enc_cbc(unsigned char *ibuf, unsigned char *obuf, unsigned int len, unsigned char *iv, unsigned char* uuid);
#endif

