#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "cpu_ppu_interface.h"
#include "cpu.h" // for cpu struct
#include "ppu.h" // needed for vram functions


CpuPpuShare* cpu_ppu_io_allocator(void)
{
	CpuPpuShare* cpu_ppu_io = malloc(sizeof(CpuPpuShare));
	if (!cpu_ppu_io) {
		fprintf(stderr, "Failed to allocate enough memory for the shared Cpu/Ppu IO struct\n");
	}

	return cpu_ppu_io; // either returns valid or NULL pointer
}

int cpu_ppu_io_init(CpuPpuShare* cpu_ppu_io)
{
	int return_code = -1;

	// Initialise registers
	cpu_ppu_io->ppu_ctrl = 0;
	cpu_ppu_io->ppu_mask = 0;
	cpu_ppu_io->ppu_status = 0;
	cpu_ppu_io->oam_addr = 0;
	cpu_ppu_io->oam_data = 0;
	cpu_ppu_io->ppu_scroll = 0;
	cpu_ppu_io->ppu_addr = 0;
	cpu_ppu_io->ppu_data = 0;
	cpu_ppu_io->oam_dma = 0;

	// Initialise internal flags
	cpu_ppu_io->nmi_pending = false;
	cpu_ppu_io->dma_pending = false;
	cpu_ppu_io->suppress_nmi_flag = false;
	cpu_ppu_io->ignore_nmi = false;
	cpu_ppu_io->nmi_lookahead = false;

	cpu_ppu_io->nmi_cycles_left = 7;

	cpu_ppu_io->buffer_write = false;
	cpu_ppu_io->buffer_address = 0;
	cpu_ppu_io->buffer_counter = 0;
	cpu_ppu_io->buffer_value = 0;

	// Ppu related stuff
	cpu_ppu_io->write_debug = false;
	cpu_ppu_io->clear_status = false;
	cpu_ppu_io->bg_early_disable_mask = false;
	cpu_ppu_io->bg_early_enable_mask = false;
	cpu_ppu_io->ppu_rendering_period = false;

	return_code = 0;

	return return_code;
}

void map_ppu_data_to_cpu_ppu_io(CpuPpuShare* cpu_ppu_io, Ppu2C02* ppu)
{
	cpu_ppu_io->oam = &ppu->oam[0];
	cpu_ppu_io->vram = &ppu->vram;
	cpu_ppu_io->vram_addr = &ppu->vram_addr;
	cpu_ppu_io->vram_tmp_addr = &ppu->vram_tmp_addr;
	cpu_ppu_io->fine_x = &ppu->fine_x;
	cpu_ppu_io->nametable_mirroring = &ppu->nametable_mirroring;
}


/* common masks, bit set and clear operations for ppu registers
 *
 * For ppu calling functions: arg 1 == p->cpu_ppu_io
 * For cpu calling functions: arg 1 == cpu->cpu_ppu_io
 */
bool ppu_status_vblank_bit_set(const CpuPpuShare* cpu_ppu_io)
{
	return ((cpu_ppu_io->ppu_status & 0x80) ? 1 : 0);
}

bool ppu_ctrl_gen_nmi_bit_set(const CpuPpuShare* cpu_ppu_io)
{
	return ((cpu_ppu_io->ppu_ctrl & 0x80) ? 1 : 0);
}

void clear_ppu_status_vblank_bit(CpuPpuShare* cpu_ppu_io)
{
	cpu_ppu_io->ppu_status &= ~0x80;
}

void set_ppu_status_vblank_bit(CpuPpuShare* cpu_ppu_io)
{
	cpu_ppu_io->ppu_status |= 0x80;
}

bool ppu_mask_bg_or_sprite_enabled(const CpuPpuShare* cpu_ppu_io)
{
	return ((cpu_ppu_io->ppu_mask & 0x18) ? 1 : 0);
}

// Called from CPU
void cpu_writes_to_vram(uint8_t data, unsigned chr_ram_size, CpuPpuShare* cpu_ppu_io)
{
	// keep address in valid range
	uint16_t addr = *(cpu_ppu_io->vram_addr) & 0x3FFF;

	// Write to pattern tables (if using CHR RAM), otherwise we are using CHR ROM
	if ((chr_ram_size) && (addr <= 0x1FFF)) {
		write_to_ppu_vram(cpu_ppu_io->vram, addr, data);
	}

	// Handle nametable and palette writes
	if (addr >= 0x2000) {
		write_to_ppu_vram(cpu_ppu_io->vram, addr, data);
		if (addr >= 0x3F00) {
			if ((addr & 0x0F) == 0) {
				// If bg palette #0, colour #0 mirror up to sprite's 0th palette and colour
				write_to_ppu_vram(cpu_ppu_io->vram, addr + 0x10, data);
			} else if ((addr & 0x1F) == 0x10) {
				// If sprite palette #0, colour #0 mirror down to bg's 0th palette and colour
				write_to_ppu_vram(cpu_ppu_io->vram, addr - 0x10, data);
			}
		}
	}
}

/* Read Functions */

/* Reading $2002 will always clear the write toggle
 * It will also clear VBLANK flag bit (bit 7)
 * The value read-back may have the VBLANK flag set or not (when close to NMI)
 */
void read_2002(CpuPpuShare* cpu_ppu_io)
{
	cpu_ppu_io->return_value = cpu_ppu_io->ppu_status;
	clear_ppu_status_vblank_bit(cpu_ppu_io);
	cpu_ppu_io->write_toggle = false; // Clear latch used by PPUSCROLL & PPUADDR
	cpu_ppu_io->suppress_nmi_flag = true;

	if (cpu_ppu_io->clear_status) {
		cpu_ppu_io->return_value &= ~0x80;
		cpu_ppu_io->clear_status = false;
	}
}

void read_2004(CpuPpuShare* cpu_ppu_io)
{
	cpu_ppu_io->return_value = cpu_ppu_io->oam[cpu_ppu_io->oam_addr];
	if ((cpu_ppu_io->oam_addr & 0x03) == 0x02) {
		// if reading back attribute bytes, return 0 for unused bitss
		cpu_ppu_io->return_value = cpu_ppu_io->oam[cpu_ppu_io->oam_addr] & 0xE3;
	}

	if (!cpu_ppu_io->ppu_rendering_period
	   || !ppu_mask_bg_or_sprite_enabled(cpu_ppu_io)) {
		// don't increment oam_addr outside of ppu rendering or if bg and sprite rendering is disabled
		return;
	}
	++cpu_ppu_io->oam_addr;
}

/* Vram read data register:
 *
 * Reads to non-palette data return an internal read buffer
 * This buffer is only updated when reading $2007
 * Palette data isn't buffered however
 * It will update the internal buffer in an unusual way (see below)
 *
 * Accessing this register will cause the vram address to increment outside of rendering
 * via the PPUCTRL register
 * Reads while rendering cause the vram address is incremented in an odd way by
 * simultaneously updating horizontal (coarse X) and vertical (Y) scrolling
 */
void read_2007(CpuPpuShare* cpu_ppu_io)
{
	uint16_t addr = *(cpu_ppu_io->vram_addr) & 0x3FFF;

	cpu_ppu_io->return_value = cpu_ppu_io->buffer_2007;
	cpu_ppu_io->buffer_2007 = read_from_ppu_vram(cpu_ppu_io->vram, addr);

	if (addr >= 0x3F00) {
		cpu_ppu_io->return_value = read_from_ppu_vram(cpu_ppu_io->vram, addr);
		// buffer data would be if the nametable mirroring kept going
		// use minus 0x1000 to get out of palette addresses and read from nametable mirrors
		cpu_ppu_io->buffer_2007 = read_from_ppu_vram(cpu_ppu_io->vram, addr - 0x1000);
	}

	if (cpu_ppu_io->ppu_rendering_period && ppu_mask_bg_or_sprite_enabled(cpu_ppu_io)) {
		inc_vert_scroll(cpu_ppu_io);
		inc_horz_scroll(cpu_ppu_io);
	} else {
		*(cpu_ppu_io->vram_addr) += ppu_vram_addr_inc(cpu_ppu_io);
	}
}

/* Write Functions */

/* PPUCTRL writes (for scrolling), previous function that calls this
 * updates the PPUCTRL bits/flags
 *
 * vram_addr byte layout: 0yyy NNYY YYYX XXXX (and vram_tmp_addr byte layout)
 *
 * For scrolling clear NN bits and later set them
 * Same NN bits get written to PPUCTLR's first two bits earlier
 */
void write_2000(const uint8_t data, CpuPpuShare* cpu_ppu_io)
{
	*(cpu_ppu_io->vram_tmp_addr) &= ~0x0C00;
	*(cpu_ppu_io->vram_tmp_addr) |= (data & 0x03) << 10;
}

/* Set OAMADDR: The value of OAMADDR when sprite_evaluation() is first called
 * determines the first sprite to be checked (this is the sprite 0)
 */
inline void write_2003(const uint8_t data, CpuPpuShare* cpu_ppu_io)
{
	cpu_ppu_io->oam_addr = data;
}

/* Write OAMDATA to previously set OAMADDR (through a $2003 write)
 * OAMADDR is incremented after the write
 */
void write_2004(const uint8_t data, CpuPpuShare* cpu_ppu_io)
{
	// If rendering is enabled (either bg or sprite) during
	// scanlines 0-239 and pre-render scanline
	// OAM writes are disabled, but glitchy OAM increment occurs
	if (ppu_mask_bg_or_sprite_enabled(cpu_ppu_io)
	    && cpu_ppu_io->ppu_rendering_period) {
		cpu_ppu_io->oam_addr += 4;  // only increment high 6 bits (same as +4)
		return;
	}
	cpu_ppu_io->oam[cpu_ppu_io->oam_addr] = data;
	++cpu_ppu_io->oam_addr;
}

/* Update PPUSCROLL value (scroll position), needs two writes for a complete update
 * The scroll position being CoarseX, fine_x, CoarseY and fine_y
 *
 * vram_addr byte layout: 0yyy NNYY YYYX XXXX (and vram_tmp_addr byte layout)
 * Notice first write clears CoarseX (XX bits from vram_tmp)
 * Whilst setting fine_x and CoarseX (by shifting out fine_x from the write value)
 *
 * The second write clears fine_y and CoarseY bits (yy and YYY bits)
 * Incoming data is: YYYY Yyyy
 * So we bit mask w/ 0x07 to get fine_y and we mask w/ 0xF8 to get CoarseY
 * We then move those bits to the correct positions through bit shifts
 *
 * First and second writes are kept track of via a write toggle flag, each write will
 * toggle the flag
 */
void write_2005(const uint8_t data, CpuPpuShare* cpu_ppu_io)
{
	// Valid address = 0x0000 to 0x3FFF
	if (!cpu_ppu_io->write_toggle) {
		// First Write
		*(cpu_ppu_io->vram_tmp_addr) &= ~0x001F;
		*(cpu_ppu_io->vram_tmp_addr) |= (data >> 3);
		*(cpu_ppu_io->fine_x) = data & 0x07;
	} else {
		// Second Write
		*(cpu_ppu_io->vram_tmp_addr) &= ~0x73E0;
		*(cpu_ppu_io->vram_tmp_addr) |= ((data & 0xF8) << 2) | ((data & 0x07) << 12);
		cpu_ppu_io->ppu_scroll = *(cpu_ppu_io->vram_tmp_addr);
	}
	cpu_ppu_io->write_toggle = !cpu_ppu_io->write_toggle;
}


/* Update PPUADDR value, needs two writes for a complete update
 *
 * vram_addr byte layout: 0yyy NNYY YYYX XXXX (and vram_tmp_addr byte layout)
 * Notice first write clears the higher byte
 * Mask incoming data w/ 0x3F to discard the upper y bit (as this keeps the
 * address within 0x0000 to 0x3FFF, and the msb bit is always 0, vram_addr is
 * 15 bits wide)
 *
 * The second write clears the lower byte and later sets it too
 * Vram_addr is updated from vram_tmp_addr
 *
 * First and second writes are kept track of via a write toggle flag, each write will
 * toggle the flag
 */
void write_2006(const uint8_t data, CpuPpuShare* cpu_ppu_io)
{
	// Valid address = 0x0000 to 0x3FFF
	if (!cpu_ppu_io->write_toggle) {
		*(cpu_ppu_io->vram_tmp_addr) &= ~0x7F00;
		*(cpu_ppu_io->vram_tmp_addr) |= (uint16_t) ((data & 0x3F) << 8);
	} else {
		*(cpu_ppu_io->vram_tmp_addr) &= ~0x00FF;
		*(cpu_ppu_io->vram_tmp_addr) |= data;
		*(cpu_ppu_io->vram_addr) = *(cpu_ppu_io->vram_tmp_addr);
		cpu_ppu_io->ppu_addr = *(cpu_ppu_io->vram_tmp_addr);
	}
	cpu_ppu_io->write_toggle = !cpu_ppu_io->write_toggle;
}


/* Write to PPU VRAM through PPUDATA register
 *
 * Writing to this register will cause the vram address to increment outside of rendering
 * via the PPUCTRL register
 * Writing while rendering causes the vram address is incremented in an odd way
 * by simultaneously updating horizontal (coarse X) and vertical (Y) scrolling
 */
void write_2007(const uint8_t data, unsigned chr_ram_size, CpuPpuShare* cpu_ppu_io)
{
	cpu_writes_to_vram(data, chr_ram_size, cpu_ppu_io);

	if (cpu_ppu_io->ppu_rendering_period && ppu_mask_bg_or_sprite_enabled(cpu_ppu_io)) {
		inc_vert_scroll(cpu_ppu_io);
		inc_horz_scroll(cpu_ppu_io);
	} else {
		*(cpu_ppu_io->vram_addr) += ppu_vram_addr_inc(cpu_ppu_io);
	}
}


/* Set DMA flag so that when the CPU is clocked it is triggered
 */
void write_4014(const uint8_t data, Cpu6502* cpu)
{
	cpu->cpu_ppu_io->dma_pending = true;
	cpu->base_addr = data;
}

/**
 * PPU_CTRL
 */
// use CPU to access shared CPU/PPU space as this is needed in CPU writes
uint8_t ppu_vram_addr_inc(const CpuPpuShare* cpu_ppu_io)
{
	if (!(cpu_ppu_io->ppu_ctrl & 0x04)) {
		return 1;
	} else {
		return 32;
	}
}

uint16_t ppu_base_nt_address(const CpuPpuShare* cpu_ppu_io)
{
	switch(cpu_ppu_io->ppu_ctrl & 0x03) {
	case 0:
		return 0x2000;
	case 1:
		return 0x2400;
	case 2:
		return 0x2800;
	case 3:
		return 0x2C00;
	default:
		return 0x2000;
	}
}


uint16_t ppu_base_pt_address(const CpuPpuShare* cpu_ppu_io)
{
	if ((cpu_ppu_io->ppu_ctrl >> 4) & 0x01) {
		return 0x1000;
	} else {
		return 0x0000;
	}
}

uint16_t ppu_sprite_pattern_table_addr(const CpuPpuShare* cpu_ppu_io)
{
	if ((cpu_ppu_io->ppu_ctrl >> 3) & 0x01) {
		return 0x1000;
	} else {
		return 0x0000;
	}
}

uint8_t ppu_sprite_height(const CpuPpuShare* cpu_ppu_io)
{
	if ((cpu_ppu_io->ppu_ctrl >> 5) & 0x01) {
		return 16; // 8 x 16 sprites
	} else {
		return 8; // 8 x 8 sprites
	}
}

/**
 * PPU_MASK
 */

bool ppu_show_bg(const CpuPpuShare* cpu_ppu_io)
{
	if (cpu_ppu_io->ppu_mask & 0x08) {
		return true;
	} else {
		return false;
	}
}


bool ppu_show_sprite(const CpuPpuShare* cpu_ppu_io)
{
	if (cpu_ppu_io->ppu_mask & 0x10) {
		return true;
	} else {
		return false;
	}
}

bool ppu_mask_left_8px_bg(const CpuPpuShare* cpu_ppu_io)
{
	if (cpu_ppu_io->ppu_mask & 0x02) {
		return false;
	} else {
		return true;
	}
}

bool ppu_mask_left_8px_sprite(const CpuPpuShare* cpu_ppu_io)
{
	if (cpu_ppu_io->ppu_mask & 0x04) {
		return false;
	} else {
		return true;
	}
}

bool ppu_show_greyscale(const CpuPpuShare* cpu_ppu_io)
{
	if (cpu_ppu_io->ppu_mask & 0x01) {
		return true;
	} else {
		return false;
	}
}

/**
 * PPU_STATUS
 */

bool sprite_overflow_occured(const CpuPpuShare* cpu_ppu_io)
{
	if (cpu_ppu_io->ppu_status & 0x20) {
		return true;
	} else {
		return false;
	}
}
