/*
 * Generic Functions 
 */

#include "helper_functions.h"


/***************************
 * FETCH OPCODE            *
 * *************************/

/* get_op_IMM : fetches operand based on IMM address modes
 */
void get_IMM_byte(Cpu6502* CPU) // change to uint8_t
{
	/* Immediate - XXX #Operand */
	CPU->operand = read_from_cpu(CPU, CPU->PC + 1);
	CPU->PC += 2; /* Update PC */
}


/* get_op_ZP_offest : fetches operand based on ZPX/ZPY address modes
 */
void get_ZP_offset_address(uint8_t offset, Cpu6502* CPU)
{
	/* Zero Page X or Y - XXX operand, X/Y */
	CPU->target_addr = (uint8_t) (read_from_cpu(CPU, CPU->PC + 1) + offset);
	/* Debugger */
	sprintf(append_int, "%.2X", CPU->target_addr - offset);
	strcpy(end, "$");
	strcat(end, append_int);
	CPU->PC += 2; /* Update PC */
}


/* get_op_ABS_offest : fetches operand based on ABS/ABSX/ABSY address modes
 */
void get_ABS_offset_address(uint8_t offset, Cpu6502* CPU)
{
	/* Absolute (modes) - XXX operand  or XXX operand, X/Y */
	CPU->target_addr = return_little_endian(CPU, CPU->PC + 1);
	CPU->target_addr = (uint16_t) (CPU->target_addr + offset);
	/* Debugger */
	sprintf(append_int, "%.4X", CPU->target_addr - offset);
	strcpy(end, "$");
	strcat(end, append_int);
	CPU->PC += 3; /* Update PC */
}


/* get_op_IND : fetches operand based on IND address mode
 */
void get_IND_address(Cpu6502* CPU)
{
	/* Indirect - JMP (operand) - 2 Byte address */
	CPU->target_addr = return_little_endian(CPU, CPU->PC + 1);
	CPU->addr_lo = read_from_cpu(CPU, CPU->target_addr); /* PC low bute */
	if ((CPU->target_addr & 0x00FF) == 0x00FF) {
		/* JUMP BUG */
		CPU->addr_hi = read_from_cpu(CPU, CPU->target_addr & 0xFF00); /* PC high byte */
	} else {
		CPU->addr_hi = read_from_cpu(CPU, CPU->target_addr + 1); /* PC high byte */
	}
	CPU->target_addr = (uint16_t) (CPU->addr_hi << 8) | CPU->addr_lo; /* get target address (little endian) */
	/* Debugger */
	sprintf(append_int, "%.X", CPU->target_addr);
	strcpy(end, "($");
	strcat(end, append_int);
	strcat(end, ")");
	CPU->PC += 3; /* Update PC */
}


/* get_op_INDX : fetches operand based on INDX address mode
 */
void get_INDX_address(Cpu6502* CPU)
{
	/* Indirect X - XXX (operand, X ) - 2 Byte address (Zero-Page) */
	CPU->target_addr = return_little_endian(CPU, CPU->PC + 1 + CPU->X);
	/* Debugger */
	sprintf(append_int, "%.2X", read_from_cpu(CPU, CPU->PC + 1));
	strcpy(end, "($");
	strcat(end, append_int);
	strcat(end, ",X)");
	CPU->PC += 2; /* Update PC */
}


/* get_op_INDY : fetches operand based on INDY address mode
 */
void get_INDY_address(Cpu6502* CPU)
{
	/* Indirect Y - XXX (operand), Y - 2 Byte address (Zero-Page) */
	CPU->target_addr = read_from_cpu(CPU, CPU->PC + 1); // Zero-lage address
	CPU->target_addr = return_little_endian(CPU, CPU->target_addr);
	CPU->target_addr = (uint16_t) (CPU->Y + CPU->target_addr); /* get target address */
	/* Debugger */
	sprintf(append_int, "%.2X", read_from_cpu(CPU, CPU->PC + 1));
	strcpy(end, "($");
	strcat(end, append_int);
	strcat(end, "),Y");
	CPU->PC += 2; /* Update PC */
}

// Determines if a page cross has occured for a certain instruction
unsigned page_cross_penalty(unsigned address_1, unsigned address_2)
{
	return ((address_1 & 0xFF00) == (address_2 & 0xFF00)) ? 0 : 1;
}

/***************************
 * OTHER                   *
 * *************************/

/* Return Status */
void log_cpu_info(Cpu6502* NES)
{
	printf("%-6.4X ", NES->old_PC);
	printf("%-20s ", instruction);
	printf("A:%.2X ", NES->old_A);
	printf("X:%.2X ", NES->old_X);
	printf("Y:%.2X ", NES->old_Y);
	printf("P:%.2X ", NES->old_P);
	printf("SP:%.2X ", NES->old_Stack);
	printf("CPU:%.4u", NES->old_Cycle);
}

void update_cpu_info(Cpu6502* NES)
{
	NES->old_A = NES->A;
	NES->old_X = NES->X;
	NES->old_Y = NES->Y;
	NES->old_P = NES->P;
	NES->old_Stack = NES->Stack;
	NES->old_PC = NES->PC;
	NES->old_Cycle = NES->Cycle;
}

/***************************
 * STACK                   *
 * *************************/

/* Genric Push function */
void stack_push(Cpu6502* NES, uint8_t value)
{
	/* SP_START - 1 - as Stack = Empty Descending */
	/* FIX LIMIT CHECK */
	if (NES->Stack == 0x00) {
		/* Overflow */
		printf("Full stack - can't PUSH\n"); // Instead wrap-around
	} else {
		NES->RAM[SP_START + NES->Stack] = value;
		--NES->Stack;
	}
}


/* Genric Pop (Pull) function */
uint8_t stack_pull(Cpu6502* NES)
{
	/* FIX LIMIT CHECK */
	if (NES->Stack == SP_START) {
		/* Underflow */
		printf("Empty stack - can't PULL\n"); // Instead wrap-around
	} else {
		++NES->Stack;
		return (NES->RAM[SP_START + NES->Stack]);
	}
	return 0; /* fail-safe - if-else branch should return a value */
}

/***************************
 * FLAGS                   *
 * *************************/

/* Bits : 7 ----------> 0 */
/* Flags: N V - - D I Z C */

void update_flag_z(Cpu6502* NES, uint8_t result)
{
	/* Zero Flag Test */
	if (!result) {
		NES->P |= FLAG_Z; /* Set Z */
	} else {
		NES->P &= ~FLAG_Z; /* Clear Z */
	}
}


void update_flag_n(Cpu6502* NES, uint8_t result)
{
	/* Negative Flag Test */
	if (result >> 7) {
		NES->P |= FLAG_N; /* Set N */
	} else {
		NES->P &= ~FLAG_N; /* Clear N */
	}
}


/* Parameters = 2 binary operands and then the result */
void update_flag_v(Cpu6502* NES, bool overflow)
{
	/* Overflow Flag Test */
	if (overflow) {
		NES->P |= FLAG_V; /* Set V */
	} else {
		NES->P &= ~FLAG_V; /* Clear V */
	}
}


void update_flag_c(Cpu6502* NES, int carry_out)
{
	if (carry_out) { // Carry out = result >> 8 (9th bit in ADC / SBC calc
		NES->P |= FLAG_C; /* Set C */
	} else {
		NES->P &= ~FLAG_C; /* Clear C */
	}
}
