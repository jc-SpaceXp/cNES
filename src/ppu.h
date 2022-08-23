#ifndef __NES_PPU__
#define __NES_PPU__

#include "extern_structs.h"
#include "cpu.h"

#include <stdbool.h>

#ifndef KiB
#define KiB (1024U)
#endif /* KiB */


enum PpuMemoryTypes {
	PRIMARY_OAM,
	SECONDARY_OAM,
	PATTERN_TABLE_1
};

/* Initialise Function */
Ppu2A03* ppu_init(CpuPpuShare* cp);
void ppu_reset(int start, Ppu2A03* p, Cpu6502* cpu); /* Emulates reset/warm-up of PPU */

/* Debug Functions */
void debug_ppu_regs(Cpu6502* cpu);
void ppu_mem_16_byte_viewer(Ppu2A03* PPU, unsigned start_addr, unsigned total_rows);
void OAM_viewer(Ppu2A03* PPU, enum PpuMemoryTypes ppu_mem); // rename to VRAM viewer?

/* Read & Write Functions */
uint8_t read_ppu_reg(uint16_t addr, Cpu6502* cpu); /* For addresses exposed to CPU */
void delay_write_ppu_reg(uint16_t addr, uint8_t data, Cpu6502* cpu); /* For addresses exposed to CPU */
void write_ppu_reg(uint16_t addr, uint8_t data, Cpu6502* cpu); /* For addresses exposed to CPU */


void clock_ppu(Ppu2A03* p, Cpu6502* cpu, Display* nes_screen);


#endif /* __NES_PPU__ */
