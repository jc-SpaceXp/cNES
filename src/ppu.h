#ifndef __NES_PPU__
#define __NES_PPU__

#include "extern_structs.h"
#include "cpu.h"

#include <stdbool.h>

#ifndef KiB
#define KiB (1024U)
#endif /* KiB */


enum PpuMemoryTypes {
	VRAM,
	PRIMARY_OAM,
	SECONDARY_OAM,
	PATTERN_TABLE_1
};

/* Initialise Function */
Ppu2C02* ppu_init(CpuPpuShare* cp);
void ppu_reset(int start, Ppu2C02* p, const Cpu6502* cpu); /* Emulates reset/warm-up of PPU */

/* Debug Functions */
void debug_ppu_regs(Cpu6502* cpu);
void ppu_mem_hexdump_addr_range(Ppu2C02* p, const enum PpuMemoryTypes ppu_mem, unsigned start_addr, uint16_t end_addr);

/* Read & Write Functions */
uint8_t read_ppu_reg(const uint16_t addr, Cpu6502* cpu); /* For addresses exposed to CPU */
void delay_write_ppu_reg(const uint16_t addr, const uint8_t data, Cpu6502* cpu); /* For addresses exposed to CPU */
void write_ppu_reg(const uint16_t addr, const uint8_t data, Cpu6502* cpu); /* For addresses exposed to CPU */


void clock_ppu(Ppu2C02* p, Cpu6502* cpu, Display* nes_screen, const bool no_logging);


#endif /* __NES_PPU__ */
