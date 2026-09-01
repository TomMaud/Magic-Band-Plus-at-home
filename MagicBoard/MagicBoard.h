#ifndef MAGICBOARD_H
#define MAGICBOARD_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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
    MASK_ALL          = 0x00,
    MASK_TOP_RIGHT    = 0x20,
    MASK_BOTTOM_RIGHT = 0x40,
    MASK_BOTTOM_LEFT  = 0x60, 
    MASK_TOP_LEFT     = 0x80,
    MASK_CENTRE       = 0xA0 
} magic_band_mask_t;

void initleds(uint8_t WS2812_PIN, uint NUM_PIXELS, uint8_t BRIGHTNESS);
void colour_fill(uint32_t colour, uint mask);
void section_fill(uint32_t centrecolour, uint32_t toprightcolour, uint32_t bottomrightcolour, uint32_t topleftcolour, uint32_t bottomleftcolour);
uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
void updatebrightness(uint8_t NewBrightness);

void pattern_clear();
void custom_colour_fill(uint8_t red, uint8_t green, uint8_t blue, uint mask);
void alternatediagonals(uint32_t colour_a, uint32_t colour_b, bool orientation);
void alternatelayers(uint32_t colour_a, uint32_t colour_b);
void commit_pattern();
void diagonalsband(uint32_t colour_a, uint32_t colour_b, uint32_t colour_c, uint32_t colour_d, uint32_t colour_e, uint32_t colour_f, uint32_t colour_g, bool orientation);
void layersband(uint32_t colour_a, uint32_t colour_b, uint32_t colour_c, uint32_t colour_d, uint32_t colour_e, uint32_t colour_f, uint32_t colour_g);
void diagonals(uint32_t layercolour, uint32_t diagonal_number, bool orientation);
void layers(uint32_t layercolour, uint32_t layer_number);

void stop_animations();
void start_crossfade_animation(uint8_t code_a, uint8_t code_b, uint8_t speed);
void start_rainbow_animation(uint8_t speed);
void start_circle_animation(uint8_t speed);
void start_corners_animation(uint8_t speed, uint8_t centre, uint8_t corner_a, uint8_t corner_b);
void start_flicker_animation(uint8_t speed, uint8_t centre, uint8_t topright, uint8_t bottomright, uint8_t topleft, uint8_t bottomleft);
void animation_worker();


#endif