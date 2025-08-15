#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "gameboy.h"
uint16_t fetch(uint16_t *PC);
void cpu_init(Registers *registers);
void handle_interrupts(Registers *registers);
uint8_t decode(uint16_t *val, Registers *registers);
uint8_t decode_CB(uint16_t *val, Registers *registers);
