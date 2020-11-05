#include "ppu.h"
#include "gui.h"

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

/* Reverse bits lookup table for an 8 bit number */
static const uint8_t reverse_bits[256] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

/* RGB values of NES colour palette -format: 0xRRGGBB */
// NES classic (FBX) from firebrandx.com
static const uint32_t palette[64] = {
	0x616161, 0x000088, 0x1F0D99, 0x371379, 0x561260, 0x5D0010, 0x520E00, 0x3A2308,
	0x21350C, 0x0D410E, 0x174417, 0x003A1F, 0x002F57, 0x000000, 0x000000, 0x000000,
	0xAAAAAA, 0x0D4DC4, 0x4B24DE, 0x6912CF, 0x9014AD, 0x9D1C48, 0x923404, 0x735005,
	0x5D6913, 0x167A11, 0x138008, 0x127649, 0x1C6691, 0x000000, 0x000000, 0x000000,
	0xFCFCFC, 0x639AFC, 0x8A7EFC, 0xB06AFC, 0xDD6FF2, 0xE771AB, 0xE38658, 0xCC9E22,
	0xA8B100, 0x72C100, 0x5ACD4E, 0x34C28E, 0x4FBECE, 0x424242, 0x000000, 0x000000,
	0xFCFCFC, 0xBED4FC, 0xCACAFC, 0xD9C4FC, 0xECC1FC, 0xFAC3E7, 0xF7CEC3, 0xE2CDA7,
	0xDADB9C, 0xC8E39E, 0xBFE5B8, 0xB2EBC8, 0xB7E5EB, 0xACACAC, 0x000000, 0x000000
};

#if 0
// Smooth (FBX) from firebrandx.com
static const uint32_t palette[64] = {
	0x6A6D6A, 0x001380, 0x1E008A, 0x39007A, 0x550056, 0x5A0018, 0x4F1000, 0x3D1C00,
	0x253200, 0x003D00, 0x004000, 0x003924, 0x002E55, 0x000000, 0x000000, 0x000000,
	0xB9BCB9, 0x1850C7, 0x4B30E3, 0x7322D6, 0x951FA9, 0x9D285C, 0x983700, 0x7F4C00,
	0x5E6400, 0x227700, 0x027E02, 0x007645, 0x006E8A, 0x000000, 0x000000, 0x000000,
	0xFFFFFF, 0x68A6FF, 0x8C9CFF, 0xB586FF, 0xD975FD, 0xE377B9, 0xE58D68, 0xD49D29,
	0xB3AF0C, 0x7BC211, 0x55CA47, 0x46CB81, 0x47C1C5, 0x4A4D4A, 0x000000, 0x000000,
	0xFFFFFF, 0xCCEAFF, 0xDDDEFF, 0xECDAFF, 0xF8D7FE, 0xFCD6F5, 0xFDDBCF, 0xF9E7B5,
	0xF1F0AA, 0xDAFAA9, 0xC9FFBC, 0xC3FBD7, 0xC4F6F6, 0xBEC1BE, 0x000000, 0x000000
};
#endif

uint32_t pixels[256 * 240];


Ppu2A03* ppu_init(CpuPpuShare* cp)
{
	Ppu2A03* ppu = malloc(sizeof(Ppu2A03));
	if (!ppu) {
		fprintf(stderr, "Failed to allocate enough memory for PPU\n");
		return ppu;
	}
	ppu->cpu_ppu_io = cp;
	ppu->cpu_ppu_io->vram = &(ppu->vram[0]); // used so CPU can access PPU VRAM w/o needing the PPU struct
	ppu->cpu_ppu_io->oam = &(ppu->oam[0]); // used so CPU can access PPU VRAM w/o needing the PPU struct
	ppu->cpu_ppu_io->buffer_2007 = 0;
	ppu->cpu_ppu_io->vram_addr = &ppu->vram_addr;
	ppu->cpu_ppu_io->vram_tmp_addr = &ppu->vram_tmp_addr;
	ppu->cpu_ppu_io->mirroring = &ppu->mirroring;
	ppu->cpu_ppu_io->fine_x = &ppu->fine_x;
	ppu->cpu_ppu_io->write_debug = false;

	ppu->reset_1 = false;
	ppu->reset_2 = false;
	ppu->cycle = 30;
	ppu->scanline = 0;
	ppu->oam_read_buffer = 0;

	/* Set PPU Latches and shift reg to 0 */
	ppu->pt_lo_shift_reg = 0;
	ppu->pt_hi_shift_reg = 0;
	ppu->pt_lo_latch = 0;
	ppu->pt_hi_latch = 0;

	/* Sprite stuff */
	ppu->sprites_found = 0;
	ppu->sprite_index = 0; // Fetch sprite #0 1st
	ppu->stop_early = false;
	ppu->sprite_zero_hit = false;
	ppu->sprite_zero_scanline = 600;
	ppu->sprite_zero_scanline_tmp = 600;
	ppu->hit_scanline = 600; // Impossible values
	ppu->hit_cycle = 600; // Impossible values

	// Zero out arrays
	memset(ppu->vram, 0, sizeof(ppu->vram));
	memset(ppu->oam, 0, sizeof(ppu->oam));
	memset(ppu->scanline_oam, 0, sizeof(ppu->scanline_oam));
	memset(ppu->sprite_x_counter, 0, sizeof(ppu->sprite_x_counter));
	memset(ppu->sprite_at_latches, 0, sizeof(ppu->sprite_at_latches));
	memset(ppu->sprite_pt_lo_shift_reg, 0, sizeof(ppu->sprite_pt_lo_shift_reg));
	memset(ppu->sprite_pt_hi_shift_reg, 0, sizeof(ppu->sprite_pt_hi_shift_reg));

	/* NTSC */
	ppu->nmi_start = 241;
	return ppu;
}

// Reset/Warm-up function, clears and sets VBL flag at certain CPU cycles
void ppu_reset(int start, Ppu2A03* p, Cpu6502* cpu)
{
	// remove p->reset_1 and 2 and isntead use a static variable
	if (start && !p->reset_1 && !p->reset_2) {
		p->cpu_ppu_io->ppu_status &= ~(0x80);  // clear VBL flag if set
		p->reset_1 = true;
	} else if (p->reset_1 && (cpu->cycle >= 27383)) {
		p->cpu_ppu_io->ppu_status |= 0x80;
		p->reset_1 = false;
		p->reset_2 = true;
	} else if (p->reset_2 && (cpu->cycle >= 57164)) {
		p->cpu_ppu_io->ppu_status |= 0x80;
		p->reset_2 = false;  // changed from true
	}
}

void append_ppu_info(Ppu2A03* ppu)
{
	printf(" PPU_CYC: %.3" PRIu16, ppu->old_cycle);
	printf(" SL: %" PRIu32 "\n", ppu->scanline);
}

void debug_ppu_regs(Cpu6502* cpu)
{
	printf("2000: %.2X\n", read_from_cpu(cpu, 0x2000));
	printf("2001: %.2X\n", read_from_cpu(cpu, 0x2001));
	printf("2002: %.2X\n", read_from_cpu(cpu, 0x2002));
	printf("2003: %.2X\n", read_from_cpu(cpu, 0x2003));
	printf("2004: %.2X\n", read_from_cpu(cpu, 0x2004));
	printf("2005: %.2X\n", read_from_cpu(cpu, 0x2005));
	printf("2006: %.2X\n", read_from_cpu(cpu, 0x2006));
	printf("2007: %.2X\n", read_from_cpu(cpu, 0x2007));
	printf("3F00: %.2X\n", read_from_cpu(cpu, 0x3F00));
	printf("3F01: %.2X\n\n", read_from_cpu(cpu, 0x3F01));
}

void ppu_mem_16_byte_viewer(Ppu2A03* ppu, unsigned start_addr, unsigned total_rows)
{
	printf("\n##################### PPU MEM #######################\n");
	printf("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
	unsigned mem = start_addr;
	while (start_addr < total_rows) {
		printf("%.4X: ", start_addr << 4);
		for (int x = 0; x < 16; x++) {
			printf("%.2X ", ppu->vram[mem]);
			//printf("%.2X ", ppu->scanline_oam[mem]);
			//printf("%.2X ", ppu->oam[mem]);
			++mem;
		}
		printf("\n");
		++start_addr;
	}
}


// fix like above
void OAM_viewer(Ppu2A03* ppu, enum Memory ppu_mem)
{
	printf("\n##################### PPU OAM #######################\n");
	printf("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
	unsigned int addr = 0; // Byte count. 16 Bytes per line
	unsigned int mem = 0;  // Start address
	unsigned int addr_limit = 0; // Byte count. 16 Bytes per line
	if (ppu_mem == PRIMARY_OAM) {
		addr_limit = 16; // Byte count. 16 Bytes per line
	} else if (ppu_mem == SECONDARY_OAM) {
		addr_limit = 2;
	}
	while (addr < addr_limit) {
		printf("%.4X: ", addr << 4);
		for (int x = 0; x < 16; x++) {
			// need condirional here as well
			//printf("%.2X ", ppu->oam[mem]);
			printf("%.2X ", ppu->scanline_oam[mem]);
			++mem;
		}
		printf("\n");
		++addr;
	}
}

uint8_t read_ppu_reg(uint16_t addr, Cpu6502* cpu)
{
	uint8_t ret;
	switch (addr) {
	case (0x2002):
		/* PPU STATUS */
		read_2002(cpu);
		ret = cpu->cpu_ppu_io->return_value;
		break;
	case (0x2004):
		/* OAM Data (read & write) */
		ret = cpu->cpu_ppu_io->oam_data;
		break;
	case (0x2007):
		/* PPU DATA */
		read_2007(cpu);
		ret = cpu->cpu_ppu_io->return_value;
		break;
	}
	return ret;
}

/* CPU uses this function */
void write_ppu_reg(uint16_t addr, uint8_t data, Cpu6502* cpu)
{
	switch (addr) {
	case (0x2000):
		/* PPU CTRL */

		// Toggling NMI during VBlank generates a NMI
		if ((cpu->cpu_ppu_io->ppu_status & 0x80)
			&& !(cpu->cpu_ppu_io->ppu_ctrl & 0x80) && (data & 0x80)) {
			cpu->cpu_ppu_io->nmi_pending = true;
			cpu->delay_nmi = true;
		}

		cpu->cpu_ppu_io->ppu_ctrl = data;
		write_2000(data, cpu);
		break;
	case (0x2001):
		/* PPU MASK */
		cpu->cpu_ppu_io->ppu_mask = data;
		break;
	case (0x2003):
		/* OAM ADDR */
		write_2003(data, cpu);
		break;
	case (0x2004):
		/* OAM Data (read & write) */
		cpu->cpu_ppu_io->oam_data = data;
		write_2004(data, cpu);
		break;
	case (0x2005):
		/* PPU SCROLL (write * 2) */
		write_2005(data, cpu);
		break;
	case (0x2006):
		/* PPU ADDR (write * 2) */
		write_2006(data, cpu);
		break;
	case (0x2007):
		/* PPU DATA */
		cpu->cpu_ppu_io->ppu_data = data;
		write_2007(data, cpu);
		break;
	case (0x4014):
		write_4014(data, cpu);
		break;
	}
}

// VRAM is written to by the CPU
void write_vram(uint8_t data, Cpu6502* cpu)
{
	uint16_t addr = *(cpu->cpu_ppu_io->vram_addr) & 0x3FFF;

	// Write to pattern tables (if using CHR RAM)
	if (cpu->cpu_mapper_io->chr->ram_size && addr <= 0x1FFF) {
		cpu->cpu_ppu_io->vram[addr] = data;
	}

	// Nametable mirroring
	if (addr >= 0x2000 && addr < 0x3000) {
		if (*(cpu->cpu_ppu_io->mirroring) == 0) {
			// Horiz mirroring
			if (addr < 0x2800) {
				// mask = 0x23FF, so that address is in the first nametable space 0x2000 to 0x23FF
				cpu->cpu_ppu_io->vram[addr & 0x23FF] = data;
				cpu->cpu_ppu_io->vram[(addr & 0x23FF) + 0x0400] = data;
			} else if (addr >= 0x2800) {
				// mask = 0x2BFF, so that address is in the third nametable space 0x2800 to 0x2BFF
				cpu->cpu_ppu_io->vram[addr & 0x2BFF] = data;
				cpu->cpu_ppu_io->vram[(addr & 0x2BFF) + 0x0400] = data;
			}
		} else if (*(cpu->cpu_ppu_io->mirroring) == 1) {
			// Vertical mirroring
			// mask = 0x27FF, so that address is in the first two nametable spaces 0x2000 to 0x27FF
			cpu->cpu_ppu_io->vram[addr & 0x27FF] = data;
			cpu->cpu_ppu_io->vram[(addr & 0x27FF) + 0x0800] = data;
		} else if (*(cpu->cpu_ppu_io->mirroring) == 4) {
			// 4 Screen
			cpu->cpu_ppu_io->vram[addr] = data; // Do nothing (no mirroring)
		} else if (*(cpu->cpu_ppu_io->mirroring) == 2 || *(cpu->cpu_ppu_io->mirroring) == 3) {
			// Single-screen mirroring
			// mask = 0x23FF, so that address is in the first nametable space 0x2000 to 0x23FF
			cpu->cpu_ppu_io->vram[addr & 0x23FF] = data;
			cpu->cpu_ppu_io->vram[(addr & 0x23FF) + 0x0400] = data;
			cpu->cpu_ppu_io->vram[(addr & 0x23FF) + 0x0800] = data;
			cpu->cpu_ppu_io->vram[(addr & 0x23FF) + 0x0C00] = data;
		}
	}

	/* Write to palettes */
	if (addr >= 0x3F00 && addr < 0x3F10) {
		cpu->cpu_ppu_io->vram[addr] = data;
		if ((addr & 0x03) == 0) {
			cpu->cpu_ppu_io->vram[addr + 0x10] = data; // If palette #0 mirror 4 palettes up
		}
	} else if (addr >= 0x3F10 && addr < 0x3F20) {
		cpu->cpu_ppu_io->vram[addr] = data;
		if ((addr & 0x03) == 0) {
			cpu->cpu_ppu_io->vram[addr - 0x10] = data; // If palette #0 mirror 4 palettes down
		}
	}
}

/* Read Functions */
void read_2002(Cpu6502* cpu)
{
	cpu->cpu_ppu_io->return_value = cpu->cpu_ppu_io->ppu_status;
	cpu->cpu_ppu_io->ppu_status &= ~0x80U;
	cpu->cpu_ppu_io->write_toggle = false; // Clear latch used by PPUSCROLL & PPUADDR
	cpu->cpu_ppu_io->suppress_nmi = true;
}

void read_2007(Cpu6502* cpu)
{
	uint16_t addr = *(cpu->cpu_ppu_io->vram_addr) & 0x3FFF;

	cpu->cpu_ppu_io->return_value = cpu->cpu_ppu_io->buffer_2007;
	cpu->cpu_ppu_io->buffer_2007 = cpu->cpu_ppu_io->vram[addr];

	if (addr >= 0x3F00) {
		cpu->cpu_ppu_io->return_value = cpu->cpu_ppu_io->vram[addr];
	}

	*(cpu->cpu_ppu_io->vram_addr) += ppu_vram_addr_inc(cpu);
}

/* Write Functions */
void write_2000(uint8_t data, Cpu6502* cpu)
{
	*(cpu->cpu_ppu_io->vram_tmp_addr) &= ~0x0C00; /* Clear bits to be set */
	*(cpu->cpu_ppu_io->vram_tmp_addr) |= (data & 0x03) << 10;
}

inline void write_2003(uint8_t data, Cpu6502* cpu)
{
	cpu->cpu_ppu_io->oam_addr = data;
}

void write_2004(uint8_t data, Cpu6502* cpu)
{
	cpu->cpu_ppu_io->oam[cpu->cpu_ppu_io->oam_addr] = data;
	++cpu->cpu_ppu_io->oam_addr;
}

void write_2005(uint8_t data, Cpu6502* cpu)
{
	// Valid address = 0x0000 to 0x3FFF
	if (!cpu->cpu_ppu_io->write_toggle) {
		// First Write
		*(cpu->cpu_ppu_io->vram_tmp_addr) &= ~0x001F; /* Clear bits that are to be set */
		*(cpu->cpu_ppu_io->vram_tmp_addr) |= (data >> 3);
		*(cpu->cpu_ppu_io->fine_x) = data & 0x07; /* same as data % 8 */
	} else {
		// Second Write
		*(cpu->cpu_ppu_io->vram_tmp_addr) &= ~0x73E0; /* Clear bits that are to be set */
		*(cpu->cpu_ppu_io->vram_tmp_addr) |= ((data & 0xF8) << 2) | ((data & 0x07) << 12);
		cpu->cpu_ppu_io->ppu_scroll = *(cpu->cpu_ppu_io->vram_tmp_addr);
	}
	cpu->cpu_ppu_io->write_toggle = !cpu->cpu_ppu_io->write_toggle;
}


void write_2006(uint8_t data, Cpu6502* cpu)
{
	// Valid address = 0x0000 to 0x3FFF
	if (!cpu->cpu_ppu_io->write_toggle) {
		*(cpu->cpu_ppu_io->vram_tmp_addr) &= ~0x7F00; /* Clear Higher Byte */
		*(cpu->cpu_ppu_io->vram_tmp_addr) |= (uint16_t) ((data & 0x3F) << 8); /* 14th bit should be clear */
	} else {
		*(cpu->cpu_ppu_io->vram_tmp_addr) &= ~0x00FF; /* Clear Lower Byte */
		*(cpu->cpu_ppu_io->vram_tmp_addr) |= data; /* Lower byte */
		*(cpu->cpu_ppu_io->vram_addr) = *(cpu->cpu_ppu_io->vram_tmp_addr);
		cpu->cpu_ppu_io->ppu_addr = *(cpu->cpu_ppu_io->vram_tmp_addr);
	}
	cpu->cpu_ppu_io->write_toggle = !cpu->cpu_ppu_io->write_toggle;
}


void write_2007(uint8_t data, Cpu6502* cpu)
{
	write_vram(data, cpu);
	*(cpu->cpu_ppu_io->vram_addr) += ppu_vram_addr_inc(cpu);
}


void write_4014(uint8_t data, Cpu6502* cpu)
{
	cpu->cpu_ppu_io->dma_pending = true;
	cpu->base_addr = data;
}

/**
 * PPU_CTRL
 */
// use CPU to access shared CPU/PPU space as this is needed in CPU writes
uint8_t ppu_vram_addr_inc(Cpu6502* cpu)
{
	if (!(cpu->cpu_ppu_io->ppu_ctrl & 0x04)) {
		return 1;
	} else {
		return 32;
	}
}

uint16_t ppu_base_nt_address(Ppu2A03* p)
{
	switch(p->cpu_ppu_io->ppu_ctrl & 0x03) {
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


uint16_t ppu_base_pt_address(Ppu2A03* p)
{
	if ((p->cpu_ppu_io->ppu_ctrl >> 4) & 0x01) {
		return 0x1000;
	} else {
		return 0x0000;
	}
}

uint16_t ppu_sprite_pattern_table_addr(Ppu2A03* p)
{
	if ((p->cpu_ppu_io->ppu_ctrl >> 3) & 0x01) {
		return 0x1000;
	} else {
		return 0x0000;
	}
}

uint8_t ppu_sprite_height(Ppu2A03* p)
{
	if ((p->cpu_ppu_io->ppu_ctrl >> 5) & 0x01) {
		return 16; /* 8 x 16 */
	} else {
		return 8; /* 8 x 8 */
	}
}

/**
 * PPU_MASK
 */

bool ppu_show_bg(Ppu2A03* p)
{
	if (p->cpu_ppu_io->ppu_mask & 0x08) {
		return true;
	} else {
		return false;
	}
}


bool ppu_show_sprite(Ppu2A03* p)
{
	if (p->cpu_ppu_io->ppu_mask & 0x10) {
		return true;
	} else {
		return false;
	}
}

bool ppu_mask_left_8px_bg(Ppu2A03* p)
{
	if (p->cpu_ppu_io->ppu_mask & 0x02) {
		return false;
	} else {
		return true;
	}
}

bool ppu_mask_left_8px_sprite(Ppu2A03* p)
{
	if (p->cpu_ppu_io->ppu_mask & 0x04) {
		return false;
	} else {
		return true;
	}
}

bool ppu_show_greyscale(Ppu2A03* p)
{
	if (p->cpu_ppu_io->ppu_mask & 0x01) {
		return true;
	} else {
		return false;
	}
}

/**
 * PPU_STATUS
 */

bool sprite_overflow_occured(Ppu2A03* p)
{
	if (p->cpu_ppu_io->ppu_status & 0x40) {
		return true;
	} else {
		return false;
	}
}

/* 
 * Helper Functions
 */

// Taken from wiki.nesdev
void inc_vert_scroll(Ppu2A03 *p)
{
	uint16_t addr = p->vram_addr;
	if ((addr & 0x7000) != 0x7000) { // If fine Y < 7
		addr += 0x1000; // Increment fineY
	} else {
		addr &= ~0x7000; // fine Y = 0
		int y = (addr & 0x03E0) >> 5; // y = coarse Y
		if (y == 29) {
			y = 0; // coarse Y = 0
			addr ^= 0x0800; // Switch vertical nametable
		} else if (y == 31) {
			y = 0; // coarse Y = 0, nametable not switched
		} else {
			y++;
		}
		addr = (addr & ~0x03E0) | (y << 5); // Put y back into vram_addr
	}
	p->vram_addr = addr;
}

void inc_horz_scroll(Ppu2A03 *p)
{
	if ((p->vram_addr & 0x001F) == 31) {
		p->vram_addr &= ~0x001F;
		p->vram_addr ^= 0x0400;
	} else {
		p->vram_addr++;
	}
}

void fetch_nt_byte(Ppu2A03 *p)
{
	p->nt_addr_tmp = 0x2000 | (p->vram_addr & 0x0FFF);
	p->nt_byte = p->vram[p->nt_addr_tmp];
}

/* Determines colour palette */
void fetch_at_byte(Ppu2A03 *p)
{
	p->at_latch = p->vram[0x23C0 | (p->vram_addr & 0x0C00) | ((p->vram_addr >> 4) & 0x38) | ((p->vram_addr >> 2) & 0x07)];
}

/* Lo & Hi determine which index of the colour palette we use (0 to 3) */
void fetch_pt_lo(Ppu2A03 *p)
{
	uint16_t pt_offset = (p->nt_byte << 4) + ((p->vram_addr  & 0x7000) >> 12);
	uint8_t latch = p->vram[ppu_base_pt_address(p) | pt_offset];
	p->pt_lo_latch = reverse_bits[latch]; // 8th bit = 1st pixel to render
}


void fetch_pt_hi(Ppu2A03 *p)
{
	uint16_t pt_offset = (p->nt_byte << 4) + ((p->vram_addr  & 0x7000) >> 12) + 8;
	uint8_t latch = p->vram[ppu_base_pt_address(p) | pt_offset];
	p->pt_hi_latch = reverse_bits[latch]; // 8th bit = 1st pixel to render
}


void render_pixel(Ppu2A03 *p)
{
	unsigned bg_palette_addr;
	/* Defines the which colour palette to use */
	if (p->nt_addr_current & 0x02) { // Right quadrants
		if (p->nt_addr_current & 0x40) {
			bg_palette_addr = p->at_current >> 6; // Bottom right
		} else {
			bg_palette_addr = p->at_current >> 2; // Top right
		}
	} else { // Left quadrants
		if (p->nt_addr_current & 0x40) {
			bg_palette_addr = p->at_current >> 4; // Bottom left
		} else {
			bg_palette_addr = p->at_current; // Top left
		}
	}
	bg_palette_addr &= 0x03;
	bg_palette_addr <<= 2;
	bg_palette_addr += 0x3F00; // bg palette mem starts here

	unsigned bg_palette_offset = ((p->pt_hi_shift_reg & 0x01) << 1) | (p->pt_lo_shift_reg & 0x01);
	if (!bg_palette_offset) {
		bg_palette_addr = 0x3F00; // Take background colour (transparent)
	}

	if (ppu_mask_left_8px_bg(p) && p->cycle < 8) {
		bg_palette_addr = 0x3F00;
		bg_palette_offset = 0;
	}

	unsigned RGB = p->vram[bg_palette_addr + bg_palette_offset]; // Get RGB values

	/* Shift each cycle */
	p->pt_hi_shift_reg >>= 1;
	p->pt_lo_shift_reg >>= 1;

	/* Sprite Stuff */
	unsigned sprite_palette_offset[8] = {0, 0, 0, 0, 0, 0, 0};
	// Is sprite active
	for (int i = 7; i >= 0; i--) { // Low priority sprites first (high priority overwrites them)
		if (p->sprite_x_counter[i] != 0) {
			p->sprite_x_counter[i] -= 1;
		} else {
			sprite_palette_offset[i] = ((p->sprite_pt_hi_shift_reg[i] & 0x01) << 1) | (p->sprite_pt_lo_shift_reg[i] & 0x01);
			// Render sprites
			unsigned sprite_palette_addr = p->sprite_at_latches[i] & 0x03;
			sprite_palette_addr <<= 2;
			sprite_palette_addr += 0x3F10;
			if ((((p->sprite_at_latches[i] & 0x20) == 0) || !bg_palette_offset) && sprite_palette_offset[i]) { // front priority 
				RGB = p->vram[sprite_palette_addr + sprite_palette_offset[i]]; // Output sprite
			} else if (((p->sprite_at_latches[i] & 0x20) == 0x20) && bg_palette_offset) {
				RGB = p->vram[bg_palette_addr + bg_palette_offset];
			}
			p->sprite_pt_lo_shift_reg[i] >>= 1;
			p->sprite_pt_hi_shift_reg[i] >>= 1;
		}
	}
	if (p->scanline == p->sprite_zero_scanline && (p->sprite_zero_hit == false)) { // If sprite is on scanline
		if (bg_palette_offset != 0 && sprite_palette_offset[0] != 0
		    && !((ppu_mask_left_8px_bg(p) || ppu_mask_left_8px_sprite(p)) && p->cycle <= 8)
		    && ((ppu_show_bg(p) && ppu_show_sprite(p)))
		    && !sprite_overflow_occured(p)
		    && p->cycle != 256) {
			p->hit_scanline = p->scanline;
			p->hit_cycle = p->cycle + 1; // Sprite #0 hit is delayed by 1 tick (cycle)
			p->sprite_zero_hit = true;
		}
	} if ((p->scanline == p->hit_scanline) && (p->cycle == p->hit_cycle)) {
		p->cpu_ppu_io->ppu_status |= 0x40; // Sprite #0 hit
	} 
	// Send pixels to pixel buffer
	pixels[(p->cycle + (256 * p->scanline) - 1)] = 0xFF000000 | palette[RGB]; // Place in palette array, alpha set to 0xFF
}

void ppu_transfer_oam(Ppu2A03* p, unsigned index)
{
	for (int i = 0; i < 4; i++) {
		p->scanline_oam[(p->sprites_found * 4) + i] = p->oam[(index * 4) + i]; // Copy remaining bytes
	}
}

void reset_secondary_oam(Ppu2A03* p)
{
	for (int i = 0; i < 32; i++) {
		p->scanline_oam[i] = 0xFF; // Reset secondary OAM
	}
	/* Reset internals */
	p->sprite_index = 0;
	p->sprites_found = 0;
	p->stop_early = false;
	p->sprite_zero_scanline = p->sprite_zero_scanline_tmp;
}

void sprite_evaluation(Ppu2A03* p)
{
	int y_offset = 0;
	switch (p->cycle % 2) {
	case 1: // Odd cycles
		p->oam_read_buffer = p->oam[p->sprite_index * 4];
		break;
	case 0: //Even cycles
		y_offset = p->scanline - p->oam_read_buffer;
		if ((y_offset >= 0)
		   && (y_offset < ppu_sprite_height(p))
		   && (p->sprites_found <= 8)
		   && !p->stop_early) {
			ppu_transfer_oam(p, p->sprite_index); // sprite found load into secondary oam

			// Setting up sprite zero hit detection
			if (p->sprite_index == 0 && p->cycle == 66) {
				p->sprite_zero_scanline_tmp = p->scanline + 1;
			}
			p->sprites_found++;
		}

		++p->sprite_index;
		if (p->sprite_index == 64) {
			p->sprite_index = 0; // above reset should cover this
			p->stop_early = true;
		}

		if ((p->sprites_found == 9) && (y_offset >= 0) && (y_offset < ppu_sprite_height(p))) {
			// Trigger sprite overflow flag
			p->cpu_ppu_io->ppu_status |= 0x20;
		}
		break;
	}
}



/*************************
 * RENDERING             *
 *************************/

void clock_ppu(Ppu2A03 *p, Cpu6502* cpu, Display* nes_screen)
{
#ifdef __DEBUG__
	if (p->cpu_ppu_io->write_debug) {
		p->cpu_ppu_io->write_debug = false;
		append_ppu_info(p);
	}
#endif /* __DEBUG__ */

	// Idle cycle thus can run tick to increment cycle from 0 to 1 initially
	p->cycle++;
	if (p->cycle > 340) {
		p->cycle = 0; /* Reset cycle count to 0, max val = 340 */

		p->scanline++;
		if (p->scanline > 261) {
			p->scanline = 0; /* Reset scanline to 0, max val == 261 */
		}
	}

	/* NMI Calc - call function update_vblank() */
	if (p->scanline == p->nmi_start) {
		if (p->cycle == 0) {
			p->cpu_ppu_io->ppu_status |= 0x80; /* In VBlank */
		}
		if (p->cpu_ppu_io->ppu_ctrl & 0x80) { /* if PPU CTRL has execute NMI on VBlank */
			if (p->cycle == 1) {
				p->cpu_ppu_io->nmi_pending = true;
				p->cpu_ppu_io->nmi_cycles_left = 7;
			}
			// suppress nmi
			if (p->cycle == 2) {
				if (p->cpu_ppu_io->suppress_nmi) {
					p->cpu_ppu_io->nmi_pending = false;
				}
			}
		}
		// clear VBlank flag if cpu clock is aligned w/ the ppu clock
		// hard coded for NTSC currently
		if (p->cpu_ppu_io->suppress_nmi && (cpu->cycle % 3 == 0)) {
			p->cpu_ppu_io->ppu_status &= ~0x80;
		}
	} else if (p->scanline == 261 && p->cycle == 0) { /* Pre-render scanline */
		// Clear VBlank, sprite hit and sprite overflow flags
		p->cpu_ppu_io->ppu_status &= ~0xE0;
	}


#ifdef __RESET__
	ppu_reset(1, p, cpu);
#endif

	if (ppu_show_bg(p) || ppu_show_sprite(p)) {
		// Sprites are evaluated for either BG or sprite rendering
		if (p->scanline <= 239) { /* Visible scanlines */
			 if (p->cycle > 64 && p->cycle <= 256) {
				sprite_evaluation(p);
			}
		}
	}

	/* Process BG Scanlines */
	if(ppu_show_bg(p)) {
		if (p->scanline <= 239) { /* Visible scanlines */
			if (p->cycle <= 256 && (p->cycle != 0)) { // 0 is an idle cycle
				// BG STUFF
				render_pixel(p); // Render pixel every cycle
				switch ((p->cycle - 1) & 0x07) {
				case 0:
					fetch_nt_byte(p);
					break;
				case 2:
					fetch_at_byte(p);
					break;
				case 4:
					fetch_pt_lo(p);
					break;
				case 6:
					fetch_pt_hi(p);
					break;
				case 7: /* 8th Cycle */
					// 8 Shifts should have occured by now, load new data
					p->at_current = p->at_next;
					p->at_next = p->at_latch;
					/* Load latched values into upper byte of shift regs */
					p->pt_lo_shift_reg |= (uint16_t) (p->pt_lo_latch << 8);
					p->pt_hi_shift_reg |= (uint16_t) (p->pt_hi_latch << 8);
					/* Used for palette calculations */
					p->nt_addr_current = p->nt_addr_next;
					p->nt_addr_next = p->nt_addr_tmp;
					// Update Scroll
					inc_horz_scroll(p);
					break;
				}
				if (p->cycle == 256) {
					inc_vert_scroll(p);
				}
			} else if (p->cycle == 257) {
				// Copy horz scroll bits from t
				p->vram_addr = (p->vram_addr & ~0x041F) | (p->vram_tmp_addr & 0x041F);
			} else if (p->cycle >= 321 && p->cycle <= 336) { // 1st 16 pixels of next scanline
				switch ((p->cycle - 1) & 0x07) {
				case 0:
					fetch_nt_byte(p);
					break;
				case 2:
					fetch_at_byte(p);
					break;
				case 4:
					fetch_pt_lo(p);
					break;
				case 6:
					fetch_pt_hi(p);
					/* Load latched values into upper byte of shift regs */
					p->pt_hi_shift_reg >>= 8;
					p->pt_lo_shift_reg >>= 8;
					p->pt_hi_shift_reg |= (uint16_t) (p->pt_hi_latch << 8);
					p->pt_lo_shift_reg |= (uint16_t) (p->pt_lo_latch << 8);
					p->at_current = p->at_next; // at_current is 1st loaded w/ garbage
					p->at_next = p->at_latch;
					p->nt_addr_current = p->nt_addr_next;
					p->nt_addr_next = p->nt_addr_tmp;
					break;
				case 7:
					// Update Scroll
					inc_horz_scroll(p);
					break;
				}
			}
		} else if (p->scanline == 240 && p->cycle == 0) {
			draw_pixels(pixels, nes_screen);  // Render frame
		} else if (p->scanline == 261) {
			// Pre-render scanline
			if (p->cycle <= 256 && (p->cycle != 0)) { // 0 is an idle cycle
				switch ((p->cycle - 1) & 0x07) {
				case 0:
					fetch_nt_byte(p);
					break;
				case 2:
					fetch_at_byte(p);
					break;
				case 4:
					fetch_pt_lo(p);
					break;
				case 6:
					fetch_pt_hi(p);
					break;
				case 7:
					// No need to fill shift registers as nothing is being rendered here
					// Update scroll
					inc_horz_scroll(p);
					break;
				}
				if (p->cycle == 256) {
					inc_vert_scroll(p);
				}
			} else if (p->cycle == 257) {
				// Copy horz scroll bits from t
				p->vram_addr = (p->vram_addr & ~0x041F) | (p->vram_tmp_addr & 0x041F);
			} else if (p->cycle >= 280 && p->cycle <= 304) {
				// Copy horz scroll bits from t
				p->vram_addr = (p->vram_addr & ~0x7BE0) | (p->vram_tmp_addr & 0x7BE0);
			} else if (p->cycle >= 321 && p->cycle <= 336) { // 1st 16 pixels of next scanline
				switch ((p->cycle - 1) & 0x07) {
				case 0:
					fetch_nt_byte(p);
					break;
				case 2:
					fetch_at_byte(p);
					break;
				case 4:
					fetch_pt_lo(p);
					break;
				case 6:
					fetch_pt_hi(p);
					/* Load latched values into upper byte of shift regs */
					p->pt_hi_shift_reg >>= 8;
					p->pt_lo_shift_reg >>= 8;
					p->pt_hi_shift_reg |= (uint16_t) (p->pt_hi_latch << 8);
					p->pt_lo_shift_reg |= (uint16_t) (p->pt_lo_latch << 8);
					p->at_current = p->at_next; // at_current is 1st loaded w/ garbage
					p->at_next = p->at_latch;
					p->nt_addr_current = p->nt_addr_next;
					p->nt_addr_next = p->nt_addr_tmp;
					break;
				case 7:
					// Update Scroll
					inc_horz_scroll(p);
					break;
				}
			}
		}
	}

	/* Process Sprites */
	if (ppu_show_sprite(p)) {
		if (p->scanline <= 239) { /* Visible scanlines */
			if (p->cycle <= 64 && (p->cycle != 0)) {
				reset_secondary_oam(p);
			} else if (p->cycle > 256 && p->cycle <= 320) { // Sprite data fetches
				static unsigned count = 0; // Counts 8 secondary OAM
				if (p->cycle == 257) {
					p->sprite_index = 0; // Using to access scanline_oam
					count = 0;
				}
				static int offset = 0;
				switch ((p->cycle - 1) & 0x07) {
				case 0:
					// Garbage NT byte - no need to emulate
					break;
				case 1:
					// When not in range the sprite is filled w/ FF
					p->sprite_addr = ppu_sprite_pattern_table_addr(p)
					               | (uint16_t) p->scanline_oam[(count * 4) + 1] << 4; // Read tile numb
					// 8x16 sprites don't use ppu_ctrl to determine base pt address
					if (ppu_sprite_height(p) == 16) {
						// Bit 0 determines base pt address, 0x1000 or 0x000
						p->sprite_addr = (0x1000 * (p->scanline_oam[(count * 4) + 1] & 0x01))
						               | (uint16_t) (p->scanline_oam[(count * 4) + 1] & 0xFE) << 4; // Read tile numb (ignoring bit 0, see & 0xFE)
					}
					offset = p->scanline - p->scanline_oam[count * 4];
					if (offset < 0) { // Keep address static until we reach the scanline in range
						offset = 0; // Stops out of bounds access for -1
					}

					// addr for rows 9-16 of 8x16 sprites
					if (ppu_sprite_height(p) == 16) {
						if (offset >= 8) { offset += 8; }
					}
					p->sprite_addr += offset;

					break;
				case 2:
					// Garbage AT byte - no need to emulate
					p->sprite_at_latches[count] = p->scanline_oam[(count * 4) + 2];

					if (p->sprite_at_latches[count] & 0x80) { // Flip vertical pixles
						// undo offset, then go from offset_max down to 0
						// e.g. 0-7 is now flipped to 7-0 (for sprites 8px high)
						p->sprite_addr = p->sprite_addr - offset
						                 + (ppu_sprite_height(p) - 1) - (offset % ppu_sprite_height(p));
					}
					break;
				case 3:
					// Read X Pos (In NES it's re-read until the 8th cycle)
					p->sprite_x_counter[count] = p->scanline_oam[(count * 4) + 3];
					break;
				case 4:
					// Fetch sprite low pt
					p->sprite_pt_lo_shift_reg[count] = reverse_bits[p->vram[p->sprite_addr]];
					if ((p->sprite_at_latches[count] & 0x40)) { // Flip horizontal pixels
						p->sprite_pt_lo_shift_reg[count] = p->vram[p->sprite_addr];
					}
					break;
				case 6:
					// Fetch sprite hi pt, turn into function once all attribute data is processed
					p->sprite_pt_hi_shift_reg[count] = reverse_bits[p->vram[p->sprite_addr + 8]];
					if ((p->sprite_at_latches[count] & 0x40)) { // Flip horizontal pixels
						p->sprite_pt_hi_shift_reg[count] = p->vram[p->sprite_addr + 8];
					}
					break;
				case 7: /* 8th Cycle */
					count++;
					for (int i = p->sprites_found; i < 8; i++) { // Less than 8 sprites, zero out pattern table for remainding sprites
						p->sprite_pt_lo_shift_reg[i] = 0; // Empty slots should have transparent/bg pixel values
						p->sprite_pt_hi_shift_reg[i] = 0; // Empty slots should have transparent/bg pixel values
					}
					break;
				}
			}
		} else if (p->scanline == 261) { /* Pre-render scanline */
			// only bg fetches occur
	
			p->sprite_index = 0;
			// Clear sprite #0 hit data
			if (p->cycle == 1) {
				p->sprite_zero_hit = false;
				p->sprite_zero_scanline = 600;
				p->sprite_zero_scanline_tmp = 600;
				p->hit_scanline = 600;
				p->hit_cycle = 600;
			}
		}
	}
}
