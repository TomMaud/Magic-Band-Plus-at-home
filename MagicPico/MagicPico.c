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

bool ble_setup(void) {
    l2cap_init();
    hci_event_cb.callback = &packet_handler;
    hci_add_event_handler(&hci_event_cb);
    hci_power_control(HCI_POWER_ON);

    bd_addr_t null_addr = {0,0,0,0,0,0};
    gap_advertisements_set_params(
        0x20, 0x40,
        0,
        0, null_addr,
        0x07,
        0x00
    );


    // Wait up to 3 seconds for BLE stack startup
    int timeout = 300;
    while (!ble_ready && timeout-- > 0) {
        sleep_ms(10);
    }

    return ble_ready;
}


void broadcast_packet(const uint8_t *data, size_t length) {
    size_t start_idx = 0;
    if (length >= 2 && data[0] == 0x83 && data[1] == 0x01) {
        start_idx = 2;
    }
    static const uint8_t scan_response_data[] = {
    0x0B, 0x09, 'M', 'a', 'g', 'i', 'c', 'P', 'i', 'c', 'o'
    };

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
    gap_scan_response_set_data(sizeof(scan_response_data), (uint8_t *) scan_response_data);
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

void customcolour(uint8_t r, uint8_t g, uint8_t b, uint8_t vib , bool flash_r, bool flash_g, bool flash_b) {
     uint8_t r_byte = (uint8_t)((r & 0x3F) << 1);
    if (flash_r) {
        r_byte |= 0x80;
    }

    uint8_t g_byte = (uint8_t)((g & 0x3F) << 1);
    if (flash_g) {
        g_byte |= 0x80;
    }

    uint8_t b_byte = (uint8_t)((b & 0x3F) << 1);
    if (flash_b) {
        b_byte |= 0x80;
    }
    uint8_t packet[] = {
        0x83,0x01,0xE9,0x08,0x00,0x0E,
        0xD2, 0x55,
        (uint8_t)(r_byte),
        (uint8_t)(g_byte),
        (uint8_t)(b_byte),
        (uint8_t)(0xB0|vib)
    };
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

void crossfade(uint8_t colour1, uint8_t colour2, uint8_t vib, uint8_t speed) {
    uint8_t packet[] = {
        0x83,0x01,0xE9,0x11,0x00,speed,0x0F,
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


uint8_t circle[] = {
    0x83,0x01,0xE9,0x0B,0x0B,0x0F,0x0F,
    0x5C,0x5D,0x48,0xA5,0xD1,0x45,0x32,
    (uint8_t)(0xB0|7)
};
uint8_t circle_len = sizeof(circle);

uint8_t Fire[] = {
    0x83,0x01,0xE9,0x10,0x00,0x0F,0x0F,
    0x54,0x5D,0x58,0xF4,0x48,0x82,0xD1,0x46,0x09,0x0A,0xD0,0x65,0x28,
    (uint8_t)(0xB0|VIB_NONE)
};
uint8_t Fire_len = sizeof(Fire);

uint8_t blueflicker[] = {0x83,0x01,0xE9,0x10,0x00,0x13,0x48,0x97,0xD0,0x0E,0xA0,0xD1,0x46,0x06,0x0F,0x30,0xD0,0x4E,0x07,
    (uint8_t)(0xB0|VIB_NONE)
};

uint8_t blueflicker_len = sizeof(blueflicker);


// --- E9 0C: Blink/animation codes ---
uint8_t blink_white[]   = {0x83,0x01,0xE1,0x00,0xE9,0x0C,0x00,0x0F,0x0F,0x5D,0x46,0x5B,0xF0,0x05,0x32,0x37,0x48,0x95}; // 18 bytes
uint8_t orange_blink[]  = {0x83,0x01,0xE1,0x00,0xE9,0x0C,0x00,0xEF,0x0F,0x4F,0x4F,0x5B,0xF0,0xFB,0x14,0x37,0x48,0x95}; // 18 bytes
uint8_t colour_cycle[]   = {0x83,0x01,0xE1,0x00,0xE9,0x0C,0x00,0x0F,0x0F,0xB1,0xB9,0xB5,0xB1,0xA2,0x30,0x7B,0x7D,0xB0}; // 18 bytes
uint8_t taste_rainbow[] = {0x83,0x01,0xE1,0x00,0xE9,0x0C,0x00,0x0F,0x0F,0x5D,0x46,0x5B,0xF0,0x05,0x32,0x37,0x48,0xB0}; // 18 bytes


// --- E9 10: Alternating colours ---
uint8_t e10_1[] = {0x83,0x01,0xE9,0x10,0x00,0x0F,0x0F,0x54,0x5D,0x58,0xF4,0x48,0x82,0xD1,0x46,0x09,0x0A,0xD0,0x65,0x28,0x21,0x02}; // 22 bytes
uint8_t e10_2[] = {0x83,0x01,0xE1,0x00,0xE9,0x10,0x00,0x13,0x48,0x97,0xD0,0x0E,0xA0,0xD1,0x46,0x06,0x0F,0x30,0xD0,0x4E,0x07,0xB0}; // 22 bytes

// --- E9 13: Another animation ---
uint8_t e13_base[] = {0x83,0x01,0xE9,0x13,0x00,0xB6,0x0F,0x40,0x44,0x58,0xF4,0x48,0x82,0xD0,0x65,0x19,0xD1,0x46,0x06,0x0A,0x30,0x7B,0xFF}; // 23 bytes
uint8_t e13_1[]    = {0x83,0x01,0xE1,0x00,0xE9,0x13,0x00,0x02,0xD0,0x37,0xF0,0xD2,0x3D,0x05,0x05,0x00,0x0E,0xFA,0x89,0x83,0x51,0x0E,0xE7,0xA0,0xB0}; // 25 bytes
uint8_t e13_2[]    = {0x83,0x01,0xE2,0x00,0xE9,0x13,0x00,0x65,0x0F,0xBD,0xB5,0xBC,0xB5,0xBC,0x7A,0xEC,0x5C,0x0A,0x29,0x15,0x29,0x15,0x48,0xAB,0xB0}; // 25 bytes



void pulse(uint8_t colour, uint8_t mask, uint8_t speed) {
     uint8_t packet[] = {0x83,0x01,0xE1,0x00,0xE9,0x14,0x00,
                         speed,
                         0xD0,0x37,0xF0,0xD2,0x3D,0x05,0x0C,0x0C,0x0E,
                         (uint8_t)(mask|colour),
                         0x89,0x83,0x51,0x0E,0xEE,0x0C,0x3D,0xB0};
    broadcast_packet(packet, sizeof(packet));
}

void dualpulse(uint8_t mask, uint8_t center_colour, uint8_t outer_colour, uint8_t speed, uint8_t vib) {
    uint8_t packet[] = {0x83,0x01,0xE1,0x00,0xE9,0x0F,0x00,
                         speed,
                         0x0F,
                         (uint8_t)(mask|center_colour),
                         (uint8_t)(mask|outer_colour),
                         0x58,0x07,0x48,0x8D,0xD2,0x46,0x2A,0x07,0x17,(uint8_t)(0xB0|vib)};
    broadcast_packet(packet, sizeof(packet));

}

void five_slot_animation(uint8_t centrecolour, uint8_t toprightcolour, uint8_t bottomrightcolour, uint8_t topleftcolour, uint8_t bottomleftcolour,
                          uint8_t timing, uint8_t vib) {
    uint8_t packet[] = {0x83,0x01,0xE1,0x00,0xE9,0x0E,0x00,
                         timing,
                         0x0F,
                         (uint8_t)(0xA0|centrecolour),
                         (uint8_t)(0xA0|toprightcolour),
                         (uint8_t)(0xA0|bottomrightcolour),
                         (uint8_t)(0xA0|topleftcolour),
                         (uint8_t)(0xA0|bottomleftcolour),
                         0x59,0x19,0x02,0x48,0xAE,
                         (uint8_t)(0xB0|vib)};
    broadcast_packet(packet, sizeof(packet));
}

void alternating_colours(uint8_t mask, uint8_t colour1, uint8_t colour2, uint8_t timing) {
    uint8_t packet[] = {0x83,0x01,0xE9,0x10,0x00,
                         timing,
                         0x0F,
                         (uint8_t)(mask|colour1),
                         (uint8_t)(mask|colour2),
                         0x58,0xF4,0x48,0x82,0xD1,0x46,0x09,0x0A,0xD0,0x65,0x28,0x21,0x02};
    broadcast_packet(packet, sizeof(packet));
}

void corners_alternate(uint8_t centrecolour,uint8_t topleftcolour, uint8_t toprightcolour, uint8_t bottomrightcolour, uint8_t bottomleftcolour, 
                        uint8_t timing, uint8_t vib) {
    uint8_t packet[] = {0x83,0x01,0xE2,0x00,0xE9,0x13,0x00,
                         timing,
                         0x0F,
                         (uint8_t)(0xA0|centrecolour),
                         (uint8_t)(0xA0|topleftcolour),
                         (uint8_t)(0xA0|toprightcolour),
                         (uint8_t)(0xA0|bottomrightcolour),
                         (uint8_t)(0xA0|bottomleftcolour),
                         0x7A,0xEC,0x5C,0x0A,0x29,0x15,0x29,0x15,0x48,0xAB,
                         (uint8_t)(0xB0|vib)};
    broadcast_packet(packet, sizeof(packet));
}