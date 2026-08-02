# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Set the exact Pico SDK path FIRST
set(PICO_SDK_PATH "/Users/tommaud/.pico-sdk/sdk/2.3.0" CACHE PATH "Path to Pico SDK" FORCE)
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# 2. Import the SDK script directly
include(pico_sdk_import.cmake)

# 3. Initialize Project & SDK
project(MagicBand C CXX ASM)
pico_sdk_init()

# ==============================================================================
# SUBDIRECTORY MODULE & EXECUTABLE
# ==============================================================================

# Add the module subfolder
add_subdirectory(MagicPico)

# Build main application
add_executable(MagicBandApp
    main.c
)

pico_set_program_name(MagicBandApp "MagicBandApp")
pico_set_program_version(MagicBandApp "0.1")

# Enable USB serial output for debugging, disable UART
pico_enable_stdio_uart(MagicBandApp 0)
pico_enable_stdio_usb(MagicBandApp 1)

target_include_directories(MagicBandApp PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link against the MagicPico library module
target_link_libraries(MagicBandApp
    MagicPico
)

# Generate UF2 and extra output files
pico_add_extra_outputs(MagicBandApp)