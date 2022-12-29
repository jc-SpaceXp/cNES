#ifndef __EXTERN_STRUCTS__
#define __EXTERN_STRUCTS__

#include "SDL2/SDL.h"

#include <stdbool.h>
#include <stdint.h>

#ifndef KiB
#define KiB (1024U)
#endif /* KiB */

#define CPU_MEMORY_SIZE  65536U // Total memory available to the CPU

typedef enum {
	HEADERLESS,
	BAD_INES,
	INES,
	NES_2,
} HeaderFormat;

/* FLAG 9 in iNES header */
typedef enum {
	NTSC = 0,
	PAL = 1,
} VideoType;

typedef enum {
	HORIZONTAL,
	VERTICAL,
	SINGLE_SCREEN_A, // "lower nametable"
	SINGLE_SCREEN_B, // "upper nametable"
	FOUR_SCREEN,
} PpuNametableMirroringType;

/* Struct for the different banks of memory present in a ROM i.e CHR_RAM
 * Data represents the actual data present in the bank and size represents
 * how large this data is in KiB.
 */
typedef struct {
	uint8_t* data;
	uint32_t size;
} Memory;

typedef struct {
	uint8_t* data;
	uint32_t rom_size;  // either CHR RAM or CHR ROM is used
	uint32_t ram_size;  // ... one of these should hold 0
} ChrMemory;

typedef struct Cartridge {
	VideoType video_mode;
	HeaderFormat header;
	Memory prg_rom;  // Program ROM, sent to CPU
	Memory prg_ram;  // Program RAM, sent to CPU
	Memory trainer;  // Trainer data (can be used by some mappers, rarely)
	ChrMemory chr;  // Sprite and background pattern tables, sent to PPU
	bool non_volatile_mem;  // battery and other types of non-volatile memory
} Cartridge;


// Shared mapper/cpu struct
typedef struct {
	// Mirror data from Cart struct
	Memory* prg_rom;
	Memory* prg_ram;
	ChrMemory* chr;
	unsigned mapper_number;

	// currently these are only MMC1 specific (struct may change later)
	unsigned prg_rom_bank_size;
	unsigned chr_bank_size;

	// prg_rom helpers
	bool prg_low_bank_fixed;  // Bank $8000 to $BFFF is fixed
	bool prg_high_bank_fixed; // Bank $C000 to $FFFF is fixed

	bool enable_prg_ram;
} CpuMapperShare;

// Shared CPU/PPU struct
typedef struct {
	/* Registers */
	uint8_t ppu_ctrl;    // $2000
	uint8_t ppu_mask;    // $2001
	uint8_t ppu_status;  // $2002
	uint8_t oam_addr;    // $2004
	uint8_t oam_data;    // $2004
	uint8_t ppu_scroll;  // $2005
	uint8_t ppu_addr;    // $2006
	uint8_t ppu_data;    // $2007
	uint8_t oam_dma;     // $4014

	/* Flags */
	bool nmi_pending;  // PPU indidcates if an nmi is pending, CPU then services the request
	bool dma_pending;  // PPU indidcates if an dma is pending, CPU then services the request
	bool suppress_nmi_flag;
	bool ignore_nmi;
	bool clear_status;
	bool write_debug;  // Trigger debug of PPU only when the CPU writes to the disassembler
	bool bg_early_enable_mask; // when true this represents the buffered/delayed $2001 write for when bg rendering has been enabled
	bool bg_early_disable_mask; // same as above but for disabling bg rendering
	bool ppu_rendering_period; // set true for scanlines 0-239 and pre-render scanline, otherwise false

	// cpu/ppu nmi synchronisation, when the cpu runs its clock it can be out of sync
	// with the ppu by 3 ppu clocks, this is set to true for the last 3 ppu clocks
	// before a NMI is latched (includes [SL/PPU_CYC] 239/340, 240/0, 240/1)
	bool nmi_lookahead;

	// more cpu/ppu synchronisation
	bool buffer_write;
	unsigned buffer_counter;
	unsigned buffer_address;
	uint8_t buffer_value;

	unsigned nmi_cycles_left;  // PPU sets this CPU decrements it

	struct PpuMemoryMap* vram; // CPU access to VRAM
	uint8_t* oam; /* OAM Address Space (Sprite RAM) */
	uint8_t buffer_2007; /* Read buffer for register 2007 */
	uint8_t return_value;
	uint16_t* vram_addr; /* VRAM address - LoopyV (v) */
	uint16_t* vram_tmp_addr; /* Temp VRAM address - LoopyT (t) */
	PpuNametableMirroringType* nametable_mirroring;
	uint8_t* fine_x; /* Fine X Scroll - only lower 4 bits are used */
	bool write_toggle; /* 1st/2nd Write toggle */
} CpuPpuShare;  // PPU MMIO


// CPU
/* Addressing Modes: */
typedef enum {
	ABS,    // 01. Absolute Mode
	ABSX,   // 02. Absolute Mode indexed via X
	ABSY,   // 03. Absolute Mode indexed via Y
	ACC,    // 04. Accumulator Mode
	IMM,    // 05. Immediate Mode
	IMP,    // 06. Implied Mode
	IND,    // 07. Indirect Mode
	INDX,   // 08. Indexed Indirect Mode via X
	INDY,   // 09. Indirect Index Mode via Y
	REL,    // 10. Relative Mode
	ZP,     // 11. Zero Page Mode
	ZPX,    // 12. Zero Page Mode indexed via X
	ZPY,    // 13. Zero Page Mode indexed via Y
	SPECIAL,// not an addressing mode but applies to unique instructions such as BRK, JSR, JMP instructions etc.
	        // these instructions don't have a common deocde logic (even in the case of JSR which uses the ABS mode)
} AddressMode;

typedef enum {
	FETCH,
	DECODE,
	EXECUTE,
} InstructionStates;

typedef struct {
	/* Memory mapped I/O */
	CpuPpuShare* cpu_ppu_io;
	CpuMapperShare* cpu_mapper_io;

	/* Registers */
	uint8_t A; /* Accumulator */
	uint8_t X; /* X reg */
	uint8_t Y; /* Y reg */
	/* Special Registers */
	uint8_t P; /* Program status register */
	uint16_t PC; /* Program counter (instruction pointer) */
	uint8_t stack;
	unsigned cycle; /* Helper variable, logs how many cpu cycles have elapsed */
	/* Memory */
	uint8_t mem[CPU_MEMORY_SIZE];

	// Decoders
	uint8_t addr_lo;
	uint8_t addr_hi;
	uint8_t index_lo;
	uint8_t index_hi;
	uint8_t unmodified_data;
	uint8_t base_addr;
	uint16_t target_addr;
	uint8_t operand;
	uint8_t opcode;
	int8_t offset;  // used in branch and indexed addressing modes
	unsigned instruction_cycles_remaining; // initial value = max number of cycles

	bool delay_nmi;  // only true when enabling NMI via $2000 during VBlank
	bool cpu_ignore_fetch_on_nmi;

	// NES controller
	unsigned controller_latch; // latch signal for controller shift register
	uint8_t player_1_controller;
	uint8_t player_2_controller;

	InstructionStates instruction_state;
	AddressMode address_mode;

	/* Previous values - for disassembler */
	uint8_t old_A;
	uint8_t old_X;
	uint8_t old_Y;
	uint8_t old_P;
	uint16_t old_PC;
	int old_stack;
	unsigned old_cycle;
	bool process_interrupt;
} Cpu6502;

// Non-mirrored memory mapping of ppu vram
struct PpuMemoryMap {
	uint8_t pattern_table_0[0x1000]; // vram: 0x0000 to 0x0FFF
	uint8_t pattern_table_1[0x1000]; // vram: 0x1000 to 0x1FFF
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

// PPU
typedef struct {
	/* Memory mapped I/O */
	CpuPpuShare* cpu_ppu_io;

	/* Memory */
	struct PpuMemoryMap vram;
	uint8_t oam[256]; /* OAM Address Space (Sprite RAM) */

	/* Sprites */
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
	uint8_t sprite_x_counter[8]; /* X pos of sprite (decremented every 8 cycles */

	/* BACKROUND */
	uint16_t vram_addr; /* VRAM address - LoopyV (v) */
	uint16_t vram_tmp_addr; /* Temp VRAM address - LoopyT (t) */
	uint8_t fine_x; /* Fine X Scroll - only lower 4 bits are used */

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

	uint32_t scanline; /* Pre-render = 261, visible = 0 - 239, post-render 240 - 260 */
	uint32_t nmi_start; /* Scanline in which NMI starts - set value depending on NTSC or PAL */
	const uint32_t nmi_end; /* Scanline in which NMI end */
	uint16_t cycle; /* PPU Cycles, each PPU mem access takes 2 cycles */
	uint16_t old_cycle;
	uint32_t old_scanline;
	bool odd_frame;
} Ppu2C02;


// SDL Display
typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* framebuffer;

	uint32_t window_id;
} Sdl2Display;

typedef struct {
	Sdl2Display* cnes_main;
	Sdl2Display* cnes_nt_viewer;
} Sdl2DisplayOutputs;

// Initialised in emu.c
extern Cpu6502* cpu;
extern CpuPpuShare* cpu_ppu;
extern Ppu2C02* ppu;
extern CpuMapperShare* cpu_mapper;

#endif /* __EXTERN_STRUCTS__ */
