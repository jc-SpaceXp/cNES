/*
 *  Memory Header File
 *
 */

#ifndef __NES_MEM__
#define __NES_MEM__

#include "cpu_struct.h"
/* Memory Constants */

/****************
 * RAM          *
 * **************/
uint8_t read_RAM(uint16_t addr)
{
	return NES->RAM[addr & 0x07FF];
}

void write_RAM(uint16_t addr, uint8_t value)
{
	NES->RAM[addr & 0x07FF] = value; /* Non-Mirrored address */
}

void read_BIG_RAM(uint16_t addr_lo, uint16_t addr_hi)
{
	/* ........ */
}

/* include the ram functions from functions_generic.h */

/****************
 * Read & Write *
 * **************/


/****************
 * STACK        *
 * **************/

/* Genric Push function */
void PUSH(uint8_t value)
{
	/* SP_START - 1 - as Stack = Empty Descending */
	if (NES->SP == &NES->RAM[SP_START - 1]) {
		/* Overflow */
		printf("Full stack - can't PUSH\n");
	} else {
		*NES->SP = value;
		--NES->SP;
	}
}


/* Genric Pop (Pull) function */
uint8_t PULL(void)
{
	if (NES->SP == &NES->RAM[SP_START + SP_OFFSET]) {
		/* Underflow */
		printf("Empty stack - can't PULL\n");
	} else {
		++NES->SP;
		return *NES->SP;
	}
	return 0; /* fail-safe - if-else branch should return a value */
}

#endif
