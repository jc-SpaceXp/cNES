/*
 * Contains CPU architechture and functions
 */
#include "cpu.h"
#include "ppu.h"  // needed for read/write functions

CpuPpuShare* mmio_init(void)
{
	CpuPpuShare* i = malloc(sizeof(CpuPpuShare));
	if (!i) {
		fprintf(stderr, "Failed to allocate enough memory for PPU I/O\n");
		return i;
	}

	i->ppu_ctrl = 0;
	i->ppu_mask = 0;
	i->ppu_status = 0;
	i->oam_addr = 0;
	i->oam_data = 0;
	i->ppu_scroll = 0;
	i->ppu_addr = 0;
	i->ppu_data = 0;
	i->oam_dma = 0;

	i->nmi_pending = false;
	i->dma_pending = false;
	i->suppress_nmi = false;
	return i;
}

/* NES_CPU : Type 6502 CPU, used to initialise CPU
 */
Cpu6502* cpu_init(uint16_t pc_init, CpuPpuShare* cp)
{
	Cpu6502* i = malloc(sizeof(Cpu6502));
	if (!i) {
		fprintf(stderr, "Failed to allocate enough memory for CPU\n");
		return i;
	}

	i->cpu_ppu_io = cp;
	i->PC = pc_init;
	i->Stack = 0xFD; // After startup stack pointer is FD
	i->Cycle = 0;
	i->P = 0x24;
	i->A = 0;
	i->X = 0;
	i->Y = 0;
	i->old_Cycle = 0;
	memset(i->RAM, 0, MEMORY_SIZE); // Zero out RAM
	return i;
}


/* rename to set_PC */
void set_pc(Cpu6502* NES)
{
	NES->PC = return_little_endian(NES, 0xFFFC);
}

uint8_t read_from_cpu(Cpu6502* NES, uint16_t addr)  // add a ppu_pointer?
{
	if (addr < 0x2000) {
		return NES->RAM[addr & 0x7FF];
	} else if (addr < 0x4000) {
		return read_ppu_reg(addr & 0x2007, PPU);
	} else {
		return NES->RAM[addr]; /* catch-all */
	}
}

/* Return 16 bit address in little endian format */
uint16_t return_little_endian(Cpu6502* NES, uint16_t addr)
{
	return ((read_from_cpu(NES, addr + 1) << 8) | read_from_cpu(NES, addr));
}

void write_to_cpu(Cpu6502* NES, uint16_t addr, uint8_t val)
{
	if (addr < 0x2000) {
		NES->RAM[addr & 0x7FF] = val;
	} else if (addr < 0x4000) {
		NES->RAM[addr & 0x2007] = val;
		write_ppu_reg(addr & 0x2007, val, PPU, NES);
	} else if (addr == 0x4014) {
		write_ppu_reg(addr, val, PPU, NES);
	} else {
		NES->RAM[addr] = val;
	}
}

void cpu_mem_viewer(Cpu6502* NES)
{
	printf("\n##################### CPU RAM #######################\n");
	printf("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
	unsigned int addr = 0;
	unsigned int mem = 0;
	while (addr < 2048) {
		printf("%.4X: ", addr << 4);
		for (int x = 0; x < 16; x++) {
			printf("%.2X ", NES->RAM[mem]);
			++mem;
		}
		printf("\n");
		++addr;
	}
}
