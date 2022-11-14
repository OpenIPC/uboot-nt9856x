#ifndef __NVT_NAND_UTILS_H__
#define __NVT_NAND_UTILS_H__

#define NDUMP_MAX_READ (1024*1024*100) // 100M

int nvt_run_cmd(const char *cmd, int flag, const char *description);

#endif /* __NVT_NAND_UTILS_H__ */
