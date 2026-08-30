#ifndef MAGICPICO_H
#define MAGICPICO_H

#include <stdint.h>
#include <stdbool.h>



#define MAGIC_PICO_ADVERTISING_TIME_MS 5000

void magicpico_init(void);
bool magicpico_is_ready(void);
void magicpico_update(void);

void send_colour(uint8_t colour,
                           uint8_t vibration,
                           uint8_t mask);

void magicpico_wake(void);
void magicpico_clear(void);

void magicpico_stop(void);
bool magicpico_is_advertising(void);

void fivecolour(uint8_t centre, uint8_t top_right, uint8_t bottom_right, uint8_t top_left, uint8_t bottom_left, uint8_t vibration);
void customcolour(uint8_t r, uint8_t g, uint8_t b, uint8_t vib , bool flash_r, bool flash_g, bool flash_b);
void dualcolour(uint8_t innercolour, uint8_t outercolour, uint8_t vib);
void crossfade(uint8_t colour1, uint8_t colour2, uint8_t vib, uint8_t speed);
void pulse(uint8_t colour, uint8_t mask, uint8_t speed, uint8_t vib);
void dualpulse(uint8_t mask, uint8_t center_colour, uint8_t outer_colour, uint8_t speed, uint8_t vib);
void five_slot_animation(uint8_t centre, uint8_t top_right, uint8_t bottom_right, uint8_t top_left, uint8_t bottom_left, uint8_t vib, uint8_t speed); 
void alternating_colours(uint8_t colour1, uint8_t colour2, uint8_t timing);
void corners_alternate(uint8_t centrecolour,uint8_t topleftcolour, uint8_t toprightcolour, uint8_t bottomrightcolour, uint8_t bottomleftcolour, uint8_t timing, uint8_t vib);
void broadcast_packet(uint8_t *packet, uint8_t packet_size);

extern uint8_t blink_white[]; // 18 bytes
extern uint8_t orange_blink[]; // 18 bytes
extern uint8_t colour_cycle[]; // 18 bytes
extern uint8_t taste_rainbow[]; // 18 bytes
extern uint8_t circle[];
extern uint8_t circle_len;
extern uint8_t taste_rainbow_len;
extern uint8_t blink_white_len;
extern uint8_t orange_blink_len;
extern uint8_t colour_cycle_len;


#endif