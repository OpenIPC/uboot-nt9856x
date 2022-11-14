

#include <asm/unaligned.h>
#include <memalign.h>
#include <common.h>
#include <command.h>
#include <ide.h>
#include <malloc.h>
#include <part_efi.h>
#include <exports.h>
#include <blk.h>
#include <part.h>
#include <linux/ctype.h>
#include <../../../disk/part_dos.h>
#include <mmc.h>
#include <errno.h>

#define INFO_SIZE (10240)
#define EMMC_BLOCK_SIZE (512)
#define STRING_BUF_SIZE (256)
extern struct mmc *init_mmc_device(int dev, bool force_init);
char gpt_info[INFO_SIZE] __attribute__ ((aligned (0x1000)));

struct src_action 
{
	char init[STRING_BUF_SIZE];
	char read[STRING_BUF_SIZE];
};

static struct src_action usb_act = 
{
	.init = "usb start",
	.read = "fatload usb 0:auto 0x%x %s ",
};

static struct src_action eth_act = 
{
	.init = "",
	.read = "tftp 0x%x %s ",
};

static struct src_action *act = & usb_act;
//from binary update
#define PART_ID_SIZE	(64)
struct part_descriptor
{
	char	part_id_name[PART_ID_SIZE];
	char	part_in_file[PART_ID_SIZE];
	char	type[PART_ID_SIZE];
	unsigned long long starting;
	char	starting_str[PART_ID_SIZE];
	unsigned long long size;
	char	size_str[PART_ID_SIZE];
	char uuid[37+1];
};
#define MAX_PART_SIZE	(32)
struct part_descriptor part_table[MAX_PART_SIZE];
//virtual table
struct part_descriptor boot1_part_table[MAX_PART_SIZE];
struct part_descriptor boot2_part_table[MAX_PART_SIZE];
struct part_descriptor user_part_table[MAX_PART_SIZE];
static int boot1_idx = 0;
static int boot2_idx = 0;
static int user_idx = 0;
static int part_idx = 0;
static int part_table_init = 0;

#ifdef CONFIG_NVT_USE_PART_TABLE
char builtin_part_table[] = CONFIG_NVT_BUILTIN_PART_TABLE;
#endif 

extern /*static*/ char *print_efiname(gpt_entry *pte);
void show_part_table_item(struct part_descriptor * des)
{
	if(des->part_id_name[0]!= 0 ){
		printf("name=%s",des->part_id_name);
		if(des->part_in_file[0])
			printf("#%s",des->part_in_file);
		if(des->type[0])
			printf("#%s",des->type);
		if(des->starting_str[0])
			printf(",start=%s", des->starting_str);
		else
			printf(",start=%llu", des->starting);
		if(des->size){
			if(des->starting_str[0])
				printf(",size=%s", des->size_str);
			else
				printf(",size=%llu", des->size);
		}
		if(des->uuid[0])
			printf(",uuid=%s", des->uuid);
		printf(";\n");
	}

}
void reset_part_table(void)
{
	boot1_idx  = user_idx = boot2_idx = part_idx = 0;
	memset(part_table, 0x0, sizeof(part_table) );
	memset(boot1_part_table, 0x0, sizeof(boot1_part_table) );
	memset(boot2_part_table, 0x0, sizeof(boot2_part_table) );
	memset(user_part_table, 0x0, sizeof(user_part_table) );
}

void gen_part_table_item(/*int index,*/ struct part_descriptor * des, gpt_entry * gpt)
{
	char encode_str[PART_ID_SIZE] ;
	
	char * pch = NULL;
	strcpy( encode_str, print_efiname(gpt));
	//printf("encode str %s \n", encode_str);
	if(encode_str[0] == 0 )
		return ;
	//printf("start lb : %lld \n", gpt->starting_lba);
	des->starting = le64_to_cpu(gpt->starting_lba) * EMMC_BLOCK_SIZE;
	des->size = (le64_to_cpu( gpt->ending_lba ) - le64_to_cpu(gpt->starting_lba) +1 ) * EMMC_BLOCK_SIZE;
	pch = strtok(encode_str, "#");
	strcpy(des->part_id_name, pch); 
	//printf("pch %s %s\n",pch,des->part_id_name );
	
	pch = strtok(NULL, "#");
	if(pch == NULL){
		//des->part_in_file[0] = 0;
		strcpy(des->part_in_file, des->part_id_name); 
		return ;
	}
	
	//printf("pch2 %s %s\n",pch,des->part_id_name );
	strcpy(des->part_in_file, pch); 
	pch = strtok(NULL, "#");
	if(pch == NULL){
		des->type[0] = 0;
		return;
	}
	strcpy(des->type, pch); 
}
int _get_mmc_dev(struct blk_desc **dev_desc,struct mmc **mmc ){
	int curr_device = 0;

	memset(gpt_info, 0x0, INFO_SIZE);
	*mmc = init_mmc_device(curr_device, false);

	if (!mmc)
		return CMD_RET_FAILURE;

	*dev_desc = blk_get_devnum_by_type(IF_TYPE_MMC, curr_device);
	if (!dev_desc) {
		printf("%s: Invalid Argument(s)\n", __func__);
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}
#if 1
static int dump_gpt_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[], int update_table)
{
	struct blk_desc *dev_desc;
	//char cmd[64] ;
	struct mmc *mmc;
	ALLOC_CACHE_ALIGN_BUFFER_PAD(gpt_header, gpt_head, 1, dev_desc->blksz);

	if(_get_mmc_dev(&dev_desc, &mmc)){
		return CMD_RET_FAILURE;
	}

	gpt_entry *gpt_pte = NULL;
	int i = 0;
	char uuid[37];
	unsigned char *uuid_bin;
	//char *gpt_info_ptr = gpt_info;

	extern /*static*/ int is_gpt_valid(struct blk_desc *dev_desc, u64 lba,
			gpt_header *pgpt_head, gpt_entry **pgpt_pte);
	/* This function validates AND fills in the GPT header and PTE */
	//printf("gpt valid check for %d\n", GPT_PRIMARY_PARTITION_TABLE_LBA);
	if (is_gpt_valid(dev_desc, GPT_PRIMARY_PARTITION_TABLE_LBA,
			 gpt_head, &gpt_pte) != 1) {
		printf("%s: *** ERROR: Invalid GPT ***\n", __func__);
		if (is_gpt_valid(dev_desc, (dev_desc->lba - 1),
				 gpt_head, &gpt_pte) != 1) {
			printf("%s: *** ERROR: Invalid Backup GPT ***\n",
			       __func__);
			return CMD_RET_FAILURE;
		} else {
			printf("%s: ***        Using Backup GPT ***\n",
			       __func__);
		}
	}
	if(update_table)
		reset_part_table();
	debug("%s: gpt-entry at %p\n", __func__, gpt_pte);
	//printf("Part\tStart LBA\tEnd LBA\t\tName\n");
	//printf("\tAttributes\n");
	//printf("\tType GUID\n");
	//printf("\tPartition GUID\n");

	for (i = 0; i < le32_to_cpu(gpt_head->num_partition_entries); i++) {
		char part_item[128] = {0};
		unsigned long long start_addr = le64_to_cpu(gpt_pte[i].starting_lba) * EMMC_BLOCK_SIZE;
		unsigned long long size = (le64_to_cpu(gpt_pte[i].ending_lba) -  le64_to_cpu(gpt_pte[i].starting_lba)+1 ) 
					*EMMC_BLOCK_SIZE;

		uuid_bin = (unsigned char *)gpt_pte[i].unique_partition_guid.b;
		uuid_bin_to_str(uuid_bin, uuid, UUID_STR_FORMAT_GUID);
		//printf("\tguid:\t%s\n", uuid);
		/* Stop at the first non valid PTE */
/*extern*/ /*static*/ int is_pte_valid(gpt_entry * pte);
		if (!is_pte_valid(&gpt_pte[i]))
			break;
#if 0
		printf(" %d st %lld %llx   end %lld %llx \n", i
		, le64_to_cpu(gpt_pte[i].starting_lba) ,le64_to_cpu(gpt_pte[i].starting_lba) 
		,le64_to_cpu(gpt_pte[i].ending_lba),le64_to_cpu(gpt_pte[i].ending_lba)
		 );
#endif
		printf(  "name=%s,start=%lld,size=%lld,uuid=%s;\n",
			print_efiname(&gpt_pte[i]), start_addr, size,uuid
		);
		if(update_table){
			//update part_descriptor
			gen_part_table_item(&(part_table[i]), &(gpt_pte[i]));
			//end update part descriptor 
		}
		sprintf( part_item, "name=%s,start=%lld,size=%lld,uuid=%s;\n",
			print_efiname(&gpt_pte[i]), start_addr, size,uuid
		);
		//printf("item :%s\n", part_item);
		strcat(gpt_info, part_item);
	//printf("gpt info =>%s \n", gpt_info);
#if 0
		printf("%3d\t0x%08llx\t0x%08llx\t\"%s\"\n", (i + 1),
			le64_to_cpu(gpt_pte[i].starting_lba),
			le64_to_cpu(gpt_pte[i].ending_lba),
			print_efiname(&gpt_pte[i]));
		printf("\tattrs:\t0x%016llx\n", gpt_pte[i].attributes.raw);
		uuid_bin = (unsigned char *)gpt_pte[i].partition_type_guid.b;
		uuid_bin_to_str(uuid_bin, uuid, UUID_STR_FORMAT_GUID);
		printf("\ttype:\t%s\n", uuid);
		uuid_bin = (unsigned char *)gpt_pte[i].unique_partition_guid.b;
		uuid_bin_to_str(uuid_bin, uuid, UUID_STR_FORMAT_GUID);
		printf("\tguid:\t%s\n", uuid);
#endif
	}
	strcat(gpt_info, "\0\0\0\0");
	//printf("gpt info =>%s \n", gpt_info);
	//printf("%s %d \n",__FUNCTION__, __LINE__);
//#ifdef CONFIG_CMD_USB	
#if 0
	run_command("usb start", 0 );
	sprintf(cmd, "fatwrite usb 0 0x%p partition_nvt.txt %x", (void*)gpt_info, strlen(gpt_info));
	run_command(cmd, 0 );
#endif
	/* Remember to free pte */
	free(gpt_pte);
	if(update_table)
		part_table_init = 1;

	return CMD_RET_SUCCESS;
}

#endif
static int do_gpt_dump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return dump_gpt_info(cmdtp, flag, argc, argv, 0);
}

static int dump_nvtgpt_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i = 0;
	run_command("gpt_dump", 0 );
	//dump virtual table
	for(i = 0 ;i <MAX_PART_SIZE; i++){
		show_part_table_item(&boot1_part_table[i]);
	}
	for(i = 0 ;i <MAX_PART_SIZE; i++){
		show_part_table_item(&boot2_part_table[i]);
	}
	for(i = 0 ;i <MAX_PART_SIZE; i++){
		show_part_table_item(&user_part_table[i]);
	}
	return CMD_RET_SUCCESS;

}

static inline int le32_to_int(unsigned char *le32)
{
    return ((le32[3] << 24) +
	    (le32[2] << 16) +
	    (le32[1] << 8) +
	     le32[0]
	   );
}
/*static inline*/ int is_extended(int part_type);
/*static*/ void _print_one_part(dos_partition_t *p, int ext_part_sector,
			   int part_num, unsigned int disksig)
{
	int lba_start = ext_part_sector + le32_to_int (p->start4);
	int lba_size  = le32_to_int (p->size4);

	if(!is_extended(p->sys_ind) ){
		printf("name=p%d,start=%lld,size=%lld;\n",part_num,(unsigned long long)lba_start*EMMC_BLOCK_SIZE,
				(unsigned long long)lba_size*EMMC_BLOCK_SIZE);
	}
}

/*static*/ int test_block_type(unsigned char *buffer);
#if 0
/*static*/ void _print_partition_extended(struct blk_desc *dev_desc,
				     int ext_part_sector, int relative,
				     int part_num, unsigned int disksig)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
	dos_partition_t *pt;
	int i;

	if (dev_desc->block_read(dev_desc->dev, ext_part_sector, 1, (ulong *) buffer) != 1) {
		printf ("** Can't read partition table on %d:%d **\n",
			dev_desc->dev, ext_part_sector);
		return;
	}
	i=test_block_type(buffer);
	if (i != DOS_MBR) {
		printf ("bad MBR sector signature 0x%02x%02x\n",
			buffer[DOS_PART_MAGIC_OFFSET],
			buffer[DOS_PART_MAGIC_OFFSET + 1]);
		return;
	}

	if (!ext_part_sector)
		disksig = le32_to_int(&buffer[DOS_PART_DISKSIG_OFFSET]);

	/* Print all primary/logical partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		/*
		 * fdisk does not show the extended partitions that
		 * are not in the MBR
		 */

		if ((pt->sys_ind != 0) &&
		    (ext_part_sector == 0 || !is_extended (pt->sys_ind)) ) {
			_print_one_part(pt, ext_part_sector, part_num, disksig);
		}

		/* Reverse engr the fdisk part# assignment rule! */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) {
			part_num++;
		}
	}

	/* Follows the extended partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		if (is_extended (pt->sys_ind)) {
			int lba_start = le32_to_int (pt->start4) + relative;

			_print_partition_extended(dev_desc, lba_start,
				ext_part_sector == 0  ? lba_start : relative,
				part_num, disksig);
		}
	}

	return;
}
#endif
static int convert_mbr_to_gpt_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct blk_desc *dev_desc;
	//char cmd[64] ;
	struct mmc *mmc;
	if(_get_mmc_dev(&dev_desc, &mmc)){
		return CMD_RET_FAILURE;
	}

	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);
	dos_partition_t *pt;
	int i;
	int ext_part_sector = 0;
	int relative = 0;
        int part_num = 1 ;
	unsigned int disksig =0;

	if (blk_dread(dev_desc, ext_part_sector, 1, (ulong *)buffer) != 1) {
		printf ("** Can't read partition table on %d:" LBAFU " **\n",
			dev_desc->devnum, ext_part_sector);
		return;
	}
	i=test_block_type(buffer);
	if (i != DOS_MBR) {
		printf ("bad MBR sector signature 0x%02x%02x\n",
			buffer[DOS_PART_MAGIC_OFFSET],
			buffer[DOS_PART_MAGIC_OFFSET + 1]);
		return CMD_RET_FAILURE;
	}

	if (!ext_part_sector)
		disksig = le32_to_int(&buffer[DOS_PART_DISKSIG_OFFSET]);

	/* Print all primary/logical partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		/*
		 * fdisk does not show the extended partitions that
		 * are not in the MBR
		 */

		if ((pt->sys_ind != 0) &&
		    (ext_part_sector == 0 || !is_extended (pt->sys_ind)) ) {
			_print_one_part(pt, ext_part_sector, part_num, disksig);
		}

		/* Reverse engr the fdisk part# assignment rule! */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) {
			part_num++;
		}
	}
	/* Follows the extended partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		if (is_extended (pt->sys_ind)) {
			int lba_start = le32_to_int (pt->start4) + relative;

			print_partition_extended(dev_desc, lba_start,
				ext_part_sector == 0  ? lba_start : relative,
				part_num, disksig);
		}
	}

	return CMD_RET_SUCCESS;
}

struct part_descriptor  parse_gpt_line(char * string)
{
//parse a line in partition.txt
#define STR_SIZE 64 
	struct part_descriptor part_des;
	char *pch = strtok(string, ",;");
	int len = 0;
#define DBG_GPT_LINE(...) //printf(__VA_ARGS__)	
	memset(&part_des, 0x0, sizeof(struct part_descriptor));

        while(pch){
                //splite by strtok.  
		do{
			char  *map_ptr;
			char name[32],value[STR_SIZE],map[STRING_BUF_SIZE];
			char *value_str = value;
			strcpy(map, pch);
			DBG_GPT_LINE("map %s\n", map);
			map_ptr = strtok(map, "=;");
			if(map_ptr == NULL){
				printf("ERROR partititon format!!!%s\n",map);
				while(1);
			}

			strcpy(name, map_ptr);
			map_ptr = strtok(NULL, "=;");
			if(map_ptr == NULL){
				printf("ERROR partititon format!!!%s\n",map);
				while(1);
			}
			strcpy(value,map_ptr);
			DBG_GPT_LINE("line: name:%s value:%s \n", name, value);
			//we want to get type

			if(!strncmp(name,"name", 4)){
				char *name_ptr;
				name_ptr =  strtok(value,"#");
				strcpy(part_des.part_id_name ,name_ptr);
				name_ptr =  strtok(NULL,"#");
				if(name_ptr == NULL){
					//goto done;
					break;
				}
				strcpy(part_des.part_in_file ,name_ptr);

				name_ptr =  strtok(NULL,"#");
				if(name_ptr == NULL)
					 //goto done;
					break;
				strcpy(part_des.type ,name_ptr);
				//goto done;
			}

			if(!strncmp(name,"start", 5)){
				strcpy(part_des.starting_str ,value);
				part_des.starting = ustrtoull(value, &value_str, 10);
				//printf("--> %s ,%lld", value, part_des.starting);
			}

			if(!strncmp(name,"size", 4)){
				strcpy(part_des.size_str ,value);
				part_des.size = ustrtoull(value, &value_str, 10);
				//printf("--> %s ,%lld", value, part_des.size);
			}

			if(!strncmp(name,"uuid", 4)){
				strncpy(part_des.uuid, value, 37);
				part_des.uuid[37] = 0;
				//printf("--> %s ,%lld", value, part_des.size);
			}

		}while(0);
                //printf("len : %d \n", strlen(pch));
                len = strlen(pch);
                pch = strtok(pch+len+1, ",;");
                //if(pch == NULL)
                  //      printf("PCH NULL");
        }

//done:
	DBG_GPT_LINE("part des : %s %s %s [%llu %llu] \n", part_des.part_id_name, part_des.part_in_file, part_des.type, part_des.starting, part_des.size);
	return part_des;
}
char*  __get_line(char *start,  int *len)
{
	char *ptr = start;
	char *ptr_end =  0;
	char *str = 0;
	while(*ptr == '\r' || *ptr=='\n'){
		ptr++;
	}
	str  = ptr;

	//printf("%d\n",__LINE__);
	if(*ptr == 0){
		return str = NULL;
	}

	//printf("%d\n",__LINE__);
	while(1){
		if(*ptr == '\r' || *ptr=='\n' || *ptr == 0){
			ptr_end  = ptr;
			break;
		}
		//printf("[%c %d]",*ptr, *ptr);
		ptr++;
	}
	//printf("\n");
	//printf("%d\n",__LINE__);
	*len = ptr_end - str;
	//printf("len %d\n",*len);
	return str;
		
}

static int refresh_part_table(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char gpt_info_read[INFO_SIZE] = {0};
	char cmd[STR_SIZE] = {0};
	int i = 0;
#define DBG_PARSE_LINE(...) //printf(__VA_ARGS__)

	if(argc == 2 ){
		char string[128] ="";
#if ( defined(CONFIG_CMD_FAT)&& defined(CONFIG_CMD_USB) )
		if(act->init[0]){
			strcpy(string, act->init);
			run_command(string,0);
		}
		//fat load partition.txt
		memset (gpt_info_read, 0x0, INFO_SIZE);
		//memset (gpt_info_cmd, 0x0, 10240);
		//sprintf(cmd ,"fatload usb 0 0x%x partition.txt ", (void*)gpt_info_read);
		memset(string, 0x0, 128);
		strcpy(string, act->read);
		//sprintf(cmd ,"fatload usb 0 0x%p partition_nvt.txt ", (void*)gpt_info_read);
		sprintf(cmd ,string, (void*)gpt_info_read, "partition_nvt.txt");
		//printf("fatload usb 0 0x%p partition_nvt.txt ", (void*)gpt_info_read);
		run_command(cmd,0);
#if 0
		sprintf(cmd ,"md 0x%p ", (void*)gpt_info_read);
		run_command(cmd,0);
#endif
#else
		printf("ERROR : usb/fat is needed to use this function\n");
		return CMD_RET_FAILURE;
#endif	
	}else if( argc == 4 ){
		void *addr;
		u32 length = 0;
		addr = (void *)simple_strtoul(argv[2], NULL, 16);
		length = simple_strtoul(argv[3], NULL, 16);
	
		memcpy(gpt_info_read, addr, length);
	}else{
		return CMD_RET_USAGE;
	}
	boot1_idx  = user_idx = boot2_idx = part_idx = 0;
	reset_part_table();

	while(i<INFO_SIZE){
		char string[STRING_BUF_SIZE] ;
		int str_len = 0;
		char *str = __get_line(gpt_info_read+i,&str_len);	
		struct part_descriptor des ;//des = parse_gpt_line(string);

		//printf(">>>string %s %d\n", gpt_info_read+i,i);
		if(!str){
			DBG_PARSE_LINE("parse_done\n");
			break;	//the end of content	
		}
		//printf("%c %c %c %c %c %d\n", str[0],str[1],str[2],str[3],str[4],str_len);
		memset(string, 0x0, STRING_BUF_SIZE);
		memcpy(string, str, str_len);

		DBG_PARSE_LINE("!!!!!string %s %d \n", string, i);
		des = parse_gpt_line(string);
		if(!strcmp(des.type, "boot1")){
			DBG_PARSE_LINE("dump to boot1\n");
			//memcpy(&(boot1_part_table[boot1_idx++]), &des,  sizeof(struct part_descriptor ));	
			boot1_part_table[boot1_idx++] = des;
			show_part_table_item(&boot1_part_table[boot1_idx-1]);
		}
		else if(!strcmp(des.type, "boot2")){
			DBG_PARSE_LINE("dump to boot2\n");
			//memcpy(&(boot2_part_table[boot2_idx++]), &des,  sizeof(struct part_descriptor ));	
			boot2_part_table[boot2_idx++] = des;
			show_part_table_item(&boot2_part_table[boot2_idx-1]);
		}
		else if(!strcmp(des.type, "user")){
			//memcpy(&(user_part_table[user_idx++]), &des,  sizeof(struct part_descriptor))	;
			user_part_table[user_idx++] = des;
		}else{
			DBG_PARSE_LINE("dump to buffer\n");
			//j+= str_len;
			part_table[part_idx++] = des;
		}
		i = str - gpt_info_read + str_len;
		//ptr+=(len+1);
	}
	part_table_init = 1;
	return CMD_RET_SUCCESS;	
} 

static int update_gpt_partition(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char gpt_info_read[INFO_SIZE] = {0};
	char gpt_info_cmd[INFO_SIZE] = {"gpt write mmc 0 "};
	char cmd[256] = {0};
	int i = 0,j = 0;
	int ret = 0;
	//get data
	if(argc == 1 ){
#if ( defined(CONFIG_CMD_FAT)&& defined(CONFIG_CMD_USB) )
		char string[128] ="";
		if(act->init[0]){
			strcpy(string, act->init);
			run_command(string,0);
		}

		//fat load partition.txt
		memset (gpt_info_read, 0x0, INFO_SIZE);
		//memset (gpt_info_cmd, 0x0, 10240);
		//sprintf(cmd ,"fatload usb 0 0x%x partition.txt ", (void*)gpt_info_read);
		//sprintf(cmd ,"fatload usb 0 0x%p partition_nvt.txt ", (void*)gpt_info_read);
		memset(string, 0x0, 128);
		strcpy(string, act->read);
		sprintf(cmd , string, (void*)gpt_info_read, "partition_nvt.txt" );
		//printf("fatload usb 0 0x%p partition_nvt.txt ", (void*)gpt_info_read);
		run_command(cmd,0);
#if 0
		sprintf(cmd ,"md 0x%p ", (void*)gpt_info_read);
		run_command(cmd,0);
#endif
		//printf("%d\n",__LINE__);
#else
		printf("ERROR : usb/fat is needed to use this function\n");
		return 0;
#endif	
	}else if( argc == 3 ){
		void *addr;
		u32 length = 0;
		addr = (void *)simple_strtoul(argv[1], NULL, 16);
		length = simple_strtoul(argv[2], NULL, 16);
	
		memcpy(gpt_info_read, addr, length);
	}else{
		return CMD_RET_USAGE;
	}

#if 0	//debug
	printf("j: %d %x\n", j,(unsigned char)gpt_info_cmd[j] );
#endif
	//printf("%d\n",__LINE__);
	j = strlen(gpt_info_cmd);
	gpt_info_cmd[j++] = '"';
	//printf("%d\n",__LINE__);
#if 0
	for(i = 0 ; i< INFO_SIZE;i++){

		if(gpt_info_read[i]=='\0')
			break;

		if(gpt_info_read[i]=='\n' || gpt_info_read[i] == '\r'){
			//gpt_info_read[i] = ' ';
		}else{
			gpt_info_cmd[j] = gpt_info_read[i];
			j++;
		}
	}
#endif
	//todo build virtual table here
#if 1
	//get line: ptr size
	//parse line
	i = 0;
	boot1_idx  = user_idx = boot2_idx = 0;
	reset_part_table();
	while(i<INFO_SIZE){
		char string[256] ;
		char string_[256] ;
		int str_len = 0;
		char *str = __get_line(gpt_info_read+i,&str_len);	
		struct part_descriptor des ;//des = parse_gpt_line(string);
	
		//printf(">>>string %s %d\n", gpt_info_read+i,i);
		if(!str){
			DBG_PARSE_LINE("parse_done\n");
			break;	//the end of content	
		}
		//printf("%c %c %c %c %c %d\n", str[0],str[1],str[2],str[3],str[4],str_len);
		memset(string, 0x0,256);
		memcpy(string, str, str_len);
		memset(string_, 0x0,256);
		memcpy(string_, str, str_len);
		
		DBG_PARSE_LINE("!!!!!string %s %d \n", string, i);
		des = parse_gpt_line(string);
		if(!strcmp(des.type, "boot1")){
			DBG_PARSE_LINE("dump to boot1\n");
			//memcpy(&(boot1_part_table[boot1_idx++]), &des,  sizeof(struct part_descriptor ));	
			boot1_part_table[boot1_idx++] = des;
			show_part_table_item(&boot1_part_table[boot1_idx-1]);
		}
		else if(!strcmp(des.type, "user")){
			//memcpy(&(user_part_table[user_idx++]), &des,  sizeof(struct part_descriptor))	;
			user_part_table[user_idx++] = des;
		}else{
			DBG_PARSE_LINE("dump to buffer\n");
			memcpy(gpt_info_cmd+j, string_, str_len );
			j+= str_len;
		}
		i = str - gpt_info_read + str_len;
		//ptr+=(len+1);
	}	
#endif
#if 0 //debug
	printf("cmd buf:0x%p \n", gpt_info_cmd);
#endif
	gpt_info_cmd[j++] = '"';
	gpt_info_cmd[j] = 0;
#ifdef DUMP_GPT_INFO_CMD
	printf("-------------\n");
	sprintf(cmd ,"md 0x%p 120", gpt_info_cmd);
	run_command(cmd,0);
#endif
#if 0 //debug
	j = 0;
	printf("check cmd\n");
	while(gpt_info_cmd[j]){
		printf("%c", gpt_info_cmd[j]);
		j++;
	}
#endif
	//run command
	ret = run_command(gpt_info_cmd,0);
	if(ret)
		printf("write gpt fial");
	ret = run_command("mmc rescan",0);//update uboot partition information
	
	part_table_init = 1;
	return CMD_RET_SUCCESS;
	
}
#if 0
static int gtable_up(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return 0;
}
#endif
#define BYTE_TO_SECTOR_SFT 	9
void gwrite(char *img_name/*, int index, int offset, int size*/,struct part_descriptor *des)
{
	char cmd[STRING_BUF_SIZE] = {0};
	int ret = 0;
	unsigned long read_blks = 0;
	//unsigned int read_len;
	//unsigned long mmc_sector_addr;
	//unsigned long mmc_sector_size;
	char* type=des->type;
	unsigned long  flen;
	//check usb file
	//
	//int index,;
	unsigned long offset = des->starting;
	unsigned long size = des->size;
	char string[128] ="";
	if(act->init[0]){
		strcpy(string, act->init);
		run_command(string,0);
	}
	//	run_command("mmc rescan",0);//this is walk around for 172 bringup case.
	if(strcmp(type, "rpmb")== 0){
		ret = run_command("mmc dev 0 3", 0);
	}else if(strcmp(type, "boot1")== 0){
		ret = run_command("mmc dev 0 1", 0);
	}else if(strcmp(type, "boot2")== 0){
		ret = run_command("mmc dev 0 2", 0);
	}else{
		ret = run_command("mmc dev 0 0", 0);
		//ret = run_command("mmc dev 0 0", 0);//this is walk around for 172 bringup case.
	}

#define NVT_FWUPDATE_MAX_WRITE_BYTES 		(450 * 1024 *1024)
#define NVT_FWUPDATE_MAX_WRITE_BLKS 		(NVT_FWUPDATE_MAX_WRITE_BYTES >> BYTE_TO_SECTOR_SFT)

	//mmc write
		//load from usb
		//write to mmc 
	unsigned long  mmc_write_addr = offset>>BYTE_TO_SECTOR_SFT;
	//printf("i[DEBUG]offset :%u %x mmc_write_addr %lld %llx\n", offset,offset, mmc_write_addr, mmc_write_addr);
	do {
		if(act == &usb_act){
			sprintf(cmd, "fatload usb 0:auto 0x%x %s 0x%x 0x%lx", CONFIG_SYS_FWUPDATE_BUF,
					img_name, NVT_FWUPDATE_MAX_WRITE_BYTES, (read_blks << BYTE_TO_SECTOR_SFT));
		}else if(act == &eth_act){
			#if 0
			/*TFTP protocol*/
			sprintf(cmd, "tftp 0x%x %s ", CONFIG_SYS_FWUPDATE_BUF, img_name);
			#else
			/*NFS protocol*/
			if ((env_get("burn_folder") != NULL) && (env_get("serverip") != NULL)) {
				sprintf(cmd, "nfs 0x%x %s:%s%s",
					CONFIG_SYS_FWUPDATE_BUF,
					env_get("serverip"),
					env_get("burn_folder"),
					img_name);
			} else {
				printf("gwrite: source error (Hint: No burn_folder or serverip)\n");
				break;
			}
			#endif
		}else{
			printf("gwrite: %s %d source error\n", __FUNCTION__, __LINE__);
			break;
		}

		//printf("ewrite fatcmd: %s\n", cmd);
		ret = run_command(cmd, 0);
		if(ret != 0) {
			printf("read image file %s error !\n", img_name);
			ret = -EIO;
			goto out;
		}
		flen = simple_strtoul(env_get("filesize"), NULL, 16);

		if(/*part_table[i].*/size < (read_blks + (flen >> BYTE_TO_SECTOR_SFT))) {
			printf("%s image size %ld large than partition size %ld, update fail !\n", img_name, size, (read_blks + (flen >> BYTE_TO_SECTOR_SFT)));
			ret = -EINVAL;
			goto out;
		}
#if 1
		unsigned long mmc_blk_size = (!(flen % 512))? flen>>BYTE_TO_SECTOR_SFT: (flen>>BYTE_TO_SECTOR_SFT)+1;
		
		sprintf(cmd, "mmc write %p 0x%lx 0x%lx",(unsigned char*)CONFIG_SYS_FWUPDATE_BUF, 
			mmc_write_addr+read_blks , mmc_blk_size);
		ret = run_command(cmd, 0);
		if(ret) break;
#endif
		if(flen < NVT_FWUPDATE_MAX_WRITE_BYTES)
			break;
		else
			read_blks += NVT_FWUPDATE_MAX_WRITE_BLKS;
	}while(1);
	//back to init start
out:
	ret = run_command("mmc dev 0 0", 0);
	ret = run_command("mmc dev 0 0", 0);
}

void dump_part_table(void)
{
	int i = 0;
	printf("-----------------\n");
	for(i = 0 ;i < MAX_PART_SIZE; i++){
		show_part_table_item(&part_table[i]);
	}
	printf("------boot1-----------\n");
	for(i = 0 ;i < MAX_PART_SIZE; i++){
		show_part_table_item(&boot1_part_table[i]);
	}
	printf("------boot2-----------\n");
	for(i = 0 ;i < MAX_PART_SIZE; i++){
		show_part_table_item(&boot2_part_table[i]);
	}
	printf("------user -----------\n");
	for(i = 0 ;i < MAX_PART_SIZE; i++){
		show_part_table_item(&user_part_table[i]);
	}
	printf("-----------------\n");
}

void process_part_table(char * part_id_name, struct part_descriptor table[], int i, char file_name[]  )
{
	char input_file[PART_ID_SIZE] = "\0";
	//printf("p[%s][dest %s][file %s][i: %d ]\n", part_id_name, table[i].part_id_name, file_name, i);
	if(!part_id_name[0])
		return;
	if(0 == strcmp(part_id_name, table[i].part_id_name)){
		if(!file_name[0]){
			strcpy(input_file,table[i].part_in_file );
		}else {
			strcpy(input_file, file_name );
		}
		printf("buring file (%s) by name %s \n", input_file ,part_id_name);	
		gwrite(input_file,  &table[i]);
		//gwrite(file_name, i , part_table[i].starting, part_table[i].size);
	}
}

void show_part_table_help(struct part_descriptor * des, int index)
{
	char mmc_str[MAX_PART_SIZE]="" ;

	if(!(des->type[0]))
		sprintf(mmc_str, "mmcblk0p%d", index+1);

	if(des->part_id_name[0]!= 0 )
		printf("Usage: gwrite %s [%s type:%s -%s ]{start=%llu,size=%llu} \n",
			des->part_id_name, des->part_in_file, des->type, mmc_str, des->starting, des->size
		 );
}

void do_gpt_write_help(void)
{
	int i = 0;
	printf("------gpt write help-----------\n");
	printf("Usage: gwrite all [all partition]\n");
	for(i = 0 ;i < MAX_PART_SIZE; i++){
		show_part_table_help(&part_table[i], i);
	}

	printf("------boot1-----------\n");
	for(i = 0 ;i < MAX_PART_SIZE; i++){
		show_part_table_help(&boot1_part_table[i], i);
	}

	printf("------boot2-----------\n");
	for(i = 0 ;i < MAX_PART_SIZE; i++){
		show_part_table_help(&boot2_part_table[i], i );
	}

	printf("------user -----------\n");
	for(i = 0 ;i < MAX_PART_SIZE; i++){
		show_part_table_help(&user_part_table[i], i);
	}
	printf("-----------------\n");

}


static int do_write_for_gpt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	//get argument
	
	char file_name[PART_ID_SIZE]="\0" ;
	char *part_id_name ;
	int i = 0 ;
	int all_burn = 0;
	
	int ret = 0;

	if(argc <2 || ret  ){
		printf("argument error or gpt table error!\n");
		return CMD_RET_FAILURE;
	}

	if(!part_table_init){
#ifdef CONFIG_NVT_USE_PART_TABLE 
		char cmd[STRING_BUF_SIZE] = "";
		char op_cmd[STRING_BUF_SIZE] = "";
		
		if(act == &usb_act)
			sprintf(op_cmd,"%s", "gpart") ;
		else if(act == &eth_act)
			sprintf(op_cmd,"%s", "gipart") ;
			
		sprintf(cmd, "%s update 0x%p 0x%x", op_cmd, builtin_part_table, sizeof(builtin_part_table));
		ret = run_command(cmd, 0 );
#else
		ret = dump_gpt_info(cmdtp, flag, argc, argv, 1);
#endif
	}
	if(strcmp(argv[1], "help") == 0){
		do_gpt_write_help();
		return CMD_RET_SUCCESS;	
	}

	part_id_name = argv[1];

	if(strcmp(part_id_name,"all")==0)
		all_burn = 1;
	else{
		if(argc == 2 ){
			strcpy(file_name, argv[1]);
		}
	}
	
	if(argc >=3 ){
		strcpy(file_name, argv[2]);
	}	
	
	//printf("argv : %s %s \n", argv[1], argv[2]);

	//debug : dump part_table
	//dump_part_table();
#define PROCESS_TABLE(TABLE) 					\
	if(all_burn){						\
		part_id_name = TABLE[i].part_id_name;		\
		sprintf(file_name, "%s", part_id_name);		\
	}							\
	process_part_table(part_id_name, TABLE, i, file_name );	

	for(i = 0 ; i < MAX_PART_SIZE;i++){
		PROCESS_TABLE(part_table);
		PROCESS_TABLE(boot1_part_table);
		PROCESS_TABLE(boot2_part_table);
		PROCESS_TABLE(user_part_table);
	}
#undef PROCESS_TABLE
	return CMD_RET_SUCCESS;
}

static int do_ewrite_for_gpt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	act = & usb_act;

	return do_write_for_gpt(cmdtp, flag, argc, argv);
}

static int do_iwrite_for_gpt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	act = & eth_act;

	return do_write_for_gpt(cmdtp, flag, argc, argv);
}

static int do_writeall_for_gpt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	//get argument
	
	char file_name[PART_ID_SIZE]="\0" ;
	char dir[STRING_BUF_SIZE]="" ;
	char *part_id_name ;
	int i = 0 ;
	
	int ret = 0;

	if(argc <1 || ret  ){
		printf("argument error or gpt table error!\n");
		return CMD_RET_FAILURE;
	}

	if(!part_table_init){
#ifdef CONFIG_NVT_USE_PART_TABLE 
		char cmd[STRING_BUF_SIZE] = "";
		sprintf(cmd, "gpart update 0x%p 0x%x", builtin_part_table, sizeof(builtin_part_table));
		ret = run_command(cmd, 0 );
#else
		ret = dump_gpt_info(cmdtp, flag, argc, argv, 1);
#endif
	}

	if(argc > 1 )
		sprintf(dir,"%s/",argv[1]);	
	
	//printf("argv : %s %s \n", argv[1], argv[2]);

	//debug : dump part_table
	//dump_part_table();
#define PROCESS_TABLE(TABLE) 						\
	part_id_name = TABLE[i].part_id_name;				\
	sprintf(file_name, "%s%s", dir, part_id_name);			\
	process_part_table(part_id_name, TABLE, i, file_name );	

	for(i = 0 ; i < MAX_PART_SIZE;i++){
		PROCESS_TABLE(part_table);
		PROCESS_TABLE(boot1_part_table);
		PROCESS_TABLE(boot2_part_table);
		PROCESS_TABLE(user_part_table);
	}

#undef PROCESS_TABLE

	return CMD_RET_SUCCESS;
}

static int update_part_table(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i = 0,j = 0;
	char gpt_info_cmd[INFO_SIZE] = {"gpt write mmc 0 "};
	int ret = 0;
	//char cmd[STRING_BUF_SIZE] = {0};

	j = strlen(gpt_info_cmd);
	gpt_info_cmd[j++] = '"';

	int len = strlen(gpt_info_cmd);
	char *str_ptr = gpt_info_cmd + len;

	for(i = 0 ;i <MAX_PART_SIZE; i++){
		struct part_descriptor * des = &part_table[i];
		//char str[STRING_BUF_SIZE] = "";
		if(des->part_id_name[0]!= 0 ){
			len = strlen(gpt_info_cmd);
			str_ptr = gpt_info_cmd + len;
			sprintf(str_ptr, "name=%s", des->part_id_name);
			len = strlen(gpt_info_cmd);
			str_ptr = gpt_info_cmd + len;
			if(des->part_in_file[0]){
				sprintf(str_ptr, "#%s", des->part_in_file);
			}			
			len = strlen(gpt_info_cmd);
			str_ptr = gpt_info_cmd + len;
			sprintf(str_ptr, ",start=%llu", des->starting);
			len = strlen(gpt_info_cmd);
			str_ptr = gpt_info_cmd + len;
			if(des->size){
				sprintf(str_ptr, ",size=%llu", des->size);
			}
			len = strlen(gpt_info_cmd);
			str_ptr = gpt_info_cmd + len;
			if(des->uuid[0]){
				sprintf(str_ptr, ",uuid=%s;", des->uuid);
			}else{
				sprintf(str_ptr, ";");
			}
			len = strlen(gpt_info_cmd);
			str_ptr = gpt_info_cmd + len;
			
#if  0
			sprintf(cmd ,"md 0x%p ", (void*)gpt_info_cmd);
			run_command(cmd,0);
#endif
		}
	}
	len = strlen(gpt_info_cmd);
	str_ptr = gpt_info_cmd + len;
	sprintf(str_ptr, "\"");

#if 0
		sprintf(cmd ,"md 0x%p ", (void*)gpt_info_cmd);
		run_command(cmd,0);
#endif

	//printf("gpt command : %s\n", gpt_info_cmd);
	ret = run_command(gpt_info_cmd,0);
	if(ret)
		printf("write gpt fial");

	return CMD_RET_SUCCESS;	
}

void nvt_part_info_list(void)
{
	int i = 0;
	//dump virtual table
	for(i = 0 ;i <MAX_PART_SIZE; i++){
		show_part_table_item(&part_table[i]);
	}
	for(i = 0 ;i <MAX_PART_SIZE; i++){
		show_part_table_item(&boot1_part_table[i]);
	}
	for(i = 0 ;i <MAX_PART_SIZE; i++){
		show_part_table_item(&boot2_part_table[i]);
	}
	for(i = 0 ;i <MAX_PART_SIZE; i++){
		show_part_table_item(&user_part_table[i]);
	}
}

static int do_gpt_part(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = CMD_RET_SUCCESS;
	if(argc == 1 )
		return CMD_RET_USAGE;

	/*update partition list without generating GPT.*/
	if(strcmp(argv[1], "load") == 0){
		return refresh_part_table(cmdtp, flag, argc, argv);
	}

	if(!part_table_init){
		//ret = run_command("gpt_update", 0 );
#ifdef CONFIG_NVT_USE_PART_TABLE 
		char cmd[STRING_BUF_SIZE] = "";
		sprintf(cmd, "gpart load 0x%p 0x%x", builtin_part_table, sizeof(builtin_part_table));
		ret = run_command(cmd, 0 );
#else
		ret = dump_gpt_info(cmdtp, flag, argc, argv, 1);
#endif
	}
	/*info read write*/
	if(strcmp(argv[1], "info") == 0){
		nvt_part_info_list();
		return CMD_RET_SUCCESS;	
	}

	/*dump current partition?*/
	if(strcmp(argv[1], "read") == 0){
		return dump_gpt_info(cmdtp, flag, argc, argv,0);
	}

	if(strcmp(argv[1], "readmbr") == 0){
		return convert_mbr_to_gpt_info(cmdtp, flag, argc, argv);
	}

	/*generate GPT using part table*/
	if(strcmp(argv[1], "write") == 0){
		return update_part_table(cmdtp, flag, argc, argv);
	}


	if(strcmp(argv[1], "local") == 0){
		return dump_gpt_info(cmdtp, flag, argc, argv,1);
	}
#ifdef CONFIG_NVT_USE_PART_TABLE
	if(strcmp(argv[1], "builtin") == 0){
		char cmd[STRING_BUF_SIZE] = "";
		sprintf(cmd, "gpart update 0x%p 0x%x", builtin_part_table, sizeof(builtin_part_table));
		return  run_command(cmd, 0 );
	}
#endif
	return ret;	
}

static int do_igpt_part(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct src_action * pre_act = act;
	int ret = 0;

	act = & usb_act;
	ret = do_gpt_part( cmdtp, flag, argc, argv);
	act = pre_act;

	return ret;
}

static int do_egpt_part(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct src_action * pre_act = act;
	int ret = 0;

	act = & usb_act;
	ret = do_gpt_part( cmdtp, flag, argc, argv);
	act = pre_act;
	
	return ret;
}

static int do_boot_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	char cmd[STRING_BUF_SIZE] = {0};
	unsigned int flen;
	char *img_name = argv[1];
	unsigned int mmc_write_addr = 0;

	char string[128] ="";
	if(act->init[0]){
		strcpy(string, act->init);
		run_command(string,0);
	}
	ret = run_command("mmc dev 0 1", 0);
	//sprintf(cmd, "fatload usb 0:auto 0x%x %s ", CONFIG_SYS_FWUPDATE_BUF,
	//		img_name);
	memset(string, 0x0, 128);
	strcpy(string, act->read);
	//sprintf(cmd ,"fatload usb 0 0x%p partition_nvt.txt ", (void*)gpt_info_read);
	sprintf(cmd ,string, CONFIG_SYS_FWUPDATE_BUF, img_name);
	ret = run_command(cmd, 0);
	flen = simple_strtoul(env_get("filesize"), NULL, 16);
	if(ret != 0) {
		printf("read image file %s error !\n", img_name);
		ret = -EIO;
		return CMD_RET_FAILURE;
	}

	unsigned int mmc_blk_size = (!(flen % 512))? flen>>BYTE_TO_SECTOR_SFT: (flen>>BYTE_TO_SECTOR_SFT)+1;

	sprintf(cmd, "mmc write %p 0x%x 0x%x",(unsigned char*)CONFIG_SYS_FWUPDATE_BUF, 
			mmc_write_addr , mmc_blk_size);
	ret = run_command(cmd, 0);
	ret = run_command("mmc dev 0 0", 0);

	return CMD_RET_SUCCESS;
}

static int do_gy(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	
	ret = run_command("mmc_bootconfig", 0);
	ret = run_command("gpart load", 0);
	ret = run_command("gpart write", 0);
	ret = run_command("gwriteall", 0);
	ret = run_command("gb onboot.bin", 0);
	
	return ret;
}

U_BOOT_CMD(
	gwrite, 3, 0, do_ewrite_for_gpt,
	"update binary",
	"- update binary from USB"
);

U_BOOT_CMD(
	giwrite, 3, 0, do_iwrite_for_gpt,
	"update binary",
	"- update binary from USB"
);

U_BOOT_CMD(
	gwriteall, 3, 0, do_writeall_for_gpt,
	"update binary from specific dirctory",
	"- update binary from USB specific dirctory"
);

U_BOOT_CMD(
	gpt_dump, 1, 0, do_gpt_dump,
	"display gpt info",
	"- display gpt info of the current MMC device"
);

U_BOOT_CMD(
	nvt_gpt_dump, 1, 0, dump_nvtgpt_info,
	"display gpt info",
	"- display gpt info of the current MMC device"
);

#if 0
U_BOOT_CMD(
	gpt_dump_mbr, 1, 0, convert_mbr_to_gpt_info,
	"display gpt info converted from DOS MBR",
	"- display gpt info of the current MMC device converted from DOS MBR"
);
#endif


U_BOOT_CMD(
	gpt_update, 3, 0, update_gpt_partition,
	"update gpt info",
	"- update gpt info of the current MMC device\n\
	gpt_update - update from usb partition_nvt.txt\n\
	gpt_update ADDR SIZE - update from config stored in memory\n\
	"
);

U_BOOT_CMD(
	gpart,4 , 0, do_egpt_part,
	"gpt partition tool",
	" - gpt partition tool\n\
	gpart info - show partition table struct\n\
	gpart load - update partition table struct from partition_nvt.txt\n\
	gpart load MEM_ADDR SIZE- update partition table struct from memory\n\
	gpart write - generate GPT by partition table struct\n\
	gpart read - show current partition table on disk\n\
	gpart readmbr - show current MBR partition table on disk\n\
	gpart local - update partition table by  partition  on disk\n\
	gpart builtin - update partition table by  uboot builtin\n\
	"
);

U_BOOT_CMD(
	gipart,4 , 0, do_igpt_part,
	"gpt partition tool",
	" - gpt partition tool\n\
	gpart info - show partition table struct\n\
	gpart load - update partition table struct from partition_nvt.txt\n\
	gpart load MEM_ADDR SIZE- update partition table struct from memory\n\
	gpart write - generate GPT by partition table struct\n\
	gpart read - show current partition table on disk\n\
	gpart readmbr - show current MBR partition table on disk\n\
	gpart local - update partition table by  partition  on disk\n\
	gpart builtin - update partition table by  uboot builtin\n\
	"
);

U_BOOT_CMD(
	gb, 2, 0, do_boot_write ,
	"write bootloader to emmc boot1",
	" - write bootloader to emmc boot1n\
	"
);


U_BOOT_CMD(
	gyss, 1, 0, do_gy,
	"update common_packge",
	"- update common package \n\
	"
);
