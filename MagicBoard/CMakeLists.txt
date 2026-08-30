# MagicBoard Sub-module CMakeLists.txt
cmake_minimum_required(VERSION 3.13)

# MagicBoard built as a static library
add_library(MagicBoard STATIC)

# Generate PIO header file
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(MagicBoard ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# Target sources and include directories
target_sources(MagicBoard PRIVATE MagicBoard.c)
target_include_directories(MagicBoard PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/generated
)

# Target dependencies
target_link_libraries(MagicBoard PRIVATE 
    pico_stdlib 
    hardware_pio 
    hardware_adc
    hardware_pwm 
    pico_multicore
)