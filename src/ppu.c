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

// Static prototype functions
static void read_2002(Cpu6502* cpu);
static void read_2004(Cpu6502* cpu);
static void read_2007(Cpu6502* cpu);
static void write_2000(const uint8_t data, Cpu6502* cpu); // OAM_ADDR
static void write_2003(const uint8_t data, Cpu6502* cpu); // OAM_ADDR
static void write_2004(const uint8_t data, Cpu6502* cpu); // OAM_DATA
static void write_2005(const uint8_t data, Cpu6502* cpu); // PPU_SCROLL
static void write_2006(const uint8_t data, Cpu6502* cpu); // PPU_ADDR
static void write_2007(const uint8_t data, Cpu6502* cpu); // PPU_DATA
static void write_4014(const uint8_t data, Cpu6502* cpu); // DMA_DATA
static uint8_t ppu_vram_addr_inc(const Cpu6502* cpu);
static void inc_vert_scroll(CpuPpuShare* cpu_ppu_io);
static void inc_horz_scroll(CpuPpuShare* cpu_ppu_io);


Ppu2C02* ppu_init(CpuPpuShare* cp)
{
	Ppu2C02* ppu = malloc(sizeof(Ppu2C02));
	if (!ppu) {
		fprintf(stderr, "Failed to allocate enough memory for PPU\n");
		return ppu;
	}
	ppu->cpu_ppu_io = cp;
	ppu->cpu_ppu_io->vram = &ppu->vram; // used so CPU can access PPU VRAM w/o needing the PPU struct
	ppu->cpu_ppu_io->oam = &(ppu->oam[0]); // used so CPU can access PPU VRAM w/o needing the PPU struct
	ppu->cpu_ppu_io->buffer_2007 = 0;
	ppu->cpu_ppu_io->vram_addr = &ppu->vram_addr;
	ppu->cpu_ppu_io->vram_tmp_addr = &ppu->vram_tmp_addr;
	ppu->cpu_ppu_io->nametable_mirroring = &ppu->nametable_mirroring;
	ppu->cpu_ppu_io->fine_x = &ppu->fine_x;
	ppu->cpu_ppu_io->write_debug = false;
	ppu->cpu_ppu_io->clear_status = false;
	ppu->cpu_ppu_io->bg_early_disable_mask = false;
	ppu->cpu_ppu_io->bg_early_enable_mask = false;
	ppu->cpu_ppu_io->ppu_rendering_period = false;

	ppu->cycle = 27;
	ppu->scanline = 0;
	ppu->oam_read_buffer = 0;
	ppu->old_cycle = ppu->cycle;
	ppu->old_scanline = ppu->scanline;
	ppu->odd_frame = false;
	ppu->nt_addr_current = 0;

	/* Set PPU Latches and shift reg to 0 */
	ppu->pt_lo_shift_reg = 0;
	ppu->pt_hi_shift_reg = 0;
	ppu->pt_lo_latch = 0;
	ppu->pt_hi_latch = 0;
	ppu->at_lo_shift_reg = 0;
	ppu->at_hi_shift_reg = 0;
	ppu->at_current = 0;
	ppu->nt_addr_current = 0;

	/* Sprite stuff */
	ppu->sprites_found = 0;
	ppu->sprite_index = 0; // Fetch sprite #0 1st
	ppu->stop_early = false;
	ppu->sprite_zero_hit = false;
	ppu->sprite_zero_scanline = 600;
	ppu->sprite_zero_scanline_tmp = 600;
	ppu->hit_scanline = 600; // Impossible values
	ppu->hit_cycle = 600; // Impossible values
	ppu->sp_frame_hit_lookahead = false;
	ppu->bg_lo_reg = 0;
	ppu->bg_hi_reg = 0;
	ppu->sp_lo_reg = 0;
	ppu->sp_hi_reg = 0;
	ppu->l_sl = 400;
	ppu->l_cl = 400;
	memset(ppu->bg_opaque_hit, 0, sizeof(ppu->bg_opaque_hit));
	memset(ppu->sp_opaque_hit, 0, sizeof(ppu->sp_opaque_hit));

	// Zero out arrays
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

/* common masks, bit set and clear operations for ppu registers
 *
 * For ppu calling functions: arg 1 == p->cpu_ppu_io
 * For cpu calling functions: arg 1 == cpu->cpu_ppu_io
 */
static inline bool ppu_status_vblank_bit_set(const CpuPpuShare* cpu_ppu_io)
{
	return ((cpu_ppu_io->ppu_status & 0x80) ? 1 : 0);
}

static inline bool ppu_ctrl_gen_nmi_bit_set(const CpuPpuShare* cpu_ppu_io)
{
	return ((cpu_ppu_io->ppu_ctrl & 0x80) ? 1 : 0);
}

static inline void clear_ppu_status_vblank_bit(CpuPpuShare* cpu_ppu_io)
{
	cpu_ppu_io->ppu_status &= ~0x80;
}

static inline void set_ppu_status_vblank_bit(CpuPpuShare* cpu_ppu_io)
{
	cpu_ppu_io->ppu_status |= 0x80;
}

static inline bool ppu_mask_bg_or_sprite_enabled(const CpuPpuShare* cpu_ppu_io)
{
	return ((cpu_ppu_io->ppu_mask & 0x18) ? 1 : 0);
}

// Reset/Warm-up function, clears and sets VBL flag at certain CPU cycles
static void ppu_vblank_warmup_seq(Ppu2C02* p, const Cpu6502* cpu)
{
	static unsigned count = 0;
	if (!count) {
		clear_ppu_status_vblank_bit(p->cpu_ppu_io);
		++count;
	} else if ((count == 1) && cpu->cycle >= 27383) {
		set_ppu_status_vblank_bit(p->cpu_ppu_io);
		++count;
	} else if ((count == 2) && cpu->cycle >= 57164) {
		set_ppu_status_vblank_bit(p->cpu_ppu_io);
		++count;
	}
}

static void append_ppu_info(Ppu2C02* ppu)
{
	printf(" PPU_CYC: %-3" PRIu16, ppu->old_cycle);
	printf(" SL: %" PRIu32 "\n", ppu->old_scanline);
	ppu->old_cycle = ppu->cycle;
	ppu->old_scanline = ppu->scanline;
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

/* For ppu calling functions: arg 1 == &p->vram
 * For cpu calling functions: arg 1 == cpu->cpu_ppu_io->vram
 */
static void write_to_ppu_vram(struct PpuMemoryMap* mem, unsigned addr, uint8_t data)
{
	if (addr < 0x1000) {
		// 0x0000 to 0x0FFF
		mem->pattern_table_0[addr] = data;
	} else if (addr < 0x2000) {
		// 0x1000 to 0x1FFF
		mem->pattern_table_1[addr & 0x0FFF] = data;
	} else if (addr < 0x2400) {
		// 0x2000 to 0x23FF
		(*mem->nametable_0)[addr & 0x03FF] = data;
	} else if (addr < 0x2800) {
		// 0x2400 to 0x27FF
		(*mem->nametable_1)[addr & 0x03FF] = data;
	} else if (addr < 0x2C00) {
		// 0x2800 to 0x2BFF
		(*mem->nametable_2)[addr & 0x03FF] = data;
	} else if (addr < 0x3000) {
		// 0x2C00 to 0x2FFF
		(*mem->nametable_3)[addr & 0x03FF] = data;
	} else if (addr < 0x3400) {
		// mirror of 0x2000 to 0x23FF
		(*mem->nametable_0)[addr & 0x03FF] = data;
	} else if (addr < 0x3800) {
		// mirror of 0x2400 to 0x27FF
		(*mem->nametable_1)[addr & 0x03FF] = data;
	} else if (addr < 0x3C00) {
		// mirror 0x2800 to 0x2BFF
		(*mem->nametable_2)[addr & 0x03FF] = data;
	} else if (addr < 0x3F00) {
		// partial mirror of 0x2C00 to 0x2FFF (0x2C00 to 0x2EFF)
		(*mem->nametable_3)[addr & 0x03FF] = data;
	} else if (addr < 0x4000) {
		// 0x3F00 to 0x3F20 and mirrors down to 0x3FFF
		mem->palette_ram[addr & 0x001F] = data;
	}
}

static uint8_t read_from_ppu_vram(const struct PpuMemoryMap* mem, unsigned addr)
{
	uint8_t ret = 0;
	if (addr < 0x1000) {
		// 0x0000 to 0x0FFF
		ret = mem->pattern_table_0[addr];
	} else if (addr < 0x2000) {
		// 0x1000 to 0x1FFF
		ret = mem->pattern_table_1[addr & 0x0FFF];
	} else if (addr < 0x2400) {
		// 0x2000 to 0x23FF
		ret = (*mem->nametable_0)[addr & 0x03FF];
	} else if (addr < 0x2800) {
		// 0x2400 to 0x27FF
		ret = (*mem->nametable_1)[addr & 0x03FF];
	} else if (addr < 0x2C00) {
		// 0x2800 to 0x2BFF
		ret = (*mem->nametable_2)[addr & 0x03FF];
	} else if (addr < 0x3000) {
		// 0x2C00 to 0x2FFF
		ret = (*mem->nametable_3)[addr & 0x03FF];
	} else if (addr < 0x3400) {
		// mirror of 0x2000 to 0x23FF
		ret = (*mem->nametable_0)[addr & 0x03FF];
	} else if (addr < 0x3800) {
		// mirror of 0x2400 to 0x27FF
		ret = (*mem->nametable_1)[addr & 0x03FF];
	} else if (addr < 0x3C00) {
		// mirror 0x2800 to 0x2BFF
		ret = (*mem->nametable_2)[addr & 0x03FF];
	} else if (addr < 0x3F00) {
		// partial mirror of 0x2C00 to 0x2FFF (0x2C00 to 0x2EFF)
		ret = (*mem->nametable_3)[addr & 0x03FF];
	} else if (addr < 0x4000) {
		// 0x3F00 to 0x3F20 and mirrors down to 0x3FFF
		ret = mem->palette_ram[addr & 0x001F];
	}

	return ret;
}

void ppu_mem_hexdump_addr_range(const Ppu2C02* ppu, const enum PpuMemoryTypes ppu_mem, unsigned start_addr, uint16_t end_addr)
{
	if (end_addr <= start_addr) {
		fprintf(stderr, "Hexdump failed, need more than 1 byte to read (end_addr must be greater than start_addr)\n");
		return;
	}

	if (ppu_mem == VRAM) {
		printf("\n##################### PPU VRAM #######################\n");
		if (end_addr > (16 * KiB - 1)) {
			end_addr = 0x3FFF; // keep value between 0 and 16 KiB - 1
		}
	} else if (ppu_mem == PRIMARY_OAM) {
		printf("\n#################### PPU OAM ALL #####################\n");
		if (end_addr >= 255) {
			end_addr = 0xFF;  // keep value between 0-255
		}
	} else if (ppu_mem == SECONDARY_OAM) {
		printf("\n################## PPU OAM SCANLINE ##################\n");
		if (end_addr > 31) {
			end_addr = 0x1F;  // keep value between 0-31
		}
	} else if (ppu_mem == PATTERN_TABLE_0) {
		printf("\n################# PPU PATTERN TABLE 0 ################\n");
		if (end_addr > 0x0FFF) {
			end_addr = 0x0FFF;  // keep value between 0 and 0x0FFF
		}
	} else if (ppu_mem == PATTERN_TABLE_1) {
		printf("\n################# PPU PATTERN TABLE 1 ################\n");
		if (end_addr > 0x0FFF) {
			end_addr = 0x0FFF;  // keep value between 0 and 0x0FFF
		}
	} else if (ppu_mem == NAMETABLE_A) {
		printf("\n################### PPU NAMETABLE A ##################\n");
		if (end_addr > 0x03FF) {
			end_addr = 0x03FF;  // keep value between 0 and 0x03FF
		}
	} else if (ppu_mem == NAMETABLE_B) {
		printf("\n################### PPU NAMETABLE B ##################\n");
		if (end_addr > 0x03FF) {
			end_addr = 0x03FF;  // keep value between 0 and 0x03FF
		}
	} else if (ppu_mem == PALETTE_RAM) {
		printf("\n################### PPU PALETTE RAM ##################\n");
		if (end_addr > 0x001F) {
			end_addr = 0x001F;  // keep value between 0 and 0x001F
		}
	}
	// print header for memory addresses
	printf("      ");
	for (int h = 0; h < 16; h++) {
		printf("%.2X ", (start_addr & 0x0F) + h);
		// halfway point, print extra space for readability
		if (h == 7) {
			printf(" ");
		}
	}
	printf("\n");

	// acutally perform hexdump here
	while (start_addr < end_addr) {
		printf("%.4X: ", start_addr);
		for (int x = 0; x < 16; x++) {
			if ((start_addr + x) > end_addr) {
				// early stop
				break;
			}
			if (ppu_mem == VRAM) {
				printf("%.2X ", read_from_ppu_vram(&ppu->vram, start_addr + x));
			} else if (ppu_mem == PRIMARY_OAM) {
				printf("%.2X ", ppu->oam[start_addr + x]);
			} else if (ppu_mem == SECONDARY_OAM) {
				printf("%.2X ", ppu->scanline_oam[start_addr + x]);
			} else if (ppu_mem == PATTERN_TABLE_0) {
				printf("%.2X ", ppu->vram.pattern_table_0[start_addr + x]);
			} else if (ppu_mem == PATTERN_TABLE_1) {
				printf("%.2X ", ppu->vram.pattern_table_1[start_addr + x]);
			} else if (ppu_mem == NAMETABLE_A) {
				printf("%.2X ", ppu->vram.nametable_A[start_addr + x]);
			} else if (ppu_mem == NAMETABLE_B) {
				printf("%.2X ", ppu->vram.nametable_B[start_addr + x]);
			} else if (ppu_mem == PALETTE_RAM) {
				printf("%.2X ", ppu->vram.palette_ram[start_addr + x]);
			}
			// halfway point, print extra space for readability
			if (x == 7) {
				printf(" ");
			}
		}
		start_addr += 16;
		printf("\n");
	}
}

uint8_t read_ppu_reg(const uint16_t addr, Cpu6502* cpu)
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
		read_2004(cpu);
		ret = cpu->cpu_ppu_io->return_value;
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
void delay_write_ppu_reg(const uint16_t addr, const uint8_t data, Cpu6502* cpu)
{
	cpu->cpu_ppu_io->buffer_write = true;
	cpu->cpu_ppu_io->buffer_counter = 2;
	if (addr == 0x2001) {
		cpu->cpu_ppu_io->buffer_counter += 3; // 3 dot delay for background/sprite rendering
	}
	cpu->cpu_ppu_io->buffer_address = addr;
	cpu->cpu_ppu_io->buffer_value = data;


	cpu->cpu_ppu_io->ppu_status &= ~0x1F;
	cpu->cpu_ppu_io->ppu_status |= (data & 0x1F);
}

void write_ppu_reg(const uint16_t addr, const uint8_t data, Cpu6502* cpu)
{
	switch (addr) {
	case (0x2000):
		/* PPU CTRL */
		if (ppu_status_vblank_bit_set(cpu->cpu_ppu_io)
			&& !ppu_ctrl_gen_nmi_bit_set(cpu->cpu_ppu_io)
			&& (data & 0x80)) {
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

// Called from CPU
static void cpu_writes_to_vram(uint8_t data, Cpu6502* cpu)
{
	// keep address in valid range
	uint16_t addr = *(cpu->cpu_ppu_io->vram_addr) & 0x3FFF;

	// Write to pattern tables (if using CHR RAM), otherwise we are using CHR ROM
	if ((cpu->cpu_mapper_io->chr->ram_size) && (addr <= 0x1FFF)) {
		write_to_ppu_vram(cpu->cpu_ppu_io->vram, addr, data);
	}

	// Handle nametable and palette writes
	if (addr >= 0x2000) {
		write_to_ppu_vram(cpu->cpu_ppu_io->vram, addr, data);
		if (addr >= 0x3F00) {
			if ((addr & 0x0F) == 0) {
				// If bg palette #0, colour #0 mirror up to sprite's 0th palette and colour
				write_to_ppu_vram(cpu->cpu_ppu_io->vram, addr + 0x10, data);
			} else if ((addr & 0x1F) == 0x10) {
				// If sprite palette #0, colour #0 mirror down to bg's 0th palette and colour
				write_to_ppu_vram(cpu->cpu_ppu_io->vram, addr - 0x10, data);
			}
		}
	}
}

/* Read Functions */

/* Reading $2002 will always clear the write toggle
 * It will also clear VBLANK flag bit (bit 7)
 * The value read-back may have the VBLANK flag set or not (when close to NMI)
 */
static void read_2002(Cpu6502* cpu)
{
	cpu->cpu_ppu_io->return_value = cpu->cpu_ppu_io->ppu_status;
	clear_ppu_status_vblank_bit(cpu->cpu_ppu_io);
	cpu->cpu_ppu_io->write_toggle = false; // Clear latch used by PPUSCROLL & PPUADDR
	cpu->cpu_ppu_io->suppress_nmi_flag = true;

	if (cpu->cpu_ppu_io->clear_status) {
		cpu->cpu_ppu_io->return_value &= ~0x80;
		cpu->cpu_ppu_io->clear_status = false;
	}
}

static void read_2004(Cpu6502* cpu)
{
	cpu->cpu_ppu_io->return_value = cpu->cpu_ppu_io->oam[cpu->cpu_ppu_io->oam_addr];
	if ((cpu->cpu_ppu_io->oam_addr & 0x03) == 0x02) {
		// if reading back attribute bytes, return 0 for unused bitss
		cpu->cpu_ppu_io->return_value = cpu->cpu_ppu_io->oam[cpu->cpu_ppu_io->oam_addr] & 0xE3;
	}

	if (!cpu->cpu_ppu_io->ppu_rendering_period
	   || !ppu_mask_bg_or_sprite_enabled(cpu->cpu_ppu_io)) {
		// don't increment oam_addr outside of ppu rendering or if bg and sprite rendering is disabled
		return;
	}
	++cpu->cpu_ppu_io->oam_addr;
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
static void read_2007(Cpu6502* cpu)
{
	uint16_t addr = *(cpu->cpu_ppu_io->vram_addr) & 0x3FFF;

	cpu->cpu_ppu_io->return_value = cpu->cpu_ppu_io->buffer_2007;
	cpu->cpu_ppu_io->buffer_2007 = read_from_ppu_vram(cpu->cpu_ppu_io->vram, addr);

	if (addr >= 0x3F00) {
		cpu->cpu_ppu_io->return_value = read_from_ppu_vram(cpu->cpu_ppu_io->vram, addr);
		// buffer data would be if the nametable mirroring kept going
		// use minus 0x1000 to get out of palette addresses and read from nametable mirrors
		cpu->cpu_ppu_io->buffer_2007 = read_from_ppu_vram(cpu->cpu_ppu_io->vram, addr - 0x1000);
	}

	if (cpu->cpu_ppu_io->ppu_rendering_period && ppu_mask_bg_or_sprite_enabled(cpu->cpu_ppu_io)) {
		inc_vert_scroll(cpu->cpu_ppu_io);
		inc_horz_scroll(cpu->cpu_ppu_io);
	} else {
		*(cpu->cpu_ppu_io->vram_addr) += ppu_vram_addr_inc(cpu);
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
static void write_2000(const uint8_t data, Cpu6502* cpu)
{
	*(cpu->cpu_ppu_io->vram_tmp_addr) &= ~0x0C00;
	*(cpu->cpu_ppu_io->vram_tmp_addr) |= (data & 0x03) << 10;
}

/* Set OAMADDR: The value of OAMADDR when sprite_evaluation() is first called
 * determines the first sprite to be checked (this is the sprite 0)
 */
static inline void write_2003(const uint8_t data, Cpu6502* cpu)
{
	cpu->cpu_ppu_io->oam_addr = data;
}

/* Write OAMDATA to previously set OAMADDR (through a $2003 write)
 * OAMADDR is incremented after the write
 */
static void write_2004(const uint8_t data, Cpu6502* cpu)
{
	// If rendering is enabled (either bg or sprite) during
	// scanlines 0-239 and pre-render scanline
	// OAM writes are disabled, but glitchy OAM increment occurs
	if (ppu_mask_bg_or_sprite_enabled(cpu->cpu_ppu_io)
	    && cpu->cpu_ppu_io->ppu_rendering_period) {
		cpu->cpu_ppu_io->oam_addr += 4;  // only increment high 6 bits (same as +4)
		return;
	}
	cpu->cpu_ppu_io->oam[cpu->cpu_ppu_io->oam_addr] = data;
	++cpu->cpu_ppu_io->oam_addr;
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
static void write_2005(const uint8_t data, Cpu6502* cpu)
{
	// Valid address = 0x0000 to 0x3FFF
	if (!cpu->cpu_ppu_io->write_toggle) {
		// First Write
		*(cpu->cpu_ppu_io->vram_tmp_addr) &= ~0x001F;
		*(cpu->cpu_ppu_io->vram_tmp_addr) |= (data >> 3);
		*(cpu->cpu_ppu_io->fine_x) = data & 0x07;
	} else {
		// Second Write
		*(cpu->cpu_ppu_io->vram_tmp_addr) &= ~0x73E0;
		*(cpu->cpu_ppu_io->vram_tmp_addr) |= ((data & 0xF8) << 2) | ((data & 0x07) << 12);
		cpu->cpu_ppu_io->ppu_scroll = *(cpu->cpu_ppu_io->vram_tmp_addr);
	}
	cpu->cpu_ppu_io->write_toggle = !cpu->cpu_ppu_io->write_toggle;
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
static void write_2006(const uint8_t data, Cpu6502* cpu)
{
	// Valid address = 0x0000 to 0x3FFF
	if (!cpu->cpu_ppu_io->write_toggle) {
		*(cpu->cpu_ppu_io->vram_tmp_addr) &= ~0x7F00;
		*(cpu->cpu_ppu_io->vram_tmp_addr) |= (uint16_t) ((data & 0x3F) << 8);
	} else {
		*(cpu->cpu_ppu_io->vram_tmp_addr) &= ~0x00FF;
		*(cpu->cpu_ppu_io->vram_tmp_addr) |= data;
		*(cpu->cpu_ppu_io->vram_addr) = *(cpu->cpu_ppu_io->vram_tmp_addr);
		cpu->cpu_ppu_io->ppu_addr = *(cpu->cpu_ppu_io->vram_tmp_addr);
	}
	cpu->cpu_ppu_io->write_toggle = !cpu->cpu_ppu_io->write_toggle;
}


/* Write to PPU VRAM through PPUDATA register
 *
 * Writing to this register will cause the vram address to increment outside of rendering
 * via the PPUCTRL register
 * Writing while rendering causes the vram address is incremented in an odd way
 * by simultaneously updating horizontal (coarse X) and vertical (Y) scrolling
 */
static void write_2007(const uint8_t data, Cpu6502* cpu)
{
	cpu_writes_to_vram(data, cpu);

	if (cpu->cpu_ppu_io->ppu_rendering_period && ppu_mask_bg_or_sprite_enabled(cpu->cpu_ppu_io)) {
		inc_vert_scroll(cpu->cpu_ppu_io);
		inc_horz_scroll(cpu->cpu_ppu_io);
	} else {
		*(cpu->cpu_ppu_io->vram_addr) += ppu_vram_addr_inc(cpu);
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
static uint8_t ppu_vram_addr_inc(const Cpu6502* cpu)
{
	if (!(cpu->cpu_ppu_io->ppu_ctrl & 0x04)) {
		return 1;
	} else {
		return 32;
	}
}

static uint16_t ppu_base_nt_address(const Ppu2C02* p)
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


static uint16_t ppu_base_pt_address(const Ppu2C02* p)
{
	if ((p->cpu_ppu_io->ppu_ctrl >> 4) & 0x01) {
		return 0x1000;
	} else {
		return 0x0000;
	}
}

static uint16_t ppu_sprite_pattern_table_addr(const Ppu2C02* p)
{
	if ((p->cpu_ppu_io->ppu_ctrl >> 3) & 0x01) {
		return 0x1000;
	} else {
		return 0x0000;
	}
}

static uint8_t ppu_sprite_height(const Ppu2C02* p)
{
	if ((p->cpu_ppu_io->ppu_ctrl >> 5) & 0x01) {
		return 16; // 8 x 16 sprites
	} else {
		return 8; // 8 x 8 sprites
	}
}

/**
 * PPU_MASK
 */

static bool ppu_show_bg(const Ppu2C02* p)
{
	if (p->cpu_ppu_io->ppu_mask & 0x08) {
		return true;
	} else {
		return false;
	}
}


static bool ppu_show_sprite(const Ppu2C02* p)
{
	if (p->cpu_ppu_io->ppu_mask & 0x10) {
		return true;
	} else {
		return false;
	}
}

static bool ppu_mask_left_8px_bg(const Ppu2C02* p)
{
	if (p->cpu_ppu_io->ppu_mask & 0x02) {
		return false;
	} else {
		return true;
	}
}

static bool ppu_mask_left_8px_sprite(const Ppu2C02* p)
{
	if (p->cpu_ppu_io->ppu_mask & 0x04) {
		return false;
	} else {
		return true;
	}
}

static bool ppu_show_greyscale(const Ppu2C02* p)
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

static bool sprite_overflow_occured(const Ppu2C02* p)
{
	if (p->cpu_ppu_io->ppu_status & 0x20) {
		return true;
	} else {
		return false;
	}
}

/* 
 * Helper Functions
 */

/* Increment fine_y after every scanline has output its pixels
 * (after 256 pixels have been rendered)
 *
 * When fine_y reaches 7 we cannot increment fine_y again, we must reset
 * fine_y and increment coarse Y as we've moved onto the next tile in the
 * vertical direction
 *
 * vram_addr byte layout: 0yyy NNYY YYYX XXXX
 * X is coarse X, Y is coarse Y, N is nametable select, y is fine Y
 *
 * Each nametable can represent a 32x30 (coarseX and coarseY, 8x8 region)
 *
 * Therefore we must change the NN bit (bit XORing w/ 0x80) when going
 * from one nametable to the other in the Y direction (including warping from
 * the bottom back to the top e.g. $2800 to $2000, hence the XOR)
 *
 * Since coarse Y is a 5 bit number and there are 32 possible numbers (0-31)
 * This gives us two free numbers (seeing as there are only 30 coarse Y tiles
 * to render to screen) which are out of bounds. This causes the PPU to interpret
 * the "tile data" as attribute data (as we've moved onto those memory addresses).
 * When trying to increment from coarse Y from 31, coarse Y is reset but the nametable
 * remains unchanged
 */
static void inc_vert_scroll(CpuPpuShare* cpu_ppu_io)
{
	uint16_t addr = *(cpu_ppu_io->vram_addr);
	if ((addr & 0x7000) != 0x7000) { // If fine_y < 7
		addr += 0x1000; // Increment fine_y
	} else {
		addr &= ~0x7000; // Clear fine_y bits
		int coarse_y = (addr & 0x03E0) >> 5;
		if (coarse_y == 29) {
			coarse_y = 0;
			addr ^= 0x0800;
		} else if (coarse_y == 31) {
			coarse_y = 0;
		} else {
			coarse_y++;
		}
		addr = (addr & ~0x03E0) | (coarse_y << 5); // Put coarse Y back into vram_addr
	}
	*(cpu_ppu_io->vram_addr) = addr;
}

/* Increment coarseX after every scanline has output its pixels
 * (after 256 pixels have been rendered)
 *
 * vram_addr byte layout: 0yyy NNYY YYYX XXXX
 * X is coarse X, Y is coarse Y, N is nametable select, y is fine Y
 *
 * Each nametable can represent a 32x30 (coarseX and coarseY, 8x8 region)
 *
 * So we can just increment coarse X until we reach our 32nd value (31 as
 * 0 is a valid number) and reset coarse X on that specific value
 *
 * fine_x handles the horizontal pixel offset and isn't adjusted here
 */
static void inc_horz_scroll(CpuPpuShare* cpu_ppu_io)
{
	if ((*(cpu_ppu_io->vram_addr) & 0x001F) == 31) {
		*(cpu_ppu_io->vram_addr) &= ~0x001F;
		*(cpu_ppu_io->vram_addr) ^= 0x0400;
	} else {
		*(cpu_ppu_io->vram_addr) += 1;
	}
}

static void fetch_nt_byte(Ppu2C02 *p)
{
	p->nt_addr_tmp = 0x2000 | (p->vram_addr & 0x0FFF);
	p->nt_byte = read_from_ppu_vram(&p->vram, p->nt_addr_tmp);
}

/* Get correct at byte from vram_addr
 *
 * vram_addr byte layout: 0yyy NNYY YYYX XXXX
 * X is coarse X, Y is coarse Y, N is nametable select, y is fine Y
 *
 * vram_addr & 0x0C00 masks the NN bits
 * NN is either 0 ($2000), 1 ($2400), 2 ($2800), 3 ($2C00)
 * for 0 attribute table starts @ $23C0, 1 $(27C0), 2 ($2BC0), 3 ($2FC0)
 * (using the NN mask | 23C0)
 *
 * (vram_addr >> 4) & 0x38 becomes: YY YYYY & 0x38 --> YY Y000
 * This is coarse Y / 4
 *
 * (vram_addr >> 2) & 0x07 becomes: XXX & 0x07 -> XXX
 * This is coarse X / 4
 *
 * Corse X and Y are the tile offsets in x/y direction
 * representing a group of 8 pixels
 *
 * These calculations work 1 attribute byte represents a 4x4 tile section
 * So we increment the attribute address in either the x or y direction once
 * we've passed 4 tiles in those directions
 */
static void fetch_at_byte(Ppu2C02 *p)
{
	p->at_latch = read_from_ppu_vram(&p->vram, 0x23C0 | (p->vram_addr & 0x0C00) | ((p->vram_addr >> 4) & 0x38) | ((p->vram_addr >> 2) & 0x07));
}

/* nt_byte represents the tile index (the nametable byte reference)
 * Pattern table of background is either found @ 0x0000 or 0x1000
 * One tile takes up 16 bytes, e.g. 0x0000 to 0x000F (one byte per row
 * for lo and hi bits)
 * Can represent pattern table as 0x0xx0 to 0x0xxF
 * where xx is the tile index (nt_byte << 4)
 * Y offset of tile is fine Y represented as a 3 bit value (0 to 7)
 */
static void fetch_pt_lo(Ppu2C02 *p)
{
	uint16_t pt_offset = (p->nt_byte << 4) + ((p->vram_addr  & 0x7000) >> 12);
	uint8_t latch = read_from_ppu_vram(&p->vram, ppu_base_pt_address(p) | pt_offset);
	p->pt_lo_latch = reverse_bits[latch]; // 8th bit = 1st pixel to render
}


/* Lo & Hi determine which index of the colour palette we use (0 to 3) */
static void fetch_pt_hi(Ppu2C02 *p)
{
	uint16_t pt_offset = (p->nt_byte << 4) + ((p->vram_addr  & 0x7000) >> 12) + 8;
	uint8_t latch = read_from_ppu_vram(&p->vram, ppu_base_pt_address(p) | pt_offset);
	p->pt_hi_latch = reverse_bits[latch]; // 8th bit = 1st pixel to render
}

/* Figure out what quadrant the input address is in and set the correct bits
 * in the lo and hi bit shift registers
 *
 * nametable_addr can either take the vram_addr or the actual nametable address
 * We ignore the upper bytes of the input so they are essentially the same number
 * nametable_addr byte layout: ???? ??YY YYYX XXXX
 * ?? are don't care bits
 */
static void fill_attribute_shift_reg(Ppu2C02 *p, uint16_t nametable_addr, uint8_t attribute_data)
{
	unsigned attr_bits = 0;
	// Right quadrants (CoarseX / 2): YYYX XXXX (select 0x02 bit)
	if (nametable_addr & 0x02) {
		// Bottom quadrants (CoarseY / 2): NNYY YYYX XXXX (select 0x04 bit)
		if (nametable_addr & 0x40) {
			attr_bits = attribute_data >> 6; // Bottom right
		} else {
			attr_bits = attribute_data >> 2; // Top right
		}
	} else { // Left quadrants
		if (nametable_addr & 0x40) {
			attr_bits = attribute_data >> 4; // Bottom left
		} else {
			attr_bits = attribute_data; // Top left
		}
	}

	// Clear registers
	p->at_hi_shift_reg = 0;
	p->at_lo_shift_reg = 0;

	// Separate 2 bit val into lo and hi shift registers
	// Shift registers contain repeated bits of the lo/hi bit
	// as the shift register represents one tile
	for (unsigned i = 0; i < 8; i++) {
		p->at_hi_shift_reg |= ((attr_bits & 0x02) >> 1) << i;
		p->at_lo_shift_reg |= (attr_bits & 0x01) << i;
	}
}


/* 3-bit number for select_lines, which selects input bits 0 through 7
 * Bit mask (1 << select lines) to get the correct input bit
 * Then shift down to the lsb
 * Intended to select a specific bit from the ppu's internal shift registers
 * when scrolling using fine_x to select the correct bit during rendering
 */
static unsigned eight_to_one_mux(uint16_t input, unsigned select_lines)
{
	return (input & (1 << select_lines)) >> select_lines;
}

static inline bool sprite_is_front_priority(const Ppu2C02* p, unsigned array_index)
{
	// if mask w/ 0x20 is zero then sprite is front priority
	return (!(p->sprite_at_latches[array_index] & 0x20)) ? 1 : 0;
}

static void render_pixel(Ppu2C02 *p)
{
	// We don't use fine_x to mux the attribute shift regs as when
	// we shift out the attribute shift registers the fine_x mux
	// will lead to a palette address of 3F00 as the upper bits
	// hold 0 and the mux will select those incorrect bits
	// Instead we reload the shift registers with new attribute data
	// when the fine_x will select a new tile to render e.g. moving from
	// pixels 1-8 in the pipeline to 9-16
	// They are then reloaded evey 8 cycles after that
	// (alternatively you can mux w/ fine_x as long as you don't shift
	// out the attribute shift registers)
	unsigned bg_palette_addr = (eight_to_one_mux(p->at_hi_shift_reg, 0) << 1)
	                         |  eight_to_one_mux(p->at_lo_shift_reg, 0);
	bg_palette_addr <<= 2;
	bg_palette_addr += 0x3F00; // bg palette mem starts here

	unsigned bg_colour_index = (eight_to_one_mux(p->pt_hi_shift_reg, p->fine_x) << 1)
	                         |  eight_to_one_mux(p->pt_lo_shift_reg, p->fine_x);
	if (!bg_colour_index) {
		bg_palette_addr = 0x3F00; // Take background colour (transparent)
	}

	if ((ppu_mask_left_8px_bg(p) && p->cycle < 8) || !ppu_show_bg(p)) {
		bg_palette_addr = 0x3F00;
		bg_colour_index = 0;
	}

	/* Summary of which pixel is chosen (BG or sprite):
	 *
	 * Regardless of sprite priority:
	 *   If bg_colour_index is non-zero and sprites colour index is 0, output BG
	 *   If sprite colour index is non-zero and bg_colour_index is 0, output sprite
	 *   If both sprite and bg colour indexes are 0, output BG @ 0x3F00
	 *
	 * If sprite priority is front (over background) and sprite colour index
	 * is non-zero, output sprite
	 *
	 * If sprite priority is back (behing background) and bg colour index
	 * is non zero, output BG
	 *
	 * Above process is simplified by always first "outputting" the BG and then
	 * overwritting this RGB value w/ the sprite version if the any of the two
	 * cases for sprite output are true
	 */
	unsigned RGB = read_from_ppu_vram(&p->vram, bg_palette_addr + bg_colour_index); // Get RGB values
	if (ppu_show_greyscale(p)) { RGB &= 0x30; }

	/* Shift out each cycle */
	p->pt_hi_shift_reg >>= 1;
	p->pt_lo_shift_reg >>= 1;
	p->at_hi_shift_reg >>= 1;
	p->at_lo_shift_reg >>= 1;

	/* Sprite Stuff */
	unsigned sprite_colour_index[8] = {0};
	// Is sprite active
	for (int i = 7; i >= 0; i--) { // Low priority sprites first (high priority overwrites them)
		if (p->sprite_x_counter[i] != 0) {
			p->sprite_x_counter[i] -= 1;
		} else {
			sprite_colour_index[i] = ((p->sprite_pt_hi_shift_reg[i] & 0x01) << 1)
			                       |  (p->sprite_pt_lo_shift_reg[i] & 0x01);
			// Render sprites
			unsigned sprite_palette_addr = p->sprite_at_latches[i] & 0x03;
			sprite_palette_addr <<= 2;
			sprite_palette_addr += 0x3F10;
			if ((sprite_is_front_priority(p, i) || !bg_colour_index) && sprite_colour_index[i]) {
				RGB = read_from_ppu_vram(&p->vram, sprite_palette_addr + sprite_colour_index[i]); // Output sprite
				if (ppu_show_greyscale(p)) { RGB &= 0x30; }
			}
			p->sprite_pt_lo_shift_reg[i] >>= 1;
			p->sprite_pt_hi_shift_reg[i] >>= 1;
		}
	}

	// Send pixels to pixel buffer
	pixels[(p->cycle + (256 * p->scanline) - 1)] = 0xFF000000 | palette[RGB]; // Place in palette array, alpha set to 0xFF
}

/* Intended for ppu cycles 257-320
 * Forms the 0-7 array indexes for secondary oam fetches
 * based on the current ppu cycle
 */
static inline unsigned sprite_fetch_index(const Ppu2C02* p)
{
	// 0: 257-264
	// 1: 265-272
	// etc.
	return (((p->cycle - 1) / 8) - 32);
}

static inline uint8_t secondary_oam_y_pos(const Ppu2C02* p, const unsigned sprite_index)
{
	return (p->scanline_oam[sprite_index * 4]);
}

static inline uint8_t secondary_oam_tile_number(const Ppu2C02* p, const unsigned sprite_index)
{
	return (p->scanline_oam[(sprite_index * 4) + 1]);
}

static inline uint8_t secondary_oam_at_byte(const Ppu2C02* p, const unsigned sprite_index)
{
	return (p->scanline_oam[(sprite_index * 4) + 2]);
}

static inline uint8_t secondary_oam_x_pos(const Ppu2C02* p, const unsigned sprite_index)
{
	return (p->scanline_oam[(sprite_index * 4) + 3]);
}

static void ppu_transfer_oam(Ppu2C02* p, const unsigned index)
{
	if (p->sprites_found == 8) {
		// disable writes to scanline/secondary oam once full
		return;
	}

	memcpy(&p->scanline_oam[p->sprites_found * 4], &p->oam[index * 4], 4); // Copy remaining bytes
}

static void reset_secondary_oam(Ppu2C02* p)
{
	memset(p->scanline_oam, 0xFF, sizeof(p->scanline_oam)); // Reset secondary OAM
	/* Reset internals */
	p->sprite_index = 0;
	p->sprites_found = 0;
	p->stop_early = false;
	p->sprite_zero_scanline = p->sprite_zero_scanline_tmp;
}

static void sprite_evaluation(Ppu2C02* p)
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


/* Sprite 0 hit peek for the next 8 pixels to be rendered
 */
static void sprite_hit_lookahead(Ppu2C02* p)
{
	// Check if sprite is in Y range
	// -1 as Y pos of sprite is delayed until the next scanline
	if ((p->scanline - p->oam[0] - 1) < ppu_sprite_height(p)
		&& (p->scanline > 0)
		&& (p->scanline < 240)) {
		// should be looking @ px 9-16 (1 index) @ cyc 8
		if (((p->cycle - 1) & 0x07) == 0x07) {
			// perform our check on the start of a tile boundry i.e. 0, 8, 16 ...
			// and make sure that the sprite 0 x pos is range
			if (((p->oam[3] / 8) == (p->cycle / 8)) && (p->cycle < 256)) {
				// reset hit_pos
				p->l_sl = 10000;
				p->l_cl = 10000;
				unsigned sp_h_offset = p->oam[3] % 8;
				unsigned sp_v_offset = p->scanline - p->oam[0] - 1;
				for (int i = 0; i < 8; i++) {
					// set to an invalid value
					p->bg_opaque_hit[i] = -10;
					p->sp_opaque_hit[i] = -10;
				}

				for (int mask = 1; mask < 9; mask++) {
					// get correct bit/pixel from bg pattern tables:
					//   h_offset is the x offset to the non-scrolled bg tile
					//     making the bg aligned to the sprite (non-scrolled)
					//   fine_x to get the correct scrolled pixel of bg tile
					//   mask iterates through 8 pixels to get our lookahead
					p->bg_lo_reg = (p->pt_lo_shift_reg >> (mask + p->fine_x + sp_h_offset - 1)) & 0x01;
					p->bg_hi_reg = (p->pt_hi_shift_reg >> (mask + p->fine_x + sp_h_offset - 1)) & 0x01;
					unsigned tmp_pt_lo = read_from_ppu_vram(&p->vram, (ppu_sprite_pattern_table_addr(p) | p->oam[1] << 4) + sp_v_offset);
					unsigned tmp_pt_hi = read_from_ppu_vram(&p->vram, (ppu_sprite_pattern_table_addr(p) | p->oam[1] << 4) + sp_v_offset + 8);
					if (ppu_sprite_height(p) == 16) {
						if (sp_v_offset >= 8) { sp_v_offset += 8; } // avoid overlap w/ hi address space i.e. 0x0008 to 0x000F
						tmp_pt_lo = read_from_ppu_vram(&p->vram, (0x1000 * (p->oam[1] & 0x01)) | ((uint16_t) ((p->oam[1] & 0xFE) << 4) + sp_v_offset));
						tmp_pt_hi = read_from_ppu_vram(&p->vram, (0x1000 * (p->oam[1] & 0x01)) | ((uint16_t) ((p->oam[1] & 0xFE) << 4) + sp_v_offset + 8));
					}

					// Ger correct bit from sprite pattern tables
					// MSB = 1st pixel (not using the reverse_bits function here)
					p->sp_lo_reg = (tmp_pt_lo >> (8 - mask)) & 0x01;
					p->sp_hi_reg = (tmp_pt_hi >> (8 - mask)) & 0x01;
					// flip sprites horizontally
					if (p->oam[2] & 0x40) {
						// already fetched pattern table data just reverse it
						p->sp_lo_reg = reverse_bits[p->sp_lo_reg];
						p->sp_hi_reg = reverse_bits[p->sp_hi_reg];
					}
					// flip sprites vertically
					if (p->oam[2] & 0x80) {
						tmp_pt_lo = read_from_ppu_vram(&p->vram, (ppu_sprite_pattern_table_addr(p) | p->oam[1] << 4) + 7 - sp_v_offset);
						tmp_pt_hi = read_from_ppu_vram(&p->vram, (ppu_sprite_pattern_table_addr(p) | p->oam[1] << 4) + 15 - sp_v_offset);
						// flip the 8x16 sprites
						if (ppu_sprite_height(p) == 16) {
							tmp_pt_lo = read_from_ppu_vram(&p->vram, (0x1000 * (p->oam[1] & 0x01)) | ((uint16_t) ((p->oam[1] & 0xFE) << 4) + 23 - sp_v_offset));
							tmp_pt_hi = read_from_ppu_vram(&p->vram, (0x1000 * (p->oam[1] & 0x01)) | ((uint16_t) ((p->oam[1] & 0xFE) << 4) + 31 - sp_v_offset));
						}
						p->sp_lo_reg = (tmp_pt_lo >> (8 - mask)) & 0x01;
						p->sp_hi_reg = (tmp_pt_hi >> (8 - mask)) & 0x01;
						if (p->oam[2] & 0x40) {
							// already fetched pattern table data just reverse it
							p->sp_lo_reg = reverse_bits[p->sp_lo_reg];
							p->sp_hi_reg = reverse_bits[p->sp_hi_reg];
						}
					}
					// report if any bg and sprite pixel is non-transparent
					// using the p->sprite_pt_xx_shift_reg[0] produces some incorrect values
					// i.e. in bases loaded: after sprite zero hit SL: 118 onwards should
					// report 0xFF when calling the lo_pt[0] instead for 118 I get 0x00
					if (p->bg_lo_reg || p->bg_hi_reg) { p->bg_opaque_hit[mask - 1] = mask - 1; }
					if (p->sp_lo_reg || p->sp_hi_reg) { p->sp_opaque_hit[mask - 1] = p->oam[3] + mask - 1; }

					if ((p->sp_frame_hit_lookahead == false)
						&& (p->bg_opaque_hit[mask - 1] != -10)
						&& (p->sp_opaque_hit[mask - 1] != -10)) {
						p->l_sl = p->scanline;
						p->l_cl = p->oam[3] + mask; // delayed by one tick (hence no minus one)

						// no sprite hit on p->cycle = 256 (x pixel == 255 (when counting from 0))
						if (p->l_cl == 256) {
							p->l_sl = 10000;
							p->l_cl = 10000;
							continue;
						}
						// ignore sprite hits if sprite or bg are masked (leftmost 8 pixels)
						if ((ppu_mask_left_8px_bg(p) || ppu_mask_left_8px_sprite(p))
							&& (p->l_cl < 9)) {
							p->l_sl = 10000;
							p->l_cl = 10000;
							continue;
						}
						p->sp_frame_hit_lookahead = true;
					}
				}
			}
		}
	}
}

/*************************
 * RENDERING             *
 *************************/

void clock_ppu(Ppu2C02* p, Cpu6502* cpu, Display* nes_screen, const bool no_logging)
{
#ifdef __DEBUG__
	if (!no_logging && p->cpu_ppu_io->write_debug) {
		p->cpu_ppu_io->write_debug = false;
		append_ppu_info(p);
	}
#endif /* __DEBUG__ */

	p->cpu_ppu_io->nmi_lookahead = false;
	p->cpu_ppu_io->clear_status = false;

	p->cycle++;
	if (p->cycle > 340) {
		p->cycle = 0; // Reset cycle count to 0, max val = 340

		p->scanline++;
		if (p->scanline > 261) {
			p->scanline = 0; // Reset scanline to 0, max val == 261
			p->odd_frame = !p->odd_frame;
		}
	}

	// set on pre-render scanline and visible frames (0-239)
	if ((p->scanline == 261) || (p->scanline <= 239)) {
		p->cpu_ppu_io->ppu_rendering_period = true;
	} else if (p->scanline == 240) { // only set once, no need for >=
		p->cpu_ppu_io->ppu_rendering_period = false;
	}

	ppu_vblank_warmup_seq(p, cpu);

	// cpu is clocked first, ppu must be updated after the ppu runs its clock
	// as the ppu is supposed to be running at the same time the write to the ppu reg occurs
	// this means a buffer system needs to be implemented to preserve this behaviour
	if (p->cpu_ppu_io->buffer_write) {
		--p->cpu_ppu_io->buffer_counter;
		// buffering a write to enable bg render sets flag
		if (p->cpu_ppu_io->buffer_address == 0x2001 && (p->cpu_ppu_io->buffer_value & 0x08)) {
			if (p->cpu_ppu_io->buffer_counter == 3) {
				cpu->cpu_ppu_io->bg_early_enable_mask = true;
			}
		}

		// buffering a write to disable bg render sets flag
		if (p->cpu_ppu_io->buffer_address == 0x2001 && !(p->cpu_ppu_io->buffer_value & 0x08)) {
			if (p->cpu_ppu_io->buffer_counter == 3) {
				cpu->cpu_ppu_io->bg_early_disable_mask = true;
			}
		}
		if (!p->cpu_ppu_io->buffer_counter) {
			write_ppu_reg(p->cpu_ppu_io->buffer_address, p->cpu_ppu_io->buffer_value, cpu);
			p->cpu_ppu_io->buffer_write = false;
			p->cpu_ppu_io->buffer_counter = 6; // reset to non-zero value
			// clear flags about buffered writes to enable/disable bg rendering
			cpu->cpu_ppu_io->bg_early_enable_mask = false;
			cpu->cpu_ppu_io->bg_early_disable_mask = false;
		}
	}

	// odd frame skip
	if (!cpu->cpu_ppu_io->bg_early_disable_mask
		&& (cpu->cpu_ppu_io->bg_early_enable_mask || (p->cpu_ppu_io->ppu_mask & 0x08))) {
		if (p->odd_frame && p->scanline == 261 && p->cycle == 339) {
			++p->cycle;
		}
	}


	/* NMI, VBlank and ppu_status register handling */
	if (p->scanline == p->nmi_start) {
		if (p->cycle == 0) {
			set_ppu_status_vblank_bit(p->cpu_ppu_io); // In VBlank
			p->cpu_ppu_io->nmi_lookahead = true;
			p->cpu_ppu_io->clear_status = true;
		}
		if (ppu_ctrl_gen_nmi_bit_set(p->cpu_ppu_io)) {
			if (p->cycle == 1) {
				p->cpu_ppu_io->nmi_pending = true;
				p->cpu_ppu_io->nmi_lookahead = true; // nmi is delayed
			} else if (p->cycle == 2) {
				p->cpu_ppu_io->nmi_lookahead = true;
			}
			if (p->cpu_ppu_io->suppress_nmi_flag
			    && (p->cycle == 1 || p->cycle == 2 || p->cycle == 3)) {
				p->cpu_ppu_io->ignore_nmi = true;
			}
		}

		if (p->cpu_ppu_io->ignore_nmi) {
			p->cpu_ppu_io->nmi_pending = false;
		}

		// Must also disable NMI after disabling NMI flag
		if (!ppu_ctrl_gen_nmi_bit_set(p->cpu_ppu_io) && p->cpu_ppu_io->nmi_pending) {
			if (p->cycle < 5) {
				p->cpu_ppu_io->ignore_nmi = true;
				p->cpu_ppu_io->nmi_pending = false;
			}
		}

		// clear VBlank flag if cpu clock is aligned w/ the ppu clock
		// hard coded for NTSC currently
		if (p->cpu_ppu_io->suppress_nmi_flag && (cpu->cycle % 3 == 0)) {
			clear_ppu_status_vblank_bit(p->cpu_ppu_io);
		}
	} else if (p->scanline == 261 && p->cycle == 0) { // Pre-render scanline
		p->cpu_ppu_io->ppu_status &= ~0x40;
	} else if (p->scanline == 261 && p->cycle == 1) { // Pre-render scanline
		// Clear VBlank, sprite hit and sprite overflow flags
		p->cpu_ppu_io->ppu_status &= ~0xE0;
	} else if (p->scanline == 240 && p->cycle == 340) {
		p->cpu_ppu_io->nmi_lookahead = true;
	} else if (p->scanline == 240 && (p->cycle == 339 || p->cycle == 340)) {
		p->cpu_ppu_io->clear_status = true;
	}


	p->cpu_ppu_io->suppress_nmi_flag = false;


	if (ppu_show_bg(p) || ppu_show_sprite(p)) {
		// Sprites are evaluated for either BG or sprite rendering
		if (p->scanline <= 239) { // Visible scanlines
			if (p->cycle > 64 && p->cycle <= 256) {
				sprite_evaluation(p);
			}
		}
	}

	// Fill pixel buffer and then render frame
	if (p->scanline <= 239) { // Visible scanlines
		if (p->cycle <= 256 && (p->cycle != 0)) { // 0 is an idle cycle
			render_pixel(p); // Render pixel every cycle
		}
	} else if (p->scanline == 240 && p->cycle == 0) {
		draw_pixels(pixels, nes_screen);  // Render frame
	}

	/* Process BG Scanlines */
	if (ppu_show_bg(p)) {
		if (p->scanline <= 239) { // Visible scanlines
			if (p->cycle <= 256 && (p->cycle != 0)) { // 0 is an idle cycle
				// reload at shift registers when we move onto a new tile
				// (the original pixels 9-16 in the pipeline)
				// e.g. if fine_x == 7 then 1 bit of the first tile is rendered
				// then we reload the shift reg on cycle 1 w/ new attribute data
				// for the next tile
				if (!((p->cycle + p->fine_x) % 8)) {
					fill_attribute_shift_reg(p, p->nt_addr_current, p->at_current);
				}
				// BG STUFF
				switch ((p->cycle - 1) & 0x07) {
				case 0: // Cycle 1, 2 (and + 8)
					fetch_nt_byte(p);
					break;
				case 2: // Cycle 3, 4 (and + 8)
					fetch_at_byte(p);
					break;
				case 4: // Cycle 5, 6 (and + 8)
					fetch_pt_lo(p);
					break;
				case 6: // Cycle 7 (and + 8)
					fetch_pt_hi(p);
					break;
				case 7: // Cycle 8 (and +8)
					// 8 Shifts should have occured by now, load new data
					// Load latched values into upper byte of shift regs
					p->pt_lo_shift_reg |= (uint16_t) (p->pt_lo_latch << 8);
					p->pt_hi_shift_reg |= (uint16_t) (p->pt_hi_latch << 8);
					// Used to fill at shift registers later
					p->at_current = p->at_latch;
					p->nt_addr_current = p->vram_addr;
					break;
				}
			} else if (p->cycle == 257) {
				// Copy horz scroll bits from t
				p->vram_addr = (p->vram_addr & ~0x041F) | (p->vram_tmp_addr & 0x041F);
			} else if (p->cycle >= 321 && p->cycle <= 336) { // 1st 16 pixels of next scanline
				switch ((p->cycle - 1) & 0x07) {
				case 0: // Cycle 321 (and + 8)
					fetch_nt_byte(p);
					break;
				case 1: // Cycle 322 (and +8)
					fill_attribute_shift_reg(p, p->nt_addr_current, p->at_current);
					break;
				case 2: // Cycle 323, 324 (and +8)
					fetch_at_byte(p);
					break;
				case 4: // Cycle 325, 326 (and +8)
					fetch_pt_lo(p);
					break;
				case 6: // Cycle 327 (and +8)
					fetch_pt_hi(p);
					// Load latched values into upper byte of shift regs
					p->pt_hi_shift_reg >>= 8;
					p->pt_lo_shift_reg >>= 8;
					p->pt_hi_shift_reg |= (uint16_t) (p->pt_hi_latch << 8);
					p->pt_lo_shift_reg |= (uint16_t) (p->pt_lo_latch << 8);
					// Used to fill at shift registers later
					p->at_current = p->at_latch;
					p->nt_addr_current = p->vram_addr;
					break;
				case 7: // Cycle 328 (and +8)
					break;
				}
			}
		} else if (p->scanline == 261) {
			// Pre-render scanline
			if (p->cycle <= 256 && (p->cycle != 0)) { // 0 is an idle cycle
				switch ((p->cycle - 1) & 0x07) {
				case 0: // Cycle 256, 257 (and +8)
					fetch_nt_byte(p);
					break;
				case 2: // Cycle 258, 259 (and +8)
					fetch_at_byte(p);
					break;
				case 4: // Cycle 260, 261 (and +8)
					fetch_pt_lo(p);
					break;
				case 6: // Cycle 262 (and +8)
					fetch_pt_hi(p);
					break;
				case 7: // Cycle 263 (and +8)
					// No need to fill shift registers as nothing is being rendered here
					break;
				}
			} else if (p->cycle == 257) {
				// Copy horz scroll bits from t
				p->vram_addr = (p->vram_addr & ~0x041F) | (p->vram_tmp_addr & 0x041F);
			} else if (p->cycle >= 280 && p->cycle <= 304) {
				// Copy horz scroll bits from t
				p->vram_addr = (p->vram_addr & ~0x7BE0) | (p->vram_tmp_addr & 0x7BE0);
			} else if (p->cycle >= 321 && p->cycle <= 336) { // 1st 16 pixels of next scanline
				switch ((p->cycle - 1) & 0x07) {
				case 0: // Cycle 321 (and + 8)
					fetch_nt_byte(p);
					break;
				case 1: // Cycle 322 (and +8)
					fill_attribute_shift_reg(p, p->nt_addr_current, p->at_current);
					break;
				case 2: // Cycle 323, 324 (and +8)
					fetch_at_byte(p);
					break;
				case 4: // Cycle 325, 326 (and +8)
					fetch_pt_lo(p);
					break;
				case 6: // Cycle 327 (and +8)
					fetch_pt_hi(p);
					// Load latched values into upper byte of shift regs
					p->pt_hi_shift_reg >>= 8;
					p->pt_lo_shift_reg >>= 8;
					p->pt_hi_shift_reg |= (uint16_t) (p->pt_hi_latch << 8);
					p->pt_lo_shift_reg |= (uint16_t) (p->pt_lo_latch << 8);
					// Used to fill at shift registers later
					p->at_current = p->at_latch;
					p->nt_addr_current = p->vram_addr;
					break;
				case 7: // Cycle 328 (and +8)
					break;
				}
			}
		}
	}

	/* Process Sprites */
	if (ppu_show_sprite(p)) {
		if (p->scanline <= 239) { // Visible scanlines
			if (p->cycle <= 64 && (p->cycle != 0)) {
				reset_secondary_oam(p);
			} else if (p->cycle > 256 && p->cycle <= 320) { // Sprite data fetches
				static unsigned count = 0; // Counts 8 secondary OAM
				count = sprite_fetch_index(p); // keep count within array bounds
				static int sprite_y_offset = 0;
				switch ((p->cycle - 1) & 0x07) {
				case 0:
					// Garbage NT byte - no need to emulate
					break;
				case 1:
					// When not in range the sprite is filled w/ FF
					p->sprite_addr = ppu_sprite_pattern_table_addr(p)
					               | (uint16_t) secondary_oam_tile_number(p, count) << 4;
					// 8x16 sprites don't use ppu_ctrl to determine base pt address
					if (ppu_sprite_height(p) == 16) {
						// Bit 0 determines base pt address, 0x1000 or 0x000
						p->sprite_addr = (0x1000 * (secondary_oam_tile_number(p, count) & 0x01))
					                   | (uint16_t) (secondary_oam_tile_number(p, count) & 0xFE) << 4;
					}
					sprite_y_offset = p->scanline - secondary_oam_y_pos(p, count);
					if (sprite_y_offset < 0) { // Keep address static until we reach the scanline in range
						sprite_y_offset = 0; // Stops out of bounds access for -1
					}

					// addr for rows 9-16 of 8x16 sprites
					if (ppu_sprite_height(p) == 16) {
						if (sprite_y_offset >= 8) { sprite_y_offset += 8; }
					}
					p->sprite_addr += sprite_y_offset;

					break;
				case 2:
					// Garbage AT byte - no need to emulate
					p->sprite_at_latches[count] = secondary_oam_at_byte(p, count);

					if (p->sprite_at_latches[count] & 0x80) { // Flip vertical pixles
						// undo y_offset, then go from y_offset_max down to 0
						// e.g. 0-7 is now flipped to 7-0 (for sprites 8px high)
						p->sprite_addr = p->sprite_addr - sprite_y_offset
						                 + (ppu_sprite_height(p) - 1)
						                 - (sprite_y_offset % ppu_sprite_height(p));
					}
					break;
				case 3:
					// Read X Pos (In NES it's re-read until the 8th cycle)
					p->sprite_x_counter[count] = secondary_oam_x_pos(p, count);
					break;
				case 4:
					// Fetch sprite low pt
					p->sprite_pt_lo_shift_reg[count] = reverse_bits[read_from_ppu_vram(&p->vram, p->sprite_addr)];
					if ((p->sprite_at_latches[count] & 0x40)) { // Flip horizontal pixels
						// already fetched pattern table data just reverse bits again
						p->sprite_pt_lo_shift_reg[count] = reverse_bits[p->sprite_pt_lo_shift_reg[count]];
					}
					break;
				case 6:
					// Fetch sprite hi pt, turn into function once all attribute data is processed
					p->sprite_pt_hi_shift_reg[count] = reverse_bits[read_from_ppu_vram(&p->vram, p->sprite_addr + 8)];
					if ((p->sprite_at_latches[count] & 0x40)) { // Flip horizontal pixels
						// already fetched pattern table data just reverse bits again
						p->sprite_pt_hi_shift_reg[count] = reverse_bits[p->sprite_pt_hi_shift_reg[count]];
					}
					break;
				case 7: // 8th Cycle
					break;
				}
			}
		} else if (p->scanline == 261) { // Pre-render scanline
			p->sp_frame_hit_lookahead = false;
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


	if ((ppu_show_bg(p) && ppu_show_sprite(p)) && !sprite_overflow_occured(p)) {
		sprite_hit_lookahead(p);
	}

	if (((int) p->scanline == p->l_sl) && (p->cycle == p->l_cl)) {
		p->cpu_ppu_io->ppu_status |= 0x40; // Sprite #0 hit
		p->l_sl = 10000;
		p->l_cl = 10000;
	}

	// increment coarse X and Y scrolling pos on visible scanlines and if rendering is enabled
	if (cpu->cpu_ppu_io->ppu_rendering_period && ppu_mask_bg_or_sprite_enabled(cpu->cpu_ppu_io)) {
		if (p->cycle <= 256 && (p->cycle != 0)) {
			if (((p->cycle - 1) & 0x07) == 0x07) { // cycles divisble by 8
				inc_horz_scroll(p->cpu_ppu_io);
			} if (p->cycle == 256) {
				inc_vert_scroll(p->cpu_ppu_io);
			}
		} else if (p->cycle >= 321 && p->cycle <= 336) { // 1st 16 pixels of next scanline
			if (((p->cycle - 1) & 0x07) == 0x07) { // cycles divisble by 8
				inc_horz_scroll(p->cpu_ppu_io);
			} if (p->cycle == 256) {
				inc_vert_scroll(p->cpu_ppu_io);
			}
		}
	}
}
