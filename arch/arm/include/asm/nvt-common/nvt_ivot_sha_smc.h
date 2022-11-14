#ifndef _SHA_SMC_H
#define _SHA_SMC_H
#define SMC_SHA_MAKEFOURCC(ch0, ch1, ch2, ch3) ((UINT32)(UINT8)(ch0) | ((UINT32)(UINT8)(ch1) << 8) | ((UINT32)(UINT8)(ch2) << 16) | ((UINT32)(UINT8)(ch3) << 24))
#define SMC_SHA_TAG         SMC_SHA_MAKEFOURCC('S', 'H', 'A', '1')

typedef enum _NVT_SMC_SHA_MODE
{

	NVT_SMC_SHA_MODE_SHA256,
	NVT_SMC_SHA_MODE_MAX

}NVT_SMC_SHA_MODE;


typedef enum _NVT_SMC_SHA_OPERATION
{
	NVT_SMC_SHA_OPERATION_ALLOC,
	NVT_SMC_SHA_OPERATION_INIT,
	NVT_SMC_SHA_OPERATION_UPDATE,
	NVT_SMC_SHA_OPERATION_FINAL,
	NVT_SMC_SHA_OPERATION_FREE,
	NVT_SMC_SHA_OPERATION_MAX

}NVT_SMC_SHA_OPERATION;




typedef struct _NVT_SMC_SHA_DATA
{

	unsigned int tag; //should be MAKEFOURCC('S', 'H', 'A', '1'), customer no need to set
	NVT_SMC_SHA_MODE sha_mode;
	NVT_SMC_SHA_OPERATION operation; //customer no need to set
	void * ctx;
	unsigned int input_size;
	unsigned char *input_data;
	unsigned char *output_data;
	unsigned int output_size;
	unsigned int reserve[24]; //customer no need to set

}NVT_SMC_SHA_DATA; //total size = 128bytes


int nvt_ivot_optee_sha_operation(NVT_SMC_SHA_DATA *sha_smc_data);
#endif //end _SHA_SMC_H
