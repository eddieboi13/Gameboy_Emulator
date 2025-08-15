//#pragma once
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>
extern uint8_t ppu_mode;
void ppu_init();
void update_ppu(uint16_t cycles);
void check_lyc_interrupt();
void ppu_set_ly(uint8_t ly);
void request_interrupt(uint8_t bit);
uint8_t read_byte_ppu(uint16_t addr);
uint32_t* get_framebuffer();
void render_frame(SDL_Renderer *renderer, SDL_Texture *texture);
void draw_test_pattern();
