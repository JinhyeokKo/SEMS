idf_component_register(
    SRCS "sqlite.c"
    INCLUDE_DIRS "inc"
    REQUIRES sdcard ntp ds18b20 common esp_psram
    )