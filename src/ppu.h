#ifndef __NES_PPU__
#define __NES_PPU__

#include "extern_structs.h"
#include "cpu.h"

#include <stdbool.h>

#ifndef KiB
#define KiB (1024U)
#endif /* KiB */


enum PpuMemoryTypes {
	VRAM,
	PRIMARY_OAM,
	SECONDARY_OAM,
	PATTERN_TABLE_0,
	PATTERN_TABLE_1,
	NAMETABLE_A,
	NAMETABLE_B,
	PALETTE_RAM
};

/* Initialise Function */
Ppu2C02* ppu_init(CpuPpuShare* cp);

/* Debug Functions */
void debug_ppu_regs(Cpu6502* cpu);
void ppu_mem_hexdump_addr_range(const Ppu2C02* p, const enum PpuMemoryTypes ppu_mem, unsigned start_addr, uint16_t end_addr);
uint16_t nametable_x_offset_address(const unsigned coarse_x);
uint16_t nametable_y_offset_address(const unsigned coarse_y);

/* Read & Write Functions */
uint8_t read_ppu_reg(const uint16_t addr, Cpu6502* cpu); /* For addresses exposed to CPU */
void delay_write_ppu_reg(const uint16_t addr, const uint8_t data, Cpu6502* cpu); /* For addresses exposed to CPU */
void write_ppu_reg(const uint16_t addr, const uint8_t data, Cpu6502* cpu); /* For addresses exposed to CPU */
void write_to_ppu_vram(struct PpuMemoryMap* mem, unsigned addr, uint8_t data);
uint8_t read_from_ppu_vram(const struct PpuMemoryMap* mem, unsigned addr);

// Cpu/ppu Registers
uint8_t ppu_vram_addr_inc(const CpuPpuShare* cpu_ppu_io);
uint16_t ppu_base_nt_address(const CpuPpuShare* cpu_ppu_io);
uint16_t ppu_base_pt_address(const CpuPpuShare* cpu_ppu_io);
uint16_t ppu_sprite_pattern_table_addr(const CpuPpuShare* cpu_ppu_io);
uint8_t ppu_sprite_height(const CpuPpuShare* cpu_ppu_io);
bool ppu_show_bg(const CpuPpuShare* cpu_ppu_io);
bool ppu_show_sprite(const CpuPpuShare* cpu_ppu_io);
bool ppu_mask_left_8px_bg(const CpuPpuShare* cpu_ppu_io);
bool ppu_mask_left_8px_sprite(const CpuPpuShare* cpu_ppu_io);
bool ppu_show_greyscale(const CpuPpuShare* cpu_ppu_io);
bool sprite_overflow_occured(const CpuPpuShare* cpu_ppu_io);

// Rendering functions
void fetch_nt_byte(const struct PpuMemoryMap* vram
                  , uint16_t vram_addr
                  , struct BackgroundRenderingInternals* bkg_internals);
void fetch_at_byte(const struct PpuMemoryMap* vram
                  , uint16_t vram_addr
                  , struct BackgroundRenderingInternals* bkg_internals);
void fetch_pt_lo(const struct PpuMemoryMap* vram
                , uint16_t vram_addr
                , uint16_t base_pt_address
                , struct BackgroundRenderingInternals* bkg_internals);
void fetch_pt_hi(const struct PpuMemoryMap* vram
                , uint16_t vram_addr
                , uint16_t base_pt_address
                , struct BackgroundRenderingInternals* bkg_internals);
void fill_attribute_shift_reg(uint16_t nametable_addr
                             , uint8_t attribute_data
                             , struct BackgroundRenderingInternals* bkg_internals);
void set_rgba_pixel_in_buffer(uint32_t* pixel_buffer, unsigned int max_width
                             , unsigned int x_pos, unsigned int y_pos
                             , unsigned int rgb, uint8_t alpha);


void clock_ppu(Ppu2C02* p, Cpu6502* cpu, Sdl2DisplayOutputs* cnes_windows, const bool no_logging);


#endif /* __NES_PPU__ */
