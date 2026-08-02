#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "MagicPico.h"

int main(void) {
    // 1. Initialize USB/UART stdio output
    stdio_init_all();
    
    // Brief delay to allow USB serial terminal to connect after boot
    sleep_ms(3000);

    printf("\n=== MagicPico Minimal Test Harness ===\n");

    // 2. Initialize low-level CYW43 Wi-Fi/BLE hardware driver
    printf("[SYS] Initializing CYW43 architecture...\n");
    if (cyw43_arch_init()) {
        printf("[ERR] Failed to initialize CYW43 driver!\n");
        return -1;
    }

    // 3. Initialize BTStack via the MagicPico module
    printf("[BLE] Initializing MagicPico BLE stack...\n");
    if (!ble_setup()) {
        printf("[ERR] MagicPico BLE initialization timed out!\n");
        return -1;
    }

    printf("[SYS] Setup complete! Starting test loop...\n\n");

    // 4. Test Loop: Cycles through actions every 4 seconds
    while (true) {
        printf("[TEST] 1. Sending Connect / Wake command...\n");
        connect();
        sleep_ms(4000);

        printf("[TEST] 2. Setting Red LED with Short Vibration...\n");
        singlecolour(COLOUR_RED, VIB_S, MASK_ALL);
        sleep_ms(4000);

        printf("[TEST] 3. Setting Blue LED...\n");
        singlecolour(COLOUR_BLUE, VIB_NONE, MASK_ALL);
        sleep_ms(4000);

        printf("[TEST] 4. Triggering Crossfade (Red -> Cyan)...\n");
        crossfade(COLOUR_RED, COLOUR_CYAN, VIB_S);
        sleep_ms(4000);

        broadcast_packet(circle, circle_len);
        sleep_ms(4000);

        crossfade(COLOUR_BLUE, COLOUR_GREEN, VIB_S3_M3_S3);
        sleep_ms(4000);

        broadcast_packet(Fire, Fire_len);
        sleep_ms(4000);

        crossfade(COLOUR_RANDOM, COLOUR_RANDOM, VIB_XS6);
        sleep_ms(4000);

        printf("[TEST] 5. Broadcasting Rainbow Effect...\n");
        broadcast_packet(rainbow, rainbow_len);
        sleep_ms(4000);

        printf("[TEST] 6. Clearing / Turning off Band...\n");
        clearband();
        sleep_ms(4000);

        printf("--- Loop complete. Restarting sequence in 2s ---\n\n");
        sleep_ms(2000);
    }

    return 0;
}