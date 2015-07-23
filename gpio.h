#ifndef GPIO_H_HWLAYER
#define GPIO_H_HWLAYER

void gpio_set_value(unsigned int gpio, int value);
int gpio_get_value(unsigned int gpio);
bool gpio_power_on(void);
bool gpio_power_off(void);

#endif /*GPIO_H_HWLAYER*/