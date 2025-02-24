set(srcs "main.c")

idf_component_register(SRCS ${srcs}
                       REQUIRES common sdcard ds18b20 sqlite websocket ntp ble_prov)
