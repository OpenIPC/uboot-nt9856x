
#ifndef _AES_SMC_H
#define _AES_SMC_H
#define SMC_AES_MAKEFOURCC(ch0, ch1, ch2, ch3) ((UINT32)(UINT8)(ch0) | ((UINT32)(UINT8)(ch1) << 8) | ((UINT32)(UINT8)(ch2) << 16) | ((UINT32)(UINT8)(ch3) << 24))
#define SMC_AES_TAG         SMC_AES_MAKEFOURCC('A', 'E', 'S', '1')

typedef enum _NVT_SMC_AES_CRYPTO
{
	NVT_SMC_AES_CRYPTO_ENCRYPTION,
	NVT_SMC_AES_CRYPTO_DECRYPTION,
	NVT_SMC_AES_CRYPTO_MAX
}NVT_SMC_AES_CRYPTO;


typedef enum _NVT_SMC_AES_MODE
{
	NVT_SMC_AES_MODE_CBC,
	NVT_SMC_AES_MODE_ECB,
	NVT_SMC_AES_MODE_MAX

}NVT_SMC_AES_MODE;

typedef enum _NVT_SMC_AES_OPERATION
{

	NVT_SMC_AES_OPERATION_ALLOC,
	NVT_SMC_AES_OPERATION_INIT,
	NVT_SMC_AES_OPERATION_UPDATE,
	NVT_SMC_AES_OPERATION_FINAL,
	NVT_SMC_AES_OPERATION_FREE,
	NVT_SMC_AES_OPERATION_MAX
	

}NVT_SMC_AES_OPERATION;


typedef struct _NVT_SMC_AES_DATA{
	
	unsigned int	tag;  //should be MAKEFOURCC('A', 'E', 'S', '1'), customer no need to set
	NVT_SMC_AES_CRYPTO	crypto_type;
	NVT_SMC_AES_MODE	aes_mode;
	NVT_SMC_AES_OPERATION	operation;//customer no need to set
	void * ctx;//customer no need to set
	unsigned char	IV[16];
	int		efuse_field;
	unsigned char	*key_buf;
	unsigned int	key_size;
	unsigned char	*input_data;
	unsigned int	input_size;
	unsigned char	*output_data;
	unsigned int	output_size;
	unsigned int	reserve[16]; //customer no need to set

}NVT_SMC_AES_DATA;   // total size = 128 bytes

int nvt_ivot_optee_aes_operation(NVT_SMC_AES_DATA *aes_smc_data);

#endif   //_AES_SMC_H
