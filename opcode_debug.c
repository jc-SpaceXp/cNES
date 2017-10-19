/* Reads NES ROM and print's what instructions we encounter */
/* in Numerical order */

#include "opcode_debug.h"

int Disassemble6502(unsigned char *code, int pc)
{
	char op_string[256];
	uint8_t *opcode = &code[pc];
	int length = 1; /* opcode length - 1-bytes long * length */

	switch (opcode[0]) {
	case 0x00:
		/* BRK */
		printf("BRX command not implimented: ");
		break;
	case 0x01:
		/* ORA - Indirect X mode*/
		printf("ORA command not implimented yet: ");
		length = 2;
		break;
	case 0x05:
		/* ORA - Zero page mode */
		printf("ORA command not implimented yet: ");
		length = 2;
		break;
	case 0x06:
		/* ASL - Zero page mode */
		printf("ASL command not implimented yet: ");
		length = 2;
		break;
	case 0x08:
		/* PHP */
		printf("PHP command not implimented yet: ");
		break;
	case 0x09:
		/* ORA - Immediate mode */
		printf("ORA command not implimented yet: ");
		length = 3;
		break;
	case 0x0A:
		/* ASL - Accumulator mode */
		printf("ASL command not implimented yet: ");
		break;
	case 0x0D:
		/* ORA - Absolute mode */
		printf("ORA command not implimented yet: ");
		length = 3;
		break;
	case 0x0E:
		/* ASL - Absolute mode */
		printf("ORA command not implimented yet: ");
		length = 3;
		break;
	case 0x10:
		/* BPL */
		printf("BPL command not implimented yet: ");
		length = 2;
		break;
	case 0x11:
		/* ORA - Indirect Y mode */
		printf("ORA command not implimented yet: ");
		length = 2;
		break;
	case 0x15:
		/* ORA - Zero Page X mode */
		printf("ORA command not implimented yet: ");
		length = 2;
		break;
	case 0x16:
		/* ASL - Zero Page X mode */
		printf("ASL command not implimented yet: ");
		length = 2;
		break;
	case 0x18:
		/* CLC */
		printf("CLC command not implimented yet: ");
		break;
	case 0x19:
		/* ORA - Absolute Y mode */
		printf("ORA command not implimented yet: ");
		length = 3;
		break;
	case 0x1D:
		/* ORA - Absolute X mode */
		printf("ORA command not implimented yet: ");
		length = 3;
		break;
	case 0x1E:
		/* ASL - Absolute X mode */
		printf("ASL command not implimented yet: ");
		length = 3;
		break;
	case 0x20:
		/* JSR - Absolute mode*/
		printf("JSR command not implimented yet: ");
		length = 3;
		break;
	case 0x21:
		/* AND - Indirect X mode*/
		printf("AND command not implimented yet: ");
		length = 2;
		break;
	case 0x24:
		/* BIT - Zero Page mode*/
		printf("BIT command not implimented yet: ");
		length = 2;
		break;
	case 0x25:
		/* AND - Zero Page mode*/
		printf("AND command not implimented yet: ");
		length = 2;
		break;
	case 0x26:
		/* ROL - Zero Page mode*/
		printf("ROL command not implimented yet: ");
		length = 2;
		break;
	case 0x28:
		/* PLP */
		printf("PLP command not implimented yet: ");
		break;
	case 0x29:
		/* AND - Immediate mode*/
		printf("******AND command not implimented yet: ");
		length = 2;
		break;
	case 0x2A:
		/* ROL - Accumulator mode*/
		printf("ROL command not implimented yet: ");
		break;
	case 0x2C:
		/* BIT - Absolute mode*/
		printf("ROL command not implimented yet: ");
		length = 3;
		break;
	case 0x2D:
		/* AND - Absolute mode*/
		printf("AND command not implimented yet: ");
		length = 3;
		break;
	case 0x2E:
		/* ROL - Absolute mode*/
		printf("ROL command not implimented yet: ");
		length = 3;
		break;
	case 0x30:
		/* BMI */
		printf("BMI command not implimented yet: ");
		length = 2;
		break;
	case 0x31:
		/* AND - Indirect Y mode*/
		printf("AND command not implimented yet: ");
		length = 2;
		break;
	case 0x35:
		/* AND - Zero Page X mode*/
		printf("AND command not implimented yet: ");
		length = 2;
		break;
	case 0x36:
		/* ROL - Zero Page X mode*/
		printf("ROL command not implimented yet: ");
		length = 2;
		break;
	case 0x38:
		/* SEC */
		printf("SEC command not implimented yet: ");
		break;
	case 0x39:
		/* AND - Absolute Y mode */
		printf("AND command not implimented yet: ");
		length = 3;
		break;
	case 0x3D:
		/* AND - Absolute X mode */
		printf("AND command not implimented yet: ");
		length = 3;
		break;
	case 0x3E:
		/* ROL - Absolute X mode */
		printf("ROL command not implimented yet: ");
		length = 3;
		break;
	case 0x40:
		/* RTI */
		printf("RTI command not implimented yet: ");
		break;
	case 0x41:
		/* EOR - Indirect X mode */
		printf("EOR command not implimented yet: ");
		length = 2;
		break;
	case 0x45:
		/* EOR - Zero Page mode */
		printf("EOR command not implimented yet: ");
		length = 2;
		break;
	case 0x46:
		/* LSR - Zero Page mode */
		printf("LSR command not implimented yet: ");
		length = 2;
		break;
	case 0x48:
		/* PHA */
		printf("PHA command not implimented yet: ");
		break;
	case 0x49:
		/* EOR - Immediate mode */
		printf("EOR command not implimented yet: ");
		length = 2;
		break;
	case 0x4A:
		/* LSR - Accumulator */
		printf("LSR command not implimented yet: ");
		break;
	case 0x4C:
		/* JMP - Absolute mode */
		printf("JMP command not implimented yet: ");
		length = 3;
		break;
	case 0x4D:
		/* EOR - Absolute mode */
		printf("EOR command not implimented yet: ");
		length = 3;
		break;
	case 0x4E:
		/* LSR - Absolute mode */
		printf("LSR command not implimented yet: ");
		length = 3;
		break;
	case 0x50:
		/* BVC */
		printf("BVC command not implimented yet: ");
		length = 2;
		break;
	case 0x51:
		/* EOR - Indirect Y mode */
		printf("EOR command not implimented yet: ");
		length = 2;
		break;
	case 0x55:
		/* EOR - Zero Page X mode */
		printf("EOR command not implimented yet: ");
		length = 2;
		break;
	case 0x56:
		/* LSR - Zero Page X mode */
		printf("LSR command not implimented yet: ");
		length = 2;
		break;
	case 0x58:
		/* CLI */
		printf("CLI command not implimented yet: ");
		break;
	case 0x59:
		/* EOR - Absolute Y mode */
		printf("EOR command not implimented yet: ");
		length = 3;
		break;
	case 0x5D:
		/* EOR - Absolute X mode */
		printf("EOR command not implimented yet: ");
		length = 3;
		break;
	case 0x5E:
		/* LSR - Absolute X mode */
		printf("LSR command not implimented yet: ");
		length = 3;
		break;
	case 0x60:
		/* RTS */
		printf("RTS command not implimented yet: ");
		break;
	case 0x61:
		/* ADC - Indirect X mode */
		printf("ADC command not implimented yet: ");
		length = 2;
		break;
	case 0x65:
		/* ADC - Zero Page mode */
		printf("ADC command not implimented yet: ");
		length = 2;
		break;
	case 0x66:
		/* ROR - Zero Page mode */
		printf("ROR command not implimented yet: ");
		length = 2;
		break;
	case 0x68:
		/* PLA */
		printf("PLA command not implimented yet: ");
		break;
	case 0x69:
		/* ADC - Immediate mode */
		printf("ADC command not implimented yet: ");
		length = 2;
		break;
	case 0x6A:
		/* ROR - Accumulator mode */
		printf("ROR command not implimented yet: ");
		break;
	case 0x6C:
		/* JMP - Indirect */
		printf("JMP command not implimented yet: ");
		length = 3;
		break;
	case 0x6D:
		/* ADC - Absolute mode */
		printf("ADC command not implimented yet: ");
		length = 3;
		break;
	case 0x6E:
		/* ROR - Absolute mode */
		printf("ROR command not implimented yet: ");
		length = 3;
		break;
	case 0x70:
		/* BVS - BRANCH (add @ end) */
		printf("BVS command not implimented yet: ");
		length = 2;
		break;
	case 0x71:
		/* ADC - Indirect Y mode */
		printf("ADC command not implimented yet: ");
		length = 2;
		break;
	case 0x75:
		/* ADC - Zero Page X mode */
		printf("ADC command not implimented yet: ");
		length = 2;
		break;
	case 0x76:
		/* ROR - Zero Page X mode */
		printf("ROR command not implimented yet: ");
		length = 2;
		break;
	case 0x78:
		/* SEI */
		printf("SEI command not implimented yet: ");
		break;
	case 0x79:
		/* ADC - Absolute Y mode */
		printf("ADC command not implimented yet: ");
		length = 3;
		break;
	case 0x7D:
		/* ADC - Absolute X mode */
		printf("ADC command not implimented yet: ");
		length = 3;
		break;
	case 0x7E:
		/* ROR - Absolute X mode */
		printf("ROR command not implimented yet: ");
		length = 3;
		break;
	case 0x81:
		/* STA - Indirect X mode */
		printf("STA command not implimented yet: ");
		length = 2;
		break;
	case 0x84:
		/* STY - Zero Page mode */
		printf("STY command not implimented yet: ");
		length = 2;
		break;
	case 0x85:
		/* STA - Zero Page mode */
		printf("STA command not implimented yet: ");
		length = 2;
		break;
	case 0x86:
		/* STX - Zero Page mode */
		printf("STX command not implimented yet: ");
		length = 2;
		break;
	case 0x88:
		/* DEY */
		printf("DEY command not implimented yet: ");
		break;
	case 0x8A:
		/* TXA */
		printf("TXA command not implimented yet: ");
		break;
	case 0x8C:
		/* STY - Absolute mode */
		printf("STY command not implimented yet: ");
		length = 3;
		break;
	case 0x8D:
		/* STA - Absolute mode */
		printf("STA command not implimented yet: ");
		length = 3;
		break;
	case 0x8E:
		/* STX - Absolute mode */
		printf("STX command not implimented yet: ");
		length = 3;
		break;
	case 0x90:
		/* BCC - BRANCH (Add later) */
		printf("BCC command not implimented yet: ");
		length = 2;
		break;
	case 0x91:
		/* STA - Indirect Y mode */
		printf("STA command not implimented yet: ");
		length = 2;
		break;
	case 0x94:
		/* STY - Zero Page X mode */
		printf("STY command not implimented yet: ");
		length = 2;
		break;
	case 0x95:
		/* STA - Zero Page X mode */
		printf("STA command not implimented yet: ");
		length = 2;
		break;
	case 0x96:
		/* STX - Zero Page Y mode */
		printf("STX command not implimented yet: ");
		length = 2;
		break;
	case 0x98:
		/* TYA */
		printf("TYA command not implimented yet: ");
		break;
	case 0x99:
		/* STA - Absolute Y mode */
		printf("STA command not implimented yet: ");
		length = 3;
		break;
	case 0x9A:
		/* TXS */
		printf("TXS command not implimented yet: ");
		break;
	case 0x9D:
		/* STA - Absolute X mode */
		printf("STA command not implimented yet: ");
		length = 3;
		break;
	case 0xA0:
		/* LDY - Immediate mode */
		printf("LDY command not implimented yet: ");
		length = 2;
		break;
	case 0xA1:
		/* LDA - Indirect X mode */
		printf("LDA command not implimented yet: ");
		length = 2;
		break;
	case 0xA2:
		/* LDX - Immediate mode */
		printf("LDX command not implimented yet: ");
		length = 2;
		break;
	case 0xA4:
		/* LDY - Zero Page mode */
		printf("LDY command not implimented yet: ");
		length = 2;
		break;
	case 0xA5:
		/* LDA - Zero Page mode */
		printf("LDA command not implimented yet: ");
		length = 2;
		break;
	case 0xA6:
		/* LDX - Zero Page mode */
		printf("LDX command not implimented yet: ");
		length = 2;
		break;
	case 0xA8:
		/* TAY */
		printf("TAY command not implimented yet: ");
		break;
	case 0xA9:
		/* LDA - Immediate mode */
		printf("LDA command not implimented yet: ");
		length = 2;
		break;
	case 0xAA:
		/* TAX */
		printf("TAX command not implimented yet: ");
		length = 2;
		break;
	case 0xAC:
		/* LDY - Absolute mode */
		printf("LDY command not implimented yet: ");
		length = 3;
		break;
	case 0xAD:
		/* LDA - Absolute mode */
		printf("LDA command not implimented yet: ");
		length = 3;
		break;
	case 0xAE:
		/* LDX - Absolute mode */
		printf("LDX command not implimented yet: ");
		length = 3;
		break;
	case 0xB0:
		/* BCS - BRANCH (add @ end */
		printf("BCS command not implimented yet: ");
		length = 2;
		break;
	case 0xB1:
		/* LDA - Indirect Y mode */
		printf("LDA command not implimented yet: ");
		length = 3;
		break;
	case 0xB4:
		/* LDY - Zero Page X mode */
		printf("LDY command not implimented yet: ");
		length = 2;
		break;
	case 0xB5:
		/* LDA - Zero Page X mode */
		printf("LDA command not implimented yet: ");
		length = 2;
		break;
	case 0xB6:
		/* LDX - Zero Page Y mode */
		printf("LDX command not implimented yet: ");
		length = 2;
		break;
	case 0xB8:
		/* CLV */
		printf("CLV command not implimented yet: ");
		break;
	case 0xB9:
		/* LDA - Absolute Y mode */
		printf("LDA command not implimented yet: ");
		length = 3;
		break;
	case 0xBA:
		/* TSX */
		printf("TSX command not implimented yet: ");
		break;
	case 0xBC:
		/* LDY - Absolute X mode */
		printf("LDY command not implimented yet: ");
		length = 3;
		break;
	case 0xBD:
		/* LDA - Absolute X mode */
		printf("LDA command not implimented yet: ");
		length = 3;
		break;
	case 0xBE:
		/* LDX - Absolute Y mode */
		printf("LDX command not implimented yet: ");
		length = 3;
		break;
	case 0xC0:
		/* CPY - Immediate mode */
		printf("CPY command not implimented yet: ");
		length = 2;
		break;
	case 0xC1:
		/* CMP - Indirect X mode */
		printf("CMP command not implimented yet: ");
		length = 2;
		break;
	case 0xC4:
		/* CPY - Zero Page mode */
		printf("CPY command not implimented yet: ");
		length = 2;
		break;
	case 0xC5:
		/* CMP - Zero Page mode */
		printf("CMP command not implimented yet: ");
		length = 2;
		break;
	case 0xC6:
		/* DEC - Zero Page mode */
		printf("DEC command not implimented yet: ");
		length = 2;
		break;
	case 0xC8:
		/* INY */
		printf("INY command not implimented yet: ");
		break;
	case 0xC9:
		/* CMP - Immediate mode */
		printf("CMP command not implimented yet: ");
		length = 2;
		break;
	case 0xCA:
		/* DEX */
		printf("DEX command not implimented yet: ");
		break;
	case 0xCC:
		/* CPY - Absolute mode */
		printf("CPY command not implimented yet: ");
		length = 3;
		break;
	case 0xCD:
		/* CMP - Absolute mode */
		printf("CMP command not implimented yet: ");
		length = 3;
		break;
	case 0xCE:
		/* DEC - Absolute mode */
		printf("DEC command not implimented yet: ");
		length = 3;
		break;
	case 0xD0:
		/* BNE - (BRANCH (add @ end) */
		printf("BNE command not implimented yet: ");
		length = 2;
		break;
	case 0xD1:
		/* CMP - Indirect Y  mode */
		printf("CMP command not implimented yet: ");
		length = 2;
		break;
	case 0xD5:
		/* CMP - Zero Page X mode */
		printf("CMP command not implimented yet: ");
		length = 2;
		break;
	case 0xD6:
		/* DEC - Zero Page X mode */
		printf("DEC command not implimented yet: ");
		length = 2;
		break;
	case 0xD8:
		/* CLD */
		printf("CLD command not implimented yet: ");
		break;
	case 0xD9:
		/* CMP - Absolute Y mode */
		printf("CMP command not implimented yet: ");
		length = 3;
		break;
	case 0xDD:
		/* CMP - Absolute X mode */
		printf("CMP command not implimented yet: ");
		length = 3;
		break;
	case 0xDE:
		/* DEC - Absolute X mode */
		printf("DEC command not implimented yet: ");
		length = 3;
		break;
	case 0xE0:
		/* CPX - Immediate mode */
		printf("CPX command not implimented yet: ");
		length = 2;
		break;
	case 0xE1:
		/* SBC - Indirect X mode */
		printf("SBC command not implimented yet: ");
		length = 2;
		break;
	case 0xE4:
		/* CPX - Zero Page mode */
		printf("CPX command not implimented yet: ");
		length = 2;
		break;
	case 0xE5:
		/* SBC - Zero Page mode */
		printf("SBC command not implimented yet: ");
		length = 2;
		break;
	case 0xE6:
		/* INC - Zero Page mode */
		printf("INC command not implimented yet: ");
		length = 2;
		break;
	case 0xE8:
		/* INX */
		printf("INX command not implimented yet: ");
		break;
	case 0xE9:
		/* SBC - Immediate mode */
		printf("SBC command not implimented yet: ");
		length = 2;
		break;
	case 0xEA:
		/* NOP */
		printf("NOP command not implimented yet: ");
		break;
	case 0xEC:
		/* CPX - Absolute mode*/
		printf("CPX command not implimented yet: ");
		length = 3;
		break;
	case 0xED:
		/* SBC - Absolute mode */
		printf("SBC command not implimented yet: ");
		length = 3;
		break;
	case 0xEE:
		/* INC - Absolute mode*/
		printf("INC command not implimented yet: ");
		length = 3;
		break;
	case 0xF0:
		/* BEQ - BRANCH (add @ end) */
		printf("beq command not implimented yet: ");
		length = 2;
		break;
	case 0xF1:
		/* SBC - Indirect Y mode */
		printf("SBC command not implimented yet: ");
		length = 2;
		break;
	case 0xF5:
		/* SBC - Zero Page X mode */
		printf("SBC command not implimented yet: ");
		length = 2;
		break;
	case 0xF6:
		/* INC - Zero Page X mode */
		printf("INC command not implimented yet: ");
		length = 2;
		break;
	case 0xF8:
		/* SED */
		printf("SED command not implimented yet: ");
		break;
	case 0xF9:
		/* SBC - Absolute Y mode */
		printf("SBC command not implimented yet: ");
		length = 3;
		break;
	case 0xFD:
		/* SBC - Absolute X mode */
		printf("SBC command not implimented yet: ");
		length = 3;
		break;
	case 0xFE:
		/* INC - Absolute X mode */
		printf("INC command not implimented yet: ");
		length = 3;
		break;
	default:
		/* Invalid command */
		printf("Undocumented code: ");
		break;
	}
	/* Return format of hex code we've processed */
	printf("%04x 0x%02x", pc, *opcode);
	if (length == 2) {
		printf("%02x\n", *(opcode+1));
	} else if (length == 3) {
		printf("%02x%02x\n", *(opcode+1), *(opcode+2));
	} else {
		printf("\n");
	}
	return length;	
}
