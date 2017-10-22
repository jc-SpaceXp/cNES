/* Reads NES ROM and execute's what instructions we encounter */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "functions.h" /* + cpu.h +*/
#include "functions_generic.h"
#include "opcode_debug.h"

/* CAN'T IMPLEMENT code[NES->PC] until ROM loading is working */
/* one workaround is let NES->PC to = 0 prior to Execute_6502(); */
void Debug_6502(unsigned char *code, uint16_t *PC)
{
	uint8_t *opcode = &code[*PC];

	switch (opcode[0]) {
	case 0x00:
		/* BRK */
		execute_BRK();
		NES->PC += 2;
		break;
	case 0x01:
		/* ORA - Indirect X mode*/
		operand = get_op_INDX(opcode, NES);
		execute_ORA(INDX, operand);
		break;
	case 0x05:
		/* ORA - Zero page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_ORA(ZP, operand);
		break;
	case 0x06:
		/* ASL - Zero page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_ASL(ZP, operand);
		break;
	case 0x08:
		/* PHP */
		printf("PHP\t");
		PUSH(NES->P);
		NES->P |= 0x30; /* Setting Bits 4 & 5 */
		++NES->PC;
		break;
	case 0x09:
		/* ORA - Immediate mode */
		operand = get_op_IMM(opcode);
		execute_ORA(IMM, operand);
		break;
	case 0x0A:
		/* ASL - Accumulator mode */
		execute_ASL(ACC, operand); /* Doesn't use operand in this case */
		++NES->PC;
		break;
	case 0x0D:
		/* ORA - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_ORA(ABS, operand);
		break;
	case 0x0E:
		/* ASL - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_ASL(ABS, operand);
		break;
	case 0x10:
		/* BPL */
		execute_BPL(opcode);
		break;
	case 0x11:
		/* ORA - Indirect Y mode */
		operand = get_op_INDY(opcode, NES);
		execute_ORA(INDY, operand);
		break;
	case 0x15:
		/* ORA - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_ORA(ZPX, operand);
		break;
	case 0x16:
		/* ASL - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_ASL(ZPX, operand);
		break;
	case 0x18:
		/* CLC */
		execute_CLC();
		++NES->PC;
		break;
	case 0x19:
		/* ORA - Absolute Y mode */
		operand = get_op_ABS_offset(opcode, NES->Y);
		execute_ORA(ABSY, operand);
		break;
	case 0x1D:
		/* ORA - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_ORA(ABSX, operand);
		break;
	case 0x1E:
		/* ASL - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_ASL(ABSX, operand);
		break;
	case 0x20:
		/* JSR - Absolute mode*/
		operand = get_op_ABS_offset(opcode, 0);
		execute_JSR(operand);
		break;
	case 0x21:
		/* AND - Indirect X mode*/
		operand = get_op_INDX(opcode, NES);
		execute_AND(INDX, operand);
		break;
	case 0x24:
		/* BIT - Zero Page mode*/
		operand = get_op_ZP_offset(opcode, 0);
		execute_BIT(operand);
		break;
	case 0x25:
		/* AND - Zero Page mode*/
		operand = get_op_ZP_offset(opcode, 0);
		execute_AND(ZP, operand);
		break;
	case 0x26:
		/* ROL - Zero Page mode*/
		operand = get_op_ZP_offset(opcode, 0);
		execute_ROL(ZP, operand);
		break;
	case 0x28:
		/* PLP */
		printf("PLP\t");
		NES->P = PULL();
		++NES->PC;
		break;
	case 0x29:
		/* AND - Immediate mode*/
		operand = get_op_IMM(opcode);
		execute_AND(IMM, operand);
		break;
	case 0x2A:
		/* ROL - Accumulator mode*/
		execute_ROL(ACC, operand); /* operand isn't used in this case */
		++NES->PC;
		break;
	case 0x2C:
		/* BIT - Absolute mode*/
		operand = get_op_ABS_offset(opcode, 0);
		execute_BIT(operand);
		break;
	case 0x2D:
		/* AND - Absolute mode*/
		operand = get_op_ABS_offset(opcode, 0);
		execute_AND(ABS, operand);
		break;
	case 0x2E:
		/* ROL - Absolute mode*/
		operand = get_op_ABS_offset(opcode, 0);
		execute_ROL(ABS, operand);
		break;
	case 0x30:
		/* BMI */
		execute_BMI(opcode);
		break;
	case 0x31:
		/* AND - Indirect Y mode*/
		operand = get_op_INDY(opcode, NES);
		execute_AND(INDY, operand);
		break;
	case 0x35:
		/* AND - Zero Page X mode*/
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_AND(ZPX, operand);
		break;
	case 0x36:
		/* ROL - Zero Page X mode*/
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_ROL(ZPX, operand);
		break;
	case 0x38:
		/* SEC */
		execute_SEC(NES);
		++NES->PC;
		break;
	case 0x39:
		/* AND - Absolute Y mode */
		operand = get_op_ABS_offset(opcode, NES->Y);
		execute_AND(ABSY, operand);
		break;
	case 0x3D:
		/* AND - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_AND(ABSX, operand);
		break;
	case 0x3E:
		/* ROL - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_ROL(ABSX, operand);
		break;
	case 0x40:
		/* RTI */
		execute_RTI();
		/* PC is pulled from stack */
		break;
	case 0x41:
		/* EOR - Indirect X mode */
		operand = get_op_INDX(opcode, NES);
		execute_EOR(INDX, operand);
		break;
	case 0x45:
		/* EOR - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_EOR(ZP, operand);
		break;
	case 0x46:
		/* LSR - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_LSR(ZP, operand);
		break;
	case 0x48:
		/* PHA */
		printf("PHA\t");
		PUSH(NES->A);
		++NES->PC;
		break;
	case 0x49:
		/* EOR - Immediate mode */
		operand = get_op_IMM(opcode);
		execute_EOR(IMM, operand);
		break;
	case 0x4A:
		/* LSR - Accumulator */
		execute_LSR(ACC, operand); /* operand not used */
		++NES->PC;
		break;
	case 0x4C:
		/* JMP - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_JMP(operand);
		break;
	case 0x4D:
		/* EOR - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_EOR(ABS, operand);
		break;
	case 0x4E:
		/* LSR - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_LSR(ABS, operand);
		break;
	case 0x50:
		/* BVC */
		execute_BVC(opcode);
		break;
	case 0x51:
		/* EOR - Indirect Y mode */
		operand = get_op_INDY(opcode, NES);
		execute_EOR(INDY, operand);
		break;
	case 0x55:
		/* EOR - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_EOR(ZPX, operand);
		break;
	case 0x56:
		/* LSR - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_LSR(ZPX, operand);
		break;
	case 0x58:
		/* CLI */
		execute_CLI();
		++NES->PC;
		break;
	case 0x59:
		/* EOR - Absolute Y mode */
		operand = get_op_ABS_offset(opcode, NES->Y);
		execute_EOR(ABSY, operand);
		break;
	case 0x5D:
		/* EOR - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_EOR(ABSX, operand);
		break;
	case 0x5E:
		/* LSR - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_LSR(ABSX, operand);
		break;
	case 0x60:
		/* RTS */
		execute_RTS();
		/* PC is pulled from stack */
		break;
	case 0x61:
		/* ADC - Indirect X mode */
		operand = get_op_INDX(opcode, NES);
		execute_ADC(INDX, operand);
		update_FLAG_V(bin_operand1, bin_operand2, bin_result);
		set_or_clear_CARRY(tmp);
		break;
	case 0x65:
		/* ADC - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_ADC(ZP, operand);
		update_FLAG_V(bin_operand1, bin_operand2, bin_result);
		set_or_clear_CARRY(tmp);
		break;
	case 0x66:
		/* ROR - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_ROR(ZP, operand);
		break;
	case 0x68:
		/* PLA */
		printf("PLA\t");
		NES->A = PULL();
		++NES->PC;
		break;
	case 0x69:
		/* ADC - Immediate mode */
		operand = get_op_IMM(opcode);
		execute_ADC(IMM, operand);
		update_FLAG_V(bin_operand1, bin_operand2, bin_result);
		set_or_clear_CARRY(tmp);
		break;
	case 0x6A:
		/* ROR - Accumulator mode */
		execute_ROR(ACC, operand); /* operand isn't used */
		++NES->PC;
		break;
	case 0x6C:
		/* JMP - Indirect */
		operand = get_op_IND(opcode, NES);
		execute_JMP(operand);
		break;
	case 0x6D:
		/* ADC - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_ADC(ABS, operand);
		update_FLAG_V(bin_operand1, bin_operand2, bin_result);
		set_or_clear_CARRY(tmp);
		break;
	case 0x6E:
		/* ROR - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_ROR(ABS, operand);
		break;
	case 0x70:
		/* BVS */
		execute_BVS(opcode);
		break;
	case 0x71:
		/* ADC - Indirect Y mode */
		operand = get_op_INDY(opcode, NES);
		execute_ADC(INDY, operand);
		update_FLAG_V(bin_operand1, bin_operand2, bin_result);
		set_or_clear_CARRY(tmp);
		break;
	case 0x75:
		/* ADC - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_ADC(ZPX, operand);
		update_FLAG_V(bin_operand1, bin_operand2, bin_result);
		set_or_clear_CARRY(tmp);
		break;
	case 0x76:
		/* ROR - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_ROR(ZPX, operand);
		break;
	case 0x78:
		/* SEI */
		execute_SEI(NES);
		++NES->PC;
		break;
	case 0x79:
		/* ADC - Absolute Y mode */
		operand = get_op_ABS_offset(opcode, NES->Y);
		execute_ADC(ABSY, operand);
		update_FLAG_V(bin_operand1, bin_operand2, bin_result);
		set_or_clear_CARRY(tmp);
		break;
	case 0x7D:
		/* ADC - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_ADC(ABSX, operand);
		update_FLAG_V(bin_operand1, bin_operand2, bin_result);
		set_or_clear_CARRY(tmp);
		break;
	case 0x7E:
		/* ROR - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_ROR(ABSX, operand);
		break;
	case 0x81:
		/* STA - Indirect X mode */
		operand = get_op_INDX(opcode, NES);
		execute_STA(operand);
		break;
	case 0x84:
		/* STY - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_STY(operand);
		break;
	case 0x85:
		/* STA - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_STA(operand);
		break;
	case 0x86:
		/* STX - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_STX(operand);
		break;
	case 0x88:
		/* DEY */
		execute_DEY();
		++NES->PC;
		break;
	case 0x8A:
		/* TXA */
		execute_TXA();
		++NES->PC;
		break;
	case 0x8C:
		/* STY - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_STY(operand);
		break;
	case 0x8D:
		/* STA - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_STA(operand);
		break;
	case 0x8E:
		/* STX - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_STX(operand);
		break;
	case 0x90:
		/* BCC */
		execute_BCC(opcode);
		break;
	case 0x91:
		/* STA - Indirect Y mode */
		operand = get_op_INDY(opcode, NES);
		execute_STA(operand);
		break;
	case 0x94:
		/* STY - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_STY(operand);
		break;
	case 0x95:
		/* STA - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_STA(operand);
		break;
	case 0x96:
		/* STX - Zero Page Y mode */
		operand = get_op_ZP_offset(opcode, NES->Y);
		execute_STX(operand);
		break;
	case 0x98:
		/* TYA */
		execute_TYA();
		++NES->PC;
		break;
	case 0x99:
		/* STA - Absolute Y mode */
		operand = get_op_ABS_offset(opcode, NES->Y);
		execute_STA(operand);
		break;
	case 0x9A:
		/* TXS */
		execute_TXS();
		++NES->PC;
		break;
	case 0x9D:
		/* STA - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_STA(operand);
		break;
	case 0xA0:
		/* LDY - Immediate mode */
		operand = get_op_IMM(opcode);
		execute_LDY(IMM, operand);
		break;
	case 0xA1:
		/* LDA - Indirect X mode */
		operand = get_op_INDX(opcode, NES);
		execute_LDA(INDX, operand);
		break;
	case 0xA2:
		/* LDX - Immediate mode */
		operand = get_op_IMM(opcode);
		execute_LDX(IMM, operand);
		break;
	case 0xA4:
		/* LDY - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_LDY(ZP, operand);
		break;
	case 0xA5:
		/* LDA - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_LDA(ZP, operand);
		break;
	case 0xA6:
		/* LDX - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_LDX(ZP, operand);
		break;
	case 0xA8:
		/* TAY */
		execute_TAY();
		++NES->PC;
		break;
	case 0xA9:
		/* LDA - Immediate mode */
		operand = get_op_IMM(opcode);
		execute_LDA(IMM, operand);
		break;
	case 0xAA:
		/* TAX */
		execute_TAX();
		++NES->PC;
		break;
	case 0xAC:
		/* LDY - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_LDY(ABS, operand);
		break;
	case 0xAD:
		/* LDA - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_LDA(ABS, operand);
		break;
	case 0xAE:
		/* LDX - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_LDX(ABS, operand);
		break;
	case 0xB0:
		/* BCS */
		execute_BCS(opcode);
		break;
	case 0xB1:
		/* LDA - Indirect Y mode */
		operand = get_op_INDY(opcode, NES);
		execute_LDA(INDY, operand);
		break;
	case 0xB4:
		/* LDY - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_LDY(ZPX, operand);
		break;
	case 0xB5:
		/* LDA - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_LDA(ZPX, operand);
		break;
	case 0xB6:
		/* LDX - Zero Page Y mode */
		operand = get_op_ZP_offset(opcode, NES->Y);
		execute_LDX(ZPX, operand);
		break;
	case 0xB8:
		/* CLV */
		execute_CLV();
		++NES->PC;
		break;
	case 0xB9:
		/* LDA - Absolute Y mode */
		operand = get_op_ABS_offset(opcode, NES->Y);
		execute_LDA(ABSY, operand);
		break;
	case 0xBA:
		/* TSX */
		execute_TSX();
		++NES->PC;
		break;
	case 0xBC:
		/* LDY - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_LDY(ABSX, operand);
		break;
	case 0xBD:
		/* LDA - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_LDA(ABSX, operand);
		break;
	case 0xBE:
		/* LDX - Absolute Y mode */
		operand = get_op_ABS_offset(opcode, NES->Y);
		execute_LDX(ABSY, operand);
		break;
	case 0xC0:
		/* CPY - Immediate mode */
		operand = get_op_IMM(opcode);
		execute_CPY(IMM, operand);
		break;
	case 0xC1:
		/* CMP - Indirect X mode */
		operand = get_op_INDX(opcode, NES);
		execute_CMP(INDX, operand);
		break;
	case 0xC4:
		/* CPY - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_CPY(ZP, operand);
		break;
	case 0xC5:
		/* CMP - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_CMP(ZP, operand);
		break;
	case 0xC6:
		/* DEC - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_DEC(operand);
		break;
	case 0xC8:
		/* INY */
		execute_INY();
		++NES->PC;
		break;
	case 0xC9:
		/* CMP - Immediate mode */
		operand = get_op_IMM(opcode);
		execute_CMP(IMM, operand);
		break;
	case 0xCA:
		/* DEX */
		execute_DEX();
		++NES->PC;
		break;
	case 0xCC:
		/* CPY - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_CPY(ABS, operand);
		break;
	case 0xCD:
		/* CMP - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_CMP(ABS, operand);
		break;
	case 0xCE:
		/* DEC - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_DEC(operand);
		break;
	case 0xD0:
		/* BNE */
		execute_BNE(opcode);
		break;
	case 0xD1:
		/* CMP - Indirect Y  mode */
		operand = get_op_INDY(opcode, NES);
		execute_CMP(INDY, operand);
		break;
	case 0xD5:
		/* CMP - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_CMP(ZPX, operand);
		break;
	case 0xD6:
		/* DEC - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_DEC(operand);
		break;
	case 0xD8:
		/* CLD */
		execute_CLD();
		++NES->PC;
		break;
	case 0xD9:
		/* CMP - Absolute Y mode */
		operand = get_op_ABS_offset(opcode, NES->Y);
		execute_CMP(ABSY, operand);
		break;
	case 0xDD:
		/* CMP - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_CMP(ABSX, operand);
		break;
	case 0xDE:
		/* DEC - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_DEC(operand);
		break;
	case 0xE0:
		/* CPX - Immediate mode */
		operand = get_op_IMM(opcode);
		execute_CPX(IMM, operand);
		break;
	case 0xE1:
		/* SBC - Indirect X mode */
		operand = get_op_INDX(opcode, NES);
		execute_SBC(INDX, operand);
		break;
	case 0xE4:
		/* CPX - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_CPX(ZP, operand);
		break;
	case 0xE5:
		/* SBC - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_SBC(ZP, operand);
		break;
	case 0xE6:
		/* INC - Zero Page mode */
		operand = get_op_ZP_offset(opcode, 0);
		execute_INC(operand);
		break;
	case 0xE8:
		/* INX */
		execute_INX();
		++NES->PC;
		break;
	case 0xE9:
		/* SBC - Immediate mode */
		operand = get_op_IMM(opcode);
		execute_SBC(IMM, operand);
		break;
	case 0xEA:
		/* NOP */
		printf("NOP\t");
		execute_NOP();
		++NES->PC;
		break;
	case 0xEC:
		/* CPX - Absolute mode*/
		operand = get_op_ABS_offset(opcode, 0);
		execute_CPX(ABS, operand);
		break;
	case 0xED:
		/* SBC - Absolute mode */
		operand = get_op_ABS_offset(opcode, 0);
		execute_SBC(ABS, operand);
		break;
	case 0xEE:
		/* INC - Absolute mode*/
		operand = get_op_ABS_offset(opcode, 0);
		execute_INC(operand);
		break;
	case 0xF0:
		/* BEQ */
		execute_BEQ(opcode);
		break;
	case 0xF1:
		/* SBC - Indirect Y mode */
		operand = get_op_INDY(opcode, NES);
		execute_SBC(INDY, operand);
		break;
	case 0xF5:
		/* SBC - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_SBC(ZPX, operand);
		break;
	case 0xF6:
		/* INC - Zero Page X mode */
		operand = get_op_ZP_offset(opcode, NES->X);
		execute_INC(operand);
		break;
	case 0xF8:
		/* SED */
		execute_SED(NES);
		++NES->PC;
		break;
	case 0xF9:
		/* SBC - Absolute Y mode */
		operand = get_op_ABS_offset(opcode, NES->Y);
		execute_SBC(ABSY, operand);
		break;
	case 0xFD:
		/* SBC - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_SBC(ABSX, operand);
		break;
	case 0xFE:
		/* INC - Absolute X mode */
		operand = get_op_ABS_offset(opcode, NES->X);
		execute_INC(operand);
		break;
	default:
		/* Invalid command */
		printf("Undocumented opcode: 0x%.2x\t", *opcode);
		++NES->PC; /* This causes a problem that undocumented opcodes advance PC by 1 */
		break;
	}
}
