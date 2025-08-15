#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DIV_REGISTER   0xFF04  // Divider Register
#define TIMA_REGISTER  0xFF05  // Timer Counter
#define TMA_REGISTER   0xFF06  // Timer Modulo
#define TAC_REGISTER   0xFF07
#define IF_REGISTER    0xFF0F
#define IE_REGISTER    0xFFFF  
#define INT_TIMER      0x04

typedef struct {
    uint16_t div_counter;   // Internal counter for DIV
    uint16_t timer_counter; // Internal counter for TIMA
} Timer;
void update_timers(Timer *timer, int cycles);
void trigger_interrupt(uint8_t interrupt_bit);
