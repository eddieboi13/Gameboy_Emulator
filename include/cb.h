#include <stdio.h>
#include <stdint.h>
void cb_rlc(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg);
void cb_rrc(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg);
void cb_rl(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg);
void cb_rr(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg);
void cb_sla(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg);
void cb_sra(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg);
void cb_swap(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg);
void cb_srl(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg);
void cb_bit(uint8_t bit_index, uint8_t reg_value, uint8_t *cycles, uint8_t *flag_reg);
void cb_res(uint8_t *reg, uint8_t data, uint8_t *cycles);
void cb_set(uint8_t *reg, uint8_t data, uint8_t *cycles);
