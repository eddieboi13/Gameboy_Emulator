// ppu.c
#include "ppu.h"
#include "memory.h"
#include "flags.h"
#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
uint8_t ppu_mode = 0;
uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

typedef struct {
    int mode_clock;
    int line;
    int mode;
} PPUState;

// Sprite structure for sorting
typedef struct {
    uint8_t y, x, tile, attr;
    int oam_index;
} Sprite;

static PPUState ppu;

void ppu_set_ly(uint8_t ly) {
    memory[LY_REGISTER] = ly;
	check_lyc_interrupt();
}
static uint8_t read_lcdc() { return read_byte_ppu(LCDC_REGISTER); }

void request_interrupt(uint8_t bit) {
    uint8_t iflag = read_byte_ppu(IF_REGISTER);
    write_byte(IF_REGISTER, iflag | bit);
}

void ppu_init() {
    memset(&ppu, 0, sizeof(ppu));
    memset(framebuffer, 0xFFFFFFFF, sizeof(framebuffer));
	ppu_mode = ppu.mode;
}

uint8_t read_byte_ppu(uint16_t addr){
	if(addr <= 0x7FFF){ 
        return memory[addr];  // ROM
    }
    else if(addr >= 0x8000 && addr <= 0x9FFF){
		return memory[addr];  // VRAM
    }
    else if(addr >= 0xA000 && addr <= 0xBFFF){
        return memory[addr];  // External RAM (cartridge)
    }
    else if(addr >= 0xC000 && addr <= 0xDFFF){
        return memory[addr];  // Work RAM
    }
    else if(addr >= 0xE000 && addr <= 0xFDFF){
        return memory[addr - 0x2000];  // Echo RAM (mirrors 0xC000-0xDDFF)
    }
    else if(addr >= 0xFE00 && addr <= 0xFE9F){
		return memory[addr];  // OAM (Object Attribute Memory)
    }
	
	else if(addr >= 0xFF00 && addr <= 0xFF7F){
        return memory[addr];  // I/O Registers
    }
    else if(addr >= 0xFF80 && addr <= 0xFFFE){
        return memory[addr];  // High RAM (HRAM)
    }
    else if(addr == IE_REGISTER){
        return memory[addr];  // Interrupt Enable Register
    }
    else{
        return 0xFF;  // Unused areas
    }
}

static int window_line_counter = 0;
// Buffer to store the color index (0-3) of each BG/Win pixel for priority checks.
static uint8_t bg_pixel_priority_buffer[SCREEN_WIDTH];

static uint32_t get_color_from_palette(uint8_t color_num, uint8_t palette) {
    uint8_t color_bits = (palette >> (color_num * 2)) & 0x03;
    switch (color_bits) {
        case 0: return 0xFFFFFFFF; // White
        case 1: return 0xFFAAAAAA; // Light Gray
        case 2: return 0xFF555555; // Dark Gray
        case 3: return 0xFF000000; // Black
    }
    return 0xFFFFFFFF;
}

static int compare_sprites(const void *a, const void *b) {
    const Sprite *sprite_a = (const Sprite *)a;
    const Sprite *sprite_b = (const Sprite *)b;
    if (sprite_a->x != sprite_b->x) {
        return sprite_a->x - sprite_b->x;
    }
    return sprite_a->oam_index - sprite_b->oam_index;
}

static void draw_scanline() {
    uint8_t lcdc = read_lcdc();
    if(!(lcdc & 0x80)) return; // LCD disabled, don't draw

    uint8_t ly = read_byte_ppu(LY_REGISTER);
    // Reset window counter at the start of a new frame
    if(ly == 0) window_line_counter = 0;

    uint8_t scy = read_byte_ppu(SCY_REGISTER);
    uint8_t scx = read_byte_ppu(SCX_REGISTER);
    uint8_t bg_palette = read_byte_ppu(BGP_REGISTER);
    uint8_t wy = read_byte_ppu(WY_REGISTER);
    uint8_t wx = read_byte_ppu(WX_REGISTER);

    uint16_t bg_tile_map_addr = (lcdc & 0x08) ? 0x9C00 : 0x9800;
    uint16_t window_tile_map_addr = (lcdc & 0x40) ? 0x9C00 : 0x9800;
    bool unsigned_mode = (lcdc & 0x10) != 0;
    uint16_t tile_data_base = unsigned_mode ? 0x8000 : 0x9000;

    bool bg_win_enabled = (lcdc & 0x01) != 0;
    if(bg_win_enabled) {
        for(int x = 0; x < SCREEN_WIDTH; x++) {
            uint8_t scroll_x = (x + scx) & 0xFF;
            uint8_t scroll_y = (ly + scy) & 0xFF;
            uint16_t tile_x = scroll_x / 8;
            uint16_t tile_y = scroll_y / 8;
            uint16_t tile_index_addr = bg_tile_map_addr + tile_y * 32 + tile_x;
            uint8_t tile_index = read_byte_ppu(tile_index_addr);
            uint16_t tile_addr = unsigned_mode ? (tile_data_base + tile_index * 16) : (tile_data_base + ((int8_t)tile_index) * 16);
            uint8_t line_in_tile = scroll_y % 8;
            uint8_t lo = read_byte_ppu(tile_addr + (line_in_tile * 2));
            uint8_t hi = read_byte_ppu(tile_addr + (line_in_tile * 2) + 1);
            int bit = 7 - (scroll_x % 8);
            uint8_t color_num = ((hi >> bit) & 1) << 1 | ((lo >> bit) & 1);
            
            bg_pixel_priority_buffer[x] = color_num; // Store color index for sprite priority
            framebuffer[ly * SCREEN_WIDTH + x] = get_color_from_palette(color_num, bg_palette);
        }
    } 
	else{
        for(int x = 0; x < SCREEN_WIDTH; x++){
            framebuffer[ly * SCREEN_WIDTH + x] = 0xFFFFFFFF;
            bg_pixel_priority_buffer[x] = 0;
        }
    }

    bool window_visible_on_line = (lcdc & 0x20) && bg_win_enabled && (ly >= wy);
    if(window_visible_on_line) {
        int w_x_eff = wx - 7;
        bool window_drew_pixels = false;
        for(int x = w_x_eff; x < SCREEN_WIDTH; x++) {
            if (x < 0) continue;

            window_drew_pixels = true;
            int window_x = x - w_x_eff;
            int window_y = window_line_counter;
            uint16_t tile_x = window_x / 8;
            uint16_t tile_y = window_y / 8;
            uint16_t tile_index_addr = window_tile_map_addr + tile_y * 32 + tile_x;
            uint8_t tile_index = read_byte_ppu(tile_index_addr);
            uint16_t tile_addr = unsigned_mode ? (tile_data_base + tile_index * 16) : (tile_data_base + ((int8_t)tile_index) * 16);
            uint8_t line_in_tile = window_y % 8;
            uint8_t lo = read_byte_ppu(tile_addr + (line_in_tile * 2));
            uint8_t hi = read_byte_ppu(tile_addr + (line_in_tile * 2) + 1);
            int bit = 7 - (window_x % 8);
            uint8_t color_num = ((hi >> bit) & 1) << 1 | ((lo >> bit) & 1);

            bg_pixel_priority_buffer[x] = color_num;
            framebuffer[ly * SCREEN_WIDTH + x] = get_color_from_palette(color_num, bg_palette);
        }
        if(window_drew_pixels) {
            window_line_counter++;
        }
    }

    if(lcdc & 0x02) {
        bool use_8x16 = (lcdc & 0x04) != 0;
        int sprite_height = use_8x16 ? 16 : 8;
        Sprite sprites_on_line[10];
        int sprite_count = 0;
        bool wx_quirk_active = window_visible_on_line && wx < 7;

        for(int i = 0; i < 40 && sprite_count < 10; i++) {
            uint16_t oam_addr = 0xFE00 + i * 4;
            uint8_t y = read_byte_ppu(oam_addr) - 16;
            uint8_t raw_x = read_byte_ppu(oam_addr + 1);
            
            if(raw_x == 0) continue;
            uint8_t x = raw_x - 8;
            
            if(wx_quirk_active && x < 8 && wx > raw_x) continue;
            if(ly >= y && ly < (y + sprite_height)) {
                sprites_on_line[sprite_count].y = y;
                sprites_on_line[sprite_count].x = x;
                uint8_t tile = read_byte_ppu(oam_addr + 2);
                sprites_on_line[sprite_count].tile = use_8x16 ? (tile & 0xFE) : tile;
                sprites_on_line[sprite_count].attr = read_byte_ppu(oam_addr + 3);
                sprites_on_line[sprite_count].oam_index = i;
                sprite_count++;
            }
        }

        if(sprite_count > 1) {
            qsort(sprites_on_line, sprite_count, sizeof(Sprite), compare_sprites);
        }
        
        for(int s = sprite_count - 1; s >= 0; s--) {
            Sprite* sprite = &sprites_on_line[s];
            bool behind_bg = (sprite->attr & 0x80);
            uint8_t palette_addr = (sprite->attr & 0x10) ? 0xFF49 : 0xFF48;
            uint8_t sprite_palette = read_byte_ppu(palette_addr);
            bool x_flip = (sprite->attr & 0x20);
            bool y_flip = (sprite->attr & 0x40);

            int line_in_sprite = ly - sprite->y;
            uint8_t tile_to_use = sprite->tile;

            if(use_8x16) {
                if((y_flip && line_in_sprite < 8) || (!y_flip && line_in_sprite >= 8)) {
                    tile_to_use = sprite->tile + 1; // Use bottom tile
                }
            }
            
            int line_in_tile = line_in_sprite % 8;
            if(y_flip) {
                line_in_tile = 7 - line_in_tile;
            }

            uint16_t tile_addr = 0x8000 + (tile_to_use * 16) + (line_in_tile * 2);
            uint8_t lo = read_byte_ppu(tile_addr);
            uint8_t hi = read_byte_ppu(tile_addr + 1);

            for(int px = 0; px < 8; px++) {
                int screen_x = sprite->x + px;
                if(screen_x < 0 || screen_x >= SCREEN_WIDTH) continue;

                int bit = x_flip ? px : 7 - px;
                uint8_t color_num = ((hi >> bit) & 1) << 1 | ((lo >> bit) & 1);

                if(color_num == 0) continue;

                if(behind_bg && bg_win_enabled && bg_pixel_priority_buffer[screen_x] != 0) {
                    continue;
                }
                
                framebuffer[ly * SCREEN_WIDTH + screen_x] = get_color_from_palette(color_num, sprite_palette);
            }
        }
    }
}



void update_ppu(uint16_t cycles) {
    static bool lcd_enabled_last = false;
    static FILE *ppu_log = NULL;
    
    if(!ppu_log) {
        ppu_log = fopen("ppu.log", "w");
        if(!ppu_log) {
            fprintf(stderr, "Failed to open ppu.log for writing\n");
            exit(1);
        }
    }

    uint8_t lcdc = memory[0xFF40];
    bool lcd_enabled = lcdc & 0x80;

    // Detect LCD enable change
    if(lcd_enabled != lcd_enabled_last) {
        if(lcd_enabled) {
            fprintf(ppu_log, "[PPU] LCD ENABLED at LY=%d\n", memory[0xFF44]);
            fflush(ppu_log);
            // Reset PPU state like real hardware
            ppu.mode_clock = 0;
            ppu.mode = 2; // OAM scan
            memory[0xFF44] = 0; // LY reset
            memory[0xFF41] = (memory[0xFF41] & 0xFC) | 0x02;
            if(memory[0xFF41] & (1 << 5))
                request_interrupt(INT_LCD);
            check_lyc_interrupt();
        } 
		else{
            fprintf(ppu_log, "[PPU] LCD DISABLED at LY=%d\n", memory[0xFF44]);
            fflush(ppu_log);
        }
    }

    // Handle LCD disabled state
    if(!lcd_enabled) {
        ppu.mode_clock = 0;
        ppu.mode = 0;
        memory[0xFF44] = 0;
        memory[0xFF41] = (memory[0xFF41] & 0xFC);
        lcd_enabled_last = lcd_enabled;
        return;
    }

    // --- Normal mode cycle ---
    ppu.mode_clock += cycles;
    uint8_t ly = memory[0xFF44];
    uint8_t stat = memory[0xFF41];

    switch(ppu.mode) {
        case 2: // OAM Scan
            if (ppu.mode_clock >= 80) {
                ppu.mode_clock -= 80;
                ppu.mode = 3;
                memory[0xFF41] = (stat & 0xFC) | 0x03;
            }
            break;
            
        case 3: // Drawing
            if (ppu.mode_clock >= 172) {
                ppu.mode_clock -= 172;
                ppu.mode = 0;
                memory[0xFF41] = (stat & 0xFC);
                draw_scanline();
                // HBlank interrupt
                if (stat & (1 << 3))
                    request_interrupt(INT_LCD);
            }
            break;
            
        case 0: // HBlank
            if (ppu.mode_clock >= 204) {
                ppu.mode_clock -= 204;
                ly++;
                memory[0xFF44] = ly;
				check_lyc_interrupt();
                
                if (ly == 144) {
                    // Enter VBlank
                    fprintf(ppu_log, "[PPU] ENTERING VBLANK: IF before:%02X IE:%02X LY=%d\n",
                           read_byte(IF_REGISTER), read_byte(IE_REGISTER), ly);
                    fflush(ppu_log);
					ppu.mode = 1;
                    memory[0xFF41] = (stat & 0xFC) | 0x01;
                    
                    // Request VBlank interrupt
                    request_interrupt(INT_VBLANK);
                    
                    // VBlank STAT interrupt
                    if (stat & (1 << 4))
                        request_interrupt(INT_LCD);
                        
                    fprintf(ppu_log, "[PPU] VBLANK INTERRUPTS REQUESTED: IF after:%02X\n",
                           read_byte(IF_REGISTER));
                    fflush(ppu_log);
                    
                } else {
                    // Continue to next scanline
                    ppu.mode = 2;
                    memory[0xFF41] = (stat & 0xFC) | 0x02;
                    
                    // OAM interrupt
                    if (stat & (1 << 5)) {
                        request_interrupt(INT_LCD);
                    }
                    check_lyc_interrupt();
                }
            }
            break;
            
        case 1: // VBlank
            if (ppu.mode_clock >= 456) {
                ppu.mode_clock -= 456;
                ly++;
                memory[0xFF44] = ly;
                check_lyc_interrupt();

                if (ly > 153) {
                    // End of VBlank, start new frame
                    ly = 0;
                    memory[0xFF44] = 0;
                    ppu.mode = 2;
                    memory[0xFF41] = (stat & 0xFC) | 0x02;
                    
                    fprintf(ppu_log, "[PPU] FRAME COMPLETE - BACK TO OAM SCAN\n");
                    fflush(ppu_log);
                    
                    // OAM interrupt
                    if (stat & (1 << 5))
                        request_interrupt(INT_LCD);
                    check_lyc_interrupt();
                }
            }
            break;
    }

    ppu_mode = ppu.mode;
    lcd_enabled_last = lcd_enabled;
}




uint32_t* get_framebuffer() {
    return framebuffer;
}

// SDL rendering loop boilerplate
void render_frame(SDL_Renderer *renderer, SDL_Texture *texture) {
    SDL_UpdateTexture(texture, NULL, framebuffer, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}
void draw_test_pattern() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            uint32_t color = ((x / 8) + (y / 8)) % 2 ? 0x000000FF : 0xFFFFFFFF;
            framebuffer[y * SCREEN_WIDTH + x] = color;
        }
    }
}
void check_lyc_interrupt() {
    uint8_t ly = memory[LY_REGISTER];
    uint8_t lyc = memory[LYC_REGISTER];

	if (ly == lyc) {
        memory[STAT_REGISTER] |= (1 << 2);  // Set coincidence flag
        if (memory[STAT_REGISTER] & (1 << 6))  // Coincidence interrupt enabled
			request_interrupt(INT_LCD);
    } else {
        memory[0xFF41] &= ~(1 << 2);  // Clear coincidence flag
    }
}

