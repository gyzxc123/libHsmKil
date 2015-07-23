#define LOG_TAG "HWLN5600"
#if !defined(NO_DEBUG) && !defined(HWLAYER_SKEL_DEBUG_ENABLED)
#warning "HWLAYER_SKEL_DEBUG_ENABLED NOT DEFIND, therefore no HWLayerN5600 debug"
#define NO_DEBUG	1
#endif

#include "CommonInclude.h"
#include "HWLayerN5600.h"
#include <hsm_imager.h>
#include "i2c.h"

// Local functions
static int n56xx_sensor_read_reg(u8 reg, u16 *pbuf, u32 len);
static int n56xx_sensor_write_reg(u8 reg, const u16 *pbuf, u32 len);
static int config_gen6_clock(void);


HwlN5600 :: HwlN5600()
{
	DBG_FUNC();
}

HwlN5600 :: ~HwlN5600()
{
	DBG_FUNC();
}

bool HwlN5600 :: ImagerPowerUp(void)
{
	IHWL_BASE_CLASS::ImagerPowerUp();
		
	// Configure clock after engine (psoc) is stable
	int ret = config_gen6_clock();
	DBG_OUT1("config clock ret = %d\n", ret);

	return true;
}

bool HwlN5600 :: WriteReg(unsigned subaddress, const unsigned short * uiBuffer, size_t nCount )
{
	DBG_FUNC();
	int err = n56xx_sensor_write_reg(subaddress, uiBuffer, nCount);
	return (err < 0) ? false : true;
}

bool HwlN5600 :: ReadReg(unsigned subaddress, unsigned short * uiBuffer, size_t nCount)
{
	DBG_FUNC();

	int err = n56xx_sensor_read_reg(subaddress, uiBuffer, nCount);
	return (err < 0) ? false : true;
}

int n56xx_sensor_read_reg(u8 reg, u16 *pbuf, u32 len)
{
//	DBG_FUNC();
	int err = 0;
	u32 bytes;
	u32 i;
	u32 reg_num_8bit = 0;
	u32 reg_num_16bit = 0;
	u16 buf_tmp[MAX_TMP_BUF_SIZE];
	u8 *pchar = (u8 *)buf_tmp;

	if (!pbuf || !len)
		return -EINVAL;

	if (reg + len < 0x80) {
		reg_num_8bit = len;
	} else {
		if (reg >= 0x80) {
			reg_num_16bit = len;
		} else {
			reg_num_16bit = reg + len - 0x80;
			reg_num_8bit = len - reg_num_16bit;
		}
	}

	bytes = reg_num_8bit + reg_num_16bit * sizeof(u16);

	if (bytes > MAX_TMP_BUF_SIZE)
		return -ENOMEM;

	err = i2c_read_reg(EV76C454_I2C_ADDR, reg, (u8 *)buf_tmp, bytes);
	if (err < 0)
		goto err_read_reg;

	/* re-organize data */
	for (i = 0; i < len; i++) {
		if (reg + i < 0x80) {
			pbuf[i] = *pchar;
			pchar++;
		} else {
			if (target_cpu_is_lsb()) {
				pbuf[i] = ((pchar[0] << 8) | pchar[1]);
			} else {
				memcpy(&pbuf[i], pchar, sizeof(u16));
			}
			pchar += 2;
		}
	}
err_read_reg:
	return err;
}

int n56xx_sensor_write_reg(u8 reg, const u16 *pbuf, u32 len)
{
//	DBG_FUNC();
	u16 buf_tmp[MAX_TMP_BUF_SIZE];
	u32 bytes;
	u32 i;
	u32 reg_num_8bit = 0;
	u32 reg_num_16bit = 0;
	u8 *pchar = (u8 *)buf_tmp;

	if (!pbuf || !len)
		return -EINVAL;

	if (reg + len < 0x80) {
		reg_num_8bit = len;
	} else {
		if (reg >= 0x80) {
			reg_num_16bit = len;
		} else {
			reg_num_16bit = reg + len - 0x80;
			reg_num_8bit = len - reg_num_16bit;
		}
	}

	bytes = reg_num_8bit + reg_num_16bit * sizeof(u16);

	if (bytes > MAX_TMP_BUF_SIZE)
		return -ENOMEM;

	/* re-organize data */
	for (i = 0; i < len; i++) {
		if (reg + i < 0x80) {
			*pchar = (pbuf[i] & 0x00FF);
			pchar++;
		} else {
			if (target_cpu_is_lsb()) {
				*pchar = (pbuf[i] >> 8);
				pchar++;
				*pchar = (pbuf[i] & 0x00FF);
				pchar++;
			} else {
				memcpy(pchar, &pbuf[i], sizeof(u16));
				pchar++;
			}
		}
	}

	return i2c_write_reg(EV76C454_I2C_ADDR, reg, (u8 *)buf_tmp, bytes);
}


///////////////////////////////////////////////////////////////////////////////
//! Configure the clock controller (spread spectrum).
/*! Currently we only support one clock chip with n5600.
	Gen5 has no clock chip.
 \return 0 on success 
*/
int config_gen6_clock()
{
	DBG_FUNC();
	int err = 0;
	u8 conf[3];
	u8 clock_conf = 0x82;
	u8 clock_conf_old;

	// Note: count not needed on read, but ignore the first
	// byte returned.
	err = i2c_read_reg(IDT6P50016_I2C_ADDR, 0x0, conf, sizeof(conf));
	if( err < 0 )
	{
		DBG_OUT1("read clock config failed with %d\n", err);
		goto err_read_reg;
	}
	clock_conf_old = conf[2];

	// For write, must include count of bytes as 1st arg.
	// On the i2c wire: <i2c addr|[R|W]> <reg> <cnt> <val>
	// Example of transmission:  D2 01 01 82
	// with ClockConfig value of 0x82.
	conf[0] = 0x1;
	conf[1] = (clock_conf & 0xff);
	err = i2c_write_reg(IDT6P50016_I2C_ADDR, 0x01, conf, 2);
	if( err < 0 ) 
	{
		DBG_OUT1("write clock config failed with %d\n", err);
		goto err_write_reg;
	}

	// Again, read 3 bytes and ignore 1st byte read.
	err = i2c_read_reg(IDT6P50016_I2C_ADDR, 0x0, conf, sizeof(conf));
	if( err < 0 ) 
	{
		DBG_OUT1("read clock config failed with %d\n", err);
		goto err_read_reg;
	}
	DBG_OUT1("Clock (VID=0x%01x, RID=0x%01x) config changed from 0x%02x to 0x%02x\n",
		conf[1] & 0xf, (conf[1] >> 4), clock_conf_old, conf[2]);

err_write_reg:
err_read_reg:
	return err;
}
