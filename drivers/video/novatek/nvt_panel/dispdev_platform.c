#include "./include/dispdev_platform.h"

#if defined __UITRON || defined __ECOS
#elif defined __FREERTOS
unsigned int dispdev_debug_level = NVT_DBG_WRN;
#else
#endif

PINMUX_LCDINIT dispdev_platform_get_disp_mode(UINT32 pin_func_id)
{
#if defined __UITRON || defined __ECOS
	return pinmux_getDispMode((PINMUX_FUNC_ID)pin_func_id);
#else
	return pinmux_get_dispmode((PINMUX_FUNC_ID) pin_func_id);
#endif
}

void dispdev_platform_set_pinmux(UINT32 pin_func_id, UINT32 pinmux)
{
#if defined __UITRON || defined __ECOS
	pinmux_setPinmux(pin_func_id, PINMUX_LCD_SEL_GPIO);
#else
	pinmux_set_config((PINMUX_FUNC_ID) pin_func_id, pinmux);
#endif
}

void dispdev_platform_delay_ms(UINT32 ms)
{
	udelay(1000*ms);
}

void dispdev_platform_delay_us(UINT32 us)
{
	//ndelay(1000 * us);
	udelay(us);
}


UINT32 dispdev_platform_request_gpio(UINT32 id, CHAR *str)
{
	return gpio_request(id, str);
}

void dispdev_platform_set_gpio_ouput(UINT32 id, BOOL high)
{
	gpio_direction_output(id, high);
}

void dispdev_platform_set_gpio_input(UINT32 id)
{
	gpio_direction_input(id);
}

void dispdev_platform_free_gpio(UINT32 id)
{
	gpio_free(id);
}

/*
PDISPDEV_OBJ dispdev_get_lcd1_dev_obj(void)
{
#if defined DISPLCDSEL_IF8BITS_TYPE
#if (DISPLCDSEL_IF8BITS_TYPE == DISPLCDSEL_IF8BITS_LCD1)
	return dispdev_get_lcd1_if8bits_dev_obj();
#endif

#elif defined DISPLCDSEL_IFDSI_TYPE
#if (DISPLCDSEL_IFDSI_TYPE == DISPLCDSEL_IFDSI_LCD1)
	return dispdev_get_lcd1_ifdsi_dev_obj();
#endif

#elif defined DDISPLCDSEL_IFPARAL_LCD1
#if (DISPLCDSEL_IFPARAL_TYPE == DISPLCDSEL_IFPARAL_LCD1)
#endif

#endif
}

EXPORT_SYMBOL(dispdev_get_lcd1_dev_obj);
*/
