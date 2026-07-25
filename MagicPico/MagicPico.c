#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"   // <-- add this back
#include "btstack.h"

static btstack_packet_callback_registration_t hci_event_cb;
static bool ble_ready = false;


enum MagicBandCOLOUR {
    COLOUR_CYAN          = 0x00,
    COLOUR_PURPLE        = 0x01,
    COLOUR_BLUE          = 0x02,
    COLOUR_MIDNIGHT_BLUE = 0x03,
    COLOUR_BRIGHT_PURPLE = 0x05,
    COLOUR_LAVENDER      = 0x06,
    COLOUR_PINK          = 0x08,
    COLOUR_YELLOW_ORANGE = 0x0F,
    COLOUR_OFF_YELLOW    = 0x10,
    COLOUR_LIME          = 0x12,
    COLOUR_ORANGE        = 0x13,
    COLOUR_RED_ORANGE    = 0x14,
    COLOUR_RED           = 0x15,
    COLOUR_GREEN         = 0x19,
    COLOUR_LIME_GREEN    = 0x1A,
    COLOUR_WHITE         = 0x1B,
    COLOUR_OFF           = 0x1D,
    COLOUR_RANDOM        = 0x1F
};

enum MagicBandVibration {
    VIB_NONE      = 0x0,   
    VIB_S         = 0x1,
    VIB_S_S       = 0x2,
    VIB_S_S_S     = 0x3,
    VIB_S_S_M     = 0x4,
    VIB_S4_M_S    = 0x5,
    VIB_S3_M3_S3  = 0x6,
    VIB_XL        = 0x7,
    VIB_XS6       = 0x8,
    VIB_S_ALT     = 0x9,
    VIB_M         = 0xA,
    VIB_L         = 0xB
};

enum MagicBandMask {
    MASK_ALL        = 0x00,  // 000b - All LEDs
    MASK_TOP_RIGHT  = 0x20,  // 001b - Only top right LED
    MASK_BOTTOM_RIGHT = 0x40,  // 010b - Only bottom right LED
    MASK_BOTTOM_LEFT  = 0x60,  // 011b - Only bottom left LED
    MASK_TOP_LEFT     = 0x80,  // 100b - Only top left LED
};

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) == BTSTACK_EVENT_STATE) {
        if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
            printf("BLE stack up\n");
            ble_ready = true;
        }
    }
}

void ble_setup(void) {
    l2cap_init();
    hci_event_cb.callback = &packet_handler;
    hci_add_event_handler(&hci_event_cb);
    hci_power_control(HCI_POWER_ON);

    // btstack_run_loop_execute() needs to spin at least once
    // before ble_ready is set — call //broadcast_packet() only after that.

    bd_addr_t null_addr = {0,0,0,0,0,0};
    gap_advertisements_set_params(
        0x20, 0x40,    // min/max interval — same 0x20/0x40 as the ESP32 version
        0,             // ADV_IND, undirected connectable
        0, null_addr,  // direct addr fields unused for undirected adv
        0x07,          // all 3 advertising channels
        0x00           // no filter/whitelist
    );
}


void broadcast_packet(const uint8_t *data, size_t length) {
    size_t start_idx = 0;
    if (length >= 2 && data[0] == 0x83 && data[1] == 0x01) {
        start_idx = 2;
    }

    uint8_t adv_data[31];
    uint8_t idx = 0;

    // Flags AD structure
    adv_data[idx++] = 0x02;
    adv_data[idx++] = 0x01;
    adv_data[idx++] = 0x06;

    // Manufacturer Data AD structure: 0x83, 0x01, then the rest of the payload
    uint8_t mfg_len = 2 + (length - start_idx);
    adv_data[idx++] = mfg_len + 1;   // +1 for the AD type byte
    adv_data[idx++] = 0xFF;
    adv_data[idx++] = 0x83;
    adv_data[idx++] = 0x01;
    memcpy(&adv_data[idx], data + start_idx, length - start_idx);
    idx += (length - start_idx);

    gap_advertisements_enable(0);
    gap_advertisements_set_data(idx, adv_data);
    gap_advertisements_enable(1);

    sleep_ms(2000);
    gap_advertisements_enable(0);
}

void singlecolour(uint8_t colour, uint8_t vib, uint8_t mask) {
    uint8_t packet[] = {0x83,0x01,
        0xE9,0x05,
        0x00,0x2E,0x0E,
                         (uint8_t)(mask|colour),(uint8_t)(0xB0|vib)};
    broadcast_packet(packet, sizeof(packet));
}

void clearband(void) {
    uint8_t packet[] = {0x83, 0x01, 0xE9, 0x05, 0x00, 0x2E, 0x0E, (uint8_t)(MASK_ALL|COLOUR_OFF), (uint8_t)(0xB0|VIB_NONE)};
    broadcast_packet(packet, sizeof(packet));
}

void dualcolour(uint8_t innercolour, uint8_t outercolour, uint8_t vib) {
    uint8_t packet[] = {
        0x83,0x01,0xE9,0x06,0x00,0x22,0x0F,
        (uint8_t)(0x40|innercolour),
        (uint8_t)(0x40|outercolour),
        (uint8_t)(0xB0|vib)
    };
    broadcast_packet(packet, sizeof(packet));
}

void fivecolour(uint8_t centrecolour, uint8_t toprightcolour, uint8_t bottomrightcolour, uint8_t topleftcolour, uint8_t bottomleftcolour, uint8_t vib) {
    uint8_t packet[] = {
        0x83,0x01,0xE9,0x09,0x00,0x2E,0x0F,
        (uint8_t)(0xA0|centrecolour),
        (uint8_t)(0xA0|toprightcolour),
        (uint8_t)(0xA0|bottomrightcolour),
        (uint8_t)(0xA0|bottomleftcolour),
        (uint8_t)(0xA0|centrecolour),
        (uint8_t)(0xB0|vib)
    };
    broadcast_packet(packet, sizeof(packet));
}

void crossfade(uint8_t colour1, uint8_t colour2, uint8_t vib) {
    uint8_t packet[] = {
        0x83,0x01,0xE9,0x11,0x00,0x6F,0x0F,
        (uint8_t)(0x40|colour1),
        (uint8_t)(0x40|colour2),
        0x58,0xF4,0x48,0x82,0xD1,0x46,0x02,0x08,0xD0,0x65,0x00,
        (uint8_t)(0xB0|vib)
    };
    broadcast_packet(packet, sizeof(packet));
}


void connect(void) {
    uint8_t ping[] = {0xCC, 0x03, 0x00, 0x00, 0x00};
    uint8_t vib = 0;
    broadcast_packet(ping, sizeof(ping));
    sleep_ms(420);
}

uint8_t rainbow[] = {
    0x83,0x01,
    0xE9,0x09,
    0x00,0x2E,0x0F,
    (uint8_t)(0xA0|COLOUR_YELLOW_ORANGE),
    (uint8_t)(0xA0|COLOUR_RED),
    (uint8_t)(0xA0|COLOUR_GREEN),
    (uint8_t)(0xA0|COLOUR_BLUE),
    (uint8_t)(0xA0|COLOUR_PURPLE),
    (uint8_t)(0xB0|0)
};

uint8_t circle[] = {
    0x83,0x01,0xE9,0x0B,0x0B,0x0F,0x0F,
    0x5C,0x5D,0x48,0xA5,0xD1,0x45,0x32,
    (uint8_t)(0xB0|7)
};


uint8_t Fire[] = {
    0x83,0x01,0xE9,0x10,0x00,0x0F,0x0F,
    0x54,0x5D,0x58,0xF4,0x48,0x82,0xD1,0x46,0x09,0x0A,0xD0,0x65,0x28,
    (uint8_t)(0xB0|VIB_NONE)
};


uint8_t blueflicker[] = {0x83,0x01,0xE9,0x10,0x00,0x13,0x48,0x97,0xD0,0x0E,0xA0,0xD1,0x46,0x06,0x0F,0x30,0xD0,0x4E,0x07,
    (uint8_t)(0xB0|VIB_NONE)
};



int main(void) {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("cyw43_arch_init failed\n");
        return -1;
    }

    ble_setup();

    while (!ble_ready) {
        sleep_ms(10);
    }

    connect();

    while (true) {
        clearband();
        crossfade(COLOUR_BLUE, COLOUR_GREEN, VIB_NONE);
        sleep_ms(3000);
    }

    return 0;
}