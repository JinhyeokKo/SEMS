#ifndef DS18B20_H
#define DS18B20_H

#include "sems_def.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "esp_timer.h"

#define HW_DS18B20_GPIO DS18B20_DAT

void ds18b20_timer_init(void);
void ds18b20_start_timer(uint32_t interval_ms);
double ds18b20_get_temp(void);

#endif