#ifndef I2C_H_HWLAYER
#define I2C_H_HWLAYER

#define MAX_TMP_BUF_SIZE	(256)
#define I2C_M_RD		0x0001	/* flags: read data, from slave to master */

int target_cpu_is_lsb();
int i2c_write_reg(u8 i2c_addr, u8 reg, const u8 *buf, u8 len);
int i2c_read_reg(u8 i2c_addr, u8 reg, u8 *buf, u8 len);
int i2c_transfer(u8 i2c_addr, u8 reg, u8 *buf, u8 len, unsigned short flags);

#endif /*I2C_H_HWLAYER*/