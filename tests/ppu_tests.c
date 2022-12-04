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

/* When the PPU is rendering, its internal vram (and temporary vram) address registers
 * are formatted as follows: 0yyy NNYY YYYX XXXX
 *
 *   where y is fine y
 *         NN is the base nametbale address, 0 ($2000), 1($2400), 2 ($2800), 3 ($2C00)
 *         Y is Coarse Y, tile offset in Y direction, incremented every 8 pixels down
 *         X is Coarse X, tile offset in X direction, incremented every 8 pixels across
 *
 * Function is needed to test functions that use this internal encoding of the vram
 * address
 *
 * Much easier to build the correct addresses from this function, rahter than
 * manually masking and shifting the nametable address
 */
static uint16_t nametable_vram_address_from_scroll_offsets(const unsigned nametable_address
                                                          , unsigned int fine_y
                                                          , const unsigned int coarse_x
                                                          , const unsigned int coarse_y)
{
	// Keep fine y maxed to 7 when out of bounds
	// otherwise it just takes on the lowest 3 bits
	if (fine_y > 7) { fine_y = 7; }

	// minus 0x2000 from base nametable address, e.g. 0x2000, 0x2400 etc minus 0x2000
	uint16_t vram_address = nametable_address & 0x0C00; // sets NN bits
	vram_address |= (fine_y & 0x07) << 12;
	vram_address |= (coarse_y % 30) << 5;
	vram_address |= coarse_x & 0x1F;

	return vram_address;
}

/* Calculating the attribute address from Coarse X/Y is a little different
 * The encoding of the vram address changes from:
 *   0yyy NNYY YYYX XXXX
 * to
 *   0010 NN11 11YY YXXX
 *
 * Where
 *   Coarse X's lowest two bits are shifted out
 *   Coarse Y & 0x0C merges into Coarse X (upper two bits from the shift out)
 *   Coarse Y's upper bit is shifted down by 4 bits (as are the other Coarse Y bits too)
 *   fine y is ignored
 */
static uint16_t attribute_address_from_nametable_scroll_offsets(const unsigned nametable_address
                                                               , const unsigned int coarse_x
                                                               , const unsigned int coarse_y)
{
	// minus 0x2000 from base nametable address, e.g. 0x2000, 0x2400 etc minus 0x2000
	uint16_t attribute_address = nametable_address & 0x0C00; // sets NN bits
	attribute_address |= 0x23C0;  // hard-coded attribute address
	attribute_address |= ((coarse_y % 30) & 0x1C) << 1;
	attribute_address |= (coarse_x & 0x1F) >> 2;

	return attribute_address;
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

START_TEST (nametable_x_offset_is_valid_for_all_coarse_x)
{
	// coarse x can be between 0-63 to refer to any x pos
	// of a tile in any of the 4 nametable regions, offset
	// address is relative to the closest nametable region
	// (e.g. 0x2000, 0x2400 etc.)
	ck_assert_uint_eq(nametable_x_offset_address(_i), _i % 32);
}

START_TEST (nametable_y_offset_is_valid_for_all_coarse_y)
{
	// coarse y can be between 0-59 to refer to any y pos
	// of a tile in any of the 4 nametable regions
	ck_assert_uint_eq(nametable_y_offset_address(_i), (_i % 30) << 5);
}

START_TEST (fetch_nametable_byte_nametable_0_addr_no_fine_y)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_B;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_B;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	unsigned fine_y = 0;
	unsigned coarse_x = 1;
	unsigned coarse_y = 17;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2000, fine_y
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0x21);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0x21);
}

START_TEST (fetch_nametable_byte_nametable_0_addr_fine_y_is_ignored)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_B;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_B;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	unsigned fine_y = 3;
	unsigned coarse_x = 1;
	unsigned coarse_y = 17;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2000, fine_y
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0x21);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0x21);
}

START_TEST (fetch_nametable_byte_nametable_0_addr_in_attribute_table)
{
	// Possible that the current vram address is in the attribute table
	// Still must read the attribute table byte as if it was in the nametable section
	ppu->vram.nametable_0 = &ppu->vram.nametable_B;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_B;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	ppu->vram_addr = 0x03DC;
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0xAC);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0xAC);
}

START_TEST (fetch_nametable_byte_nametable_1_addr_no_fine_y)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_A;
	ppu->vram.nametable_2 = &ppu->vram.nametable_B;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	ppu->vram_addr = 0x06B1;
	unsigned fine_y = 0;
	unsigned coarse_x = 1;
	unsigned coarse_y = 19;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2400, fine_y
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0xB1);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0xB1);
}

START_TEST (fetch_nametable_byte_nametable_1_addr_fine_y_is_ignored)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_A;
	ppu->vram.nametable_2 = &ppu->vram.nametable_B;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	unsigned fine_y = 1;
	unsigned coarse_x = 1;
	unsigned coarse_y = 19;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2400, fine_y
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0xB1);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0xB1);
}

START_TEST (fetch_nametable_byte_nametable_1_addr_in_attribute_table)
{
	// Possible that the current vram address is in the attribute table
	// Still must read the attribute table byte as if it was in the nametable section
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_A;
	ppu->vram.nametable_2 = &ppu->vram.nametable_B;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	ppu->vram_addr = 0x07C1;
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0xA1);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0xA1);
}

START_TEST (fetch_nametable_byte_nametable_2_addr_no_fine_y)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_A;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_A;
	unsigned fine_y = 0;
	unsigned coarse_x = 1;
	unsigned coarse_y = 0;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2800, fine_y
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0x80);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0x80);
}

START_TEST (fetch_nametable_byte_nametable_2_addr_fine_y_is_ignored)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_A;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_A;
	unsigned fine_y = 2;
	unsigned coarse_x = 1;
	unsigned coarse_y = 0;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2800, fine_y
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0x80);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0x80);
}

START_TEST (fetch_nametable_byte_nametable_2_addr_in_attribute_table)
{
	// Possible that the current vram address is in the attribute table
	// Still must read the attribute table byte as if it was in the nametable section
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_A;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_A;
	ppu->vram_addr = 0x0BEF;
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0xAF);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0xAF);
}

START_TEST (fetch_nametable_byte_nametable_3_addr_no_fine_y)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_B;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_A;
	unsigned fine_y = 0;
	unsigned coarse_x = 13;
	unsigned coarse_y = 23;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2C00, fine_y
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0xED);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0xED);
}

START_TEST (fetch_nametable_byte_nametable_3_addr_fine_y_is_ignored)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_B;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_A;
	unsigned fine_y = 7;
	unsigned coarse_x = 13;
	unsigned coarse_y = 23;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2C00, fine_y
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0xED);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0xED);
}

START_TEST (fetch_nametable_byte_nametable_3_addr_in_attribute_table)
{
	// Possible that the current vram address is in the attribute table
	// Still must read the attribute table byte as if it was in the nametable section
	ppu->vram.nametable_0 = &ppu->vram.nametable_B;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_A;
	ppu->vram_addr = 0x0FEB;
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0xAD);

	fetch_nt_byte(ppu);

	ck_assert_uint_eq(ppu->nt_byte, 0xAD);
}

START_TEST (vram_encoder_nametable_0_no_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x0000, nametable_vram_address_from_scroll_offsets(0x2000, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_0_coarse_x_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 1;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x0001, nametable_vram_address_from_scroll_offsets(0x23FF, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_0_coarse_x_y_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 2;
	unsigned coarse_y = 1;

	ck_assert_uint_eq(0x0022, nametable_vram_address_from_scroll_offsets(0x2008, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_0_coarse_x_y_offset_max)
{
	// address before attribute bytes (0x23C0 onwards)
	unsigned fine_y = 0;
	unsigned coarse_x = 31;
	unsigned coarse_y = 29;

	ck_assert_uint_eq(0x03BF, nametable_vram_address_from_scroll_offsets(0x2112, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_0_fine_y_only)
{
	unsigned fine_y = 7;
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x7000, nametable_vram_address_from_scroll_offsets(0x3000, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_0_all_offsets)
{
	unsigned fine_y = 4;
	unsigned coarse_x = 12;
	unsigned coarse_y = 5;

	ck_assert_uint_eq(0x40AC, nametable_vram_address_from_scroll_offsets(0x0000, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_0_coarse_x_out_of_bounds)
{
	// address shouldn't bleed into the adjacent nametable!
	unsigned fine_y = 0;
	unsigned coarse_x = 40; // 8 * 1
	unsigned coarse_y = 3;

	ck_assert_uint_eq(0x0068, nametable_vram_address_from_scroll_offsets(0x239B, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_0_coarse_y_out_of_bounds)
{
	// address shouldn't bleed into the attribute tables!
	unsigned fine_y = 0;
	unsigned coarse_x = 0;
	unsigned coarse_y = 38; // 8 * 0x0020

	ck_assert_uint_eq(0x0100, nametable_vram_address_from_scroll_offsets(0x239B, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_1_no_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x0400, nametable_vram_address_from_scroll_offsets(0x0400, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_1_coarse_x_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 4;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x0404, nametable_vram_address_from_scroll_offsets(0x24FF, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_1_coarse_x_y_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 1;
	unsigned coarse_y = 2;

	ck_assert_uint_eq(0x0441, nametable_vram_address_from_scroll_offsets(0x2778, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_1_coarse_x_y_offset_max)
{
	// address before attribute bytes (0x23C0 onwards)
	unsigned fine_y = 0;
	unsigned coarse_x = 31;
	unsigned coarse_y = 29;

	ck_assert_uint_eq(0x07BF, nametable_vram_address_from_scroll_offsets(0x2512, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_1_fine_y_only)
{
	unsigned fine_y = 7;
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x7400, nametable_vram_address_from_scroll_offsets(0x3400, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_1_all_offsets)
{
	unsigned fine_y = 4;
	unsigned coarse_x = 12;
	unsigned coarse_y = 5;

	ck_assert_uint_eq(0x44AC, nametable_vram_address_from_scroll_offsets(0x2400, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_1_fine_y_out_of_bounds)
{
	unsigned fine_y = 100; // should be restricted to 7
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x7400, nametable_vram_address_from_scroll_offsets(0x2470, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_1_coarse_x_out_of_bounds)
{
	// address shouldn't bleed into the adjacent nametable!
	unsigned fine_y = 0;
	unsigned coarse_x = 39; // 7 * 1
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x0407, nametable_vram_address_from_scroll_offsets(0x2687, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_1_coarse_y_out_of_bounds)
{
	// address shouldn't bleed into the attribute tables!
	unsigned fine_y = 0;
	unsigned coarse_x = 0;
	unsigned coarse_y = 39; // 9 * 0x0020

	ck_assert_uint_eq(0x0520, nametable_vram_address_from_scroll_offsets(0x2439, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_2_no_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x0800, nametable_vram_address_from_scroll_offsets(0x0800, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_2_coarse_x_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 31;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x081F, nametable_vram_address_from_scroll_offsets(0x281F, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_2_coarse_x_y_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 4;
	unsigned coarse_y = 10;

	ck_assert_uint_eq(0x0944, nametable_vram_address_from_scroll_offsets(0x2878, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_2_coarse_x_y_offset_max)
{
	// address before attribute bytes (0x23C0 onwards)
	unsigned fine_y = 0;
	unsigned coarse_x = 31;
	unsigned coarse_y = 29;

	ck_assert_uint_eq(0x0BBF, nametable_vram_address_from_scroll_offsets(0x2812, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_2_fine_y_only)
{
	unsigned fine_y = 3;
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x3800, nametable_vram_address_from_scroll_offsets(0x3800, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_2_all_offsets)
{
	unsigned fine_y = 4;
	unsigned coarse_x = 11;
	unsigned coarse_y = 5;

	ck_assert_uint_eq(0x48AB, nametable_vram_address_from_scroll_offsets(0x2900, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_2_fine_y_out_of_bounds)
{
	unsigned fine_y = 15; // should be restricted to 7
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x7800, nametable_vram_address_from_scroll_offsets(0x2A15, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_2_coarse_x_out_of_bounds)
{
	// address shouldn't bleed into the adjacent nametable!
	unsigned fine_y = 0;
	unsigned coarse_x = 42; // 10 * 1
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x080A, nametable_vram_address_from_scroll_offsets(0x2A47, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_2_coarse_y_out_of_bounds)
{
	// address shouldn't bleed into the attribute tables!
	unsigned fine_y = 0;
	unsigned coarse_x = 0;
	unsigned coarse_y = 41; // 11 * 0x0020

	ck_assert_uint_eq(0x0960, nametable_vram_address_from_scroll_offsets(0x2841, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_3_no_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x0C00, nametable_vram_address_from_scroll_offsets(0x0C00, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_3_coarse_x_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 13;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x0C0D, nametable_vram_address_from_scroll_offsets(0x2D0D, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_3_coarse_x_y_offset)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 11;
	unsigned coarse_y = 12;

	ck_assert_uint_eq(0x0D8B, nametable_vram_address_from_scroll_offsets(0x2C78, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_3_coarse_x_y_offset_max)
{
	// address before attribute bytes (0x23C0 onwards)
	unsigned fine_y = 0;
	unsigned coarse_x = 31;
	unsigned coarse_y = 29;

	ck_assert_uint_eq(0x0FBF, nametable_vram_address_from_scroll_offsets(0x2C12, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_3_fine_y_only)
{
	unsigned fine_y = 1;
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x1C00, nametable_vram_address_from_scroll_offsets(0x3C11, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_3_all_offsets)
{
	unsigned fine_y = 6;
	unsigned coarse_x = 21;
	unsigned coarse_y = 3;

	ck_assert_uint_eq(0x6C75, nametable_vram_address_from_scroll_offsets(0x2D70, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_3_fine_y_out_of_bounds)
{
	unsigned fine_y = 31; // should be restricted to 7
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x7C00, nametable_vram_address_from_scroll_offsets(0x2D15, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_3_coarse_x_out_of_bounds)
{
	// address shouldn't bleed into the adjacent nametable!
	unsigned fine_y = 0;
	unsigned coarse_x = 51; // 19 * 1
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x0C13, nametable_vram_address_from_scroll_offsets(0x2D47, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (vram_encoder_nametable_3_coarse_y_out_of_bounds)
{
	// address shouldn't bleed into the attribute tables!
	unsigned fine_y = 0;
	unsigned coarse_x = 3;
	unsigned coarse_y = 31; // 2 * 0x0020

	ck_assert_uint_eq(0x0C23, nametable_vram_address_from_scroll_offsets(0x2D41, fine_y
	                                                                    , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_0_no_offset)
{
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x23C0, attribute_address_from_nametable_scroll_offsets(0x2001
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_0_random_offset)
{
	unsigned coarse_x = 20;
	unsigned coarse_y = 7;

	ck_assert_uint_eq(0x23CD, attribute_address_from_nametable_scroll_offsets(0x201F
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_0_coarse_x_out_of_bounds_wraps_around)
{
	unsigned coarse_x = 15 + 32;
	unsigned coarse_y = 24;

	ck_assert_uint_eq(0x23F3, attribute_address_from_nametable_scroll_offsets(0x2200
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_0_coarse_y_out_of_bounds_wraps_around)
{
	unsigned coarse_x = 3;
	unsigned coarse_y = 16 + 30;

	ck_assert_uint_eq(0x23E0, attribute_address_from_nametable_scroll_offsets(0x2201
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_0_tile_sample_within_at_byte)
{
	// indexes are: top left, top right, bottom left and bottom right tiles
	unsigned coarse_x[4] = {1, 3, 0, 2};
	unsigned coarse_y[4] = {0, 1, 2, 3};

	ck_assert_uint_eq(0x23C0, attribute_address_from_nametable_scroll_offsets(0x2001
	                                                                         , coarse_x[_i], coarse_y[_i]));
}

START_TEST (attribute_address_encoder_nametable_1_no_offset)
{
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x27C0, attribute_address_from_nametable_scroll_offsets(0x2400
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_1_random_offset)
{
	unsigned coarse_x = 1;
	unsigned coarse_y = 28;

	ck_assert_uint_eq(0x27F8, attribute_address_from_nametable_scroll_offsets(0x241F
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_1_coarse_x_out_of_bounds_wraps_around)
{
	unsigned coarse_x = 10 + 32;
	unsigned coarse_y = 11;

	ck_assert_uint_eq(0x27D2, attribute_address_from_nametable_scroll_offsets(0x27FF
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_1_coarse_y_out_of_bounds_wraps_around)
{
	unsigned coarse_x = 29;
	unsigned coarse_y = 12 + 30;

	ck_assert_uint_eq(0x27DF, attribute_address_from_nametable_scroll_offsets(0x2601
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_1_tile_sample_within_at_byte)
{
	// indexes are: top left, top right, bottom left and bottom right tiles
	unsigned coarse_x[4] = {16, 19, 17, 18};
	unsigned coarse_y[4] = {5, 4, 6, 7};

	ck_assert_uint_eq(0x27CC, attribute_address_from_nametable_scroll_offsets(0x240D
	                                                                         , coarse_x[_i], coarse_y[_i]));
}

START_TEST (attribute_address_encoder_nametable_2_no_offset)
{
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x2BC0, attribute_address_from_nametable_scroll_offsets(0x1800
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_2_random_offset)
{
	unsigned coarse_x = 13;
	unsigned coarse_y = 13;

	ck_assert_uint_eq(0x2BDB, attribute_address_from_nametable_scroll_offsets(0x291F
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_2_coarse_x_out_of_bounds_wraps_around)
{
	unsigned coarse_x = 5 + 32;
	unsigned coarse_y = 26;

	ck_assert_uint_eq(0x2BF1, attribute_address_from_nametable_scroll_offsets(0x3A00
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_2_coarse_y_out_of_bounds_wraps_around)
{
	unsigned coarse_x = 18;
	unsigned coarse_y = 17 + 30;

	ck_assert_uint_eq(0x2BE4, attribute_address_from_nametable_scroll_offsets(0x2BB1
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_2_tile_sample_within_at_byte)
{
	// indexes are: top left, top right, bottom left and bottom right tiles
	unsigned coarse_x[4] = {24, 27, 25, 26};
	unsigned coarse_y[4] = {9, 9, 10, 11};

	ck_assert_uint_eq(0x2BD6, attribute_address_from_nametable_scroll_offsets(0x2812
	                                                                         , coarse_x[_i], coarse_y[_i]));
}

START_TEST (attribute_address_encoder_nametable_3_no_offset)
{
	unsigned coarse_x = 0;
	unsigned coarse_y = 0;

	ck_assert_uint_eq(0x2FC0, attribute_address_from_nametable_scroll_offsets(0x2C00
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_3_random_offset)
{
	unsigned coarse_x = 8;
	unsigned coarse_y = 29;

	ck_assert_uint_eq(0x2FFA, attribute_address_from_nametable_scroll_offsets(0x2C3F
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_3_coarse_x_out_of_bounds_wraps_around)
{
	unsigned coarse_x = 16 + 32;
	unsigned coarse_y = 6;

	ck_assert_uint_eq(0x2FCC, attribute_address_from_nametable_scroll_offsets(0x2FFF
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_3_coarse_y_out_of_bounds_wraps_around)
{
	unsigned coarse_x = 24;
	unsigned coarse_y = 1 + 30;

	ck_assert_uint_eq(0x2FC6, attribute_address_from_nametable_scroll_offsets(0x2DE1
	                                                                         , coarse_x, coarse_y));
}

START_TEST (attribute_address_encoder_nametable_3_tile_sample_within_at_byte)
{
	// indexes are: top left, top right, bottom left and bottom right tiles
	unsigned coarse_x[4] = {16, 18, 17, 18};
	unsigned coarse_y[4] = {12, 13, 15, 14};

	ck_assert_uint_eq(0x2FDC, attribute_address_from_nametable_scroll_offsets(0x0C0D
	                                                                         , coarse_x[_i], coarse_y[_i]));
}

START_TEST (fetch_attribute_byte_nametable_0_random_scroll_offsets)
{
	unsigned fine_y = 0;
	unsigned coarse_x = 4;
	unsigned coarse_y = 20;
	uint16_t attribute_addr = attribute_address_from_nametable_scroll_offsets(0x2000
	                                                                         , coarse_x
	                                                                         , coarse_y);
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2000, fine_y
	                                                           , coarse_x, coarse_y);
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	write_to_ppu_vram(&ppu->vram, attribute_addr, 0xED);


	fetch_at_byte(ppu);


	ck_assert_uint_eq(0xED, ppu->at_latch);
}

START_TEST (fetch_attribute_byte_nametable_1_random_scroll_offsets)
{
	unsigned fine_y = 4;
	unsigned coarse_x = 30;
	unsigned coarse_y = 9;
	uint16_t attribute_addr = attribute_address_from_nametable_scroll_offsets(0x2400
	                                                                         , coarse_x
	                                                                         , coarse_y);
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2400, fine_y
	                                                           , coarse_x, coarse_y);
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	write_to_ppu_vram(&ppu->vram, attribute_addr, 0x51);


	fetch_at_byte(ppu);


	ck_assert_uint_eq(0x51, ppu->at_latch);
}

START_TEST (fetch_attribute_byte_nametable_2_random_scroll_offsets)
{
	unsigned fine_y = 1;
	unsigned coarse_x = 12;
	unsigned coarse_y = 23;
	uint16_t attribute_addr = attribute_address_from_nametable_scroll_offsets(0x2800
	                                                                         , coarse_x
	                                                                         , coarse_y);
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2800, fine_y
	                                                           , coarse_x, coarse_y);
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	write_to_ppu_vram(&ppu->vram, attribute_addr, 0x08);


	fetch_at_byte(ppu);


	ck_assert_uint_eq(0x08, ppu->at_latch);
}

START_TEST (fetch_attribute_byte_nametable_3_random_scroll_offsets)
{
	unsigned fine_y = 9;
	unsigned coarse_x = 19 + 32;
	unsigned coarse_y = 23;
	uint16_t attribute_addr = attribute_address_from_nametable_scroll_offsets(0x2C00
	                                                                         , coarse_x
	                                                                         , coarse_y);
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2C00, fine_y
	                                                           , coarse_x, coarse_y);
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	write_to_ppu_vram(&ppu->vram, attribute_addr, 0xC3);


	fetch_at_byte(ppu);


	ck_assert_uint_eq(0xC3, ppu->at_latch);
}


Suite* ppu_suite(void)
{
	Suite* s;
	TCase* tc_ppu_vram_read_writes;
	TCase* tc_ppu_rendering;
	TCase* tc_ppu_unit_test_helpers;

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
	tcase_add_loop_test(tc_ppu_rendering, nametable_x_offset_is_valid_for_all_coarse_x, 0, 63);
	tcase_add_loop_test(tc_ppu_rendering, nametable_y_offset_is_valid_for_all_coarse_y, 0, 59);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_0_addr_no_fine_y);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_0_addr_fine_y_is_ignored);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_0_addr_in_attribute_table);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_1_addr_no_fine_y);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_1_addr_fine_y_is_ignored);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_1_addr_in_attribute_table);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_2_addr_no_fine_y);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_2_addr_fine_y_is_ignored);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_2_addr_in_attribute_table);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_3_addr_no_fine_y);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_3_addr_fine_y_is_ignored);
	tcase_add_test(tc_ppu_rendering, fetch_nametable_byte_nametable_3_addr_in_attribute_table);
	tcase_add_test(tc_ppu_rendering, fetch_attribute_byte_nametable_0_random_scroll_offsets);
	tcase_add_test(tc_ppu_rendering, fetch_attribute_byte_nametable_1_random_scroll_offsets);
	tcase_add_test(tc_ppu_rendering, fetch_attribute_byte_nametable_2_random_scroll_offsets);
	tcase_add_test(tc_ppu_rendering, fetch_attribute_byte_nametable_3_random_scroll_offsets);
	suite_add_tcase(s, tc_ppu_rendering);
	tc_ppu_unit_test_helpers = tcase_create("PPU Unit Test Helper Functions");
	tcase_add_checked_fixture(tc_ppu_unit_test_helpers, setup, teardown);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_0_no_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_0_coarse_x_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_0_coarse_x_y_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_0_coarse_x_y_offset_max);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_0_fine_y_only);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_0_all_offsets);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_0_coarse_x_out_of_bounds);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_0_coarse_y_out_of_bounds);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_1_no_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_1_coarse_x_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_1_coarse_x_y_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_1_coarse_x_y_offset_max);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_1_fine_y_only);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_1_all_offsets);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_1_fine_y_out_of_bounds);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_1_coarse_x_out_of_bounds);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_1_coarse_y_out_of_bounds);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_2_no_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_2_coarse_x_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_2_coarse_x_y_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_2_coarse_x_y_offset_max);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_2_fine_y_only);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_2_all_offsets);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_2_fine_y_out_of_bounds);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_2_coarse_x_out_of_bounds);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_2_coarse_y_out_of_bounds);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_3_no_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_3_coarse_x_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_3_coarse_x_y_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_3_coarse_x_y_offset_max);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_3_fine_y_only);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_3_all_offsets);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_3_fine_y_out_of_bounds);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_3_coarse_x_out_of_bounds);
	tcase_add_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_3_coarse_y_out_of_bounds);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_0_no_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_0_random_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_0_coarse_x_out_of_bounds_wraps_around);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_0_coarse_y_out_of_bounds_wraps_around);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_0_tile_sample_within_at_byte, 0, 3);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_1_no_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_1_random_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_1_coarse_x_out_of_bounds_wraps_around);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_1_coarse_y_out_of_bounds_wraps_around);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_1_tile_sample_within_at_byte, 0, 3);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_2_no_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_2_random_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_2_coarse_x_out_of_bounds_wraps_around);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_2_coarse_y_out_of_bounds_wraps_around);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_2_tile_sample_within_at_byte, 0, 3);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_3_no_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_3_random_offset);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_3_coarse_x_out_of_bounds_wraps_around);
	tcase_add_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_3_coarse_y_out_of_bounds_wraps_around);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_3_tile_sample_within_at_byte, 0, 3);
	suite_add_tcase(s, tc_ppu_unit_test_helpers);

	return s;
}
