#ifndef __CPU_PPU_INTERFACE__
#define __CPU_PPU_INTERFACE__

#include "cpu_fwd.h"
#include "ppu.h" // need full header for enum used in cpu/ppu struct
#include "cpu_ppu_interface_fwd.h"

#include <stdbool.h>
#include <inttypes.h>


struct CpuPpuShare {
	// Registers
	uint8_t ppu_ctrl;    // $2000
	uint8_t ppu_mask;    // $2001
	uint8_t ppu_status;  // $2002
	uint8_t oam_addr;    // $2003
	uint8_t oam_data;    // $2004
	uint8_t ppu_scroll;  // $2005
	uint8_t ppu_addr;    // $2006
	uint8_t ppu_data;    // $2007
	uint8_t oam_dma;     // $4014

	// Internal flags
	bool nmi_pending; // PPU indicates if a NMI is pending, CPU then services that request
	bool dma_pending; // PPU indicates if a DMA is pending, CPU then services that request
	bool suppress_nmi_flag;
	bool ignore_nmi;
	bool clear_status;
	bool bg_early_enable_mask; // When true this represents the buffered/delayed writes for $2001 when enabling BG rendering
	bool bg_early_disable_mask; // Same as above except for disabling BG rendering
	bool ppu_rendering_period; // Set true for scalines 0-239 and pre-render scanline, otherwise false

	// cpu/ppu nmi synchronisation, when the cpu runs its clock it can be
	// out odf sync with the ppu by 3 ppu clocks, this is set to true for
	// the last 3 ppu clocks before a nmi is latched
	// Is set on (SL/PPU_CYC) 239/340, 240/0 and 240/1
	bool nmi_lookahead;

	// more cpu/ppu synchronisation
	bool buffer_write;
	unsigned buffer_counter;
	unsigned buffer_address;
	uint8_t buffer_value;

	unsigned nmi_cycles_left;

	struct PpuMemoryMap* vram;
	uint8_t* oam;
	uint8_t buffer_2007;
	uint8_t return_value;
	uint16_t* vram_addr;
	uint16_t* vram_tmp_addr;
	PpuNametableMirroringType* nametable_mirroring;
	uint8_t* fine_x;
	bool write_toggle;
};

CpuPpuShare* cpu_ppu_io_allocator(void);
int cpu_ppu_io_init(CpuPpuShare* cpu_ppu_io);
void map_ppu_data_to_cpu_ppu_io(CpuPpuShare* cpu_ppu_io, Ppu2C02* ppu);

bool ppu_status_vblank_bit_set(const CpuPpuShare* cpu_ppu_io);
bool ppu_ctrl_gen_nmi_bit_set(const CpuPpuShare* cpu_ppu_io);
void clear_ppu_status_vblank_bit(CpuPpuShare* cpu_ppu_io);
void set_ppu_status_vblank_bit(CpuPpuShare* cpu_ppu_io);
bool ppu_mask_bg_or_sprite_enabled(const CpuPpuShare* cpu_ppu_io);

void cpu_writes_to_vram(uint8_t data, unsigned chr_ram_size, CpuPpuShare* cpu_ppu_io);

/* Read Functions */
void read_2002(CpuPpuShare* cpu_ppu_io);
void read_2004(CpuPpuShare* cpu_ppu_io);
void read_2007(CpuPpuShare* cpu_ppu_io);

/* Write Functions */
void write_2000(const uint8_t data, CpuPpuShare* cpu_ppu_io);
void write_2003(const uint8_t data, CpuPpuShare* cpu_ppu_io);
void write_2004(const uint8_t data, CpuPpuShare* cpu_ppu_io);
void write_2005(const uint8_t data, CpuPpuShare* cpu_ppu_io);
void write_2006(const uint8_t data, CpuPpuShare* cpu_ppu_io);
void write_2007(const uint8_t data, unsigned chr_ram_size, CpuPpuShare* cpu_ppu_io);
void write_4014(const uint8_t data, Cpu6502* cpu);

/**
 * PPU_CTRL
 */
uint8_t ppu_vram_addr_inc(const CpuPpuShare* cpu_ppu_io);
uint16_t ppu_base_nt_address(const CpuPpuShare* cpu_ppu_io);
uint16_t ppu_base_pt_address(const CpuPpuShare* cpu_ppu_io);
uint16_t ppu_sprite_pattern_table_addr(const CpuPpuShare* cpu_ppu_io);
uint8_t ppu_sprite_height(const CpuPpuShare* cpu_ppu_io);

/**
 * PPU_MASK
 */

bool ppu_show_bg(const CpuPpuShare* cpu_ppu_io);
bool ppu_show_sprite(const CpuPpuShare* cpu_ppu_io);
bool ppu_mask_left_8px_bg(const CpuPpuShare* cpu_ppu_io);
bool ppu_mask_left_8px_sprite(const CpuPpuShare* cpu_ppu_io);
bool ppu_show_greyscale(const CpuPpuShare* cpu_ppu_io);

/**
 * PPU_STATUS
 */

bool sprite_overflow_occured(const CpuPpuShare* cpu_ppu_io);

#endif /* __CPU_PPU_INTERFACE__ */
