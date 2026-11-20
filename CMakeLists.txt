cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_FETCH_FROM_GIT ON)

include(${CMAKE_CURRENT_LIST_DIR}/external/pico_sdk_import.cmake)

set(PROJECT_TARGET MusicBox)
project(${PROJECT_TARGET} C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(ENABLE_USB_OUTPUT 1)
set(ENABLE_UART_OUTPUT 0)

# set(LOG 0)

pico_sdk_init()

include(FetchContent)

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum
    GIT_TAG v0.9.7
)
FetchContent_MakeAvailable(magic_enum)

file(GLOB_RECURSE PROJECT_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/sources/*.cpp
)

add_executable(${PROJECT_TARGET} ${PROJECT_SOURCES})

target_compile_definitions(${PROJECT_TARGET} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    PICO_STDIO_USB_STDOUT_TIMEOUT_US=20000
    PICO_ENABLE_USB_RESET_VIA_BAUD_RATE=1
)

target_include_directories(${PROJECT_TARGET} PRIVATE
    sources
    sources/usb # tusb_config.h
)

target_link_libraries(${PROJECT_TARGET}
    pico_stdlib
    pico_unique_id
    pico_multicore
    hardware_adc
    hardware_pwm
    hardware_gpio
    hardware_dma
    tinyusb_device
    magic_enum::magic_enum
)

target_link_options(${PROJECT_TARGET} PRIVATE -Wl,--wrap=panic)

pico_enable_stdio_usb(${PROJECT_TARGET} ${ENABLE_USB_OUTPUT})
pico_enable_stdio_uart(${PROJECT_TARGET} ${ENABLE_UART_OUTPUT})

pico_add_extra_outputs(${PROJECT_TARGET})
