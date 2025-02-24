#ifndef DS18B20_H
#define DS18B20_H

#include "sems_def.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"

#define HW_DS18B20_GPIO DS18B20_DAT

esp_err_t ds18b20_init(void);
double ds18b20_get_temp(void);

#endif