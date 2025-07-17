cmake_minimum_required(VERSION 3.13)
include(~/pico/pico-sdk/pico_sdk_init.cmake)

project(i2s_sine_example C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(i2s_sine
    main.c
    i2s.pio
)

pico_generate_pio_header(i2s_sine ${CMAKE_CURRENT_LIST_DIR}/i2s.pio)
target_link_libraries(i2s_sine pico_stdlib hardware_pio)
pico_add_extra_outputs(i2s_sine)
