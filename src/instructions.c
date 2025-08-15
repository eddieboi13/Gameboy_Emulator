#include "instructions.h"
#include "flags.h"
#include "memory.h"
void incr_8bit(uint8_t *reg, uint8_t *flag_reg){
	UPDATE_FLAG(*flag_reg, FLAG_H, ((*reg & 0xF) + 1) > 0xF);
	(*reg)++;
	UPDATE_FLAG(*flag_reg, FLAG_Z, (*reg == 0));
	CLEAR_FLAG(*flag_reg, FLAG_N);
}

void decr_8bit(uint8_t *reg, uint8_t *flag_reg){
    uint8_t old = *reg;
	(*reg)--;
    UPDATE_FLAG(*flag_reg, FLAG_Z, (*reg == 0));
    SET_FLAG(*flag_reg, FLAG_N);
    UPDATE_FLAG(*flag_reg, FLAG_H, ((*reg & 0x0F) > (old & 0x0F)));	
}

void incr_16bit(uint16_t *reg, uint8_t *cycles){
	(*reg)++;
	*cycles = 8;
}

void decr_16bit(uint16_t *reg, uint8_t *cycles){
	(*reg)--;
     *cycles = 8;
}

void incr_mem8(uint16_t addr, uint8_t *flag_reg) {
    uint8_t temp = read_byte(addr);
    incr_8bit(&temp, flag_reg);
    write_byte(addr, temp);
}

void load_8bit(uint8_t *reg, uint8_t data){
	(*reg) = data;
}

void add_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg){ //really just used for the A register
	uint16_t full = *reg + data;
	UPDATE_FLAG(*flag_reg, FLAG_H, (((*reg & 0xF) + (data & 0xF)) > 0xF));
	*reg += data;
	UPDATE_FLAG(*flag_reg, FLAG_Z, (*reg == 0));
	CLEAR_FLAG(*flag_reg, FLAG_N);
	UPDATE_FLAG(*flag_reg, FLAG_C, (full > 0xFF));
}

void add_16bit(uint16_t *reg, uint16_t data, uint8_t *flag_reg, uint8_t *cycles){
	bool half_carry = ((*reg & 0x0FFF) + (data & 0x0FFF)) > 0x0FFF;
	uint32_t res = 	*reg + data;
    *reg += data;
    CLEAR_FLAG(*flag_reg, FLAG_N);
    UPDATE_FLAG(*flag_reg, FLAG_H, half_carry);
	UPDATE_FLAG(*flag_reg, FLAG_C, res > 0xFFFF);
    *cycles = 8;
}

void adc_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg){
    uint16_t full = *reg + data + GET_FLAG(*flag_reg, FLAG_C);
    UPDATE_FLAG(*flag_reg, FLAG_H, (((*reg & 0xF) + (data & 0xF) + GET_FLAG(*flag_reg, FLAG_C)) > 0xF));
    *reg = full & 0xFF;
    UPDATE_FLAG(*flag_reg, FLAG_Z, (*reg == 0));
    CLEAR_FLAG(*flag_reg, FLAG_N);
    UPDATE_FLAG(*flag_reg, FLAG_C, (full > 0xFF));
}

void sub_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg){
	int16_t full = *reg - data;
	UPDATE_FLAG(*flag_reg, FLAG_H, ((*reg & 0xF) < (data & 0xF)));
	*reg -= data;
	UPDATE_FLAG(*flag_reg, FLAG_Z, (*reg == 0));
    SET_FLAG(*flag_reg, FLAG_N);
    UPDATE_FLAG(*flag_reg, FLAG_C, (full < 0));
}

void sbc_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg){
    int16_t full = *reg - data - GET_FLAG(*flag_reg, FLAG_C);
    UPDATE_FLAG(*flag_reg, FLAG_H, ((*reg & 0xF) < ((data & 0xF) + GET_FLAG(*flag_reg, FLAG_C))));
    *reg = full & 0xFF;
    UPDATE_FLAG(*flag_reg, FLAG_Z, (*reg == 0));
    SET_FLAG(*flag_reg, FLAG_N);
    UPDATE_FLAG(*flag_reg, FLAG_C, (full < 0));
}

void and_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg){//slightly different for (HL)
    *reg &= data;
    UPDATE_FLAG(*flag_reg, FLAG_Z, (*reg == 0));
    CLEAR_FLAG(*flag_reg, FLAG_N);
    SET_FLAG(*flag_reg, FLAG_H);
    CLEAR_FLAG(*flag_reg, FLAG_C);
}

void or_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg){
    *reg |= data;
    UPDATE_FLAG(*flag_reg, FLAG_Z, (*reg == 0));
    CLEAR_FLAG(*flag_reg, FLAG_N);
    CLEAR_FLAG(*flag_reg, FLAG_H);
    CLEAR_FLAG(*flag_reg, FLAG_C);
}

void xor_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg){
    *reg ^= data;
    UPDATE_FLAG(*flag_reg, FLAG_Z, (*reg == 0));
    CLEAR_FLAG(*flag_reg, FLAG_N);
    CLEAR_FLAG(*flag_reg, FLAG_H);
    CLEAR_FLAG(*flag_reg, FLAG_C);
}

void cp_8bit(uint8_t *reg, uint8_t data, uint8_t *flag_reg){
	int16_t res = *reg - data;
	UPDATE_FLAG(*flag_reg, FLAG_Z, (res == 0));
	SET_FLAG(*flag_reg, FLAG_N);
	UPDATE_FLAG(*flag_reg, FLAG_H, ((*reg & 0xF) < (data & 0xF)));
	UPDATE_FLAG(*flag_reg, FLAG_C, (res < 0));
}

void rst_inst(uint16_t *PC, uint16_t *SP, uint8_t *cycles, uint16_t newPC){
	*SP -=2;
	write_byte((*SP)+1, *PC>>8);
	write_byte(*SP, *PC & 0xFF);
	*cycles = 16;
	*PC = newPC;

}

void pop_inst(uint16_t *reg, uint16_t *SP, uint8_t *cycles){
	uint8_t low = read_byte(*SP);
	(*SP)++;
	uint8_t high = read_byte(*SP);
	(*SP)++;
	*reg = (high << 8) | low;
	*cycles =12;
}

void push_inst(uint16_t *reg, uint16_t *SP, uint8_t *cycles){
    (*SP)--;
	write_byte(*SP, (*reg >> 8));
	(*SP)--;
	write_byte(*SP, (*reg & 0xFF));
	*cycles = 16;
}

void ret_inst(uint16_t *PC, uint16_t *SP, uint8_t *cycles, uint8_t *flag_reg){
	if(GET_FLAG(*flag_reg, FLAG_Z)){
		pop_inst(PC, SP, cycles);
		*cycles = 20;
	}
	else{
		*cycles = 8;
	}
}


