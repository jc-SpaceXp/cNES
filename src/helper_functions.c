/*
 * Generic Functions 
 */

#include "helper_functions.h"



/***************************
 * FETCH OPCODE            *
 * *************************/

/* get_op_IMM : fetches operand based on IMM address modes
 */
void get_op_IMM(CPU_6502* CPU) // change to uint8_t
{
	/* Immediate - XXX #Operand */
	CPU->operand = read_addr(CPU, CPU->PC + 1);
	CPU->PC += 2; /* Update PC */
}


/* get_op_ZP_offest : fetches operand based on ZPX/ZPY address modes
 */
void get_op_ZP_offset(uint8_t offset, CPU_6502* CPU)
{
	/* Zero Page X or Y - XXX operand, X/Y */
	CPU->target_addr = (uint8_t) (read_addr(CPU, CPU->PC + 1) + offset);
	/* Debugger */
	sprintf(append_int, "%.2X", CPU->target_addr - offset);
	strcpy(end, "$");
	strcat(end, append_int);
	CPU->PC += 2; /* Update PC */
}


/* get_op_ABS_offest : fetches operand based on ABS/ABSX/ABSY address modes
 */
void get_op_ABS_offset(uint8_t offset, CPU_6502* CPU)
{
	/* Absolute (modes) - XXX operand  or XXX operand, X/Y */
	CPU->target_addr = fetch_16(CPU, CPU->PC + 1);
	CPU->target_addr = (uint16_t) (CPU->target_addr + offset);
	/* Debugger */
	sprintf(append_int, "%.4X", CPU->target_addr - offset);
	strcpy(end, "$");
	strcat(end, append_int);
	CPU->PC += 3; /* Update PC */
}


/* get_op_IND : fetches operand based on IND address mode
 */
void get_op_IND(CPU_6502* CPU)
{
	/* Indirect - JMP (operand) - 2 Byte address */
	CPU->target_addr = fetch_16(CPU, CPU->PC + 1);
	CPU->addr_lo = read_addr(CPU, CPU->target_addr); /* PC low bute */
	if ((CPU->target_addr & 0x00FF) == 0x00FF) {
		/* JUMP BUG */
		CPU->addr_hi = read_addr(CPU, CPU->target_addr & 0xFF00); /* PC high byte */
	} else {
		CPU->addr_hi = read_addr(CPU, CPU->target_addr + 1); /* PC high byte */
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
void get_op_INDX(CPU_6502* CPU)
{
	/* Indirect X - XXX (operand, X ) - 2 Byte address (Zero-Page) */
	CPU->target_addr = fetch_16(CPU, CPU->PC + 1 + CPU->X);
	/* Debugger */
	sprintf(append_int, "%.2X", read_addr(CPU, CPU->PC + 1));
	strcpy(end, "($");
	strcat(end, append_int);
	strcat(end, ",X)");
	CPU->PC += 2; /* Update PC */
}


/* get_op_INDY : fetches operand based on INDY address mode
 */
void get_op_INDY(CPU_6502* CPU)
{
	/* Indirect Y - XXX (operand), Y - 2 Byte address (Zero-Page) */
	CPU->target_addr = read_addr(CPU, CPU->PC + 1); // Zero-lage address
	CPU->target_addr = fetch_16(CPU, CPU->target_addr);
	CPU->target_addr = (uint16_t) (CPU->Y + CPU->target_addr); /* get target address */
	/* Debugger */
	sprintf(append_int, "%.2X", read_addr(CPU, CPU->PC + 1));
	strcpy(end, "($");
	strcat(end, append_int);
	strcat(end, "),Y");
	CPU->PC += 2; /* Update PC */
}

/* using getch_16() for IND instructions and ABS causes some errors
 * most likely due to the code layout
 * will have to look into some more
 */

// Determines if a page cross has occured for a certain instruction
unsigned PAGE_CROSS(unsigned val1, unsigned val2)
{
	return ((val1 & 0xFF00) == (val2 & 0xFF00)) ? 0 : 1;
}

/***************************
 * OTHER                   *
 * *************************/

/* Return Status */
void RET_NES_CPU(void)
{
	printf("%-6.4X ", NES->old_PC);
	printf("%-20s ", instruction);
	printf("A:%.2X ", NES->old_A);
	printf("X:%.2X ", NES->old_X);
	printf("Y:%.2X ", NES->old_Y);
	printf("P:%.2X ", NES->old_P);
	printf("SP:%.2X ", NES->old_Stack);
	printf("CPU:%.4d", NES->old_Cycle);
}

void transfer_cpu(void)
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
 * ADD & SUB RELATED FUNCS *
 * *************************/

/* Base10toBase2 : converts an 8 bit number into it's binary equivalent
 */
void Base10toBase2(uint8_t quotient, int *bin_array)
{
	memset(bin_array, 0, 8*sizeof(int)); /* reset array to 0 */
	while ((quotient != 0) && (bin_array != NULL)) {
		*bin_array = quotient % 2;
		quotient /= 2;
		++bin_array;
	}
}

/* Base2toBase10 : converts an 8 bit array into it's decimal equivalent
 */
unsigned int Base2toBase10(int *bin_array, unsigned int dec_out)
{
	unsigned counter = 0;
	power2 = 1;
	while (counter == 0) {
		dec_out = *bin_array;
		++counter;
		++bin_array;
	}	
	for (counter = 1; counter < 8; counter++) {
		power2 *= 2;
		dec_out = dec_out + (power2 * (*bin_array));
		++bin_array;
	}
	return dec_out;
}

/* full_adder : full adder w/ carry in and out functionality
 *              returns cOUT --> mask into NES->P
 */
void full_adder(int *bin_sum1, int *bin_sum2, int cIN, unsigned *cOUT, int *result)
{
	*cOUT = 0; /* Reset cOUT (tmp in this case) */
	unsigned counter = 0; /* for loop is cleaner - but breaks cOUT */
	while (counter < 8) {
		/* Fetch operand */
		result[counter] = (*bin_sum1 ^ *bin_sum2) ^ cIN;
		*cOUT = (cIN & (*bin_sum1 ^ *bin_sum2)) | (*bin_sum1 & *bin_sum2);
		++counter;
		++bin_sum1;
		++bin_sum2;
		cIN = *cOUT; /* update carry in */
	}
}

/***************************
 * STACK                   *
 * *************************/

/* Genric Push function */
void PUSH(uint8_t value)
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
uint8_t PULL(void)
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

void update_FLAG_Z(uint8_t result)
{
	/* Zero Flag Test */
	if (result == 0) {
		NES->P |= FLAG_Z; /* Set Z */
	} else {
		NES->P &= ~(FLAG_Z); /* Clear Z */
	}
}


void update_FLAG_N(uint8_t result)
{
	/* Negative Flag Test */
	if (result >= 0x00 && result <= 0x7F) {
		NES->P &= ~(FLAG_N); /* Clear N */
	} else {
		NES->P |= FLAG_N; /* Set N */
	}
}


/* Parameters = 2 binary operands and then the result */
void update_FLAG_V(int *bin_array1, int *bin_array2, int *result)
{
	/* Overflow Flag Test */
	if (bin_array1[7] != bin_array2[7]) {
		/* Overflow is impossible if MSB (signs) are different */
		NES->P &= ~(FLAG_V); /* Clear V */
	} else if (bin_array1[7] != result[7]) {
		NES->P |= FLAG_V; /* Set V */
	} else {
		NES->P &= ~(FLAG_V); /* Clear V */
	}
}


void update_FLAG_C(uint8_t cOUT)
{
	/* Carry Flag Update */
	if (cOUT == 1) {
		NES->P |= cOUT; /* carry flag = 0th bit hence no shift */
	}

}
/* value is either 0x00 or 0x01 - catch all statement doesn't work */
void set_or_clear_CARRY(unsigned value)
{
	/* Sets a FLAG if = 1, else if 0 then flag is cleared */ 
	if (value == 0) {
		NES->P &= (value | 0xFE); /* Need 0xFE as after AND need to preserve NES-> P values */ 
	} else if (value == 1) {
		NES->P |= value;
	}
}
