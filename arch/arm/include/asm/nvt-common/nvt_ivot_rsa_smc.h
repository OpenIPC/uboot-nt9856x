#ifndef _RSA_SMC_H
#define _RSA_SMC_H
#define SMC_RSA_MAKEFOURCC(ch0, ch1, ch2, ch3) ((UINT32)(UINT8)(ch0) | ((UINT32)(UINT8)(ch1) << 8) | ((UINT32)(UINT8)(ch2) << 16) | ((UINT32)(UINT8)(ch3) << 24))
#define SMC_RSA_TAG         SMC_RSA_MAKEFOURCC('R', 'S', 'A', '1')


typedef enum _NVT_SMC_RSA_MODE
{
	NVT_SMC_RSA_MODE_1024,
	NVT_SMC_RSA_MODE_2048,
	NVT_SMC_RSA_MODE_MAX
}NVT_SMC_RSA_MODE;

typedef enum _NVT_SMC_RSA_OPERATION
{

	NVT_SMC_RSA_OPERATION_OPEN,
	NVT_SMC_RSA_OEPRATION_UPDATE,
	NVT_SMC_RSA_OPERATION_CLOSE,
	NVT_SMC_RSA_OPERATION_MAX

}NVT_SMC_RSA_OPERATION;


typedef struct _NVT_SMC_RSA_DATA
{

	unsigned int 	tag;   //should be MAKEFOURCC('R', 'S', 'A', '1') , customer no need to set
	NVT_SMC_RSA_MODE rsa_mode;
	NVT_SMC_RSA_OPERATION operation;// customer no need to set
	unsigned char 	*n_key;
	unsigned int 	n_key_size;
	unsigned char 	*ed_key;
	unsigned int	ed_key_size;
	unsigned char	*input_data;
	unsigned int	input_size;
	unsigned char	*output_data;
	unsigned int 	reserve[22];//customer no need to set

} NVT_SMC_RSA_DATA;

int nvt_ivot_optee_rsa_operation(NVT_SMC_RSA_DATA *rsa_smc_data);

#endif
