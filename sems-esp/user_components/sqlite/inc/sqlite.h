#ifndef SQLITE_H
#define SQLITE_H

#include "sems_def.h"
#include "sqlite3.h"
#include "sqllib.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "ds18b20.h"
#include "ntp.h"
#include "sdcard.h"

void sqlite_init(uint32_t interval_ms);

#endif