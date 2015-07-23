#define LOG_TAG "GPIO"
#if !defined(NO_DEBUG) && !defined(GPIO_DEBUG_ENABLED)
#warning "GPIO_DEBUG_ENABLED NOT DEFIND, therefore no gpio debug"
#define NO_DEBUG	1
#endif

#include "gpio.h"
#include "camera_device.h"

#ifndef NO_DEBUG
const char *dbg_gpio_name(unsigned int gpio)
{
	const char	*GPIOName[] = {"PwrEn", "Aimer", "Illum", "Reset", "VSync"};

	return (gpio < ARRAY_SIZE(GPIOName))? GPIOName[gpio] : "Unknown";
}
#endif

///////////////////////////////////////////////////////////////////////////////
//! Uses an ioctl call to the kernel driver to issue an GPIO write.
/*! 
 \param gpio 
 \param value 
*/
void gpio_set_value(unsigned int gpio, int value)
{
	DBG_FUNC();
	struct hsm_gpio_data data;
	data.pin = gpio;
	data.value = value;
	int ret = camera_ioctl(HSM_GPIO_WRITE, &data);
	if (ret < 0)
	{
		DBG_OUT1("GpioSetValue %s(%i)=%i error (%d) %s\n", dbg_gpio_name(gpio), gpio, value, errno, strerror (errno));
	}
	else
	{
		DBG_OUT1("GpioSetValue %s(%i)=%i, ioctl=%u (%X)\n", dbg_gpio_name(gpio), gpio, value, HSM_GPIO_WRITE, HSM_GPIO_WRITE);
#ifndef NO_DEBUG
		gpio_get_value(gpio);
#endif
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Uses an ioctl call to the kernel driver to issue an GPIO read.
/*! 
 \param gpio 
 \return port value 
*/
int gpio_get_value(unsigned int gpio)
{
	DBG_FUNC();
	struct hsm_gpio_data data;
	data.pin = gpio;
	data.value = -1;
	int ret = camera_ioctl(HSM_GPIO_READ, &data);
	if (ret < 0)
	{
		DBG_OUT1("GpioGetValue %s(%i) error (%d) %s\n", dbg_gpio_name(gpio), gpio, errno, strerror (errno));
	}
	else
	{
		DBG_OUT1("GpioGetValue %s(%i):%i, ioctl=%u (%X)\n", dbg_gpio_name(gpio), gpio, data.value, HSM_GPIO_READ, HSM_GPIO_READ);
	}
	return data.value;
}

///////////////////////////////////////////////////////////////////////////////
//! Uses an ioctl call to the kernel driver to supply power.
/*! 
*/
bool gpio_power_on(void)
{
	DBG_FUNC();
	struct hsm_gpio_data data;
	data.pin = 0; // doesn't matter
	data.value = 1; // enable
	int ret = camera_ioctl(HSM_SUPPLY_WRITE, &data);
	if (ret < 0)
	{
		DBG_OUT1("camera_power_on error (%d) %s\n",  errno, strerror (errno));
		return false;
	}
	else
	{
		//DBG_OUT1("camera_power_on success! ioctl=%u (%X)\n",  HSM_GPIO_WRITE, HSM_GPIO_WRITE);
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
//! Uses an ioctl call to the kernel driver to remove power.
/*! 
*/
bool gpio_power_off(void)
{
	DBG_FUNC();
	struct hsm_gpio_data data;
	data.pin = 0; // doesn't matter
	data.value = 0; // disable
	int ret = camera_ioctl(HSM_SUPPLY_WRITE, &data);
	if (ret < 0)
	{
		DBG_OUT1("camera_power_off error (%d) %s\n",  errno, strerror (errno));
		return false;
	}
	else
	{
		//DBG_OUT1("camera_power_off success! ioctl=%u (%X)\n",  HSM_GPIO_WRITE, HSM_GPIO_WRITE);
		return true;
	}
}
