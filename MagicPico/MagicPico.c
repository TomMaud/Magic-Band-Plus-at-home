#include "MagicPico.h"

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "btstack.h"
#include "MagicBoard.h"

static btstack_packet_callback_registration_t hci_event_cb;

static volatile bool ble_ready = false;
static volatile bool advertising_active = false;

static absolute_time_t advertising_stop_time;


static void packet_handler(uint8_t packet_type,
                           uint16_t channel,
                           uint8_t *packet,
                           uint16_t size)
{
    (void)channel;
    (void)size;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    if (hci_event_packet_get_type(packet) == BTSTACK_EVENT_STATE) {

        if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
            printf("BLE stack up\n");
            ble_ready = true;
        }
    }
}



void magicpico_init(void)
{
    printf("Initialising BLE...\n");

    l2cap_init();

    hci_event_cb.callback = &packet_handler;
    hci_add_event_handler(&hci_event_cb);

    hci_power_control(HCI_POWER_ON);

    bd_addr_t null_addr = {
        0, 0, 0, 0, 0, 0
    };

    gap_advertisements_set_params(
        0x0020,
        0x0020,
        0,
        0,
        null_addr,
        0x07,
        0x00
    );

    gap_advertisements_enable(0);

    advertising_active = false;
}

bool magicpico_is_ready(void)
{
    return ble_ready;
}

void broadcast_packet(uint8_t *data, uint8_t length)
{
    size_t start_idx = 0;

    if (length >= 2 &&
        data[0] == 0x83 &&
        data[1] == 0x01)
    {
        start_idx = 2;
    }

    size_t payload_len = length - start_idx;


    uint8_t mfg_len = 2 + payload_len;

    if ((5 + mfg_len) > 31) {
        return;
    }

    uint8_t adv_data[31];
    uint8_t idx = 0;

    adv_data[idx++] = 0x02;
    adv_data[idx++] = 0x01;
    adv_data[idx++] = 0x06;

    adv_data[idx++] = mfg_len + 1;
    adv_data[idx++] = 0xFF;


    adv_data[idx++] = 0x83;
    adv_data[idx++] = 0x01;


    memcpy(
        &adv_data[idx],
        data + start_idx,
        payload_len
    );

    idx += payload_len;


    gap_advertisements_enable(0);

    advertising_active = false;

    gap_advertisements_set_data(
        idx,
        adv_data
    );

    gap_advertisements_enable(1);

    advertising_active = true;

    advertising_stop_time =
        make_timeout_time_ms(
            MAGIC_PICO_ADVERTISING_TIME_MS
        );

}

void magicpico_update(void)
{
    if (!advertising_active) {
        return;
    }

    if (absolute_time_diff_us(
            get_absolute_time(),
            advertising_stop_time
        ) <= 0)
    {
        gap_advertisements_enable(0);
        advertising_active = false;
    }
}


void magicpico_stop(void)
{
    gap_advertisements_enable(0);

    advertising_active = false;

    printf("Advertising stopped\n");
}


bool magicpico_is_advertising(void)
{
    return advertising_active;
}


void send_colour(uint8_t colour, uint8_t vibration, uint8_t mask)
{
    uint8_t packet[] = {
        0x83,
        0x01,
        0xE9,
        0x05,
        0x00,
        0x2E,
        0x0E,
        (uint8_t)(mask | colour),
        (uint8_t)(0xB0 | vibration)
    };

    broadcast_packet(packet, sizeof(packet));
}

void fivecolour(uint8_t centrecolour,
                uint8_t toprightcolour,
                uint8_t bottomrightcolour,
                uint8_t topleftcolour,
                uint8_t bottomleftcolour,
                uint8_t vib)
{
    uint8_t packet[] = {
        0x83,
        0x01,
        0xE9,
        0x09,
        0x00,
        0x2E,
        0x0F,
        (uint8_t)(0xA0 | (centrecolour     & 0x1F)),
        (uint8_t)(0xA0 | (toprightcolour   & 0x1F)),
        (uint8_t)(0xA0 | (bottomrightcolour & 0x1F)),
        (uint8_t)(0xA0 | (bottomleftcolour & 0x1F)),
        (uint8_t)(0xA0 | (topleftcolour    & 0x1F)),
        (uint8_t)(0xB0 | vib)
    };

    broadcast_packet(packet, sizeof(packet));
}

void magicpico_clear(void)
{
    uint8_t packet[] = {
        0x83,
        0x01,
        0xE9,
        0x05,
        0x00,
        0x2E,
        0x0E,
        (uint8_t)(MASK_ALL | COLOUR_OFF),
        (uint8_t)(0xB0 | VIB_NONE)
    };


    broadcast_packet(packet, sizeof(packet));
}



void magicpico_wake(void)
{
    uint8_t packet[] = {
        0xCC,
        0x03,
        0x00,
        0x00,
        0x00
    };

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

void dualcolour(uint8_t innercolour, uint8_t outercolour, uint8_t vib) {
    uint8_t packet[] = {
        0x83,0x01,0xE9,0x06,0x00,0x22,0x0F,
        (uint8_t)(0x40|innercolour),
        (uint8_t)(0x40|outercolour),
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



void pulse(uint8_t colour, uint8_t mask, uint8_t speed, uint8_t vib) {
     uint8_t packet[] = {0x83,0x01,0xE1,0x00,0xE9,0x14,0x00,
                         speed,
                         0xD0,0x37,0xF0,0xD2,0x3D,0x05,0x0C,0x0C,0x0E,
                         (uint8_t)(mask|colour),
                         0x89,0x83,0x51,0x0E,0xEE,0x0C,0x3D,(uint8_t)(0xB0|vib)};
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

void alternating_colours(uint8_t colour1, uint8_t colour2, uint8_t timing) {
    uint8_t packet[] = {0x83,0x01,0xE9,0x10,0x00,
                         timing,
                         0x0F,
                         (uint8_t)(MASK_TOP_RIGHT|colour1),
                         (uint8_t)(MASK_TOP_RIGHT|colour2),
                         0x58,0xF4,0x48,0x82,0xD1,0x46,0x09,0x0A,0xD0,0x65,0x28,0x21,0x02};
    broadcast_packet(packet, sizeof(packet));
}

void corners_alternate(uint8_t centrecolour,uint8_t topleftcolour, uint8_t toprightcolour, uint8_t bottomrightcolour, uint8_t bottomleftcolour, 
                        uint8_t timing, uint8_t vib) {uint8_t packet[] = {0x83,0x01,0xE2,0x00,0xE9,0x13,0x00,
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

uint8_t circle[] = {
    0x83,0x01,0xE9,0x0B,0x0B,0x0F,0x0F,
    0x5C,0x5D,0x48,0xA5,0xD1,0x45,0x32,
    (uint8_t)(0xB0|7)};
uint8_t blink_white[]   = {0x83,0x01,0xE1,0x00,0xE9,0x0C,0x00,0x0F,0x0F,0x5D,0x46,0x5B,0xF0,0x05,0x32,0x37,0x48,0x95};
uint8_t orange_blink[]  = {0x83,0x01,0xE1,0x00,0xE9,0x0C,0x00,0xEF,0x0F,0x4F,0x4F,0x5B,0xF0,0xFB,0x14,0x37,0x48,0x95};
uint8_t colour_cycle[]   = {0x83,0x01,0xE1,0x00,0xE9,0x0C,0x00,0x0F,0x0F,0xB1,0xB9,0xB5,0xB1,0xA2,0x30,0x7B,0x7D,0xB0};
uint8_t taste_rainbow[] = {0x83,0x01,0xE1,0x00,0xE9,0x0C,0x00,0x0F,0x0F,0x5D,0x46,0x5B,0xF0,0x05,0x32,0x37,0x48,0xB0};

uint8_t taste_rainbow_len = sizeof(taste_rainbow);
uint8_t blink_white_len = sizeof(blink_white);
uint8_t orange_blink_len = sizeof(orange_blink);
uint8_t colour_cycle_len = sizeof(colour_cycle);
uint8_t circle_len = sizeof(circle);