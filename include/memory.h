#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
extern uint8_t memory[0x10000];
void init_memory();
uint8_t read_byte(uint16_t addr);
void write_byte(uint16_t addr, uint8_t data);
uint16_t read_word(uint16_t addr);
void write_word(uint16_t addr, uint16_t data);
size_t load_rom(const char *path);
