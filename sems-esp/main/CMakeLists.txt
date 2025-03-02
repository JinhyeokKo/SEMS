set(srcs "main.c")

idf_component_register(SRCS ${srcs}
                       REQUIRES common sdcard ds18b20 websocket ntp ble_prov sqlite)
