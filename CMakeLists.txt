# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_EXTRAS_PATH "../pico-extras")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(rp2040-vult C CXX ASM)

# set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

include_directories(".")


# Generate Vult C++ code from Vult Code every time
set(VULT_EXAMPLE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vult/examples")
add_custom_command(
        OUTPUT ALL ${CMAKE_CURRENT_BINARY_DIR}/vult.cpp
        COMMAND vultc ${CMAKE_CURRENT_SOURCE_DIR}/dsp.vult
                -i ${VULT_EXAMPLE_PATH}/osc
                -i ${VULT_EXAMPLE_PATH}/util
                -i ${VULT_EXAMPLE_PATH}/filters
                -i ${VULT_EXAMPLE_PATH}/env
                -i ${VULT_EXAMPLE_PATH}/midi
                -i ${VULT_EXAMPLE_PATH}/effects
                -ccode
                -real fixed
                -samplerate 44100
                -o vult 
        COMMENT "Transcompiling Vult DSP Code to C++."
)

add_library(midi_lib INTERFACE)
target_sources(midi_lib INTERFACE midi_input.cpp midi_input_usb.cpp)
target_include_directories(midi_lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(midi_lib INTERFACE pico_stdlib)

add_library(audio_lib INTERFACE)
target_sources(audio_lib INTERFACE audio_i2s.c audio.cpp audio_subsystem.cpp)
target_include_directories(audio_lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(audio_lib INTERFACE pico_stdlib hardware_pio hardware_dma pico_util_buffer)

add_library(vult_runtime_lib INTERFACE)
target_sources(vult_runtime_lib INTERFACE vultin.cpp)
target_include_directories(vult_runtime_lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(vult_runtime_lib INTERFACE pico_stdlib)


add_executable(rp2040-vult
        usb_descriptors.c
        main.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/vult.cpp)

pico_generate_pio_header(rp2040-vult ${CMAKE_CURRENT_LIST_DIR}/audio_i2s.pio)

pico_set_program_name(rp2040-vult "rp2040-vult")
pico_set_program_version(rp2040-vult "0.1")

pico_enable_stdio_uart(rp2040-vult 1)
#pico_enable_stdio_usb(rp2040-vult 0)

# Add the standard library to the build
target_link_libraries(rp2040-vult pico_stdlib midi_lib audio_lib vult_runtime_lib)

# Add any user requested libraries
target_link_libraries(rp2040-vult
        hardware_adc
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_watchdog
        hardware_clocks
        pico_util_buffer
        pico_multicore
        pico_stdio_uart
        tinyusb_device
        tinyusb_board
        )

target_compile_definitions(rp2040-vult PRIVATE
        USE_AUDIO_I2S=1
        )

add_compile_options("-Wall" "-Wredundant-decls")

pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)

pico_set_boot_stage2(rp2040-vult slower_boot2)

pico_add_extra_outputs(rp2040-vult)