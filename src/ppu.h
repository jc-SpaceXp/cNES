#ifndef __NES_PPU__
#define __NES_PPU__

#include "extern_structs.h"
#include "cpu.h"
//#include "gui.h"

#include <stdbool.h>
//#include <stdio.h>

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
PPU_Struct *ppu_init(CpuPpuShare* cp);
//void debug_entry(PPU_Struct *p);
//void debug_exit(PPU_Struct *p);
void ppu_reset(int start, PPU_Struct *p, Cpu6502* CPU); /* Emulates reset/warm-up of PPU */

/* Debug Functions */
void append_ppu_info(PPU_Struct* PPU);
void debug_ppu_regs(Cpu6502* CPU);
void ppu_mem_16_byte_viewer(PPU_Struct* PPU, unsigned start_addr, unsigned total_rows);
void OAM_viewer(PPU_Struct* PPU, enum Memory ppu_mem); // rename to VRAM viewer?

/* Read & Write Functions */
//uint8_t read_PPU();
uint8_t read_ppu_reg(uint16_t addr, Cpu6502* CPU); /* For addresses exposed to CPU */
void write_ppu_reg(uint16_t addr, uint8_t data, Cpu6502* CPU); /* For addresses exposed to CPU */
void write_vram(uint8_t data, Cpu6502* CPU);

void read_2002(Cpu6502* CPU);
//uint8_t read_2004(uint16_t addr, PPU_Struct *p);
void read_2007(Cpu6502 *CPU);

void write_2000(uint8_t data, Cpu6502* CPU); /* PPU_CTRL */
void write_2003(uint8_t data, Cpu6502* CPU); /* OAM_DATA */
void write_2004(uint8_t data, Cpu6502* CPU); /* OAM_ADDR */
void write_2005(uint8_t data, Cpu6502* CPU); /* PPU_SCROLL */
void write_2006(uint8_t data, Cpu6502* CPU); /* PPU_ADDR */
void write_2007(uint8_t data, Cpu6502* CPU); /* PPU_DATA */
void write_4014(uint8_t data, Cpu6502* CPU); /* DMA_DATA */


/* PPU_CTRL FUNCTIONS */
//uint16_t ppu_nametable_addr();
uint8_t ppu_vram_addr_inc(Cpu6502* CPU);
uint16_t ppu_base_nt_address(PPU_Struct *p);
uint16_t ppu_base_pt_address(PPU_Struct *p);
uint16_t ppu_sprite_pattern_table_addr(PPU_Struct *p);
//uint16_t ppu_bg_pattern_table_addr();
uint8_t ppu_sprite_height(PPU_Struct *p);
//bool ppu_gen_nmi();

/* PPU_MASK FUNCTIONS */
//bool ppu_show_greyscale();
//bool ppu_bg_show_left_8();
//bool ppu_sprite_show_left_8();
bool ppu_show_bg(PPU_Struct *p);
bool ppu_show_sprite(PPU_Struct *p);
//bool ppu_intense_reds();
//bool ppu_intense_greens();
//bool ppu_intense_blues();

/* PPU_CTRL FUNCTIONS */
//bool ppu_sprite_overflow();
//bool ppu_sprite_0_hit();
//bool ppu_vblank();

void inc_vert_scroll(PPU_Struct *p);
void inc_horz_scroll(PPU_Struct *p);

void fetch_nt_byte(PPU_Struct *p);
void fetch_at_byte(PPU_Struct *p);
void fetch_pt_lo(PPU_Struct *p);
void fetch_pt_hi(PPU_Struct *p);

void render_pixel(PPU_Struct *p);

void ppu_transfer_oam(PPU_Struct* p, unsigned index);
void reset_secondary_oam(PPU_Struct* p);
void sprite_evaluation(PPU_Struct* p);

void ppu_tick(PPU_Struct *p);
void ppu_step(PPU_Struct *p, Cpu6502* CPU, Display* nes_screen);


#endif /* __NES_PPU__ */
