# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/ubuntu/pico/pico-sdk")

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(tinyprogrammer C CXX ASM)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(tinyprogrammer tinyprogrammer.cpp )

pico_set_program_name(tinyprogrammer "tinyprogrammer")
pico_set_program_version(tinyprogrammer "0.1")

pico_enable_stdio_uart(tinyprogrammer 1)
pico_enable_stdio_usb(tinyprogrammer 0)
target_include_directories(tinyprogrammer PRIVATE ${CMAKE_CURRENT_LIST_DIR})
# Add the standard library to the build
target_link_libraries(tinyprogrammer pico_stdlib pico_multicore pico_sync hardware_adc hardware_clocks hardware_spi tinyusb_device tinyusb_board hardware_pio)

pico_add_extra_outputs(tinyprogrammer)

