#include "timer.h"
#include "memory.h"
#include "cpu.h"

void update_timers(Timer *timer, int cycles) {
    // Update DIV register (increments at 16384 Hz)
    timer->div_counter += cycles;
    if(timer->div_counter >= 256){
        timer->div_counter -= 256;
        memory[DIV_REGISTER]++;
    }

    // Update TIMA if timer is enabled
    uint8_t tac = read_byte(TAC_REGISTER);
    if(tac & 0x04){ // Timer enabled
        int timer_freq[] = {1024, 16, 64, 256}; // Clock frequencies
        int freq = timer_freq[tac & 0x03];

        timer->timer_counter += cycles;
        while(timer->timer_counter >= freq){
            timer->timer_counter -= freq;
            uint8_t tima = read_byte(TIMA_REGISTER);
            if(tima == 0xFF){
                // Timer overflow - trigger interrupt and reload
                write_byte(TIMA_REGISTER, read_byte(TMA_REGISTER));
                trigger_interrupt(INT_TIMER);
            } 
			else{
                write_byte(TIMA_REGISTER, tima + 1);
            }
        }
    }
}
void trigger_interrupt(uint8_t interrupt_bit) {
    uint8_t if_reg = read_byte(IF_REGISTER);
    write_byte(IF_REGISTER, if_reg | interrupt_bit);
}
