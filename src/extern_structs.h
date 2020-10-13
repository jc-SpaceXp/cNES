#ifndef __EXTERN_STRUCTS__
#define __EXTERN_STRUCTS__

#include "SDL2/SDL.h"

#include <stdbool.h>
#include <stdint.h>

#ifndef KiB
#define KiB (1024U)
#endif /* KiB */

#define CPU_MEMORY_SIZE  65536U // Total memory available to the CPU

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
	bool suppress_nmi;
	bool write_debug;  // Trigger debug of PPU only when the CPU writes to the disassembler

	unsigned nmi_cycles_left;  // PPU sets this CPU decrements it

	uint8_t* VRAM; // CPU access to VRAM
	uint8_t* OAM; /* OAM Address Space (Sprite RAM) */
	uint8_t buffer_2007; /* Read buffer for register 2007 */
	uint8_t return_value;
	uint16_t* vram_addr; /* VRAM address - LoopyV (v) */
	uint16_t* vram_tmp_addr; /* Temp VRAM address - LoopyT (t) */
	unsigned* mirroring; // 0 = Horz, 1 = vert, 4 = 4 screen
	uint8_t* fineX; /* Fine X Scroll - only lower 4 bits are used */
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

	/* Registers */
	uint8_t A; /* Accumulator */
	uint8_t X; /* X reg */
	uint8_t Y; /* Y reg */
	/* Special Registers */
	uint8_t P; /* Program status register */
	unsigned Cycle;
	int Stack; /* only being used for debugging */
	uint16_t PC; /* Program counter (instruction pointer) */
	/* Memory */
	uint8_t MEM[CPU_MEMORY_SIZE];

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
	//bool write_flag;  // 1 for store operations i.e. STA, STX etc
	void (*decoder)(int); // decoder function pointer, see .c file

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
	unsigned old_Cycle;
	int old_Stack;
	uint16_t old_PC;
} Cpu6502;

// PPU
typedef struct {
	/* Memory mapped I/O */
	CpuPpuShare* cpu_ppu_io;



	/* Memory */
	uint8_t VRAM[16 * KiB]; /* PPU memory space (VRAM) */
	uint8_t OAM[256]; /* OAM Address Space (Sprite RAM) */

	/* Sprites */
	uint8_t scanline_OAM[32]; // Secondary OAM, change to scanline
	uint8_t OAM_read_buffer;
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
	uint8_t fineX; /* Fine X Scroll - only lower 4 bits are used */

	/* Latches & Shift registers */
	uint8_t pt_lo_latch; /* Most recent fetch pt_lo fetch */
	uint8_t pt_hi_latch;
	uint16_t pt_lo_shift_reg; /* Stores a 16 pixels in the pipeline - 1st 8 pixels to be rendered are in lowest byte */
	uint16_t pt_hi_shift_reg;
	uint8_t nt_byte; /* NT byte that is rendered 16 PPU cycles later */
	uint8_t at_latch; /* AT byte that is rendered 16 PPU cycles later */
	uint8_t at_current; /* AT byte for pixels 1 - 8 in pipeline */
	uint8_t at_next; /* AT byte for pixels 9 - 16 in pipeline */

	uint16_t nt_addr_tmp; /* Address used to generate nt byte */
	uint16_t nt_addr_next; /* Next tile address for pixels 9 -16 in pipeline */
	uint16_t nt_addr_current; /* Current tile address for pixels 1 - 8 in pipeline */

	//int palette[4]; /* Stores the 4 colour palette */
	bool reset_1;
	bool reset_2;

	// maybe delete (keeping in case it is used by something late on)
	unsigned mirroring; // 0 = Horz, 1 = vert, 4 = 4 screen

	uint32_t scanline; /* Pre-render = 261, visible = 0 - 239, post-render 240 - 260 */
	uint32_t nmi_start; /* Scanline in which NMI starts - set value depending on NTSC or PAL */
	const uint32_t nmi_end; /* Scanline in which NMI end */
	uint16_t cycle; /* PPU Cycles, each PPU mem access takes 2 cycles */
	uint16_t old_cycle;
} PPU_Struct;


// SDL Display
typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* framebuffer;
} Display;

extern Display* nes_screen;

// Initialised in emu.c
extern Cpu6502* CPU;
extern CpuPpuShare* cpu_ppu;
extern PPU_Struct* PPU;

#endif /* __EXTERN_STRUCTS__ */
