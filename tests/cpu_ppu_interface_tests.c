#include <check.h>

#include <stdlib.h>

#include "cpu_ppu_interface.h"
#include "ppu.h" // for vram functions (reads/writes and increments)
#include "cpu.h" // for ppu reg read/write functions which call cpu/ppu functions e.g. write_2007()
#include "cart.h" // needed for cpu/ppu $2007 writes
#include "cpu_mapper_interface.h" // For cpu/mapper struct

Cpu6502* cpio_cpu;
Cartridge* cpio_cart;
CpuPpuShare* cpu_ppu_tester;
CpuMapperShare* cpio_cpu_mapper;
struct PpuMemoryMap* cpio_vram;
uint8_t cpio_oam[256];
uint16_t cpio_vram_addr;
uint16_t cpio_vram_tmp_addr;
uint8_t cpio_fine_x;

static void cart_setup(void)
{
	cpio_cart = malloc(sizeof(Cartridge)); // test double
	if (!cpio_cart) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to cpu struct");
	}
}

static void cart_teardown(void)
{
	free(cpio_cart);
}

static void cpu_setup(void)
{
	cpio_cpu = malloc(sizeof(Cpu6502)); // test double
	if (!cpio_cpu) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to cpu struct");
	}
}

static void cpu_teardown(void)
{
	free(cpio_cpu);
}

static void cpu_mapper_setup(void)
{
	cpio_cpu_mapper = malloc(sizeof(CpuMapperShare)); // test double
	if (!cpio_cpu_mapper) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to cpu/mapper struct");
	}
}

static void cpu_mapper_teardown(void)
{
	free(cpio_cpu_mapper);
}

static void vram_setup(void)
{
	cpio_vram = malloc(sizeof(struct PpuMemoryMap)); // test double
	if (!cpio_vram) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to vram struct");
	}

	cpio_vram->pattern_table_0k = malloc(4 * KiB);
	cpio_vram->pattern_table_4k = malloc(4 * KiB);

	if (!cpio_vram->pattern_table_0k || !cpio_vram->pattern_table_4k) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to the ppu chr data");
	}

	cpu_ppu_tester->vram = cpio_vram;
	cpu_ppu_tester->vram->pattern_table_0k = cpio_vram->pattern_table_0k;
	cpu_ppu_tester->vram->pattern_table_4k = cpio_vram->pattern_table_4k;
}

static void vram_teardown(void)
{
	free(cpio_vram->pattern_table_0k);
	free(cpio_vram->pattern_table_4k);
	free(cpio_vram);
}

static void setup(void)
{
	cpu_ppu_tester = malloc(sizeof(CpuPpuShare)); // test double
	if (!cpu_ppu_tester) {
		// malloc fails
		ck_abort_msg("Failed to allocate memory to cpu/ppu struct");
	}

	cpu_setup();
	cart_setup();
	cpu_mapper_setup();
	vram_setup();

	cpu_ppu_io_init(cpu_ppu_tester);

	cpu_ppu_tester->oam = &cpio_oam[0];
	cpu_ppu_tester->vram_addr = &cpio_vram_addr;
	cpu_ppu_tester->vram_tmp_addr = &cpio_vram_tmp_addr;
	cpu_ppu_tester->fine_x = &cpio_fine_x;
	cpio_cpu->cpu_ppu_io = cpu_ppu_tester;
	cpio_cpu->cpu_mapper_io = cpio_cpu_mapper;
	cpio_cpu->cpu_mapper_io->chr_rom = &cpio_cart->chr_rom;
	cpio_cpu->cpu_mapper_io->chr_ram = &cpio_cart->chr_ram;
	cpio_cpu->cpu_mapper_io->chr_ram->size = 1; // allow writes to CHR-RAM
}

static void teardown(void)
{
	free(cpu_ppu_tester);
	cpu_teardown();
	cpu_mapper_teardown();
	cart_teardown();
	vram_teardown();
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


START_TEST (read_ppu_status_2002_resets)
{
	// Reads from $2002 reset the 2006/7 write toggle and
	// Vblank flag in $2002
	cpu_ppu_tester->ppu_status = 0xE0; // set all $2002 flags

	//read_ppu_reg(0x2002, cpio_cpu);
	read_2002(cpu_ppu_tester);

	ck_assert_uint_eq(0x60, cpu_ppu_tester->ppu_status);
}

START_TEST (read_ppu_status_2002_return_value)
{
	// Reads from $2002 returns the current $2002 value
	cpu_ppu_tester->ppu_status = 0xE0; // set all $2002 flags

	uint8_t data = read_ppu_reg(0x2002, cpio_cpu);

	ck_assert_uint_eq(0xE0, data);
}

START_TEST (read_oam_data_2004_outside_of_rendering)
{
	// Reads from $2004 return the value held in oam at OAMADDR
	cpu_ppu_tester->oam_addr = 0x31;
	cpu_ppu_tester->oam[cpu_ppu_tester->oam_addr] = 0xBA;
	cpu_ppu_tester->ppu_rendering_period = true; // should be set outside of vblank
	cpu_ppu_tester->ppu_mask = 0x00; // FORCED blanking
	// however, with sprite and background rendering disabled
	// we are forced blanking and therefore not rendering

	uint8_t data = read_ppu_reg(0x2004, cpio_cpu);

	ck_assert_uint_eq(0xBA, data);
	// no increment is performed outside of rendering
	ck_assert_uint_eq(0x31, cpu_ppu_tester->oam_addr);
}

START_TEST (read_oam_data_2004_during_rendering)
{
	// Reads from $2004 return the value held in oam at OAMADDR
	cpu_ppu_tester->oam_addr = 0x31;
	cpu_ppu_tester->oam[cpu_ppu_tester->oam_addr] = 0xBB;
	cpu_ppu_tester->ppu_rendering_period = true; // should be set outside of vblank
	cpu_ppu_tester->ppu_mask = 0x10; // show sprites

	uint8_t data = read_ppu_reg(0x2004, cpio_cpu);

	ck_assert_uint_eq(0xBB, data);
	// no increment is performed outside of rendering
	ck_assert_uint_eq(0x31 + 1, cpu_ppu_tester->oam_addr);
}

START_TEST (read_oam_data_2004_unused_attribute_bits_as_clear)
{
	// Reads from $2004 return the value held in oam at OAMADDR
	// when reading from an attribute the unused bits are
	// read back as 0, unused bits being: 2-4
	cpu_ppu_tester->oam_addr = 0x0A;
	cpu_ppu_tester->oam[cpu_ppu_tester->oam_addr] = 0xFF;

	uint8_t data = read_ppu_reg(0x2004, cpio_cpu);

	ck_assert_uint_eq(0xFF & ~0x1C, data);
	ck_assert_uint_eq(0x0A, cpu_ppu_tester->oam_addr);
}

START_TEST (read_ppu_data_2007_non_palette_buffering)
{
	// Reads from $2007 return an internal buffer
	cpu_ppu_tester->buffer_2007 = 0x8D;
	*(cpu_ppu_tester->vram_addr) = 0x0008;
	write_to_ppu_vram(cpu_ppu_tester->vram, *(cpu_ppu_tester->vram_addr), 0x15);

	uint8_t data = read_ppu_reg(0x2007, cpio_cpu);

	ck_assert_uint_eq(0x8D, data);
	ck_assert_uint_eq(0x15, cpu_ppu_tester->buffer_2007);
}

START_TEST (read_ppu_data_2007_palette_buffering)
{
	// Reads from $2007 updates the internal buffer from a nametable address
	// whilst immediately returning the value pointed to in the palette region
	cpu_ppu_tester->vram->nametable_0 = &cpu_ppu_tester->vram->nametable_A;
	cpu_ppu_tester->vram->nametable_1 = &cpu_ppu_tester->vram->nametable_A;
	cpu_ppu_tester->vram->nametable_2 = &cpu_ppu_tester->vram->nametable_A;
	cpu_ppu_tester->vram->nametable_3 = &cpu_ppu_tester->vram->nametable_A;
	cpu_ppu_tester->buffer_2007 = 0x8D;
	*(cpu_ppu_tester->vram_addr) = 0x3F03;
	write_to_ppu_vram(cpu_ppu_tester->vram, *(cpu_ppu_tester->vram_addr), 0x15);
	write_to_ppu_vram(cpu_ppu_tester->vram, *(cpu_ppu_tester->vram_addr) - 0x1000, 0xD0);

	uint8_t data = read_ppu_reg(0x2007, cpio_cpu);

	ck_assert_uint_eq(0x15, data);
	ck_assert_uint_eq(0xD0, cpu_ppu_tester->buffer_2007);
}

START_TEST (read_ppu_data_2007_outside_of_rendering)
{
	// Reads from $2007 outside of rendering causes the vram address
	// to increment by either 1 or 32 (by value in PPUCTRL flag)
	uint8_t reg_val[2] = {0x00, 0x04};  // clear bit (+1), set bit (+32)
	unsigned increment[2] = {1, 32};
	cpu_ppu_tester->ppu_ctrl = reg_val[_i];
	cpu_ppu_tester->ppu_rendering_period = false; // should be set to false during vblank
	cpu_ppu_tester->ppu_mask = 0x08; // show background
	cpu_ppu_tester->buffer_2007 = 0x1F;
	*(cpu_ppu_tester->vram_addr) = 0x0009;
	write_to_ppu_vram(cpu_ppu_tester->vram, *(cpu_ppu_tester->vram_addr), 0x15);

	read_ppu_reg(0x2007, cpio_cpu);

	ck_assert_uint_eq(0x0009 + increment[_i], *(cpu_ppu_tester->vram_addr));
}

START_TEST (read_ppu_data_2007_during_rendering)
{
	// Reads from $2007 during rendering causes a glitchy increment of
	// the vram address (both to coarse x and fine y)
	cpu_ppu_tester->ppu_rendering_period = true; // should be set outside of vblank
	cpu_ppu_tester->ppu_mask = 0x08; // show background
	cpu_ppu_tester->buffer_2007 = 0x1F;
	*(cpu_ppu_tester->vram_addr) = 0x0009;
	write_to_ppu_vram(cpu_ppu_tester->vram, *(cpu_ppu_tester->vram_addr), 0x15);

	read_ppu_reg(0x2007, cpio_cpu);

	// glitchy increment is +1 to fine Y and +1 to coarse X (when not at a tile edge)
	ck_assert_uint_eq(0x0009 + 1 + 0x1000, *(cpu_ppu_tester->vram_addr));
}

START_TEST (write_ppu_ctrl_2000_scrolling_clears_specific_bits)
{
	// Writes to $2000 clears the 0x0C00 bits
	*(cpu_ppu_tester->vram_tmp_addr) = 0xFFFF;
	write_ppu_reg(0x2000, 0x40, cpio_cpu);

	ck_assert_uint_eq(0xFFFF & ~0x0C00, *(cpu_ppu_tester->vram_tmp_addr));
}

START_TEST (write_ppu_ctrl_2000_scrolling_sets_specific_bits)
{
	// Writes to $2000 sets the 0x0C00 bits from input bits 0-1
	*(cpu_ppu_tester->vram_tmp_addr) = 0x0000;
	write_ppu_reg(0x2000, 0x03, cpio_cpu);

	ck_assert_uint_eq(0x0C00, *(cpu_ppu_tester->vram_tmp_addr));
}

START_TEST (write_oam_addr_2003_sets_oam_address)
{
	// Writes to $2003 set the OAMADDR
	write_ppu_reg(0x2003, 0x17, cpio_cpu);

	ck_assert_uint_eq(0x17, cpu_ppu_tester->oam_addr);
}

START_TEST (write_oam_data_2004_outside_rendering_period)
{
	// Writes to $2004 sends data to OAM and increment the OAMADDR
	cpu_ppu_tester->oam_addr = 0x05;

	write_ppu_reg(0x2004, 0xFF, cpio_cpu);

	ck_assert_uint_eq(0xFF, cpu_ppu_tester->oam_data);
	ck_assert_uint_eq(0xFF, cpu_ppu_tester->oam[0x05]);
	ck_assert_uint_eq(0x05 + 1, cpu_ppu_tester->oam_addr);
}

START_TEST (write_oam_data_2004_during_rendering_period)
{
	// Writes to $2004 during rendering increments OAMADDR in a glitchy manner
	cpu_ppu_tester->oam_addr = 0x21;
	cpu_ppu_tester->ppu_mask = 0x10; // show sprites
	cpu_ppu_tester->ppu_rendering_period = true; // on active scanline

	// Only upper 6 bits are incremented
	// similar to shifting down by 2, adding 1, shifting up by 2, keeping original low 2 bits
	// which is the same as +4 believe it or not
	uint8_t expected_result = (((cpu_ppu_tester->oam_addr >> 2) + 1) << 2)
	                        |  (cpu_ppu_tester->oam_addr & 0x03);

	write_ppu_reg(0x2004, 0xFF, cpio_cpu);

	ck_assert_uint_ne(0xFF, cpu_ppu_tester->oam[cpu_ppu_tester->oam_addr]);
	ck_assert_uint_eq(expected_result, cpu_ppu_tester->oam_addr);
}

START_TEST (write_ppu_scroll_2005_scrolling_1st_write_clears_coarse_x)
{
	// 1st write to $2005 clears the coarseX bits (0x1F), setting fineX
	// and coarseX from the input
	cpu_ppu_tester->write_toggle = 0;
	*(cpu_ppu_tester->vram_tmp_addr) = 0xFFFF;
	write_ppu_reg(0x2005, 0x03, cpio_cpu);

	ck_assert_uint_eq((uint16_t) ~0x001F, *(cpu_ppu_tester->vram_tmp_addr));
}

START_TEST (write_ppu_scroll_2005_scrolling_1st_write_sets_coarse_x_and_fine_x)
{
	// 1st write to $2005 clears the coarseX bits (0x1F), setting fineX
	// and coarseX from the input
	cpu_ppu_tester->write_toggle = 0;
	*(cpu_ppu_tester->vram_tmp_addr) = 0x0000;
	write_ppu_reg(0x2005, 0xFF, cpio_cpu);

	ck_assert_uint_eq(0xFF & 0x07, *(cpu_ppu_tester->fine_x));
	ck_assert_uint_eq(0xFF >> 3, *(cpu_ppu_tester->vram_tmp_addr));
}

START_TEST (write_ppu_scroll_2005_scrolling_1st_write_toggles_write_bit)
{
	cpu_ppu_tester->write_toggle = 0;
	write_ppu_reg(0x2005, 0xFF, cpio_cpu);

	ck_assert_uint_eq(1, cpu_ppu_tester->write_toggle);
}

START_TEST (write_ppu_scroll_2005_scrolling_2nd_write_clears_coarse_y_and_fine_y)
{
	// 2nd write to $2005 clears the coarseY bits (0x73E0), setting fineY
	// and coarseX from the input
	cpu_ppu_tester->write_toggle = 1;
	*(cpu_ppu_tester->vram_tmp_addr) = 0xFFFF;
	write_ppu_reg(0x2005, 0x00, cpio_cpu);

	ck_assert_uint_eq((uint16_t) ~0x73E0, *(cpu_ppu_tester->vram_tmp_addr));
}

START_TEST (write_ppu_scroll_2005_scrolling_2nd_write_sets_coarse_y_and_fine_y)
{
	// 2nd write to $2005 clears the coarseY bits (0x73E0), setting fineY
	// and coarseX from the input
	cpu_ppu_tester->write_toggle = 1;
	*(cpu_ppu_tester->vram_tmp_addr) = 0x0001; // NN = 0, coarse x = 1
	uint8_t fine_y = 5;
	uint8_t coarse_y = 18;
	write_ppu_reg(0x2005, (coarse_y << 3) | fine_y, cpio_cpu);

	ck_assert_uint_eq(*(cpu_ppu_tester->vram_tmp_addr)
	                          , nametable_vram_address_from_scroll_offsets(0x2000
	                                                                      , fine_y
	                                                                      , 1, coarse_y));
}

START_TEST (write_ppu_scroll_2005_scrolling_2nd_write_toggles_write_bit)
{
	cpu_ppu_tester->write_toggle = 1;
	write_ppu_reg(0x2005, 0x0F, cpio_cpu);

	ck_assert_uint_eq(0, cpu_ppu_tester->write_toggle);
}

START_TEST (write_ppu_addr_2006_scrolling_1st_write_clears_upper_byte)
{
	// 1st write to $2006 clears the upper byte of the tmp vram address
	// and later set it too
	cpu_ppu_tester->write_toggle = 0;
	*(cpu_ppu_tester->vram_tmp_addr) = 0x7FFF; // 15th bit shouldn't be set
	write_ppu_reg(0x2006, 0x00, cpio_cpu);

	ck_assert_uint_eq(0x00FF, *(cpu_ppu_tester->vram_tmp_addr));
}

START_TEST (write_ppu_addr_2006_scrolling_1st_write_sets_upper_byte)
{
	// 1st write to $2006 sets upper byte of the tmp vram address,
	// setting the tmp vram address to be within 0x0000 to 0x3FFF (as it
	// keeps fine y to its lower two bits out of three, full range of fine y
	// gives an address range of 0x0xxx to 0x7xxx)
	cpu_ppu_tester->write_toggle = 0;
	*(cpu_ppu_tester->vram_tmp_addr) = 0x7FFF; // 15th bit shouldn't be set
	write_ppu_reg(0x2006, 0x29, cpio_cpu);

	ck_assert_uint_eq(0x29FF, *(cpu_ppu_tester->vram_tmp_addr));
}

START_TEST (write_ppu_addr_2006_scrolling_1st_write_toggles_write_bit)
{
	cpu_ppu_tester->write_toggle = 0;
	*(cpu_ppu_tester->vram_tmp_addr) = 0x7FFF; // 15th bit shouldn't be set
	write_ppu_reg(0x2006, 0x00, cpio_cpu);

	ck_assert_uint_eq(1, cpu_ppu_tester->write_toggle);
}

START_TEST (write_ppu_addr_2006_scrolling_2nd_write_clears_lower_byte)
{
	// 2nd write to $2006 clears the lower byte of the tmp vram address
	cpu_ppu_tester->write_toggle = 1;
	*(cpu_ppu_tester->vram_tmp_addr) = 0x7FFF; // 15th bit shouldn't be set
	write_ppu_reg(0x2006, 0x00, cpio_cpu);

	ck_assert_uint_eq(0x7F00, *(cpu_ppu_tester->vram_tmp_addr));
}

START_TEST (write_ppu_addr_2006_scrolling_2nd_write_sets_lower_byte)
{
	// 2nd write to $2006 sets the lower byte of the tmp vram address
	cpu_ppu_tester->write_toggle = 1;
	*(cpu_ppu_tester->vram_tmp_addr) = 0x1FFF; // 15th bit shouldn't be set
	write_ppu_reg(0x2006, 0x1E, cpio_cpu);

	ck_assert_uint_eq(0x1F1E, *(cpu_ppu_tester->vram_tmp_addr));
}

START_TEST (write_ppu_addr_2006_scrolling_2nd_write_updates)
{
	// 2nd write to $2006 should update the vram address to what
	// the tmp vram address is (also setting the PPUADDR too)
	// while inverting the write toggle bit
	cpu_ppu_tester->write_toggle = 1;
	*(cpu_ppu_tester->vram_tmp_addr) = 0x1FFF; // 15th bit shouldn't be set
	write_ppu_reg(0x2006, 0x1E, cpio_cpu);

	ck_assert_uint_eq(0x1F1E, *(cpu_ppu_tester->vram_addr));
	ck_assert_uint_eq(0x1F1E, *(cpu_ppu_tester->vram_tmp_addr));
	ck_assert_uint_eq((uint8_t) 0x1F1E, cpu_ppu_tester->ppu_addr);
	ck_assert_uint_eq(0, cpu_ppu_tester->write_toggle);
}

START_TEST (write_ppu_data_2007_outside_of_rendering)
{
	// Writes to $2007 during rendering write to vram and should
	// increment the vram address by the vram address increment flag
	// in PPUCTRL (either 1 (across) or 32 (down))
	uint8_t reg_val[2] = {0x00, 0x04};  // clear bit (+1), set bit (+32)
	unsigned increment[2] = {1, 32};
	cpu_ppu_tester->ppu_ctrl = reg_val[_i];
	cpu_ppu_tester->ppu_rendering_period = false;
	// avoid writing to ROM pattern table, use nametables
	cpu_ppu_tester->vram->nametable_0 = &cpu_ppu_tester->vram->nametable_A;
	cpu_ppu_tester->vram->nametable_1 = &cpu_ppu_tester->vram->nametable_A;
	*(cpu_ppu_tester->vram_addr) = 0x2001;

	write_ppu_reg(0x2007, 0x6B, cpio_cpu);

	ck_assert_uint_eq(0x2001 + increment[_i], *(cpu_ppu_tester->vram_addr));
	ck_assert_uint_eq(0x6B, read_from_ppu_vram(cpu_ppu_tester->vram, 0x2001));
}

START_TEST (write_ppu_data_2007_during_rendering)
{
	// Writes to $2007 during rendering write to vram and should
	// perform a glitchy increment of the vram address
	cpu_ppu_tester->ppu_ctrl &= ~0x04;
	cpu_ppu_tester->ppu_mask = 0x10; // show sprites
	cpu_ppu_tester->ppu_rendering_period = true;
	// avoid writing to ROM pattern table, use nametables
	cpu_ppu_tester->vram->nametable_0 = &cpu_ppu_tester->vram->nametable_A;
	cpu_ppu_tester->vram->nametable_1 = &cpu_ppu_tester->vram->nametable_A;
	*(cpu_ppu_tester->vram_addr) = 0x2001;

	write_ppu_reg(0x2007, 0x6B, cpio_cpu);

	ck_assert_uint_ne(0x2001 + 1, *(cpu_ppu_tester->vram_addr));
	ck_assert_uint_ne(0x2001 + 32, *(cpu_ppu_tester->vram_addr));
	// glitchy increment is +1 to fine Y and +1 to coarse X (when not at a tile edge)
	ck_assert_uint_eq(0x2001 + 1 + 0x1000, *(cpu_ppu_tester->vram_addr));
	ck_assert_uint_eq(0x6B, read_from_ppu_vram(cpu_ppu_tester->vram, 0x2001));
}


START_TEST (ppu_ctrl_base_nt_address)
{
	// testing a previously unused function
	uint8_t reg_val[5] = {0x00, 0x01, 0x02, 0x03, 0xF8};
	// NN bits: 00 ($2000), 01 ($2400), 10 ($2800), 11 ($2C00)
	unsigned nt_address[5] = {0x2000, 0x2400, 0x2800, 0x2C00, 0x2000};
	cpu_ppu_tester->ppu_ctrl = reg_val[_i];

	ck_assert_uint_eq(nt_address[_i], ppu_base_nt_address(cpu_ppu_tester));
}

START_TEST (ppu_ctrl_vram_addr_inc_value)
{
	uint8_t reg_val[4] = {0x00, 0x04, 0xFF & ~0x04, 0xFF};
	unsigned increment[4] = {1, 32, 1, 32}; // clear bit (+1), set bit (+32)
	cpu_ppu_tester->ppu_ctrl = reg_val[_i];

	ck_assert_uint_eq(increment[_i], ppu_vram_addr_inc(cpu_ppu_tester));
}

START_TEST (ppu_ctrl_base_sprite_pattern_table_address)
{
	uint8_t reg_val[4] = {0x00, 0x08, 0xFF & ~0x08, 0xFF};
	// clear bit ($0000), set bit ($1000)
	uint16_t base_address[4] = {0x0000, 0x1000, 0x0000, 0x1000};
	cpu_ppu_tester->ppu_ctrl = reg_val[_i];

	ck_assert_uint_eq(base_address[_i], ppu_sprite_pattern_table_addr(cpu_ppu_tester));
}

START_TEST (ppu_ctrl_base_pt_address)
{
	uint8_t reg_val[4] = {0x00, 0x10, 0xFF & ~0x10, 0xFF};
	// clear bit ($0000), set bit ($1000)
	uint16_t pt_address[4] = {0x0000, 0x1000, 0x0000, 0x1000};
	cpu_ppu_tester->ppu_ctrl = reg_val[_i];

	ck_assert_uint_eq(pt_address[_i], ppu_base_pt_address(cpu_ppu_tester));
}

START_TEST (ppu_ctrl_sprite_height)
{
	uint8_t reg_val[4] = {0x00, 0x20, 0xFF & ~0x20, 0xFF};
	unsigned height[4] = {8, 16, 8, 16}; // clear bit (8), set bit (16)
	cpu_ppu_tester->ppu_ctrl = reg_val[_i];

	ck_assert_uint_eq(height[_i], ppu_sprite_height(cpu_ppu_tester));
}

START_TEST (ppu_ctrl_enable_nmi)
{
	uint8_t reg_val[4] = {0x00, 0x80, 0xFF & ~0x80, 0xFF};
	bool enable_nmi[4] = {false, true, false, true}; // clear bit (false), set bit (true)
	cpu_ppu_tester->ppu_ctrl = reg_val[_i];

	ck_assert(enable_nmi[_i] == ppu_ctrl_gen_nmi_bit_set(cpu_ppu_tester));
}

START_TEST (ppu_mask_enable_greyscale)
{
	uint8_t reg_val[4] = {0x00, 0x01, 0xFF & ~0x01, 0xFF};
	bool result[4] = {false, true, false, true}; // clear bit (false), set bit (true)
	cpu_ppu_tester->ppu_mask = reg_val[_i];

	ck_assert(result[_i] == ppu_show_greyscale(cpu_ppu_tester));
}

START_TEST (ppu_mask_hide_leftmost_8_pixels_background)
{
	uint8_t reg_val[4] = {0x00, 0x02, 0xEF & ~0x02, 0xFF};
	bool result[4] = {true, false, true, false}; // clear bit (true), set bit (false)
	cpu_ppu_tester->ppu_mask = reg_val[_i];

	ck_assert(result[_i] == ppu_mask_left_8px_bg(cpu_ppu_tester));
}

START_TEST (ppu_mask_hide_leftmost_8_pixels_sprite)
{
	uint8_t reg_val[4] = {0x00, 0x04, 0x0F & ~0x04, 0xFF};
	bool result[4] = {true, false, true, false}; // clear bit (true), set bit (false)
	cpu_ppu_tester->ppu_mask = reg_val[_i];

	ck_assert(result[_i] == ppu_mask_left_8px_sprite(cpu_ppu_tester));
}

START_TEST (ppu_mask_show_background)
{
	uint8_t reg_val[4] = {0x00, 0x08, 0xAF & ~0x08, 0xFF};
	bool result[4] = {false, true, false, true}; // clear bit (false), set bit (true)
	cpu_ppu_tester->ppu_mask = reg_val[_i];

	ck_assert(result[_i] == ppu_show_bg(cpu_ppu_tester));
}

START_TEST (ppu_mask_show_sprite)
{
	uint8_t reg_val[4] = {0x00, 0x10, 0xBF & ~0x10, 0xFF};
	bool result[4] = {false, true, false, true}; // clear bit (false), set bit (true)
	cpu_ppu_tester->ppu_mask = reg_val[_i];

	ck_assert(result[_i] == ppu_show_sprite(cpu_ppu_tester));
}

START_TEST (ppu_mask_rendering_enabled)
{
	uint8_t reg_val[8] = {0x00, 0x18, 0x10, 0x08, 0xFF & ~0x18, 0xFF & ~0x10, 0xFF & ~0x08, 0xFF};
	// false if no bits set, otherwise true
	bool result[8] = {false, true, true, true, false, true, true, true};
	cpu_ppu_tester->ppu_mask = reg_val[_i];

	ck_assert(result[_i] == ppu_mask_bg_or_sprite_enabled(cpu_ppu_tester));
}

START_TEST (ppu_status_sprite_overflow)
{
	uint8_t reg_val[4] = {0x00, 0x20, 0xFF & ~0x20, 0xFF};
	bool result[4] = {false, true, false, true}; // clear bit (false), set bit (true)
	cpu_ppu_tester->ppu_status = reg_val[_i];

	ck_assert(result[_i] == sprite_overflow_occured(cpu_ppu_tester));
}

START_TEST (ppu_status_vblank_period)
{
	uint8_t reg_val[4] = {0x00, 0x80, 0xFF & ~0x80, 0xFF};
	bool vblank_period[4] = {false, true, false, true}; // clear bit (false), set bit (true)
	cpu_ppu_tester->ppu_status = reg_val[_i];

	ck_assert(vblank_period[_i] == ppu_status_vblank_bit_set(cpu_ppu_tester));
}


Suite* cpu_ppu_master_suite(void)
{
	Suite* s;

	s = suite_create("Cpu/Ppu Register Tests");

	return s;
}

Suite* ppu_registers_read_write_suite(void)
{
	Suite* s;
	TCase* tc_ppu_register_reads;
	TCase* tc_ppu_register_writes;

	s = suite_create("Ppu Registers Read/Write Tests");
	tc_ppu_register_reads = tcase_create("Ppu Register Reads");
	tcase_add_checked_fixture(tc_ppu_register_reads, setup, teardown);
	tcase_add_test(tc_ppu_register_reads, read_ppu_status_2002_resets);
	tcase_add_test(tc_ppu_register_reads, read_ppu_status_2002_return_value);
	tcase_add_test(tc_ppu_register_reads, read_oam_data_2004_outside_of_rendering);
	tcase_add_test(tc_ppu_register_reads, read_oam_data_2004_during_rendering);
	tcase_add_test(tc_ppu_register_reads, read_oam_data_2004_unused_attribute_bits_as_clear);
	tcase_add_test(tc_ppu_register_reads, read_ppu_data_2007_non_palette_buffering);
	tcase_add_test(tc_ppu_register_reads, read_ppu_data_2007_palette_buffering);
	tcase_add_loop_test(tc_ppu_register_reads, read_ppu_data_2007_outside_of_rendering, 0, 2);
	tcase_add_test(tc_ppu_register_reads, read_ppu_data_2007_during_rendering);
	suite_add_tcase(s, tc_ppu_register_reads);
	tc_ppu_register_writes = tcase_create("Ppu Register Writes");
	tcase_add_checked_fixture(tc_ppu_register_writes, setup, teardown);
	tcase_add_test(tc_ppu_register_writes, write_ppu_ctrl_2000_scrolling_clears_specific_bits);
	tcase_add_test(tc_ppu_register_writes, write_ppu_ctrl_2000_scrolling_sets_specific_bits);
	tcase_add_test(tc_ppu_register_writes, write_oam_addr_2003_sets_oam_address);
	tcase_add_test(tc_ppu_register_writes, write_oam_data_2004_outside_rendering_period);
	tcase_add_test(tc_ppu_register_writes, write_oam_data_2004_during_rendering_period);
	tcase_add_test(tc_ppu_register_writes, write_ppu_scroll_2005_scrolling_1st_write_clears_coarse_x);
	tcase_add_test(tc_ppu_register_writes, write_ppu_scroll_2005_scrolling_1st_write_sets_coarse_x_and_fine_x);
	tcase_add_test(tc_ppu_register_writes, write_ppu_scroll_2005_scrolling_1st_write_toggles_write_bit);
	tcase_add_test(tc_ppu_register_writes, write_ppu_scroll_2005_scrolling_2nd_write_clears_coarse_y_and_fine_y);
	tcase_add_test(tc_ppu_register_writes, write_ppu_scroll_2005_scrolling_2nd_write_toggles_write_bit);
	tcase_add_test(tc_ppu_register_writes, write_ppu_scroll_2005_scrolling_2nd_write_sets_coarse_y_and_fine_y);
	tcase_add_test(tc_ppu_register_writes, write_ppu_addr_2006_scrolling_1st_write_clears_upper_byte);
	tcase_add_test(tc_ppu_register_writes, write_ppu_addr_2006_scrolling_1st_write_sets_upper_byte);
	tcase_add_test(tc_ppu_register_writes, write_ppu_addr_2006_scrolling_1st_write_toggles_write_bit);
	tcase_add_test(tc_ppu_register_writes, write_ppu_addr_2006_scrolling_2nd_write_clears_lower_byte);
	tcase_add_test(tc_ppu_register_writes, write_ppu_addr_2006_scrolling_2nd_write_sets_lower_byte);
	tcase_add_test(tc_ppu_register_writes, write_ppu_addr_2006_scrolling_2nd_write_updates);
	tcase_add_loop_test(tc_ppu_register_writes, write_ppu_data_2007_outside_of_rendering, 0, 2);
	tcase_add_test(tc_ppu_register_writes, write_ppu_data_2007_during_rendering);
	suite_add_tcase(s, tc_ppu_register_writes);

	return s;
}

Suite* ppu_registers_flags_suite(void)
{
	Suite* s;
	TCase* tc_ppu_register_flags;

	s = suite_create("Ppu Registers Flags Tests");
	tc_ppu_register_flags = tcase_create("Ppu Register Flag Functions");
	tcase_add_checked_fixture(tc_ppu_register_flags, setup, teardown);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_ctrl_base_nt_address, 0, 5);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_ctrl_vram_addr_inc_value, 0, 4);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_ctrl_base_sprite_pattern_table_address, 0, 4);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_ctrl_base_pt_address, 0, 4);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_ctrl_sprite_height, 0, 4);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_ctrl_enable_nmi, 0, 4);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_mask_enable_greyscale, 0, 4);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_mask_hide_leftmost_8_pixels_background, 0, 4);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_mask_hide_leftmost_8_pixels_sprite, 0, 4);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_mask_show_background, 0, 4);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_mask_show_sprite, 0, 4);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_mask_rendering_enabled, 0, 8);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_status_sprite_overflow, 0, 4);
	tcase_add_loop_test(tc_ppu_register_flags, ppu_status_vblank_period, 0, 4);
	suite_add_tcase(s, tc_ppu_register_flags);

	return s;
}
