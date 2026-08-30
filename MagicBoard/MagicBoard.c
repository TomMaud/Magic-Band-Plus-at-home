#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/multicore.h"
#include "MagicBoard.h"

enum { NUM_PIXELS = 31 };
uint brightness;

magic_band_mask_t holdmask[5] = {0x20,0x40, 0x60, 0x80, 0xA0};

typedef enum {
    ANIM_NONE = 0,
    ANIM_CROSSFADE,
    ANIM_RAINBOW,
    ANIM_CIRCLE,
    ANIM_CORNERS,
    ANIM_FLICKER
} animation_type_t;


volatile animation_type_t current_animation = ANIM_NONE;
volatile uint32_t anim_colour_a = 0;
volatile uint32_t anim_colour_b = 0;
volatile uint8_t anim_speed = 50;
volatile uint32_t anim_colour_c = 0;
volatile uint32_t anim_colour_d = 0;
volatile uint32_t anim_colour_e = 0;


void stop_animations() {
    current_animation = ANIM_NONE; 
    sleep_ms(5);
}

static inline uint8_t scale8_floor(uint8_t v, uint8_t scale) {
    return (uint8_t)(((uint16_t)v * (uint16_t)scale) / 255u);
}

static inline uint8_t apply_deadband(uint8_t v) {
#if WS2812_OFF_DEADBAND > 0
    return (v < WS2812_OFF_DEADBAND) ? 0 : v;
#else
    return v;
#endif
}


uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    r = apply_deadband(scale8_floor(r, brightness));
    g = apply_deadband(scale8_floor(g, brightness));
    b = apply_deadband(scale8_floor(b, brightness));

    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

volatile uint32_t anim_clear = 0;


uint32_t get_led_colour_value(uint8_t colour_code, uint8_t brightness) {
    switch (colour_code) {
        case 0x00: return urgb_u32(0x00, 0xF2, 0xF2, brightness);
        case 0x01: return urgb_u32(0x8A, 0x2B, 0xE2, brightness);
        case 0x02: return urgb_u32(0x00, 0x40, 0xFF, brightness);
        case 0x03: return urgb_u32(0x00, 0x11, 0x4F, brightness);
        case 0x05: return urgb_u32(0xBF, 0x00, 0xFF, brightness);
        case 0x06: return urgb_u32(0xB5, 0x7E, 0xED, brightness);
        case 0x08: return urgb_u32(0xFF, 0x69, 0xB4, brightness); 
        case 0x0F: return urgb_u32(0xFF, 0xA5, 0x00, brightness);
        case 0x10: return urgb_u32(0xFF, 0xD7, 0x00, brightness);
        case 0x12: return urgb_u32(0x00, 0xFF, 0x66, brightness);
        case 0x13: return urgb_u32(0xFF, 0x66, 0x00, brightness);
        case 0x14: return urgb_u32(0xFF, 0x22, 0x00, brightness);
        case 0x15: return urgb_u32(0xFF, 0x00, 0x00, brightness);
        case 0x19: return urgb_u32(0x00, 0xFF, 0x00, brightness);
        case 0x1A: return urgb_u32(0x7C, 0xFC, 0x00, brightness);
        case 0x1B: return urgb_u32(0xFF, 0xFF, 0xFF, brightness);
        case 0x1D: return urgb_u32(0x00, 0x00, 0x00, brightness); 
        case 0x1F: {
            uint8_t valid_colours[] = {
                0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x08, 
                0x0F, 0x10, 0x12, 0x13, 0x14, 0x15, 0x19, 
                0x1A, 0x1B
            };
            uint8_t random_index = rand() % (sizeof(valid_colours) / sizeof(valid_colours[0]));
            return get_led_colour_value(valid_colours[random_index], brightness);
        }
        default:   return urgb_u32(0x00, 0x00, 0x00, brightness);
    }
}



#define IS_RGBW false



static PIO pio;
static uint sm ;
static uint offset;
static bool leds_ready = false;
static uint32_t pixelcolours[NUM_PIXELS];


static uint BottomLeft[] = {0,4,5,6,7,8};
static uint BottomRight[] = {1,2,3,11,12,13};
static uint TopRight[] = {22,23,24,25,26,30};
static uint TopLeft[] = {17,18,19,27,28,29};
static uint Centre[] = {9,10,14,15,16,20,21};


static bool put_pixel(uint32_t pixel_grb) {
    if (!leds_ready) {
        return false;
    }

    absolute_time_t deadline = make_timeout_time_ms(2);
    while (pio_sm_is_tx_fifo_full(pio, sm)) {
        if (time_reached(deadline)) {
            return false;
        }
    }

    pio_sm_put(pio, sm, pixel_grb << 8u);
    return true;
}
void commit_pattern() {
    for (uint i = 0; i < NUM_PIXELS; ++i) {
        if (!put_pixel(pixelcolours[i])) {
            return;
        }
    }
}

void pattern_clear() {
    stop_animations();
    for (uint i = 0; i < NUM_PIXELS; ++i) {
        pixelcolours[i] = anim_clear;
    }
    commit_pattern();
}

void section(uint32_t colour, uint mask){
    switch (mask) {
        case MASK_CENTRE:
            for (uint i = 0; i < sizeof(Centre) / sizeof(Centre[0]); ++i) {
                pixelcolours[Centre[i]] = colour;}
            break;
        case MASK_TOP_RIGHT:
            for (uint i = 0; i < sizeof(TopRight) / sizeof(TopRight[0]); ++i) {
                pixelcolours[TopRight[i]] = colour;}
            break;
        case MASK_BOTTOM_RIGHT:
            for (uint i = 0; i < sizeof(BottomRight) / sizeof(BottomRight[0]); ++i) {
                pixelcolours[BottomRight[i]] = colour;}
            break;
        case MASK_BOTTOM_LEFT:
            for (uint i = 0; i < sizeof(BottomLeft) / sizeof(BottomLeft[0]); ++i) {
                pixelcolours[BottomLeft[i]] = colour;
            }
            break;
        case MASK_TOP_LEFT:
            for (uint i = 0; i < sizeof(TopLeft) / sizeof(TopLeft[0]); ++i) {
                pixelcolours[TopLeft[i]] = colour;
            }
            break;
    }
}


void colour_fill(uint32_t colour_code, uint mask) {
    printf("Filling colour 0x%08X with mask 0x%02X\n", colour_code, mask);
    uint32_t colour = get_led_colour_value(colour_code, brightness);
    if (mask == MASK_ALL) {
        for (uint pixel = 0; pixel < NUM_PIXELS; ++pixel) {
            pixelcolours[pixel] = colour;
        }
    } else {
        for (uint pixel = 0; pixel < NUM_PIXELS; ++pixel) {
            pixelcolours[pixel] = anim_clear;
        }
        section(colour, mask);
    }
    commit_pattern();
}
void custom_colour_fill(uint8_t red, uint8_t green, uint8_t blue, uint mask) {
    uint32_t colour = urgb_u32(red, green, blue, brightness);
    if (mask == MASK_ALL) {
        for (uint pixel = 0; pixel < NUM_PIXELS; ++pixel) {
            pixelcolours[pixel] = colour;
        }
    } else {
        for (uint pixel = 0; pixel < NUM_PIXELS; ++pixel) {
            pixelcolours[pixel] = anim_clear;
        }
        section(colour, mask);
    }
    commit_pattern();
}

void section_fill(uint32_t centrecolour, uint32_t toprightcolour, uint32_t bottomrightcolour, uint32_t topleftcolour, uint32_t bottomleftcolour) {
    section(get_led_colour_value(centrecolour, brightness), MASK_CENTRE);
    section(get_led_colour_value(toprightcolour, brightness), MASK_TOP_RIGHT);
    section(get_led_colour_value(bottomrightcolour, brightness), MASK_BOTTOM_RIGHT);
    section(get_led_colour_value(topleftcolour, brightness), MASK_TOP_LEFT);
    section(get_led_colour_value(bottomleftcolour, brightness), MASK_BOTTOM_LEFT);
    commit_pattern();
}


void animation_worker() {
    while (true) {
        switch (current_animation) {
            case ANIM_CROSSFADE: {
                uint32_t colA = anim_colour_a;
                uint32_t colB = anim_colour_b;
                uint8_t speed = anim_speed > 0 ? anim_speed : 10;
                
                int total_steps = 50;
                
                while (current_animation == ANIM_CROSSFADE) {

                    uint8_t ar = (colA >> 8) & 0xFF;
                    uint8_t ag = (colA >> 16) & 0xFF;
                    uint8_t ab = (colA & 0xFF);
                    
                    uint8_t br = (colB >> 8) & 0xFF;
                    uint8_t bg = (colB >> 16) & 0xFF;
                    uint8_t bb = (colB & 0xFF);
                    
                    for (int step = 0; step <= total_steps && current_animation == ANIM_CROSSFADE; step++) {
                        uint8_t cr_centre = ar + ((int16_t)br - ar) * step / total_steps;
                        uint8_t cg_centre = ag + ((int16_t)bg - ag) * step / total_steps;
                        uint8_t cb_centre = ab + ((int16_t)bb - ab) * step / total_steps;
                        uint32_t colour_centre = ((uint32_t)cr_centre << 8) | ((uint32_t)cg_centre << 16) | cb_centre;
                        
                        uint8_t cr_outer = br + ((int16_t)ar - br) * step / total_steps;
                        uint8_t cg_outer = bg + ((int16_t)ag - bg) * step / total_steps;
                        uint8_t cb_outer = bb + ((int16_t)ab - bb) * step / total_steps;
                        uint32_t colour_outer = ((uint32_t)cr_outer << 8) | ((uint32_t)cg_outer << 16) | cb_outer;
                        
                        for (uint i = 0; i < sizeof(Centre) / sizeof(Centre[0]); ++i) {
                            pixelcolours[Centre[i]] = colour_centre;
                        }
                        
                        uint *outer_groups[] = {TopRight, BottomRight, TopLeft, BottomLeft};
                        for (int g = 0; g < 4; g++) {
                            for (uint i = 0; i < 6; ++i) {
                                pixelcolours[outer_groups[g][i]] = colour_outer;
                            }
                        }
                        
                        commit_pattern();
                        
                        for (int w = 0; w < speed; w += 5) {
                            if (current_animation != ANIM_CROSSFADE) break;
                            sleep_ms(5+speed/5);
                        }
                    }
                    
                    uint32_t temp = colA;
                    colA = colB;
                    colB = temp;
                }
                break;
            }
            
            case ANIM_RAINBOW: {
                while (current_animation == ANIM_RAINBOW) {
                    for (int i = 0; i < 4 && current_animation == ANIM_RAINBOW; ++i) {
                        switch (i) {
                            case 0: section_fill(0x15, 0x02, 0x19, 0x0F, 0x01); break;
                            case 1: section_fill(0x15, 0x0F, 0x02, 0x01, 0x19); break;
                            case 2: section_fill(0x15, 0x01, 0x0F, 0x19, 0x02); break;
                            case 3: section_fill(0x15, 0x19, 0x01, 0x02, 0x0F); break;
                        }
                        

                        int wait_ms = anim_speed > 0 ? anim_speed : 50;
                        for (int w = 0; w < wait_ms; w += 5) {
                            if (current_animation != ANIM_RAINBOW) break;
                            sleep_ms(5+anim_speed/5);
                        }
                    }
                }
                break;
            }

            case ANIM_CIRCLE: {
                while (current_animation == ANIM_CIRCLE) {
                    for (int i = 0; i < 4 && current_animation == ANIM_CIRCLE; ++i) {
                        switch (i) {
                            case 0: section_fill(0x1D, 0x1B, 0x1D, 0x1D, 0x1D); break;
                            case 1: section_fill(0x1D, 0x1D, 0x1B, 0x1D, 0x1D); break;
                            case 2: section_fill(0x1D, 0x1D, 0x1D, 0x1D, 0x1B); break;
                            case 3: section_fill(0x1D, 0x1D, 0x1D, 0x1B, 0x1D); break;
                        }
                        
                        int wait_ms = anim_speed > 0 ? anim_speed : 50;
                        for (int w = 0; w < wait_ms; w += 5) {
                            if (current_animation != ANIM_CIRCLE) break;
                            sleep_ms(5+anim_speed/5);
                        }
                    }
                }
                break;
            }

            case ANIM_CORNERS: {
                while (current_animation == ANIM_CORNERS) {
                    for (int i = 0; i < 2 && current_animation == ANIM_CORNERS; ++i) {
                        switch (i) {
                            case 0: section_fill(anim_colour_c, anim_colour_a, 0x1D, 0x1D, anim_colour_a); break;
                            case 1: section_fill(anim_colour_c, 0x1D, anim_colour_b, anim_colour_b, 0x1D); break;

                        }

                        int wait_ms = anim_speed > 0 ? anim_speed : 50;
                        for (int w = 0; w < wait_ms; w += 5) {
                            if (current_animation != ANIM_CORNERS) break;
                            sleep_ms(5+anim_speed);
                        }
                    }
                }
                break;
            }

            case ANIM_FLICKER: {
                while (current_animation == ANIM_FLICKER) {
                    for (int i = 0; i < 5 && current_animation == ANIM_FLICKER; ++i) {
                       uint8_t random_index = rand() % 5;
                       switch (i)
                       {
                       case 0:
                        if (random_index == 0) {
                           section(anim_colour_c, MASK_CENTRE);}
                        else{
                           section(anim_clear, MASK_CENTRE);}
                        
                        break;
                       case 1:
                        if (random_index == 0) {
                           section(anim_colour_a, MASK_TOP_RIGHT);}
                        else{
                           section(anim_clear, MASK_TOP_RIGHT);}
                        
                        break;
                        case 2:
                        if (random_index == 0) {
                           section(anim_colour_b, MASK_BOTTOM_RIGHT);}
                        else{
                           section(anim_clear, MASK_BOTTOM_RIGHT);}
                        
                        break;
                        case 3:
                        if (random_index == 0) {
                           section(anim_colour_c, MASK_TOP_LEFT);}
                        else{
                           section(anim_clear, MASK_TOP_LEFT);}
                        
                        break;
                       case 4:
                        if (random_index == 0) {
                           section(anim_colour_d, MASK_BOTTOM_LEFT);}
                        else{
                           section(anim_clear, MASK_BOTTOM_LEFT);}
                        
                        break;
                       default:
                        break;
                       }
                       

                       
                    }
                    commit_pattern();
                    sleep_ms(anim_speed > 0 ? anim_speed : 50);
                }
                break;
            }

            case ANIM_NONE:
            default:
                sleep_ms(10);
                break;
        }
    }
}

void start_crossfade_animation(uint8_t code_a, uint8_t code_b, uint8_t speed) {
    stop_animations();
    anim_colour_a = get_led_colour_value(code_a, brightness);
    anim_colour_b = get_led_colour_value(code_b, brightness);
    anim_speed = speed;
    current_animation = ANIM_CROSSFADE;
}

void start_rainbow_animation(uint8_t speed) {
    stop_animations();
    anim_speed = speed;
    current_animation = ANIM_RAINBOW;
}

void start_circle_animation(uint8_t speed) {
    stop_animations();
    anim_speed = speed;
    current_animation = ANIM_CIRCLE;
}

void start_corners_animation(uint8_t speed, uint8_t centre, uint8_t corner_a, uint8_t corner_b) {
    anim_colour_c = get_led_colour_value(centre, brightness);
    anim_colour_a = get_led_colour_value(corner_a, brightness);
    anim_colour_b = get_led_colour_value(corner_b, brightness);
    stop_animations();
    anim_speed = speed;
    current_animation = ANIM_CORNERS;
}

void start_flicker_animation(uint8_t speed, uint8_t centre, uint8_t topright, uint8_t bottomright, uint8_t topleft, uint8_t bottomleft) {
    anim_colour_c = get_led_colour_value(centre, brightness);
    anim_colour_a = get_led_colour_value(topright, brightness);
    anim_colour_b = get_led_colour_value(bottomright, brightness);
    anim_colour_d = get_led_colour_value(topleft, brightness);
    anim_colour_e = get_led_colour_value(bottomleft, brightness);
    stop_animations();
    anim_speed = speed;
    current_animation = ANIM_FLICKER;
}

void initleds(uint8_t WS2812_PIN, uint num_pixels,uint8_t BRIGHTNESS) {
    printf("LED init: start\n");
    anim_clear = urgb_u32(0, 0, 0, BRIGHTNESS);
    leds_ready = false;
    pio = pio0;
    int claimed_sm = pio_claim_unused_sm(pio, false);
    if (claimed_sm < 0) {
        printf("LED init failed: no free PIO state machine\n");
        return;
    }
    sm = (uint)claimed_sm;

    if (!pio_can_add_program(pio, &ws2812_program)) {
        printf("LED init failed: no room for ws2812 program in PIO instruction memory\n");
        pio_sm_unclaim(pio, sm);
        return;
    }
    brightness = BRIGHTNESS;
    offset = pio_add_program(pio, &ws2812_program);
    printf("LED init: program loaded\n");
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    printf("LED init: state machine configured\n");
    leds_ready = true;
    pattern_clear();
    printf("LED init: complete\n");
    multicore_launch_core1(animation_worker);
}