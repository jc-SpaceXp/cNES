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


Suite* ppu_suite(void)
{
	Suite* s;
	TCase* tc_ppu_vram_read_writes;

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
	suite_add_tcase(s, tc_ppu_vram_read_writes);

	return s;
}
