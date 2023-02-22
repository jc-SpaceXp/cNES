#include "mappers.h"
#include "cart.h"
#include "cpu.h"
#include "ppu.h"
#include "cpu_ppu_interface.h"
#include "cpu_mapper_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Static prototype functions
static void mmc1_reg_write(Cpu6502* cpu, const uint16_t addr, const uint8_t val);
static void mapper_000(Cartridge* cart, Cpu6502* cpu, Ppu2C02* ppu);
static void mapper_001(Cartridge* cart, Cpu6502* cpu, const Ppu2C02* ppu);

// Helper functions
static inline void set_prg_rom_bank_1(Cpu6502* cpu, const unsigned prg_bank_offset, const unsigned kib_size)
{
	memcpy(&cpu->mem[0x8000]
		  , cpu->cpu_mapper_io->prg_rom->data + ((prg_bank_offset) * (kib_size))
		  , kib_size);
}

static inline void set_prg_rom_bank_2(Cpu6502* cpu, const unsigned prg_bank_offset)
{
	memcpy(&cpu->mem[0xC000]
		  , cpu->cpu_mapper_io->prg_rom->data + ((prg_bank_offset) * (16 * KiB))
		  , 16 * KiB);
}

static inline void set_chr_bank_1(Cpu6502* cpu, const unsigned chr_bank_offset, const unsigned kib_size)
{
	memcpy(&cpu->cpu_ppu_io->vram->pattern_table_0[0x0000]
		  , cpu->cpu_mapper_io->chr->data + ((chr_bank_offset) * (kib_size))
		  , kib_size);
}

static inline void set_chr_bank_2(Cpu6502* cpu, const unsigned chr_bank_offset)
{
	memcpy(&cpu->cpu_ppu_io->vram->pattern_table_1[0x0000]
		  , cpu->cpu_mapper_io->chr->data + ((chr_bank_offset) * (4 * KiB))
		  , 4 * KiB);
}

// allow writes to PRG RAM / WRAM (CPU: 0x6000 to 0x7FFF) if enabled
// calling function handles the address space
static void prg_ram_writes(bool enable_prg_ram, Cpu6502* cpu, uint16_t addr, uint8_t val)
{
	if (enable_prg_ram) {
		cpu->mem[addr] = val;
	}
}

void mapper_write(Cpu6502* cpu, uint16_t addr, uint8_t val)
{
	switch (cpu->cpu_mapper_io->mapper_number) {
	case 1:
		if (addr >= 0x8000) {
			mmc1_reg_write(cpu, addr, val);
		} else if (addr >= 0x6000) {
			prg_ram_writes(cpu->cpu_mapper_io->enable_prg_ram, cpu, addr, val);
		}
		break;
	default:
		// ignore writes if the mapper doesn't use cpu registers to write to i.e. mapper 0
		break;
	}
}

static inline uint8_t cpu_open_bus(const Cpu6502* cpu)
{
	return cpu->data_bus;
}

static uint8_t prg_ram_reads(bool enable_prg_ram, const Cpu6502* cpu, uint16_t addr)
{
	uint8_t read_val = 0;

	if (enable_prg_ram) {
		read_val = cpu->mem[addr];
	} else {
		// if PRG RAM is disabled reads will return open bus behaviour
		read_val = cpu_open_bus(cpu);
	}

	return read_val;
}

uint8_t mapper_read(const Cpu6502* cpu, uint16_t addr)
{
	uint8_t read_val = 0;
	// Read from PRG ROM regardless of mapper
	if (addr >= 0x8000) {
		read_val = cpu->mem[addr];
		return read_val; // early return
	}

	switch (cpu->cpu_mapper_io->mapper_number) {
	case 0:
		read_val = cpu_open_bus(cpu);
		break;
	case 1:
		read_val = cpu_open_bus(cpu);

		if ((addr >= 0x6000) && (addr < 0x8000)) {
			read_val = prg_ram_reads(cpu->cpu_mapper_io->enable_prg_ram, cpu, addr);
		}
		break;
	default:
		break;
	}

	return read_val;
}

// used to change the nametable mirroring on the fly
static void set_nametable_mirroring(struct PpuMemoryMap* vram
                                   , PpuNametableMirroringType nametable_mirroring)
{
	if (nametable_mirroring == SINGLE_SCREEN_A) {
		vram->nametable_0 = &vram->nametable_A;
		vram->nametable_1 = &vram->nametable_A;
		vram->nametable_2 = &vram->nametable_A;
		vram->nametable_3 = &vram->nametable_A;
	} else if (nametable_mirroring == SINGLE_SCREEN_B) {
		vram->nametable_0 = &vram->nametable_B;
		vram->nametable_1 = &vram->nametable_B;
		vram->nametable_2 = &vram->nametable_B;
		vram->nametable_3 = &vram->nametable_B;
	} else if (nametable_mirroring == HORIZONTAL) {
		vram->nametable_0 = &vram->nametable_A;
		vram->nametable_1 = &vram->nametable_A;
		vram->nametable_2 = &vram->nametable_B;
		vram->nametable_3 = &vram->nametable_B;
	} else if (nametable_mirroring == VERTICAL) {
		vram->nametable_0 = &vram->nametable_A;
		vram->nametable_1 = &vram->nametable_B;
		vram->nametable_2 = &vram->nametable_A;
		vram->nametable_3 = &vram->nametable_B;
	}
}

void init_mapper(Cartridge* cart, Cpu6502* cpu, Ppu2C02* ppu)
{
	// init mirroring mapping
	set_nametable_mirroring(&ppu->vram, ppu->nametable_mirroring);
	switch (cpu->cpu_mapper_io->mapper_number) {
	case 0:
		mapper_000(cart, cpu, ppu);
		break;
	case 1:
		mapper_001(cart, cpu, ppu);
		break;
	default:
		fprintf(stderr, "Mapper %d isn't implemented\n", cpu->cpu_mapper_io->mapper_number);
		break;
	}
}


/* NROM mapper */
static void mapper_000(Cartridge* cart, Cpu6502* cpu, Ppu2C02* ppu)
{
	/* Load PRG_ROM into CPU program memory space */
	if (cart->prg_rom.size == (16 * KiB)) {
		memcpy(&cpu->mem[0x8000], cart->prg_rom.data, 16 * KiB); // First 16KiB
		memcpy(&cpu->mem[0xC000], cart->prg_rom.data, 16 * KiB); // Last 16KiB (Mirrored)
	} else {
		memcpy(&cpu->mem[0x8000], cart->prg_rom.data, 32 * KiB);
	}
	free(cart->prg_rom.data);

	/* Load CHR_ROM data into PPU VRAM */
	if (cart->chr.rom_size) {
		memcpy(&ppu->vram.pattern_table_0[0x0000], cart->chr.data, 4 * KiB);
		memcpy(&ppu->vram.pattern_table_1[0x0000], cart->chr.data + (4 * KiB), 4 * KiB);
	}
	free(cart->chr.data);
}


/* SxROM (MMC1) mapper */
// power on state
static void mapper_001(Cartridge* cart, Cpu6502* cpu, const Ppu2C02* ppu)
{
	(void) ppu; // suppress unused variable warning
	unsigned prg_rom_banks = cart->prg_rom.size / (16 * KiB);
	set_prg_rom_bank_1(cpu, 0, 16 * KiB);
	set_prg_rom_bank_2(cpu, prg_rom_banks - 1);
}

static void normalise_any_out_of_bounds_bank(unsigned* bank_select, unsigned total_banks)
{
	// Out of range bank
	if (*bank_select >= total_banks) {
		// ignore upper bits of bank number
		// banks should be aligned to a power of 2
		// otherwise calculation below should be "&= log2(total_banks)"
		*bank_select &= total_banks - 1;
	}
}


static void mmc1_reg_write(Cpu6502* cpu, const uint16_t addr, const uint8_t val)
{
	static unsigned write_count = 0;
	static unsigned buffer = 0;
	static unsigned write_cycle = 0;

	// ignore adjacent writes
	if (write_cycle == (cpu->cycle - 1)) {
		return;
	}

	// Process reset bit first
	if (val & 0x80) {
		write_count = 0;
		buffer = 0;

		unsigned prg_rom_banks = cpu->cpu_mapper_io->prg_rom->size / (16 * KiB);
		cpu->cpu_mapper_io->prg_high_bank_fixed = true;
		cpu->cpu_mapper_io->prg_rom_bank_size = 16;
		set_prg_rom_bank_2(cpu, prg_rom_banks - 1);

		write_cycle = cpu->cycle;  // update write_cycle
		return; // early return
	}

	// write lsb of val to write_count'th bit (bits 0 through 4)
	buffer |= (val & 0x01) << write_count;
	++write_count;
	write_cycle = cpu->cycle;  // update write_cycle

	if (write_count == 5) {
		if ((addr >= 0x8000) && (addr <= 0x9FFF)) {
			// reg 0: xxxC FHMM
			// MM bits
			switch (buffer & 0x03) {
			case 0x00: // 1-screen mirroring nametable 0
				*(cpu->cpu_ppu_io->nametable_mirroring) = SINGLE_SCREEN_A;
				set_nametable_mirroring(cpu->cpu_ppu_io->vram
				                       , *cpu->cpu_ppu_io->nametable_mirroring);
				break;
			case 0x01: // 1-screen mirroring nametable 1
				*(cpu->cpu_ppu_io->nametable_mirroring) = SINGLE_SCREEN_B;
				set_nametable_mirroring(cpu->cpu_ppu_io->vram
				                       , *cpu->cpu_ppu_io->nametable_mirroring);
				break;
			case 0x02: // 0b10 vertical mirroring
				*(cpu->cpu_ppu_io->nametable_mirroring) = VERTICAL;
				set_nametable_mirroring(cpu->cpu_ppu_io->vram
				                       , *cpu->cpu_ppu_io->nametable_mirroring);
				break;
			case 0x03: // 0b11 horizontal mirroring
				*(cpu->cpu_ppu_io->nametable_mirroring) = HORIZONTAL;
				set_nametable_mirroring(cpu->cpu_ppu_io->vram
				                       , *cpu->cpu_ppu_io->nametable_mirroring);
				break;
			}
			// H bit
			switch ((buffer >> 2) & 0x01) {
			case 0: // 0 = fixed lower bank
				cpu->cpu_mapper_io->prg_low_bank_fixed = true;
				cpu->cpu_mapper_io->prg_high_bank_fixed = false;
				break;
			case 1: // 1 = fixed upper bank
				cpu->cpu_mapper_io->prg_low_bank_fixed = false;
				cpu->cpu_mapper_io->prg_high_bank_fixed = true;
				break;
			}
			// F bit
			switch ((buffer >> 3) & 0x01) {
			// Size in KiB
			case 0:
				cpu->cpu_mapper_io->prg_rom_bank_size = 32;
				break;
			case 1:
				cpu->cpu_mapper_io->prg_rom_bank_size = 16;
				break;
			}
			// C bit
			switch ((buffer >> 4) & 0x01) {
			case 0: // can either be rom or ram
			// Size in KiB
				cpu->cpu_mapper_io->chr_bank_size = 8;
				break;
			case 1:
				cpu->cpu_mapper_io->chr_bank_size = 4;
				break;
			}
		} else if ((addr >= 0xA000) && (addr <= 0xBFFF)) {
			// reg 1: RxxC CCCC
			// C bits
			unsigned bank_select = buffer & 0x1F;
			unsigned chr_rom_banks = cpu->cpu_mapper_io->chr->rom_size / (4 * KiB);
			normalise_any_out_of_bounds_bank(&bank_select, chr_rom_banks);

			if (cpu->cpu_mapper_io->chr_bank_size == 8) {
				// ignore lowest bit (can only be aligned to even 4K banks: 0, 2, 4 etc.)
				// so can just shift out the lsb and offset each bank by 8K
				bank_select >>= 1;
			}
			// Only copy CHR ROM
			if (cpu->cpu_mapper_io->chr->rom_size) {
				set_chr_bank_1(cpu, bank_select, cpu->cpu_mapper_io->chr_bank_size * KiB);
			}
		} else if ((addr >= 0xC000) && (addr <= 0xDFFF)) {
			// reg 2: RxxC CCCC (ignored if CHR banks are in 8K mode)
			unsigned bank_select = buffer & 0x1F;
			unsigned chr_rom_banks = cpu->cpu_mapper_io->chr->rom_size / (4 * KiB);
			normalise_any_out_of_bounds_bank(&bank_select, chr_rom_banks);

			if (cpu->cpu_mapper_io->chr_bank_size == 4) {
				// Only copy CHR ROM
				if (cpu->cpu_mapper_io->chr->rom_size) {
					set_chr_bank_2(cpu, bank_select);
				}
			}
		} else if (addr >= 0xE000) { // else (addr >= 0xE000 && addr <= 0xFFFF)
			// reg 3: RxxB PPPP
			unsigned bank_select = buffer & 0x0F;
			unsigned prg_rom_banks = cpu->cpu_mapper_io->prg_rom->size / (16 * KiB);
			normalise_any_out_of_bounds_bank(&bank_select, prg_rom_banks);

			if (cpu->cpu_mapper_io->prg_rom_bank_size == 32) {
				// ignore lowest bit (can only be aligned to even 16K banks: 0, 2, 4 etc.)
				// so can just shift out the lsb and offset each bank by 32K
				bank_select >>= 1;
				set_prg_rom_bank_1(cpu, bank_select, 32 * KiB);
			} else {
				if (cpu->cpu_mapper_io->prg_low_bank_fixed) {
					set_prg_rom_bank_1(cpu, 0, 16 * KiB);
					set_prg_rom_bank_2(cpu, bank_select);
				} else if (cpu->cpu_mapper_io->prg_high_bank_fixed) {
					set_prg_rom_bank_1(cpu, bank_select, 16 * KiB);
					set_prg_rom_bank_2(cpu, prg_rom_banks - 1);
				}
			}
			cpu->cpu_mapper_io->enable_prg_ram = !((buffer & 0x10) >> 4);

			// Disable PRG RAM if there is actually no PRG RAM present
			// only valid for NES2.0 headers as iNES headers always have at least
			// 8K PRG RAM when no PRG RAM is specified
			if (!cpu->cpu_mapper_io->prg_ram->size) {
				cpu->cpu_mapper_io->enable_prg_ram = false;
			}
		}
		write_count = 0;
		buffer = 0;
	}
}
