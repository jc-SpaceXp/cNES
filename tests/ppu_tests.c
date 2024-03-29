#include <check.h>

#include <stdlib.h>

#include "ppu.h"
#include "cpu_ppu_interface.h"
#include "bits_and_bytes.h"

// Disable ASan for specific tests, apply to globals or function declarations
// Only for clang at the moment as clang will flag the out of bounds pixel
// test as a gloal buffer overflow
#if defined(__clang__)
#define ATTRIBUTE_NO_SANITIZE_ADDRESS __attribute__((no_sanitize("address")))
#else
#define ATTRIBUTE_NO_SANITIZE_ADDRESS
#endif

Ppu2C02* ppu;
CpuPpuShare* cpu_ppu;
struct PpuMemoryMap* vram;
uint32_t pixel_buffer[256 * 240];
ATTRIBUTE_NO_SANITIZE_ADDRESS uint32_t pixel_buffer_ignores_asan[256 * 240];

static void setup(void)
{
	cpu_ppu = cpu_ppu_io_allocator();
	ppu = ppu_allocator();
	if (!cpu_ppu | !ppu) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to ppu struct");
	}

	if (ppu_init(ppu, cpu_ppu)) {
		ck_abort_msg("Failed to initialise the ppu struct");
	}

	ppu->vram.pattern_table_0k = malloc(4 * KiB);
	ppu->vram.pattern_table_4k = malloc(4 * KiB);
	if (!ppu->vram.pattern_table_0k || !ppu->vram.pattern_table_4k) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory for pattern tables");
	}
}

static void teardown(void)
{
	free(cpu_ppu);
	free(ppu->vram.pattern_table_0k);
	free(ppu->vram.pattern_table_4k);
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
	vram->pattern_table_0k = malloc(4 * KiB);
	vram->pattern_table_4k = malloc(4 * KiB);
	if (!vram->pattern_table_0k || !vram->pattern_table_4k) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory for pattern tables");
	}

}

static void vram_teardown(void)
{
	teardown();
	free(vram->pattern_table_0k);
	free(vram->pattern_table_4k);
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

static uint8_t reverse_bits_in_byte(uint8_t input)
{
	uint8_t output = 0;

	for (int i = 0; i < 8; i++) {
		// gets bits 0-7 and shift up to bits 7-0
		output |= get_nth_bit(input, i) << (7 - i);
	}

	return output;
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


/* Test helpers unit tests
 */

// Remeber that the ppu's internal vram (and temporary vram) address registers
//   are formatted as follows: 0yyy NNYY YYYX XXXX
//
//   where y is fine y
//         NN is the base nametbale address, 0 ($2000), 1($2400), 2 ($2800), 3 ($2C00)
//         Y is Coarse Y, tile offset in Y direction, incremented every 8 pixels down
//         X is Coarse X, tile offset in X direction, incremented every 8 pixels across
//
// These tests ensure that any given coarse x, coarse y and fine y values are enoded using
// the format above
//
// Out of bounds values will wrap back around to 0 from the max coarse x or y or fine y vals
// such that the offset address shouldn't bleed into the adjacent nametable
START_TEST (vram_encoder_nametable_0)
{
	unsigned fine_y[8] =   {0, 0, 0,  0, 7,  4,  0, 0};
	unsigned coarse_x[8] = {0, 1, 2, 31, 0, 12, 40, 0};
	unsigned coarse_y[8] = {0, 0, 1, 29, 0,  5,  3, 38};
	uint16_t vram_address[8][2] = { {0x2000, 0x0000}
	                              , {0x23FF, 0x0001}
	                              , {0x2008, 0x0022}
	                              , {0x2112, 0x03BF}
	                              , {0x3000, 0x7000}
	                              , {0x0000, 0x40AC}
	                              , {0x239B, 0x0068}
	                              , {0x239B, 0x0100} };

	uint16_t expected_offset = vram_address[_i][1];
	uint16_t encoded_vram_address = nametable_vram_address_from_scroll_offsets(vram_address[_i][0]
	                                                                    , fine_y[_i]
	                                                                    , coarse_x[_i]
	                                                                    , coarse_y[_i]);

	ck_assert_uint_eq(expected_offset, encoded_vram_address);
}

START_TEST (vram_encoder_nametable_1)
{
	unsigned fine_y[9] =   {0, 0, 0,  0, 7,  4, 100,  0, 0};
	unsigned coarse_x[9] = {0, 4, 1, 31, 0, 12,   0, 39, 0};
	unsigned coarse_y[9] = {0, 0, 2, 29, 0,  5,   0,  0, 39};
	uint16_t vram_address[9][2] = { {0x0400, 0x0400}
	                              , {0x24FF, 0x0404}
	                              , {0x2778, 0x0441}
	                              , {0x2512, 0x07BF}
	                              , {0x3400, 0x7400}
	                              , {0x2400, 0x44AC}
	                              , {0x2470, 0x7400}
	                              , {0x2687, 0x0407}
	                              , {0x2439, 0x0520} };

	uint16_t expected_offset = vram_address[_i][1];
	uint16_t encoded_vram_address = nametable_vram_address_from_scroll_offsets(vram_address[_i][0]
	                                                                    , fine_y[_i]
	                                                                    , coarse_x[_i]
	                                                                    , coarse_y[_i]);

	ck_assert_uint_eq(expected_offset, encoded_vram_address);
}

START_TEST (vram_encoder_nametable_2)
{
	unsigned fine_y[9] =   {0,  0,  0,  0, 3,  4, 15,  0, 0};
	unsigned coarse_x[9] = {0, 31,  4, 31, 0, 11,  0, 42, 0};
	unsigned coarse_y[9] = {0,  0, 10, 29, 0,  5,  0,  0, 41};
	uint16_t vram_address[9][2] = { {0x0800, 0x0800}
	                              , {0x281F, 0x081F}
	                              , {0x2878, 0x0944}
	                              , {0x2812, 0x0BBF}
	                              , {0x3800, 0x3800}
	                              , {0x2900, 0x48AB}
	                              , {0x2A15, 0x7800}
	                              , {0x2A47, 0x080A}
	                              , {0x2841, 0x0960} };

	uint16_t expected_offset = vram_address[_i][1];
	uint16_t encoded_vram_address = nametable_vram_address_from_scroll_offsets(vram_address[_i][0]
	                                                                    , fine_y[_i]
	                                                                    , coarse_x[_i]
	                                                                    , coarse_y[_i]);

	ck_assert_uint_eq(expected_offset, encoded_vram_address);
}

START_TEST (vram_encoder_nametable_3)
{
	unsigned fine_y[9] =   {0,  0,  0,  0, 1,  6, 31,  0, 0};
	unsigned coarse_x[9] = {0, 13, 11, 31, 0, 21,  0, 51, 3};
	unsigned coarse_y[9] = {0,  0, 12, 29, 0,  3,  0,  0, 31};
	uint16_t vram_address[9][2] = { {0x0C00, 0x0C00}
	                              , {0x2D0D, 0x0C0D}
	                              , {0x2C78, 0x0D8B}
	                              , {0x2C12, 0x0FBF}
	                              , {0x3C11, 0x1C00}
	                              , {0x2D70, 0x6C75}
	                              , {0x2D15, 0x7C00}
	                              , {0x2D47, 0x0C13}
	                              , {0x2D41, 0x0C23} };

	uint16_t expected_offset = vram_address[_i][1];
	uint16_t encoded_vram_address = nametable_vram_address_from_scroll_offsets(vram_address[_i][0]
	                                                                    , fine_y[_i]
	                                                                    , coarse_x[_i]
	                                                                    , coarse_y[_i]);

	ck_assert_uint_eq(expected_offset, encoded_vram_address);
}


// The encoding of the attribute address is as follows:
//   0010 NN11 11YY YXXX
// Where
//   Coarse X's lowest two bits are shifted out
//   Coarse Y & 0x0C merges into Coarse X (upper two bits from the shift out)
//   Coarse Y's upper bit is shifted down by 4 bits (as are the other Coarse Y bits too)
//   fine y is ignored
//
//   These tests handle the those coarse x, coarse y, fine y and nametable inputs
//   Values exceeding their maximum value should wrap back around (from 0 to their max val)
//   see the + 32 or + 30 values for coarse x or coarse y below
START_TEST (attribute_address_encoder_nametable_0)
{
	unsigned coarse_x[4] = {0, 20, 15 + 32,       3};
	unsigned coarse_y[4] = {0,  7,      24, 16 + 30};
	uint16_t vram_address[4] = {0x2001, 0x201F, 0x2200, 0x2201};

	uint16_t expected_address[4] = {0x23C0, 0x23CD, 0x23F3, 0x23E0};
	uint16_t attribute_address = attribute_address_from_nametable_scroll_offsets(vram_address[_i]
	                                                                     , coarse_x[_i]
	                                                                     , coarse_y[_i]);

	ck_assert_uint_eq(expected_address[_i], attribute_address);
}

START_TEST (attribute_address_encoder_nametable_0_tile_sample_within_at_byte)
{
	// indexes are: top left, top right, bottom left and bottom right tiles
	unsigned coarse_x[4] = {1, 3, 0, 2};
	unsigned coarse_y[4] = {0, 1, 2, 3};

	ck_assert_uint_eq(0x23C0, attribute_address_from_nametable_scroll_offsets(0x2001
	                                                                         , coarse_x[_i], coarse_y[_i]));
}

START_TEST (attribute_address_encoder_nametable_1)
{
	unsigned coarse_x[4] = {0,  1, 10 + 32,       29};
	unsigned coarse_y[4] = {0, 28,      11, 12 + 30};
	uint16_t vram_address[4] = {0x2400, 0x241F, 0x27FF, 0x2601};

	uint16_t expected_address[4] = {0x27C0, 0x27F8, 0x27D2, 0x27DF};
	uint16_t attribute_address = attribute_address_from_nametable_scroll_offsets(vram_address[_i]
	                                                                     , coarse_x[_i]
	                                                                     , coarse_y[_i]);

	ck_assert_uint_eq(expected_address[_i], attribute_address);
}

START_TEST (attribute_address_encoder_nametable_1_tile_sample_within_at_byte)
{
	// indexes are: top left, top right, bottom left and bottom right tiles
	unsigned coarse_x[4] = {16, 19, 17, 18};
	unsigned coarse_y[4] = {5, 4, 6, 7};

	ck_assert_uint_eq(0x27CC, attribute_address_from_nametable_scroll_offsets(0x240D
	                                                                         , coarse_x[_i], coarse_y[_i]));
}

START_TEST (attribute_address_encoder_nametable_2)
{
	unsigned coarse_x[4] = {0, 13, 5 + 32,      18};
	unsigned coarse_y[4] = {0, 13,     26, 17 + 30};
	uint16_t vram_address[4] = {0x1800, 0x291F, 0x3A00, 0x2BB1};

	uint16_t expected_address[4] = {0x2BC0, 0x2BDB, 0x2BF1, 0x2BE4};
	uint16_t attribute_address = attribute_address_from_nametable_scroll_offsets(vram_address[_i]
	                                                                     , coarse_x[_i]
	                                                                     , coarse_y[_i]);

	ck_assert_uint_eq(expected_address[_i], attribute_address);
}

START_TEST (attribute_address_encoder_nametable_2_tile_sample_within_at_byte)
{
	// indexes are: top left, top right, bottom left and bottom right tiles
	unsigned coarse_x[4] = {24, 27, 25, 26};
	unsigned coarse_y[4] = {9, 9, 10, 11};

	ck_assert_uint_eq(0x2BD6, attribute_address_from_nametable_scroll_offsets(0x2812
	                                                                         , coarse_x[_i], coarse_y[_i]));
}

START_TEST (attribute_address_encoder_nametable_3)
{
	unsigned coarse_x[4] = {0,  8, 16 + 32,    24};
	unsigned coarse_y[4] = {0, 29,      6, 1 + 30};
	uint16_t vram_address[4] = {0x2C00, 0x2C3F, 0x2FFF, 0x2DE1};

	uint16_t expected_address[4] = {0x2FC0, 0x2FFA, 0x2FCC, 0x2FC6};
	uint16_t attribute_address = attribute_address_from_nametable_scroll_offsets(vram_address[_i]
	                                                                     , coarse_x[_i]
	                                                                     , coarse_y[_i]);

	ck_assert_uint_eq(expected_address[_i], attribute_address);
}

START_TEST (attribute_address_encoder_nametable_3_tile_sample_within_at_byte)
{
	// indexes are: top left, top right, bottom left and bottom right tiles
	unsigned coarse_x[4] = {16, 18, 17, 18};
	unsigned coarse_y[4] = {12, 13, 15, 14};

	ck_assert_uint_eq(0x2FDC, attribute_address_from_nametable_scroll_offsets(0x0C0D
	                                                                         , coarse_x[_i], coarse_y[_i]));
}


START_TEST (reverse_bits_function_all_valid_inputs)
{
	// re-reversing the reversed bits will produce the original number
	// and also verify the function works as expected
	ck_assert_uint_eq(_i, reverse_bits_in_byte(reverse_bits_in_byte(_i)));
}


/* Vram unit tests
 */
START_TEST (pattern_table_0_writes)
{
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x0000, 0x004B, 0x0FFF};
	uint8_t write_val[3] = {0xD1, 0x72, 0x29};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], vram->pattern_table_0k[vram_address[_i]]);
}

START_TEST (pattern_table_1_writes)
{
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x1000, 0x11BD, 0x1FFF};
	uint8_t write_val[3] = {0xE1, 0x52, 0x3A};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], vram->pattern_table_4k[vram_address[_i] & 0x0FFF]);
}

// Nametable 0 is from 0x2000 to 0x23FF
START_TEST (nametable_0_writes)
{
	vram->nametable_0 = &vram->nametable_A;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x2000, 0x210D, 0x23FF};
	uint8_t write_val[3] = {0x09, 0x13, 0xA5};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], (*vram->nametable_0)[vram_address[_i] & 0x03FF]);
}

// Nametable 1 is from 0x2400 to 0x27FF
START_TEST (nametable_1_writes)
{
	vram->nametable_1 = &vram->nametable_A;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x2400, 0x260D, 0x27FF};
	uint8_t write_val[3] = {0x11, 0x46, 0x91};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], (*vram->nametable_1)[vram_address[_i] & 0x03FF]);
}

// Nametable 2 is from 0x2800 to 0x2BFF
START_TEST (nametable_2_writes)
{
	vram->nametable_2 = &vram->nametable_A;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x2800, 0x29AD, 0x2BFF};
	uint8_t write_val[3] = {0x3F, 0x77, 0xD8};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], (*vram->nametable_2)[vram_address[_i] & 0x03FF]);
}

// Nametable 3 is from 0x2C00 to 0x2FFF
START_TEST (nametable_3_writes)
{
	vram->nametable_3 = &vram->nametable_A;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x2C00, 0x2EEA, 0x2FFF};
	uint8_t write_val[3] = {0x0B, 0x93, 0xFF};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], (*vram->nametable_3)[vram_address[_i] & 0x03FF]);
}

// Nametable 0 is from 0x2000 to 0x23FF
START_TEST (nametable_0_mirror_writes)
{
	vram->nametable_0 = &vram->nametable_B;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x3000, 0x310D, 0x33FF};
	uint8_t write_val[3] = {0x09, 0x13, 0xA5};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], (*vram->nametable_0)[vram_address[_i] & 0x03FF]);
}

// Nametable 1 is from 0x2400 to 0x27FF
START_TEST (nametable_1_mirror_writes)
{
	vram->nametable_1 = &vram->nametable_B;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x3400, 0x360D, 0x37FF};
	uint8_t write_val[3] = {0x11, 0x46, 0x91};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], (*vram->nametable_1)[vram_address[_i] & 0x03FF]);
}

// Nametable 2 is from 0x2800 to 0x2BFF
START_TEST (nametable_2_mirror_writes)
{
	vram->nametable_2 = &vram->nametable_B;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x3800, 0x39AD, 0x3BFF};
	uint8_t write_val[3] = {0x3F, 0x77, 0xD8};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], (*vram->nametable_2)[vram_address[_i] & 0x03FF]);
}

// Nametable 3 is from 0x2C00 to 0x2FFF
START_TEST (nametable_3_partial_mirror_writes)
{
	vram->nametable_3 = &vram->nametable_B;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x3C00, 0x3DDA, 0x3EFF}; // upper limit is $3EFF not $3FFF
	uint8_t write_val[3] = {0x0B, 0x93, 0xFF};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], (*vram->nametable_3)[vram_address[_i] & 0x03FF]);
}

// Palette RAM is from 0x3F00 to 0x3F1F
START_TEST (palette_ram_writes)
{
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x3F00, 0x3F11, 0x3F1F};
	uint8_t write_val[3] = {0xA0, 0xC6, 0xE1};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], vram->palette_ram[vram_address[_i] & 0x001F]);
}

// Palette RAM mirrors are from 0x3F20 to 0x3FFF
START_TEST (palette_ram_mirror_writes)
{
	// Lower, other and upper bounds addresses
	uint16_t vram_address[4] = {0x3F20, 0x3F4A, 0x3F91, 0x3FFF};
	uint8_t write_val[4] = {0x53, 0x66, 0x77, 0xB0};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], vram->palette_ram[vram_address[_i] & 0x001F]);
}

START_TEST (writes_past_upper_bound_have_no_effect)
{
	vram->nametable_0 = &vram->nametable_A;
	vram->nametable_1 = &vram->nametable_A;
	vram->nametable_2 = &vram->nametable_A;
	vram->nametable_3 = &vram->nametable_A;
	write_to_ppu_vram(vram, 0x4FFF, 0xAA);

	ck_assert_uint_ne(0xAA, vram->pattern_table_0k[0x4FFF & 0x0FFF]);
	ck_assert_uint_ne(0xAA, vram->pattern_table_4k[0x4FFF & 0x0FFF]);
	ck_assert_uint_ne(0xAA, (*vram->nametable_0)[0x4FFF & 0x03FF]);
	ck_assert_uint_ne(0xAA, (*vram->nametable_1)[0x4FFF & 0x03FF]);
	ck_assert_uint_ne(0xAA, (*vram->nametable_2)[0x4FFF & 0x03FF]);
	ck_assert_uint_ne(0xAA, (*vram->nametable_3)[0x4FFF & 0x03FF]);
	ck_assert_uint_ne(0xAA, vram->palette_ram[0x4FFF & 0x001F]);
}

START_TEST (pattern_table_0_reads)
{
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x0000, 0x004B, 0x0FFF};
	uint8_t write_val[3] = {0xD1, 0x72, 0x29};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], read_from_ppu_vram(vram, vram_address[_i]));
}

START_TEST (pattern_table_1_reads)
{
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x1000, 0x11BD, 0x1FFF};
	uint8_t write_val[3] = {0xE1, 0x52, 0x3A};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], read_from_ppu_vram(vram, vram_address[_i]));
}

// Nametable 0 is from 0x2000 to 0x23FF
START_TEST (nametable_0_reads)
{
	vram->nametable_0 = &vram->nametable_A;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x2000, 0x210D, 0x23FF};
	uint8_t write_val[3] = {0x09, 0x13, 0x3A};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], read_from_ppu_vram(vram, vram_address[_i]));
}

// Nametable 1 is from 0x2400 to 0x27FF
START_TEST (nametable_1_reads)
{
	vram->nametable_1 = &vram->nametable_A;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x2400, 0x260D, 0x27FF};
	uint8_t write_val[3] = {0x11, 0x46, 0x91};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], read_from_ppu_vram(vram, vram_address[_i]));
}

// Nametable 2 is from 0x2800 to 0x2BFF
START_TEST (nametable_2_reads)
{
	vram->nametable_2 = &vram->nametable_A;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x2800, 0x29AD, 0x2BFF};
	uint8_t write_val[3] = {0x3F, 0x77, 0xD8};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], read_from_ppu_vram(vram, vram_address[_i]));
}

// Nametable 3 is from 0x2C00 to 0x2FFF
START_TEST (nametable_3_reads)
{
	vram->nametable_3 = &vram->nametable_A;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x2C00, 0x2EEA, 0x2FFF};
	uint8_t write_val[3] = {0x0B, 0x93, 0xFF};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], read_from_ppu_vram(vram, vram_address[_i]));
}

// Nametable 0 is from 0x2000 to 0x23FF
START_TEST (nametable_0_mirror_reads)
{
	vram->nametable_0 = &vram->nametable_B;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x3000, 0x310D, 0x33FF};
	uint8_t write_val[3] = {0x09, 0x13, 0xA5};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], read_from_ppu_vram(vram, vram_address[_i]));
}

// Nametable 1 is from 0x2400 to 0x27FF
START_TEST (nametable_1_mirror_reads)
{
	vram->nametable_1 = &vram->nametable_B;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x3400, 0x360D, 0x37FF};
	uint8_t write_val[3] = {0x11, 0x46, 0x91};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], read_from_ppu_vram(vram, vram_address[_i]));
}

// Nametable 2 is from 0x2800 to 0x2BFF
START_TEST (nametable_2_mirror_reads)
{
	vram->nametable_2 = &vram->nametable_B;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x3800, 0x39AD, 0x3BFF};
	uint8_t write_val[3] = {0x3F, 0x77, 0xD8};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], read_from_ppu_vram(vram, vram_address[_i]));
}

// Nametable 3 is from 0x2C00 to 0x2FFF
START_TEST (nametable_3_partial_mirror_reads)
{
	vram->nametable_3 = &vram->nametable_B;
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x3C00, 0x3DDA, 0x3EFF}; // upper limit is $3EFF not $3FFF
	uint8_t write_val[3] = {0x0B, 0x93, 0xFF};

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i], read_from_ppu_vram(vram, vram_address[_i]));
}

// Palette RAM is from 0x3F00 to 0x3F1F
// upper two bits of the upper byte aren't implemented (read back as 0s)
START_TEST (palette_ram_reads_ignores_unused_bits)
{
	// Lower, other and upper bounds addresses
	uint16_t vram_address[3] = {0x3F00, 0x3F11, 0x3F1F};
	uint8_t write_val[3] = {0xA0, 0xC6, 0xE1};
	unsigned palette_data_mask = 0x3F; // upper 2 bits aren't used for palette data

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i] & palette_data_mask
	                 , read_from_ppu_vram(vram, vram_address[_i]));
}

// Palette RAM mirrors are from 0x3F20 to 0x3FFF
// upper two bits of the upper byte aren't implemented (read back as 0s)
START_TEST (palette_ram_mirror_reads_ignores_unused_bits)
{
	// Lower, other and upper bounds addresses
	uint16_t vram_address[4] = {0x3F20, 0x3F4A, 0x3F91, 0x3FFF};
	uint8_t write_val[4] = {0x53, 0x66, 0x77, 0xB0};
	unsigned palette_data_mask = 0x3F; // upper 2 bits aren't used for palette data

	write_to_ppu_vram(vram, vram_address[_i], write_val[_i]);

	ck_assert_uint_eq(write_val[_i] & palette_data_mask
	                 , read_from_ppu_vram(vram, vram_address[_i]));
}


/* Ppu rendering unit tests
 */
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
	memset(vram->nametable_A, 0x00, sizeof(vram->nametable_A));
	memset(vram->nametable_B, 0x00, sizeof(vram->nametable_B));

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
	memset(vram->nametable_A, 0x00, sizeof(vram->nametable_A));
	memset(vram->nametable_B, 0x00, sizeof(vram->nametable_B));

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
	memset(vram->nametable_B, 0x00, sizeof(vram->nametable_B));

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
	memset(vram->nametable_A, 0x00, sizeof(vram->nametable_A));

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

START_TEST (fetch_nametable_byte_nametable_0_addr_ignore_fine_y)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_B;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_B;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	unsigned fine_y[2] = {0, 3};
	unsigned coarse_x = 1;
	unsigned coarse_y = 17;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2000, fine_y[_i]
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0x21);

	fetch_nt_byte(&ppu->vram, ppu->vram_addr, &ppu->bkg_internals);

	ck_assert_uint_eq(ppu->bkg_internals.nt_byte, 0x21);
}

START_TEST (fetch_nametable_byte_nametable_1_addr_ignore_fine_y)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_A;
	ppu->vram.nametable_2 = &ppu->vram.nametable_B;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	ppu->vram_addr = 0x06B1;
	unsigned fine_y[2] = {0, 1};
	unsigned coarse_x = 1;
	unsigned coarse_y = 19;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2400, fine_y[_i]
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0xB1);

	fetch_nt_byte(&ppu->vram, ppu->vram_addr, &ppu->bkg_internals);

	ck_assert_uint_eq(ppu->bkg_internals.nt_byte, 0xB1);
}

START_TEST (fetch_nametable_byte_nametable_2_addr_ignore_fine_y)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_A;
	ppu->vram.nametable_1 = &ppu->vram.nametable_A;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_A;
	unsigned fine_y[2] = {0, 2};
	unsigned coarse_x = 1;
	unsigned coarse_y = 0;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2800, fine_y[_i]
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0x80);

	fetch_nt_byte(&ppu->vram, ppu->vram_addr, &ppu->bkg_internals);

	ck_assert_uint_eq(ppu->bkg_internals.nt_byte, 0x80);
}

START_TEST (fetch_nametable_byte_nametable_3_addr_ignore_fine_y)
{
	ppu->vram.nametable_0 = &ppu->vram.nametable_B;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_A;
	ppu->vram.nametable_3 = &ppu->vram.nametable_A;
	unsigned fine_y[2] = {0, 7};
	unsigned coarse_x = 13;
	unsigned coarse_y = 23;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2C00, fine_y[_i]
	                                                           , coarse_x, coarse_y);
	write_to_ppu_vram(&ppu->vram, 0x2000 | (ppu->vram_addr & 0x0FFF), 0xED);

	fetch_nt_byte(&ppu->vram, ppu->vram_addr, &ppu->bkg_internals);

	ck_assert_uint_eq(ppu->bkg_internals.nt_byte, 0xED);
}

START_TEST (fetch_nametable_byte_in_attribute_table)
{
	// Possible that the current vram address is in the attribute table
	// Still must read the attribute table byte as if it was in the nametable section
	ppu->vram.nametable_0 = &ppu->vram.nametable_B;
	ppu->vram.nametable_1 = &ppu->vram.nametable_B;
	ppu->vram.nametable_2 = &ppu->vram.nametable_B;
	ppu->vram.nametable_3 = &ppu->vram.nametable_B;
	unsigned coarse_x = 16;
	unsigned coarse_y = 12;
	uint16_t nametable_address[4] = {0x2000, 0x2400, 0x2800, 0x2C00};
	ppu->vram_addr = attribute_address_from_nametable_scroll_offsets(nametable_address[_i]
	                                                                , coarse_x
	                                                                , coarse_y);
	uint8_t write_vals[4] = {0xAC, 0xAF, 0xA1, 0xAD};
	write_to_ppu_vram(&ppu->vram, ppu->vram_addr, write_vals[_i]);

	fetch_nt_byte(&ppu->vram, ppu->vram_addr, &ppu->bkg_internals);

	ck_assert_uint_eq(ppu->bkg_internals.nt_byte, write_vals[_i]);
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


	fetch_at_byte(&ppu->vram, ppu->vram_addr, &ppu->bkg_internals);


	ck_assert_uint_eq(0xED, ppu->bkg_internals.at_latch);
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


	fetch_at_byte(&ppu->vram, ppu->vram_addr, &ppu->bkg_internals);


	ck_assert_uint_eq(0x51, ppu->bkg_internals.at_latch);
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


	fetch_at_byte(&ppu->vram, ppu->vram_addr, &ppu->bkg_internals);


	ck_assert_uint_eq(0x08, ppu->bkg_internals.at_latch);
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


	fetch_at_byte(&ppu->vram, ppu->vram_addr, &ppu->bkg_internals);


	ck_assert_uint_eq(0xC3, ppu->bkg_internals.at_latch);
}


START_TEST (fetch_pattern_table_lo_for_fine_y_offsets)
{
	ppu->cpu_ppu_io->ppu_ctrl = _i << 4; // address is 0x0000 or 0x1000
	ppu->bkg_internals.nt_byte = 0x41;
	uint8_t pt_byte = 0x37;
	unsigned fine_y[2] = {0, 5}; // fine y influences the pt address e.g. 0x0000 to 0x0007
	unsigned coarse_x[2] = {1, 14};
	unsigned coarse_y[2] = {21, 9};
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2C00, fine_y[_i]
	                                                           , coarse_x[_i], coarse_y[_i]);
	uint16_t pattern_table_address = (ppu->bkg_internals.nt_byte << 4) + fine_y[_i];
	write_to_ppu_vram(&ppu->vram, ppu_base_pt_address(ppu->cpu_ppu_io)
	                              | pattern_table_address
	                              , pt_byte);

	// fine y is taken from vram_address
	fetch_pt_lo(&ppu->vram, ppu->vram_addr
	           , ppu_base_pt_address(ppu->cpu_ppu_io), &ppu->bkg_internals);

	ck_assert_uint_eq(reverse_bits_in_byte(pt_byte), ppu->bkg_internals.pt_lo_latch);
}

START_TEST (fetch_pattern_table_hi_for_fine_y_offsets)
{
	ppu->cpu_ppu_io->ppu_ctrl = _i << 4; // address is 0x0000 or 0x1000
	ppu->bkg_internals.nt_byte = 0x20;
	uint8_t pt_byte = 0x19;
	unsigned fine_y[2] = {0, 7}; // fine y influences the pt address e.g. 0x0000 to 0x0007
	unsigned coarse_x[2] = {19, 4};
	unsigned coarse_y[2] = {9, 23};
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2C00, fine_y[_i]
	                                                           , coarse_x[_i], coarse_y[_i]);
	uint16_t pattern_table_address = (ppu->bkg_internals.nt_byte << 4) + fine_y[_i] + 8;
	write_to_ppu_vram(&ppu->vram, ppu_base_pt_address(ppu->cpu_ppu_io)
	                              | pattern_table_address
	                              , pt_byte);

	// fine y is taken from vram_address
	fetch_pt_hi(&ppu->vram, ppu->vram_addr
	           , ppu_base_pt_address(ppu->cpu_ppu_io), &ppu->bkg_internals);

	ck_assert_uint_eq(reverse_bits_in_byte(pt_byte), ppu->bkg_internals.pt_hi_latch);
}

START_TEST (attribute_shift_reg_from_bottom_right_quadrant)
{
	// Bottom right is bits 7-6
	// bottom set by one in 2^1 bit in coarse_y
	// right set by one in 2^1 bit in coarse_x
	uint8_t attribute_byte = 0x8F;
	unsigned fine_y = 7;
	unsigned coarse_x = 6;
	unsigned coarse_y = 14;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2008, fine_y
	                                                           , coarse_x, coarse_y);

	fill_attribute_shift_reg(ppu->vram_addr, attribute_byte, &ppu->bkg_internals);

	ck_assert_uint_eq(0xFF, ppu->bkg_internals.at_hi_shift_reg);
	ck_assert_uint_eq(0x00, ppu->bkg_internals.at_lo_shift_reg);
}

START_TEST (attribute_shift_reg_from_top_right_quadrant)
{
	// Top right is bits 3-2
	// top set by zero in 2^1 bit in coarse_y
	// right set by one in 2^1 bit in coarse_x
	uint8_t attribute_byte = 0xFB;
	unsigned fine_y = 0;
	unsigned coarse_x = 3;
	unsigned coarse_y = 21;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2400, fine_y
	                                                           , coarse_x, coarse_y);

	fill_attribute_shift_reg(ppu->vram_addr, attribute_byte, &ppu->bkg_internals);

	ck_assert_uint_eq(0xFF, ppu->bkg_internals.at_hi_shift_reg);
	ck_assert_uint_eq(0x00, ppu->bkg_internals.at_lo_shift_reg);
}

START_TEST (attribute_shift_reg_from_bottom_left_quadrant)
{
	// Bottom left is bits 5-4
	// bottom set by one in 2^1 bit in coarse_y
	// left set by zero in 2^1 bit in coarse_x
	uint8_t attribute_byte = 0xCF;
	unsigned fine_y = 0;
	unsigned coarse_x = 1;
	unsigned coarse_y = 2;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2800, fine_y
	                                                           , coarse_x, coarse_y);

	fill_attribute_shift_reg(ppu->vram_addr, attribute_byte, &ppu->bkg_internals);

	ck_assert_uint_eq(0x00, ppu->bkg_internals.at_hi_shift_reg);
	ck_assert_uint_eq(0x00, ppu->bkg_internals.at_lo_shift_reg);
}

START_TEST (attribute_shift_reg_from_top_left_quadrant)
{
	// Top left is bits 1-0
	// top set by zero in 2^1 bit in coarse_y
	// left set by zero in 2^1 bit in coarse_x
	uint8_t attribute_byte = 0xFD;
	unsigned fine_y = 2;
	unsigned coarse_x = 17;
	unsigned coarse_y = 17;
	ppu->vram_addr = nametable_vram_address_from_scroll_offsets(0x2C00, fine_y
	                                                           , coarse_x, coarse_y);

	fill_attribute_shift_reg(ppu->vram_addr, attribute_byte, &ppu->bkg_internals);

	ck_assert_uint_eq(0x00, ppu->bkg_internals.at_hi_shift_reg);
	ck_assert_uint_eq(0xFF, ppu->bkg_internals.at_lo_shift_reg);
}

START_TEST (pixel_buffer_set_corner_pixels)
{
	const unsigned int max_width = 256;
	unsigned int x_pos[4] = {0, 255,  0, 255}; // top left, top right, bottom left, bottom right
	unsigned int y_pos[4] = {0, 0,  239, 239};
	unsigned pixel_index = x_pos[_i] + (256 * y_pos[_i]);
	uint32_t rgb[4] = {0x0035313A, 0x00FFC0CB, 0x0032CD32, 0x006A5ACD};
	uint8_t alpha = 0xFF;
	rgb[_i] |= (uint32_t) alpha << 24;

	set_rgba_pixel_in_buffer(&pixel_buffer[0], max_width, x_pos[_i], y_pos[_i], rgb[_i], alpha);

	ck_assert_uint_eq(rgb[_i], pixel_buffer[pixel_index]);
}

// Suppress any errors of buffer overflow from ASan
// ASan will catch these errors during runtime (during actual emulation)
START_TEST (pixel_buffer_set_out_of_bounds_allowed)
{
	// Setting/accessing an array with an out of bounds subscript
	// is undefined behaviour and allowed in the function as no bound
	// checking is done
	const unsigned int max_width = 256;
	unsigned int x_pos = max_width;
	unsigned int y_pos = 242;
	uint32_t rgb = 0x00FFFF00;
	uint8_t alpha = 0xFF;
	rgb |= (uint32_t) alpha << 24;

	set_rgba_pixel_in_buffer(&pixel_buffer_ignores_asan[0], max_width, x_pos, y_pos, rgb, alpha);

	ck_assert_uint_eq(1, 1); // don't rely on undefined behaviour for result
}


START_TEST (bkg_render_left_masking_unmasking)
{
	// A bit in 0x02 will show the leftmost 8 background pixels, no bit = output the common background 0x3F00
	uint8_t mask_to_output[9][2] = { {0x00, 24}
	                               , {0x00, 24}
	                               , {0x00, 24}
	                               , {0x00, 24}
	                               , {0x02, 5}
	                               , {0x00, 24}
	                               , {0x00, 24}
	                               , {0x00, 24}
	                               , {0x00, 5}  // 9th pixel is outputted regardless of masking (if bkg rendering enabled)
	};
	ppu->cycle = _i;
	ppu->cpu_ppu_io->ppu_mask = mask_to_output[_i][0] | 0x08; // allow rendering of background
	ppu->fine_x = 0;
	ppu->bkg_internals.at_hi_shift_reg = 0x00;
	ppu->bkg_internals.at_lo_shift_reg = 0x00; // 0x00 (hi/lo)
	ppu->bkg_internals.pt_hi_shift_reg = 0xFF;
	ppu->bkg_internals.pt_lo_shift_reg = 0x00; // 0x10 (hi/lo)
	write_to_ppu_vram(&ppu->vram, 0x3F00, 24); // background colour
	write_to_ppu_vram(&ppu->vram, 0x3F02, 5); // non-background colour via pt

	uint8_t colour_reference = 0x00;
	get_bkg_pixel(ppu, &colour_reference);

	ck_assert_uint_eq(colour_reference, mask_to_output[_i][1]);
}

START_TEST (bkg_render_enabled_or_disabled)
{
	// A bit in 0x08 will enable background rendering, no bit == disabled (output common background colour)
	// A bit in 0x02 will show the leftmost 8 background pixels, no bit = output the common background 0x3F00
	uint8_t mask_to_output[6][2] = { {0x02, 11} // disabled
	                               , {0x0A, 3}  // enabled (and not hidden), cycle < 8
	                               , {0x08, 3}  // enabled, cycle > 8
	                               , {0x00, 11} // disabled
	                               , {0x08, 3}  // enabled, cycle > 8
	                               , {0x0A, 3}  // enabled, cycle > 8
	};
	ppu->cycle = _i * 7;
	ppu->cpu_ppu_io->ppu_mask = mask_to_output[_i][0];
	ppu->fine_x = 0;
	ppu->bkg_internals.at_hi_shift_reg = 0x00;
	ppu->bkg_internals.at_lo_shift_reg = 0x00; // 0x00 (hi/lo)
	ppu->bkg_internals.pt_hi_shift_reg = 0xFF;
	ppu->bkg_internals.pt_lo_shift_reg = 0xFF; // 0x11 (hi/lo)
	write_to_ppu_vram(&ppu->vram, 0x3F00, 11); // background colour
	write_to_ppu_vram(&ppu->vram, 0x3F03, 3); // non-background colour via pt

	uint8_t colour_reference = 0x00;
	get_bkg_pixel(ppu, &colour_reference);

	ck_assert_uint_eq(colour_reference, mask_to_output[_i][1]);
}

START_TEST (bkg_palette_address_non_zero_offsets_no_fine_x)
{
	// bytes: at_hi, at_lo, pt_hi, pt_lo, palette_offset from $3F00
	// non-zero offsets, as 0 offsets will render $3F00
	uint8_t attribute_pattern_offsets[12][5] = { {0, 0, 0, 1, 1}
	                                           , {0, 0, 1, 0, 2}
	                                           , {0, 0, 1, 1, 3}
	                                           , {0, 1, 0, 1, 5}
	                                           , {0, 1, 1, 0, 6}
	                                           , {0, 1, 1, 1, 7}
	                                           , {1, 0, 0, 1, 9}
	                                           , {1, 0, 1, 0, 10}
	                                           , {1, 0, 1, 1, 11}
	                                           , {1, 1, 0, 1, 13}
	                                           , {1, 1, 1, 0, 14}
	                                           , {1, 1, 1, 1, 15}
	};
	uint8_t output = 15;
	ppu->cycle = 300;
	// A bit in 0x08 will enable background rendering, no bit == disabled (output common background colour)
	ppu->cpu_ppu_io->ppu_mask = 0x08;
	ppu->bkg_internals.at_hi_shift_reg = attribute_pattern_offsets[_i][0];
	ppu->bkg_internals.at_lo_shift_reg = attribute_pattern_offsets[_i][1];
	ppu->bkg_internals.pt_hi_shift_reg = attribute_pattern_offsets[_i][2];
	ppu->bkg_internals.pt_lo_shift_reg = attribute_pattern_offsets[_i][3];
	write_to_ppu_vram(&ppu->vram, 0x3F00, 2); // background colour
	write_to_ppu_vram(&ppu->vram, 0x3F00 + attribute_pattern_offsets[_i][4], output); // non-background colour via pt an at (at * 2 plus pt)

	uint8_t colour_reference = 0x00;
	get_bkg_pixel(ppu, &colour_reference);

	ck_assert_uint_eq(colour_reference, output);
}

START_TEST (bkg_palette_address_offsets_with_fine_x)
{
	// bytes: pt_hi, pt_lo, palette_offset from $3F00
	uint8_t attribute_pattern_offsets[8][3] = { {0, 1, 1}
	                                          , {1, 0, 2}
	                                          , {1, 1, 3}
	                                          , {0, 1, 1}
	                                          , {1, 0, 2}
	                                          , {1, 1, 3}
	                                          , {0, 1, 1}
	                                          , {1, 0, 2}
	};
	uint8_t output = 3;
	ppu->cycle = 280;
	ppu->fine_x = _i;
	// A bit in 0x08 will enable background rendering, no bit == disabled (output common background colour)
	ppu->cpu_ppu_io->ppu_mask = 0x08;
	ppu->bkg_internals.at_hi_shift_reg = 0;
	ppu->bkg_internals.at_lo_shift_reg = 0;
	// pt bits will be placed into the fine_x bit position, all other bits are 0's
	ppu->bkg_internals.pt_hi_shift_reg = attribute_pattern_offsets[_i][0] << _i;
	ppu->bkg_internals.pt_lo_shift_reg = attribute_pattern_offsets[_i][1] << _i;
	write_to_ppu_vram(&ppu->vram, 0x3F00, 2); // background colour
	write_to_ppu_vram(&ppu->vram, 0x3F00 + attribute_pattern_offsets[_i][2], output); // non-background colour via pt an at (at * 2 plus pt)

	uint8_t colour_reference = 0x00;
	get_bkg_pixel(ppu, &colour_reference);

	ck_assert_uint_eq(colour_reference, output);
}

START_TEST (bkg_output_transparent_pixel)
{
	// bytes: at_hi, at_lo, pt_hi, pt_lo, palette_offset from $3F00 (ignore for pt_lo && pt_hi == 0)
	uint8_t attribute_pattern_offsets[6][5] = { {0, 0, 0, 0, 1}
	                                          , {0, 0, 1, 0, 2}
	                                          , {0, 1, 0, 0, 6}
	                                          , {1, 0, 0, 0, 9}
	                                          , {1, 0, 1, 1, 11}
	                                          , {1, 1, 0, 0, 13}
	};
	uint8_t output = 11;
	ppu->cycle = 45;
	// A bit in 0x08 will enable background rendering, no bit == disabled (output common background colour)
	ppu->cpu_ppu_io->ppu_mask = 0x08;
	ppu->fine_x = _i; // fine_x should always select 0 bits for pt_hi and pt_lo
	ppu->bkg_internals.pt_hi_shift_reg = attribute_pattern_offsets[_i][0];
	ppu->bkg_internals.pt_lo_shift_reg = attribute_pattern_offsets[_i][1];
	ppu->bkg_internals.pt_hi_shift_reg = attribute_pattern_offsets[_i][2];
	ppu->bkg_internals.pt_lo_shift_reg = attribute_pattern_offsets[_i][3];
	write_to_ppu_vram(&ppu->vram, 0x3F00, output); // background colour
	write_to_ppu_vram(&ppu->vram, 0x3F08, 4); // shouldn't be read back (3F00 should be read instead)
	write_to_ppu_vram(&ppu->vram, 0x3F00 + attribute_pattern_offsets[_i][4], 15); // non-background colour via pt and at (at * 2 plus pt), missed due to fine_x val

	uint8_t colour_reference = 0x00;
	get_bkg_pixel(ppu, &colour_reference);

	ck_assert_uint_eq(colour_reference, output);
}


START_TEST (debug_all_nametables)
{
	uint16_t nametable_addr = 0x2000;
	uint16_t base_nt_addr = 0x2000; // start from NT 0
	for (int coarse_y = 0; coarse_y < 60; coarse_y++) {
		if (coarse_y == 30) {
			base_nt_addr = 0x2800; // start from NT 2 (bottom left)
		}
		nametable_addr = base_nt_addr + ((coarse_y % 30) << 5);
		ck_assert_uint_eq(nametable_addr, base_nt_addr + ((coarse_y % 30) << 5));
		for (int coarse_x = 0; coarse_x < 64; coarse_x++) {
			if (coarse_x == 32) {
				base_nt_addr |= 0x0400; // add 0x400 from NT0 or NT2
				nametable_addr = base_nt_addr + ((coarse_y % 30) << 5);
			}
			ck_assert_uint_eq(nametable_addr, base_nt_addr + (coarse_x & 31) + ((coarse_y % 30) << 5));
			// valid x increments/addresses are say 0x2000 to 0x201F
			// so values past 31 need to be masked out
			// valid y vals are 0-29 so use remainder to calculate the y offset
			nametable_addr++;
		}
		base_nt_addr &= ~0x0400; // reset back to leftmost NT (after coarse_x resets to 0)
	}
}


START_TEST (clear_scanline_oam)
{
	ppu->scanline_oam[4] = 0x83;
	uint8_t expected_result[8 * 4];
	// Clearing scanline/secondary oam should hold all 0xFF's
	memset(expected_result, 0xFF, sizeof(expected_result));

	reset_secondary_oam(ppu);

	ck_assert_mem_eq(ppu->scanline_oam, expected_result, sizeof(expected_result));
}


START_TEST (sprite_eval_odd_cycles_read_only)
{
	ppu->stop_early = false;
	ppu->sprite_index = 0;
	ppu->sprites_found = 7;
	ppu->cpu_ppu_io->ppu_ctrl = 0; // sprite height is 8 pixels
	ppu->scanline = 35;
	// set y pos of all sprites (and other bytes) to 230 (out of Y range)
	memset(ppu->oam, 230, sizeof(ppu->oam));
	uint8_t expected_result[8 * 4];
	memset(expected_result, 0xB1, sizeof(expected_result));
	memset(ppu->scanline_oam, 0xB1, sizeof(ppu->scanline_oam));

	// Loop through until cycle 256 has been reached (end of sprite eval)
	for (int i = 0; i < 192; i++) {
		bool odd_cycle = !(i & 0x01);
		if (odd_cycle) {
			// starting from an odd cycle, odd + even = odd
			ppu->cycle = 65 + i; // sprite evaluation starts at cycle 65
			sprite_evaluation(ppu);
		}

		// Only even cycles change sprite index
		ck_assert_uint_eq(ppu->sprite_index, 0);
		ck_assert_uint_eq(ppu->oam_read_buffer, 230);
		// No oam data should have been transfered
		ck_assert_mem_eq(ppu->scanline_oam, expected_result, sizeof(expected_result));
	}
}

START_TEST (sprite_eval_in_range_oam_transfer)
{
	ppu->stop_early = false;
	ppu->sprite_index = 5;
	ppu->sprites_found = 2;
	uint8_t ppu_ctrl_byte[2] = { 0x00, 0x20 };  // 8 and 16 pixel high sprites
	unsigned y_pos_start[2] = { 30, 20 };  // y pos starts for 8 and 16 pixel high sprites
	ppu->cpu_ppu_io->ppu_ctrl = ppu_ctrl_byte[_i];
	ppu->scanline = 35;
	uint8_t expected_result[8 * 4];
	memset(expected_result, 0xC2, sizeof(expected_result));
	memset(ppu->scanline_oam, 0xC2, sizeof(ppu->scanline_oam));
	// set y pos of all sprites (and other bytes) to be in Y range
	// in range if y pos (top most sprite pixel) + sprite height overlaps
	// with the current scanline
	// Also write unique oam values and check we copy those bytes in the correct order
	for (int i = 0; i < 4; i++) {
		ppu->oam[(ppu->sprite_index * 4) + i] = y_pos_start[_i] + i;
		expected_result[(ppu->sprites_found * 4) + i] = y_pos_start[_i] + i;
	}

	// Loop enough for one sprite evaluation
	for (int i = 0; i < 2; i++) {
		ppu->cycle = 65 + i; // sprite evaluation starts at cycle 65

		sprite_evaluation(ppu);
	}

	ck_assert_mem_eq(ppu->scanline_oam, expected_result, sizeof(expected_result));
	ck_assert_uint_eq(ppu->sprites_found, 3);
}

START_TEST (sprite_eval_none_in_range)
{
	ppu->stop_early = false;
	ppu->sprite_index = 0;
	ppu->sprites_found = 7;
	uint8_t ppu_ctrl_byte[2] = { 0x00, 0x20 };  // 8 and 16 pixel high sprites
	ppu->cpu_ppu_io->ppu_ctrl = ppu_ctrl_byte[_i];
	ppu->scanline = 35;
	// set y pos of all sprites (and other bytes) to 230 (out of range)
	memset(ppu->oam, 230, sizeof(ppu->oam));
	uint8_t expected_result[8 * 4];
	memset(expected_result, 0xE3, sizeof(expected_result));
	memset(ppu->scanline_oam, 0xE3, sizeof(ppu->scanline_oam));

	// Loop through until all 64 sprites have been evaluated
	for (int i = 0; i < 128; i++) {
		ppu->cycle = 65 + i; // sprite evaluation starts at cycle 65

		sprite_evaluation(ppu);
	}

	// No need to check intermediate reads and writes
	ck_assert_uint_eq(ppu->sprite_index, 0);
	ck_assert_uint_eq(ppu->sprites_found, 7);
	ck_assert(ppu->stop_early == true);
	// No oam data should have been transfered
	ck_assert_mem_eq(ppu->scanline_oam, expected_result, sizeof(expected_result));
}

START_TEST (sprite_eval_transfer_oam_on_even_cycles_only)
{
	// transfers only happen if sprite is in Y range
	ppu->stop_early = false;
	ppu->sprite_index = 0;
	ppu->sprites_found = 0;
	ppu->cpu_ppu_io->ppu_ctrl = 0; // sprite height is 8 pixels
	ppu->scanline = 35;
	ppu->oam_read_buffer = 32; // even cycles reads from here instead of oam
	// set y pos of all sprites (and other bytes) to be in Y range
	memset(ppu->oam, 21, sizeof(ppu->oam));
	uint8_t expected_result[8 * 4];
	memset(expected_result, 21, sizeof(expected_result));
	memset(ppu->scanline_oam, 0xF0, sizeof(ppu->scanline_oam));

	// Loop through until cycle 256 has been reached (end of sprite eval)
	for (int i = 0; i < 192; i++) {
		bool even_cycle = i & 0x01;
		if (even_cycle) {
			// starting from an odd cycle, odd + odd = even
			ppu->cycle = 65 + i; // sprite evaluation starts at cycle 65
			sprite_evaluation(ppu);

			// Only even cycles change sprite index
			int initial_index = (i & 0x7F) / 2; // keep value between 0 and 63
			// when we've processed all sprites reset the initial index (0 to 63 = 64 sprites)
			if (initial_index == 63) { initial_index = -1; }

			ck_assert_uint_eq(ppu->sprite_index, initial_index + 1);
			ck_assert_uint_eq(ppu->oam_read_buffer, 32);
		}
	}
	// Final oam transfer
	ck_assert_mem_eq(ppu->scanline_oam, expected_result, sizeof(expected_result));
}

START_TEST (sprite_eval_sprites_found_behaviour)
{
	ppu->stop_early = false;
	ppu->sprite_index = 0;
	ppu->sprites_found = 0;
	ppu->cpu_ppu_io->ppu_ctrl = 0; // sprite height is 8 pixels
	ppu->scanline = 35;
	memset(ppu->oam, 31, sizeof(ppu->oam));
	memset(ppu->oam + 8 * 4, 34, 8); // set 9th and 10th sprites in range to a different value
	uint8_t expected_result[8 * 4];
	memset(expected_result, 31, sizeof(expected_result));
	memset(ppu->scanline_oam, 0xC2, sizeof(ppu->scanline_oam));
	int loop_index = _i * 2; // 2 cycles needed for each sprite evaluation
	// _i starts @ 1 (1st index is 2)
	unsigned sprites_evaluated[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 9 };

	// Loop through until we evlauate _i sprites
	for (int i = 0; i < loop_index; i++) {
		ppu->cycle = 65 + i; // sprite evaluation starts at cycle 65

		sprite_evaluation(ppu);
	}

	unsigned bytes_written_to = (_i * 4) & 0x1F; // 4 bytes per oam entry (and keep within 0-31)
	// values are only 0-9, 8 sprites on scanline, 9th for sprite overflow
	// after that no more sprites are evaluated
	ck_assert_uint_eq(ppu->sprites_found, sprites_evaluated[_i - 1]);
	// This also checks if the 9th sprite found transfers its oam data (since its different)
	ck_assert_mem_eq(ppu->scanline_oam, expected_result, bytes_written_to);
}

START_TEST (sprite_eval_sprite_overflow_behaviour)
{
	ppu->stop_early = false;
	ppu->sprite_index = 23;
	ppu->sprites_found = 7;
	ppu->cpu_ppu_io->ppu_ctrl = 0; // sprite height is 8 pixels
	ppu->cpu_ppu_io->ppu_status = 0;
	ppu->scanline = 35;

	const int sprites_to_check = 6;
	// expected ppu status on each even cycle, a 2d square array
	// as we need to verify which sprite caused the sprite overflow (if any)
	// 1st col is the 1st sprite after the 8th in range, 2nd is the 2nd after etc
	// a 0x20 indicates that the Xth sprite after the 8th is in range
	uint8_t expected_ppu_status[6][6] =
	     { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00} // overflow on 8th sprite (shouldn't happen)
	     , {0x00, 0x20, 0x20, 0x20, 0x20, 0x20} // overflow on 8th + 1 sprite after
	     , {0x00, 0x00, 0x20, 0x20, 0x20, 0x20} // overflow on 8th + 2 sprites after
	     , {0x00, 0x00, 0x00, 0x20, 0x20, 0x20} // overflow on 8th + 3 sprites after
	     , {0x00, 0x00, 0x00, 0x00, 0x20, 0x20} // overflow on 8th + 4 sprites after
	     , {0x00, 0x00, 0x00, 0x00, 0x00, 0x20} // overflow on 8th + 5 sprites after
	     };
	memset(ppu->oam, 0, sizeof(ppu->oam));
	ppu->oam[ppu->sprite_index * 4] = 31; // always make the next sprite (8th) in Y range
	// offset the 9th sprite in Y range depending on the unit test loop index
	unsigned oam_byte_offset[6] = { 0, 0, 1, 2, 3, 0};
	ppu->oam[((ppu->sprite_index + _i - 1) * 4) + oam_byte_offset[_i - 1]] = 31;

	// Evaluate the 8th and potentially 9th sprites in Y range
	for (int i = 0; i < (sprites_to_check * 2); i++) {
		ppu->cycle = 65 + i; // sprite evaluation starts at cycle 65

		sprite_evaluation(ppu);

		bool even_cycle = i & 0x01; // start on odd cycle, odd + odd = even cycle
		if (even_cycle) {
			unsigned sprite_to_eval = i / 2;
			unsigned ppu_status_results = _i - 1;
			ck_assert_uint_eq(ppu->cpu_ppu_io->ppu_status
			                 , expected_ppu_status[ppu_status_results][sprite_to_eval]);
		}
	}
}


START_TEST (sprite_fetch_address_8_pixel_sprites)
{
	// 8x8 sprites go from 0x0XX0 to 0x0XX7 (lo) and 0x0XX8 to 0x0XXF (hi)
	// where XX is the tile number (from oam byte 1)
	ppu->cpu_ppu_io->ppu_ctrl = 0; // 8x8 sprites
	// 0x0000 is 0x00 and 0x1000 is 0x08
	uint16_t pattern_table_start[8] = {0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x08, 0x08};
	ppu->cpu_ppu_io->ppu_ctrl |= pattern_table_start[_i];
	ppu->scanline = 131 + _i;
	int y_offset = 0;
	uint8_t tile_number = _i;
	ppu->scanline_oam[_i * 4] = 131; // offset is scanline - y_byte
	ppu->scanline_oam[(_i * 4) + 1] = tile_number;
	// pattern_table start << 9 is 0x0000 or 0x1000
	uint16_t res = (pattern_table_start[_i] << 9) | (tile_number << 4);
	res |= _i; // add sprite Y offset (initial offset was 0)

	get_sprite_address(ppu, &y_offset, _i);

	ck_assert_uint_eq(ppu->sprite_addr, res);
}

START_TEST (sprite_fetch_address_16_pixel_sprites)
{
	// 8x16 sprites go from 0xXXX0 to 0xXX1F (lo and hi bytes)
	// lo: 0xXXX0 to 0xXXX7 and 0xXX10 to 0xXX17
	// hi: 0xXXX8 to 0xXXXF and 0xXX18 to 0xXX1F
	ppu->cpu_ppu_io->ppu_ctrl = 0x20; // 8x16 sprites
	// 0x0000 is 0x00 and 0x1000 is 0x08, no effect on output
	uint16_t pattern_table_start[8] = {0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x08, 0x08};
	ppu->cpu_ppu_io->ppu_ctrl |= pattern_table_start[_i & 0x07];
	// test the tile index offset works correctly (first 16 tests)
	// and then test different Y offsets (last 16 tests)
	unsigned scanline[32] = { 131, 131, 131, 131, 131, 131, 131, 131
	                        , 131, 131, 131, 131, 131, 131, 131, 131
	                        , 131, 132, 133, 134, 135, 136, 137, 138
	                        , 139, 140, 141, 142, 143, 144, 145, 146 };
	ppu->scanline = scanline[_i];
	// Y offset vals are 0-7 and 16-23
	// as if we add 8 to the initial value we may end up in the hi pattern table region
	// e.g. 0x0008 etc.
	int y_offset = 0;
	uint8_t tile_number = _i;
	unsigned sprite_number = _i & 0x07;
	ppu->scanline_oam[sprite_number * 4] = 131; // offset is scanline - oam Y byte
	ppu->scanline_oam[(sprite_number * 4) + 1] = tile_number;
	y_offset = ppu->scanline - ppu->scanline_oam[sprite_number * 4]; // reset in function call
	if (y_offset >= 8) { y_offset += 8; }
	uint16_t res = (0x1000 * (tile_number & 0x01)) | ((tile_number >> 1) << 5);
	res |= y_offset;

	get_sprite_address(ppu, &y_offset, sprite_number);

	ck_assert_uint_eq(ppu->sprite_addr, res);
}

START_TEST (flip_8_pixel_sprites_vertically)
{
	ppu->cpu_ppu_io->ppu_ctrl = 0; // 8x8 sprites
	// 0x0000 is 0x00 and 0x1000 is 0x08
	uint16_t pattern_table_start[8] = {0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x08, 0x08};
	ppu->cpu_ppu_io->ppu_ctrl |= pattern_table_start[_i];
	ppu->scanline = 131 + _i;
	int y_offset = _i;
	int flipped_offset = 7 - y_offset;
	uint8_t tile_number = _i;
	ppu->scanline_oam[_i * 4] = 131; // offset is scanline - oam Y byte
	ppu->scanline_oam[(_i * 4) + 1] = tile_number;
	// pattern_table start << 9 is 0x0000 or 0x1000
	uint16_t res = (pattern_table_start[_i] << 9) | (tile_number << 4);
	res |= flipped_offset; // add 7 - sprite Y offset (initial offset was 0)
	ppu->sprite_addr = (pattern_table_start[_i] << 9) | (tile_number << 4);
	unsigned expected_y_offsets[8] = {7, 6, 5, 4, 3, 2, 1, 0};

	flip_sprites_vertically(ppu, y_offset);

	ck_assert_uint_eq(flipped_offset, expected_y_offsets[_i]);
	ck_assert_uint_eq(ppu->sprite_addr, res);
}

START_TEST (flip_16_pixel_sprites_vertically)
{
	ppu->cpu_ppu_io->ppu_ctrl = 0x20; // 8x16 sprites
	// 0x0000 is 0x00 and 0x1000 is 0x08
	uint16_t pattern_table_start[8] = {0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x08, 0x08};
	ppu->cpu_ppu_io->ppu_ctrl |= pattern_table_start[_i & 0x07];
	ppu->scanline = 131 + _i;
	int y_offset = _i;
	int flipped_offset = 23 - _i;
	if (y_offset >= 8) {
		y_offset += 8;
		flipped_offset -= 8;
	}
	unsigned sprite_number = _i & 0x07;
	uint8_t tile_number = _i;
	ppu->scanline_oam[sprite_number * 4] = 131; // offset is scanline - oam Y byte
	ppu->scanline_oam[(sprite_number * 4) + 1] = tile_number;
	// pattern_table start << 9 is 0x0000 or 0x1000
	uint16_t res = (0x1000 * (tile_number & 0x01)) | ((tile_number >> 1) << 5);
	res |= flipped_offset;
	ppu->sprite_addr = (0x1000 * (tile_number & 0x01)) | ((tile_number >> 1) << 5);
	unsigned expected_y_offsets[16] = { 23, 22, 21, 20, 19, 18, 17, 16
	                                  ,  7,  6,  5,  4,  3,  2,  1,  0 };

	flip_sprites_vertically(ppu, y_offset);

	ck_assert_uint_eq(flipped_offset, expected_y_offsets[_i]);
	ck_assert_uint_eq(ppu->sprite_addr, res);
}

START_TEST (sprite_fetch_pattern_tables)
{
	ppu->cpu_ppu_io->ppu_ctrl = 0;
	// 0x0000 is 0x00 and 0x1000 is 0x08
	uint16_t pattern_table_start[8] = {0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x08, 0x08};
	ppu->cpu_ppu_io->ppu_ctrl |= pattern_table_start[_i];
	ppu->scanline = 131 + _i;
	int y_offset = 0;
	uint8_t tile_number = _i;
	unsigned sprite_number = _i & 0x07;
	uint8_t pattern_table_shift_reg[8] = { 0 };
	ppu->scanline_oam[sprite_number * 4] = 131; // offset is scanline - oam Y byte
	ppu->scanline_oam[(sprite_number * 4) + 1] = tile_number;
	get_sprite_address(ppu, &y_offset, sprite_number);
	// Simulate pattern table lo and hi fetches
	unsigned address_offsets[8] = {0, 0, 0, 8, 0, 8, 8, 8};
	ppu->sprite_addr += address_offsets[sprite_number];
	uint8_t write_val[8] = {0x78, 0x17, 0xE5, 0xCC, 0x4A, 0x01, 0x1F, 0x09};
	write_to_ppu_vram(&ppu->vram, ppu->sprite_addr, write_val[sprite_number]);
	// 0x00 no horizontal flip, 0x40 flip sprites horizontally
	uint8_t attributes_to_results[8][2] = { { 0x00, reverse_bits_in_byte(write_val[sprite_number]) }
                                          , { 0x40, write_val[sprite_number] }
                                          , { 0x00, reverse_bits_in_byte(write_val[sprite_number]) }
                                          , { 0x40, write_val[sprite_number] }
                                          , { 0x00, reverse_bits_in_byte(write_val[sprite_number]) }
                                          , { 0x40, write_val[sprite_number] }
                                          , { 0x00, reverse_bits_in_byte(write_val[sprite_number]) }
                                          , { 0x40, write_val[sprite_number] }
                                          };
	uint8_t attribute = attributes_to_results[sprite_number][0];
	uint8_t expected_result = attributes_to_results[sprite_number][1];
	ppu->sprite_at_latches[sprite_number] = attribute;


	load_sprite_pattern_table_data(ppu, &pattern_table_shift_reg[0]
	                              , sprite_number, ppu->sprite_addr);

	ck_assert_uint_eq(pattern_table_shift_reg[sprite_number], expected_result);
}


START_TEST (sprite_render_all_inactive)
{
	// If x counter is non-zero (inactive) it should only be decremented once
	for (int i = 7; i >= 0; i--) {
		ppu->sprite_x_counter[i] = 1 + i; // highest: 8, lowest: 1
		ppu->sprite_pt_hi_shift_reg[i] = 0xFF;
		ppu->sprite_pt_lo_shift_reg[i] = 0xFF; // pt: 0x11 (hi/lo)
		ppu->sprite_at_latches[i] = 0;
	}

	ppu->cycle = 50;
	ppu->cpu_ppu_io->ppu_mask = 0x10;
	write_to_ppu_vram(&ppu->vram, 0x3F00, 2); // background colour
	write_to_ppu_vram(&ppu->vram, 0x3F10, 5); // background colour, invalid entry
	write_to_ppu_vram(&ppu->vram, 0x3F13, 15); // non-background colour via pt and at (at * 2 plus pt)

	uint8_t colour_reference = 0x00;
	get_sprite_pixel(ppu, &colour_reference);

	for (int i = 7; i >= 0; i--) {
		ck_assert_uint_eq(ppu->sprite_pt_hi_shift_reg[i], 0xFF);
		ck_assert_uint_eq(ppu->sprite_x_counter[i], i);
	}
	// check that $3F00 is output on the sprite if no sprite has been found
	ck_assert_uint_eq(ppu->current_pixel.sprite_pattern_index, 0);
	ck_assert_uint_eq(ppu->current_pixel.sprite_col, 2);
}

START_TEST (sprite_render_in_range_shifts_pt_out)
{
	// In range when x_counter == 0
	for (int i = 7; i >= 0; i--) {
		ppu->sprite_x_counter[i] = 0;
		ppu->sprite_pt_hi_shift_reg[i] = 0xFF >> _i;
		ppu->sprite_pt_lo_shift_reg[i] = 0xFF >> _i;
		ppu->sprite_at_latches[i] = 0;
	}

	ppu->cycle = 83;
	ppu->cpu_ppu_io->ppu_mask = 0x10;

	uint8_t colour_reference = 0x00;
	get_sprite_pixel(ppu, &colour_reference);

	for (int i = 7; i >= 0; i--) {
		ck_assert_uint_eq(ppu->sprite_pt_hi_shift_reg[i], 0xFF >> (_i + 1));
		ck_assert_uint_eq(ppu->sprite_x_counter[i], 0); // no decrement should happen
	}
}

START_TEST (sprite_renders_correct_palette_address)
{
	// bytes: attribute, pt_hi, pt_lo, palette_offset form $3F10
	// non-zero offsets, as 0 offsets will render $3F00
	uint8_t attribute_pattern_offset[12][4] = { {0, 0, 1, 1}
	                                          , {0, 1, 0, 2}
	                                          , {0, 1, 1, 3}
	                                          , {1, 0, 1, 5}
	                                          , {1, 1, 0, 6}
	                                          , {1, 1, 1, 7}
	                                          , {2, 0, 1, 9}
	                                          , {2, 1, 0, 10}
	                                          , {2, 1, 1, 11}
	                                          , {3, 0, 1, 13}
	                                          , {3, 1, 0, 14}
	                                          , {3, 1, 1, 15}
	};
	unsigned active_sprite = 5;
	unsigned palette_offset = attribute_pattern_offset[_i][3];
	// If x counter is decremented and non-zero nothing is done
	for (int i = 7; i >= 0; i--) {
		ppu->sprite_x_counter[i] = 250;
		ppu->sprite_pt_hi_shift_reg[i] = attribute_pattern_offset[_i][1];
		ppu->sprite_pt_lo_shift_reg[i] = attribute_pattern_offset[_i][2];
		ppu->sprite_at_latches[i] = attribute_pattern_offset[_i][0];
	}


	ppu->cycle = 12;
	ppu->cpu_ppu_io->ppu_mask = 0x10;
	ppu->sprite_x_counter[active_sprite] = 0;
	write_to_ppu_vram(&ppu->vram, 0x3F00, 2); // background colour
	write_to_ppu_vram(&ppu->vram, 0x3F10, 5); // background colour, invalid entry
	write_to_ppu_vram(&ppu->vram, 0x3F10 + palette_offset, 8); // non-background colour via pt and at (at * 2 plus pt)

	uint8_t colour_reference = 0x00;
	get_sprite_pixel(ppu, &colour_reference);

	ck_assert_uint_eq(colour_reference, 8);
}

START_TEST (sprite_renders_highest_priority_sprite)
{
	// higher priority sprites have a lower index
	unsigned low_priority = 5;
	unsigned high_priority = 2;
	// If x counter is decremented and non-zero nothing is done
	for (int i = 7; i >= 0; i--) {
		ppu->sprite_x_counter[i] = 250;
		ppu->sprite_pt_hi_shift_reg[i] = 0xFF;
		ppu->sprite_pt_lo_shift_reg[i] = 0xFF;
		ppu->sprite_at_latches[i] = 0xFF;
	}
	// Low priority
	ppu->sprite_x_counter[low_priority] = 0;
	ppu->sprite_pt_hi_shift_reg[low_priority] = 1;
	ppu->sprite_pt_lo_shift_reg[low_priority] = 0;
	ppu->sprite_at_latches[low_priority] = 0;
	write_to_ppu_vram(&ppu->vram, 0x3F12, 10); // non-background colour via pt and at (at * 2 plus pt)
	// High priority
	ppu->sprite_x_counter[high_priority] = 0;
	ppu->sprite_pt_hi_shift_reg[high_priority] = 1;
	ppu->sprite_pt_lo_shift_reg[high_priority] = 1;
	ppu->sprite_at_latches[high_priority] = 0;
	write_to_ppu_vram(&ppu->vram, 0x3F13, 7); // non-background colour via pt and at (at * 2 plus pt)
	ppu->cycle = 50;
	ppu->cpu_ppu_io->ppu_mask = 0x10;
	write_to_ppu_vram(&ppu->vram, 0x3F00, 2); // background colour
	write_to_ppu_vram(&ppu->vram, 0x3F10, 5); // background colour, invalid entry

	uint8_t colour_reference = 0x00;
	get_sprite_pixel(ppu, &colour_reference);

	ck_assert_uint_eq(colour_reference, 7);
}

START_TEST (sprite_renders_highest_priority_non_transparent_sprite)
{
	// higher priority sprites have a lower index
	unsigned low_priority = 6;
	unsigned high_priority = 2;
	// If x counter is decremented and non-zero nothing is done
	for (int i = 7; i >= 0; i--) {
		ppu->sprite_x_counter[i] = 250; // force sprites out of range
		ppu->sprite_pt_hi_shift_reg[i] = 0xFF;
		ppu->sprite_pt_lo_shift_reg[i] = 0xFF;
		ppu->sprite_at_latches[i] = 0xFF;
	}
	// Low priority (non-transparent)
	ppu->sprite_x_counter[low_priority] = 0; // make sure sprites are overlapping
	ppu->sprite_pt_hi_shift_reg[low_priority] = 0;
	ppu->sprite_pt_lo_shift_reg[low_priority] = 1;
	ppu->sprite_at_latches[low_priority] = 0;
	write_to_ppu_vram(&ppu->vram, 0x3F11, 11); // non-background colour via pt and at (at * 2 plus pt)
	// High priority (transparent)
	ppu->sprite_x_counter[high_priority] = 0; // make sure sprites are overlapping
	ppu->sprite_pt_hi_shift_reg[high_priority] = 0;
	ppu->sprite_pt_lo_shift_reg[high_priority] = 0;
	ppu->sprite_at_latches[high_priority] = 0;
	ppu->cycle = 21;
	ppu->cpu_ppu_io->ppu_mask = 0x10;
	write_to_ppu_vram(&ppu->vram, 0x3F00, 2); // background colour
	write_to_ppu_vram(&ppu->vram, 0x3F10, 5); // background colour, invalid entry

	uint8_t colour_reference = 0x00;
	get_sprite_pixel(ppu, &colour_reference);

	ck_assert_uint_eq(colour_reference, 11);
	ck_assert_uint_eq(ppu->current_pixel.sprite_col, 11);
	ck_assert_uint_eq(ppu->current_pixel.sprite_pattern_index, 1);
	ck_assert_uint_eq(ppu->current_pixel.scanline_sprite, low_priority);
}

START_TEST (sprite_renders_left_masking)
{
	// A bit in 0x04 will show the leftmost 8 pixels containing any sprites
	// no bit = output the common background 0x3F00
	uint8_t mask_to_output[9][2] = { {0x00, 2}
	                               , {0x04, 22}
	                               , {0x00, 2}
	                               , {0x04, 22}
	                               , {0x04, 22}
	                               , {0x04, 22}
	                               , {0x00, 2}
	                               , {0x00, 2}
	                               , {0x00, 22}  // 9th pixel is outputted regardless of masking (if bkg rendering enabled)
	};
	// Force spites in range
	for (int i = 7; i >= 0; i--) {
		ppu->sprite_x_counter[i] = 0;
		ppu->sprite_pt_hi_shift_reg[i] = 0xFF;
		ppu->sprite_pt_lo_shift_reg[i] = 0; // 10 (pt hi/lo)
		ppu->sprite_at_latches[i] = 0;
	}
	ppu->cycle = _i;
	ppu->cpu_ppu_io->ppu_mask = mask_to_output[_i][0] | 0x10; // enable sprites
	write_to_ppu_vram(&ppu->vram, 0x3F00, 2); // background colour
	write_to_ppu_vram(&ppu->vram, 0x3F10, 5); // background colour, invalid entry
	write_to_ppu_vram(&ppu->vram, 0x3F12, 22); // non-background colour via pt and at (at * 2 plus pt)

	uint8_t colour_reference = 0x00;
	get_sprite_pixel(ppu, &colour_reference);

	ck_assert_uint_eq(colour_reference, mask_to_output[_i][1]);
}

START_TEST (sprite_renders_enabled_disabled)
{
	// A bit in 0x04 will show the leftmost 8 pixels containing any sprites
	// no bit = output the common background 0x3F00
	// A bit in 0x10 enables sprite rendering, disabled otherwise
	uint8_t mask_to_output[8][2] = { {0x14, 19}
	                               , {0x10, 1}
	                               , {0x14, 19}
	                               , {0x00, 1}
	                               , {0x04, 1}
	                               , {0x10, 19}
	                               , {0x14, 19}
	                               , {0x00, 1}
	};
	// Force spites in range
	for (int i = 7; i >= 0; i--) {
		ppu->sprite_x_counter[i] = 0;
		ppu->sprite_pt_hi_shift_reg[i] = 0xFF;
		ppu->sprite_pt_lo_shift_reg[i] = 0; // 10 (pt hi/lo)
		ppu->sprite_at_latches[i] = 0;
	}
	ppu->cycle = _i * 6;
	ppu->cpu_ppu_io->ppu_mask = mask_to_output[_i][0];
	write_to_ppu_vram(&ppu->vram, 0x3F00, 1); // background colour
	write_to_ppu_vram(&ppu->vram, 0x3F10, 5); // background colour, invalid entry
	write_to_ppu_vram(&ppu->vram, 0x3F12, 19); // non-background colour via pt and at (at * 2 plus pt)

	uint8_t colour_reference = 0x00;
	get_sprite_pixel(ppu, &colour_reference);

	ck_assert_uint_eq(colour_reference, mask_to_output[_i][1]);
}


START_TEST (bkg_and_sprite_transparent_pixels)
{
	ppu->current_pixel.bkg_pattern_index = 0;
	ppu->current_pixel.sprite_pattern_index = 0;
	ppu->current_pixel.bkg_col = 4; // output of $3F00
	ppu->current_pixel.sprite_col = 10; // random output, should also be $3F00
	ppu->current_pixel.scanline_sprite = 2; // scanline sprite being rendered
	ppu->sprite_at_latches[ppu->current_pixel.scanline_sprite] = 0x00; // sprite is ahead of bkg (ingored)

	get_pixel(&ppu->current_pixel, sprite_is_front_priority(ppu, ppu->current_pixel.scanline_sprite));

	ck_assert_uint_eq(ppu->current_pixel.output_col, 4);
}

START_TEST (bkg_transparent_sprite_not_transparent)
{
	ppu->current_pixel.bkg_pattern_index = 0;
	ppu->current_pixel.sprite_pattern_index = 3;
	ppu->current_pixel.bkg_col = 1; // output of $3F00
	ppu->current_pixel.sprite_col = 31;
	ppu->current_pixel.scanline_sprite = 4; // scanline sprite being rendered
	ppu->sprite_at_latches[ppu->current_pixel.scanline_sprite] = 0x20; // sprite is behing bkg (ingored)

	get_pixel(&ppu->current_pixel, sprite_is_front_priority(ppu, ppu->current_pixel.scanline_sprite));

	ck_assert_uint_eq(ppu->current_pixel.output_col, 31);
}

START_TEST (bkg_not_transparent_sprite_transparent)
{
	ppu->current_pixel.bkg_pattern_index = 1;
	ppu->current_pixel.sprite_pattern_index = 0;
	ppu->current_pixel.bkg_col = 19;
	ppu->current_pixel.sprite_col = 5; // output of $3F00
	ppu->current_pixel.scanline_sprite = 0; // scanline sprite being rendered
	ppu->sprite_at_latches[ppu->current_pixel.scanline_sprite] = 0x00; // sprite is ahead of bkg (ignore)

	get_pixel(&ppu->current_pixel, sprite_is_front_priority(ppu, ppu->current_pixel.scanline_sprite));

	ck_assert_uint_eq(ppu->current_pixel.output_col, 19);
}

START_TEST (sprite_priority_ahead_of_bkg)
{
	ppu->current_pixel.bkg_pattern_index = 1;
	ppu->current_pixel.sprite_pattern_index = 1;
	ppu->current_pixel.bkg_col = 25;
	ppu->current_pixel.sprite_col = 28;
	ppu->current_pixel.scanline_sprite = 6; // scanline sprite being rendered
	ppu->sprite_at_latches[ppu->current_pixel.scanline_sprite] = 0x00; // sprite is ahead of bkg

	get_pixel(&ppu->current_pixel, sprite_is_front_priority(ppu, ppu->current_pixel.scanline_sprite));

	ck_assert_uint_eq(ppu->current_pixel.output_col, 28);
}

START_TEST (sprite_priority_behind_bkg)
{
	ppu->current_pixel.bkg_pattern_index = 3;
	ppu->current_pixel.sprite_pattern_index = 1;
	ppu->current_pixel.bkg_col = 7;
	ppu->current_pixel.sprite_col = 13;
	ppu->current_pixel.scanline_sprite = 7; // scanline sprite being rendered
	ppu->sprite_at_latches[ppu->current_pixel.scanline_sprite] = 0x20; // sprite is behind bkg

	get_pixel(&ppu->current_pixel, sprite_is_front_priority(ppu, ppu->current_pixel.scanline_sprite));

	ck_assert_uint_eq(ppu->current_pixel.output_col, 7);
}


Suite* ppu_master_suite(void)
{
	Suite* s;

	s = suite_create("All Ppu Tests");

	return s;
}

Suite* ppu_test_helpers_suite(void)
{
	Suite* s;
	TCase* tc_ppu_unit_test_helpers;

	s = suite_create("Ppu Unit Test Helpers");
	tc_ppu_unit_test_helpers = tcase_create("PPU Unit Test Helper Functions");
	tcase_add_checked_fixture(tc_ppu_unit_test_helpers, setup, teardown);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_0, 0, 8);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_1, 0, 9);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_2, 0, 9);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, vram_encoder_nametable_3, 0, 9);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_0, 0, 4);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_0_tile_sample_within_at_byte, 0, 4);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_1, 0, 4);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_1_tile_sample_within_at_byte, 0, 4);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_2, 0, 4);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_2_tile_sample_within_at_byte, 0, 4);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_3, 0, 4);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, attribute_address_encoder_nametable_3_tile_sample_within_at_byte, 0, 4);
	tcase_add_loop_test(tc_ppu_unit_test_helpers, reverse_bits_function_all_valid_inputs, 0, 256);
	suite_add_tcase(s, tc_ppu_unit_test_helpers);

	return s;
}

Suite* ppu_vram_suite(void)
{
	Suite* s;
	TCase* tc_ppu_vram_read_writes;

	s = suite_create("Ppu Vram Tests");
	tc_ppu_vram_read_writes = tcase_create("VRAM Read/Write Tests");
	tcase_add_checked_fixture(tc_ppu_vram_read_writes, vram_setup, vram_teardown);
	tcase_add_loop_test(tc_ppu_vram_read_writes, pattern_table_0_writes, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, pattern_table_1_writes, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_0_writes, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_1_writes, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_2_writes, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_3_writes, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_0_mirror_writes, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_1_mirror_writes, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_2_mirror_writes, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_3_partial_mirror_writes, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, palette_ram_writes, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, palette_ram_mirror_writes, 0, 4);
	tcase_add_test(tc_ppu_vram_read_writes, writes_past_upper_bound_have_no_effect);
	tcase_add_loop_test(tc_ppu_vram_read_writes, pattern_table_0_reads, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, pattern_table_1_reads, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_0_reads, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_1_reads, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_2_reads, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_3_reads, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_0_mirror_reads, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_1_mirror_reads, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_2_mirror_reads, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, nametable_3_partial_mirror_reads, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, palette_ram_reads_ignores_unused_bits, 0, 3);
	tcase_add_loop_test(tc_ppu_vram_read_writes, palette_ram_mirror_reads_ignores_unused_bits, 0, 3);
	suite_add_tcase(s, tc_ppu_vram_read_writes);

	return s;
}

Suite* ppu_rendering_suite(void)
{
	Suite* s;
	TCase* tc_bkg_rendering;
	TCase* tc_sprite_evaluation;
	TCase* tc_sprite_rendering;
	TCase* tc_bkg_sprite_priority;

	s = suite_create("Ppu Rendering Related Tests");
	tc_bkg_rendering = tcase_create("Background Rendering Tests");
	tcase_add_checked_fixture(tc_bkg_rendering, vram_setup, vram_teardown);
	tcase_add_test(tc_bkg_rendering, nametable_mirroring_horizontal);
	tcase_add_test(tc_bkg_rendering, nametable_mirroring_horizontal_read_writes);
	tcase_add_test(tc_bkg_rendering, nametable_mirroring_vertical);
	tcase_add_test(tc_bkg_rendering, nametable_mirroring_vertical_read_writes);
	tcase_add_test(tc_bkg_rendering, nametable_mirroring_single_screen_A);
	tcase_add_test(tc_bkg_rendering, nametable_mirroring_single_screen_A_read_writes);
	tcase_add_test(tc_bkg_rendering, nametable_mirroring_single_screen_B);
	tcase_add_test(tc_bkg_rendering, nametable_mirroring_single_screen_B_read_writes);
	tcase_add_loop_test(tc_bkg_rendering, nametable_x_offset_is_valid_for_all_coarse_x, 0, 64);
	tcase_add_loop_test(tc_bkg_rendering, nametable_y_offset_is_valid_for_all_coarse_y, 0, 60);
	tcase_add_loop_test(tc_bkg_rendering, fetch_nametable_byte_nametable_0_addr_ignore_fine_y, 0, 2);
	tcase_add_loop_test(tc_bkg_rendering, fetch_nametable_byte_nametable_1_addr_ignore_fine_y, 0, 2);
	tcase_add_loop_test(tc_bkg_rendering, fetch_nametable_byte_nametable_2_addr_ignore_fine_y, 0, 2);
	tcase_add_loop_test(tc_bkg_rendering, fetch_nametable_byte_nametable_3_addr_ignore_fine_y, 0, 2);
	tcase_add_loop_test(tc_bkg_rendering, fetch_nametable_byte_in_attribute_table, 0, 4);
	tcase_add_test(tc_bkg_rendering, fetch_attribute_byte_nametable_0_random_scroll_offsets);
	tcase_add_test(tc_bkg_rendering, fetch_attribute_byte_nametable_1_random_scroll_offsets);
	tcase_add_test(tc_bkg_rendering, fetch_attribute_byte_nametable_2_random_scroll_offsets);
	tcase_add_test(tc_bkg_rendering, fetch_attribute_byte_nametable_3_random_scroll_offsets);
	tcase_add_loop_test(tc_bkg_rendering, fetch_pattern_table_lo_for_fine_y_offsets, 0, 2);
	tcase_add_loop_test(tc_bkg_rendering, fetch_pattern_table_hi_for_fine_y_offsets, 0, 2);
	tcase_add_test(tc_bkg_rendering, attribute_shift_reg_from_bottom_right_quadrant);
	tcase_add_test(tc_bkg_rendering, attribute_shift_reg_from_top_right_quadrant);
	tcase_add_test(tc_bkg_rendering, attribute_shift_reg_from_bottom_left_quadrant);
	tcase_add_test(tc_bkg_rendering, attribute_shift_reg_from_top_left_quadrant);
	tcase_add_loop_test(tc_bkg_rendering, pixel_buffer_set_corner_pixels, 0, 4);
	tcase_add_test(tc_bkg_rendering, pixel_buffer_set_out_of_bounds_allowed);
	tcase_add_loop_test(tc_bkg_rendering, bkg_render_left_masking_unmasking, 0, 9);
	tcase_add_loop_test(tc_bkg_rendering, bkg_render_enabled_or_disabled, 0, 6);
	tcase_add_loop_test(tc_bkg_rendering, bkg_palette_address_non_zero_offsets_no_fine_x, 0, 12);
	tcase_add_loop_test(tc_bkg_rendering, bkg_palette_address_offsets_with_fine_x, 0, 8);
	tcase_add_loop_test(tc_bkg_rendering, bkg_output_transparent_pixel, 0, 6);
	tcase_add_test(tc_bkg_rendering, debug_all_nametables);
	suite_add_tcase(s, tc_bkg_rendering);
	tc_sprite_evaluation = tcase_create("Sprite Evaluation Tests");
	tcase_add_checked_fixture(tc_sprite_evaluation, setup, teardown);
	tcase_add_test(tc_sprite_evaluation, clear_scanline_oam);
	tcase_add_test(tc_sprite_evaluation, sprite_eval_odd_cycles_read_only);
	tcase_add_loop_test(tc_sprite_evaluation, sprite_eval_in_range_oam_transfer, 0, 2);
	tcase_add_loop_test(tc_sprite_evaluation, sprite_eval_none_in_range, 0, 2);
	tcase_add_test(tc_sprite_evaluation, sprite_eval_transfer_oam_on_even_cycles_only);
	tcase_add_loop_test(tc_sprite_evaluation, sprite_eval_sprites_found_behaviour, 1, 11);
	tcase_add_loop_test(tc_sprite_evaluation, sprite_eval_sprite_overflow_behaviour, 1, 7);
	suite_add_tcase(s, tc_sprite_evaluation);
	tc_sprite_rendering = tcase_create("Sprite Rendering Tests");
	tcase_add_checked_fixture(tc_sprite_rendering, setup, teardown);
	tcase_add_loop_test(tc_sprite_rendering, sprite_fetch_address_8_pixel_sprites, 0, 8);
	tcase_add_loop_test(tc_sprite_rendering, sprite_fetch_address_16_pixel_sprites, 0, 32);
	tcase_add_loop_test(tc_sprite_rendering, flip_8_pixel_sprites_vertically, 0, 8);
	tcase_add_loop_test(tc_sprite_rendering, flip_16_pixel_sprites_vertically, 0, 16);
	tcase_add_loop_test(tc_sprite_rendering, sprite_fetch_pattern_tables, 0, 8);
	tcase_add_test(tc_sprite_rendering, sprite_render_all_inactive);
	tcase_add_loop_test(tc_sprite_rendering, sprite_render_in_range_shifts_pt_out, 0, 8);
	tcase_add_loop_test(tc_sprite_rendering, sprite_renders_correct_palette_address, 0, 12);
	tcase_add_test(tc_sprite_rendering, sprite_renders_highest_priority_sprite);
	tcase_add_test(tc_sprite_rendering, sprite_renders_highest_priority_non_transparent_sprite);
	tcase_add_loop_test(tc_sprite_rendering, sprite_renders_left_masking, 0, 9);
	tcase_add_loop_test(tc_sprite_rendering, sprite_renders_enabled_disabled, 0, 8);
	suite_add_tcase(s, tc_sprite_rendering);
	tc_bkg_sprite_priority = tcase_create("Background vs Sprite Rendering Tests");
	tcase_add_checked_fixture(tc_bkg_sprite_priority, setup, teardown);
	tcase_add_test(tc_bkg_sprite_priority, bkg_and_sprite_transparent_pixels);
	tcase_add_test(tc_bkg_sprite_priority, bkg_transparent_sprite_not_transparent);
	tcase_add_test(tc_bkg_sprite_priority, bkg_not_transparent_sprite_transparent);
	tcase_add_test(tc_bkg_sprite_priority, sprite_priority_ahead_of_bkg);
	tcase_add_test(tc_bkg_sprite_priority, sprite_priority_behind_bkg);
	suite_add_tcase(s, tc_bkg_sprite_priority);

	return s;
}
