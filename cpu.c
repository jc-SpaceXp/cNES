/*
 * Contains CPU Architechture
 */
#include "cpu.h"

/* NES_CPU : Type 6501 CPU, used to initialise CPU
 */
CPU_6502* NES_CPU(uint16_t pc_init)
{
	CPU_6502 *i = malloc(sizeof(CPU_6502));
	i->PC = pc_init;
	i->SP = &(i->RAM[SP_START + SP_OFFSET]);
	i->P = 0x00;
	memset(i->RAM, 0, NES_RAM_SIZE); /* Initialise RAM to 0 */
	return i;
}
