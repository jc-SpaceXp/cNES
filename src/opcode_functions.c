/*
 * Execute Functions (Opcode functions)
 *
 */

#include "opcode_functions.h" /* + cpu.h */
#include "helper_functions.h"
#include <stdio.h>


int bin_operand1[8];
int bin_operand2[8];
int bin_result[8];
unsigned int power2 = 1;
/***************************
 * STORAGE                 *
 * *************************/

/* execute_LDA: LDA command - Load A with memory
 */
void execute_LDA(enum MODES address_mode, CPU_6502* CPU)
{
	if (address_mode == IMM) {
		strcpy(instruction, "LDA #$");
		sprintf(append_int, "%.2X", CPU->operand);
		strcat(instruction, append_int);
		CPU->A = CPU->operand;
	} else {
		strcpy(instruction, "LDA ");
		strcat(instruction, end);
		CPU->A = read_addr(CPU, CPU->target_addr);
	}
	update_FLAG_N(CPU->A);
	update_FLAG_Z(CPU->A);
}


/* execute_LDX: LDX command - Load X with memory
 */
void execute_LDX(enum MODES address_mode, CPU_6502* CPU)
{
	if (address_mode == IMM) {
		CPU->X = CPU->operand;
		strcpy(instruction, "LDX #$");
		sprintf(append_int, "%.2X", NES->operand);
		strcat(instruction, append_int);
	} else {
		//printf("LDX $%.4X    ", operand);
		strcpy(instruction, "LDX ");
		strcat(instruction, end);
		CPU->X = read_addr(CPU, CPU->target_addr);
	}
	update_FLAG_N(CPU->X);
	update_FLAG_Z(CPU->X);
}


/* execute_LDY: LDY command - Load Y with memory
 */
void execute_LDY(enum MODES address_mode, CPU_6502* CPU)
{
	if (address_mode == IMM) {
		strcpy(instruction, "LDY #$");
		sprintf(append_int, "%.2X", CPU->operand);
		strcat(instruction, append_int);
		CPU->Y = CPU->operand;
	} else {
		strcpy(instruction, "LDY ");
		strcat(instruction, end);
		CPU->Y = read_addr(CPU, CPU->target_addr);
	}
	update_FLAG_N(CPU->Y);
	update_FLAG_Z(CPU->Y);
}


/* execute_STA: STA command - Store A in memory
 */
void execute_STA(CPU_6502* CPU)
{
	strcpy(instruction, "STA ");
	strcat(instruction, end);
	write_addr(CPU, CPU->target_addr, CPU->A);
}


/* execute_STX: STX command - Store X in memory
 */
void execute_STX(CPU_6502* CPU)
{
	strcpy(instruction, "STX ");
	strcat(instruction, end);
	write_addr(CPU, CPU->target_addr, CPU->X);
}


/* execute_STY: STY command - Store Y in memory
 */
void execute_STY(CPU_6502* CPU)
{
	strcpy(instruction, "STY ");
	strcat(instruction, end);
	write_addr(CPU, CPU->target_addr, CPU->Y);
}


/* execute_TAX: TAX command - Transfer A to X
 */
void execute_TAX(CPU_6502* CPU)
{
	strcpy(instruction, "TAX");
	CPU->X = CPU->A;
	update_FLAG_N(CPU->X);
	update_FLAG_Z(CPU->X);
}


/* execute_TAY: TAY command - Transfer A to Y
 */
void execute_TAY(CPU_6502* CPU)
{
	strcpy(instruction, "TAY");
	CPU->Y = CPU->A;
	update_FLAG_N(CPU->Y);
	update_FLAG_Z(CPU->Y);
}


/* execute_TSX: TSX command - Transfer SP to X
 */
void execute_TSX(CPU_6502* CPU)
{
	strcpy(instruction, "TSX");
	CPU->X = CPU->Stack;
	update_FLAG_N(CPU->X);
	update_FLAG_Z(CPU->X);
}


/* execute_TXA: TXA command - Transfer X to A
 */
void execute_TXA(CPU_6502* CPU)
{
	strcpy(instruction, "TXA");
	CPU->A = CPU->X;
	update_FLAG_N(CPU->A);
	update_FLAG_Z(CPU->A);
}


/* execute_TXS: TXS command - Transfer X to SP
 */
void execute_TXS(CPU_6502* CPU)
{
	strcpy(instruction, "TXS");
	CPU->Stack = CPU->X;
}


/* execute_TYA: TYA command - Transfer Y to A
 */
void execute_TYA(CPU_6502* CPU)
{
	strcpy(instruction, "TYA");
	CPU->A = CPU->Y;
	update_FLAG_N(CPU->A);
	update_FLAG_Z(CPU->A);
}

/***************************
 * MATH                    *
 * *************************/

/* execute_ADC: ADC command - Add mem w/ A and C (A + M + C : then set flags)
 */
void execute_ADC(enum MODES address_mode, CPU_6502* CPU)
{
	Base10toBase2(CPU->A, bin_operand1);
	if (address_mode == IMM) {
		/* Immediate - ADC #Operand */
		strcpy(instruction, "ADC #$");
		sprintf(append_int, "%X", CPU->operand);
		strcat(instruction, append_int);
		Base10toBase2(CPU->operand, bin_operand2);
	} else {
		strcpy(instruction, "ADC ");
		strcat(instruction, end);
		Base10toBase2(read_addr(CPU, CPU->target_addr), bin_operand2);
	}
	unsigned carry_out = 0;
	full_adder(bin_operand1, bin_operand2, NES->P & FLAG_C, &carry_out, bin_result);
	CPU->A = Base2toBase10(bin_result, 0);
	update_FLAG_N(CPU->A);
	update_FLAG_V(bin_operand1, bin_operand2, bin_result);
	update_FLAG_Z(CPU->A);
	set_or_clear_CARRY(carry_out);
}

/* execute_DEC: DEC command - Decrement Mem by one
 */
void execute_DEC(CPU_6502* CPU)
{
	strcpy(instruction, "DEC ");
	strcat(instruction, end);
	write_addr(CPU, CPU->target_addr, read_addr(CPU, CPU->target_addr) - 1);
	update_FLAG_N(read_addr(CPU, CPU->target_addr));
	update_FLAG_Z(read_addr(CPU, CPU->target_addr));
}


/* execute_DEX: DEX command - Decrement X by one
 */
void execute_DEX(CPU_6502 *CPU)
{
	/* Implied Mode */
	strcpy(instruction, "DEX");
	--(CPU->X);
	update_FLAG_N(CPU->X);
	update_FLAG_Z(CPU->X);
}


/* execute_DEY: DEY command - Decrement Y by one
 */
void execute_DEY(CPU_6502* CPU)
{
	/* Implied Mode */
	strcpy(instruction, "DEY");
	--(CPU->Y);
	update_FLAG_N(CPU->Y);
	update_FLAG_Z(CPU->Y);
}


/* execute_INC: INC command - Increment Mem by one
 */
void execute_INC(CPU_6502* CPU)
{
	strcpy(instruction, "INC ");
	strcat(instruction, end);
	write_addr(CPU, CPU->target_addr, read_addr(CPU, CPU->target_addr) + 1);
	update_FLAG_N(read_addr(CPU, CPU->target_addr));
	update_FLAG_Z(read_addr(CPU, CPU->target_addr));
}


/* execute_INX: INX command - Increment X by one
 */
void execute_INX(CPU_6502* CPU)
{
	strcpy(instruction, "INX");
	/* Implied Mode */
	++(CPU->X);
	update_FLAG_N(CPU->X);
	update_FLAG_Z(CPU->X);
}


/* execute_INY: DEY command - Increment Y by one
 */
void execute_INY(CPU_6502* CPU)
{
	strcpy(instruction, "INY");
	/* Implied Mode */
	++(CPU->Y);
	update_FLAG_N(CPU->Y);
	update_FLAG_Z(CPU->Y);
}


/* execute_SBC: SBC command - Subtract mem w/ A and C (A - M -C : then set flags)
 */
void execute_SBC(enum MODES address_mode, CPU_6502* CPU)

{
	/* SBC = ADC w/ 2nd operand converted to 2's compliment */
	Base10toBase2(CPU->A, bin_operand1);
	if (address_mode == IMM) {
		/* Immediate - SBC #Operand */
		strcpy(instruction, "SBC #$");
		sprintf(append_int, "%.2X", CPU->operand);
		strcat(instruction, append_int);
		Base10toBase2(CPU->operand ^ 0xFF, bin_operand2);
	} else {
		strcpy(instruction, "SBC ");
		strcat(instruction, end);
		Base10toBase2(read_addr(CPU, CPU->target_addr) ^ 0xFF, bin_operand2);
	}
	unsigned carry_out = 0;
	full_adder(bin_operand1, bin_operand2, NES->P & FLAG_C, &carry_out, bin_result);
	CPU->A = Base2toBase10(bin_result, 0);
	update_FLAG_N(CPU->A);
	update_FLAG_V(bin_operand1, bin_operand2, bin_result);
	update_FLAG_Z(CPU->A);
	set_or_clear_CARRY(carry_out);
}

/***************************
 * BITWISE                 *
 * *************************/

/* execute_AND: AND command - AND memory with Acc
 */
void execute_AND(enum MODES address_mode, CPU_6502* CPU)
{
	if (address_mode == IMM) {
		strcpy(instruction, "AND #$");
		sprintf(append_int, "%.2X", CPU->operand);
		strcat(instruction, append_int);
		CPU->A &= CPU->operand;
	} else {
		strcpy(instruction, "AND ");
		strcat(instruction, end);
		CPU->A &= read_addr(CPU, CPU->target_addr);
	}
	update_FLAG_N(CPU->A);
	update_FLAG_Z(CPU->A);
}


/* execute_ASL: ASL command - Arithmetic Shift Left one bit (Acc or mem)
 * ASL == LSL
 */
void execute_ASL(enum MODES address_mode, CPU_6502* CPU)
{
	unsigned high_bit = 0;
	if (address_mode == ACC) {
		/* Accumulator - ASL #Operand */
		strcpy(instruction, "ASL A");
		high_bit = CPU->A & 0x80; /* Mask 7th bit */
		CPU->A = CPU->A << 1;
		update_FLAG_N(CPU->A);
		update_FLAG_Z(CPU->A);
	} else {
		/* Shift value @ address 1 bit to the left */
		strcpy(instruction, "ASL ");
		strcat(instruction, end);
		high_bit = read_addr(CPU, CPU->target_addr) & 0x80; /* Mask 7th bit */
		write_addr(CPU, CPU->target_addr, read_addr(CPU, CPU->target_addr) << 1);
		update_FLAG_N(read_addr(CPU, CPU->target_addr));
		update_FLAG_Z(read_addr(CPU, CPU->target_addr));
	}
	high_bit = high_bit >> 7; /* needed so that function below works */;
	/* Update Carry */
	set_or_clear_CARRY(high_bit);
}


/* execute_BIT: BIT command - BIT test (AND) between mem and Acc
 */
void execute_BIT(CPU_6502* CPU)
{
	strcpy(instruction, "BIT ");
	strcat(instruction, end);
	int tmp = CPU->A & read_addr(CPU, CPU->target_addr);
	/* Update Flags */
	/* N = Bit 7, V = Bit 6 (of fetched operand) & Z = 1 (if AND result = 0) */
	/* Setting 7th Bit */
	if ((read_addr(CPU, CPU->target_addr) & FLAG_N) == FLAG_N) {
		CPU->P |= FLAG_N; /* set */
	} else {
		CPU->P &= ~(FLAG_N); /* clear flag */
	}
	/* Setting 6th Bit */
	if ((read_addr(CPU, CPU->target_addr) & FLAG_V) == FLAG_V) {
		CPU->P |= FLAG_V;
	} else {
		CPU->P &= ~(FLAG_V);
	}
	/* Setting Zero FLAG */
	if (tmp == 0) {
		CPU->P |= FLAG_Z; /* set */
	} else {
		CPU->P &= ~(FLAG_Z); /* clear */
	}
	/* CLC/CLV could be called - but wanted to reduce function overhead */
}


/* execute_EOR: EOR command - Exclusive OR memory with Acc
 */
void execute_EOR(enum MODES address_mode, CPU_6502* CPU)
{
	if (address_mode == IMM) {
		/* Immediate - AND #Operand */
		strcpy(instruction, "EOR #$");
		sprintf(append_int, "%.2X", CPU->operand);
		strcat(instruction, append_int);
		CPU->A ^= CPU->operand;
	} else {
		strcpy(instruction, "EOR ");
		strcat(instruction, end);
		CPU->A ^= read_addr(CPU, CPU->target_addr);
	}
	update_FLAG_N(CPU->A);
	update_FLAG_Z(CPU->A);
}


/* execute_LSR: LSR command - Logical Shift Right by one bit (Acc or mem)
 */
void execute_LSR(enum MODES address_mode, CPU_6502* CPU)
{
	unsigned low_bit = 0;
	if (address_mode == ACC) {
		/* Accumulator - LSR #Operand */
		strcpy(instruction, "LSR A");
		low_bit = CPU->A & 0x01; /* Mask 0th bit */
		CPU->A = CPU->A >> 1;
		update_FLAG_N(CPU->A); /* Should always clear N flag */
		update_FLAG_Z(CPU->A);
	} else {
		strcpy(instruction, "LSR ");
		strcat(instruction, end);
		low_bit = read_addr(CPU, CPU->target_addr) & 0x01; /* Mask 0th bit */
		write_addr(CPU, CPU->target_addr, read_addr(CPU, CPU->target_addr) >> 1);
		update_FLAG_N(read_addr(CPU, CPU->target_addr)); /* Should always clear N flag */
		update_FLAG_Z(read_addr(CPU, CPU->target_addr));
	}
	/* Update Carry */
	set_or_clear_CARRY(low_bit);
}
	

/* execute_ORA: ORA command - OR memory with Acc
 */
void execute_ORA(enum MODES address_mode, CPU_6502* CPU)
{
	if (address_mode == IMM) {
		/* Immediate - AND #Operand */
		strcpy(instruction, "ORA #$");
		sprintf(append_int, "%.2X", CPU->operand);
		strcat(instruction, append_int);
		CPU->A |= CPU->operand;
	} else {
		strcpy(instruction, "ORA ");
		strcat(instruction, end);
		CPU->A |= read_addr(CPU, CPU->target_addr);
	}
	update_FLAG_N(CPU->A);
	update_FLAG_Z(CPU->A);
}


/* execute_ROL: ROL command - Rotate Shift Left one bit (Acc or mem)
 * ROL == LSL (execpt Carry Flag is copied into LSB & Carry = MSB after shift)
 */
void execute_ROL(enum MODES address_mode, CPU_6502* CPU)
{
	unsigned high_bit = 0;
	if (address_mode == ACC) {
		/* Accumulator - ROL #Operand */
		strcpy(instruction, "ROL A");
		high_bit = CPU->A & 0x80; /* Mask 7th bit */
		CPU->A = CPU->A << 1;
		/* Testing if Status Reg has a 1 in Carry Flag */
		if ((CPU->P & FLAG_C) == 0x01) {
			CPU->A |= (CPU->P & FLAG_C);
		} /* if carry = 0 then do nothing as that still leaves a zero in the 0th bit */
		update_FLAG_N(CPU->A);
		update_FLAG_Z(CPU->A);
	} else {
		strcpy(instruction, "ROL ");
		strcat(instruction, end);
		high_bit = read_addr(CPU, CPU->target_addr) & 0x80; /* Mask 7th bit */
		write_addr(CPU, CPU->target_addr, read_addr(CPU, CPU->target_addr) << 1);
		if ((CPU->P & FLAG_C) == 0x01) {
			write_addr(CPU, CPU->target_addr, read_addr(CPU, CPU->target_addr) | 0x01);
		}
		update_FLAG_N(read_addr(CPU, CPU->target_addr));
		update_FLAG_Z(read_addr(CPU, CPU->target_addr));
	}
	/* Update Flag */
	high_bit = high_bit >> 7; /* needed so that function below works */;
	set_or_clear_CARRY(high_bit);
}


/* execute_ROR: ROR command - Rotate Shift Right one bit (Acc or mem)
 * ROR == LSR (execpt MSB = carry & LSB copied into carry)
 */
void execute_ROR(enum MODES address_mode, CPU_6502* CPU)
{
	unsigned low_bit = 0;
	if (address_mode == ACC) {
		/* Accumulator - ROR #Operand */
		strcpy(instruction, "ROR A");
		low_bit = CPU->A & 0x01; /* Mask 0th bit */
		CPU->A = CPU->A >> 1; /* Shift right */
		if ((CPU->P & FLAG_C) == 0x01) {
			CPU->A |= 0x80; /* Set 7th bit to 1 - if carry = 1 */
		} /* if carry = 0 then do nothing as that still leaves a zero in the 0th bit */
		update_FLAG_N(CPU->A);
		update_FLAG_Z(CPU->A);
	} else {
		strcpy(instruction, "ROR ");
		strcat(instruction, end);
		low_bit = read_addr(CPU, CPU->target_addr) & 0x01;
		write_addr(CPU, CPU->target_addr, read_addr(CPU, CPU->target_addr) >> 1);
		if ((CPU->P & FLAG_C) == 0x01) {
			/* Set 7th bit to 1 - if carry = 1 */
			write_addr(CPU, CPU->target_addr, read_addr(CPU, CPU->target_addr) | 0x80);
		} /* if carry = 0 then do nothing as that still leaves a zero in the 0th bit */
		update_FLAG_N(read_addr(CPU, CPU->target_addr));
		update_FLAG_Z(read_addr(CPU, CPU->target_addr));
	}
	/* Update Carry */
	set_or_clear_CARRY(low_bit);
}

/***************************
 * BRANCH                  *
 * *************************/
/* all are in RELATIVE address mode : -126 to +129 on pc (due to +2 @ end) */
/* No flag changes */

/* execute_BCC: BCC command - Branch on Carry Clear (C = 0)
 */
void execute_BCC(uint8_t *ptr_code)
{
	/* Debugger */
	sprintf(append_int, "%.4X", NES->PC + 2 + (int8_t) *(ptr_code+1));
	strcpy(instruction, "BCC $");
	strcat(instruction, append_int);

	if ((NES->P & FLAG_C) == 0x00) {
		NES->PC += (int8_t) *(ptr_code+1);
		NES->Cycle += 1 + PAGE_CROSS(NES->old_PC + 2, NES->PC + 2);
	}
	NES->PC += 2;
}


/* execute_BCS: BCS command - Branch on Carry Set (C = 1)
 */
void execute_BCS(uint8_t *ptr_code)
{
	/* Debugger */
	sprintf(append_int, "%.4X", NES->PC + 2 + (int8_t) *(ptr_code+1));
	strcpy(instruction, "BCS $");
	strcat(instruction, append_int);

	if ((NES->P & FLAG_C) == 0x01) {
		NES->PC += (int8_t) *(ptr_code+1);
		NES->Cycle += 1 + PAGE_CROSS(NES->old_PC + 2, NES->PC + 2);
	}
	NES->PC += 2;
}


/* execute_BEQ: BEQ command - Branch on Zero result (Z = 1)
 */
void execute_BEQ(uint8_t *ptr_code)
{
	/* Debugger */
	sprintf(append_int, "%.4X", NES->PC + 2 + (int8_t) *(ptr_code+1));
	strcpy(instruction, "BEQ $");
	strcat(instruction, append_int);

	if ((NES->P & FLAG_Z) == FLAG_Z) {
		NES->PC += (int8_t) *(ptr_code+1);
		NES->Cycle += 1 + PAGE_CROSS(NES->old_PC + 2, NES->PC + 2);
	}
	NES->PC += 2;
}


/* execute_BMI: BMI command - Branch on Minus result (N = 1)
 */
void execute_BMI(uint8_t *ptr_code)
{
	/* Debugger */
	sprintf(append_int, "%.4X", NES->PC + 2 + (int8_t) *(ptr_code+1));
	strcpy(instruction, "BMI $");
	strcat(instruction, append_int);

	if ((NES->P & FLAG_N) == FLAG_N) {
		NES->PC += (int8_t) *(ptr_code+1);
		NES->Cycle += 1 + PAGE_CROSS(NES->old_PC + 2, NES->PC + 2);
	}
	NES->PC += 2;
}


/* execute_BNE: BNE command - Branch on NOT Zero result (Z = 0)
 */
void execute_BNE(uint8_t *ptr_code)
{
	/* Debugger */
	sprintf(append_int, "%.4X", NES->PC + 2 + (int8_t) *(ptr_code+1));
	strcpy(instruction, "BNE $");
	strcat(instruction, append_int);

	if ((NES->P & FLAG_Z) == 0x00) {
		NES->PC += (int8_t) *(ptr_code+1);
		NES->Cycle += 1 + PAGE_CROSS(NES->old_PC + 2, NES->PC + 2);
	}
	NES->PC += 2;
}


/* execute_BPL: BPL command - Branch on Plus result (N = 0)
 */
void execute_BPL(uint8_t *ptr_code)
{
	/* Debugger */
	sprintf(append_int, "%.4X", NES->PC + 2 + (int8_t) *(ptr_code+1));
	strcpy(instruction, "BPL $");
	strcat(instruction, append_int);

	if ((NES->P & FLAG_N) == 0x00) {
		NES->PC += (int8_t) *(ptr_code+1);
		NES->Cycle += 1 + PAGE_CROSS(NES->old_PC + 2, NES->PC + 2);
	}
	NES->PC += 2;
}


/* execute_BVC: BVC command - Branch on Overflow Clear (V = 0)
 */
void execute_BVC(uint8_t *ptr_code)
{
	/* Debugger */
	sprintf(append_int, "%.4X", NES->PC + 2 + (int8_t) *(ptr_code+1));
	strcpy(instruction, "BVC $");
	strcat(instruction, append_int);

	if ((NES->P & FLAG_V) == 0x00) {
		NES->PC += (int8_t) *(ptr_code+1);
		NES->Cycle += 1 + PAGE_CROSS(NES->old_PC + 2, NES->PC + 2);
	}
	NES->PC += 2;
}


/* execute_BVS: BVS command - Branch on Overflow Set (V = 1)
 */
void execute_BVS(uint8_t *ptr_code)
{
	/* Debugger */
	sprintf(append_int, "%.4X", NES->PC + 2 + (int8_t) *(ptr_code+1));
	strcpy(instruction, "BVS $");
	strcat(instruction, append_int);

	if ((NES->P & FLAG_V) == FLAG_V) {
		NES->PC += (int8_t) *(ptr_code+1);
		NES->Cycle += 1 + PAGE_CROSS(NES->old_PC + 2, NES->PC + 2);
	}
	NES->PC += 2;
}


/***************************
 * JUMP                    *
 * *************************/

/* execute_JMP: JMP command - JuMP to another location
 */
void execute_JMP(CPU_6502* CPU)
{
	strcpy(instruction, "JMP ");
	strcat(instruction, end);
	CPU->PC = CPU->target_addr;
}


/* execute_JSR: JSR command - Jump to SubRoutine
 */
void execute_JSR(CPU_6502* CPU)
{
	strcpy(instruction, "JSR $");
	sprintf(append_int, "%.4X", CPU->target_addr);
	strcat(instruction, append_int);
	/* Absolute - JSR operand */
	/* PC + 2 is pushed onto stack - always PUSH high byte first */
	PUSH((uint8_t) ((CPU->PC - 1) >> 8)); /* Push PCH (PC High byte onto stack) */
	PUSH((uint8_t) (CPU->PC - 1)); /* Push PCL (PC Low byte onto stack) */

	CPU->PC = CPU->target_addr;
	/* NB: get_op_ABS_offset sets PC to += 3 therfore to push PC + 2
	 * all we need to do is just push PC - 1
	 */
}



/* execute_RTI: RTI command - ReTurn from Interrupt
 */
void execute_RTI(CPU_6502* CPU)
{
	/* Implied */
	strcpy(instruction, "RTI");
	/* PULL SR */
	CPU->addr_lo = PULL(); // Isn't actually PCL, used as a tmp for now
	CPU->P = CPU->addr_lo | 0x20; /* Bit 5 is always set */
	/* PULL PC */
	CPU->addr_lo = PULL(); /* PULL PCL */
	CPU->addr_hi = PULL(); /* PULL PCH */
	CPU->PC = CPU->addr_lo | (CPU->addr_hi << 8);
}


/* execute_RTS: RTS command - ReTurn from Sub-routine
 */
void execute_RTS(CPU_6502* CPU)
{
	/* Implied */
	strcpy(instruction, "RTS");
	/* opposite of JSR - PULL PCL first */
	CPU->addr_lo = PULL(); /* Pull PCL */
	CPU->addr_hi = PULL(); /* Pull PCH */
	CPU->PC = (CPU->addr_hi << 8) | CPU->addr_lo ;
	++CPU->PC;

}

/***************************
 * Registers               *
 * *************************/

/* execute_CLC: CLC command - Clear Carry flag
 */
void execute_CLC(CPU_6502* CPU)
{
	/* CLC */
	strcpy(instruction, "CLC");
	CPU->P &= ~(FLAG_C); /* set Flag C to: 11111110 then AND to P */
}


/* execute_CLD: CLD command - Clear Decimal Mode (Decimal mode not supported in NES) 
 */
void execute_CLD(CPU_6502* CPU)
{
	/* CLD */
	strcpy(instruction, "CLD");
	CPU->P &= ~(FLAG_D);

}


/* execute_CLI: CLI command - Clear Interrupt disable bit
 */
void execute_CLI(CPU_6502* CPU)
{
	/* CLI */
	strcpy(instruction, "CLI");
	CPU->P &= ~(FLAG_I);
}


/* execute_CLV: CLV command - Clear Overflow flag
 */
void execute_CLV(CPU_6502* CPU)
{
	/* CLV */
	strcpy(instruction, "CLV");
	CPU->P &= ~(FLAG_V);
}


/* execute_CMP: CMP command - Compare mem w/ A (A - M then set flags)
 */
void execute_CMP(enum MODES address_mode, CPU_6502* CPU)
{
	/* CMP - same as SBC except result isn't stored and V flag isn't changed */
	Base10toBase2(CPU->A, bin_operand1);
	if (address_mode == IMM) {
		/* Immediate - SBC #Operand */
		strcpy(instruction, "CMP #$");
		sprintf(append_int, "%.2X", CPU->operand);
		strcat(instruction, append_int);
		Base10toBase2(CPU->operand ^ 0xFF, bin_operand2);
	} else {
		strcpy(instruction, "CMP ");
		strcat(instruction, end);
		Base10toBase2(read_addr(CPU, CPU->target_addr) ^ 0xFF, bin_operand2);
	}
	unsigned carry_out = 0;
	full_adder(bin_operand1, bin_operand2, 1, &carry_out, bin_result);
	set_or_clear_CARRY(carry_out);
	/* Store difference in operand member to update flags */
	CPU->operand = Base2toBase10(bin_result, 0); // Result is discarded
	update_FLAG_N(CPU->operand);
	update_FLAG_Z(CPU->operand);
}


/* execute_CPX: CPX command - Compare mem w/ X (X - M then set flags)
 */
void execute_CPX(enum MODES address_mode, CPU_6502* CPU)
{
	Base10toBase2(CPU->X, bin_operand1);
	if (address_mode == IMM) {
		/* Immediate - SBC #Operand */
		strcpy(instruction, "CPX #$");
		sprintf(append_int, "%.2X", CPU->operand);
		strcat(instruction, append_int);
		Base10toBase2(CPU->operand ^ 0xFF, bin_operand2);
	} else {
		strcpy(instruction, "CPX ");
		strcat(instruction, end);
		Base10toBase2(read_addr(CPU, CPU->target_addr) ^ 0xFF, bin_operand2);
	}
	unsigned carry_out = 0;
	full_adder(bin_operand1, bin_operand2, 1, &carry_out, bin_result);
	set_or_clear_CARRY(carry_out);
	CPU->operand = Base2toBase10(bin_result, 0);
	update_FLAG_N(CPU->operand);
	update_FLAG_Z(CPU->operand);
}


/* execute_CPY: CPY command - Compare mem w/ Y (Y - M then set flags)
 */
void execute_CPY(enum MODES address_mode, CPU_6502* CPU)
{
	Base10toBase2(CPU->Y, bin_operand1);
	if (address_mode == IMM) {
		/* Immediate - SBC #Operand */
		strcpy(instruction, "CPY #$");
		sprintf(append_int, "%.2X", CPU->operand);
		strcat(instruction, append_int);
		Base10toBase2(CPU->operand ^ 0xFF, bin_operand2);
	} else {
		strcpy(instruction, "CPY ");
		strcat(instruction, end);
		Base10toBase2(read_addr(CPU, CPU->target_addr) ^ 0xFF, bin_operand2);
	}
	unsigned carry_out = 0;
	full_adder(bin_operand1, bin_operand2, 1, &carry_out, bin_result);
	set_or_clear_CARRY(carry_out);
	CPU->operand = Base2toBase10(bin_result, 0);
	update_FLAG_N(CPU->operand);
	update_FLAG_Z(CPU->operand);
}


/* execute_SEC: SEC command - Set Carry flag (C = 1)
 */
void execute_SEC(CPU_6502* CPU)
{
	/* SEC */
	strcpy(instruction, "SEC");
	CPU->P |= FLAG_C;
}


/* execute_SED: SED command - Set Decimal Mode (Decimal mode not supported in NES) 
 */
void execute_SED(CPU_6502* CPU)
{
	/* SED */
	strcpy(instruction, "SED");
	CPU->P |= FLAG_D;
}


/* execute_SEI: SEI command - Set Interrupt disable bit (I = 1)
 */
void execute_SEI(CPU_6502* CPU)
{
	/* SEI */
	strcpy(instruction, "SEI");
	CPU->P |= FLAG_I;
}


/***************************
 * SYSTEM                  *
 * *************************/

/* execute_BRK: BRK command - Fore Break - Store PC & P (along w/ X, Y & A) 
 */
void execute_BRK(void)
{
	/* 2 Pushes onto stack
	 * A) PC + 2 (2 seperate pushes as stack = 8 bits vs 16 bit PC)
	 * B) Status Register (w/ bits 4 & 4 set)
	 *
	 * - also set I in Status reg
	 * RTI does the inverese POPs A & B
	 */

	/* PC + 2 is pushed onto stack - always PUSH high byte first */
	strcpy(instruction, "BRK");
	PUSH((uint8_t) ((NES->PC + 2) >> 8)); /* Push PCH (PC High byte onto stack) */
	PUSH((uint8_t) (NES->PC + 2)); /* Push PCL (PC Low byte onto stack) */
	PUSH(NES->P | 0x30);           /* PUSH Staus Reg - w/ bits 4 & 5 set */
	NES->P |= FLAG_I;              /* Flag I is set */
	NES->PC = (read_addr(NES, 0xFFFF) << 8) | read_addr(NES, 0xFFFE);
}


/* execute_NOP: NOP command - Does nothing (No OPeration)
 */
void execute_NOP(void)
{
	strcpy(instruction, "NOP");
}

/* Non opcode interrupts */
void execute_IRQ(void)
{
	strcpy(instruction, "IRQ");
	/* PC is pushed onto stack, high byte first */
	PUSH((uint8_t) ((NES->PC + 2) >> 8)); /* Push PCH (PC High byte onto stack) */
	PUSH((uint8_t) (NES->PC + 2)); /* Push PCL (PC Low byte onto stack) */
	/* PUSH Staus Reg - w/ bits 4 & 5 clear */
	PUSH(NES->P & ~(0x30));
	/* Flag I is set */
	NES->P |= FLAG_I;
	NES->PC = (read_addr(NES, 0xFFFF) << 8) | read_addr(NES, 0xFFFE);
}


void execute_NMI(void)
{
	strcpy(end, " + NMI");
	strcat(instruction, end);
	/* PC is pushed onto stack, high byte first */
	PUSH((uint8_t) (NES->PC >> 8)); /* Push PCH (PC High byte onto stack) */
	PUSH((uint8_t) NES->PC); /* Push PCL (PC Low byte onto stack) */
	/* PUSH Staus Reg - w/ bits 4 & 5 clear */
	PUSH(NES->P & ~(0x30));
	/* Flag I is set */
	NES->P |= FLAG_I;
	NES->PC = (read_addr(NES, 0xFFFB) << 8) | read_addr(NES, 0xFFFA);
	NES->NMI_PENDING = 0;
}


void execute_DMA(void)
{
	/* Actual execution occurs via PPU */
	strcpy(end, " + DMA");
	strcat(instruction, end);
}
