# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(p2p-lora-chat C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_CXX_ENABLE_RTTI 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(p2p-lora-chat main.cpp )

pico_set_program_name(p2p-lora-chat "p2p-lora-chat")
pico_set_program_version(p2p-lora-chat "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(p2p-lora-chat 1)
pico_enable_stdio_usb(p2p-lora-chat 1)

add_subdirectory(RadioLib)
include_directories(./RadioLib)

# Add the standard library to the build
target_link_libraries(p2p-lora-chat
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(p2p-lora-chat PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(p2p-lora-chat 
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_watchdog
        hardware_clocks
        hardware_pwm
        pico_multicore
        pico_cyw43_arch_none
        RadioLib
        )

pico_add_extra_outputs(p2p-lora-chat)

