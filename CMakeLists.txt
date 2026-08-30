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

# Force the SDK path globally across all sub-projects and external builds
set(PICO_SDK_PATH "/Users/tommaud/.pico-sdk/sdk/2.3.0" CACHE PATH "Path to the Pico SDK" FORCE)
set(PICO_BOARD pico_w CACHE STRING "Board type" FORCE)

include(pico_sdk_import.cmake)

project(MagicController C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pass the SDK path down to subdirectories like MagicBoard
set(PICO_SDK_PATH "${PICO_SDK_PATH}" PARENT_SCOPE)

pico_sdk_init()

add_subdirectory(MagicBoard)

add_executable(MagicController
    main.c
    MagicPico/MagicPico.c
    lib/dhcpserver/dhcpserver.c
    lib/dnsserver/dnsserver.c
)

pico_enable_stdio_uart(MagicController 0)
pico_enable_stdio_usb(MagicController 1)

# Set include paths matching your folder layout
target_include_directories(MagicController PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/MagicPico
    ${CMAKE_CURRENT_LIST_DIR}/lib/dhcpserver
    ${CMAKE_CURRENT_LIST_DIR}/lib/dnsserver
)

target_link_libraries(MagicController
    pico_stdlib
    pico_multicore
    pico_cyw43_arch_lwip_poll
    pico_btstack_cyw43
    pico_btstack_ble
    MagicBoard
)

pico_add_extra_outputs(MagicController)