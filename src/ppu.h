#ifndef __NES_PPU__
#define __NES_PPU__

#include "extern_structs.h"
#include "cpu.h"

#include <stdbool.h>

#ifndef KiB
#define KiB (1024U)
#endif /* KiB */


enum Memory {
	PRIMARY_OAM,
	SECONDARY_OAM,
	PATTERN_TABLE_1
};

/* Global defintions */
static const uint8_t reverse_bits[256];
static const unsigned int palette[64];

/* Initialise Function */
Ppu2A03* ppu_init(CpuPpuShare* cp);
void ppu_reset(int start, Ppu2A03* p, Cpu6502* cpu); /* Emulates reset/warm-up of PPU */

/* Debug Functions */
void append_ppu_info(Ppu2A03* PPU);
void debug_ppu_regs(Cpu6502* cpu);
void ppu_mem_16_byte_viewer(Ppu2A03* PPU, unsigned start_addr, unsigned total_rows);
void OAM_viewer(Ppu2A03* PPU, enum Memory ppu_mem); // rename to VRAM viewer?

/* Read & Write Functions */
uint8_t read_ppu_reg(uint16_t addr, Cpu6502* cpu); /* For addresses exposed to CPU */
void delay_write_ppu_reg(uint16_t addr, uint8_t data, Cpu6502* cpu); /* For addresses exposed to CPU */
void write_ppu_reg(uint16_t addr, uint8_t data, Cpu6502* cpu); /* For addresses exposed to CPU */
void write_vram(uint8_t data, Cpu6502* cpu);

void read_2002(Cpu6502* cpu);
void read_2007(Cpu6502* cpu);

void write_2000(uint8_t data, Cpu6502* cpu); /* PPU_CTRL */
void write_2003(uint8_t data, Cpu6502* cpu); /* OAM_DATA */
void write_2004(uint8_t data, Cpu6502* cpu); /* OAM_ADDR */
void write_2005(uint8_t data, Cpu6502* cpu); /* PPU_SCROLL */
void write_2006(uint8_t data, Cpu6502* cpu); /* PPU_ADDR */
void write_2007(uint8_t data, Cpu6502* cpu); /* PPU_DATA */
void write_4014(uint8_t data, Cpu6502* cpu); /* DMA_DATA */


/* PPU_CTRL FUNCTIONS */
uint8_t ppu_vram_addr_inc(Cpu6502* cpu);
uint16_t ppu_base_nt_address(Ppu2A03* p);
uint16_t ppu_base_pt_address(Ppu2A03* p);
uint16_t ppu_sprite_pattern_table_addr(Ppu2A03* p);
uint8_t ppu_sprite_height(Ppu2A03* p);

/* PPU_MASK FUNCTIONS */
bool ppu_show_bg(Ppu2A03* p);
bool ppu_show_sprite(Ppu2A03* p);
bool ppu_mask_left_8px_bg(Ppu2A03* p);
bool ppu_mask_left_8px_sprite(Ppu2A03* p);
bool ppu_show_greyscale(Ppu2A03* p);

/* PPU_STATUS FUNCTIONS */
bool sprite_overflow_occured(Ppu2A03* p);

void inc_vert_scroll(Ppu2A03* p);
void inc_horz_scroll(Ppu2A03* p);

void fetch_nt_byte(Ppu2A03* p);
void fetch_at_byte(Ppu2A03* p);
void fetch_pt_lo(Ppu2A03* p);
void fetch_pt_hi(Ppu2A03* p);

void render_pixel(Ppu2A03* p);

void ppu_transfer_oam(Ppu2A03* p, unsigned index);
void reset_secondary_oam(Ppu2A03* p);
void sprite_evaluation(Ppu2A03* p);

void clock_ppu(Ppu2A03* p, Cpu6502* cpu, Display* nes_screen);


#endif /* __NES_PPU__ */
