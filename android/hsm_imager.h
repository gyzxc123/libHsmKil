/*
 * Camera driver for barcode imagers from Honeywell.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _HSM_IMAGER_H
#define _HSM_IMAGER_H

struct hsm_iic_data {
	unsigned char i2c_addr;
	unsigned char reg;
	unsigned char *buf;
	unsigned char len;
};

struct hsm_gpio_config {
	unsigned char pin;
	unsigned char dir;
	unsigned char def_value;
};

struct hsm_gpio_data {
	unsigned char pin;
	unsigned char value;
};

enum hsm_gpio_id {
	HSM_GPIO_POWER_ENABLE = 0,
	HSM_GPIO_AIMER,
	HSM_GPIO_ILLUMINATOR,
	HSM_GPIO_ENGINE_RESET,
	HSM_GPIO_VSYNC_GPIO,
};
#define HSM_NUM_GPIO 5

enum hsm_mount_options {
	HSM_MOUNT_RIGHT_SIDE_UP = 0,
	HSM_MOUNT_RIGHT,
	HSM_MOUNT_UPSIDE_DOWN,		/* This is what most devices use. Ears
					 * up, causing the image upside down. */
	HSM_MOUNT_LEFT,
};

#define HSM_PLATFORM_BUFFER_ALLIGN	0x0001
#define HSM_PLATFORM_PROBE_AFTER_S_INPUT	0x1000
#define HSM_PLATFORM_SUPPORTS_SUPPLY	0x4000
#define HSM_PLATFORM_HAS_STATUS	0x8000
#define HSM_PLATFORM_SET_SKIP_IMAGES(n)	(n<<8)
#define HSM_PLATFORM_GET_SKIP_IMAGES(n)	((n>>8)&0xF)

#define HSM_PROPERTIES_V1 1
struct hsm_engine_properties {
	int version;
	int imager;
	int width;
	int height;
	enum hsm_mount_options mount;
	u8 i2c_addr_sensor;		//!< Address of camera sensor in 7bit format
	u8 i2c_addr_psoc;		//!< Address of PSOC in 7bit format
	u8 i2c_addr_clock;		//!< Address of clock chip in 7bit format
	int spreadspectrum;		//!< Currently not used
	int platform_details;		//!< Workaround hints for Platforms
	int Reserved[3];
};

#define HSM_STATUS_NEED_REINIT 0x1
#define HSM_STATUS_V1 1
struct hsm_status_data {
	int version;
	int status;
	int Reserved;
};

#ifdef __KERNEL__
#define HSM_PLATFORM_VERSION 1
struct hsm_gpio {
	char * name;
	resource_size_t port;
	bool init;
	bool inverted;
};

struct hsm_platform_data {
	unsigned int pixelformat;
	enum hsm_mount_options mount;
	int spreadspectrum;
	int (*hsm_supply_en)(int en); /* This function should enable the VCC on sensor */
	struct hsm_gpio gpio[HSM_NUM_GPIO];
};
#endif

/*
 * Note these are 7 bit I2C device addresses.
 * The R/W bit is not included and the actual address is in D6..D0.  D7 = 0.
 */
#define PSOC_I2C_ADDR	 	0x40	//!< System control in engine
#define MT9V022_I2C_ADDR 	0x48	//!< Image sensor IT5000 (gen5)
#define EV76C454_I2C_ADDR 	0x18	//!< Image sensor N5600 (gen6)
#define IDT6P50016_I2C_ADDR	0x69	//!< spread spectrum clock chip

#define N5600_WIDTH	832
#define N5600_HEIGHT	640

#define IT5000_WIDTH	752
#define IT5000_HEIGHT	480

#ifndef __KERNEL__
/* This can make integration into user space a little easier. */
#ifndef BASE_VIDIOC_PRIVATE
#define BASE_VIDIOC_PRIVATE		192
#warning "BASE_VIDIOC_PRIVATE not defined. I assumed a value taken from V3.0 kernel."
#endif
#endif

#if 0
#define HSM_GET_PROPERTIES _IOWR('V', BASE_VIDIOC_PRIVATE + 0, struct hsm_engine_properties)
#define HSM_IIC_WRITE      _IOWR('V', BASE_VIDIOC_PRIVATE + 2, struct hsm_iic_data)
#define HSM_IIC_READ       _IOWR('V', BASE_VIDIOC_PRIVATE + 3, struct hsm_iic_data)
/* #define HSM_GPIO_CONFIG    _IOWR('V', BASE_VIDIOC_PRIVATE + 6, struct hsm_gpio_config) */
#define HSM_GPIO_WRITE     _IOWR('V', BASE_VIDIOC_PRIVATE + 7, struct hsm_gpio_data)
#define HSM_GPIO_READ      _IOWR('V', BASE_VIDIOC_PRIVATE + 8, struct hsm_gpio_data)
#define HSM_IIC_TRANSFER   _IOWR('V', BASE_VIDIOC_PRIVATE + 9, struct i2c_rdwr_ioctl_data)
#define HSM_STATUS_READ    _IOWR('V', BASE_VIDIOC_PRIVATE + 10, struct hsm_status_data)
#define HSM_SUPPLY_WRITE   _IOWR('V', BASE_VIDIOC_PRIVATE + 11, struct hsm_gpio_data)
#define HSM_SUPPLY_READ    _IOWR('V', BASE_VIDIOC_PRIVATE + 12, struct hsm_gpio_data)
#else
	enum scanner_command {
		HSM_GET_PROPERTIES = 0x7000,
		HSM_IIC_WRITE,
		HSM_IIC_READ,
		HSM_GPIO_WRITE,
		HSM_GPIO_READ,
		HSM_IIC_TRANSFER,
		HSM_STATUS_READ,
		HSM_SUPPLY_WRITE,
		HSM_SUPPLY_READ,
	};
#endif

#endif /* _HSM_IMAGER_H */

