cmake_minimum_required(VERSION 3.20)
set(PICO_SDK_FETCH_FROM_GIT on)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++23>)
if((EXISTS $ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake) AND (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/pico_sdk_import.cmake))
execute_process(COMMAND ln -s $ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake ${CMAKE_CURRENT_BINARY_DIR}/pico_sdk_import.cmake)
elseif(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/pico_sdk_import.cmake)
file(DOWNLOAD https://raw.githubusercontent.com/raspberrypi/pico-sdk/master/external/pico_sdk_import.cmake pico_sdk_import.cmake)
endif()
include(${CMAKE_CURRENT_BINARY_DIR}/pico_sdk_import.cmake)
project(rp2040_pendsv_svc_test C CXX ASM)
pico_sdk_init()
add_executable(main main.cpp context_switch.s)
target_link_libraries(main pico_stdlib hardware_exception)
pico_enable_stdio_usb(main on)
pico_add_extra_outputs(main)