//cpu.c
#include "cpu.h"
#include "memory.h"
#include "instructions.h"
#include "cb.h"
#include "flags.h"
#include "ppu.h"
bool halt_bug;
void cpu_init(Registers *registers){
	registers->AF = 0x01B0;
	registers->BC = 0x0013;
	registers->DE = 0x00D8;
	registers->HL = 0x014D;
	registers->PC = 0x0100;
	registers->SP = 0xFFFE;
	registers->ime = false;
	registers->ime_delay = false;
	registers->halted = false;
}

void handle_interrupts(Registers *registers) {
    uint8_t ie = read_byte(IE_REGISTER);
    uint8_t if_reg = read_byte(IF_REGISTER);
    uint8_t triggered = ie & if_reg;

    if(if_reg & 0x1F) {
        if(registers->halted) {
            registers->halted = false;
        }
    }

    if(registers->ime && !registers->ime_delay && triggered) {
        registers->ime = false;
        uint16_t vector = 0;
        uint8_t interrupt_bit = 0;
        if(triggered & INT_VBLANK) {
            vector = VBLANK_VECTOR;
            interrupt_bit = INT_VBLANK;
        } 
		else if(triggered & INT_LCD) {
            vector = LCD_VECTOR;
            interrupt_bit = INT_LCD;
        } 
		else if(triggered & INT_TIMER) {
            vector = TIMER_VECTOR;
            interrupt_bit = INT_TIMER;
        } 
		else if(triggered & INT_SERIAL) {
            vector = SERIAL_VECTOR;
            interrupt_bit = INT_SERIAL;
        } 
		else if(triggered & INT_JOYPAD) {
            vector = JOYPAD_VECTOR;
            interrupt_bit = INT_JOYPAD;
        }
        if(vector != 0) {
            write_byte(IF_REGISTER, if_reg & ~interrupt_bit);
            (registers->SP)--;
            write_byte(registers->SP, registers->PC >> 8);
            (registers->SP)--;
            write_byte(registers->SP, registers->PC & 0xFF);
            registers->PC = vector;        
		}
    }
}


uint16_t fetch(uint16_t *PC) {
    if (halt_bug) {
        halt_bug = false;
        return read_byte(*PC);
    }
	uint8_t val = read_byte(*PC);
    (*PC)++;
	if (val == 0xCB) {
        uint8_t cbcode = read_byte(*PC);
        (*PC)++;
		return (0xCB << 8) | cbcode;
    }

    return val;
}

uint8_t decode_CB(uint16_t *val, Registers *registers){
	uint8_t cycles = 8;
	switch(*val){
		case 0x00:
			cb_rlc(&registers->B, &cycles, &registers->F);
			break;
		case 0x01:
			cb_rlc(&registers->C, &cycles, &registers->F);
			break;
		case 0x02:
			cb_rlc(&registers->D, &cycles, &registers->F);
			break;
		case 0x03:
			cb_rlc(&registers->E, &cycles, &registers->F);
			break;
		case 0x04:
			cb_rlc(&registers->H, &cycles, &registers->F);
			break;
		case 0x05:
			cb_rlc(&registers->L, &cycles, &registers->F);
			break;
		case 0x06:{
    		uint8_t temp = read_byte(registers->HL);
   			cb_rlc(&temp, &cycles, &registers->F);
    		write_byte(registers->HL, temp);
    		cycles = 16;
    		break;
		}
		case 0x07:
			cb_rlc(&registers->A, &cycles, &registers->F);
			break;
		case 0x08:
			cb_rrc(&registers->B, &cycles, &registers->F);
			break;
		case 0x09:
			cb_rrc(&registers->C, &cycles, &registers->F);
			break;
		case 0x0A:
			cb_rrc(&registers->D, &cycles, &registers->F);
			break;
		case 0x0B:
			cb_rrc(&registers->E, &cycles, &registers->F);
			break;
		case 0x0C:
			cb_rrc(&registers->H, &cycles, &registers->F);
			break;
		case 0x0D:
			cb_rrc(&registers->L, &cycles, &registers->F);
			break;
		case 0x0E:{
			uint8_t temp = read_byte(registers->HL);
            cb_rrc(&temp, &cycles, &registers->F);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
        }
		case 0x0F:
			cb_rrc(&registers->A, &cycles, &registers->F);
			break;
		case 0x10:
			cb_rl(&registers->B, &cycles, &registers->F);
			break;
		case 0x11:
			cb_rl(&registers->C, &cycles, &registers->F);
			break;
		case 0x12:
			cb_rl(&registers->D, &cycles, &registers->F);
			break;
		case 0x13:
			cb_rl(&registers->E, &cycles, &registers->F);
			break;
		case 0x14:
			cb_rl(&registers->H, &cycles, &registers->F);
			break;
		case 0x15:
			cb_rl(&registers->L, &cycles, &registers->F);
			break;
		case 0x16:{
            uint8_t temp = read_byte(registers->HL);
            cb_rl(&temp, &cycles, &registers->F);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
        }
		case 0x17:
			cb_rl(&registers->A, &cycles, &registers->F);
			break;
		case 0x18:
			cb_rr(&registers->B, &cycles, &registers->F);
			break;
		case 0x19:
			cb_rr(&registers->C, &cycles, &registers->F);
			break;
		case 0x1A:
			cb_rr(&registers->D, &cycles, &registers->F);
			break;
		case 0x1B:
			cb_rr(&registers->E, &cycles, &registers->F);
			break;
		case 0x1C:
			cb_rr(&registers->H, &cycles, &registers->F);
			break;
		case 0x1D:
			cb_rr(&registers->L, &cycles, &registers->F);
			break;
		case 0x1E:{
            uint8_t temp = read_byte(registers->HL);
            cb_rr(&temp, &cycles, &registers->F);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
        	}
		case 0x1F:
			cb_rr(&registers->A, &cycles, &registers->F);
			break;
		case 0x20:
			cb_sla(&registers->B, &cycles, &registers->F);
			break;
		case 0x21:
			cb_sla(&registers->C, &cycles, &registers->F);
			break;
		case 0x22:
			cb_sla(&registers->D, &cycles, &registers->F);
			break;
		case 0x23:
			cb_sla(&registers->E, &cycles, &registers->F);
			break;
		case 0x24:
			cb_sla(&registers->H, &cycles, &registers->F);
			break;
		case 0x25:
			cb_sla(&registers->L, &cycles, &registers->F);
			break;
		case 0x26:{
            uint8_t temp = read_byte(registers->HL);
            cb_sla(&temp, &cycles, &registers->F);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0x27:
			cb_sla(&registers->A, &cycles, &registers->F);
            break;
		case 0x28:
			cb_sra(&registers->B, &cycles, &registers->F);
            break;
		case 0x29:
			cb_sra(&registers->C, &cycles, &registers->F);
            break;
		case 0x2A:
			cb_sra(&registers->D, &cycles, &registers->F);
            break;
		case 0x2B:
			cb_sra(&registers->E, &cycles, &registers->F);
            break;
		case 0x2C:
			cb_sra(&registers->H, &cycles, &registers->F);
            break;
		case 0x2D:
			cb_sra(&registers->L, &cycles, &registers->F);
            break;
		case 0x2E:{
            uint8_t temp = read_byte(registers->HL);
            cb_sra(&temp, &cycles, &registers->F);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0x2F:
			cb_sra(&registers->A, &cycles, &registers->F);
            break;
		case 0x30:
			cb_swap(&registers->B, &cycles, &registers->F);
			break;
		case 0x31:
            cb_swap(&registers->C, &cycles, &registers->F);
            break;
		case 0x32:
            cb_swap(&registers->D, &cycles, &registers->F);
            break;
		case 0x33:
            cb_swap(&registers->E, &cycles, &registers->F);
            break;
		case 0x34:
            cb_swap(&registers->H, &cycles, &registers->F);
            break;
		case 0x35:
            cb_swap(&registers->L, &cycles, &registers->F);
            break;
		case 0x36:{
            uint8_t temp = read_byte(registers->HL);
            cb_swap(&temp, &cycles, &registers->F);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0x37:
			cb_swap(&registers->A, &cycles, &registers->F);
			break;
		case 0x38:
            cb_srl(&registers->B, &cycles, &registers->F);
			break;
		case 0x39:
            cb_srl(&registers->C, &cycles, &registers->F);
            break;
		case 0x3A:
            cb_srl(&registers->D, &cycles, &registers->F);
            break;
		case 0x3B:
            cb_srl(&registers->E, &cycles, &registers->F);
            break;
		case 0x3C:
            cb_srl(&registers->H, &cycles, &registers->F);
            break;
		case 0x3D:
            cb_srl(&registers->L, &cycles, &registers->F);
            break;
		case 0x3E:{
            uint8_t temp = read_byte(registers->HL);
            cb_srl(&temp, &cycles, &registers->F);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0x3F:
			cb_srl(&registers->A, &cycles, &registers->F);
			break;
		case 0x40:
			cb_bit(0, registers->B, &cycles, &registers->F);
			break;
		case 0x41:
			cb_bit(0, registers->C, &cycles, &registers->F);
			break;
		case 0x42:
			cb_bit(0, registers->D, &cycles, &registers->F);
			break;
		case 0x43:
			cb_bit(0, registers->E, &cycles, &registers->F);
			break;
		case 0x44:
			cb_bit(0, registers->H, &cycles, &registers->F);
			break;
		case 0x45:
			cb_bit(0, registers->L, &cycles, &registers->F);
			break;
		case 0x46:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_bit(0, temp, &cycles, &registers->F);
            cycles = 16;
            break;
            }
		case 0x47:
			cb_bit(0, registers->A, &cycles, &registers->F);
        	break;
		case 0x48:
			cb_bit(1, registers->B, &cycles, &registers->F);
            break;
		case 0x49:
			cb_bit(1, registers->C, &cycles, &registers->F);
            break;
		case 0x4A:
			cb_bit(1, registers->D, &cycles, &registers->F);
            break;
		case 0x4B:
			cb_bit(1, registers->E, &cycles, &registers->F);
            break;
		case 0x4C:
			cb_bit(1, registers->H, &cycles, &registers->F);
            break;
		case 0x4D:
			cb_bit(1, registers->L, &cycles, &registers->F);
            break;
		case 0x4E:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_bit(1, temp, &cycles, &registers->F);
            cycles = 16;
            break;
            }
		case 0x4F:
			cb_bit(1, registers->A, &cycles, &registers->F);
            break;
		case 0x50:
			cb_bit(2, registers->B, &cycles, &registers->F);
            break;
		case 0x51:
			cb_bit(2, registers->C, &cycles, &registers->F);
            break;
		case 0x52:
			cb_bit(2, registers->D, &cycles, &registers->F);
            break;
		case 0x53:
			cb_bit(2, registers->E, &cycles, &registers->F);
            break;
		case 0x54:
			cb_bit(2, registers->H, &cycles, &registers->F);
            break;
		case 0x55:
			cb_bit(2, registers->L, &cycles, &registers->F);
            break;
		case 0x56:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_bit(2, temp, &cycles, &registers->F);
            cycles = 16;
            break;
            }
		case 0x57:
			cb_bit(2, registers->A, &cycles, &registers->F);
            break;
		case 0x58:
			cb_bit(3, registers->B, &cycles, &registers->F);
            break;
		case 0x59:
			cb_bit(3, registers->C, &cycles, &registers->F);
            break;
		case 0x5A:
			cb_bit(3, registers->D, &cycles, &registers->F);
            break;
		case 0x5B:
			cb_bit(3, registers->E, &cycles, &registers->F);
            break;
		case 0x5C:
			cb_bit(3, registers->H, &cycles, &registers->F);
            break;
		case 0x5D:
			cb_bit(3, registers->L, &cycles, &registers->F);
            break;
		case 0x5E:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_bit(3, temp, &cycles, &registers->F);
            cycles = 16;
            break;
            }
		case 0x5F:
			cb_bit(3, registers->A, &cycles, &registers->F);
            break;
		case 0x60:
			cb_bit(4, registers->B, &cycles, &registers->F);
            break;
		case 0x61:
			cb_bit(4, registers->C, &cycles, &registers->F);
            break;
		case 0x62:
			cb_bit(4, registers->D, &cycles, &registers->F);
            break;
		case 0x63:
			cb_bit(4, registers->E, &cycles, &registers->F);
            break;
		case 0x64:
			cb_bit(4, registers->H, &cycles, &registers->F);
            break;
		case 0x65:
			cb_bit(4, registers->L, &cycles, &registers->F);
            break;
		case 0x66:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_bit(4, temp, &cycles, &registers->F);
            cycles = 16;
            break;
            }
		case 0x67:
			cb_bit(4, registers->A, &cycles, &registers->F);
            break;
		case 0x68:
			cb_bit(5, registers->B, &cycles, &registers->F);
            break;
		case 0x69:
			cb_bit(5, registers->C, &cycles, &registers->F);
            break;
		case 0x6A:
			cb_bit(5, registers->D, &cycles, &registers->F);
            break;
		case 0x6B:
			cb_bit(5, registers->E, &cycles, &registers->F);
            break;
		case 0x6C:
			cb_bit(5, registers->H, &cycles, &registers->F);
            break;
		case 0x6D:
			cb_bit(5, registers->L, &cycles, &registers->F);
            break;
		case 0x6E:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_bit(5, temp, &cycles, &registers->F);
            cycles = 16;
            break;
            }
		case 0x6F:
			cb_bit(5, registers->A, &cycles, &registers->F);
            break;
		case 0x70:
			cb_bit(6, registers->B, &cycles, &registers->F);
            break;
		case 0x71:
			cb_bit(6, registers->C, &cycles, &registers->F);
            break;
		case 0x72:
			cb_bit(6, registers->D, &cycles, &registers->F);
            break;
		case 0x73:
			cb_bit(6, registers->E, &cycles, &registers->F);
            break;
		case 0x74:
			cb_bit(6, registers->H, &cycles, &registers->F);
            break;
		case 0x75:
			cb_bit(6, registers->L, &cycles, &registers->F);
            break;
		case 0x76:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_bit(6, temp, &cycles, &registers->F);
            cycles = 16;
            break;
            }
		case 0x77:
			cb_bit(6, registers->A, &cycles, &registers->F);
            break;
		case 0x78:
			cb_bit(7, registers->B, &cycles, &registers->F);
            break;
		case 0x79:
			cb_bit(7, registers->C, &cycles, &registers->F);
            break;
		case 0x7A:
			cb_bit(7, registers->D, &cycles, &registers->F);
            break;
		case 0x7B:
			cb_bit(7, registers->E, &cycles, &registers->F);
            break;
		case 0x7C:
			cb_bit(7, registers->H, &cycles, &registers->F);
            break;
		case 0x7D:
			cb_bit(7, registers->L, &cycles, &registers->F);
            break;
		case 0x7E:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_bit(7, temp, &cycles, &registers->F);
            cycles = 16;
            break;
            }
		case 0x7F:
			cb_bit(7, registers->A, &cycles, &registers->F);
            break;
		case 0x80:
			cb_res(&registers->B, 0, &cycles);
			break;
		case 0x81:
			cb_res(&registers->C, 0, &cycles);
            break;
		case 0x82:
			cb_res(&registers->D, 0, &cycles);
            break;
		case 0x83:
			cb_res(&registers->E, 0, &cycles);
           break;
		case 0x84:
			cb_res(&registers->H, 0, &cycles);
            break;
		case 0x85:
			cb_res(&registers->L, 0, &cycles);
            break;
		case 0x86:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_res(&temp, 0,&cycles);
			write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0x87:
			cb_res(&registers->A, 0, &cycles);
            break;
		case 0x88:
			cb_res(&registers->B, 1, &cycles);
            break;
		case 0x89:
			cb_res(&registers->C, 1, &cycles);
            break;
		case 0x8A:
			cb_res(&registers->D, 1, &cycles);
            break;
		case 0x8B:
			cb_res(&registers->E, 1, &cycles);
            break;
		case 0x8C:
			cb_res(&registers->H, 1, &cycles);
            break;
		case 0x8D:
			cb_res(&registers->L, 1, &cycles);
            break;
		case 0x8E:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_res(&temp, 1,&cycles);
			write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }	
		case 0x8F:
			cb_res(&registers->A, 1, &cycles);
            break;
		case 0x90:
			cb_res(&registers->B, 2, &cycles);
            break;
		case 0x91:
			cb_res(&registers->C, 2, &cycles);
            break;
		case 0x92:
			cb_res(&registers->D, 2, &cycles);
            break;
		case 0x93:
			cb_res(&registers->E, 2, &cycles);
            break;
		case 0x94:
			cb_res(&registers->H, 2, &cycles);
            break;
		case 0x95:
			cb_res(&registers->L, 2, &cycles);
            break;
		case 0x96:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_res(&temp, 2,&cycles);
			write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0x97:
			cb_res(&registers->A, 2, &cycles);
            break;
		case 0x98:	
			cb_res(&registers->B, 3, &cycles);
            break;
		case 0x99:
			cb_res(&registers->C, 3, &cycles);
            break;
		case 0x9A:
			cb_res(&registers->D, 3, &cycles);
            break;
		case 0x9B:
			cb_res(&registers->E, 3, &cycles);
            break;
		case 0x9C:
			cb_res(&registers->H, 3, &cycles);
            break;
		case 0x9D:
			cb_res(&registers->L, 3, &cycles);
            break;
		case 0x9E:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_res(&temp, 3,&cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0x9F:
			cb_res(&registers->A, 3, &cycles);
            break;
		case 0xA0:
			cb_res(&registers->B, 4, &cycles);
            break;
		case 0xA1:
			cb_res(&registers->C, 4, &cycles);
            break;
		case 0xA2:
			cb_res(&registers->D, 4, &cycles);
            break;
		case 0xA3:
			cb_res(&registers->E, 4, &cycles);
            break;
		case 0xA4:
			cb_res(&registers->H, 4, &cycles);
            break;
		case 0xA5:
			cb_res(&registers->L, 4, &cycles);
            break;
		case 0xA6:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_res(&temp, 4, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xA7:
			cb_res(&registers->A, 4, &cycles);
            break;
		case 0xA8:
			cb_res(&registers->B, 5, &cycles);
            break;
		case 0xA9:
			cb_res(&registers->C, 5, &cycles);
            break;
		case 0xAA:
			cb_res(&registers->D, 5, &cycles);
            break;
		case 0xAB:
			cb_res(&registers->E, 5, &cycles);
            break;
		case 0xAC:
			cb_res(&registers->H, 5, &cycles);
            break;
		case 0xAD:
			cb_res(&registers->L, 5, &cycles);
            break;
		case 0xAE:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_res(&temp, 5, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xAF:
			cb_res(&registers->A, 5, &cycles);
            break;
		case 0xB0:
			cb_res(&registers->B, 6, &cycles);
            break;
		case 0xB1:
			cb_res(&registers->C, 6, &cycles);
            break;
		case 0xB2:
			cb_res(&registers->D, 6, &cycles);
            break;
		case 0xB3:
			cb_res(&registers->E, 6, &cycles);
            break;
		case 0xB4:
			cb_res(&registers->H, 6, &cycles);
            break;
		case 0xB5:
			cb_res(&registers->L, 6, &cycles);
            break;
		case 0xB6:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_res(&temp, 6, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xB7:
			cb_res(&registers->A, 6, &cycles);
            break;
		case 0xB8:
			cb_res(&registers->B, 7, &cycles);
            break;
		case 0xB9:
			cb_res(&registers->C, 7, &cycles);
            break;
		case 0xBA:
			cb_res(&registers->D, 7, &cycles);
            break;
		case 0xBB:
			cb_res(&registers->E, 7, &cycles);
            break;
		case 0xBC:
			cb_res(&registers->H, 7, &cycles);
            break;
		case 0xBD:
			cb_res(&registers->L, 7, &cycles);
            break;
		case 0xBE:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_res(&temp, 7, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xBF:
			cb_res(&registers->A, 7, &cycles);
            break;
		case 0xC0:
			cb_set(&registers->B, 0, &cycles);
            break;
		case 0xC1:
			cb_set(&registers->C, 0, &cycles);
            break;
		case 0xC2:
			cb_set(&registers->D, 0, &cycles);
            break;
		case 0xC3:
			cb_set(&registers->E, 0, &cycles);
            break;
		case 0xC4:
			cb_set(&registers->H, 0, &cycles);
            break;
		case 0xC5:
			cb_set(&registers->L, 0, &cycles);
            break;
		case 0xC6:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_set(&temp, 0, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xC7:
			cb_set(&registers->A, 0, &cycles);
            break;
		case 0xC8:
			cb_set(&registers->B, 1, &cycles);
            break;
		case 0xC9:
			cb_set(&registers->C, 1, &cycles);
            break;
		case 0xCA:
			cb_set(&registers->D, 1, &cycles);
            break;
		case 0xCB:
			cb_set(&registers->E, 1, &cycles);
            break;
		case 0xCC:
			cb_set(&registers->H, 1, &cycles);
            break;
		case 0xCD:
			cb_set(&registers->L, 1, &cycles);
            break;
		case 0xCE:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_set(&temp, 1, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xCF:
			cb_set(&registers->A, 1, &cycles);
            break;
		case 0xD0:
			cb_set(&registers->B, 2, &cycles);
            break;
		case 0xD1:
			cb_set(&registers->C, 2, &cycles);
            break;
		case 0xD2:
			cb_set(&registers->D, 2, &cycles);
            break;
		case 0xD3:
			cb_set(&registers->E, 2, &cycles);
            break;
		case 0xD4:
			cb_set(&registers->H, 2, &cycles);
            break;
		case 0xD5:
			cb_set(&registers->L, 2, &cycles);
            break;
		case 0xD6:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_set(&temp, 2, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xD7:
			cb_set(&registers->A, 2, &cycles);
            break;
		case 0xD8:
			cb_set(&registers->B, 3, &cycles);
            break;
		case 0xD9:
			cb_set(&registers->C, 3, &cycles);
            break;
		case 0xDA:
			cb_set(&registers->D, 3, &cycles);
            break;
		case 0xDB:
			cb_set(&registers->E, 3, &cycles);
            break;
		case 0xDC:
			cb_set(&registers->H, 3, &cycles);
            break;
		case 0xDD:
			cb_set(&registers->L, 3, &cycles);
            break;
		case 0xDE:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_set(&temp, 3, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xDF:
			cb_set(&registers->A, 3, &cycles);
            break;
		case 0xE0:
			cb_set(&registers->B, 4, &cycles);
            break;
		case 0xE1:
			cb_set(&registers->C, 4, &cycles);
            break;
		case 0xE2:
			cb_set(&registers->D, 4, &cycles);
            break;
		case 0xE3:
			cb_set(&registers->E, 4, &cycles);
            break;
		case 0xE4:
			cb_set(&registers->H, 4, &cycles);
            break;
		case 0xE5:
			cb_set(&registers->L, 4, &cycles);
            break;
		case 0xE6:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_set(&temp, 4, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xE7:
			cb_set(&registers->A, 4, &cycles);
            break;
		case 0xE8:
			cb_set(&registers->B, 5, &cycles);
            break;
		case 0xE9:
			cb_set(&registers->C, 5, &cycles);
            break;
		case 0xEA:
			cb_set(&registers->D, 5, &cycles);
            break;
		case 0xEB:
			cb_set(&registers->E, 5, &cycles);
            break;
		case 0xEC:
			cb_set(&registers->H, 5, &cycles);
            break;
		case 0xED:
			cb_set(&registers->L, 5, &cycles);
            break;
		case 0xEE:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_set(&temp, 5, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xEF:
			cb_set(&registers->A, 5, &cycles);
            break;
		case 0xF0:
			cb_set(&registers->B, 6, &cycles);
            break;
		case 0xF1:
			cb_set(&registers->C, 6, &cycles);
            break;
		case 0xF2:
			cb_set(&registers->D, 6, &cycles);
            break;
		case 0xF3:
			cb_set(&registers->E, 6, &cycles);
            break;
		case 0xF4:
			cb_set(&registers->H, 6, &cycles);
            break;
		case 0xF5:
			cb_set(&registers->L, 6, &cycles);
            break;
		case 0xF6:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_set(&temp, 6, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xF7:
			cb_set(&registers->A, 6, &cycles);
            break;
		case 0xF8:
			cb_set(&registers->B, 7, &cycles);
            break;
		case 0xF9:
			cb_set(&registers->C, 7, &cycles);
            break;
		case 0xFA:
			cb_set(&registers->D, 7, &cycles);
            break;
		case 0xFB:
			cb_set(&registers->E, 7, &cycles);
            break;
		case 0xFC:
			cb_set(&registers->H, 7, &cycles);
            break;
		case 0xFD:
			cb_set(&registers->L, 7, &cycles);
            break;
		case 0xFE:
			{
            uint8_t temp = read_byte(registers->HL);
            cb_set(&temp, 7, &cycles);
            write_byte(registers->HL, temp);
            cycles = 16;
            break;
            }
		case 0xFF:
			cb_set(&registers->A, 7, &cycles);
            break;

	}
	return cycles;
}
uint8_t decode(uint16_t *val, Registers *registers){
	uint8_t prefix = ((*val & 0xFF00)>>8);
	uint8_t cycles = 4; //CPU Cycles

	uint8_t imm8 = read_byte(registers->PC);
	//These are used for the 
	uint8_t msb = registers->A & 0x80;
	uint8_t lsb = registers->A & 0x01;

	if(prefix == 0x00){
		switch(*val){
			case 0x00:
				break;
			case 0x01:{
				uint8_t low8 = read_byte(registers->PC);
				(registers->PC)++;
				uint8_t high8 = read_byte(registers->PC);
				(registers->PC)++;
				registers->BC = (high8 << 8) | low8;
				cycles = 12; 
				break;
				}
			case 0x02:
				write_byte(registers->BC, registers->A);
				cycles = 8;
				break;
			case 0x03:
				registers->BC++;
				cycles = 8;
				break;
			case 0x04:
				incr_8bit(&registers->B, &registers->F);
				break;
			case 0x05:
				decr_8bit(&registers->B, &registers->F);
				break;
			case 0x06:
				registers->B = imm8;
				(registers->PC)++;
				cycles = 8;
				break;
			case 0x07:
				//rotate left instruction
				CLEAR_FLAG(registers->F, FLAG_Z);
				CLEAR_FLAG(registers->F, FLAG_N);
				CLEAR_FLAG(registers->F, FLAG_H);
				UPDATE_FLAG(registers->F, FLAG_C, (msb != 0));
				registers->F |= (msb << 4);
				registers->A = registers->A << 1;
				registers->A |= msb >> 7;
				break;
			case 0x08:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				write_byte(imm16, (registers->SP) & 0xFF);
				write_byte(imm16+1, ((registers->SP) >> 8));
				cycles = 20;
				break;
				}
			case 0x09:
				add_16bit(&registers->HL, registers->BC, &registers->F, &cycles);
				break;
			case 0x0A:
				registers->A = read_byte(registers->BC);
				cycles = 8;
				break;
			case 0x0B:
				decr_16bit(&registers->BC, &cycles);
				break;
			case 0x0C:
				incr_8bit(&registers->C, &registers->F);
				break;
			case 0x0D:
				decr_8bit(&registers->C, &registers->F);
				break;
			case 0x0E:
				registers->C = imm8;
				cycles = 8;
				(registers->PC)++;
				break;
			case 0x0F:{
				//rotate right instruction
                CLEAR_FLAG(registers->F, FLAG_Z);
                CLEAR_FLAG(registers->F, FLAG_N);
                CLEAR_FLAG(registers->F, FLAG_H);
                UPDATE_FLAG(registers->F, FLAG_C, (lsb == 1));
				registers->A = registers->A >> 1;
                registers->A |= lsb << 7;
				break;
				}
			case 0x10:
				//stop instruction
				registers->halted = true;
				(registers->PC)++;
				break;
			case 0x11:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				registers->DE = imm16;
				cycles = 12;
				break;
				}
			case 0x12:
				write_byte(registers->DE, registers->A);
				cycles = 8;
				break;
			case 0x13:
				incr_16bit(&registers->DE, &cycles);
				break;
			case 0x14:
				incr_8bit(&registers->D, &registers->F);
				break;
			case 0x15:
				decr_8bit(&registers->D, &registers->F);
				break;
			case 0x16:
				registers->D = read_byte(registers->PC);
				(registers->PC)++;
				cycles = 8;
				break;
			case 0x17:{
                CLEAR_FLAG(registers->F, FLAG_Z);
				CLEAR_FLAG(registers->F, FLAG_N);
				CLEAR_FLAG(registers->F, FLAG_H);
				uint8_t prev_c = GET_FLAG(registers->F, FLAG_C);
				UPDATE_FLAG(registers->F, FLAG_C, msb);
                registers->A = registers->A << 1;
                registers->A |= prev_c;
				break;
				}
			case 0x18:
				registers->PC += (int8_t)imm8+1;
				cycles = 12;
				break;
			case 0x19:
				add_16bit(&registers->HL, registers->DE, &registers->F, &cycles);
				break;
			case 0x1A:
				registers->A = read_byte(registers->DE);
				cycles = 8;
				break;
			case 0x1B:
				decr_16bit(&registers->DE, &cycles);
				break;
			case 0x1C:
				incr_8bit(&registers->E, &registers->F);
				break;
			case 0x1D:
				decr_8bit(&registers->E, &registers->F);
				break;
			case 0x1E:
				registers->E = imm8;
				(registers->PC)++;
				cycles = 8;
				break;
			case 0x1F:{
                CLEAR_FLAG(registers->F, FLAG_Z);
				CLEAR_FLAG(registers->F, FLAG_N);
				CLEAR_FLAG(registers->F, FLAG_H);
				uint8_t prev_c = GET_FLAG(registers->F,FLAG_C);
                UPDATE_FLAG(registers->F, FLAG_C,lsb);
                registers->A = registers->A >> 1;
                registers->A |= (prev_c << 7);
				break;
				}
			case 0x20:
				if(!GET_FLAG(registers->F, FLAG_Z)){
					registers->PC += (int8_t)imm8 + 1;
					cycles = 12;
				}
				else{
					registers->PC+=1;
					cycles = 8;
				}
				break;
			case 0x21:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				registers->HL = imm16;
				cycles = 12;
				break;
				}
			case 0x22:
				write_byte(registers->HL, registers->A);
				incr_16bit(&registers->HL, &cycles);
				break;
			case 0x23:
				incr_16bit(&registers->HL, &cycles);
				break;
			case 0x24:
				incr_8bit(&registers->H, &registers->F);
				break;
			case 0x25:
				decr_8bit(&registers->H, &registers->F);
				break;
			case 0x26:
				registers->H = read_byte(registers->PC);
				cycles = 8;
				(registers->PC)++;
				break;
			case 0x27: { // DAA - Decimal Adjust Accumulator
    			uint16_t a = registers->A;
    
    			if (!GET_FLAG(registers->F, FLAG_N)) {
        // After addition
        			if (GET_FLAG(registers->F, FLAG_H) || (a & 0x0F) > 9) {
            			a += 0x06;
        			}
        			if (GET_FLAG(registers->F, FLAG_C) || a > 0x9F) {
            			a += 0x60;
        			}
    			} 
				else {
        // After subtraction
        			if (GET_FLAG(registers->F, FLAG_H)) {
            			a = (a - 6) & 0xFF;
        			}
        			if (GET_FLAG(registers->F, FLAG_C)) {
            			a -= 0x60;
        			}
    			}
    			CLEAR_FLAG(registers->F, FLAG_H);
    			CLEAR_FLAG(registers->F, FLAG_Z);
    			if (a & 0x100) {
        			SET_FLAG(registers->F, FLAG_C);
    			}
    			a &= 0xFF;
    			if (a == 0) {
        			SET_FLAG(registers->F, FLAG_Z);
    			}
    
    			registers->A = (uint8_t)a;
    			break;
			}			
			case 0x28:
				if(GET_FLAG(registers->F, FLAG_Z)){
					registers->PC+= (int8_t)imm8 +1;
					cycles = 12;
				}
				else{
					cycles = 8;
					(registers->PC)++;
				}
				break;
			case 0x29:
				add_16bit(&registers->HL, registers->HL, &registers->F, &cycles);
				break;
			case 0x2A:
				registers->A = read_byte(registers->HL);
				incr_16bit(&registers->HL, &cycles);
				break;
			case 0x2B:
				decr_16bit(&registers->HL, &cycles);
				break;
			case 0x2C:
				incr_8bit(&registers->L, &registers->F);
				break;
			case 0x2D:
				decr_8bit(&registers->L, &registers->F);
				break;
			case 0x2E:
				registers->L = imm8;
				registers->PC++;
				cycles = 8;
				break;
			case 0x2F:
				registers->A = ~(registers->A);
				SET_FLAG(registers->F, FLAG_N);
				SET_FLAG(registers->F, FLAG_H);
				break;
			case 0x30:
				if(!GET_FLAG(registers->F, FLAG_C)){
					(registers->PC)+= (int8_t)imm8+1;
					cycles = 12;
				}
				else{
					(registers->PC)++;
					cycles = 8;
				}
				break;
			case 0x31:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				registers->SP = imm16;
				cycles = 12;
				break;
				}
			case 0x32:
				write_byte(registers->HL, registers->A);
				decr_16bit(&registers->HL, &cycles);
				break;
			case 0x33:
				incr_16bit(&registers->SP, &cycles);
				break;
			case 0x34:
				incr_mem8(registers->HL, &registers->F);
				cycles = 12;
				break;
			case 0x35:{
				uint8_t temp = read_byte(registers->HL);
				decr_8bit(&temp, &registers->F);
				write_byte(registers->HL, temp);
				cycles = 12;
				break;
				}
			case 0x36:
				write_byte(registers->HL, imm8);
				(registers->PC)++;
				cycles = 12;
				break;
			case 0x37:
				CLEAR_FLAG(registers->F, FLAG_N);
				CLEAR_FLAG(registers->F, FLAG_H);
				SET_FLAG(registers->F, FLAG_C);
				break;
			case 0x38:
				if(GET_FLAG(registers->F, FLAG_C)){
                    registers->PC+= (int8_t)imm8 + 1;
                    cycles = 12;
                }
                else{
                    cycles = 8;
                	(registers->PC)++;
				}
                break;	
			case 0x39:
				add_16bit(&registers->HL, registers->SP, &registers->F, &cycles);
				break;
			case 0x3A:
				registers->A = read_byte(registers->HL);
				decr_16bit(&registers->HL, &cycles);
				break;
			case 0x3B:
				decr_16bit(&registers->SP, &cycles);
				break;
			case 0x3C:
				incr_8bit(&registers->A, &registers->F);
				break;
			case 0x3D:
				decr_8bit(&registers->A, &registers->F);
				break;
			case 0x3E:
				registers->A = imm8;
				cycles = 8;
				(registers->PC)++;
				break;
			case 0x3F:
				if(GET_FLAG(registers->F, FLAG_C)){
					CLEAR_FLAG(registers->F, FLAG_C);
				}
				else{
					SET_FLAG(registers->F, FLAG_C);
				}
				CLEAR_FLAG(registers->F, FLAG_N);
                CLEAR_FLAG(registers->F, FLAG_H);
				break;
			case 0x40:
				load_8bit(&registers->B, registers->B);
				break;
			case 0x41:
				load_8bit(&registers->B, registers->C);
				break;
			case 0x42:
				load_8bit(&registers->B, registers->D);
				break;
			case 0x43:
				load_8bit(&registers->B, registers->E);
				break;
			case 0x44:
				load_8bit(&registers->B, registers->H);
				break;
			case 0x45:
				load_8bit(&registers->B, registers->L);
				break;
			case 0x46:
				load_8bit(&registers->B, read_byte(registers->HL));
				cycles = 8;
				break;
			case 0x47:
				load_8bit(&registers->B, registers->A);
				break;
			case 0x48:
				load_8bit(&registers->C, registers->B);
				break;
			case 0x49:
				load_8bit(&registers->C, registers->C);
				break;
			case 0x4A:
				load_8bit(&registers->C, registers->D);
				break;
			case 0x4B:
				load_8bit(&registers->C, registers->E);
				break;
			case 0x4C:
				load_8bit(&registers->C, registers->H);
				break;
			case 0x4D:
				load_8bit(&registers->C, registers->L);
				break;
			case 0x4E:
				load_8bit(&registers->C, read_byte(registers->HL));
				cycles = 8;
				break;
			case 0x4F:
				load_8bit(&registers->C, registers->A);
				break;
			case 0x50:
				load_8bit(&registers->D, registers->B);
				break;
			case 0x51:
				load_8bit(&registers->D, registers->C);
				break;
			case 0x52:
				load_8bit(&registers->D, registers->D);
				break;
			case 0x53:
				load_8bit(&registers->D, registers->E);
				break;
			case 0x54:
				load_8bit(&registers->D, registers->H);
				break;
			case 0x55:
				load_8bit(&registers->D, registers->L);
				break;
			case 0x56:
				load_8bit(&registers->D, read_byte(registers->HL));
				cycles = 8;
				break;
			case 0x57:
				load_8bit(&registers->D, registers->A);
				break;
			case 0x58:
				load_8bit(&registers->E, registers->B);
				break;
			case 0x59:
				load_8bit(&registers->E, registers->C);
				break;
			case 0x5A:
				load_8bit(&registers->E, registers->D);
				break;
			case 0x5B:
				load_8bit(&registers->E, registers->E);
				break;
			case 0x5C:
				load_8bit(&registers->E, registers->H);
				break;
			case 0x5D:
				load_8bit(&registers->E, registers->L);
				break;
			case 0x5E:
				load_8bit(&registers->E, read_byte(registers->HL));
				cycles = 8;
				break;
			case 0x5F:
				load_8bit(&registers->E, registers->A);
				break;
			case 0x60:
				load_8bit(&registers->H, registers->B);
				break;
			case 0x61:
				load_8bit(&registers->H, registers->C);
				break;
			case 0x62:
				load_8bit(&registers->H, registers->D);
				break;
			case 0x63:
				load_8bit(&registers->H, registers->E);
				break;
			case 0x64:
				load_8bit(&registers->H, registers->H);
				break;
			case 0x65:
				load_8bit(&registers->H, registers->L);
				break;
			case 0x66:
				load_8bit(&registers->H, read_byte(registers->HL));
				cycles = 8;
				break;
			case 0x67:
				load_8bit(&registers->H, registers->A);
				break;
			case 0x68:
				load_8bit(&registers->L, registers->B);
				break;
			case 0x69:
				load_8bit(&registers->L, registers->C);
				break;
			case 0x6A:
				load_8bit(&registers->L, registers->D);
				break;
			case 0x6B:
				load_8bit(&registers->L, registers->E);
				break;
			case 0x6C:
				load_8bit(&registers->L, registers->H);
				break;
			case 0x6D:
				load_8bit(&registers->L, registers->L);
				break;
			case 0x6E:
				load_8bit(&registers->L, read_byte(registers->HL));
				cycles = 8;
				break;
			case 0x6F:
				load_8bit(&registers->L, registers->A);
				break;
			case 0x70:
				write_byte(registers->HL, registers->B);
				cycles = 8;
				break;
			case 0x71:
				write_byte(registers->HL, registers->C);
				cycles = 8;
				break;
			case 0x72:
				write_byte(registers->HL, registers->D);
				cycles = 8;
				break;
			case 0x73:
				write_byte(registers->HL, registers->E);
				cycles = 8;
				break;
			case 0x74:
				write_byte(registers->HL, registers->H);
				cycles = 8;
				break;
			case 0x75:
                write_byte(registers->HL, registers->L);
				cycles = 8;
				break;
			case 0x76:{
				//printf("HALT reached: IME=%d, IF=%02X, IE=%02X\n", registers->ime, read_byte(0xFF0F), read_byte(0xFFFF));
				uint8_t IF = read_byte(0xFF0F);
    			uint8_t IE = read_byte(0xFFFF);
				if(!registers->ime && (IF & IE & 0x1F)){
					halt_bug = true;
				}
				else{
					registers->halted = true;
				}
				break;
				}
			case 0x77:
				write_byte(registers->HL, registers->A);
				cycles = 8;
				break;
			case 0x78:
				load_8bit(&registers->A, registers->B);
				break;
			case 0x79:
				load_8bit(&registers->A, registers->C);
				break;
			case 0x7A:
				load_8bit(&registers->A, registers->D);
				break;
			case 0x7B:
				load_8bit(&registers->A, registers->E);
				break;
			case 0x7C:
				load_8bit(&registers->A, registers->H);
				break;
			case 0x7D:
				load_8bit(&registers->A, registers->L);
				break;
			case 0x7E:
				load_8bit(&registers->A, read_byte(registers->HL));
				cycles = 8;
				break;
			case 0x7F:
				load_8bit(&registers->A, registers->A);
				break;
			case 0x80:
				add_8bit(&registers->A, registers->B, &registers->F);
				break;
			case 0x81:
				add_8bit(&registers->A, registers->C, &registers->F);
				break;
			case 0x82:
				add_8bit(&registers->A, registers->D, &registers->F);
				break;
			case 0x83:
				add_8bit(&registers->A, registers->E, &registers->F);
				break;
			case 0x84:
				add_8bit(&registers->A, registers->H, &registers->F);
				break;
			case 0x85:
				add_8bit(&registers->A, registers->L, &registers->F);
				break;
			case 0x86:
				add_8bit(&registers->A, read_byte(registers->HL), &registers->F);
				cycles = 8;
				break;
			case 0x87:
				add_8bit(&registers->A, registers->A, &registers->F);
				break;
			case 0x88:
				adc_8bit(&registers->A, registers->B, &registers->F);
				break;
			case 0x89:
				adc_8bit(&registers->A, registers->C, &registers->F);
				break;
			case 0x8A:
				adc_8bit(&registers->A, registers->D, &registers->F);
				break;
			case 0x8B:
				adc_8bit(&registers->A, registers->E, &registers->F);
				break;
			case 0x8C:
				adc_8bit(&registers->A, registers->H, &registers->F);
				break;
			case 0x8D:
				adc_8bit(&registers->A, registers->L, &registers->F);
				break;
			case 0x8E:
				adc_8bit(&registers->A, read_byte(registers->HL), &registers->F);
				cycles = 8;
				break;
			case 0x8F:
				adc_8bit(&registers->A, registers->A, &registers->F);
				break;
			case 0x90:
				sub_8bit(&registers->A, registers->B, &registers->F);
				break;
			case 0x91:
				sub_8bit(&registers->A, registers->C, &registers->F);
				break;
			case 0x92:
				sub_8bit(&registers->A, registers->D, &registers->F);
				break;
			case 0x93:
				sub_8bit(&registers->A, registers->E, &registers->F);
				break;
			case 0x94:
				sub_8bit(&registers->A, registers->H, &registers->F);
				break;
			case 0x95:
				sub_8bit(&registers->A, registers->L, &registers->F);
				break;
			case 0x96:
				sub_8bit(&registers->A, read_byte(registers->HL), &registers->F);
				cycles = 8;
				break;
			case 0x97:
				sub_8bit(&registers->A, registers->A, &registers->F);
				break;
			case 0x98:
				sbc_8bit(&registers->A, registers->B, &registers->F);
				break;
			case 0x99:
				sbc_8bit(&registers->A, registers->C, &registers->F);
				break;
			case 0x9A:
				sbc_8bit(&registers->A, registers->D, &registers->F);
				break;
			case 0x9B:
				sbc_8bit(&registers->A, registers->E, &registers->F);
				break;
			case 0x9C:
				sbc_8bit(&registers->A, registers->H, &registers->F);
				break;
			case 0x9D:
				sbc_8bit(&registers->A, registers->L, &registers->F);
				break;
			case 0x9E:
				sbc_8bit(&registers->A, read_byte(registers->HL), &registers->F);
				cycles = 8;
				break;
			case 0x9F:
				sbc_8bit(&registers->A, registers->A, &registers->F);
				break;
			case 0xA0:
				and_8bit(&registers->A, registers->B, &registers->F);
				break;
			case 0xA1:
				and_8bit(&registers->A, registers->C, &registers->F);
				break;
			case 0xA2:
				and_8bit(&registers->A, registers->D, &registers->F);
				break;
			case 0xA3:
				and_8bit(&registers->A, registers->E, &registers->F);
				break;
			case 0xA4:
				and_8bit(&registers->A, registers->H, &registers->F);
				break;
			case 0xA5:
				and_8bit(&registers->A, registers->L, &registers->F);
				break;
			case 0xA6:
				and_8bit(&registers->A, read_byte(registers->HL), &registers->F);
				cycles = 8;
				break;
			case 0xA7:
				and_8bit(&registers->A, registers->A, &registers->F);
				break;
			case 0xA8:
				xor_8bit(&registers->A, registers->B, &registers->F);
				break;
			case 0xA9:
				xor_8bit(&registers->A, registers->C, &registers->F);
				break;
			case 0xAA:
				xor_8bit(&registers->A, registers->D, &registers->F);
				break;
			case 0xAB:
				xor_8bit(&registers->A, registers->E, &registers->F);
				break;
			case 0xAC:
				xor_8bit(&registers->A, registers->H, &registers->F);
				break;
			case 0xAD:
				xor_8bit(&registers->A, registers->L, &registers->F);
				break;
			case 0xAE:
				xor_8bit(&registers->A, read_byte(registers->HL), &registers->F);
				cycles = 8;
				break;
			case 0xAF:
				xor_8bit(&registers->A, registers->A, &registers->F);
				break;
			case 0xB0:
				or_8bit(&registers->A, registers->B, &registers->F);
				break;
			case 0xB1:
				or_8bit(&registers->A, registers->C, &registers->F);
				break;
			case 0xB2:
				or_8bit(&registers->A, registers->D, &registers->F);
				break;
			case 0xB3:
				or_8bit(&registers->A, registers->E, &registers->F);
				break;
			case 0xB4:
				or_8bit(&registers->A, registers->H, &registers->F);
				break;
			case 0xB5:
				or_8bit(&registers->A, registers->L, &registers->F);
				break;
			case 0xB6:
				or_8bit(&registers->A, read_byte(registers->HL), &registers->F);
				cycles = 8;
				break;
			case 0xB7:
				or_8bit(&registers->A, registers->A, &registers->F);
				break;
			case 0xB8:
				cp_8bit(&registers->A, registers->B, &registers->F);
				break;
			case 0xB9:
				cp_8bit(&registers->A, registers->C, &registers->F);
				break;
			case 0xBA:
				cp_8bit(&registers->A, registers->D, &registers->F);
				break;
			case 0xBB:
				cp_8bit(&registers->A, registers->E, &registers->F);
				break;
			case 0xBC:
				cp_8bit(&registers->A, registers->H, &registers->F);
				break;
			case 0xBD:
				cp_8bit(&registers->A, registers->L, &registers->F);
				break;
			case 0xBE:
				cp_8bit(&registers->A, read_byte(registers->HL), &registers->F);
				cycles = 8;
				break;
			case 0xBF:
				cp_8bit(&registers->A, registers->A, &registers->F);
				break;
			case 0xC0:
				if(!GET_FLAG(registers->F, FLAG_Z)){
        			pop_inst(&registers->PC, &registers->SP, &cycles);
        			cycles = 20;
    			}           
    			else{       
        			cycles = 8;
    			}
				break;
			case 0xC1:
				pop_inst(&registers->BC, &registers->SP, &cycles);
				break;
			case 0xC2:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				if(!GET_FLAG(registers->F, FLAG_Z)){
                      registers->PC = imm16;
                      cycles = 16;
                 }
                 else{
                      cycles = 12;
                  }
                  break;
			}
			case 0xC3:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				registers->PC = imm16;
				cycles = 16;
				break;
				}
			case 0xC4:{
				 uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;	
				if(!GET_FLAG(registers->F, FLAG_Z)){
					push_inst(&registers->PC, &registers->SP, &cycles);
					registers->PC = imm16;
					cycles = 24;
				}
				else{
					cycles = 12;
				}
				break;
			}
			case 0xC5:
				push_inst(&registers->BC, &registers->SP, &cycles);
				break;
			case 0xC6:
				add_8bit(&registers->A, imm8, &registers->F);
				(registers->PC)++;
				cycles = 8;
				break;
			case 0xC7:
				rst_inst(&registers->PC, &registers->SP, &cycles, 0);
				break;
			case 0xC8:
				if(GET_FLAG(registers->F, FLAG_Z)){
                    pop_inst(&registers->PC, &registers->SP, &cycles);
                    cycles = 20;
                }
                else{
                    cycles = 8;
                }
                break;
			case 0xC9:
				pop_inst(&registers->PC, &registers->SP, &cycles);
                cycles = 16;
				break;
			case 0xCA:{
				 uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				if(GET_FLAG(registers->F, FLAG_Z)){
                      registers->PC = imm16;
                      cycles = 16;
                 }
                 else{
                      cycles = 12;
                  }
                  break;
				  }
			case 0xCB:
				break;//CB Prefix instructions
			case 0xCC:{
				 uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				if(GET_FLAG(registers->F, FLAG_Z)){
                    push_inst(&registers->PC, &registers->SP, &cycles);
                    registers->PC = imm16;
                    cycles = 24;
                }
                else{
                    cycles = 12;
                }
				break;
				}
			case 0xCD:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
                push_inst(&registers->PC, &registers->SP, &cycles);
                registers->PC=imm16;
                cycles = 24;
				break;
				}
			case 0xCE:
				adc_8bit(&registers->A, imm8, &registers->F);
				(registers->PC)++;
				cycles = 8;
				break;
			case 0xCF:
				rst_inst(&registers->PC, &registers->SP, &cycles, 8);
				break;
			case 0xD0:
				if(!GET_FLAG(registers->F, FLAG_C)){
                    pop_inst(&registers->PC, &registers->SP, &cycles);
                    cycles = 20;
                }
                else{
                    cycles = 8;
                }
                break;
			case 0xD1:
				pop_inst(&registers->DE, &registers->SP, &cycles);
				break;
			case 0xD2:{
				 uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				if(!GET_FLAG(registers->F, FLAG_C)){
                      registers->PC = imm16;
                      cycles = 16;
                 }
                 else{
                      cycles = 12;
                  }
                  break;
				  }
			case 0xD3:
				break;
			case 0xD4:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				if(!GET_FLAG(registers->F, FLAG_C)){
                    push_inst(&registers->PC, &registers->SP, &cycles);
                    registers->PC = imm16;
                    cycles = 24;
                }
                else{
                    cycles = 12;
                }
                break;
				}
			case 0xD5:
				push_inst(&registers->DE, &registers->SP, &cycles);
				break;
			case 0xD6:
				sub_8bit(&registers->A, imm8, &registers->F);
				(registers->PC)++;
				cycles = 8;
				break;
			case 0xD7:
				rst_inst(&registers->PC, &registers->SP, &cycles, 16);
				break;
			case 0xD8:
				if(GET_FLAG(registers->F, FLAG_C)){
                    pop_inst(&registers->PC, &registers->SP, &cycles);
                    cycles = 20;
                }
                else{
                    cycles = 8;
                }
                break;
			case 0xD9:
				pop_inst(&registers->PC, &registers->SP, &cycles);
                registers->ime = true;
				cycles = 16;
				break;
			case 0xDA:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				if(GET_FLAG(registers->F, FLAG_C)){
                	registers->PC = imm16;
                    cycles = 16;
                }
                else{
                	cycles = 12;
                }
                break;
				}
			case 0xDB:
				break;
			case 0xDC:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				if(GET_FLAG(registers->F, FLAG_C)){
                    push_inst(&registers->PC, &registers->SP, &cycles);
                    registers->PC = imm16;
                    cycles = 24;
                }
                else{
                    cycles = 12;
                }
                break;
				}
			case 0xDD:
				break;
			case 0xDE:
				sbc_8bit(&registers->A, imm8, &registers->F);
				(registers->PC)++;
				cycles = 8;
				break;
			case 0xDF:
				rst_inst(&registers->PC, &registers->SP, &cycles, 0x18);
				break;
			case 0xE0:
				write_byte(0xFF00 + imm8, registers->A);
				registers->PC+=1;
				cycles = 12;
				break;
			case 0xE1:
				pop_inst(&registers->HL, &registers->SP, &cycles);
				break;
			case 0xE2:
				write_byte(0xFF00 + registers->C, registers->A);
				cycles = 8;
				break;
			case 0xE3:
				break;
			case 0xE4:
				break;
			case 0xE5:
				push_inst(&registers->HL, &registers->SP, &cycles);
				break;
			case 0xE6:
				and_8bit(&registers->A, imm8, &registers->F);
				(registers->PC)++;
				cycles = 8;
				break;
			case 0xE7:
				rst_inst(&registers->PC, &registers->SP, &cycles, 0x20);
				break;
			case 0xE8:{
				int8_t offset = (int8_t)imm8;
    			uint16_t prevSP = registers->SP;
    			registers->SP += offset;
	    		(registers->PC) += 1;
    			cycles = 16;
    			CLEAR_FLAG(registers->F, FLAG_Z);
    			CLEAR_FLAG(registers->F, FLAG_N);
    			UPDATE_FLAG(registers->F, FLAG_H, ((prevSP ^ offset ^ registers->SP) & 0x10));
    			UPDATE_FLAG(registers->F, FLAG_C, ((prevSP ^ offset ^ registers->SP) & 0x100));
    			break;			
				}
			case 0xE9:
				registers->PC = registers->HL;
				break;
			case 0xEA:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				write_byte(imm16, registers->A);
				cycles = 16;
				break;
				}
			case 0xEB:
				break;
			case 0xEC:
				break;
			case 0xED:
				break;
			case 0xEE:
				xor_8bit(&registers->A, imm8, &registers->F);
				(registers->PC)++;
				cycles = 8;
				break;
			case 0xEF:
				rst_inst(&registers->PC, &registers->SP, &cycles, 0x28);
				break;
			case 0xF0:
				load_8bit(&registers->A, read_byte(0xFF00 + imm8));
				(registers->PC) += 1;
				cycles = 12;
				break;
			case 0xF1:
				pop_inst(&registers->AF, &registers->SP, &cycles);
				registers->F &= 0xF0;
				break;
			case 0xF2:
				load_8bit(&registers->A, read_byte(0xFF00 + registers->C));
				cycles = 8;
				break;
			case 0xF3:
				registers->ime = false;
				registers->ime_delay = false;
				break;
			case 0xF4:
				break;
			case 0xF5:
				push_inst(&registers->AF, &registers->SP, &cycles);
				break;
			case 0xF6:
				or_8bit(&registers->A, imm8, &registers->F);
				(registers->PC)++;
				cycles = 8;
				break;
			case 0xF7:
				rst_inst(&registers->PC, &registers->SP, &cycles, 0x30);
				break;
			case 0xF8:{
				registers->HL = (registers->SP)+ (int8_t)imm8;
				CLEAR_FLAG(registers->F, FLAG_Z);
				CLEAR_FLAG(registers->F, FLAG_N);
				UPDATE_FLAG(registers->F, FLAG_H, ((registers->SP & 0xF) + ((int8_t)imm8 & 0xF)) > 0xF);
    			UPDATE_FLAG(registers->F, FLAG_C, ((registers->SP & 0xFF) + ((int8_t)imm8 & 0xFF)) > 0xFF);
				(registers->PC)++;
				cycles = 12;
				break;
			}
			case 0xF9:
				(registers->SP) = registers->HL;
				cycles = 8;
				break;
			case 0xFA:{
				uint8_t low8 = read_byte(registers->PC);
                (registers->PC)++;
                uint8_t high8 = read_byte(registers->PC);
                (registers->PC)++;
                uint16_t imm16 = (high8 << 8) | low8;
				load_8bit(&registers->A, read_byte(imm16));
				cycles = 16;
				break;
				}
			case 0xFB:
				registers->ime_delay = true;
				break;
			case 0xFC:
				break;
			case 0xFD:
				break;
			case 0xFE:
				cp_8bit(&registers->A, imm8, &registers->F);
				(registers->PC)++;
				cycles = 8;
				break;
			case 0xFF:
				rst_inst(&registers->PC, &registers->SP, &cycles, 0x38);
				break;

		}
		return cycles;
	}
	else{
		*val = (*val & 0x00FF); //get lower byte of CB prefixed codes
		return decode_CB(val, registers);
	}

}
