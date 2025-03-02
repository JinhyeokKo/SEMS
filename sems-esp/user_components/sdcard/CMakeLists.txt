idf_component_register(
    SRCS "sdcard.c"
    INCLUDE_DIRS "inc"
    REQUIRES fatfs common
    WHOLE_ARCHIVE)
