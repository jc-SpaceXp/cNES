#include <check.h>

#include "ppu.h"

Ppu2C02* ppu;
CpuPpuShare* cpu_ppu;
struct PpuMemoryMap* vram;

static void setup(void)
{
	CpuPpuShare* cpu_ppu = mmio_init();
	ppu = ppu_init(cpu_ppu);
	if (!cpu_ppu | !ppu) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to ppu struct");
	}
}

static void teardown(void)
{
	free(cpu_ppu);
	free(ppu);
}

static void vram_setup(void)
{
	setup();
	vram = malloc(sizeof(struct PpuMemoryMap)); // test double
	if (!vram) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to vram struct");
	}
}

static void vram_teardown(void)
{
	teardown();
	free(vram);
}

static void check_horizontal_nametable_A_mirroring(struct PpuMemoryMap* vram
                                                  , uint16_t addr_offset
                                                  , uint8_t expected_val)
{
	// nametable A is nametable 0 (0x2000) and nametable 1 (0x2400)
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2000 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2400 + addr_offset));
	ck_assert_uint_ne(expected_val, read_from_ppu_vram(vram, 0x2800 + addr_offset));
	ck_assert_uint_ne(expected_val, read_from_ppu_vram(vram, 0x2C00 + addr_offset));
}

static void check_horizontal_nametable_B_mirroring(struct PpuMemoryMap* vram
                                                  , uint16_t addr_offset
                                                  , uint8_t expected_val)
{
	// nametable B is nametable 2 (0x2800) and nametable 3 (0x2C00)
	ck_assert_uint_ne(expected_val, read_from_ppu_vram(vram, 0x2000 + addr_offset));
	ck_assert_uint_ne(expected_val, read_from_ppu_vram(vram, 0x2400 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2800 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2C00 + addr_offset));
}

static void check_vertical_nametable_A_mirroring(struct PpuMemoryMap* vram
                                                , uint16_t addr_offset
                                                , uint8_t expected_val)
{
	// nametable A is nametable 0 (0x2000) and nametable 2 (0x2800)
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2000 + addr_offset));
	ck_assert_uint_ne(expected_val, read_from_ppu_vram(vram, 0x2400 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2800 + addr_offset));
	ck_assert_uint_ne(expected_val, read_from_ppu_vram(vram, 0x2C00 + addr_offset));
}

static void check_vertical_nametable_B_mirroring(struct PpuMemoryMap* vram
                                                , uint16_t addr_offset
                                                , uint8_t expected_val)
{
	// nametable B is nametable 1 (0x2400) and nametable 3 (0x2C00)
	ck_assert_uint_ne(expected_val, read_from_ppu_vram(vram, 0x2000 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2400 + addr_offset));
	ck_assert_uint_ne(expected_val, read_from_ppu_vram(vram, 0x2800 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2C00 + addr_offset));
}

static void check_single_screen_A_nametable_mirroring(struct PpuMemoryMap* vram
                                                     , uint16_t addr_offset
                                                     , uint8_t expected_val)
{
	// nametable A is nametable 0 (0x2000) and nametable 1 (0x2400)
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2000 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2400 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2800 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2C00 + addr_offset));
	// no writes to nametable B should happen too
	ck_assert_uint_ne(vram->nametable_B[addr_offset]
	                 , read_from_ppu_vram(vram, 0x2C00 + addr_offset));
}

static void check_single_screen_B_nametable_mirroring(struct PpuMemoryMap* vram
                                                     , uint16_t addr_offset
                                                     , uint8_t expected_val)
{
	// nametable A is nametable 0 (0x2000) and nametable 1 (0x2400)
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2000 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2400 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2800 + addr_offset));
	ck_assert_uint_eq(expected_val, read_from_ppu_vram(vram, 0x2C00 + addr_offset));
	// no writes to nametable A should happen too
	ck_assert_uint_ne(vram->nametable_A[addr_offset]
	                 , read_from_ppu_vram(vram, 0x2400 + addr_offset));
}

START_TEST (pattern_table_0_writes_lower_bound)
{
	write_to_ppu_vram(vram, 0x0000, 0xD1);
	ck_assert_uint_eq(0xD1, vram->pattern_table_0[0x0000]);
}

START_TEST (pattern_table_0_writes_other_bound)
{
	write_to_ppu_vram(vram, 0x004B, 0x72);
	ck_assert_uint_eq(0x72, vram->pattern_table_0[0x004B]);
}

START_TEST (pattern_table_0_writes_upper_bound)
{
	write_to_ppu_vram(vram, 0x0FFF, 0x29);
	ck_assert_uint_eq(0x29, vram->pattern_table_0[0x0FFF]);
}

START_TEST (pattern_table_1_writes_lower_bound)
{
	write_to_ppu_vram(vram, 0x1000, 0xE1);
	ck_assert_uint_eq(0xE1, vram->pattern_table_1[0x0000]);
}

START_TEST (pattern_table_1_writes_other_bound)
{
	write_to_ppu_vram(vram, 0x11BD, 0x52);
	ck_assert_uint_eq(0x52, vram->pattern_table_1[0x11BD & 0x0FFF]);
}

START_TEST (pattern_table_1_writes_upper_bound)
{
	write_to_ppu_vram(vram, 0x1FFF, 0x3A);
	ck_assert_uint_eq(0x3A, vram->pattern_table_1[0x2FFF & 0x0FFF]);
}

// Nametable 0 is from 0x2000 to 0x23FF
START_TEST (nametable_0_writes_lower_bound)
{
	vram->nametable_0 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2000, 0x09);
	ck_assert_uint_eq(0x09, (*vram->nametable_0)[0x2000 & 0x03FF]);
}

START_TEST (nametable_0_writes_other_bound)
{
	vram->nametable_0 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x210D, 0x13);
	ck_assert_uint_eq(0x13, (*vram->nametable_0)[0x210D & 0x03FF]);
}

START_TEST (nametable_0_writes_upper_bound)
{
	vram->nametable_0 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x23FF, 0xA5);
	ck_assert_uint_eq(0xA5, (*vram->nametable_0)[0x23FF & 0x03FF]);
}

// Nametable 1 is from 0x2400 to 0x27FF
START_TEST (nametable_1_writes_lower_bound)
{
	vram->nametable_1 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2400, 0x11);
	ck_assert_uint_eq(0x11, (*vram->nametable_1)[0x2400 & 0x03FF]);
}

START_TEST (nametable_1_writes_other_bound)
{
	vram->nametable_1 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x260D, 0x46);
	ck_assert_uint_eq(0x46, (*vram->nametable_1)[0x260D & 0x03FF]);
}

START_TEST (nametable_1_writes_upper_bound)
{
	vram->nametable_1 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x27FF, 0x91);
	ck_assert_uint_eq(0x91, (*vram->nametable_1)[0x27FF & 0x03FF]);
}

// Nametable 2 is from 0x2800 to 0x2BFF
START_TEST (nametable_2_writes_lower_bound)
{
	vram->nametable_2 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2800, 0x3F);
	ck_assert_uint_eq(0x3F, (*vram->nametable_2)[0x2800 & 0x03FF]);
}

START_TEST (nametable_2_writes_other_bound)
{
	vram->nametable_2 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x29AD, 0x77);
	ck_assert_uint_eq(0x77, (*vram->nametable_2)[0x29AD & 0x03FF]);
}

START_TEST (nametable_2_writes_upper_bound)
{
	vram->nametable_2 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2BFF, 0xD8);
	ck_assert_uint_eq(0xD8, (*vram->nametable_2)[0x2BFF & 0x03FF]);
}

// Nametable 3 is from 0x2C00 to 0x2FFF
START_TEST (nametable_3_writes_lower_bound)
{
	vram->nametable_3 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2C00, 0x0B);
	ck_assert_uint_eq(0x0B, (*vram->nametable_3)[0x2C00 & 0x03FF]);
}

START_TEST (nametable_3_writes_other_bound)
{
	vram->nametable_3 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2EEA, 0x93);
	ck_assert_uint_eq(0x93, (*vram->nametable_3)[0x2EEA & 0x03FF]);
}

START_TEST (nametable_3_writes_upper_bound)
{
	vram->nametable_3 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2FFF, 0xFF);
	ck_assert_uint_eq(0xFF, (*vram->nametable_3)[0x2FFF & 0x03FF]);
}

// Nametable 0 is from 0x2000 to 0x23FF
START_TEST (nametable_0_mirror_writes_lower_bound)
{
	vram->nametable_0 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3000, 0x09);
	ck_assert_uint_eq(0x09, (*vram->nametable_0)[0x3000 & 0x03FF]);
}

START_TEST (nametable_0_mirror_writes_other_bound)
{
	vram->nametable_0 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x310D, 0x13);
	ck_assert_uint_eq(0x13, (*vram->nametable_0)[0x310D & 0x03FF]);
}

START_TEST (nametable_0_mirror_writes_upper_bound)
{
	vram->nametable_0 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x33FF, 0xA5);
	ck_assert_uint_eq(0xA5, (*vram->nametable_0)[0x33FF & 0x03FF]);
}

// Nametable 1 is from 0x2400 to 0x27FF
START_TEST (nametable_1_mirror_writes_lower_bound)
{
	vram->nametable_1 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3400, 0x11);
	ck_assert_uint_eq(0x11, (*vram->nametable_1)[0x3400 & 0x03FF]);
}

START_TEST (nametable_1_mirror_writes_other_bound)
{
	vram->nametable_1 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x360D, 0x46);
	ck_assert_uint_eq(0x46, (*vram->nametable_1)[0x360D & 0x03FF]);
}

START_TEST (nametable_1_mirror_writes_upper_bound)
{
	vram->nametable_1 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x37FF, 0x91);
	ck_assert_uint_eq(0x91, (*vram->nametable_1)[0x37FF & 0x03FF]);
}

// Nametable 2 is from 0x2800 to 0x2BFF
START_TEST (nametable_2_mirror_writes_lower_bound)
{
	vram->nametable_2 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3800, 0x3F);
	ck_assert_uint_eq(0x3F, (*vram->nametable_2)[0x3800 & 0x03FF]);
}

START_TEST (nametable_2_mirror_writes_other_bound)
{
	vram->nametable_2 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x39AD, 0x77);
	ck_assert_uint_eq(0x77, (*vram->nametable_2)[0x39AD & 0x03FF]);
}

START_TEST (nametable_2_mirror_writes_upper_bound)
{
	vram->nametable_2 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3BFF, 0xD8);
	ck_assert_uint_eq(0xD8, (*vram->nametable_2)[0x3BFF & 0x03FF]);
}

// Nametable 3 is from 0x2C00 to 0x2FFF
START_TEST (nametable_3_partial_mirror_writes_lower_bound)
{
	vram->nametable_3 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3C00, 0x0B);
	ck_assert_uint_eq(0x0B, (*vram->nametable_3)[0x3C00 & 0x03FF]);
}

START_TEST (nametable_3_partial_mirror_writes_other_bound)
{
	vram->nametable_3 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3DDA, 0x93);
	ck_assert_uint_eq(0x93, (*vram->nametable_3)[0x3DDA & 0x03FF]);
}

START_TEST (nametable_3_partial_mirror_writes_upper_bound)
{
	vram->nametable_3 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3EFF, 0xFF);
	ck_assert_uint_eq(0xFF, (*vram->nametable_3)[0x3EFF & 0x03FF]);
}

// Palette RAM is from 0x3F00 to 0x3F1F
START_TEST (palette_ram_writes_lower_bound)
{
	write_to_ppu_vram(vram, 0x3F00, 0xA0);
	ck_assert_uint_eq(0xA0, vram->palette_ram[0x3F00 & 0x001F]);
}

START_TEST (palette_ram_writes_other_bound)
{
	write_to_ppu_vram(vram, 0x3F11, 0xC6);
	ck_assert_uint_eq(0xC6, vram->palette_ram[0x3F11 & 0x001F]);
}

START_TEST (palette_ram_writes_upper_bound)
{
	write_to_ppu_vram(vram, 0x3F1F, 0xE1);
	ck_assert_uint_eq(0xE1, vram->palette_ram[0x3F1F & 0x001F]);
}

// Palette RAM mirrors are from 0x3F20 to 0x3FFF
START_TEST (palette_ram_mirror_writes_lower_bound)
{
	write_to_ppu_vram(vram, 0x3F20, 0x53);
	ck_assert_uint_eq(0x53, vram->palette_ram[0x3F20 & 0x001F]);
}

START_TEST (palette_ram_mirror_writes_other_bound_1)
{
	write_to_ppu_vram(vram, 0x3F4A, 0x66);
	ck_assert_uint_eq(0x66, vram->palette_ram[0x3F4A & 0x001F]);
}

START_TEST (palette_ram_mirror_writes_other_bound_2)
{
	write_to_ppu_vram(vram, 0x3F91, 0x77);
	ck_assert_uint_eq(0x77, vram->palette_ram[0x3F91 & 0x001F]);
}

START_TEST (palette_ram_mirror_writes_upper_bound)
{
	write_to_ppu_vram(vram, 0x3FFF, 0xB0);
	ck_assert_uint_eq(0xB0, vram->palette_ram[0x3FFF & 0x001F]);
}

START_TEST (writes_past_upper_bound_have_no_effect)
{
	vram->nametable_0 = &vram->nametable_A;
	vram->nametable_1 = &vram->nametable_A;
	vram->nametable_2 = &vram->nametable_A;
	vram->nametable_3 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x4FFF, 0xAA);

	ck_assert_uint_ne(0xAA, vram->pattern_table_0[0x4FFF & 0x0FFF]);
	ck_assert_uint_ne(0xAA, vram->pattern_table_1[0x4FFF & 0x0FFF]);
	ck_assert_uint_ne(0xAA, (*vram->nametable_0)[0x4FFF & 0x03FF]);
	ck_assert_uint_ne(0xAA, (*vram->nametable_1)[0x4FFF & 0x03FF]);
	ck_assert_uint_ne(0xAA, (*vram->nametable_2)[0x4FFF & 0x03FF]);
	ck_assert_uint_ne(0xAA, (*vram->nametable_3)[0x4FFF & 0x03FF]);
	ck_assert_uint_ne(0xAA, vram->palette_ram[0x4FFF & 0x001F]);
}

START_TEST (pattern_table_0_reads_lower_bound)
{
	write_to_ppu_vram(vram, 0x0000, 0xD1);
	ck_assert_uint_eq(0xD1, read_from_ppu_vram(vram, 0x0000));
}

START_TEST (pattern_table_0_reads_other_bound)
{
	write_to_ppu_vram(vram, 0x004B, 0x72);
	ck_assert_uint_eq(0x72, read_from_ppu_vram(vram, 0x004B));
}

START_TEST (pattern_table_0_reads_upper_bound)
{
	write_to_ppu_vram(vram, 0x0FFF, 0x29);
	ck_assert_uint_eq(0x29, read_from_ppu_vram(vram, 0x0FFF));
}

START_TEST (pattern_table_1_reads_lower_bound)
{
	write_to_ppu_vram(vram, 0x1000, 0xE1);
	ck_assert_uint_eq(0xE1, read_from_ppu_vram(vram, 0x1000));
}

START_TEST (pattern_table_1_reads_other_bound)
{
	write_to_ppu_vram(vram, 0x11BD, 0x52);
	ck_assert_uint_eq(0x52, read_from_ppu_vram(vram, 0x11BD));
}

START_TEST (pattern_table_1_reads_upper_bound)
{
	write_to_ppu_vram(vram, 0x1FFF, 0x3A);
	ck_assert_uint_eq(0x3A, read_from_ppu_vram(vram, 0x1FFF));
}

// Nametable 0 is from 0x2000 to 0x23FF
START_TEST (nametable_0_reads_lower_bound)
{
	vram->nametable_0 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2000, 0x09);
	ck_assert_uint_eq(0x09, read_from_ppu_vram(vram, 0x2000));
}

START_TEST (nametable_0_reads_other_bound)
{
	vram->nametable_0 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x210D, 0x13);
	ck_assert_uint_eq(0x13, read_from_ppu_vram(vram, 0x210D));
}

START_TEST (nametable_0_reads_upper_bound)
{
	vram->nametable_0 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x23FF, 0xA5);
	ck_assert_uint_eq(0xA5, read_from_ppu_vram(vram, 0x23FF));
}

// Nametable 1 is from 0x2400 to 0x27FF
START_TEST (nametable_1_reads_lower_bound)
{
	vram->nametable_1 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2400, 0x11);
	ck_assert_uint_eq(0x11, read_from_ppu_vram(vram, 0x2400));
}

START_TEST (nametable_1_reads_other_bound)
{
	vram->nametable_1 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x260D, 0x46);
	ck_assert_uint_eq(0x46, read_from_ppu_vram(vram, 0x260D));
}

START_TEST (nametable_1_reads_upper_bound)
{
	vram->nametable_1 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x27FF, 0x91);
	ck_assert_uint_eq(0x91, read_from_ppu_vram(vram, 0x27FF));
}

// Nametable 2 is from 0x2800 to 0x2BFF
START_TEST (nametable_2_reads_lower_bound)
{
	vram->nametable_2 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2800, 0x3F);
	ck_assert_uint_eq(0x3F, read_from_ppu_vram(vram, 0x2800));
}

START_TEST (nametable_2_reads_other_bound)
{
	vram->nametable_2 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x29AD, 0x77);
	ck_assert_uint_eq(0x77, read_from_ppu_vram(vram, 0x29AD));
}

START_TEST (nametable_2_reads_upper_bound)
{
	vram->nametable_2 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2BFF, 0xD8);
	ck_assert_uint_eq(0xD8, read_from_ppu_vram(vram, 0x2BFF));
}

// Nametable 3 is from 0x2C00 to 0x2FFF
START_TEST (nametable_3_reads_lower_bound)
{
	vram->nametable_3 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2C00, 0x0B);
	ck_assert_uint_eq(0x0B, read_from_ppu_vram(vram, 0x2C00));
}

START_TEST (nametable_3_reads_other_bound)
{
	vram->nametable_3 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2EEA, 0x93);
	ck_assert_uint_eq(0x93, read_from_ppu_vram(vram, 0x2EEA));
}

START_TEST (nametable_3_reads_upper_bound)
{
	vram->nametable_3 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x2FFF, 0xFF);
	ck_assert_uint_eq(0xFF, read_from_ppu_vram(vram, 0x2FFF));
}

// Nametable 0 is from 0x2000 to 0x23FF
START_TEST (nametable_0_mirror_reads_lower_bound)
{
	vram->nametable_0 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3000, 0x09);
	ck_assert_uint_eq(0x09, read_from_ppu_vram(vram, 0x3000));
}

START_TEST (nametable_0_mirror_reads_other_bound)
{
	vram->nametable_0 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x310D, 0x13);
	ck_assert_uint_eq(0x13, read_from_ppu_vram(vram, 0x310D));
}

START_TEST (nametable_0_mirror_reads_upper_bound)
{
	vram->nametable_0 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x33FF, 0xA5);
	ck_assert_uint_eq(0xA5, read_from_ppu_vram(vram, 0x33FF));
}

// Nametable 1 is from 0x2400 to 0x27FF
START_TEST (nametable_1_mirror_reads_lower_bound)
{
	vram->nametable_1 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3400, 0x11);
	ck_assert_uint_eq(0x11, read_from_ppu_vram(vram, 0x3400));
}

START_TEST (nametable_1_mirror_reads_other_bound)
{
	vram->nametable_1 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x360D, 0x46);
	ck_assert_uint_eq(0x46, read_from_ppu_vram(vram, 0x360D));
}

START_TEST (nametable_1_mirror_reads_upper_bound)
{
	vram->nametable_1 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x37FF, 0x91);
	ck_assert_uint_eq(0x91, read_from_ppu_vram(vram, 0x37FF));
}

// Nametable 2 is from 0x2800 to 0x2BFF
START_TEST (nametable_2_mirror_reads_lower_bound)
{
	vram->nametable_2 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3800, 0x3F);
	ck_assert_uint_eq(0x3F, read_from_ppu_vram(vram, 0x3800));
}

START_TEST (nametable_2_mirror_reads_other_bound)
{
	vram->nametable_2 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x39AD, 0x77);
	ck_assert_uint_eq(0x77, read_from_ppu_vram(vram, 0x39AD));
}

START_TEST (nametable_2_mirror_reads_upper_bound)
{
	vram->nametable_2 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3BFF, 0xD8);
	ck_assert_uint_eq(0xD8, read_from_ppu_vram(vram, 0x3BFF));
}

// Nametable 3 is from 0x2C00 to 0x2FFF
START_TEST (nametable_3_partial_mirror_reads_lower_bound)
{
	vram->nametable_3 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3C00, 0x0B);
	ck_assert_uint_eq(0x0B, read_from_ppu_vram(vram, 0x3C00));
}

START_TEST (nametable_3_partial_mirror_reads_other_bound)
{
	vram->nametable_3 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3DDA, 0x93);
	ck_assert_uint_eq(0x93, read_from_ppu_vram(vram, 0x3DDA));
}

START_TEST (nametable_3_partial_mirror_reads_upper_bound)
{
	vram->nametable_3 = &vram->nametable_B;
	write_to_ppu_vram(vram, 0x3EFF, 0xFF);
	ck_assert_uint_eq(0xFF, read_from_ppu_vram(vram, 0x3EFF));
}

// Palette RAM is from 0x3F00 to 0x3F1F
START_TEST (palette_ram_reads_lower_bound)
{
	write_to_ppu_vram(vram, 0x3F00, 0xA0);
	ck_assert_uint_eq(0xA0, read_from_ppu_vram(vram, 0x3F00));
}

START_TEST (palette_ram_reads_other_bound)
{
	write_to_ppu_vram(vram, 0x3F11, 0xC6);
	ck_assert_uint_eq(0xC6, read_from_ppu_vram(vram, 0x3F11));
}

START_TEST (palette_ram_reads_upper_bound)
{
	write_to_ppu_vram(vram, 0x3F1F, 0xE1);
	ck_assert_uint_eq(0xE1, read_from_ppu_vram(vram, 0x3F1F));
}

// Palette RAM mirrors are from 0x3F20 to 0x3FFF
START_TEST (palette_ram_mirror_reads_lower_bound)
{
	write_to_ppu_vram(vram, 0x3F20, 0x53);
	ck_assert_uint_eq(0x53, read_from_ppu_vram(vram, 0x3F20));
}

START_TEST (palette_ram_mirror_reads_other_bound_1)
{
	write_to_ppu_vram(vram, 0x3F4A, 0x66);
	ck_assert_uint_eq(0x66, read_from_ppu_vram(vram, 0x3F4A));
}

START_TEST (palette_ram_mirror_reads_other_bound_2)
{
	write_to_ppu_vram(vram, 0x3F91, 0x77);
	ck_assert_uint_eq(0x77, read_from_ppu_vram(vram, 0x3F91));
}

START_TEST (palette_ram_mirror_reads_upper_bound)
{
	write_to_ppu_vram(vram, 0x3FFF, 0xB0);
	ck_assert_uint_eq(0xB0, read_from_ppu_vram(vram, 0x3FFF));
}

START_TEST (nametable_mirroring_horizontal)
{
	vram->nametable_0 = &vram->nametable_A;
	vram->nametable_1 = &vram->nametable_A;
	vram->nametable_2 = &vram->nametable_B;
	vram->nametable_3 = &vram->nametable_B;

	memset(vram->nametable_A, 0x01, sizeof(vram->nametable_A));
	memset(vram->nametable_B, 0x34, sizeof(vram->nametable_B));

	ck_assert_mem_eq((*vram->nametable_0), vram->nametable_A, sizeof(vram->nametable_A));
	ck_assert_mem_eq((*vram->nametable_1), vram->nametable_A, sizeof(vram->nametable_A));
	ck_assert_mem_eq((*vram->nametable_2), vram->nametable_B, sizeof(vram->nametable_B));
	ck_assert_mem_eq((*vram->nametable_3), vram->nametable_B, sizeof(vram->nametable_B));
}

START_TEST (nametable_mirroring_horizontal_read_writes)
{
	vram->nametable_0 = &vram->nametable_A;
	vram->nametable_1 = &vram->nametable_A;
	vram->nametable_2 = &vram->nametable_B;
	vram->nametable_3 = &vram->nametable_B;

	write_to_ppu_vram(vram, 0x2000 + 0x0200, 0x23);
	write_to_ppu_vram(vram, 0x2400 + 0x0110, 0x28);
	write_to_ppu_vram(vram, 0x2800 + 0x000F, 0xB0);
	write_to_ppu_vram(vram, 0x2C00 + 0x001F, 0xBA);

	// helper ck_assert functions
	check_horizontal_nametable_A_mirroring(vram, 0x0200, 0x23);
	check_horizontal_nametable_A_mirroring(vram, 0x0110, 0x28);
	// check other nametable
	check_horizontal_nametable_B_mirroring(vram, 0x000F, 0xB0);
	check_horizontal_nametable_B_mirroring(vram, 0x001F, 0xBA);
}

START_TEST (nametable_mirroring_vertical)
{
	vram->nametable_0 = &vram->nametable_A;
	vram->nametable_1 = &vram->nametable_B;
	vram->nametable_2 = &vram->nametable_A;
	vram->nametable_3 = &vram->nametable_B;

	memset(vram->nametable_A, 0x02, sizeof(vram->nametable_A));
	memset(vram->nametable_B, 0x13, sizeof(vram->nametable_B));

	ck_assert_mem_eq((*vram->nametable_0), vram->nametable_A, sizeof(vram->nametable_A));
	ck_assert_mem_eq((*vram->nametable_1), vram->nametable_B, sizeof(vram->nametable_B));
	ck_assert_mem_eq((*vram->nametable_2), vram->nametable_A, sizeof(vram->nametable_A));
	ck_assert_mem_eq((*vram->nametable_3), vram->nametable_B, sizeof(vram->nametable_B));
}

START_TEST (nametable_mirroring_vertical_read_writes)
{
	vram->nametable_0 = &vram->nametable_A;
	vram->nametable_1 = &vram->nametable_B;
	vram->nametable_2 = &vram->nametable_A;
	vram->nametable_3 = &vram->nametable_B;

	write_to_ppu_vram(vram, 0x2000 + 0x0200, 0x23);
	write_to_ppu_vram(vram, 0x2800 + 0x0000, 0x17);
	write_to_ppu_vram(vram, 0x2400 + 0x0110, 0x4C);
	write_to_ppu_vram(vram, 0x2C00 + 0x0301, 0x88);

	// helper ck_assert functions
	check_vertical_nametable_A_mirroring(vram, 0x0200, 0x23);
	check_vertical_nametable_A_mirroring(vram, 0x0000, 0x17);
	// check other nametable
	check_vertical_nametable_B_mirroring(vram, 0x0110, 0x4C);
	check_vertical_nametable_B_mirroring(vram, 0x0301, 0x88);
}

START_TEST (nametable_mirroring_single_screen_A)
{
	vram->nametable_0 = &vram->nametable_A;
	vram->nametable_1 = &vram->nametable_A;
	vram->nametable_2 = &vram->nametable_A;
	vram->nametable_3 = &vram->nametable_A;

	memset(vram->nametable_A, 0xF0, sizeof(vram->nametable_A));
	memset(vram->nametable_B, 0xFF, sizeof(vram->nametable_B));

	ck_assert_mem_eq((*vram->nametable_0), vram->nametable_A, sizeof(vram->nametable_A));
	ck_assert_mem_eq((*vram->nametable_1), vram->nametable_A, sizeof(vram->nametable_A));
	ck_assert_mem_eq((*vram->nametable_2), vram->nametable_A, sizeof(vram->nametable_A));
	ck_assert_mem_eq((*vram->nametable_3), vram->nametable_A, sizeof(vram->nametable_A));
}

START_TEST (nametable_mirroring_single_screen_A_read_writes)
{
	vram->nametable_0 = &vram->nametable_A;
	vram->nametable_1 = &vram->nametable_A;
	vram->nametable_2 = &vram->nametable_A;
	vram->nametable_3 = &vram->nametable_A;

	write_to_ppu_vram(vram, 0x2000 + 0x02D0, 0x23);
	write_to_ppu_vram(vram, 0x2400 + 0x0111, 0x28);
	write_to_ppu_vram(vram, 0x2800 + 0x000F, 0xB0);
	write_to_ppu_vram(vram, 0x2C00 + 0x03AF, 0x0A);

	// check nametable 0 writes are mirrored correctly
	check_single_screen_A_nametable_mirroring(vram, 0x02D0, 0x23);
	// check nametable 1 writes are mirrored correctly
	check_single_screen_A_nametable_mirroring(vram, 0x0111, 0x28);
	// check nametable 2 writes are mirrored correctly
	check_single_screen_A_nametable_mirroring(vram, 0x000F, 0xB0);
	// check nametable 3 writes are mirrored correctly
	check_single_screen_A_nametable_mirroring(vram, 0x03AF, 0x0A);
}

START_TEST (nametable_mirroring_single_screen_B)
{
	vram->nametable_0 = &vram->nametable_B;
	vram->nametable_1 = &vram->nametable_B;
	vram->nametable_2 = &vram->nametable_B;
	vram->nametable_3 = &vram->nametable_B;

	memset(vram->nametable_A, 0xFF, sizeof(vram->nametable_B));
	memset(vram->nametable_B, 0x0F, sizeof(vram->nametable_B));

	ck_assert_mem_eq((*vram->nametable_0), vram->nametable_B, sizeof(vram->nametable_B));
	ck_assert_mem_eq((*vram->nametable_1), vram->nametable_B, sizeof(vram->nametable_B));
	ck_assert_mem_eq((*vram->nametable_2), vram->nametable_B, sizeof(vram->nametable_B));
	ck_assert_mem_eq((*vram->nametable_3), vram->nametable_B, sizeof(vram->nametable_B));
}

START_TEST (nametable_mirroring_single_screen_B_read_writes)
{
	vram->nametable_0 = &vram->nametable_B;
	vram->nametable_1 = &vram->nametable_B;
	vram->nametable_2 = &vram->nametable_B;
	vram->nametable_3 = &vram->nametable_B;

	write_to_ppu_vram(vram, 0x2000 + 0x0086, 0x02);
	write_to_ppu_vram(vram, 0x2400 + 0x020C, 0x58);
	write_to_ppu_vram(vram, 0x2800 + 0x000F, 0x94);
	write_to_ppu_vram(vram, 0x2C00 + 0x01FF, 0xF8);

	// check nametable 0 writes are mirrored correctly
	check_single_screen_B_nametable_mirroring(vram, 0x0086, 0x02);
	// check nametable 1 writes are mirrored correctly
	check_single_screen_B_nametable_mirroring(vram, 0x020C, 0x58);
	// check nametable 2 writes are mirrored correctly
	check_single_screen_B_nametable_mirroring(vram, 0x000F, 0x94);
	// check nametable 3 writes are mirrored correctly
	check_single_screen_B_nametable_mirroring(vram, 0x01FF, 0xF8);
}


Suite* ppu_suite(void)
{
	Suite* s;
	TCase* tc_ppu_vram_read_writes;
	TCase* tc_ppu_rendering;

	s = suite_create("Ppu Tests");
	tc_ppu_vram_read_writes = tcase_create("VRAM Read/Write Tests");
	tcase_add_checked_fixture(tc_ppu_vram_read_writes, vram_setup, vram_teardown);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_0_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_0_writes_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_0_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_1_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_1_writes_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_1_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_writes_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_writes_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_writes_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_writes_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_mirror_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_mirror_writes_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_mirror_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_mirror_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_mirror_writes_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_mirror_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_mirror_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_mirror_writes_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_mirror_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_partial_mirror_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_partial_mirror_writes_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_partial_mirror_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_writes_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_mirror_writes_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_mirror_writes_other_bound_1);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_mirror_writes_other_bound_2);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_mirror_writes_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, writes_past_upper_bound_have_no_effect);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_0_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_0_reads_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_0_reads_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_1_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_1_reads_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, pattern_table_1_reads_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_reads_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_reads_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_reads_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_reads_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_reads_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_reads_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_reads_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_reads_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_mirror_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_mirror_reads_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_0_mirror_reads_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_mirror_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_mirror_reads_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_1_mirror_reads_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_mirror_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_mirror_reads_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_2_mirror_reads_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_partial_mirror_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_partial_mirror_reads_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, nametable_3_partial_mirror_reads_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_reads_other_bound);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_reads_upper_bound);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_mirror_reads_lower_bound);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_mirror_reads_other_bound_1);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_mirror_reads_other_bound_2);
	tcase_add_test(tc_ppu_vram_read_writes, palette_ram_mirror_reads_upper_bound);
	suite_add_tcase(s, tc_ppu_vram_read_writes);
	tc_ppu_rendering = tcase_create("PPU Rendering Related Tests");
	tcase_add_checked_fixture(tc_ppu_rendering, vram_setup, vram_teardown);
	tcase_add_test(tc_ppu_rendering, nametable_mirroring_horizontal);
	tcase_add_test(tc_ppu_rendering, nametable_mirroring_horizontal_read_writes);
	tcase_add_test(tc_ppu_rendering, nametable_mirroring_vertical);
	tcase_add_test(tc_ppu_rendering, nametable_mirroring_vertical_read_writes);
	tcase_add_test(tc_ppu_rendering, nametable_mirroring_single_screen_A);
	tcase_add_test(tc_ppu_rendering, nametable_mirroring_single_screen_A_read_writes);
	tcase_add_test(tc_ppu_rendering, nametable_mirroring_single_screen_B);
	tcase_add_test(tc_ppu_rendering, nametable_mirroring_single_screen_B_read_writes);
	suite_add_tcase(s, tc_ppu_rendering);

	return s;
}
