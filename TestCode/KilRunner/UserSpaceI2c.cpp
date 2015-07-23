   
#include "commoninclude.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/i2c-dev.h>
#include "UserSpaceI2c.h"

// construction zone ahead !!!
// Note: I started this to find that the actual device does not have the user space I2C enabled in kernel config.
// We also need control over the GPIO user space interface (and the Regulator for the VCC). These are also not available...

// Just wanted to hack something quick that could issues the reset command from PSOC to Sensor.
// Keep it here since we will need that in future.

/* wrap for kernel API */
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#include "hsm_imager.h"	// kernel declarations for imager

// until we find a better place
#define I2C_DEVICE "/dev/i2c-0"

CUserSpaceI2c::CUserSpaceI2c()
: m_fd(-1)
{

}

CUserSpaceI2c::~CUserSpaceI2c()
{

}

bool CUserSpaceI2c::Init()
{
	bool bStatus=false;

	m_fd = open(I2C_DEVICE,O_RDWR);
	if(m_fd<=0)
	{
		DBG_OUT("I2C open failed!");
	}
	else
	{
		int res = ioctl(m_fd, I2C_SLAVE, PSOC_I2C_ADDR);
		if(res<0)
		{
			printf("I2C I2C_ADDRESS failed (%d) %s", errno, strerror (errno));
		}
		else
		{
			bStatus = true;
		}
	}
	return bStatus;
}

/**
 * Write bytes to the image engine PSOC (microcontroller) over I2C bus.
 * After sending a start bit, this function should send the slave address
 * of the PSOC (typically 0x80), followed by cnt bytes of data from buffer.
 *
 * @param buffer  pointer to buffer of bytes of raw values to write via I2C.
 * @param cnt  number of bytes to send from buffer to PSOC.
 * @return TRUE if I2C transaction was successful.
 */
bool  CUserSpaceI2c::WriteIIC_PSOC(const unsigned char *buffer, unsigned char cnt)
{
	if(m_fd<=0)
	{
		printf("CUserSpaceI2c::WriteIIC_PSOC failed, not initialized!\n");
		return false;
	}

	ssize_t nwrite = 0;
	nwrite = write(m_fd, buffer, cnt);
	if(nwrite<=0)
	{
		printf("errno=%i\n", errno);
	}
	return (nwrite == cnt);
}

/**
 * Read bytes from the image engine PSOC (microcontroller). This function
 * will perform an I2C read transaction of cnt bytes, storing each byte in
 * the buffer. This function does not need to perform any write transaction.
 * The Scan Driver will invoke all necessary write transactions
 * with WriteIIC_PSOC_MT9V022.
 *
 * @param  buffer  points to buffer where cnt bytes of read data will be stored
 * @param  cnt   number of bytes to receive and store in buffer before sending NAK
 * @return  TRUE if I2C transaction was successful and cnt bytes read, otherwise FALSE.
 */
bool  CUserSpaceI2c::ReadIIC_PSOC(unsigned char *buffer, unsigned char cnt)
{
//	int res = i2c_smbus_read_i2c_block_data(m_fd, __u8 command, cnt, buffer);
	if(m_fd<=0)
	{
		printf("CUserSpaceI2c::ReadIIC_PSOC failed, not initialized\n");
		return false;
	}

	ssize_t nread = 0;
	nread = read(m_fd, buffer, cnt);
	return (nread == cnt);
}
#if 0
// Below is not working! gen5 and gen6 need different code (see KIL).
bool CUserSpaceI2c::ResetSensor()
{
	// Issue reset, wait 100ms, then reload all IIC registers.
	unsigned char data = 0;
	bool RetVal = WriteIIC_PSOC(0x82, &data, 1);
	usleep(100 * 1000);
	return RetVal;
}
#endif
