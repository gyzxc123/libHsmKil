#define LOG_TAG "I2C"
#if !defined(NO_DEBUG) && !defined(I2C_DEBUG_ENABLED)
#warning "I2C_DEBUG_ENABLED NOT DEFINED, therefore no i2c debug"
#define NO_DEBUG	1
#endif

#include <unistd.h>
#include "CommonInclude.h"
#include "i2c.h"
#include "camera_device.h"

/*
 * I2C Message - used for pure i2c transaction, also from /dev interface
 */
struct i2c_msg {
	unsigned short addr;	/* slave address			*/
	unsigned short flags;		
	short len;		/* msg length				*/
	char *buf;		/* pointer to msg data			*/
};

/* This is the structure as used in the I2C_RDWR ioctl call */
struct i2c_rdwr_ioctl_data {
	struct i2c_msg *msgs;	/* pointers to i2c_msgs */
	int nmsgs;		/* number of i2c_msgs */
};


///////////////////////////////////////////////////////////////////////////////
//! Finds the endian of the used CPU.
/*!  
 \return 1 for little endian, else 0 
*/
int target_cpu_is_lsb()
{
//	DBG_FUNC();
	unsigned int x = 1;
	unsigned char *p = (unsigned char *)&x;
	if (*p == 1) {
		return 1;
	} else {
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Uses an ioctl call to the kernel driver to issue an I2C write.
/*! 
 \param i2c_addr 
 \param reg 
 \param buf 
 \param len 
 \return 0 on success 
*/
int i2c_write_reg(u8 i2c_addr, u8 reg, const u8 *buf, u8 len)
{
	DBG_FUNC();
	long ret;
	struct hsm_iic_data data;

	if (!buf || !len)
		return -EINVAL;

	DBG_I2C(true, i2c_addr, reg, buf, len);

	data.i2c_addr = i2c_addr;
	data.reg = reg;
	data.buf = const_cast<unsigned char *>(buf);	// casting away the consts is reall poor style, we need to fixme:
	data.len = len;
	ret = camera_ioctl(HSM_IIC_WRITE, &data);

	if( ret )
	{
		KIL_ERR("Error WRITE_IIC return = %d, err=%d\n", ret, errno);
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
//! Uses an ioctl call to the kernel driver to issue an I2C read.
/*! 
 \param i2c_addr 
 \param reg 
 \param buf 
 \param len 
 \return 0 on success 
*/
int i2c_read_reg(u8 i2c_addr, u8 reg, u8 *buf, u8 len)
{
	DBG_FUNC();
	long ret;
	struct hsm_iic_data data;

	if (!buf || !len)
		return -EINVAL;

	data.i2c_addr = i2c_addr;
	data.reg = reg;
	data.len = len;
	data.buf = buf;
	ret = camera_ioctl(HSM_IIC_READ, &data);

	DBG_I2C(false, i2c_addr, reg, buf, len);

	if( ret )
	{
		KIL_ERR("Error READ_IIC return = %d, err=%d\n", ret, errno);
	}

	return ret;
}

// A helper that sends the I2C transfer data to the kernel driver.
///////////////////////////////////////////////////////////////////////////////
//! Uses an ioctl call to the kernel driver to issue an I2C transfer.
/*!
 \param i2c_msg
 \return zero on success, else negative errno
*/
int i2c_transfer(u8 i2c_addr, u8 reg, u8 *buf, u8 len, unsigned short flags)
{
	DBG_FUNC();

	#ifdef CAMERA_FOR_SCANNER
	return 0;
	#endif

	struct i2c_msg msg[2];
	msg[0].addr = i2c_addr;
	msg[0].flags = 0;
	msg[0].len = sizeof(reg);
	msg[0].buf = (char *)&reg;

	msg[1].addr = i2c_addr;
	msg[1].flags = flags;
	msg[1].len = len;
	msg[1].buf = (char *)buf;

	i2c_rdwr_ioctl_data data;
	data.msgs = &msg[0];
	data.nmsgs = 1;
	int ret = camera_ioctl(HSM_IIC_TRANSFER, &data);
	KIL_ERR("camera_ioctl(HSM_IIC_TRANSFER, &data):%d\n",ret);
	//if( ret == 1 )//modify by zhaozuzhao 2014.04.29
	{
		usleep(5000);
		data.msgs = &msg[1];
		data.nmsgs = 1;
		ret = camera_ioctl(HSM_IIC_TRANSFER, &data);
		ret = (ret==1) ? 0 : ret;
	}

	DBG_I2C(!(flags & I2C_M_RD), i2c_addr, reg, buf, len);

	if ( ret )
	{
		KIL_ERR("Error I2C transfer, Reg=%02X, ret=%d, (%d) %s\n", (unsigned int)reg, ret, errno, strerror(errno));
	}

	return ret;
}

