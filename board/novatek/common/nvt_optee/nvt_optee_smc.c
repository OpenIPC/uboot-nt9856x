#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <part.h>
#include <asm/hardware.h>
#include <linux/arm-smccc.h>
#include <asm/nvt-common/nvt_ivot_optee.h>
#include <asm/nvt-common/nvt_ivot_aes_smc.h>
#include <asm/nvt-common/nvt_ivot_sha_smc.h>
#include <asm/nvt-common/nvt_ivot_efuse_smc.h>
#include <asm/nvt-common/nvt_ivot_rsa_smc.h>
#include <asm/nvt-common/nvt_common.h>
#include <nvt_optee/nvt_ivot_optee_smc_id_def.h>
#include <nvt_optee/nvt_optee_software_smc_id.h>          //define in optee_os/core/arch/arm/plat-novatek
#include <nvt_optee/nvt_optee_hw_smc_id.h>                //define in optee_os/core/arch/arm/plat-novatek
#include <nvt_optee/nvt_optee_custom_smc_id.h>            // define in optee_os/core/arch/arm/plat-novatek



int nvt_ivot_l2_cache(int enable)
{
        int ret=0;
        struct arm_smccc_res return_val={0};

	printf("l2cache:%d\n",enable);

	if (enable) {
		__arm_smccc_smc(NVT_L2CACHE_CTRL, 1, 0x00, 0x00,0x00,0x00,0x00, 0x00, &return_val, NULL);
	} else {
		__arm_smccc_smc(NVT_L2CACHE_CTRL, 0, 0x00, 0x00,0x00,0x00,0x00, 0x00, &return_val, NULL);
	}

	return 0;
}

int nvt_ivot_l2_aux_ctrl_cfg(unsigned long val)
{
	int ret=0;
	struct arm_smccc_res return_val={0};

	__arm_smccc_smc(NVT_L2CACHE_AUX_CTRL, val, 0xFFF00000, 0x00,0x00,0x00,0x00, 0x00, &return_val, NULL);

	return 0;
}

int nvt_ivot_actlr_smp_cfg(int enable)
{
	int ret=0;
	struct arm_smccc_res return_val={0};

	__arm_smccc_smc(NVT_ACTLR_SMP_CFG, enable, 0x00, 0x00,0x00,0x00,0x00, 0x00, &return_val, NULL);

	return 0;
}

#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT

int nvt_ivot_optee_efuse_operation(NVT_SMC_EFUSE_DATA *efuse_data)
{

	unsigned long shm_size=0;
	unsigned long shm_addr=0;
	if(nvt_dts_optee_nsmem(&shm_addr, &shm_size)!=0)
	{
		printf("parsing share memory fail\r\n");
		return -1;
	}
	struct arm_smccc_res return_val={0};
	unsigned char* share_data= (unsigned char *)shm_addr;
	unsigned int invalidate_size = 0;
	int ret =0;
	NVT_SMC_EFUSE_DATA *smc_efuse_data = (NVT_SMC_EFUSE_DATA *)share_data;
	memcpy(smc_efuse_data, efuse_data, sizeof(NVT_SMC_EFUSE_DATA));
	smc_efuse_data->tag = SMC_EFUSE_TAG;
	if(efuse_data->cmd == NVT_SMC_EFUSE_WRITE_KEY || efuse_data->cmd == NVT_SMC_EFUSE_COMPARE_KEY ||
		efuse_data->cmd == NVT_SMC_EFUSE_CHECK_KEY_FIELD || efuse_data->cmd == NVT_SMC_EFUSE_READ_KEY_FIELD ||
		efuse_data->cmd == NVT_SMC_EFUSE_LOCK_READ_KEY_FIELD || efuse_data->cmd == NVT_SMC_EFUSE_LOCK_ENGINE_READ_KEY_FIELD ||
		efuse_data->cmd == NVT_SMC_EFUSE_TRIGGER_KEY_SET)
	{
		smc_efuse_data->key_data.tag = SMC_EFUSE_KEY_TAG;
	}

	 __arm_smccc_smc(NVT_EFUSE_OPERATION, (unsigned long)share_data, 0x00, 0x00,0x00, 0x00, 0x00,0x00, &return_val, NULL);
	if(return_val.a0 < 0)
	{
		printf("NVT_EFUSE_OPERATION fail (%d)  cmd:%x\r\n",return_val.a0,smc_efuse_data->cmd);
	}
	//update output size
	invalidate_size = ((sizeof(NVT_SMC_EFUSE_DATA) / CONFIG_SYS_CACHELINE_SIZE)) * CONFIG_SYS_CACHELINE_SIZE;
	invalidate_dcache_range((unsigned long)share_data, (unsigned long)(share_data +invalidate_size));
	memcpy(&efuse_data->key_data, &smc_efuse_data->key_data,sizeof(efuse_data->key_data));

	return return_val.a0;


}

int nvt_ivot_optee_sha_operation(NVT_SMC_SHA_DATA *sha_data)
{
	unsigned long shm_size=0;
	unsigned long shm_addr=0;
	if(nvt_dts_optee_nsmem(&shm_addr, &shm_size)!=0)
	{
		printf("parsing share memory fail\r\n");
		return -1;
	}
	unsigned int max_sha_input_size = shm_size - sizeof(NVT_SMC_SHA_DATA);
	struct arm_smccc_res return_val={0};
	unsigned char* share_data= (unsigned char *)shm_addr;
	unsigned int invalidate_size = 0;
	int ret =0;
	NVT_SMC_SHA_DATA *smc_sha_data = (NVT_SMC_SHA_DATA *)share_data;
	if(sha_data->input_data == NULL || sha_data->output_data == NULL)
	{
		printf("intput_data == NULL || output_data == NULl error\r\n");
		return -1;
	}
	if(sha_data->input_size ==0)
	{
		printf("sha_data->input_size == 0 error\r\n");
		return -1;
	}

	memcpy(smc_sha_data, sha_data, sizeof(NVT_SMC_SHA_DATA));
	smc_sha_data->tag = SMC_SHA_TAG;
	//allocate ctx
	smc_sha_data->operation = NVT_SMC_SHA_OPERATION_ALLOC;
	 __arm_smccc_smc(NVT_SHA_OPERATION, (unsigned long)share_data, 0x00, 0x00,0x00, 0x00, 0x00,0x00, &return_val, NULL);
	if(return_val.a0 != 0)
	{
		printf("NVT_SMC_SHA_OPERATION_ALLOC fail (%d)\r\n",return_val.a0);
		return return_val.a0;
	}
	//flush ctx from optee
	invalidate_size = ((sizeof(NVT_SMC_SHA_DATA) / CONFIG_SYS_CACHELINE_SIZE)) * CONFIG_SYS_CACHELINE_SIZE;
	invalidate_dcache_range((unsigned long)share_data, (unsigned long)(share_data +invalidate_size));

	//init
	smc_sha_data->operation = NVT_SMC_SHA_OPERATION_INIT;
	 __arm_smccc_smc(NVT_SHA_OPERATION, (unsigned long)share_data, 0x00, 0x00,0x00, 0x00, 0x00,0x00, &return_val, NULL);
	if(return_val.a0 != 0)
	{
		printf("NVT_SMC_SHA_OPERATION_ALLOC fail (%d)\r\n",return_val.a0);
		return return_val.a0;
	}

	//update
	unsigned int total_intput_size = sha_data->input_size;
	unsigned int current_intput_size=0;
	unsigned int current_intput_offset=0;
	sha_data->output_size =0;
	while(1)
	{
		smc_sha_data->operation = NVT_SMC_SHA_OPERATION_UPDATE;
		if(total_intput_size < max_sha_input_size)
		{
			current_intput_size = total_intput_size;
		}
		else
		{
			current_intput_size = max_sha_input_size;
                }
                smc_sha_data->input_data = (unsigned char*)( (unsigned int)share_data + sizeof(NVT_SMC_AES_DATA));
                smc_sha_data->input_size = current_intput_size;
                memcpy(smc_sha_data->input_data, (unsigned char *)(sha_data->input_data + current_intput_offset), current_intput_size);
                __arm_smccc_smc(NVT_SHA_OPERATION, (unsigned long )share_data, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, &return_val, NULL);
                if(return_val.a0 != 0)
                {
                        printf("NVT_SMC_SHA_OPERATION_UPDATE fail (%d)\r\n",return_val.a0);
                        ret = -1;
                        goto sha_operation_exit;
                }
                total_intput_size = total_intput_size - current_intput_size;
                current_intput_offset = current_intput_offset + current_intput_size;
                if(total_intput_size <=0)
                {
                        break;
                }
        }

	//final
	smc_sha_data->operation = NVT_SMC_SHA_OPERATION_FINAL;
	smc_sha_data->output_data = (unsigned char *)((unsigned int)share_data + sizeof(NVT_SMC_AES_DATA));
        __arm_smccc_smc(NVT_SHA_OPERATION, (unsigned long)share_data, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, &return_val, NULL);
        if(return_val.a0 != 0)
        {
                printf("NVT_SMC_SHA_OPERATION_FINAL fail (%d)\r\n",return_val.a0);
                ret = -1;
                goto sha_operation_exit;
        }
	//update output data
	invalidate_size = ((smc_sha_data->output_size / CONFIG_SYS_CACHELINE_SIZE)) * CONFIG_SYS_CACHELINE_SIZE;
	invalidate_dcache_range((unsigned long)smc_sha_data->output_data, (unsigned long)(smc_sha_data->output_data +invalidate_size));
	//update output size
	invalidate_size = ((sizeof(NVT_SMC_SHA_DATA) / CONFIG_SYS_CACHELINE_SIZE)) * CONFIG_SYS_CACHELINE_SIZE;
	invalidate_dcache_range((unsigned long)share_data, (unsigned long)(share_data +invalidate_size));
	if(smc_sha_data->output_size == 0)
	{
		printf("sha output size = 0 error\r\n");
		ret = -1;
                goto sha_operation_exit;
	}
	sha_data->output_size = smc_sha_data->output_size;
	memcpy(sha_data->output_data, smc_sha_data->output_data, smc_sha_data->output_size);

	ret = 0;
sha_operation_exit:
	//free
        smc_sha_data->operation = NVT_SMC_SHA_OPERATION_FREE;
        __arm_smccc_smc(NVT_SHA_OPERATION, (unsigned long )share_data, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, &return_val, NULL);
        if(return_val.a0 != 0)
        {
                printf("NVT_SMC_SHA_OPERATION_FREE fail (%d)\r\n",return_val.a0);
        }
        return ret;

}

int nvt_ivot_optee_rsa_operation(NVT_SMC_RSA_DATA *rsa_data)
{

	unsigned long shm_size=0;
	unsigned long shm_addr=0;
	if(nvt_dts_optee_nsmem(&shm_addr, &shm_size)!=0)
	{
		printf("parsing share memory fail\r\n");
		return -1;
	}
	struct arm_smccc_res return_val={0};
	unsigned char* share_data= (unsigned char *)shm_addr;
	unsigned int invalidate_size = 0;
	int ret =0;
	NVT_SMC_RSA_DATA *smc_rsa_data= (NVT_SMC_RSA_DATA *)share_data;
	if(rsa_data->n_key == NULL || rsa_data->ed_key== NULL)
	{
		printf("n_key addr == %x || ed_key addr= %x error, please check key\r\n",rsa_data->n_key,rsa_data->ed_key);
		return -1;
	}
	if(rsa_data->input_data == NULL || rsa_data->output_data == NULL)
	{
		printf("intput_data addr== %x || output_data addr== %x error\r\n",rsa_data->input_data,rsa_data->output_data);
		return -1;
	}
	if(rsa_data->n_key_size != rsa_data->input_size || rsa_data->n_key_size == 0 ||
		rsa_data->ed_key_size == 0 || rsa_data->input_size == 0)
	{
		printf("someboby size == 0 error, or  n key size == input data size error\r\n");
		printf("rsa_data->n_key_size:%d rsa_data->input_size:%d rsa_data->ed_key_size:%d, error\r\n",
		rsa_data->n_key_size, rsa_data->input_size,rsa_data->ed_key_size);
		return -1;
	}

	memcpy(smc_rsa_data, rsa_data, sizeof(NVT_SMC_RSA_DATA));
	smc_rsa_data->tag = SMC_RSA_TAG;
	smc_rsa_data->n_key = (unsigned char *)((unsigned int)share_data + sizeof(NVT_SMC_RSA_DATA));
	smc_rsa_data->ed_key = (unsigned char *)((unsigned int)share_data + sizeof(NVT_SMC_RSA_DATA) + rsa_data->n_key_size);
	memcpy(smc_rsa_data->n_key, rsa_data->n_key, rsa_data->n_key_size);
	memcpy(smc_rsa_data->ed_key, rsa_data->ed_key, rsa_data->ed_key_size);
	//rsa open
	smc_rsa_data->operation = NVT_SMC_RSA_OPERATION_OPEN;
	 __arm_smccc_smc(NVT_RSA_OPERATION, (unsigned long)share_data, 0x00, 0x00,0x00, 0x00, 0x00,0x00, &return_val, NULL);
	if(return_val.a0 != 0)
	{
		printf("NVT_SMC_RSA_OPERATION_OPEN fail (%d)\r\n",return_val.a0);
		return return_val.a0;
	}
	// rsa update
	unsigned int total_intput_size = rsa_data->input_size;
	unsigned int current_intput_size=0;
	unsigned int current_output_offset=0;
	unsigned int current_intput_offset=0;
	while(1)
	{
		smc_rsa_data->operation = NVT_SMC_RSA_OEPRATION_UPDATE;
		smc_rsa_data->input_data = (unsigned char*)((unsigned int)share_data + sizeof(NVT_SMC_RSA_DATA));
		if(total_intput_size < rsa_data->n_key_size)
		{
			current_intput_size = total_intput_size;
			memset(smc_rsa_data->input_data,0,rsa_data->n_key_size);//intput size should be n key size, need pending 0
		}
		else
		{
			current_intput_size = rsa_data->n_key_size;
		}
		smc_rsa_data->input_size = rsa_data->n_key_size; //always be equal to n key size
		memcpy(smc_rsa_data->input_data, (unsigned char *)(rsa_data->input_data + current_intput_offset), current_intput_size);
		smc_rsa_data->output_data = (unsigned char *)((unsigned int)share_data + sizeof(NVT_SMC_RSA_DATA));
		__arm_smccc_smc(NVT_RSA_OPERATION, (unsigned long )share_data, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, &return_val, NULL);
		if(return_val.a0 != 0)
		{
			printf("NVT_SMC_RSA_OEPRATION_UPDATE fail (%d)\r\n",return_val.a0);
			ret = -1;
			goto rsa_operation_exit;
		}

		invalidate_size = ((smc_rsa_data->input_size / CONFIG_SYS_CACHELINE_SIZE)) * CONFIG_SYS_CACHELINE_SIZE;
		invalidate_dcache_range((unsigned long)smc_rsa_data->output_data, (unsigned long)(smc_rsa_data->output_data +invalidate_size));
		memcpy((unsigned char *)(rsa_data->output_data + current_output_offset), (unsigned char *)smc_rsa_data->output_data, smc_rsa_data->input_size);
		total_intput_size = total_intput_size - current_intput_size;
		current_output_offset = current_output_offset + current_intput_size;
		current_intput_offset = current_intput_offset + current_intput_size;
		if(total_intput_size <=0)
		{
			break;
		}
	}

rsa_operation_exit:

	//free
	smc_rsa_data->operation = NVT_SMC_RSA_OPERATION_CLOSE;
	__arm_smccc_smc(NVT_RSA_OPERATION, (unsigned long )share_data, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, &return_val, NULL);
	if(return_val.a0 != 0)
	{
		printf("NVT_SMC_RSA_OPERATION_FREE fail (%d)\r\n",return_val.a0);
	}
	return ret;
}

int nvt_ivot_optee_aes_operation(NVT_SMC_AES_DATA *aes_data)
{

	unsigned long shm_size=0;
	unsigned long shm_addr=0;
	if(nvt_dts_optee_nsmem(&shm_addr, &shm_size)!=0)
	{
		printf("parsing share memory fail\r\n");
		return -1;
	}
	unsigned int max_aes_input_size = shm_size - sizeof(NVT_SMC_AES_DATA);
	struct arm_smccc_res return_val={0};
	unsigned char* share_data= (unsigned char *)shm_addr;
	unsigned int invalidate_size = 0;
	int ret =0;
	NVT_SMC_AES_DATA *smc_aes_data= (NVT_SMC_AES_DATA *)share_data;

	if(aes_data->input_data == NULL || aes_data->output_data == NULL)
	{
		printf("intput_data == NULL || output_data == NULL error\r\n");
		return -1;

	}
	if(aes_data->key_buf == NULL && aes_data->efuse_field==-1)
	{
		printf("aes_data->key_buf == NULL && aes_data->efuse_field==-1 are fail, please check using efuse key or buffer key\r\n");
		return -1;
	}
	memcpy(smc_aes_data, aes_data, sizeof(NVT_SMC_AES_DATA));
	smc_aes_data->tag = SMC_AES_TAG;

	//allocate ctx
	smc_aes_data->operation = NVT_SMC_AES_OPERATION_ALLOC;
	 __arm_smccc_smc(NVT_AES_OPERATION, (unsigned long)share_data, 0x00, 0x00,0x00, 0x00, 0x00,0x00, &return_val, NULL);
	if(return_val.a0 != 0)
	{
		printf("NVT_SMC_AES_OPERATION_ALLOC fail (%d)\r\n",return_val.a0);
		return return_val.a0;
	}
	invalidate_size = ((sizeof(NVT_SMC_AES_DATA) / CONFIG_SYS_CACHELINE_SIZE)) * CONFIG_SYS_CACHELINE_SIZE;
	invalidate_dcache_range((unsigned long)share_data, (unsigned long)(share_data +invalidate_size));

	//cipher init
	smc_aes_data->operation = NVT_SMC_AES_OPERATION_INIT;
	smc_aes_data->key_buf =(unsigned char *)(share_data + sizeof(NVT_SMC_AES_DATA));
	if(aes_data->efuse_field == -1)
	{
		if(aes_data->key_size < 16)
		{
			printf("aes_data->key_size toot small(%d)\r\n",aes_data->key_size);
			ret = -1;
			goto aes_operation_exit;
		}
		memcpy(smc_aes_data->key_buf, aes_data->key_buf, aes_data->key_size);
	}

	 __arm_smccc_smc(NVT_AES_OPERATION, (unsigned long )share_data, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, &return_val, NULL);
	if(return_val.a0 != 0)
	{
		printf("NVT_SMC_AES_OPERATION_INIT fail (%d)\r\n",return_val.a0);
		ret = -1;
		goto aes_operation_exit;
	}

	// update
	unsigned int total_intput_size = aes_data->input_size;
	unsigned int current_intput_size=0;
	unsigned int current_output_offset=0;
	unsigned int current_intput_offset=0;
	aes_data->output_size =0;
	if(total_intput_size & 0xf)
	{
		printf("aes intput size (%d) need alignment 16\r\n",total_intput_size);
		ret = -1;
		goto aes_operation_exit;
	}

	while(1)
	{
		smc_aes_data->operation = NVT_SMC_AES_OPERATION_UPDATE;
		if(total_intput_size < max_aes_input_size)
		{
			current_intput_size = total_intput_size;
		}
		else
		{
			current_intput_size = max_aes_input_size;
		}
		smc_aes_data->input_data = (unsigned char*)(current_intput_offset + (unsigned int)share_data + sizeof(NVT_SMC_AES_DATA));
		smc_aes_data->input_size = current_intput_size;
		memcpy(smc_aes_data->input_data, (unsigned char *)(aes_data->input_data + current_intput_offset), current_intput_size);
		smc_aes_data->output_data = (unsigned char *)(current_output_offset + (unsigned int)share_data + sizeof(NVT_SMC_AES_DATA));
		smc_aes_data->output_size = current_intput_size;//  input size should be equal to output size
		__arm_smccc_smc(NVT_AES_OPERATION, (unsigned long )share_data, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, &return_val, NULL);
		if(return_val.a0 != 0)
		{
			printf("NVT_SMC_AES_OPERATION_UPDATE fail (%d)\r\n",return_val.a0);
			ret = -1;
			goto aes_operation_exit;
		}

		invalidate_size = ((smc_aes_data->output_size / CONFIG_SYS_CACHELINE_SIZE)) * CONFIG_SYS_CACHELINE_SIZE;
		invalidate_dcache_range((unsigned long)smc_aes_data->output_data, (unsigned long)(smc_aes_data->output_data +invalidate_size));
		memcpy((unsigned char *)(aes_data->output_data + current_output_offset), (unsigned char *)smc_aes_data->output_data, smc_aes_data->output_size);
		aes_data->output_size = aes_data->output_size + current_intput_size;
		total_intput_size = total_intput_size - current_intput_size;
		current_output_offset = current_output_offset + current_intput_size;
		current_intput_offset = current_intput_offset + current_intput_size;
		if(total_intput_size <=0)
		{
			break;
		}
	}

	//final
	smc_aes_data->operation = NVT_SMC_AES_OPERATION_FINAL;
	__arm_smccc_smc(NVT_AES_OPERATION, (unsigned long)share_data, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, &return_val, NULL);
	if(return_val.a0 != 0)
	{
		printf("NVT_SMC_AES_OPERATION_FINAL fail (%d)\r\n",return_val.a0);
		ret = -1;
		goto aes_operation_exit;
	}
	ret = 0;
aes_operation_exit:

	//free
	smc_aes_data->operation = NVT_SMC_AES_OPERATION_FREE;
	__arm_smccc_smc(NVT_AES_OPERATION, (unsigned long )share_data, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, &return_val, NULL);
	if(return_val.a0 != 0)
	{
		printf("NVT_SMC_AES_OPERATION_FREE fail (%d)\r\n",return_val.a0);
	}
	return ret;
}
#endif




