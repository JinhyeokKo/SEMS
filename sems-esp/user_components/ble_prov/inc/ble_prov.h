#ifndef BLE_PROV_H
#define BLE_PROV_H

#include "esp_err.h"
#include <stdbool.h>

#define MAX_SSID_LEN 32
#define MAX_PASSWORD_LEN 64

typedef struct {
    char ssid[MAX_SSID_LEN];
    char password[MAX_PASSWORD_LEN];
    bool credentials_received;
} wifi_credentials_t;


esp_err_t ble_prov_init(void);
esp_err_t ble_prov_get_credentials(wifi_credentials_t* credentials);
esp_err_t ble_prov_start(void);
esp_err_t ble_prov_stop(void);

#endif