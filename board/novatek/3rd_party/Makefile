KBUILD_CFLAGS := -fno-builtin -Werror -Iboard/$(BOARDDIR)/../common/include

ifeq ($(CONFIG_NVT_RTK_BT_FWLOAD), y)
KBUILD_CFLAGS += -Iboard/$(BOARDDIR)/../3rd_party/rtk_bt/include -Iboard/$(BOARDDIR)/../3rd_party/rtk_bt/osi/include
endif


obj-$(CONFIG_NVT_RTK_BT_FWLOAD) += rtk_bt/osi/src/bt_list.o rtk_bt/osi/src/bt_skbuff.o rtk_bt/osi/src/osi_function.o \
	rtk_bt/osi/src/osi_usb.o rtk_bt/osi/src/osi_userial.o rtk_bt/src/boot_usb/usb_rtk_bt.o \
	rtk_bt/src/boot_usb/hardware_usb.o
