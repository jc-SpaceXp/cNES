#include <check.h>

#include <stdlib.h>
#include <stdio.h>

#include "cpu_mapper_interface.h" // For cpu/mapper struct
#include "cart.h"
#include "cpu.h"
#include "ppu.h"
#include "cpu_ppu_interface.h"

Cartridge* mp_cart;
CpuMapperShare* cpu_mapper_tester;
Cpu6502* mp_cpu;
Ppu2C02* mp_ppu;
CpuPpuShare* mp_cpu_ppu_io;
//PpuNametableMirroringType mp_ppu_mirroring;

static void cart_setup(void)
{
	mp_cart = malloc(sizeof(Cartridge)); // test double
	if (!mp_cart) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to cpu struct");
	}

	// Free'ing NULL is defined, if pointer isn't set then free is invalid
	mp_cart->prg_rom.data = NULL;
	mp_cart->prg_ram.data = NULL;
	mp_cart->chr.data = NULL;
}

static void cart_teardown(void)
{
	free(mp_cart);
}

static void cpu_setup(void)
{
	mp_cpu = malloc(sizeof(Cpu6502)); // test double
	if (!mp_cpu) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to cpu struct");
	}
}

static void cpu_teardown(void)
{
	free(mp_cpu);
}

static void cpu_mapper_setup(void)
{
	cpu_mapper_tester = malloc(sizeof(CpuMapperShare)); // test double
	if (!cpu_mapper_tester) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to cpu/mapper struct");
	}
}

static void cpu_mapper_teardown(void)
{
	free(cpu_mapper_tester);
}

static void cpu_ppu_setup(void)
{
	mp_cpu_ppu_io = malloc(sizeof(CpuPpuShare)); // test double
	if (!mp_cpu_ppu_io) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to cpu/ppu struct");
	}
}

static void cpu_ppu_teardown(void)
{
	free(mp_cpu_ppu_io);
}

static void ppu_setup(void)
{
	mp_ppu = malloc(sizeof(Ppu2C02)); // test double
	if (!mp_ppu) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to ppu struct");
	}
}

static void ppu_teardown(void)
{
	free(mp_ppu);
}

static void setup(void)
{
	cpu_setup();
	ppu_setup();
	cart_setup();
	cpu_mapper_setup();
	cpu_ppu_setup();

	mp_cpu->cpu_mapper_io = cpu_mapper_tester;
	mp_cpu->cpu_mapper_io->chr = &mp_cart->chr;
	mp_cpu->cpu_mapper_io->chr->ram_size = 1; // allow writes to CHR-RAM

	map_ppu_data_to_cpu_ppu_io(mp_cpu_ppu_io, mp_ppu);
	mp_cpu->cpu_ppu_io = mp_cpu_ppu_io;
	mp_ppu->nametable_mirroring = SINGLE_SCREEN_A;
}

static void teardown(void)
{
	cpu_teardown();
	ppu_teardown();
	cpu_mapper_teardown();
	cart_teardown();
	cpu_ppu_teardown();
}

static void set_prg_bank(uint8_t* prg_data, uint8_t block_pattern, size_t total_bytes)
{
	memset(prg_data, block_pattern, total_bytes);
}


START_TEST (mapper_000_prg_rom_banks)
{
	mp_cart->chr.rom_size = 0;
	cpu_mapper_tester->mapper_number = 0;
	unsigned int prg_rom_sizes[2] = {16 * KiB, 32 * KiB};
	uint8_t* prg_window = calloc(32 * KiB, sizeof(uint8_t));
	uint8_t prg_banks[2][2] = {{0xA0, 0xA0}, {0xA0, 0x08}};
	mp_cart->prg_rom.size = prg_rom_sizes[_i];
	mp_cart->prg_rom.data = prg_window;
	set_prg_bank(mp_cart->prg_rom.data,            prg_banks[_i][0], 16 * KiB);
	set_prg_bank(mp_cart->prg_rom.data + 16 * KiB, prg_banks[_i][1], 16 * KiB);
	// Mirror prg_banks into arrays so we can check the result
	// as the prg_rom.data is free'd by the mapper_000() function
	uint8_t prg_array_1[16 * KiB] = {0};
	uint8_t prg_array_2[16 * KiB] = {0};
	memset(&prg_array_1[0], prg_banks[_i][0], 16 * KiB);
	memset(&prg_array_2[0], prg_banks[_i][1], 16 * KiB);

	init_mapper(mp_cart, mp_cpu, mp_ppu);

	ck_assert_mem_eq(&mp_cpu->mem[0x8000], &prg_array_1[0], 16 * KiB);
	ck_assert_mem_eq(&mp_cpu->mem[0xC000], &prg_array_2[0], 16 * KiB);
}

START_TEST (mapper_000_chr_rom_banks)
{
	mp_cart->chr.rom_size = 8 * KiB; // chr data is always 8K for mapper 0
	cpu_mapper_tester->mapper_number = 0;
	uint8_t* chr_window = calloc(8 * KiB, sizeof(uint8_t));
	// Split chr 8K into 2 4K regions
	uint8_t chr_banks[2][2] = {{0xA0, 0xA0}, {0xA0, 0x08}};
	mp_cart->chr.data = chr_window;
	set_prg_bank(mp_cart->chr.data,           chr_banks[_i][0], 4 * KiB);
	set_prg_bank(mp_cart->chr.data + 4 * KiB, chr_banks[_i][1], 4 * KiB);
	// Mirror prg_banks into arrays so we can check the result
	// as the prg_rom.data is free'd by the mapper_000() function
	uint8_t chr_array_1[4 * KiB] = {0};
	uint8_t chr_array_2[4 * KiB] = {0};
	memset(&chr_array_1[0], chr_banks[_i][0], 4 * KiB);
	memset(&chr_array_2[0], chr_banks[_i][1], 4 * KiB);;
	// Need to set prg data too otherwise we get segfaults
	mp_cart->prg_rom.size = 32 * KiB;
	uint8_t* prg_window = calloc(32 * KiB, sizeof(uint8_t));
	mp_cart->prg_rom.data = prg_window;

	init_mapper(mp_cart, mp_cpu, mp_ppu);

	// pattern table_0 is vram address 0x0000-0x0FFF (4K)
	// pattern table_1 is vram address 0x1000-0x1FFF (4K)
	ck_assert_mem_eq(&mp_ppu->vram.pattern_table_0[0x0000], &chr_array_1[0], 4 * KiB);
	ck_assert_mem_eq(&mp_ppu->vram.pattern_table_1[0x0000], &chr_array_2[0], 4 * KiB);
}


Suite* mapper_000_suite(void)
{
	Suite* s;
	TCase* tc_prg_and_chr_banks;

	s = suite_create("Mapper 000 Tests");
	tc_prg_and_chr_banks = tcase_create("PRG and CHR Banks Tests");
	tcase_add_checked_fixture(tc_prg_and_chr_banks, setup, teardown);
	tcase_add_loop_test(tc_prg_and_chr_banks, mapper_000_prg_rom_banks, 0, 2);
	tcase_add_loop_test(tc_prg_and_chr_banks, mapper_000_chr_rom_banks, 0, 2);
	suite_add_tcase(s, tc_prg_and_chr_banks);

	return s;
}
