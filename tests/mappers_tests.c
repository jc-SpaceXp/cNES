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

// 0 indexed bit pos
static uint8_t get_nth_bit(unsigned int input, unsigned int bit_pos)
{
	return (input & (1 << bit_pos)) >> bit_pos;
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
	memset(mp_cart->prg_rom.data,            prg_banks[_i][0], 16 * KiB);
	memset(mp_cart->prg_rom.data + 16 * KiB, prg_banks[_i][1], 16 * KiB);
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
	memset(mp_cart->chr.data,           chr_banks[_i][0], 4 * KiB);
	memset(mp_cart->chr.data + 4 * KiB, chr_banks[_i][1], 4 * KiB);
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

START_TEST (mapper_000_unmapped_open_bus_reads)
{
	cpu_mapper_tester->mapper_number = 0;
	uint16_t addr = 0x50EF;

	mp_cpu->mem[addr] = 0x3D;
	mp_cpu->data_bus = 0x98;

	ck_assert_uint_eq(mapper_read(mp_cpu, addr), mp_cpu->data_bus);
}

START_TEST (mapper_000_prg_rom_reads)
{
	cpu_mapper_tester->mapper_number = 0;
	uint16_t prg_rom_addr = 0xABC0; // PRG ROM window is $8000 to $FFFF

	mp_cpu->mem[prg_rom_addr] = 0x3E;

	ck_assert_uint_eq(mapper_read(mp_cpu, prg_rom_addr), mp_cpu->mem[prg_rom_addr]);
}

START_TEST (mapper_001_last_write_selects_reg)
{
	cpu_mapper_tester->mapper_number = 1;
	cpu_mapper_tester->prg_rom_bank_size = 32; // changing this through MMC1 reg
	cpu_mapper_tester->chr_bank_size = 4; // changing this through MMC1 reg
	mp_cpu->cycle = 13;
	uint16_t ctrl_reg = 0x9000; // $8000 to $9FFF
	uint16_t chr0_reg = 0xB015; // $A000 to $BFFF
	uint16_t chr1_reg = 0x9000; // $C000 to $DFFF

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, chr0_reg, 0x00); // 1 (buffer: xxxx0)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, 0x00); // 2 (buffer: xxx00)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, 0x00); // 3 (buffer: xx000)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, 0x01); // 4 (buffer: x1000)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 5 (buffer: 01000)
	mp_cpu->cycle += 5;

	ck_assert_uint_eq(cpu_mapper_tester->prg_rom_bank_size, 16);
	ck_assert_uint_eq(cpu_mapper_tester->chr_bank_size, 8);
}

START_TEST (mapper_001_five_writes_selects_reg)
{
	cpu_mapper_tester->mapper_number = 1;
	cpu_mapper_tester->prg_rom_bank_size = 16; // changing this through MMC1 reg
	cpu_mapper_tester->chr_bank_size = 4; // changing this through MMC1 reg
	mp_cpu->cycle = 13;
	uint16_t ctrl_reg = 0x9000; // $8000 to $9FFF
	// value should only change on 5th write
	uint8_t prg_rom_bank_size[5] = {16, 16, 16, 16, 32};
	uint8_t chr_bank_size[5] = {4, 4, 4, 4, 8};

	// 1st write is LSB and last is MSB
	for (int i = 0; i <= _i; ++i) {
		mapper_write(mp_cpu, ctrl_reg + i, 0x00); // 5 writes (buffer: 00000)
		mp_cpu->cycle += 5;
	}

	ck_assert_uint_eq(cpu_mapper_tester->prg_rom_bank_size, prg_rom_bank_size[_i]);
	ck_assert_uint_eq(cpu_mapper_tester->chr_bank_size, chr_bank_size[_i]);
}

START_TEST (mapper_001_reg0_mm_bits)
{
	cpu_mapper_tester->mapper_number = 1;
	mp_ppu->nametable_mirroring = HORIZONTAL;
	mp_cpu->cycle = 13;
	uint16_t ctrl_reg = 0x9000; // $8000 to $9FFF
	PpuNametableMirroringType mirror_mode[4] = { SINGLE_SCREEN_A
	                                           , SINGLE_SCREEN_B
	                                           , VERTICAL
	                                           , HORIZONTAL};

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, ctrl_reg, get_nth_bit(_i, 0)); // 1 (buffer: xxxx?)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, get_nth_bit(_i, 1)); // 2 (buffer: xxx??)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 3 (buffer: xx0??)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x01); // 4 (buffer: x10??)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 5 (buffer: 010??)
	mp_cpu->cycle += 5;

	ck_assert_uint_eq(mp_ppu->nametable_mirroring, mirror_mode[_i]);
}

START_TEST (mapper_001_reg0_h_bit)
{
	cpu_mapper_tester->mapper_number = 1;
	cpu_mapper_tester->prg_low_bank_fixed = false;
	cpu_mapper_tester->prg_high_bank_fixed = false;
	mp_cpu->cycle = 13;
	uint16_t ctrl_reg = 0x9000; // $8000 to $9FFF
	bool expected_fixed_low_bank[2] = {true, false};
	bool expected_fixed_high_bank[2] = {false, true};

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 1 (buffer: xxxx0)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 2 (buffer: xxx00)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, _i); // 3 (buffer: xxi00) (H bit)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x01); // 4 (buffer: x1i00)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 5 (buffer: 01i00)
	mp_cpu->cycle += 5;

	ck_assert_uint_eq(cpu_mapper_tester->prg_low_bank_fixed, expected_fixed_low_bank[_i]);
	ck_assert_uint_eq(cpu_mapper_tester->prg_high_bank_fixed, expected_fixed_high_bank[_i]);
}

START_TEST (mapper_001_reg0_f_bit)
{
	cpu_mapper_tester->mapper_number = 1;
	mp_cpu->cycle = 13;
	uint16_t ctrl_reg = 0x9000; // $8000 to $9FFF
	uint8_t prg_rom_bank_size[2] = {32, 16}; // KiB size

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 1 (buffer: xxxx0)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 2 (buffer: xxx00)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 3 (buffer: xx000)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, _i); // 4 (buffer: xi000) (F bit)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 5 (buffer: 0i000)
	mp_cpu->cycle += 5;

	ck_assert_uint_eq(cpu_mapper_tester->prg_rom_bank_size, prg_rom_bank_size[_i]);
}

START_TEST (mapper_001_reg0_c_bit)
{
	cpu_mapper_tester->mapper_number = 1;
	mp_cpu->cycle = 13;
	uint16_t ctrl_reg = 0x9000; // $8000 to $9FFF
	uint8_t chr_bank_size[2] = {8, 4}; // KiB size

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 1 (buffer: xxxx0)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 2 (buffer: xxx00)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 3 (buffer: xx000)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, 0x00); // 4 (buffer: x0000)
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, ctrl_reg, _i); // 5 (buffer: i0000)
	mp_cpu->cycle += 5;

	ck_assert_uint_eq(cpu_mapper_tester->chr_bank_size, chr_bank_size[_i]);
}

START_TEST (mapper_001_reg1_chr0_bank_select_4k_rom)
{
	cpu_mapper_tester->mapper_number = 1;
	mp_cpu->cycle = 13;
	uint16_t chr0_reg = 0xA49B; // $A000 to $BFFF
	cpu_mapper_tester->chr_bank_size = 4; // KiB size, 32 possible banks in 128K chr
	cpu_mapper_tester->chr->rom_size = 4 * KiB;
	cpu_mapper_tester->chr->ram_size = 0;
	unsigned int bank_select = _i; // 0-31 banks
	uint8_t* chr_window = calloc(128 * KiB, sizeof(uint8_t));
	mp_cart->chr.data = chr_window;
	for (int bank = 0; bank < 32; ++bank) {
		memset(mp_cart->chr.data + bank * 4 * KiB, bank, 4 * KiB);
	}

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 0)); // 1
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 1)); // 2
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 2)); // 3
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 3)); // 4
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 4)); // 5
	mp_cpu->cycle += 5;

	ck_assert_mem_eq(&mp_ppu->vram.pattern_table_0[0x0000]
	                , chr_window + bank_select * 4 * KiB
	                , 4 * KiB);
	free(chr_window);
}

START_TEST (mapper_001_reg1_chr0_bank_select_4k_ram_ignored)
{
	cpu_mapper_tester->mapper_number = 1;
	mp_cpu->cycle = 13;
	uint16_t chr0_reg = 0xA49B; // $A000 to $BFFF
	cpu_mapper_tester->chr_bank_size = 4; // KiB size, 32 possible banks in 128K chr
	cpu_mapper_tester->chr->rom_size = 0;
	cpu_mapper_tester->chr->ram_size = 4 * KiB;
	// There are no chr banks for chr ram in MMC1 but testing to see
	// that no bankswitching is attempted
	unsigned int bank_select = _i; // 0-31 banks
	uint8_t* chr_window = calloc(128 * KiB, sizeof(uint8_t));
	mp_cart->chr.data = chr_window;
	for (int bank = 0; bank < 32; ++bank) {
		memset(mp_cart->chr.data + bank * 4 * KiB, bank, 4 * KiB);
	}

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 0)); // 1
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 1)); // 2
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 2)); // 3
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 3)); // 4
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 4)); // 5
	mp_cpu->cycle += 5;

	ck_assert_mem_ne(&mp_ppu->vram.pattern_table_0[0x0000]
	                , chr_window + bank_select * 4 * KiB
	                , 4 * KiB);
	free(chr_window);
}

START_TEST (mapper_001_reg1_chr0_bank_select_8k_rom)
{
	cpu_mapper_tester->mapper_number = 1;
	mp_cpu->cycle = 13;
	uint16_t chr0_reg = 0xB49B; // $A000 to $BFFF
	cpu_mapper_tester->chr_bank_size = 8; // KiB size, 16 possible 8K banks in 128K chr
	cpu_mapper_tester->chr->rom_size = 8 * KiB;
	cpu_mapper_tester->chr->ram_size = 0;
	unsigned int bank_select = _i; // 0-31 banks
	uint8_t* chr_window = calloc(128 * KiB, sizeof(uint8_t));
	mp_cart->chr.data = chr_window;
	for (int bank = 0; bank < 16; ++bank) {
		memset(mp_cart->chr.data + bank * 8 * KiB, bank, 8 * KiB);
	}

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 0)); // 1
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 1)); // 2
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 2)); // 3
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 3)); // 4
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 4)); // 5
	mp_cpu->cycle += 5;

	// split up into 2 4K banks due to ppu memory layout
	// since lowest bit is ignored we get this pattern for even banks
	// (pattern_table_0): 0 0 2 2 4 4 6 6 8 8 etc. for increasing
	// even 4K banks (0, 2, 4 etc.) (via (_i >> 1) * 2)
	ck_assert_mem_eq(&mp_ppu->vram.pattern_table_0[0x0000]
	                , chr_window + ((bank_select >> 1) * 2) * 4 * KiB
	                , 4 * KiB);
	// odd 4K banks (1, 3, 5 etc.) (via even banks calc + 1)
	ck_assert_mem_eq(&mp_ppu->vram.pattern_table_1[0x0000]
	                , chr_window + (((bank_select >> 1) * 2) + 1) * 4 * KiB
	                , 4 * KiB);
	free(chr_window);
}

START_TEST (mapper_001_reg1_chr0_bank_select_8k_ram_ignored)
{
	cpu_mapper_tester->mapper_number = 1;
	mp_cpu->cycle = 13;
	uint16_t chr0_reg = 0xB49B; // $A000 to $BFFF
	cpu_mapper_tester->chr_bank_size = 8; // KiB size, 16 possible 8K banks in 128K chr
	cpu_mapper_tester->chr->rom_size = 0;
	cpu_mapper_tester->chr->ram_size = 8 * KiB;
	// There are no chr banks for chr ram in MMC1 but testing to see
	// that no bankswitching is attempted
	unsigned int bank_select = _i; // 0-31 banks
	uint8_t* chr_window = calloc(128 * KiB, sizeof(uint8_t));
	mp_cart->chr.data = chr_window;
	for (int bank = 0; bank < 16; ++bank) {
		memset(mp_cart->chr.data + bank * 8 * KiB, bank, 8 * KiB);
	}

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 0)); // 1
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 1)); // 2
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 2)); // 3
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 3)); // 4
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr0_reg, get_nth_bit(_i, 4)); // 5
	mp_cpu->cycle += 5;

	// split up into 2 4K banks due to ppu memory layout
	// since lowest bit is ignored we get this pattern for even banks
	// (pattern_table_0): 0 0 2 2 4 4 6 6 8 8 etc. for increasing
	// even 4K banks (0, 2, 4 etc.) (via (_i >> 1) * 2)
	ck_assert_mem_ne(&mp_ppu->vram.pattern_table_0[0x0000]
	                , chr_window + ((bank_select >> 1) * 2) * 4 * KiB
	                , 4 * KiB);
	// odd 4K banks (1, 3, 5 etc.) (via even banks calc + 1)
	ck_assert_mem_ne(&mp_ppu->vram.pattern_table_1[0x0000]
	                , chr_window + (((bank_select >> 1) * 2) + 1) * 4 * KiB
	                , 4 * KiB);
	free(chr_window);
}

START_TEST (mapper_001_reg2_chr1_bank_select_4k_rom)
{
	cpu_mapper_tester->mapper_number = 1;
	mp_cpu->cycle = 13;
	uint16_t chr1_reg = 0xCF1C; // $C000 to $DFFF
	cpu_mapper_tester->chr_bank_size = 4; // KiB size, 32 possible banks in 128K chr
	cpu_mapper_tester->chr->rom_size = 4 * KiB;
	cpu_mapper_tester->chr->ram_size = 0;
	unsigned int bank_select = _i; // 0-31 banks
	uint8_t* chr_window = calloc(128 * KiB, sizeof(uint8_t));
	mp_cart->chr.data = chr_window;
	for (int bank = 0; bank < 32; ++bank) {
		memset(mp_cart->chr.data + bank * 4 * KiB, bank, 4 * KiB);
	}

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 0)); // 1
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 1)); // 2
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 2)); // 3
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 3)); // 4
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 4)); // 5
	mp_cpu->cycle += 5;

	ck_assert_mem_eq(&mp_ppu->vram.pattern_table_1[0x0000]
	                , chr_window + bank_select * 4 * KiB
	                , 4 * KiB);
	free(chr_window);
}

START_TEST (mapper_001_reg2_chr1_bank_select_4k_ram_ignored)
{
	cpu_mapper_tester->mapper_number = 1;
	mp_cpu->cycle = 13;
	uint16_t chr1_reg = 0xCF1C; // $C000 to $DFFF
	cpu_mapper_tester->chr_bank_size = 4; // KiB size, 32 possible banks in 128K chr
	cpu_mapper_tester->chr->rom_size = 0;
	cpu_mapper_tester->chr->ram_size = 4 * KiB;
	// There are no chr banks for chr ram in MMC1 but testing to see
	// that no bankswitching is attempted
	unsigned int bank_select = _i; // 0-31 banks
	uint8_t* chr_window = calloc(128 * KiB, sizeof(uint8_t));
	mp_cart->chr.data = chr_window;
	for (int bank = 0; bank < 32; ++bank) {
		memset(mp_cart->chr.data + bank * 4 * KiB, bank, 4 * KiB);
	}

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 0)); // 1
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 1)); // 2
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 2)); // 3
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 3)); // 4
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 4)); // 5
	mp_cpu->cycle += 5;

	ck_assert_mem_ne(&mp_ppu->vram.pattern_table_1[0x0000]
	                , chr_window + bank_select * 4 * KiB
	                , 4 * KiB);
	free(chr_window);
}

START_TEST (mapper_001_reg2_chr1_bank_select_8k_rom_ignored)
{
	cpu_mapper_tester->mapper_number = 1;
	mp_cpu->cycle = 13;
	uint16_t chr1_reg = 0xCF1C; // $C000 to $DFFF
	cpu_mapper_tester->chr_bank_size = 8; // KiB size, 16 possible 8K banks in 128K chr
	cpu_mapper_tester->chr->rom_size = 8 * KiB;
	cpu_mapper_tester->chr->ram_size = 0;
	unsigned int bank_select = _i; // 0-31 banks
	uint8_t* chr_window = calloc(128 * KiB, sizeof(uint8_t));
	mp_cart->chr.data = chr_window;
	for (int bank = 0; bank < 16; ++bank) {
		memset(mp_cart->chr.data + bank * 8 * KiB, bank, 8 * KiB);
	}
	// Set chr data to all 1's
	memset(&mp_ppu->vram.pattern_table_0[0x0000], 0xFF, 4 * KiB);
	memset(&mp_ppu->vram.pattern_table_1[0x0000], 0xFF, 4 * KiB);

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 0)); // 1
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 1)); // 2
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 2)); // 3
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 3)); // 4
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 4)); // 5
	mp_cpu->cycle += 5;

	// split up into 2 4K banks due to ppu memory layout
	// since lowest bit is ignored we get this pattern for even banks
	// (pattern_table_0): 0 0 2 2 4 4 6 6 8 8 etc. for increasing
	// even 4K banks (0, 2, 4 etc.) (via (_i >> 1) * 2)
	ck_assert_mem_ne(&mp_ppu->vram.pattern_table_1[0x0000]
	                , chr_window + ((bank_select >> 1) * 2) * 4 * KiB
	                , 4 * KiB);
	// odd 4K banks (1, 3, 5 etc.) (via even banks calc + 1)
	ck_assert_mem_ne(&mp_ppu->vram.pattern_table_1[0x0000]
	                , chr_window + (((bank_select >> 1) * 2) + 1) * 4 * KiB
	                , 4 * KiB);
	free(chr_window);
}

START_TEST (mapper_001_reg2_chr1_bank_select_8k_ram_ignored)
{
	cpu_mapper_tester->mapper_number = 1;
	mp_cpu->cycle = 13;
	uint16_t chr1_reg = 0xCF1C; // $C000 to $DFFF
	cpu_mapper_tester->chr_bank_size = 8; // KiB size, 16 possible 8K banks in 128K chr
	cpu_mapper_tester->chr->rom_size = 0;
	cpu_mapper_tester->chr->ram_size = 8 * KiB;
	// There are no chr banks for chr ram in MMC1 but testing to see
	// that no bankswitching is attempted
	unsigned int bank_select = _i; // 0-31 banks
	uint8_t* chr_window = calloc(128 * KiB, sizeof(uint8_t));
	mp_cart->chr.data = chr_window;
	for (int bank = 0; bank < 16; ++bank) {
		memset(mp_cart->chr.data + bank * 8 * KiB, bank, 8 * KiB);
	}
	// Set chr data to all 1's
	memset(&mp_ppu->vram.pattern_table_0[0x0000], 0xFF, 4 * KiB);
	memset(&mp_ppu->vram.pattern_table_1[0x0000], 0xFF, 4 * KiB);

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 0)); // 1
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 1)); // 2
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 2)); // 3
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 3)); // 4
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, chr1_reg, get_nth_bit(_i, 4)); // 5
	mp_cpu->cycle += 5;

	// split up into 2 4K banks due to ppu memory layout
	// since lowest bit is ignored we get this pattern for even banks
	// (pattern_table_0): 0 0 2 2 4 4 6 6 8 8 etc. for increasing
	// even 4K banks (0, 2, 4 etc.) (via (_i >> 1) * 2)
	ck_assert_mem_ne(&mp_ppu->vram.pattern_table_1[0x0000]
	                , chr_window + ((bank_select >> 1) * 2) * 4 * KiB
	                , 4 * KiB);
	// odd 4K banks (1, 3, 5 etc.) (via even banks calc + 1)
	ck_assert_mem_ne(&mp_ppu->vram.pattern_table_1[0x0000]
	                , chr_window + (((bank_select >> 1) * 2) + 1) * 4 * KiB
	                , 4 * KiB);
	free(chr_window);
}

START_TEST (mapper_001_reg3_prg_bank_select_32k)
{
	cpu_mapper_tester->mapper_number = 1;
	cpu_mapper_tester->prg_rom_bank_size = 32;
	mp_cpu->cycle = 13;
	uint16_t prg_reg = 0xFF5E; // $E000 to $FFFF
	unsigned int bank_select = _i; // 0-31 banks
	uint8_t* prg_window = calloc(256 * KiB, sizeof(uint8_t));
	mp_cart->prg_rom.data = prg_window;
	mp_cart->prg_rom.size = 256 * KiB;
	cpu_mapper_tester->prg_rom = &mp_cart->prg_rom;
	for (int bank = 0; bank < 8; ++bank) {
		memset(mp_cart->prg_rom.data + bank * 32 * KiB, bank, 32 * KiB);
	}

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 0)); // 1
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 1)); // 2
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 2)); // 3
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 3)); // 4
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, 0x00); // 5 (only 4 bits are needed for prg banks)
	mp_cpu->cycle += 5;

	ck_assert_mem_eq(&mp_cpu->mem[0x8000]
	                , prg_window + (bank_select >> 1) * 32 * KiB
	                , 32 * KiB);
	free(prg_window);
}

START_TEST (mapper_001_reg3_prg_lo_bank_select_16k)
{
	cpu_mapper_tester->mapper_number = 1;
	cpu_mapper_tester->prg_rom_bank_size = 16;
	// Lo/Hi bank mirror each other, one must be true and one must be false
	cpu_mapper_tester->prg_low_bank_fixed = false;
	cpu_mapper_tester->prg_high_bank_fixed = true;
	mp_cpu->cycle = 13;
	uint16_t prg_reg = 0xFF5E; // $E000 to $FFFF
	unsigned int bank_select = _i; // 0-31 banks
	uint8_t* prg_window = calloc(256 * KiB, sizeof(uint8_t));
	mp_cart->prg_rom.data = prg_window;
	mp_cart->prg_rom.size = 256 * KiB;
	unsigned int total_banks = mp_cart->prg_rom.size / (16 * KiB);
	cpu_mapper_tester->prg_rom = &mp_cart->prg_rom;
	for (int bank = 0; bank < 16; ++bank) {
		memset(mp_cart->prg_rom.data + bank * 16 * KiB, bank, 16 * KiB);
	}

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 0)); // 1
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 1)); // 2
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 2)); // 3
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 3)); // 4
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, 0x00); // 5 (only 4 bits are needed for prg banks)
	mp_cpu->cycle += 5;

	// First prg rom bank is swappable
	ck_assert_mem_eq(&mp_cpu->mem[0x8000]
	                , prg_window + bank_select * 16 * KiB
	                , 16 * KiB);
	// Last prg rom bank is fixed to the last 16K prg bank
	ck_assert_mem_eq(&mp_cpu->mem[0xC000]
	                , prg_window + (total_banks - 1) * 16 * KiB
	                , 16 * KiB);
	free(prg_window);
}

START_TEST (mapper_001_reg3_prg_hi_bank_select_16k)
{
	cpu_mapper_tester->mapper_number = 1;
	cpu_mapper_tester->prg_rom_bank_size = 16;
	// Lo/Hi bank mirror each other, one must be true and one must be false
	cpu_mapper_tester->prg_low_bank_fixed = true;
	cpu_mapper_tester->prg_high_bank_fixed = false;
	mp_cpu->cycle = 13;
	uint16_t prg_reg = 0xFF5E; // $E000 to $FFFF
	unsigned int bank_select = _i; // 0-31 banks
	uint8_t* prg_window = calloc(256 * KiB, sizeof(uint8_t));
	mp_cart->prg_rom.data = prg_window;
	mp_cart->prg_rom.size = 256 * KiB;
	cpu_mapper_tester->prg_rom = &mp_cart->prg_rom;
	for (int bank = 0; bank < 16; ++bank) {
		memset(mp_cart->prg_rom.data + bank * 16 * KiB, bank, 16 * KiB);
	}

	// 1st write is LSB and last is MSB
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 0)); // 1
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 1)); // 2
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 2)); // 3
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, get_nth_bit(_i, 3)); // 4
	mp_cpu->cycle += 5;
	mapper_write(mp_cpu, prg_reg, 0x00); // 5 (only 4 bits are needed for prg banks)
	mp_cpu->cycle += 5;

	// First prg rom bank is fixed to the first 16K bank
	ck_assert_mem_eq(&mp_cpu->mem[0x8000]
	                , prg_window
	                , 16 * KiB);
	// Last prg rom bank is swappable
	ck_assert_mem_eq(&mp_cpu->mem[0xC000]
	                , prg_window + bank_select * 16 * KiB
	                , 16 * KiB);
	free(prg_window);
}

START_TEST (mapper_001_prg_ram_enabled_writes)
{
	cpu_mapper_tester->mapper_number = 1;
	cpu_mapper_tester->enable_prg_ram = true;
	uint16_t prg_ram_addr = 0x6000; // PRG RAM window is $6000 to $7FFF

	mapper_write(mp_cpu, prg_ram_addr, 0xC3); // trigger PRG RAM write

	ck_assert_uint_eq(mp_cpu->mem[prg_ram_addr], 0xC3);
}

START_TEST (mapper_001_prg_ram_disabled_writes)
{
	cpu_mapper_tester->mapper_number = 1;
	cpu_mapper_tester->enable_prg_ram = false;
	uint16_t prg_ram_addr = 0x6000; // PRG RAM window is $6000 to $7FFF

	mapper_write(mp_cpu, prg_ram_addr, 0x0F);

	ck_assert_uint_ne(mp_cpu->mem[prg_ram_addr], 0x0F);
}

START_TEST (mapper_001_prg_ram_enabled_reads)
{
	cpu_mapper_tester->mapper_number = 1;
	cpu_mapper_tester->enable_prg_ram = true;
	uint16_t prg_ram_addr = 0x6000; // PRG RAM window is $6000 to $7FFF

	mp_cpu->mem[prg_ram_addr] = 0xC3;
	mp_cpu->data_bus = 0x5E;

	ck_assert_uint_eq(mapper_read(mp_cpu, prg_ram_addr), 0xC3);
}

START_TEST (mapper_001_prg_ram_disabled_reads)
{
	cpu_mapper_tester->mapper_number = 1;
	cpu_mapper_tester->enable_prg_ram = false;
	uint16_t prg_ram_addr = 0x7B00; // PRG RAM window is $6000 to $7FFF

	mp_cpu->mem[prg_ram_addr] = 0x0D;
	mp_cpu->data_bus = 0x5E;

	ck_assert_uint_eq(mapper_read(mp_cpu, prg_ram_addr), mp_cpu->data_bus);
}

START_TEST (mapper_001_unmapped_open_bus_reads)
{
	cpu_mapper_tester->mapper_number = 1;
	uint16_t addr = 0x4FC2;

	mp_cpu->mem[addr] = 0x0D;
	mp_cpu->data_bus = 0x5E;

	ck_assert_uint_eq(mapper_read(mp_cpu, addr), mp_cpu->data_bus);
}

START_TEST (mapper_001_prg_rom_reads)
{
	cpu_mapper_tester->mapper_number = 1;
	uint16_t prg_rom_addr = 0x9FC2; // PRG ROM window is $8000 to $FFFF

	mp_cpu->mem[prg_rom_addr] = 0x0D;

	ck_assert_uint_eq(mapper_read(mp_cpu, prg_rom_addr), mp_cpu->mem[prg_rom_addr]);
}


Suite* mapper_master_suite(void)
{
	Suite* s;

	s = suite_create("All Mapper Tests");

	return s;
}

Suite* mapper_000_suite(void)
{
	Suite* s;
	TCase* tc_prg_and_chr_banks;
	TCase* tc_other_misc_tests;

	s = suite_create("Mapper 000 Tests");
	tc_prg_and_chr_banks = tcase_create("PRG and CHR Banks Tests");
	tcase_add_checked_fixture(tc_prg_and_chr_banks, setup, teardown);
	tcase_add_loop_test(tc_prg_and_chr_banks, mapper_000_prg_rom_banks, 0, 2);
	tcase_add_loop_test(tc_prg_and_chr_banks, mapper_000_chr_rom_banks, 0, 2);
	suite_add_tcase(s, tc_prg_and_chr_banks);
	tc_other_misc_tests = tcase_create("Mapper 000 Misc. Tests");
	tcase_add_checked_fixture(tc_other_misc_tests, setup, teardown);
	tcase_add_test(tc_other_misc_tests, mapper_000_unmapped_open_bus_reads);
	tcase_add_test(tc_other_misc_tests, mapper_000_prg_rom_reads);
	suite_add_tcase(s, tc_other_misc_tests);

	return s;
}

Suite* mapper_001_suite(void)
{
	Suite* s;
	TCase* tc_mmc1_registers;
	TCase* tc_mmc1_reg0_bits; // ctrl register
	TCase* tc_mmc1_reg1_bank_select; // chr0 register
	TCase* tc_mmc1_reg2_bank_select; // chr1 register
	TCase* tc_mmc1_reg3_bank_select; // prg register
	TCase* tc_mmc1_prg_ram;
	TCase* tc_mmc1_other;

	s = suite_create("Mapper 001 Tests");
	tc_mmc1_registers = tcase_create("MMC1 Registers Tests");
	tcase_add_checked_fixture(tc_mmc1_registers, setup, teardown);
	tcase_add_test(tc_mmc1_registers, mapper_001_last_write_selects_reg);
	tcase_add_loop_test(tc_mmc1_registers, mapper_001_five_writes_selects_reg, 0, 5);
	suite_add_tcase(s, tc_mmc1_registers);
	tc_mmc1_reg0_bits = tcase_create("MMC1 Reg0/Ctrl Register Tests");
	tcase_add_checked_fixture(tc_mmc1_reg0_bits, setup, teardown);
	tcase_add_loop_test(tc_mmc1_reg0_bits, mapper_001_reg0_mm_bits, 0, 4);
	tcase_add_loop_test(tc_mmc1_reg0_bits, mapper_001_reg0_h_bit, 0, 2);
	tcase_add_loop_test(tc_mmc1_reg0_bits, mapper_001_reg0_f_bit, 0, 2);
	tcase_add_loop_test(tc_mmc1_reg0_bits, mapper_001_reg0_c_bit, 0, 2);
	suite_add_tcase(s, tc_mmc1_reg0_bits);
	tc_mmc1_reg1_bank_select = tcase_create("MMC1 Reg1/Chr0 Register Tests");
	tcase_add_checked_fixture(tc_mmc1_reg1_bank_select, setup, teardown);
	tcase_add_loop_test(tc_mmc1_reg1_bank_select, mapper_001_reg1_chr0_bank_select_4k_rom, 0, 32);
	tcase_add_loop_test(tc_mmc1_reg1_bank_select, mapper_001_reg1_chr0_bank_select_4k_ram_ignored, 0, 32);
	tcase_add_loop_test(tc_mmc1_reg1_bank_select, mapper_001_reg1_chr0_bank_select_8k_rom, 0, 32);
	tcase_add_loop_test(tc_mmc1_reg1_bank_select, mapper_001_reg1_chr0_bank_select_8k_ram_ignored, 0, 32);
	suite_add_tcase(s, tc_mmc1_reg1_bank_select);
	tc_mmc1_reg2_bank_select = tcase_create("MMC1 Reg2/Chr1 Register Tests");
	tcase_add_checked_fixture(tc_mmc1_reg2_bank_select, setup, teardown);
	tcase_add_loop_test(tc_mmc1_reg2_bank_select, mapper_001_reg2_chr1_bank_select_4k_rom, 0, 32);
	tcase_add_loop_test(tc_mmc1_reg2_bank_select, mapper_001_reg2_chr1_bank_select_4k_ram_ignored, 0, 32);
	tcase_add_loop_test(tc_mmc1_reg2_bank_select, mapper_001_reg2_chr1_bank_select_8k_rom_ignored, 0, 32);
	tcase_add_loop_test(tc_mmc1_reg2_bank_select, mapper_001_reg2_chr1_bank_select_8k_ram_ignored, 0, 32);
	suite_add_tcase(s, tc_mmc1_reg2_bank_select);
	tc_mmc1_reg3_bank_select = tcase_create("MMC1 Reg3/Prg Register Tests");
	tcase_add_checked_fixture(tc_mmc1_reg3_bank_select, setup, teardown);
	tcase_add_loop_test(tc_mmc1_reg3_bank_select, mapper_001_reg3_prg_bank_select_32k, 0, 16);
	tcase_add_loop_test(tc_mmc1_reg3_bank_select, mapper_001_reg3_prg_lo_bank_select_16k, 0, 16);
	tcase_add_loop_test(tc_mmc1_reg3_bank_select, mapper_001_reg3_prg_hi_bank_select_16k, 0, 16);
	suite_add_tcase(s, tc_mmc1_reg3_bank_select);
	tc_mmc1_prg_ram = tcase_create("MMC1 PRG RAM Tests");
	tcase_add_checked_fixture(tc_mmc1_prg_ram, setup, teardown);
	tcase_add_test(tc_mmc1_prg_ram, mapper_001_prg_ram_enabled_writes);
	tcase_add_test(tc_mmc1_prg_ram, mapper_001_prg_ram_disabled_writes);
	tcase_add_test(tc_mmc1_prg_ram, mapper_001_prg_ram_enabled_reads);
	tcase_add_test(tc_mmc1_prg_ram, mapper_001_prg_ram_disabled_reads);
	suite_add_tcase(s, tc_mmc1_prg_ram);
	tc_mmc1_other = tcase_create("MMC1 Misc. Tests");
	tcase_add_checked_fixture(tc_mmc1_other, setup, teardown);
	tcase_add_test(tc_mmc1_other, mapper_001_unmapped_open_bus_reads);
	tcase_add_test(tc_mmc1_other, mapper_001_prg_rom_reads);
	suite_add_tcase(s, tc_mmc1_other);

	return s;
}
