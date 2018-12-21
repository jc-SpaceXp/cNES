/* Reads CPU ROM and execute's what instructions we encounter.
 * Also modify PC and CPU cycles for each opcode. Page boundry
 * calculations occur here except for branch instructions where
 * they are computed in "opcode_functions.c".
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "opcode_functions.h" /* + cpu.h +*/
#include "helper_functions.h"
#include "opcode_table.h"

void cpu_step(uint16_t PC, Cpu6502* CPU)
{
	uint8_t opcode = read_from_cpu(CPU, PC);

	/* Process NMI */
	if (!CPU->cpu_ppu_io->nmi_pending) {
		switch (opcode) {
		case 0x00:
			/* BRK */
			execute_BRK(CPU);
			CPU->PC += 2;
			CPU->Cycle += 7;
			break;
		case 0x01:
			/* ORA - Indirect X mode*/
			get_INDX_address(CPU);
			execute_ORA(INDX, CPU);
			CPU->Cycle += 6;
			break;
		case 0x05:
			/* ORA - Zero page mode */
			get_ZP_offset_address(0, CPU);
			execute_ORA(ZP, CPU);
			CPU->Cycle += 3;
			break;
		case 0x06:
			/* ASL - Zero page mode */
			get_ZP_offset_address(0, CPU);
			execute_ASL(ZP, CPU);
			CPU->Cycle += 5;
			break;
		case 0x08:
			/* PHP */
			// 3 + 10 spaces 
			strcpy(instruction, "PHP");
			stack_push(CPU, CPU->P | 0x30); /* Set Bits 4 & 5 to 1 for PHP */
			++CPU->PC;
			CPU->Cycle += 3;
			break;
		case 0x09:
			/* ORA - Immediate mode */
			get_IMM_byte(CPU);
			execute_ORA(IMM, CPU);
			CPU->Cycle += 2;
			break;
		case 0x0A:
			/* ASL - Accumulator mode */
			execute_ASL(ACC, CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x0D:
			/* ORA - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_ORA(ABS, CPU);
			CPU->Cycle += 4;
			break;
		case 0x0E:
			/* ASL - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_ASL(ABS, CPU);
			CPU->Cycle += 6;
			break;
		case 0x10:
			/* BPL */
			execute_BPL(CPU);
			CPU->Cycle += 2; // Branch not taken, +1 if taken (in execute function)
			break;
		case 0x11:
			/* ORA - Indirect Y mode */
			get_INDY_address(CPU);
			execute_ORA(INDY, CPU);
			CPU->Cycle += 5; // Branch not taken, +1 if taken (in execute function)
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0x15:
			/* ORA - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_ORA(ZPX, CPU);
			CPU->Cycle += 4;
			break;
		case 0x16:
			/* ASL - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_ASL(ZPX, CPU);
			CPU->Cycle += 6;
			break;
		case 0x18:
			/* CLC */
			execute_CLC(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x19:
			/* ORA - Absolute Y mode */
			get_ABS_offset_address(CPU->Y, CPU);
			strcat(end, ",Y");
			execute_ORA(ABSY, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0x1D:
			/* ORA - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_ORA(ABSX, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->X);
			break;
		case 0x1E:
			/* ASL - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_ASL(ABSX, CPU);
			CPU->Cycle += 7;
			break;
		case 0x20:
			/* JSR - Absolute mode*/
			get_ABS_offset_address(0, CPU);
			execute_JSR(CPU);
			CPU->Cycle += 6;
			break;
		case 0x21:
			/* AND - Indirect X mode*/
			get_INDX_address(CPU);
			execute_AND(INDX, CPU);
			CPU->Cycle += 6;
			break;
		case 0x24:
			/* BIT - Zero Page mode*/
			get_ZP_offset_address(0, CPU);
			execute_BIT(CPU);
			CPU->Cycle += 3;
			break;
		case 0x25:
			/* AND - Zero Page mode*/
			get_ZP_offset_address(0, CPU);
			execute_AND(ZP, CPU);
			CPU->Cycle += 3;
			break;
		case 0x26:
			/* ROL - Zero Page mode*/
			get_ZP_offset_address(0, CPU);
			execute_ROL(ZP, CPU);
			CPU->Cycle += 5;
			break;
		case 0x28:
			/* PLP */
			strcpy(instruction, "PLP");
			CPU->P = stack_pull(CPU) & ~(0x10); /* B flag may exist on stack not on P */
			CPU->P |= 0x20; /* bit 5 always set */
			++CPU->PC;
			CPU->Cycle += 4;
			break;
		case 0x29:
			/* AND - Immediate mode*/
			get_IMM_byte(CPU);
			execute_AND(IMM, CPU);
			CPU->Cycle += 2;
			break;
		case 0x2A:
			/* ROL - Accumulator mode*/
			execute_ROL(ACC, CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x2C:
			/* BIT - Absolute mode*/
			get_ABS_offset_address(0, CPU);
			execute_BIT(CPU);
			CPU->Cycle += 4;
			break;
		case 0x2D:
			/* AND - Absolute mode*/
			get_ABS_offset_address(0, CPU);
			execute_AND(ABS, CPU);
			CPU->Cycle += 4;
			break;
		case 0x2E:
			/* ROL - Absolute mode*/
			get_ABS_offset_address(0, CPU);
			execute_ROL(ABS, CPU);
			CPU->Cycle += 6;
			break;
		case 0x30:
			/* BMI */
			execute_BMI(CPU);
			CPU->Cycle += 2; // Branch not taken, additional cycles added in execution function
			break;
		case 0x31:
			/* AND - Indirect Y mode*/
			get_INDY_address(CPU);
			execute_AND(INDY, CPU);
			CPU->Cycle += 5;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0x35:
			/* AND - Zero Page X mode*/
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_AND(ZPX, CPU);
			CPU->Cycle += 4;
			break;
		case 0x36:
			/* ROL - Zero Page X mode*/
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_ROL(ZPX, CPU);
			CPU->Cycle += 6;
			break;
		case 0x38:
			/* SEC */
			execute_SEC(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x39:
			/* AND - Absolute Y mode */
			get_ABS_offset_address(CPU->Y, CPU);
			strcat(end, ",Y");
			execute_AND(ABSY, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0x3D:
			/* AND - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_AND(ABSX, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->X);
			break;
		case 0x3E:
			/* ROL - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_ROL(ABSX, CPU);
			CPU->Cycle += 7;
			break;
		case 0x40:
			/* RTI */
			execute_RTI(CPU); /* PC is pulled from stack */
			CPU->Cycle += 6;
			break;
		case 0x41:
			/* EOR - Indirect X mode */
			get_INDX_address(CPU);
			execute_EOR(INDX, CPU);
			CPU->Cycle += 6;
			break;
		case 0x45:
			/* EOR - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_EOR(ZP, CPU);
			CPU->Cycle += 3;
			break;
		case 0x46:
			/* LSR - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_LSR(ZP, CPU);
			CPU->Cycle += 5;
			break;
		case 0x48:
			/* PHA */
			strcpy(instruction, "PHA");
			stack_push(CPU, CPU->A);
			++CPU->PC;
			CPU->Cycle += 3;
			break;
		case 0x49:
			/* EOR - Immediate mode */
			get_IMM_byte(CPU);
			execute_EOR(IMM, CPU);
			CPU->Cycle += 2;
			break;
		case 0x4A:
			/* LSR - Accumulator */
			execute_LSR(ACC, CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x4C:
			/* JMP - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_JMP(CPU);
			CPU->Cycle += 3;
			break;
		case 0x4D:
			/* EOR - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_EOR(ABS, CPU);
			CPU->Cycle += 4;
			break;
		case 0x4E:
			/* LSR - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_LSR(ABS, CPU);
			CPU->Cycle += 6;
			break;
		case 0x50:
			/* BVC */
			execute_BVC(CPU);
			CPU->Cycle += 2; // Branch no taken, penalties calc in exec function
			break;
		case 0x51:
			/* EOR - Indirect Y mode */
			get_INDY_address(CPU);
			execute_EOR(INDY, CPU);
			CPU->Cycle += 5;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0x55:
			/* EOR - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_EOR(ZPX, CPU);
			CPU->Cycle += 4;
			break;
		case 0x56:
			/* LSR - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_LSR(ZPX, CPU);
			CPU->Cycle += 6;
			break;
		case 0x58:
			/* CLI */
			execute_CLI(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x59:
			/* EOR - Absolute Y mode */
			get_ABS_offset_address(CPU->Y, CPU);
			strcat(end, ",Y");
			execute_EOR(ABSY, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0x5D:
			/* EOR - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_EOR(ABSX, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->X);
			break;
		case 0x5E:
			/* LSR - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_LSR(ABSX, CPU);
			CPU->Cycle += 7;
			break;
		case 0x60:
			/* RTS */
			execute_RTS(CPU); /* PC is pulled from stack */
			CPU->Cycle += 6;
			break;
		case 0x61:
			/* ADC - Indirect X mode */
			get_INDX_address(CPU);
			execute_ADC(INDX, CPU);
			CPU->Cycle += 6;
			break;
		case 0x65:
			/* ADC - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_ADC(ZP, CPU);
			CPU->Cycle += 3;
			break;
		case 0x66:
			/* ROR - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_ROR(ZP, CPU);
			CPU->Cycle += 5;
			break;
		case 0x68:
			/* PLA */
			strcpy(instruction, "PLA");
			CPU->A = stack_pull(CPU);
			update_flag_z(CPU, CPU->A);
			update_flag_n(CPU, CPU->A);
			++CPU->PC;
			CPU->Cycle += 4;
			break;
		case 0x69:
			/* ADC - Immediate mode */
			get_IMM_byte(CPU);
			execute_ADC(IMM, CPU);
			CPU->Cycle += 2;
			break;
		case 0x6A:
			/* ROR - Accumulator mode */
			execute_ROR(ACC, CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x6C:
			/* JMP - Indirect */
			get_IND_address(CPU);
			execute_JMP(CPU);
			CPU->Cycle += 5;
			break;
		case 0x6D:
			/* ADC - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_ADC(ABS, CPU);
			CPU->Cycle += 4;
			break;
		case 0x6E:
			/* ROR - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_ROR(ABS, CPU);
			CPU->Cycle += 6;
			break;
		case 0x70:
			/* BVS */
			execute_BVS(CPU);
			CPU->Cycle += 2; // Branch not taken, penalties in exec function
			break;
		case 0x71:
			/* ADC - Indirect Y mode */
			get_INDY_address(CPU);
			execute_ADC(INDY, CPU);
			CPU->Cycle += 5;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0x75:
			/* ADC - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_ADC(ZPX, CPU);
			CPU->Cycle += 4;
			break;
		case 0x76:
			/* ROR - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_ROR(ZPX, CPU);
			CPU->Cycle += 6;
			break;
		case 0x78:
			/* SEI */
			execute_SEI(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x79:
			/* ADC - Absolute Y mode */
			get_ABS_offset_address(CPU->Y, CPU);
			strcat(end, ",Y");
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			execute_ADC(ABSY, CPU);
			CPU->Cycle += 4;
			break;
		case 0x7D:
			/* ADC - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->X);
			execute_ADC(ABSX, CPU);
			CPU->Cycle += 4;
			break;
		case 0x7E:
			/* ROR - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_ROR(ABSX, CPU);
			CPU->Cycle += 7;
			break;
		case 0x81:
			/* STA - Indirect X mode */
			get_INDX_address(CPU);
			execute_STA(CPU);
			CPU->Cycle += 6;
			break;
		case 0x84:
			/* STY - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_STY(CPU);
			CPU->Cycle += 3;
			break;
		case 0x85:
			/* STA - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_STA(CPU);
			CPU->Cycle += 3;
			break;
		case 0x86:
			/* STX - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_STX(CPU);
			CPU->Cycle += 3;
			break;
		case 0x88:
			/* DEY */
			execute_DEY(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x8A:
			/* TXA */
			execute_TXA(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x8C:
			/* STY - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_STY(CPU);
			CPU->Cycle += 4;
			break;
		case 0x8D:
			/* STA - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_STA(CPU);
			CPU->Cycle += 4;
			break;
		case 0x8E:
			/* STX - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_STX(CPU);
			CPU->Cycle += 4;
			break;
		case 0x90:
			/* BCC */
			execute_BCC(CPU);
			CPU->Cycle += 2; // Branch not taken, penalties in exec function
			break;
		case 0x91:
			/* STA - Indirect Y mode */
			get_INDY_address(CPU);
			execute_STA(CPU);
			CPU->Cycle += 6;
			break;
		case 0x94:
			/* STY - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_STY(CPU);
			CPU->Cycle += 4;
			break;
		case 0x95:
			/* STA - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_STA(CPU);
			CPU->Cycle += 4;
			break;
		case 0x96:
			/* STX - Zero Page Y mode */
			get_ZP_offset_address(CPU->Y, CPU);
			strcat(end, ",Y");
			execute_STX(CPU);
			CPU->Cycle += 4;
			break;
		case 0x98:
			/* TYA */
			execute_TYA(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x99:
			/* STA - Absolute Y mode */
			get_ABS_offset_address(CPU->Y, CPU);
			strcat(end, ",Y");
			execute_STA(CPU);
			CPU->Cycle += 5;
			break;
		case 0x9A:
			/* TXS */
			execute_TXS(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0x9D:
			/* STA - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_STA(CPU);
			CPU->Cycle += 5;
			break;
		case 0xA0:
			/* LDY - Immediate mode */
			get_IMM_byte(CPU);
			execute_LDY(IMM, CPU);
			CPU->Cycle += 2;
			break;
		case 0xA1:
			/* LDA - Indirect X mode */
			get_INDX_address(CPU);
			execute_LDA(INDX, CPU);
			CPU->Cycle += 6;
			break;
		case 0xA2:
			/* LDX - Immediate mode */
			get_IMM_byte(CPU);
			execute_LDX(IMM, CPU);
			CPU->Cycle += 2;
			break;
		case 0xA4:
			/* LDY - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_LDY(ZP, CPU);
			CPU->Cycle += 3;
			break;
		case 0xA5:
			/* LDA - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_LDA(ZP, CPU);
			CPU->Cycle += 3;
			break;
		case 0xA6:
			/* LDX - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_LDX(ZP, CPU);
			CPU->Cycle += 3;
			break;
		case 0xA8:
			/* TAY */
			execute_TAY(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0xA9:
			/* LDA - Immediate mode */
			get_IMM_byte(CPU);
			execute_LDA(IMM, CPU);
			CPU->Cycle += 2;
			break;
		case 0xAA:
			/* TAX */
			execute_TAX(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0xAC:
			/* LDY - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_LDY(ABS, CPU);
			CPU->Cycle += 4;
			break;
		case 0xAD:
			/* LDA - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_LDA(ABS, CPU);
			CPU->Cycle += 4;
			break;
		case 0xAE:
			/* LDX - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_LDX(ABS, CPU);
			CPU->Cycle += 4;
			break;
		case 0xB0:
			/* BCS */
			execute_BCS(CPU);
			CPU->Cycle += 2; // Branch not taken, penalties added in exec function
			break;
		case 0xB1:
			/* LDA - Indirect Y mode */
			get_INDY_address(CPU);
			execute_LDA(INDY, CPU);
			CPU->Cycle += 5;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0xB4:
			/* LDY - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_LDY(ZPX, CPU);
			CPU->Cycle += 4;
			break;
		case 0xB5:
			/* LDA - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_LDA(ZPX, CPU);
			CPU->Cycle += 4;
			break;
		case 0xB6:
			/* LDX - Zero Page Y mode */
			get_ZP_offset_address(CPU->Y, CPU);
			strcat(end, ",Y");
			execute_LDX(ZPX, CPU);
			CPU->Cycle += 4;
			break;
		case 0xB8:
			/* CLV */
			execute_CLV(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0xB9:
			/* LDA - Absolute Y mode */
			get_ABS_offset_address(CPU->Y, CPU);
			strcat(end, ",Y");
			execute_LDA(ABSY, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0xBA:
			/* TSX */
			execute_TSX(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0xBC:
			/* LDY - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_LDY(ABSX, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->X);
			break;
		case 0xBD:
			/* LDA - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_LDA(ABSX, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->X);
			break;
		case 0xBE:
			/* LDX - Absolute Y mode */
			get_ABS_offset_address(CPU->Y, CPU);
			strcat(end, ",Y");
			execute_LDX(ABSY, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0xC0:
			/* CPY - Immediate mode */
			get_IMM_byte(CPU);
			execute_CPY(IMM, CPU);
			CPU->Cycle += 2;
			break;
		case 0xC1:
			/* CMP - Indirect X mode */
			get_INDX_address(CPU);
			execute_CMP(INDX, CPU);
			CPU->Cycle += 6;
			break;
		case 0xC4:
			/* CPY - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_CPY(ZP, CPU);
			CPU->Cycle += 3;
			break;
		case 0xC5:
			/* CMP - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_CMP(ZP, CPU);
			CPU->Cycle += 3;
			break;
		case 0xC6:
			/* DEC - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_DEC(CPU);
			CPU->Cycle += 5;
			break;
		case 0xC8:
			/* INY */
			execute_INY(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0xC9:
			/* CMP - Immediate mode */
			get_IMM_byte(CPU);
			execute_CMP(IMM, CPU);
			CPU->Cycle += 2;
			break;
		case 0xCA:
			/* DEX */
			execute_DEX(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0xCC:
			/* CPY - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_CPY(ABS, CPU);
			CPU->Cycle += 4;
			break;
		case 0xCD:
			/* CMP - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_CMP(ABS, CPU);
			CPU->Cycle += 4;
			break;
		case 0xCE:
			/* DEC - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_DEC(CPU);
			CPU->Cycle += 6;
			break;
		case 0xD0:
			/* BNE */
			execute_BNE(CPU);
			CPU->Cycle += 2;
			break;
		case 0xD1:
			/* CMP - Indirect Y  mode */
			get_INDY_address(CPU);
			execute_CMP(INDY, CPU);
			CPU->Cycle += 5;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0xD5:
			/* CMP - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_CMP(ZPX, CPU);
			CPU->Cycle += 4;
			break;
		case 0xD6:
			/* DEC - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_DEC(CPU);
			CPU->Cycle += 6;
			break;
		case 0xD8:
			/* CLD */
			execute_CLD(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0xD9:
			/* CMP - Absolute Y mode */
			get_ABS_offset_address(CPU->Y, CPU);
			strcat(end, ",Y");
			execute_CMP(ABSY, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0xDD:
			/* CMP - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_CMP(ABSX, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->X);
			break;
		case 0xDE:
			/* DEC - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_DEC(CPU);
			CPU->Cycle += 7;
			break;
		case 0xE0:
			/* CPX - Immediate mode */
			get_IMM_byte(CPU);
			execute_CPX(IMM, CPU);
			CPU->Cycle += 2;
			break;
		case 0xE1:
			/* SBC - Indirect X mode */
			get_INDX_address(CPU);
			execute_SBC(INDX, CPU);
			CPU->Cycle += 6;
			break;
		case 0xE4:
			/* CPX - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_CPX(ZP, CPU);
			CPU->Cycle += 3;
			break;
		case 0xE5:
			/* SBC - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_SBC(ZP, CPU);
			CPU->Cycle += 3;
			break;
		case 0xE6:
			/* INC - Zero Page mode */
			get_ZP_offset_address(0, CPU);
			execute_INC(CPU);
			CPU->Cycle += 5;
			break;
		case 0xE8:
			/* INX */
			execute_INX(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0xE9:
			/* SBC - Immediate mode */
			get_IMM_byte(CPU);
			execute_SBC(IMM, CPU);
			CPU->Cycle += 2;
			break;
		case 0xEA:
			/* NOP */
			execute_NOP();
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0xEC:
			/* CPX - Absolute mode*/
			get_ABS_offset_address(0, CPU);
			execute_CPX(ABS, CPU);
			CPU->Cycle += 4;
			break;
		case 0xED:
			/* SBC - Absolute mode */
			get_ABS_offset_address(0, CPU);
			execute_SBC(ABS, CPU);
			CPU->Cycle += 4;
			break;
		case 0xEE:
			/* INC - Absolute mode*/
			get_ABS_offset_address(0, CPU);
			execute_INC(CPU);
			CPU->Cycle += 6;
			break;
		case 0xF0:
			/* BEQ */
			execute_BEQ(CPU);
			CPU->Cycle += 2; // Branch not taken, +1 if taken (in execute_BEQ)
			break;
		case 0xF1:
			/* SBC - Indirect Y mode */
			get_INDY_address(CPU);
			execute_SBC(INDY, CPU);
			CPU->Cycle += 5;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0xF5:
			/* SBC - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_SBC(ZPX, CPU);
			CPU->Cycle += 4;
			break;
		case 0xF6:
			/* INC - Zero Page X mode */
			get_ZP_offset_address(CPU->X, CPU);
			strcat(end, ",X");
			execute_INC(CPU);
			CPU->Cycle += 6;
			break;
		case 0xF8:
			/* SED */
			execute_SED(CPU);
			++CPU->PC;
			CPU->Cycle += 2;
			break;
		case 0xF9:
			/* SBC - Absolute Y mode */
			get_ABS_offset_address(CPU->Y, CPU);
			strcat(end, ",Y");
			execute_SBC(ABSY, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->Y);
			break;
		case 0xFD:
			/* SBC - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ", X");
			execute_SBC(ABSX, CPU);
			CPU->Cycle += 4;
			CPU->Cycle += page_cross_penalty(CPU->target_addr, CPU->target_addr - CPU->X);
			break;
		case 0xFE:
			/* INC - Absolute X mode */
			get_ABS_offset_address(CPU->X, CPU);
			strcat(end, ", X");
			execute_INC(CPU);
			CPU->Cycle += 7;
			break;
		default:
			/* Invalid command */
			printf("Undocumented opcode: 0x%.2x\t", opcode); // Sits here forever
			break;
		}
	} else {
		execute_NMI(CPU);
		//printf("NMI COUNT: %d\n", ++CPU->delay_nmi);
		CPU->Cycle += 7;
	}

	if (CPU->cpu_ppu_io->dma_pending) {
		execute_DMA();
		if ((CPU->Cycle - 1) & 0x01) {  // add 1 cycle for on odd cycles
			CPU->Cycle += 1;
		}
		CPU->Cycle += 513;
		CPU->cpu_ppu_io->dma_pending = false;
	}
}
