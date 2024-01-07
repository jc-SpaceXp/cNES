#ifndef __NES_PPU__
#define __NES_PPU__

#include "ppu_fwd.h"
#include "cpu_fwd.h"
#include "gui_fwd.h"
#include "cpu_ppu_interface_fwd.h"

#include <stdbool.h>
#include <inttypes.h>

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

typedef enum PpuNametableMirroringType {
	HORIZONTAL,
	VERTICAL,
	SINGLE_SCREEN_A, // "lower nametable"
	SINGLE_SCREEN_B, // "upper nametable"
	FOUR_SCREEN,
} PpuNametableMirroringType;


// Non-mirrored memory mapping of ppu vram
struct PpuMemoryMap {
	uint8_t* pattern_table_0k; // vram: 0x0000 to 0x0FFF
	uint8_t* pattern_table_4k; // vram: 0x1000 to 0x1FFF
	uint8_t nametable_A[0x0400]; // vram: 0x2000 to 0x2400
	uint8_t nametable_B[0x0400]; // second pattern table, address depends on nametable mirroring
	uint8_t palette_ram[0x0020]; // vram: 0x3F00 to 0x3F1F

	// store here, instead of inside ppu struct
	// allows a generic struct function to read/write the nametables
	// from both the cpu and ppu calling functions (and saves on code duplication)
	uint8_t (*nametable_0)[0x0400];
	uint8_t (*nametable_1)[0x0400];
	uint8_t (*nametable_2)[0x0400];
	uint8_t (*nametable_3)[0x0400];
};

struct BackgroundRenderingInternals {
	// Internal data latches and shift registers
	uint8_t pt_lo_latch; // Most recent fetch pt_lo fetch
	uint8_t pt_hi_latch;
	uint16_t pt_lo_shift_reg; // Stores 16 pixels in the pipeline, 1st 8 pixels to be rendered are in lowest byte
	uint16_t pt_hi_shift_reg;
	uint8_t at_lo_shift_reg; // Matches lowest 8 bits for the pattern table shift register
	uint8_t at_hi_shift_reg;
	uint8_t nt_byte; // NT byte that is rendered 16 PPU cycles later
	uint8_t at_latch; // AT byte that is rendered 16 PPU cycles later

	// Helpers
	uint8_t at_current; // AT byte for pixels 1-8 in pipeline
	uint16_t nt_addr_current; // Current tile address for pixels 1-8 in pipeline
};

struct Ppu2C02 {
	// Memory mapped I/O
	CpuPpuShare* cpu_ppu_io;

	// Memory
	struct PpuMemoryMap vram;
	uint8_t oam[256]; // OAM Address Space (Sprite RAM)

	// Sprites
	uint8_t scanline_oam[32]; // Secondary OAM, change to scanline
	uint8_t oam_read_buffer;
	unsigned sprites_found; // Number of sprites found in next scanlie: MAX 8
	unsigned sprite_index; // Max 63 (0 indexed)
	bool stop_early;
	bool sprite_zero_hit;
	unsigned sprite_zero_scanline; // Scanlines of sprite 0 if any
	unsigned sprite_zero_scanline_tmp; // Scanlines of sprite 0 if any
	unsigned hit_scanline; // Needed because Y is calculated 1 scanline ahead
	unsigned hit_cycle; // Needed because Y is calculated 1 scanline ahead
	uint16_t sprite_addr;
	uint8_t sprite_at_latches[8]; // Holds attribute byte data for 8 sprites
	uint8_t sprite_pt_lo_shift_reg[8];
	uint8_t sprite_pt_hi_shift_reg[8];
	uint8_t sprite_x_counter[8]; // X pos of sprite (decremented every 8 cycles

	// BACKROUND
	uint16_t vram_addr; // VRAM address - LoopyV (v)
	uint16_t vram_tmp_addr; // Temp VRAM address - LoopyT (t)
	uint8_t fine_x; // Fine X Scroll - only lower 4 bits are used

	struct BackgroundRenderingInternals bkg_internals;

	PpuNametableMirroringType nametable_mirroring;

	// lookahead hit
	unsigned bg_lo_reg;
	unsigned bg_hi_reg;
	unsigned sp_lo_reg;
	unsigned sp_hi_reg;
	int bg_opaque_hit[8];
	int sp_opaque_hit[8];
	bool sp_frame_hit_lookahead;
	int l_sl;
	int l_cl;

	uint32_t scanline; // Pre-render = 261, visible = 0-239, post-render 240-260
	uint32_t nmi_start; // Scanline in which NMI starts, set value depending on NTSC or PAL
	const uint32_t nmi_end; // Scanline in which NMI end
	uint16_t cycle; // PPU Cycles, each PPU mem access takes 2 cycles
	uint16_t old_cycle;
	uint32_t old_scanline;
	bool odd_frame;
};


/* Initialise Function */
Ppu2C02* ppu_allocator(void);
int ppu_init(Ppu2C02* ppu, CpuPpuShare* cp);

/* Debug Functions */
void append_ppu_info(Ppu2C02* cpu);
void debug_ppu_regs(Cpu6502* cpu);
void ppu_mem_hexdump_addr_range(const Ppu2C02* p, const enum PpuMemoryTypes ppu_mem, unsigned start_addr, uint16_t end_addr);
uint16_t nametable_x_offset_address(const unsigned coarse_x);
uint16_t nametable_y_offset_address(const unsigned coarse_y);

/* Read & Write Functions */
void write_to_ppu_vram(struct PpuMemoryMap* mem, unsigned addr, uint8_t data);
uint8_t read_from_ppu_vram(const struct PpuMemoryMap* mem, unsigned addr);

void inc_vert_scroll(CpuPpuShare* cpu_ppu_io);
void inc_horz_scroll(CpuPpuShare* cpu_ppu_io);


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

void reset_secondary_oam(Ppu2C02* p);
void sprite_evaluation(Ppu2C02* p);
void get_sprite_address(Ppu2C02* ppu, int* y_offset, unsigned count);
void flip_sprites_vertically(Ppu2C02* ppu, int y_offset);
void load_sprite_pattern_table_data(Ppu2C02* ppu, uint8_t* pattern_shift_reg
                                   , unsigned sprite_number, uint16_t sprite_addr);

void get_bkg_pixel(Ppu2C02* ppu, uint8_t* colour_ref);
void get_sprite_pixel(Ppu2C02* ppu, uint8_t* colour_ref);


void clock_ppu(Ppu2C02* p, Cpu6502* cpu, Sdl2DisplayOutputs* cnes_windows);


#endif /* __NES_PPU__ */
