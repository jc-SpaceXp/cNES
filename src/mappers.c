#include "mappers.h"
#include "cart.h"
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
	memcpy(&cpu->cpu_ppu_io->vram[0x0000]
		  , cpu->cpu_mapper_io->chr->data + ((chr_bank_offset) * (kib_size))
		  , kib_size);
}

static inline void set_chr_bank_2(Cpu6502* cpu, const unsigned chr_bank_offset)
{
	memcpy(&cpu->cpu_ppu_io->vram[0x1000]
		  , cpu->cpu_mapper_io->chr->data + ((chr_bank_offset) * (4 * KiB))
		  , 4 * KiB);
}

void mapper_write(Cpu6502* cpu, uint16_t addr, uint8_t val)
{
	switch (cpu->cpu_mapper_io->mapper_number) {
	case 1:
		mmc1_reg_write(cpu, addr, val);
		break;
	default:
		// ignore writes if the mapper doesn't use cpu registers to write to i.e. mapper 0
		break;
	}
}

// used to change the nametable mirroring on the fly
void change_nt_mirroring(Cpu6502* cpu)
{
	// If not using single_screen
	if ((*(cpu->cpu_ppu_io->mirroring) != 2)
	   || (*(cpu->cpu_ppu_io->mirroring) != 3)) {
		if (*(cpu->cpu_ppu_io->mirroring) == 0) {
			// Horizontal mirroring
			memcpy(&cpu->cpu_ppu_io->vram[0x2400], &cpu->cpu_ppu_io->vram[0x2000], KiB);
			memcpy(&cpu->cpu_ppu_io->vram[0x2800], &cpu->cpu_ppu_io->vram[0x2C00], KiB);
	   } else if (*(cpu->cpu_ppu_io->mirroring) == 1) {
			// Verical mirroring
			memcpy(&cpu->cpu_ppu_io->vram[0x2800], &cpu->cpu_ppu_io->vram[0x2000], KiB);
			memcpy(&cpu->cpu_ppu_io->vram[0x2400], &cpu->cpu_ppu_io->vram[0x2C00], KiB);
	   }
	}
}

void init_mapper(Cartridge* cart, Cpu6502* cpu, Ppu2C02* ppu)
{
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
	if (cart->prg_rom.size == 16 * KiB) {
		memcpy(&cpu->mem[0x8000], cart->prg_rom.data, 16 * KiB); // First 16KiB
		memcpy(&cpu->mem[0xC000], cart->prg_rom.data, 16 * KiB); // Last 16KiB (Mirrored)
	} else {
		memcpy(&cpu->mem[0x8000], cart->prg_rom.data, 32 * KiB);
	}
	free(cart->prg_rom.data);

	/* Load CHR_ROM data into PPU VRAM */
	if (cart->chr.rom_size) {
		memcpy(&ppu->vram[0x0000], cart->chr.data, 8 * KiB);
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


static void mmc1_reg_write(Cpu6502* cpu, const uint16_t addr, const uint8_t val)
{
	static unsigned write_count = 0;
	static unsigned buffer = 0;
	static unsigned write_cycle = 0;

	// ignore adjacent writes
	if (write_cycle == cpu->cycle - 1) {
		return;
	}

	// Process reset bit first
	if (val & 0x80) {
		write_count = 0;
		buffer = 0;

		unsigned prg_rom_banks = cpu->cpu_mapper_io->prg_rom->size / (16 * KiB);
		cpu->cpu_mapper_io->is_lower_fixed = true;
		cpu->cpu_mapper_io->prg_rom_bank_size = 16;
		set_prg_rom_bank_2(cpu, prg_rom_banks - 1);

		write_cycle = cpu->cycle;  // update write_cycle
		return; // early return
	}

	buffer |= (val & 0x01) << write_count;
	++write_count;
	write_cycle = cpu->cycle;  // update write_cycle

	if (write_count == 5) {
		if (addr >= 0x8000 && addr <= 0x9FFF) { // reg 0 (control)
			// MM bits
			switch (buffer & 0x03) {
			case 0x00: // 1-screen mirroring nametable 0
				*(cpu->cpu_ppu_io->mirroring) = 2;
				puts("Switch to lower nametable");
				printf("%d\n", cpu->cycle);
				break;
			case 0x01: // 1-screen mirroring nametable 1
				*(cpu->cpu_ppu_io->mirroring) = 3;
				puts("Switch to upper nametable");
				break;
			case 0x02: // 0b10 vertical mirroring
				*(cpu->cpu_ppu_io->mirroring) = 1;
				change_nt_mirroring(cpu);
				break;
			case 0x03: // 0b11 horizontal mirroring
				*(cpu->cpu_ppu_io->mirroring) = 0;
				change_nt_mirroring(cpu);
				break;
			}
			// H bit
			switch (buffer >> 2 & 0x01) {
			case 0: // 0 = fixed upper bank
				cpu->cpu_mapper_io->is_upper_fixed = true;
				cpu->cpu_mapper_io->is_lower_fixed = false;
				break;
			case 1: // 1 = fixed lower bank
				cpu->cpu_mapper_io->is_lower_fixed = true;
				cpu->cpu_mapper_io->is_upper_fixed = false;
				break;
			}
			// F bit
			switch (buffer >> 3 & 0x01) {
			case 0:
				cpu->cpu_mapper_io->prg_rom_bank_size = 32;
				break;
			case 1:
				cpu->cpu_mapper_io->prg_rom_bank_size = 16;
				break;
			}
			// C bit
			switch (buffer >> 4 & 0x01) {
			case 0: // can either be rom or ram
				cpu->cpu_mapper_io->chr_bank_size = 8;
				break;
			case 1:
				cpu->cpu_mapper_io->chr_bank_size = 4;
				break;
			}
		} else if (addr >= 0xA000 && addr <= 0xBFFF) { // reg 1
			// C bits
			unsigned bank_select = buffer & 0x1F;
			if (cpu->cpu_mapper_io->chr_bank_size == 8) {
				bank_select >>= 1;
			}
			set_chr_bank_1(cpu, bank_select, cpu->cpu_mapper_io->chr_bank_size * KiB);
		} else if (addr >= 0xC000 && addr <= 0xDFFF) { // reg 2
			unsigned bank_select = buffer & 0x1F;
			if (cpu->cpu_mapper_io->chr_bank_size == 4) {
				set_chr_bank_2(cpu, bank_select);
			}
		} else if (addr >= 0xE000) { // else (addr >= 0xE000 && addr <= 0xFFFF) --> reg 3
			unsigned bank_select = buffer & 0x0F;
			unsigned prg_rom_banks = cpu->cpu_mapper_io->prg_rom->size / (16 * KiB);
			if (cpu->cpu_mapper_io->prg_rom_bank_size == 32) {
				bank_select >>= 1;
				set_prg_rom_bank_1(cpu, bank_select, 32);
			} else {
				if (cpu->cpu_mapper_io->is_upper_fixed) {
					set_prg_rom_bank_1(cpu, 0, 16 * KiB);
					set_prg_rom_bank_2(cpu, bank_select);
				} else if (cpu->cpu_mapper_io->is_lower_fixed) {
					set_prg_rom_bank_1(cpu, bank_select, 16 * KiB);
					set_prg_rom_bank_2(cpu, prg_rom_banks - 1);
				}
			}
			cpu->cpu_mapper_io->enable_prg_ram = !((buffer & 0x10) >> 4);
		}
		write_count = 0;
		buffer = 0;
	}

	// allow writes to PRG RAM / WRAM (CPU: 0x6000 to 0x7FFF) if enabled through reg 3 previously
	// if disabled you can't read/write to this space (reads will return open bus behaviour)
	if (cpu->cpu_mapper_io->enable_prg_ram && (addr >= 0x6000) && (addr < 0x8000)) {
		cpu->mem[addr] = val;
	}
}
