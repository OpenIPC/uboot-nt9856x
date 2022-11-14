#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <part.h>
#include <asm/hardware.h>
#include <asm/nvt-common/nvt_ivot_optee.h>
#include <asm/nvt-common/nvt_ivot_aes_smc.h>
#include <asm/nvt-common/nvt_ivot_sha_smc.h>
#include <asm/nvt-common/nvt_ivot_efuse_smc.h>
#include <asm/nvt-common/nvt_ivot_rsa_smc.h>
#include <asm/nvt-common/nvt_common.h>
#include <linux/arm-smccc.h>
#include <nvt_optee/nvt_ivot_optee_smc_id_def.h>
#include <nvt_optee/nvt_optee_software_smc_id.h>          //define in optee_os/core/arch/arm/plat-novatek
#include <nvt_optee/nvt_optee_hw_smc_id.h>   		//define in optee_os/core/arch/arm/plat-novatek
#include <malloc.h>
/**
* Call with struct optee_msg_arg as argument
 *
 * Call register usage:
 * a0   SMC Function ID, OPTEE_SMC*CALL_WITH_ARG
 * a1   Upper 32 bits of a 64-bit physical pointer to a struct optee_msg_arg
 * a2   Lower 32 bits of a 64-bit physical pointer to a struct optee_msg_arg
 * a3   Cache settings, not used if physical pointer is in a predefined shared
 *      memory area else per OPTEE_SMC_SHM_*
 * a4-6 Not used
 * a7   Hypervisor Client ID register
 *
 * Normal return register usage:
 * a0   Return value, OPTEE_SMC_RETURN_*
 * a1-3 Not used
 * a4-7 Preserved
**/
static int atoi(const char *str)
{
        return (int)simple_strtoul(str, '\0', 10);
}


static int nvt_sample_send_fast(void)
{
	int ret=0;
	struct arm_smccc_res return_val={0};
	struct TEST_DATA{
		char test_intput[10];
	};
	unsigned long shm_size=0;
	unsigned long shm_addr=0;
	if(nvt_dts_optee_nsmem(&shm_addr, &shm_size)!=0)
	{
		printf("parsing share memory fail\r\n");
		return -1;
	}
	struct TEST_DATA * driver_data= (struct TEST_DATA *)shm_addr;
	sprintf(driver_data->test_intput,"hello~~\n");
	//a0: for smc cmd,   a2: for share memory , others not use
	__arm_smccc_smc( NVT_HELLO_WORD, 0x00, shm_addr, 0x00,0x00,0x00,0x00, 0x00, &return_val, NULL);
	printf("intput data:%s\n",driver_data->test_intput);
	printf("return value :%x %x %x %x\n",return_val.a0,return_val.a1,return_val.a2,return_val.a3);

	return 0;
}
#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
static void efuse_is_secure_enable(void)
{
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	int ret=0;
	efuse_data.cmd = NVT_SMC_EFUSE_IS_SECURE;
	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0)
	{
		printf("nvt_ivot_optee_efuse_operation error ret:%d\r\n",ret);
	}
	printf("secure enable:%d\n",ret);

}
static void write_efuse_key(UINT32 keyset)
{
	int ret=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	NVT_SMC_EFUSE_KEY_DATA key_data={0};
	unsigned char key[16]={0x4,0x3,0x2,0x1,0x8,0x7,0x6,0x5,0x12,0x11,0x10,0x9,0x16,0x15,0x14,0x13};
	efuse_data.cmd = NVT_SMC_EFUSE_WRITE_KEY;
	efuse_data.key_data.field = keyset;
	memcpy(efuse_data.key_data.data , key, 16);

	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0){
		printf("write_key error ret:%d\n",ret);
	}
	printf("write key finish\r\n");
}

static void compare_efuse_key(void)
{
	int ret=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	NVT_SMC_EFUSE_KEY_DATA key_data={0};
	unsigned char key[16]={0x4,0x3,0x2,0x1,0x8,0x7,0x6,0x5,0x12,0x11,0x10,0x9,0x16,0x15,0x14,0x13};
	efuse_data.cmd = NVT_SMC_EFUSE_COMPARE_KEY;
	efuse_data.key_data.field = EFUSE_OTP_1ST_KEY_SET_FIELD;
	memcpy(efuse_data.key_data.data , key, 16);
	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0){
		printf("write_key error ret:%d\n",ret);
	}

	printf("key compare ret:%d\n",ret);

}

static void efuse_secure_enable(void)
{
	int ret=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_ENABLE_SECURE;

	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0)
	{
		printf("nvt_ivot_optee_efuse_operation error ret:%d\r\n",ret);
	}

}

static void efuse_data_encrypted_enable(void)
{
	int ret=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_ENABLE_DATA_ENCRYPTED;
	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0)
	{
		printf("nvt_ivot_optee_efuse_operation error ret:%d\r\n",ret);
	}

}

static void efuse_rsa_key_check_enable(void)
{
	int ret=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_ENABLE_RSA_KEY_CHECK;
	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0)
	{
		printf("nvt_ivot_optee_efuse_operation error ret:%d\r\n",ret);
	}
}

static void efuse_check_key_field(void)
{

	int ret=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_CHECK_KEY_FIELD;
	efuse_data.key_data.field = EFUSE_OTP_1ST_KEY_SET_FIELD;
	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0)
	{
		printf("nvt_ivot_optee_efuse_operation error ret:%d\r\n",ret);
		return;
	}
	printf("key field %d :status :%d\r\n",EFUSE_OTP_1ST_KEY_SET_FIELD, ret);
	return;


}

static void efuse_lock_engine_read_key_field(void)
{
	int ret=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_LOCK_ENGINE_READ_KEY_FIELD;
	efuse_data.key_data.field = EFUSE_OTP_1ST_KEY_SET_FIELD;
	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0)
	{
		printf("nvt_ivot_optee_efuse_operation error ret:%d\r\n",ret);
		return;
	}
	return;
}

static void efuse_lock_read_key_field(UINT32 keyset)
{
	int ret=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_LOCK_READ_KEY_FIELD;
	if(keyset < SECUREBOOT_READ_LOCK_KEY_SET_START || keyset> SECUREBOOT_READ_LOCK_KEY_SET_END) {
		printf("nvt_ivot_optee_efuse_operation error unknow enum => %d\r\n",keyset);
		return;
	}

	if(keyset == SECUREBOOT_2ND_KEY_SET_READ_LOCK || keyset == SECUREBOOT_3RD_KEY_SET_READ_LOCK) {
		printf("Error => 2nd & 3rd key set are RSA checksum area => can not configure as read lock\r\n",keyset);
		return;
	}
	efuse_data.key_data.field = keyset;
	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0)
	{
		printf("nvt_ivot_optee_efuse_operation error ret:%d\r\n",ret);
		return;
	}
	return;
}

static void efuse_read_key_field(UINT32 keyset)
{
	int ret=0;
	int i=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_READ_KEY_FIELD;
	efuse_data.key_data.field = keyset;
	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0)
	{
		printf("nvt_ivot_optee_efuse_operation error ret:%d\r\n",ret);
		return;
	}
	printf("key filed:%d\r\n",keyset);
	printf("value: ");
	for(i=0;i<16;i++)
	{
		printf("%x ",efuse_data.key_data.data[i]);
	}
	printf("\n");
	return;
}

static void efuse_trigger_key_field(UINT32 keyset)
{
	int ret=0;
	int i=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_TRIGGER_KEY_SET;
	efuse_data.key_data.field = keyset;
	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0)
	{
		printf("nvt_ivot_optee_efuse_operation error ret:%d\r\n",ret);
		return;
	}
	printf("key filed:%d\r\n",keyset);
	printf("value: \r\n");
	for(i=0;i<4;i++)
	{
		printf("[0x%08x]=[0x%08x]\r\n ", (0xF0620010+i*4), *(UINT32 *)((0xF0620010+i*4)));
	}
	printf("\n");
	return;
}

static void is_ras_key_check_enable(void)
{
	int ret=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_IS_RSA_KEY_CHECK;
	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0)
	{
		printf("nvt_ivot_optee_efuse_operation error ret:%d\r\n",ret);
	}
	printf("rsa key check enable:%d\n",ret);

}

static void is_data_encrypted_enable(void)
{
	int ret=0;
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_IS_DATA_ENCRYPTED;
	ret = nvt_ivot_optee_efuse_operation(&efuse_data);
	if(ret < 0)
	{
		printf("nvt_ivot_optee_efuse_operation error ret:%d\r\n",ret);
	}
	printf("data encrypted enable:%d\n",ret);

}

static unsigned char N_key[]={ 0xD3,0xA2,0x63,0xB3,0x09,0xCC,0x0D,0x6E,0x29,0x63,0xB1,0xFD,0x1D,0xC4,0x21,0xB2,0x40,0x68,0x4C,0xF2,0x48,0x10,0xE0,0x89,0xF5,0x3E,0x5A,0xDE,0x66,0xD5,0xF6,0xD5,0x28,0x9B,0x04,0xD7,0xF6,0x74,0x80,0xA9,0xCE,0x7E,0xC3,0x16,0x41,0x84,0xA3,0x62,0x59,0xFC,0xC8,0x1C,0x76,0x11,0x18,0x70,0xF8,0x4E,0xE1,0xE1,0x90,0x99,0x7A,0x2C,0xD6,0x3B,0xB4,0x2E,0x23,0x60,0xB9,0x68,0xFC,0xC6,0x0C,0xF7,0x12,0x41,0x65,0xA9,0xD7,0x52,0x7B,0x35,0xA5,0x3E,0xB7,0x33,0x1A,0x66,0x4D,0x5E,0x29,0x3D,0x12,0x46,0x9D,0xF1,0x68,0xF5,0x65,0x64,0x2B,0xC9,0xAE,0xDA,0x1C,0x46,0x53,0xC2,0x2D,0xD5,0xEA,0x87,0xB4,0x2B,0x94,0x6D,0x80,0x50,0x8A,0x3A,0x51,0x13,0xDA,0xE4,0xD9,0x3D,0x50,0x34,0x55,0xC7,0xC5,0x2A,0x32,0xAC,0x1E,0x66,0xCA,0x0D,0x28,0x8F,0x58,0x7F,0xC7,0x41,0x9B,0x96,0xF3,0xCF,0xBF,0x64,0x1C,0xAA,0x9C,0x6E,0x21,0x0D,0x15,0xAF,0x1E,0xE6,0x91,0x02,0xCF,0xF1,0x2A,0x71,0xC2,0xF2,0x97,0x9F,0x1B,0x85,0x7A,0xCF,0xA8,0x67,0x44,0x27,0xFB,0x2C,0xDC,0x5A,0xC5,0xAB,0xDA,0x43,0x7E,0xA3,0xCA,0x84,0x1C,0x7E,0x48,0xE2,0xBA,0x59,0x23,0x4F,0x37,0xA4,0x3F,0x7C,0xA1,0xE1,0x60,0x96,0xB1,0xC8,0x1C,0xF5,0x21,0xFE,0x9F,0xB3,0x37,0x89,0x98,0x7F,0x2B,0x4D,0x2A,0xEC,0x6E,0xE6,0x6D,0x33,0x42,0x7E,0xA7,0x7F,0x5C,0x3E,0xED,0xEC,0xDE,0x27,0x13,0xE0,0xA8,0x9B,0x8E,0xC6,0x11,0x6A,0xD0,0xFD,0xD0,0x0D,0x9E,0x3F,0xB4,0xA0,0xAF,0x67};

static unsigned char E_key[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01};


static unsigned char D_key[]={ 0x0A,0x49,0x66,0xE2,0x31,0x74,0x37,0x24,0xE7,0x23,0x1E,0xE8,0x28,0x35,0xBB,0xD3,0x8F,0xB8,0xE2,0x20,0x96,0xAB,0x27,0x56,0xDD,0x52,0x4A,0x15,0x6C,0x57,0x33,0x17,0xBA,0x51,0x0A,0xA3,0xBA,0xA9,0x80,0x05,0x80,0xF1,0x7D,0x67,0x0C,0x09,0x37,0xED,0xD4,0x64,0xEA,0x8F,0x23,0x98,0x02,0x21,0x9F,0x98,0x29,0xF7,0x8E,0x51,0x3F,0x74,0x85,0x77,0x42,0x73,0x49,0xA9,0xEE,0x69,0x31,0x7C,0x28,0xEE,0x2B,0x77,0x7D,0x4B,0x0B,0x99,0xC9,0x3E,0x5A,0xC9,0x59,0x1B,0x45,0x49,0xBA,0xB1,0xFC,0x7C,0x28,0xF2,0xC0,0xC3,0x96,0xAF,0xF2,0xDD,0x33,0x19,0xAB,0x03,0x94,0x03,0x41,0x17,0xFB,0xA5,0x5D,0xB0,0x79,0xEB,0xF2,0x5B,0x7E,0x34,0xAB,0xFB,0x58,0xAC,0x87,0xE3,0xBE,0xC1,0x5B,0x0E,0xD3,0xB7,0x43,0x09,0xAB,0xC6,0x31,0xBB,0x83,0x0F,0x3C,0x31,0xCB,0x53,0x79,0xC6,0x20,0xA8,0xD2,0xE6,0x95,0xA2,0x5E,0x7A,0x0D,0x0C,0xB9,0xB9,0x56,0x78,0xC7,0x99,0xB0,0x68,0x48,0xE3,0x99,0x19,0xA1,0xED,0xA0,0xB8,0xA4,0xDD,0x1E,0x81,0x63,0x62,0x50,0xE2,0x8F,0x7D,0x77,0x6F,0x1E,0x26,0x28,0xD3,0x34,0xA7,0x86,0x88,0x63,0xD3,0xAE,0x8E,0xC3,0xB8,0x7B,0x5F,0xD9,0x5B,0x4D,0x0A,0xD6,0x61,0x2C,0x5A,0xB9,0x0F,0x40,0xCF,0xAB,0xA5,0x51,0x0C,0x68,0xA3,0x59,0x2B,0x20,0x6B,0x26,0xBF,0xD3,0xEE,0x0F,0x1E,0x7E,0x3F,0xB7,0x9D,0x05,0x2D,0x3B,0xBA,0x0B,0x22,0xCB,0x0E,0x93,0x80,0x87,0x39,0x1C,0xA9,0xFB,0x2C,0xDE,0xBF,0x8A,0x24,0x7A,0xE1,0x01,0x29};
static void rsa_encryped_decrypted_test(void)
{
	#define MAX_RSA_SIZE 256
	unsigned char *input_buf = NULL;
	unsigned char *output_buf = NULL;
	unsigned char *output_buf2 = NULL;
	unsigned int i=0;
	int ret=0;
	//set test input buf
	input_buf = (unsigned char *)malloc(MAX_RSA_SIZE);
	for(i=0;i < MAX_RSA_SIZE ; i++)
	{
		input_buf[i] = i;
	}
	output_buf = (unsigned char *)malloc(MAX_RSA_SIZE);

	//encrypted
	NVT_SMC_RSA_DATA rsa_data={0};
	rsa_data.rsa_mode = NVT_SMC_RSA_MODE_2048;
	rsa_data.n_key = N_key;
	rsa_data.n_key_size = sizeof(N_key);
	rsa_data.ed_key = D_key;
	rsa_data.ed_key_size = sizeof(D_key);
	rsa_data.input_data = input_buf;
	rsa_data.input_size = MAX_RSA_SIZE;
	rsa_data.output_data = output_buf;

	ret = nvt_ivot_optee_rsa_operation(&rsa_data);

	if(ret != 0){
		printf("nvt_ivot_optee_rsa_decrypt fail ret:%d\n",ret);
		free(input_buf);
		free(output_buf);
		return ;
	}

	//decrypted

	output_buf2 = (unsigned char *)malloc(MAX_RSA_SIZE);

	NVT_SMC_RSA_DATA rsa_data1={0};
	rsa_data1.rsa_mode = NVT_SMC_RSA_MODE_2048;
	rsa_data1.n_key = N_key;
	rsa_data1.n_key_size = sizeof(N_key);
	rsa_data1.ed_key = E_key;
	rsa_data1.ed_key_size = sizeof(E_key);
	rsa_data1.input_data = output_buf;
	rsa_data1.input_size = MAX_RSA_SIZE;
	rsa_data1.output_data = output_buf2;

	ret = nvt_ivot_optee_rsa_operation(&rsa_data1);

	if(ret != 0){
		printf("nvt_ivot_optee_rsa_decrypt fail ret:%d\n",ret);
		free(input_buf);
		free(output_buf);
		free(output_buf2);
		return ;
	}

	printf("intput:%x %x %x %x\r\n",input_buf[0],input_buf[1],input_buf[2],input_buf[3]);
	printf("output:%x %x %x %x\r\n",output_buf2[0],output_buf2[1],output_buf2[2],output_buf2[3]);
	printf("input data:\r\n");
	for(i=0;i<MAX_RSA_SIZE;i++)
	{
		printf("%02x ",input_buf[i]);
	}
	printf("\r\n");
	printf("decryped data:\r\n");
	for(i=0;i<MAX_RSA_SIZE;i++)
	{
		printf("%02x ",output_buf2[i]);
	}
	printf("\r\n");
	if(memcmp(input_buf,output_buf2,MAX_RSA_SIZE)== 0)
	{
		printf("rsa encrypted/decryped ok\r\n");
	}
	else
	{
		printf("rsa encrypted/decryped fail\r\n");
	}
	free(input_buf);
	free(output_buf);
	free(output_buf2);
	return ;
}

static void sha256_test(void)
{


	unsigned char input[]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15};
	unsigned char output[32]={0};
	unsigned int i=0;
	unsigned char input_tmp[16]={0};
	int ret=0;
	memcpy(input_tmp,input,16);

	NVT_SMC_SHA_DATA sha_data={0};
	sha_data.input_data = input_tmp;
	sha_data.output_data = output;
	sha_data.input_size = sizeof(input_tmp);
	sha_data.sha_mode = NVT_SMC_SHA_MODE_SHA256;
	ret = nvt_ivot_optee_sha_operation(&sha_data);
	if(ret != 0)
	{
		printf("nvt_ivot_optee_sha_operation fail,ret:%d\n",ret);
		return;
	}
	printf("input:\r\n");
	for(i=0;i< sizeof(input_tmp); i++)
	{
		printf("%02x ",input_tmp[i]);
	}
	printf("\r\n");

	printf("output:\r\n");
	for(i=0;i< sizeof(output); i++)
	{
		printf("%02x ",output[i]);
	}
	printf("\r\n");
}
static void aes_encrypted_decrypted_test(void)
{

	//size need align 16
	#define MAX_AES_SIZE 80
	unsigned char *input= NULL;
	unsigned char *output= NULL;
	unsigned char *output2= NULL;
	int i=0;
	int ret=0;
	unsigned char test_aes_key[16]={0x04,0x03,0x02,0x01,0x08,0x07,0x06,0x05,0x12,0x11,0x10,0x09,0x16,0x15,0x14,0x013};
	NVT_SMC_AES_DATA aes_data ={0};
	NVT_SMC_AES_DATA aes_data1 ={0};
	input = malloc(MAX_AES_SIZE);
	output = malloc(MAX_AES_SIZE);
	output2 = malloc(MAX_AES_SIZE);
	for(i=0; i< MAX_AES_SIZE; i++)
	{
		input[i]=i;
	}
	//encrypted
	memset(aes_data.IV,0,16);
	aes_data.key_size = 16;
	aes_data.crypto_type = NVT_SMC_AES_CRYPTO_ENCRYPTION;
	aes_data.aes_mode = NVT_SMC_AES_MODE_CBC;
	aes_data.input_data = input;
	aes_data.input_size = MAX_AES_SIZE;
	aes_data.output_data = output;
	#if 0
	//using aes key from buffer
		aes_data.efuse_field = -1;
	#else
	//using aes key from efuse
		aes_data.efuse_field = 0;
	#endif

	ret = nvt_ivot_optee_aes_operation(&aes_data);
	if(ret != 0)
	{
		printf("nvt_ivot_optee_aes_operation fail\r\n",ret);
		free(input);
		free(output);
		free(output2);

		return ;
	}

	//decrypted
	memset(aes_data1.IV,0,16);
	aes_data1.key_size = 16;
	aes_data1.crypto_type = NVT_SMC_AES_CRYPTO_DECRYPTION;
	aes_data1.aes_mode = NVT_SMC_AES_MODE_CBC;
	aes_data1.input_data = output;
	aes_data1.input_size = MAX_AES_SIZE;
	aes_data1.output_data = output2;
	#if 0
	//using aes key from buffer
		aes_data1.efuse_field = -1;
	#else
	//using aes key from efuse
		aes_data1.efuse_field = 0;
	#endif

	ret = nvt_ivot_optee_aes_operation(&aes_data1);
	if(ret != 0)
	{
		printf("nvt_ivot_optee_aes_operation fail\r\n",ret);
		free(input);
		free(output);
		free(output2);
		return;
	}
	printf("input:\r\n");
	for(i=0;i<MAX_AES_SIZE; i++)
	{
		printf("%02x ",input[i]);
	}
	printf("\r\n");

	printf("output:\r\n");
	for(i=0;i<MAX_AES_SIZE; i++)
	{
		printf("%02x ",output2[i]);
	}
	printf("\r\n");


	if(memcmp(input,output2,MAX_AES_SIZE) == 0)
	{
		printf("aes ok!\r\n");
	}
	else
	{
		printf("aes fail\r\n");
	}

}
#endif

static int do_optee(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]){

	int ret=0;
	if(argc < 2){
		printf("argc %d error\n",argc);
		return -1;
	}

	if(strncmp(argv[0],"nvt_optee",9) == 0){
		if(strncmp(argv[1],"fast_sample",11)==0)
		{
			ret = nvt_sample_send_fast();
			if(ret< 0){
				printf("nvt_sample_send_fast fail ret:%d\n",ret);
			}
		}
		else if(strncmp(argv[1],"l2_cache",8)==0)
		{
			ret = nvt_ivot_l2_cache(atoi(argv[2]));
			if(ret< 0){
				printf("nvt_l2_cache fail ret:%d\n",ret);
			}
		}
#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
		else if(strncmp(argv[1],"secure_is_enable",16)==0)
		{
			efuse_is_secure_enable();
		}
		else if(strncmp(argv[1],"secure_write_key",16)==0)
		{
			UINT32 keyset;
			if (strncmp(argv[2], "0", 1) == 0) {
				keyset = EFUSE_OTP_1ST_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "1", 1) == 0) {
				keyset = EFUSE_OTP_2ND_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "2", 1) == 0) {
				keyset = EFUSE_OTP_3RD_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "3", 1) == 0) {
				keyset = EFUSE_OTP_4TH_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "4", 1) == 0) {
				keyset = EFUSE_OTP_5TH_KEY_SET_FIELD;
			} else {
				printf("error keyset:%s\n",argv[2]);
				return 0;
			}
			printf("write_key_field keyset(0~4):%d\n",keyset);

			write_efuse_key(keyset);
		}
		else if(strncmp(argv[1],"secure_compare_key",15)==0)
		{
			compare_efuse_key();
		}
		else if(strncmp(argv[1],"secure_enable",13)==0)
		{
			efuse_secure_enable();
		}
		else if(strncmp(argv[1],"data_encrypted_enable",21)==0)
		{
			efuse_data_encrypted_enable();
		}
		else if(strncmp(argv[1],"rsa_key_check_enable",20)==0)
		{
			efuse_rsa_key_check_enable();
		}
		else if(strncmp(argv[1],"rsa_key_check_is_enable",23)==0)
		{
			is_ras_key_check_enable();
		}
		else if(strncmp(argv[1],"data_encrypted_is_enable",24)==0)
		{
			is_data_encrypted_enable();
		}
		else if(strncmp(argv[1],"rsa_test",8)==0)
		{
			rsa_encryped_decrypted_test();
		}
		else if(strncmp(argv[1],"sha256_test",11)==0)
		{
			sha256_test();
		}
		else if(strncmp(argv[1],"aes_test",8)== 0)
		{
			aes_encrypted_decrypted_test();
		}
		else if(strncmp(argv[1],"check_key_field",15)== 0)
		{
			efuse_check_key_field();
		}
		else if(strncmp(argv[1],"read_key_field",14)==0)
		{
			UINT32 keyset;
			if (strncmp(argv[2], "0", 1) == 0) {
				keyset = EFUSE_OTP_1ST_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "1", 1) == 0) {
				keyset = EFUSE_OTP_2ND_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "2", 1) == 0) {
				keyset = EFUSE_OTP_3RD_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "3", 1) == 0) {
				keyset = EFUSE_OTP_4TH_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "4", 1) == 0) {
				keyset = EFUSE_OTP_5TH_KEY_SET_FIELD;
			} else {
				printf("error keyset:%s\n",argv[2]);
				return 0;
			}
			printf("read_key_field keyset(0~4):%d\n",keyset);
			efuse_read_key_field(keyset);
		}
		else if(strncmp(argv[1],"lock_read_key_field",19)==0)
		{
			UINT32 keyset;
			if (strncmp(argv[2], "0", 1) == 0) {
				keyset = SECUREBOOT_1ST_KEY_SET_READ_LOCK;
			} else if (strncmp(argv[2], "1", 1) == 0) {
				keyset = SECUREBOOT_2ND_KEY_SET_READ_LOCK;
			} else if (strncmp(argv[2], "2", 1) == 0) {
				keyset = SECUREBOOT_3RD_KEY_SET_READ_LOCK;
			} else if (strncmp(argv[2], "3", 1) == 0) {
				keyset = SECUREBOOT_4TH_KEY_SET_READ_LOCK;
			} else if (strncmp(argv[2], "4", 1) == 0) {
				keyset = SECUREBOOT_5TH_KEY_SET_READ_LOCK;
			} else {
				printf("error keyset:%s\n",argv[2]);
				return 0;
			}

			efuse_lock_read_key_field(keyset);
		}
		else if(strncmp(argv[1],"trigger_key_field",17)==0)
		{
			UINT32 keyset;
			if (strncmp(argv[2], "0", 1) == 0) {
				keyset = EFUSE_OTP_1ST_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "1", 1) == 0) {
				keyset = EFUSE_OTP_2ND_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "2", 1) == 0) {
				keyset = EFUSE_OTP_3RD_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "3", 1) == 0) {
				keyset = EFUSE_OTP_4TH_KEY_SET_FIELD;
			} else if (strncmp(argv[2], "4", 1) == 0) {
				keyset = EFUSE_OTP_5TH_KEY_SET_FIELD;
			} else {
				printf("error keyset:%s\n",argv[2]);
				return 0;
			}

			efuse_trigger_key_field(keyset);
		}
		else if(strncmp(argv[1],"lock_engine_read_key_field",26)==0)
		{
			efuse_lock_engine_read_key_field();
		}
#endif
		else
		{
			printf("error cmd:%s\n",argv[1]);
		}
	}

	return 0;
}

U_BOOT_CMD(nvt_optee, 3, 0 , do_optee,

        "optee test cmd:",
        " - this is for optee cmd\n"
        "[Option] \n"
		"              [fast_sample]\n"
		"              [secure_is_enable]\n"
		"              [data_encrypted_is_enable]\n"
		"              [rsa_key_check_is_enable]\n"
		"              [secure_enable]\n"
		"              [data_encrypted_enable]\n"
		"              [rsa_key_check_enable]\n"
		"              [secure_write_key]\n"
		"              [trigger_key_field]\n"
		"              [secure_compare_key]\n"
		"              [rsa_test]\n"
		"              [sha256_test]\n"
		"              [aes_test]\n"
		"              [check_key_field]\n"
		"              [read_key_field]\n"
		"              [lock_read_key_field] [keyset](0/3/4)(1/2 for RSA checksum)\n"
		"              [lock_engine_read_key_field]\n"
);
