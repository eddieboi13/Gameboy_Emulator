#include "memory.h"
#include "gameboy.h"
#include "ppu.h"
#include "flags.h"
#include "joypad.h"
#include "audio.h"

uint8_t memory[0x10000];
uint8_t *full_rom = NULL;
long rom_size = 0;
uint8_t mbc_type = 0;
uint8_t current_rom_bank = 1;
bool ram_enabled = false;
extern Registers registers;
extern JoypadState joypad;
extern uint8_t ppu_mode; // 0=HBlank,1=VBlank,2=OAM,3=VRAM

static inline void do_oam_dma(uint8_t high_byte) {
    uint16_t src = ((uint16_t)high_byte) << 8;   // XX00
    for (int i = 0; i < 160; i++) {
        // Copy from system memory to OAM
        // Use direct memory[] so we don't accidentally apply access blocks
        memory[0xFE00 + i] = memory[src + i];
    }
}

void init_memory() {
    memset(memory, 0, sizeof(memory));
    memory[JOYPAD_REGISTER] = 0xCF;
    memory[TIMA_REGISTER] = 0x00;
    memory[TMA_REGISTER] = 0x00;
    memory[TAC_REGISTER] = 0x00;
    memory[C1_SWEEP_REGISTER] = 0x80;
    memory[C1_TIMER_REGISTER] = 0xBF;
    memory[C1_VOLUME_REGISTER] = 0xF3;
    memory[C1_HIGH_REGISTER] = 0xBF;
    memory[C2_TIMER_REGISTER] = 0x3F;
    memory[C2_VOLUME_REGISTER] = 0x00;
    memory[C2_VOLUME_REGISTER] = 0xBF;
    memory[C3_DAC_REGISTER] = 0x7F;
    memory[C3_TIMER_REGISTER] = 0xFF;
    memory[C3_OUTPUT_REGISTER] = 0x9F;
    memory[C3_HIGH_REGISTER] = 0xBF;
    memory[C4_TIMER_REGISTER] = 0xFF;
    memory[C4_VOLUME_REGISTER] = 0x00;
    memory[C4_FREQ_REGISTER] = 0x00;
    memory[C4_CONTROL_REGISTER] = 0xBF;
    memory[MASTER_VOLUME_REGISTER] = 0x77;
    memory[SOUND_PANNING_REGISTER] = 0xF3;
    memory[AUDIO_MASTER_REGISTER] = 0xF1;
    memory[LCDC_REGISTER] = 0x91;
    memory[SCY_REGISTER] = 0x00;
    memory[SCX_REGISTER] = 0x00;
    memory[LYC_REGISTER] = 0x00; 
    memory[BGP_REGISTER] = 0xFC;
    memory[OGP_REGISTER] = 0xFF;
    memory[OGBP1_REGISTER] = 0xFF;
    memory[WY_REGISTER] = 0x00;
    memory[WX_REGISTER] = 0x00;
    memory[IE_REGISTER] = 0x00; 
}

size_t load_rom(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open ROM file: %s\n", path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    rom_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    full_rom = malloc(rom_size);
    if (!full_rom) {
        fprintf(stderr, "Failed to allocate memory for ROM\n");
        exit(1);
    }
    size_t bytes_read = fread(full_rom, 1, rom_size, f);
    if (bytes_read != rom_size) {
        fprintf(stderr, "Failed to read full ROM\n");
        exit(1);
    }
    memcpy(memory, full_rom, 0x4000);
    memcpy(&memory[0x4000], &full_rom[0x4000], 0x4000);

    fclose(f);
    printf("ROM loaded: %ld bytes\n", rom_size);
    return bytes_read;
}

uint8_t read_byte(uint16_t addr){
    if(addr >= 0x4000 && addr <= 0x7FFF){
    	if(mbc_type >= 1){
        	uint32_t offset = (current_rom_bank * 0x4000) + (addr - 0x4000);
	        return full_rom[offset];
    	}
	}
	if(addr <= 0x7FFF) return memory[addr];
    
	// VRAM (blocked in Mode 3)
    else if(addr >= 0x8000 && addr <= 0x9FFF){
        if(ppu_mode == 3) return 0xFF;
        return memory[addr];
    }
    // External RAM
    
	else if(addr >= 0xA000 && addr <= 0xBFFF) return memory[addr];
    
	// WRAM
    else if(addr >= 0xC000 && addr <= 0xDFFF) return memory[addr];
    
    // Echo RAM
    else if(addr >= 0xE000 && addr <= 0xFDFF) return memory[addr - 0x2000];
    
	// OAM (blocked in Modes 2 & 3)
    else if (addr >= 0xFE00 && addr <= 0xFE9F){
		if(ppu_mode == 2 || ppu_mode == 3) return 0xFF;
        return memory[addr];
    }
    //LY
	else if(addr == LY_REGISTER) return memory[addr];
    
	// I/O
    else if(addr >= 0xFF00 && addr <= 0xFF7F){
        if(addr == JOYPAD_REGISTER){
			uint8_t joyp_val = memory[JOYPAD_REGISTER];
       //Button being pressed means bit is set to 0
        	if((joyp_val & 0x10) == 0) {
            	if(!joypad.right) joyp_val &= ~0x01;
            	if(!joypad.left)  joyp_val &= ~0x02;
            	if(!joypad.up)    joyp_val &= ~0x04;
            	if(!joypad.down)  joyp_val &= ~0x08;
        	}
        
        	if((joyp_val & 0x20) == 0) {
            	if(!joypad.a)      joyp_val &= ~0x01;
            	if(!joypad.b)      joyp_val &= ~0x02;
            	if(!joypad.select) joyp_val &= ~0x04;
            	if(!joypad.start)  joyp_val &= ~0x08;
        	}
        
        	return joyp_val;
		}
		return memory[addr];
    }
    // HRAM
    else if(addr >= 0xFF80 && addr <= 0xFFFE) return memory[addr];
    
	// IE
    else if(addr == IE_REGISTER) return memory[addr];
    
    // Unused
    else{
        return 0xFF;
    }
}

void write_byte(uint16_t addr, uint8_t value) {
    static bool first_serial = true;
    if(addr >= 0x0000 && addr <= 0x1FFF){
		if(mbc_type == 1 || mbc_type == 2 || mbc_type == 3){
       		if ((value & 0x0F) == 0x0A){
         		ram_enabled = true;
	        } 
			else{
            	ram_enabled = false;
            }
        }
		if(mbc_type == 5 || mbc_type == 6){
			if((addr & 0x0100) == 0){
         		if((value & 0x0F) == 0x0A){
          	    	ram_enabled = true;
	            } 
				else{
                	ram_enabled = false;
            	}
        	}
		}
		if(mbc_type >= 0x0F && mbc_type <= 0x13){
        	if ((value & 0x0F) == 0x0A){
    	        ram_enabled = true;
            } 
            else{                    
				ram_enabled = false;
            }
        }
	}
	if(addr >= 0x2000 && addr <= 0x3FFF){
    	if(mbc_type == 1 || mbc_type == 2 || mbc_type == 3){
        	uint8_t bank = value & 0x1F;
	        if(bank == 0){
            	bank = 1;
        	}
        	current_rom_bank = (current_rom_bank & 0xE0) | bank;
    	    return;
	    }
		if(mbc_type == 5 || mbc_type == 6){
			if((addr & 0x0100) != 0){
				uint8_t bank = value & 0x0F;
            	if(bank == 0){
                	bank = 1;
	            }
				current_rom_bank = bank;
			}
		}
		if(mbc_type >= 0x0F && mbc_type <= 0x13){
			uint8_t bank = value & 0x7F;
                if(bank == 0){
                    bank = 1;
                }
                current_rom_bank = bank;
		}
	}	
	if(addr >= 0x4000 && addr <= 0x5FFF && mbc_type == 0x01) current_rom_bank |= (value & 0xC0);
	if(addr <= 0x7FFF) return;

    // VRAM (blocked in Mode 3)
    else if(addr >= 0x8000 && addr <= 0x9FFF) {
        if(ppu_mode == 3) {
            return;
        }
        memory[addr] = value;
    }

    // External RAM
    else if(addr >= 0xA000 && addr <= 0xBFFF && ram_enabled) memory[addr] = value;

    // WRAM
    else if(addr >= 0xC000 && addr <= 0xDFFF) memory[addr] = value;

    // Echo RAM mirror
    else if(addr >= 0xE000 && addr <= 0xFDFF) memory[addr - 0x2000] = value;

    // OAM (blocked in Modes 2 & 3)
    else if(addr >= 0xFE00 && addr <= 0xFE9F) {
        if(ppu_mode == 2 || ppu_mode == 3) {
            return;
        }
        memory[addr] = value;
    }

    // I/O
    else if(addr >= 0xFF00 && addr <= 0xFF7F) {
		if(addr == JOYPAD_REGISTER) {
            memory[JOYPAD_REGISTER] = (memory[JOYPAD_REGISTER] & 0xCF) | (value & 0x30);
			return;
        }

        // DIV — any write resets to 0
        if(addr == 0xFF04) {
            memory[addr] = 0;
            return;
        }

        if(addr == TIMA_REGISTER || addr == TMA_REGISTER || addr == TAC_REGISTER) {
            memory[addr] = value;
            return;
        }

        if (addr == C1_HIGH_REGISTER && (value & 0x80)) {
        apu.ch1_triggered = true;
    }
    // Channel 2 Trigger (NR24)
    if (addr == C2_HIGH_REGISTER && (value & 0x80)) {
        apu.ch2_triggered = true;
    }
    // Channel 3 Trigger (NR34)
    if (addr == C3_HIGH_REGISTER && (value & 0x80)) {
        apu.ch3_triggered = true;
    }
    // Channel 4 Trigger (NR44)
    if (addr == C4_CONTROL_REGISTER && (value & 0x80)) {
        apu.ch4_triggered = true;
    }

		if(addr == STAT_REGISTER) {
            uint8_t hw_low = memory[addr] & 0x07;        // mode + coincidence bit are HW maintained
            memory[addr] = (value & 0x78) | hw_low;
            check_lyc_interrupt();
            return;
        }

        if(addr == LY_REGISTER) {
            memory[addr] = 0;
            check_lyc_interrupt();
            return;
        }

        if(addr == LYC_REGISTER) {
            memory[addr] = value;
            check_lyc_interrupt();
            return;
        }

        // OAM DMA (XX00–XX9F -> FE00–FE9F)
        if(addr == DMA_REGISTER) {
            memory[addr] = value;
            do_oam_dma(value);
            return;
        }
        if(addr == IF_REGISTER) {
            memory[addr] = value & 0x1F; //only interested in bit 0-4
            return;
        }

        // LCDC/SCY/SCX/BGP/OBP0/OBP1/WY/WX etc.
        memory[addr] = value;

        // Serial printing for test ROMs
        if(addr == 0xFF02 && value == 0x81) {
            if(first_serial) {
                printf("PC=%04X: ", registers.PC);
                first_serial = false;
            }
            putchar(memory[0xFF01]);
            fflush(stdout);
            if(memory[0xFF01] == '\n') {
                first_serial = true;
            }
        }
    }

    // HRAM
    else if(addr >= 0xFF80 && addr <= 0xFFFE) memory[addr] = value;

    // IE
    else if(addr == IE_REGISTER) memory[addr] = value & 0x1F;

    // Unused
    else {
        return;
    }
}

uint16_t read_word(uint16_t addr){
    return read_byte(addr) | (read_byte(addr + 1) << 8);
}

void write_word(uint16_t addr, uint16_t data){
    write_byte(addr, data & 0xFF);
    write_byte(addr + 1, data >> 8);
}

