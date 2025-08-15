#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
void incr_8bit(uint8_t *reg, uint8_t *flag_reg);
void decr_8bit(uint8_t *reg, uint8_t *flag_reg);
void incr_16bit(uint16_t *reg, uint8_t *cycles);
void decr_16bit(uint16_t *reg, uint8_t *cycles);
void incr_mem8(uint16_t addr, uint8_t *flag_reg); 
void load_8bit(uint8_t *reg, uint8_t data);
void add_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg);
void add_16bit(uint16_t *reg, uint16_t data, uint8_t *flag_reg, uint8_t *cycles);
void adc_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg);
void sub_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg);
void sbc_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg);
void and_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg);
void or_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg);
void xor_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg);
void cp_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg);
void rst_inst(uint16_t *PC, uint16_t *SP, uint8_t *cycles, uint16_t newPC);
void pop_inst(uint16_t *reg, uint16_t *SP, uint8_t *cycles);
void push_inst(uint16_t *reg, uint16_t *SP, uint8_t *cycles);
void ret_inst(uint16_t *PC, uint16_t *SP, uint8_t *cycles, uint8_t *flag_reg);

