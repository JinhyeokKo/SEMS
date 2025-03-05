idf_component_register(
    SRCS "wifi_prov.c"
    INCLUDE_DIRS "inc"
    REQUIRES esp_wifi nvs_flash ble_prov common)
