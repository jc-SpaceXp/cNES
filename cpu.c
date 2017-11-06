/*
 * Contains CPU Architechture
 */
#include "cpu.h"
/* #include "ppu.h" */

/* NES_CPU : Type 6502 CPU, used to initialise CPU
 */
CPU_6502* NES_CPU(uint16_t pc_init)
{
	CPU_6502 *i = malloc(sizeof(CPU_6502));
	i->PC = pc_init;
	i->Stack = 0xFD; /* Hard coded will change */
	i->P = 0x24;
	i->A = 0;
	i->X = 0;
	i->Y = 0;
	memset(i->RAM, 0, MEMORY_SIZE); /* Initialise RAM to 0 */
	return i;
}

/*********** ADD TO CPU.H **************/
/*** WILL REPLACE get_op functions *******/
uint8_t read_addr(CPU_6502* NES, uint16_t addr)
{
	if (addr < 0x2000) {
		return NES->RAM[addr & 0x7FF];
	} /* else if (addr < 0x4000) {
		return read_PPU_Reg(addr &0x2007);
		*/
	else {
		return NES->RAM[addr]; /* catch-all */
	}
}

/* Same as get_op_ABS - to make ABS_offset add NES->X or NES-Y to addr 
 * in calling function ---> fetch_16LE(NES, addr + NEs->X)
 */
uint16_t fetch_16(CPU_6502* NES, uint16_t addr)
{
	/* returns Little Endian format */
	return ((read_addr(NES, addr + 1) << 8) | read_addr(NES, addr));
}

/* indirect addressing - with JMP bug - NOT COMPLETED */
uint16_t fetch_16_IND(CPU_6502* NES, uint16_t addr)
{
	/* returns Little Endian format */
	return ((read_addr(NES, addr + 1) << 8) | read_addr(NES, addr));
}

void write_addr(CPU_6502* NES, uint16_t addr, uint8_t val)
{
	if (addr < 0x2000) {
		NES->RAM[addr & 0x7FF] = val;
	} /* else if (addr < 0x4000) {
		NES->RAM[addr &0x2007] =  val; Get a non-mirrored address
	}  */
	else {
		NES->RAM[addr] = val;
	}
}
