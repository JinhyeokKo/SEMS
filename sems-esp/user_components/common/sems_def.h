#ifndef SEMS_DEF_H
#define SEMS_DEF_H

#include "esp_err.h"
#include "esp_log.h"
#include "sdkconfig.h"

// DS18B20 PIN SET
#define DS18B20_DAT GPIO_NUM_4

// SDCARD SET
#define SDCARD_MISO GPIO_NUM_13
#define SDCARD_MOSI GPIO_NUM_11
#define SDCARD_CLK  GPIO_NUM_12
#define SDCARD_CS   GPIO_NUM_10

#define MOUNT_POINT_SET "/sdcard"

// NTP TIMEZONE SET
#define TIMEZONE_NTP "KST-9"
#define SERVER_NTP   "time.windows.com"

// IF NOT USE BLE(Enter directly)
// #define CONFIG_WIFI
#ifdef CONFIG_WIFI
#define WIFI_SSID     "SSID"
#define WIFI_PASSWORD "PASSWORD"
#endif

// WIFI SET
#ifndef CONFIG_WIFI
#define MAX_SSID_LENGTH     32
#define MAX_PASSWORD_LENGTH 64
#endif

// #define CUSTOM_IP
#ifdef CUSTOM_IP
#define IP4_IP_SET      {192, 168, 0, 3}
#define IP4_GATEWAY_SET {192, 168, 0, 1}
#define IP4_NETMASK_SET {255, 255, 255, 0}
#define IP4_DNS_SET     {8, 8, 8, 8}
#endif

// #define CUSTOM_mDNS
#ifdef CUSTOM_mDNS
#define mDNS_HOST_NAME "sems"
#endif

// DB SET
#define DB_NAME "temp.db"
#define SAVE_TIME_INTERVAL 10000

#endif