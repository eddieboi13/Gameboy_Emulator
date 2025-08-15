#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
// Flags
#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10
#define SET_FLAG(F, FLAG)     ((F) |= (FLAG))
#define CLEAR_FLAG(F, FLAG)   ((F) &= ~(FLAG))
#define GET_FLAG(F, FLAG)     (((F) & (FLAG)) != 0)
#define UPDATE_FLAG(F, FLAG, condition) \
    do { if (condition) SET_FLAG(F, FLAG); else CLEAR_FLAG(F, FLAG); } while (0)
// Interrupt types
#define INT_VBLANK  0x01  // Bit 0
#define INT_LCD     0x02  // Bit 1  
#define INT_TIMER   0x04  // Bit 2
#define INT_SERIAL  0x08  // Bit 3
#define INT_JOYPAD  0x10  // Bit 4

// Interrupt vectors
#define VBLANK_VECTOR 0x40
#define LCD_VECTOR    0x48
#define TIMER_VECTOR  0x50
#define SERIAL_VECTOR 0x58
#define JOYPAD_VECTOR 0x60


/********* Memory Addresses *********/

// Interrupt Registers
#define IE_REGISTER 0xFFFF  // Interrupt Enable
#define IF_REGISTER 0xFF0F  // Interrupt Flag

// Timer Registers
#define DIV_REGISTER  0xFF04
#define TIMA_REGISTER 0xFF05
#define TMA_REGISTER  0xFF06
#define TAC_REGISTER  0xFF07

// Input Registers
#define JOYPAD_REGISTER 0xFF00

// Video Registers
#define LCDC_REGISTER  0xFF40
#define STAT_REGISTER  0xFF41
#define SCY_REGISTER   0xFF42
#define SCX_REGISTER   0xFF43
#define LY_REGISTER    0xFF44
#define LYC_REGISTER   0xFF45
#define DMA_REGISTER   0xFF46
#define BGP_REGISTER   0xFF47
#define OGP_REGISTER   0xFF48
#define OGBP1_REGISTER 0xFF49
#define WY_REGISTER    0xFF4A
#define WX_REGISTER    0xFF4B

// Audio Registers
#define AUDIO_MASTER_REGISTER     0xFF26
#define SOUND_PANNING_REGISTER    0xFF25
#define MASTER_VOLUME_REGISTER    0xFF24
// Channel 1 (Pulse with period sweep)
#define C1_SWEEP_REGISTER   0xFF10
#define C1_TIMER_REGISTER   0xFF11
#define C1_VOLUME_REGISTER  0xFF12
#define C1_LOW_REGISTER     0xFF13
#define C1_HIGH_REGISTER    0xFF14
// Channel 2 (Pulse)
#define C2_TIMER_REGISTER   0xFF16
#define C2_VOLUME_REGISTER  0xFF17
#define C2_LOW_REGISTER     0xFF18
#define C2_HIGH_REGISTER    0xFF19
// Channel 3 (Wave Output)
#define C3_DAC_REGISTER     0xFF1A
#define C3_TIMER_REGISTER   0xFF1B
#define C3_OUTPUT_REGISTER  0xFF1C
#define C3_LOW_REGISTER     0xFF1D
#define C3_HIGH_REGISTER    0xFF1E
// Channel 4 (Noise)
#define C4_TIMER_REGISTER   0xFF20
#define C4_VOLUME_REGISTER  0xFF21
#define C4_FREQ_REGISTER    0xFF22
#define C4_CONTROL_REGISTER 0xFF23

