#ifndef MAGICBAND_H
#define MAGICBAND_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// --- Enums ---
typedef enum {
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
} magic_band_colour_t;

typedef enum {
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
} magic_band_vib_t;

typedef enum {
    MASK_ALL          = 0x00,  // 000b - All LEDs
    MASK_TOP_RIGHT    = 0x20,  // 001b - Only top right LED
    MASK_BOTTOM_RIGHT = 0x40,  // 010b - Only bottom right LED
    MASK_BOTTOM_LEFT  = 0x60,  // 011b - Only bottom left LED
    MASK_TOP_LEFT     = 0x80   // 100b - Only top left LED
} magic_band_mask_t;

// --- Preset Arrays ---
extern const uint8_t rainbow[];
extern const uint8_t rainbow_len;

extern const uint8_t circle[];
extern const uint8_t circle_len;

extern const uint8_t Fire[];
extern const uint8_t Fire_len;

extern const uint8_t blue_flicker[];
extern const uint8_t blue_flicker_len;

// --- Core API Functions ---
bool ble_setup(void);
void broadcast_packet(const uint8_t *data, size_t length);

// --- Command Functions ---
void connect(void);
void clearband(void);
void singlecolour(uint8_t colour, uint8_t vib, uint8_t mask);
void dualcolour(uint8_t innercolour, uint8_t outercolour, uint8_t vib);
void fivecolour(uint8_t centrecolour, uint8_t toprightcolour, uint8_t bottomrightcolour, uint8_t topleftcolour, uint8_t bottomleftcolour, uint8_t vib);
void crossfade(uint8_t colour1, uint8_t colour2, uint8_t vib);

#endif