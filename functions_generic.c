/*
 * Generic Functions 
 */

#include "functions_generic.h"



/***************************
 * FETCH OPCODE            *
 * *************************/

/* get_op_IMM : fetches operand based on IMM address modes
 */
size_t get_op_IMM(uint8_t *ptr_code)
{
	/* Immediate - XXX #Operand */
	operand = *(ptr_code+1);
	NES->PC += 2; /* Update PC */
	return operand;
}


/* get_op_ZP_offest : fetches operand based on ZPX/ZPY address modes
 */
size_t get_op_ZP_offset(uint8_t *ptr_code, uint8_t offset)
{
	/* Zero Page X or Y - XXX operand, X/Y */
	operand = *(ptr_code+1) + offset;
	NES->PC += 2; /* Update PC */
	return operand;
	/* return NES->RAM[operand] -- makes us not use address modes for commands */
	/* allows us to have one generic command */
}


/* get_op_ABS_offest : fetches operand based on ABS/ABSX/ABSY address modes
 */
size_t get_op_ABS_offset(uint8_t *ptr_code, uint8_t offset)
{
	/* Absolute (modes) - XXX operand  or XXX operand, X/Y */
	operand = ((uint16_t) (*(ptr_code+2) << 8) | *(ptr_code+1)) + offset;
	NES->PC += 3; /* Update PC */
	return operand;
}


/* get_op_IND : fetches operand based on IND address mode
 */
size_t get_op_IND(uint8_t *ptr_code, CPU_6502 *NESCPU)
{
	/* Indirect - JMP (operand) - 2 Byte address */
	operand = ((uint16_t) (*(ptr_code+2) << 8) | *(ptr_code+1));
	operand = NESCPU->RAM[operand]; /* PC low bute */
	tmp = NESCPU->RAM[operand + 1]; /* PC high byte */
	operand = (uint16_t) (tmp << 8) | operand; /* get target address (little endian) */
	NES->PC += 3; /* Update PC */
	return operand;
}


/* get_op_INDX : fetches operand based on INDX address mode
 */
size_t get_op_INDX(uint8_t *ptr_code, CPU_6502 *NESCPU)
{
	/* Indirect X - XXX (operand, X ) - 2 Byte address */
	operand = NESCPU->RAM[*(ptr_code+1) + NESCPU->X]; /* sum address (LSB) */
	tmp = NESCPU->RAM[*(ptr_code+1) + NESCPU->X + 1]; /* Sum address + 1 (MSB) */
	operand = (uint16_t) (tmp << 8) | operand; /* get target address (little endian) */
	NES->PC += 2; /* Update PC */
	return operand;
}


/* get_op_INDY : fetches operand based on INDY address mode
 */
size_t get_op_INDY(uint8_t *ptr_code, CPU_6502 *NESCPU)
{
	/* Indirect Y - XXX (operand), Y - 2 Byte address */
	operand = NESCPU->RAM[*(ptr_code+1)]; /* sum address (LSB) */
	tmp = NESCPU->RAM[*(ptr_code+1) + 1]; /* sum address + 1 (MSB) */
	operand = (uint16_t) (tmp << 8) | operand; /* get little endian */
	operand += NESCPU->Y; /* get target address */
	NES->PC += 2; /* Update PC */
	return operand;
}

/***************************
 * STATUS                  *
 * *************************/

/* Return Status */
void RET_NES_CPU(void)
{
	printf("A:%.2X ", NES->A);
	printf("X:%.2X ", NES->X);
	printf("Y:%.2X ", NES->Y);
	printf("P:%.2X ", NES->P);
	printf("SP:%.2X ", NES->Stack);
	printf("PC:%.4X\n", NES->PC);
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
	counter = 0;
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
void full_adder(int *bin_sum1, int *bin_sum2, int cIN, uint8_t *cOUT, int *result)
{
	*cOUT = 0; /* Reset cOUT (tmp in this case) */
	counter = 0; /* for loop is cleaner - but breaks cOUT */
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
		printf("Full stack - can't PUSH\n");
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
		printf("Empty stack - can't PULL\n");
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
	/* if (int8_t) result >= 0) - both execute in ~ same time */
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
void set_or_clear_CARRY(uint8_t value)
{
	/* Sets a FLAG if = 1, else if 0 then flag is cleared */ 
	if (value == 0) {
		NES->P &= (value | 0xFE); /* Need 0xFE as after AND need to preserve NES-> P values */ 
	} else if (value == 1) {
		NES->P |= value;
	}
}
