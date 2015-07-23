#define LOG_TAG "HWLIT5000"
#if !defined(NO_DEBUG) && !defined(HWLAYER_SKEL_DEBUG_ENABLED)
#warning "HWLAYER_SKEL_DEBUG_ENABLED NOT DEFIND, therefore no HWLayerIT5000 debug"
#define NO_DEBUG	1
#endif

#include "CommonInclude.h"
#include "HWLayerIT5000.h"
#include <hsm_imager.h>
#include "i2c.h"

// Local functions
static int it5000_sensor_read_reg(unsigned int reg_address, unsigned short *pbuf, unsigned int nCount);
static int it5000_sensor_write_reg(unsigned int reg_address, const unsigned short *pbuf, unsigned int nCount);


HwlIT5000 :: HwlIT5000()
{
	DBG_FUNC();
}

HwlIT5000 :: ~HwlIT5000()
{
	DBG_FUNC();
}

bool HwlIT5000 :: WriteReg(unsigned subaddress, const unsigned short * uiBuffer, size_t nCount )
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER
	return true;
	#endif
	int err = it5000_sensor_write_reg(subaddress, uiBuffer, nCount);
	return (err < 0) ? false : true;
}

bool HwlIT5000 :: ReadReg(unsigned subaddress, unsigned short * uiBuffer, size_t nCount)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER
	return true;
	#endif

	int err = it5000_sensor_read_reg(subaddress, uiBuffer, nCount);
	return (err < 0) ? false : true;
}

/*
  Gen5 PSOC needs special care:
	I2C normally works this way:
		Write:	START,addr,W,reg,data,STOP
		Read:	START,addr,W,reg, START,addr,R,data,STOP
	Gen5 PSOC:
		Write:	START,addr,W,reg,STOP, START,addr,W,data,STOP
		Read:	START,addr,W,reg,STOP, START,addr,R,data,STOP
	The STOP condition after the first part is required by the PSOC.

	For write I had implemented in KIL since first days.
	Read seemed to work in the standard way. But that was a wrong assumption.
	If we read from PSOC in std. I2C notation, then a following reset command is ignored.
	With the PSOC notation from above it works just fine.

	So far I did not see any non-std. behavior with Gen6.
	The PSOC in Gen6 uses another I2C library as can be read in the specification.
*/

bool HwlIT5000 :: WriteIIC_PSOC(unsigned char address, const unsigned char * p, unsigned cnt )
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER
	return true;
	#endif
	
	int err = i2c_transfer(PSOC_I2C_ADDR, address, const_cast<unsigned char *>(p), cnt, 0);
	return (err < 0) ? false : true;
}

bool HwlIT5000 :: ReadIIC_PSOC(unsigned char address, unsigned char *p, unsigned int cnt)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER
	return true;
	#endif

	int err = i2c_transfer(PSOC_I2C_ADDR, address, p, cnt, I2C_M_RD);
	return (err < 0) ? false : true;
}

int it5000_sensor_write_reg(unsigned int reg_address, const unsigned short *pbuf, unsigned int nCount)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER
	return true;
	#endif
	unsigned short buf_tmp[MAX_TMP_BUF_SIZE];
	unsigned int bytes;
	unsigned char *pchar = (unsigned char *) buf_tmp;

	if (!pbuf || !nCount)
		return -EINVAL;

	bytes = nCount * sizeof(unsigned short);

	if (bytes > MAX_TMP_BUF_SIZE)
		return -ENOMEM;

	/* re-organize data */
	for (int i = 0; i < nCount; i++)
	{
		if (target_cpu_is_lsb())
		{
			*pchar = (pbuf[i] >> 8);
			pchar++;
			*pchar = (pbuf[i] & 0x00FF);
			pchar++;
		}
		else
		{
			memcpy(pchar, &pbuf[i], sizeof(unsigned short));
			pchar++;
		}
	}
	return i2c_write_reg(MT9V022_I2C_ADDR, reg_address, (unsigned char *) buf_tmp, bytes);
}

int it5000_sensor_read_reg(unsigned int reg_address, unsigned short *pbuf, unsigned int nCount)
{
	DBG_FUNC();
	#ifdef CAMERA_FOR_SCANNER
	return true;
	#endif
	int err = 0;
	unsigned int bytes;
	unsigned short buf_tmp[MAX_TMP_BUF_SIZE];
	unsigned char *pchar = (unsigned char *) buf_tmp;

	if (!pbuf || !nCount)
		return -EINVAL;

	bytes = nCount * sizeof(unsigned short);

	if (bytes > MAX_TMP_BUF_SIZE)
		return -ENOMEM;

	err = i2c_read_reg(MT9V022_I2C_ADDR, reg_address, (unsigned char *) buf_tmp, bytes);
	if (err == 0)
	{
		/* re-organize data */
		for (int i = 0; i < nCount; i++)
		{
			if (target_cpu_is_lsb())
			{
				pbuf[i] = ((pchar[0] << 8) | pchar[1]);
			}
			else
			{
				memcpy(&pbuf[i], pchar, sizeof(unsigned short));
			}
			pchar += 2;
		}
	}
	return err;
}

