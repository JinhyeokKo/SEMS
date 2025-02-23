set(srcs "main.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "."
                       REQUIRES sdcard ds18b20 sqlite websocket ntp ble_prov
                       WHOLE_ARCHIVE)
