#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#include "cpu.h"
#include "memory.h"
#include "timer.h"
#include "ppu.h"
#include "joypad.h"
#include "audio.h"

Registers registers = {0};
Timer timer = {0};
extern JoypadState joypad;

const int CYCLES_PER_FRAME = 70224;  // Total T-cycles for one full frame refresh
const float FRAME_DURATION = 1000.0f / 59.7f; // Target time per frame in milliseconds (~16.7ms)

int main(int argc, char **argv) {
    if(argc < 2){
        fprintf(stderr, "Usage: %s rom.gb\n", argv[0]);
        return 1;
    }

    init_memory();
    load_rom(argv[1]);
      
    cpu_init(&registers);
    ppu_init();


    SDL_Init(SDL_INIT_VIDEO |  SDL_INIT_AUDIO);
    SDL_Window* window = SDL_CreateWindow("Game Boy Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160 * 4, 144 * 4, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
	SDL_AudioSpec want, have;
	SDL_memset(&want, 0, sizeof(want)); 
	want.freq = 44100; 
	want.format = AUDIO_S16SYS; 
	want.channels = 1; 
	want.samples = 512; 
	want.callback = audio_callback; 
	want.userdata = NULL; 
	SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	audio_set_sample_rate(have.freq);
	SDL_PauseAudioDevice(device, 0);


    bool running = true;
    
    while(running) {
        uint32_t frame_start_time = SDL_GetTicks();

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running = false;
            }
			if(event.type == SDL_KEYDOWN){
				update_joyp(&event, true);
			}
			if(event.type == SDL_KEYUP){
				update_joyp(&event, false);
			}
        }

        int cycles_this_frame = 0;
        while(cycles_this_frame < CYCLES_PER_FRAME){
            uint16_t prev_pc = registers.PC;
            handle_interrupts(&registers);
            if(registers.PC != prev_pc){
                int interrupt_cycles = 20;
                update_timers(&timer, interrupt_cycles);
                update_ppu(interrupt_cycles);
				update_audio(interrupt_cycles);
                cycles_this_frame += interrupt_cycles;
                continue;
            }

            if(registers.halted){
                int halt_cycles = 4;
                update_timers(&timer, halt_cycles);
                update_ppu(halt_cycles);
				update_audio(halt_cycles);
                cycles_this_frame += halt_cycles;
                continue;
            }
            
            uint16_t opcode = fetch(&registers.PC);
            uint8_t cycles_executed = decode(&opcode, &registers);

            update_timers(&timer, cycles_executed);
            update_ppu(cycles_executed);
			update_audio(cycles_executed);
            cycles_this_frame += cycles_executed;
            
            if(registers.ime_delay){
                registers.ime = true;
                registers.ime_delay = false;
            }
        }

        render_frame(renderer, texture);

        uint32_t frame_end_time = SDL_GetTicks();
        int time_elapsed = frame_end_time - frame_start_time;

        if(time_elapsed < FRAME_DURATION){
            SDL_Delay(FRAME_DURATION - time_elapsed);
        }

    }

    SDL_PauseAudioDevice(device, 1);
	SDL_CloseAudioDevice(device);
	SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
