# Create static library target for the module
add_library(MagicPico STATIC
    MagicPico.c
)

# Expose header folder to any target linking to MagicPico
target_include_directories(MagicPico PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

# Dependencies for BLE / CYW43 architecture
target_link_libraries(MagicPico PUBLIC
    pico_stdlib
    pico_cyw43_arch_none 
    pico_btstack_cyw43
    pico_btstack_ble
)

# Generate GATT header from Magicband.gatt
pico_btstack_make_gatt_header(MagicPico PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}/Magicband.gatt
)