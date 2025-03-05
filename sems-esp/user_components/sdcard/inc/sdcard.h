#ifndef SDCARD_H
#define SDCARD_H

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "sems_def.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#define PIN_NUM_MISO SDCARD_MISO
#define PIN_NUM_MOSI SDCARD_MOSI
#define PIN_NUM_CLK  SDCARD_CLK
#define PIN_NUM_CS   SDCARD_CS

#define MOUNT_POINT MOUNT_POINT_SET

esp_err_t sdcard_init(void);
const char *get_mount_point(void);

#endif