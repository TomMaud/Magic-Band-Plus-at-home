#ifndef MAGICBAND_H
#define MAGICBAND_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


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
bool magicpico_is_ready(void);
void magicpico_init(void);
void broadcast_packet(uint8_t *data, uint8_t length);
void magicpico_update(void);

// --- Command Functions ---
void magicpico_wake(void);
void magicpico_clear(void);
void send_colour(uint8_t colour, uint8_t vib, uint8_t mask);
void dualcolour(uint8_t innercolour, uint8_t outercolour, uint8_t vib);
void fivecolour(uint8_t centrecolour, uint8_t toprightcolour, uint8_t bottomrightcolour, uint8_t topleftcolour, uint8_t bottomleftcolour, uint8_t vib);
void crossfade(uint8_t colour1, uint8_t colour2, uint8_t vib, uint8_t speed);
void customcolour(uint8_t r, uint8_t g, uint8_t b, uint8_t vib , bool flash_r, bool flash_g, bool flash_b);
void pulse(uint8_t colour, uint8_t mask, uint8_t speed, uint8_t vib);
void dualpulse(uint8_t mask, uint8_t center_colour, uint8_t outer_colour, uint8_t speed, uint8_t vib);
void alternating_colours(uint8_t colour1, uint8_t colour2, uint8_t timing);
void five_slot_animation(uint8_t centrecolour, uint8_t toprightcolour, uint8_t bottomrightcolour, uint8_t topleftcolour, uint8_t bottomleftcolour, uint8_t timing, uint8_t vib);
void corners_alternate(uint8_t centrecolour,uint8_t topleftcolour, uint8_t toprightcolour, uint8_t bottomrightcolour, uint8_t bottomleftcolour, uint8_t timing, uint8_t vib);

extern const uint8_t blink_white[]; // 18 bytes
extern const uint8_t orange_blink[]; // 18 bytes
extern const uint8_t colour_cycle[]; // 18 bytes
extern const uint8_t taste_rainbow[]; // 18 bytes



extern uint8_t taste_rainbow_len;
extern uint8_t blink_white_len;
extern uint8_t orange_blink_len;
extern uint8_t colour_cycle_len;

#endif