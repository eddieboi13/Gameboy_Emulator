//prefix CB instructions
#include "cb.h"
#include "flags.h"
void cb_rlc(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg){
	uint8_t msb = *reg & 0x80;
    *reg = *reg << 1;
    *reg |= msb >> 7;
	UPDATE_FLAG(*flag_reg, FLAG_Z, *reg == 0);
	CLEAR_FLAG(*flag_reg, FLAG_N);
	CLEAR_FLAG(*flag_reg, FLAG_H);
	UPDATE_FLAG(*flag_reg, FLAG_C, msb);
	*cycles = 8;
}

void cb_rrc(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg){
	uint8_t lsb = *reg & 0x01;
    *reg = *reg >> 1;
    *reg |= lsb << 7;
    UPDATE_FLAG(*flag_reg, FLAG_Z, *reg == 0);
    CLEAR_FLAG(*flag_reg, FLAG_N);
    CLEAR_FLAG(*flag_reg, FLAG_H);
    UPDATE_FLAG(*flag_reg, FLAG_C, lsb);
	*cycles = 8;
}

void cb_rl(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg){
	uint8_t msb = *reg & 0x80;
    CLEAR_FLAG(*flag_reg, FLAG_N);
    CLEAR_FLAG(*flag_reg, FLAG_H);
	bool prev_c = GET_FLAG(*flag_reg,FLAG_C);
    UPDATE_FLAG(*flag_reg, FLAG_C, msb);
    *reg = *reg << 1;
    *reg |= prev_c;
	UPDATE_FLAG(*flag_reg, FLAG_Z, *reg == 0);
	*cycles = 8;
}

void cb_rr(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg){
	uint8_t lsb = *reg & 0x01;
    bool prev_c = GET_FLAG(*flag_reg, FLAG_C);
    CLEAR_FLAG(*flag_reg, FLAG_N);
    CLEAR_FLAG(*flag_reg, FLAG_H);
    UPDATE_FLAG(*flag_reg, FLAG_C, lsb);
    *reg = (*reg) >> 1;
	*reg |= (prev_c ? 0x80 : 0x00);
	UPDATE_FLAG(*flag_reg, FLAG_Z, *reg == 0);
	*cycles = 8;
}

void cb_sla(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg){
	uint8_t msb = *reg & 0x80;
    *reg = *reg << 1;
    UPDATE_FLAG(*flag_reg, FLAG_Z, *reg == 0);
    CLEAR_FLAG(*flag_reg, FLAG_N);
    CLEAR_FLAG(*flag_reg, FLAG_H);
    UPDATE_FLAG(*flag_reg, FLAG_C, msb >> 7);
	*cycles = 8;
}

void cb_sra(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg){
	uint8_t lsb = *reg & 0x01;
	uint8_t msb = *reg & 0x80;
	*reg = (*reg >> 1) | msb;
	UPDATE_FLAG(*flag_reg, FLAG_Z, *reg == 0);
    CLEAR_FLAG(*flag_reg, FLAG_N);
    CLEAR_FLAG(*flag_reg, FLAG_H);
    UPDATE_FLAG(*flag_reg, FLAG_C, lsb);

	*cycles = 8;
}

void cb_swap(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg){
	uint8_t upper = (*reg & 0xF0) >> 4;
	uint8_t lower = (*reg & 0x0F) << 4;
	*reg = lower | upper;
	UPDATE_FLAG(*flag_reg, FLAG_Z, *reg == 0);
    CLEAR_FLAG(*flag_reg, FLAG_N);
    CLEAR_FLAG(*flag_reg, FLAG_H);
	CLEAR_FLAG(*flag_reg, FLAG_C);
}

void cb_srl(uint8_t *reg, uint8_t *cycles, uint8_t *flag_reg){
	uint8_t lsb = *reg & 0x01;
    *reg = *reg>>1;
    UPDATE_FLAG(*flag_reg, FLAG_Z, *reg == 0);
    CLEAR_FLAG(*flag_reg, FLAG_N);
    CLEAR_FLAG(*flag_reg, FLAG_H);
    UPDATE_FLAG(*flag_reg, FLAG_C, (lsb != 0));
	*cycles = 8;
}

void cb_bit(uint8_t bit_index, uint8_t reg_value, uint8_t *cycles, uint8_t *flag_reg){
    UPDATE_FLAG(*flag_reg, FLAG_Z, !(reg_value & (1 << bit_index)));
    CLEAR_FLAG(*flag_reg, FLAG_N);
    SET_FLAG(*flag_reg, FLAG_H);
    *cycles = 8;
}

void cb_res(uint8_t *reg, uint8_t data, uint8_t *cycles){
	*reg &= ~(1 << data);
	*cycles = 8;
}

void cb_set(uint8_t *reg, uint8_t data, uint8_t *cycles){
	*reg |= (1 << data);
    *cycles = 8;
}
