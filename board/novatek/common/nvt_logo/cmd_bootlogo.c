
#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <part.h>
#include <asm/hardware.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/shm_info.h>
#include <stdlib.h>
#include <linux/arm-smccc.h>
#include "cmd_bootlogo.h"
#include <asm/arch/display.h>
#include <asm/arch/top.h>
#include <linux/libfdt.h>

#include "logo.dat"   //jpg bitstream binary

extern void jpeg_setfmt(unsigned int fmt);
extern void jpeg_decode(unsigned char* inbuf, unsigned char* outbuf);
extern void jpeg_getdim(unsigned int* width, unsigned int* height);

UINT32 dma_getDramBaseAddr(DMA_ID id);
UINT32 dma_getDramCapacity(DMA_ID id);

#define HEAVY_LOAD_CTRL_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_CTRL_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_ADDR_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_START_ADDR_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_SIZE_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_DMA_SIZE_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_WAIT_CYCLE_OFS(ch)   (DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE_OFS + ((ch) * 0x10))

#define PROTECT_START_ADDR_OFS(ch)      (DMA_PROTECT_STARTADDR0_REG0_OFS+(ch)*8)
#define PROTECT_END_ADDR_OFS(ch)        (DMA_PROTECT_STOPADDR0_REG0_OFS+(ch)*8)
#define PROTECT_CH_MSK0_OFS(ch)         (DMA_PROTECT_RANGE0_MSK0_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK1_OFS(ch)         (DMA_PROTECT_RANGE0_MSK1_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK2_OFS(ch)         (DMA_PROTECT_RANGE0_MSK2_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK3_OFS(ch)         (DMA_PROTECT_RANGE0_MSK3_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK4_OFS(ch)         (DMA_PROTECT_RANGE0_MSK4_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK5_OFS(ch)         (DMA_PROTECT_RANGE0_MSK5_REG_OFS+(ch)*32)

static UINT32 chip_id = 0x0;

#define INREG32(x)          			(*((volatile UINT32*)(x)))
#define OUTREG32(x, y)      			(*((volatile UINT32*)(x)) = (y))    ///< Write 32bits IO register
#define SETREG32(x, y)      			OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
#define CLRREG32(x, y)      			OUTREG32((x), INREG32(x) & ~(y))    ///< Clear 32bits IO register

#define LOGO_DBG_MSG 0
#if LOGO_DBG_MSG
#define _Y_LOG(fmt, args...)         	printf(DBG_COLOR_YELLOW fmt DBG_COLOR_END, ##args)
#define _R_LOG(fmt, args...)         	printf(DBG_COLOR_RED fmt DBG_COLOR_END, ##args)
#define _M_LOG(fmt, args...)         	printf(DBG_COLOR_MAGENTA fmt DBG_COLOR_END, ##args)
#define _G_LOG(fmt, args...)         	printf(DBG_COLOR_GREEN fmt DBG_COLOR_END, ##args)
#define _W_LOG(fmt, args...)         	printf(DBG_COLOR_WHITE fmt DBG_COLOR_END, ##args)
#define _X_LOG(fmt, args...)         	printf(DBG_COLOR_HI_GRAY fmt DBG_COLOR_END, ##args)
#else
#define _Y_LOG(fmt, args...)
#define _W_LOG(fmt, args...)
#endif
#define GPIO_LCD_SIF_SEN            	L_GPIO_22//SIF CH1
#define GPIO_LCD_SIF_SCK            	L_GPIO_23
#define GPIO_LCD_SIF_SDA            	L_GPIO_24
extern uint8_t *nvt_fdt_buffer;


#define PLL_CLKSEL_IDE_CLKSRC_480   (0x00)    //< Select IDE clock source as 480 MHz
#define PLL_CLKSEL_IDE_CLKSRC_PLL6  (0x01)    //< Select IDE clock source as PLL6 (for IDE/ETH)
#define PLL_CLKSEL_IDE_CLKSRC_PLL4  (0x02)    //< Select IDE clock source as PLL4 (for SSPLL)
#define PLL_CLKSEL_IDE_CLKSRC_PLL9  (0x03)    //< Select IDE clock source as PLL9 (for IDE/ETH backup)

#define PLL4_OFFSET 0xF0021318
#define PLL6_OFFSET 0xF0021288
#define PLL9_OFFSET 0xF002134c

#define DSI_RSTN_OFFSET 0xF0020088



/**
    Set module clock rate

    Set module clock rate, one module at a time.
    
    @param[in] id      Module ID(PLL_CLKSEL_*), one module at a time.
                          Please refer to pll.h
    @param[in] value    Moudle clock rate(PLL_CLKSEL_*_*), please refer to pll.h

    @return void
*/
void pll_set_ideclock_rate(int id, u32 value)
{
	REGVALUE reg_data;
	UINT32 ui_reg_offset;

	if (id == PLL_CLKSEL_IDE_CLKSRC_480) {
		return;
	} else if (id == PLL_CLKSEL_IDE_CLKSRC_PLL6) {
		ui_reg_offset = PLL6_OFFSET;
	} else if (id == PLL_CLKSEL_IDE_CLKSRC_PLL4) {
		ui_reg_offset = PLL4_OFFSET;
	} else if (id == PLL_CLKSEL_IDE_CLKSRC_PLL9) {
		ui_reg_offset = PLL9_OFFSET;
	} else {
		printf("no support soruce 0x%x.\r\n", id);
	}

	reg_data = ((value / 12000000) * 131072);
	
	OUTREG32(ui_reg_offset, reg_data & 0xFF);
	OUTREG32(ui_reg_offset + 0x4, (reg_data >> 8) & 0xFF);
	OUTREG32(ui_reg_offset + 0x8, (reg_data >> 16) & 0xFF);
}


static int nvt_getfdt_logo_addr_size(ulong addr, ulong *fdt_addr, ulong *fdt_len)
{
	int len;
	int nodeoffset; /* next node offset from libfdt */
	const u32 *nodep; /* property node pointer */

	*fdt_addr = 0;
	*fdt_len = 0;

	nodeoffset = fdt_path_offset((const void *)addr, "/logo");
	if (nodeoffset < 0) {
		return -1;
	} else {
	    nodep = fdt_getprop((const void *)addr, nodeoffset, "enable", &len);
        if((nodep>0)&&(be32_to_cpu(nodep[0]) == 1)){
            nodep = fdt_getprop((const void *)addr, nodeoffset, "lcd_type", &len);
            if(nodep<=0){
                printf("no lcd_type\n");
                return -5;
            }
        } else {
            printf("no ebable\n");
            return -2;
        }
	}

	nodeoffset = fdt_path_offset((const void *)addr, "/nvt_memory_cfg/logo-fb");
	if (nodeoffset < 0) {
        printf("no logo-fb\n");
		return -3;
	}

	nodep = fdt_getprop((const void *)addr, nodeoffset, "reg", &len);
	if (len == 0) {
        printf("no reg\n");
		return -4;
	}

	*fdt_addr = be32_to_cpu(nodep[0]);
	*fdt_len = be32_to_cpu(nodep[1]);
	return 0;
}
static int do_bootlogo(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	PDISP_OBJ       p_emu_disp_obj;
	DISPCTRL_PARAM  emu_disp_ctrl;
	DISPDEV_PARAM   emu_disp_dev;
	DISPLAYER_PARAM emu_disp_lyr;
	ulong logo_addr, logo_size;
	unsigned int img_width, img_height;
	int ret=0;
	int nodeoffset; /* next node offset from libfdt */
	const u32 *nodep; /* property node pointer */
	int len;
	int clock_source = 1;
	int clock_freq = 300000000;
	REGVALUE reg_data;
	UINT32 ui_reg_offset;

	_W_LOG("do_bootlogo\r\n");
	nvt_pinmux_probe();

	reg_data = INREG32(DSI_RSTN_OFFSET);
	reg_data &= 0xFFFFFFF7;
	OUTREG32(DSI_RSTN_OFFSET, reg_data);
	udelay(1000);
	reg_data |= 0x8;
	OUTREG32(DSI_RSTN_OFFSET, reg_data);
	
	p_emu_disp_obj = disp_get_display_object(DISP_1);
	p_emu_disp_obj->open();

	nodeoffset = fdt_path_offset((const void *)nvt_fdt_buffer, "/ide@f0800000");
	if (nodeoffset < 0) {
		printf("no find node ide@f0800000\n");
	} else {
	    nodep = fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "clock-source", &len);
		if (len > 0) {
        	clock_source = be32_to_cpu(nodep[0]);
		} else {
			printf("no ide clock_source info, used default PLL\n");
		}		
		
		nodep = fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "source-frequency", &len);
		if (len > 0) {
			clock_freq  = be32_to_cpu(nodep[0]);
		} else {
        	printf("no ide source-frequency info, used default \n");
		}
		
		//printf("lcd clk src:%d freq:%d : 0x%x : %d\n", clock_source, clock_freq, *nodep, len);
	}
			
	emu_disp_dev.SEL.HOOK_DEVICE_OBJECT.dev_id         = DISPDEV_ID_PANEL;
	emu_disp_dev.SEL.HOOK_DEVICE_OBJECT.p_disp_dev_obj   = dispdev_get_lcd1_dev_obj();
	p_emu_disp_obj->dev_ctrl(DISPDEV_HOOK_DEVICE_OBJECT, &emu_disp_dev);

	pll_set_ideclock_rate(clock_source, clock_freq);
		
	emu_disp_ctrl.SEL.SET_SRCCLK.src_clk = (DISPCTRL_SRCCLK)clock_source;//DISPCTRL_SRCCLK_PLL6;
	p_emu_disp_obj->disp_ctrl(DISPCTRL_SET_SRCCLK, &emu_disp_ctrl);

	////////////////////////////DISP-1//////////////////////////////////////////////////
	emu_disp_dev.SEL.SET_REG_IF.lcd_ctrl     = DISPDEV_LCDCTRL_GPIO;
	emu_disp_dev.SEL.SET_REG_IF.ui_sif_ch     = SIF_CH2;
	emu_disp_dev.SEL.SET_REG_IF.ui_gpio_sen   = L_GPIO(22);
	emu_disp_dev.SEL.SET_REG_IF.ui_gpio_clk   = L_GPIO(23);
	emu_disp_dev.SEL.SET_REG_IF.ui_gpio_data  = L_GPIO(24);
	p_emu_disp_obj->dev_ctrl(DISPDEV_SET_REG_IF, &emu_disp_dev);


	p_emu_disp_obj->dev_ctrl(DISPDEV_CLOSE_DEVICE, NULL);

	emu_disp_dev.SEL.GET_PREDISPSIZE.dev_id = DISPDEV_ID_PANEL;
	p_emu_disp_obj->dev_ctrl(DISPDEV_GET_PREDISPSIZE, &emu_disp_dev);
	_Y_LOG("Pre Get Size =%d, %d\r\n", (int)(emu_disp_dev.SEL.GET_PREDISPSIZE.ui_buf_width), (int)(emu_disp_dev.SEL.GET_PREDISPSIZE.ui_buf_height));

	p_emu_disp_obj->dev_ctrl(DISPDEV_GET_LCDMODE, &emu_disp_dev);
	_Y_LOG("LCD mode =%d\r\n", (int)(emu_disp_dev.SEL.GET_LCDMODE.mode));

	emu_disp_dev.SEL.OPEN_DEVICE.dev_id = DISPDEV_ID_PANEL;
	p_emu_disp_obj->dev_ctrl(DISPDEV_OPEN_DEVICE, &emu_disp_dev);

	p_emu_disp_obj->dev_ctrl(DISPDEV_GET_DISPSIZE, &emu_disp_dev);

	emu_disp_lyr.SEL.SET_WINSIZE.ui_win_width     = emu_disp_dev.SEL.GET_DISPSIZE.ui_buf_width;;
	emu_disp_lyr.SEL.SET_WINSIZE.ui_win_height   	= emu_disp_dev.SEL.GET_DISPSIZE.ui_buf_height;
	emu_disp_lyr.SEL.SET_WINSIZE.i_win_ofs_x      = 0;
	emu_disp_lyr.SEL.SET_WINSIZE.i_win_ofs_y      = 0;
	p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_WINSIZE, &emu_disp_lyr);
	_W_LOG(fmt,args...)("DISPLAYER_OP_SET_WINSIZE %d %d\r\n",emu_disp_lyr.SEL.SET_WINSIZE.ui_win_width,emu_disp_lyr.SEL.SET_WINSIZE.ui_win_height);

	ret=nvt_getfdt_logo_addr_size((ulong)nvt_fdt_buffer, &logo_addr, &logo_size);
	if(ret!=0) {
		printf("err:%d\r\n",ret);
		return -1;
	}
	_Y_LOG("logo_addr %x logo_size %x\r\n",logo_addr,logo_size);

	{
		_Y_LOG("start JPEG decode size: %x\n",sizeof(inbuf));
		jpeg_setfmt(1);
		jpeg_decode(inbuf, (unsigned char*)logo_addr);
		jpeg_getdim(&img_width, &img_height);
		_Y_LOG("image size: %d x %d\n", img_width, img_height);
		if(logo_size < img_width*img_height*2) {
			printf("(%d,%d) size small 0x%x\r\n",img_width,img_height,logo_size);
			return -1;
		}
		flush_dcache_range((unsigned long)logo_addr,(unsigned long)(logo_addr+logo_size));
	}

	{
		UINT32 buf_w = img_width;
		UINT32 buf_h = img_height;
		UINT32 uiVDO_YAddr = logo_addr;
		UINT32 VDO_BUF_SIZE = buf_w*buf_h;
		UINT32	uiVDO_UVAddr = uiVDO_YAddr+VDO_BUF_SIZE;
		_W_LOG("show logo %x %x\r\n",uiVDO_YAddr,uiVDO_UVAddr);

		emu_disp_lyr.SEL.SET_MODE.buf_format   = DISPBUFFORMAT_YUV422PACK;
		emu_disp_lyr.SEL.SET_MODE.buf_mode     = DISPBUFMODE_BUFFER_REPEAT;
		emu_disp_lyr.SEL.SET_MODE.buf_number   = DISPBUFNUM_3;
		p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_MODE, &emu_disp_lyr);
		_W_LOG("DISPLAYER_OP_SET_MODE\r\n");

		emu_disp_lyr.SEL.SET_BUFSIZE.ui_buf_width   =  buf_w;
		emu_disp_lyr.SEL.SET_BUFSIZE.ui_buf_height  =  buf_h;
		emu_disp_lyr.SEL.SET_BUFSIZE.ui_buf_line_ofs = buf_w;
		p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_BUFSIZE, &emu_disp_lyr);
		_W_LOG("DISPLAYER_OP_SET_BUFSIZE %d %d\r\n",buf_w,buf_h);

		#if 0 //fill YUV color for test
		memset((void *)uiVDO_YAddr, 0x4C, VDO_BUF_SIZE/2);
		memset((void *)uiVDO_YAddr+VDO_BUF_SIZE/2, 0x67, VDO_BUF_SIZE/2);
		memset((void *)uiVDO_UVAddr, 0x4C, VDO_BUF_SIZE/2);
		memset((void *)uiVDO_UVAddr+VDO_BUF_SIZE/2, 0xAD, VDO_BUF_SIZE/2);
		#endif
		memset((void *)&emu_disp_lyr,0,sizeof(DISPLAYER_PARAM));
		emu_disp_lyr.SEL.SET_VDOBUFADDR.buf_sel = DISPBUFADR_0;
		emu_disp_lyr.SEL.SET_VDOBUFADDR.ui_addr_y0 = uiVDO_YAddr;
		emu_disp_lyr.SEL.SET_VDOBUFADDR.ui_addr_cb0 = uiVDO_UVAddr;
		emu_disp_lyr.SEL.SET_VDOBUFADDR.ui_addr_cr0 = uiVDO_UVAddr;
		p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_VDO1,DISPLAYER_OP_SET_VDOBUFADDR,&emu_disp_lyr);
		_W_LOG("DISPLAYER_OP_SET_VDOBUFADDR\r\n");

		emu_disp_ctrl.SEL.SET_ALL_LYR_EN.b_en      = TRUE;
		emu_disp_ctrl.SEL.SET_ALL_LYR_EN.disp_lyr  = DISPLAYER_VDO1;
		p_emu_disp_obj->disp_ctrl(DISPCTRL_SET_ALL_LYR_EN, &emu_disp_ctrl);
		p_emu_disp_obj->load(TRUE);
		p_emu_disp_obj->wait_frm_end(TRUE);

		_W_LOG("DISPCTRL_SET_ALL_LYR_EN\r\n");

	}

	return 0;
}



U_BOOT_CMD(
	bootlogo,	2,	1,	do_bootlogo,
	"show lcd bootlogo",
	"no argument means LCD output only, \n"
);
