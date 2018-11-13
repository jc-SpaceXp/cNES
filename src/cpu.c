/*
 * Contains CPU architechture and functions
 */
#include "cpu.h"
#include "ppu.h"

/* NES_CPU : Type 6502 CPU, used to initialise CPU
 */
CPU_6502* NES_CPU(uint16_t pc_init)
{
	CPU_6502 *i = malloc(sizeof(CPU_6502));
	i->PC = pc_init;
	i->Stack = 0xFD; // After startup stack pointer is FD
	i->Cycle = 0;
	i->P = 0x24;
	i->A = 0;
	i->X = 0;
	i->Y = 0;
	i->NMI_PENDING = 0;
	memset(i->RAM, 0, MEMORY_SIZE); // Zero out RAM
	return i;
}

/* rename to set_PC */
void NES_PC(CPU_6502* NES)
{
	NES->PC = fetch_16(NES, 0xFFFC);
}

uint8_t read_addr(CPU_6502* NES, uint16_t addr)
{
	if (addr < 0x2000) {
		return NES->RAM[addr & 0x7FF];
	} else if (addr < 0x4000) {
		read_PPU_Reg(addr & 0x2007, PPU);
	} else {
		return NES->RAM[addr]; /* catch-all */
	}
}

/* Return 16 bit address in little endian format */
uint16_t fetch_16(CPU_6502* NES, uint16_t addr)
{
	return ((read_addr(NES, addr + 1) << 8) | read_addr(NES, addr));
}

void write_addr(CPU_6502* NES, uint16_t addr, uint8_t val)
{
	if (addr < 0x2000) {
		NES->RAM[addr & 0x7FF] = val;
	} else if (addr < 0x4000) {
		NES->RAM[addr & 0x2007] = val;
		write_PPU_Reg(addr & 0x2007, val, PPU);
	} else if (addr == 0x4014) {
		write_PPU_Reg(addr, val, PPU);
	} else {
		NES->RAM[addr] = val;
	}
}

void cpu_ram_viewer(void)
{
	printf("\n##################### CPU RAM #######################\n");
	printf("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
	unsigned int addr = 0;
	unsigned int mem = 0;
	while (addr < 50) {
		printf("%.4X: ", addr << 4);
		for (int x = 0; x < 16; x++) {
			printf("%.2X ", NES->RAM[mem]);
			++mem;
		}
		printf("\n");
		++addr;
	}
}
