#ifndef __DISPDEV_MAIN_H__
#define __DISPDEV_MAIN_H__
#include <linux/cdev.h>
#include <linux/types.h>
#include "dispdev_drv.h"


#define MODULE_MINOR_ID      0
#define MODULE_MINOR_COUNT   1
#define MODULE_NAME          "nvt_dispdev"

typedef struct dispdev_drv_info {
	MODULE_INFO module_info;

	struct class *pmodule_class;
	struct device *pdevice[MODULE_MINOR_COUNT];
	struct resource *presource[MODULE_REG_NUM];
	struct cdev cdev;
	dev_t dev_id;

	// proc entries
	struct proc_dir_entry *pproc_module_root;
	struct proc_dir_entry *pproc_help_entry;
	struct proc_dir_entry *pproc_cmd_entry;
} DISPDEV_DRV_INFO, *PDISPDEV_DRV_INFO;


#endif
