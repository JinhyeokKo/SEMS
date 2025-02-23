# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "user_components")
set(COMPONENTS main sqlite sdcard utils ds18b20 websocket ntp ble_prov)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(sems-esp)
