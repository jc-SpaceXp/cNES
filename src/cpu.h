/*
 * Contains CPU Architechture
 */
#ifndef __6502_CPU__
#define __6502_CPU__

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MEMORY_SIZE  65536U // Total memory available to the CPU
/* Status_Flags */
#define FLAG_C  0x01 /* Carry */
#define FLAG_Z  0x02 /* Zero */
#define FLAG_I  0x04 /* Interupt Enabled */
#define FLAG_D  0x08 /* Decimal mode - not supported on NES */
#define FLAG_V  0x40 /* Overflow */
#define FLAG_N  0x80 /* Negative */
/* Stack Pointer Definitions - Empty Descending Stack */
#define SP_START   0x0100U /* Stack pointer upper byte is fixed to 1 */
#define SP_OFFSET  0xFFU

/* Interrupt Vectors (lo address) */
#define BRK_VECTOR 0xFFFEU  // IRQ and BRK vector: 0xFFFE and 0xFFFF
#define IRQ_VECTOR 0xFFFEU  // IRQ and BRK vector: 0xFFFE and 0xFFFF
#define NMI_VECTOR 0xFFFAU  // NMI vector: 0xFFFA and 0xFFFB
#define RST_VECTOR 0xFFFCU  // Reset vector: 0xFFFC and 0xFFFD

#if 0

// New "ShareBus" idea, instead on #including .cpu.h in ppu.c
// and then including ppu.h in cpu.c, we can just include cpu.h
// into ppu.h so that we can pass this struct whic both the cpu and
// ppu can freely operate on w/o carefully tring to define functions
// that have access to both live structures during execution
typedef struct {
	uint8_t ppu_addr;
	uint8_t ppu_ctrl;
} SharedBus;

#endif

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
} CpuPpuShare;  // PPU MMIO 

typedef struct {
	/* Memory mapped I/O */
	CpuPpuShare* cpu_ppu_io;

	/* Registers */
	uint8_t A; /* Accumulator */
	uint8_t X; /* X Reg */
	uint8_t Y; /* Y Reg */
	/* Special Registers */
	uint8_t P; /* Program status register - contains flags */
	unsigned Cycle;
	int Stack; /* only being used for debugging */
	uint16_t PC; /* Program counter (Instruction Pointer) */
	/* Memory */
	uint8_t RAM[MEMORY_SIZE]; /* 2 Kb internal RAM */

	uint8_t addr_lo;
	uint8_t addr_hi;
	uint16_t target_addr;
	uint8_t operand;

	/* Previous Values - for Disassembler */
	uint8_t old_A;
	uint8_t old_X;
	uint8_t old_Y;
	uint8_t old_P;
	unsigned old_Cycle;
	int old_Stack;
	uint16_t old_PC;
} Cpu6502;


/* Program Status Register
 *
 * Bit 0 = Carry
 * Bit 1 = Zero
 * Bit 2 = Interrupt enable/disable
 * Bit 3 = Decimal mode (not present in NES)
 * Bit 4 = - (Doesn't Store anything)
 * Bit 5 = Breakpoint (used in debugging - not used in NES)
 * Bit 6 = V - Overflow
 * Bit 7 = N - Negative
 */

/* Header Prototypes */
Cpu6502* CPU;

CpuPpuShare* mmio_init(void);
Cpu6502* cpu_init(uint16_t pc_init, CpuPpuShare* cp); /* NES_CPU : Type 6501 CPU, used to initialise CPU */
void set_pc(Cpu6502* CPU); /* Set PC via reset vector */

uint8_t read_from_cpu(Cpu6502* CPU, uint16_t addr);  // Read byte from CPU mempry
uint16_t return_little_endian(Cpu6502* CPU, uint16_t addr); // Returns 2 byte
void write_to_cpu(Cpu6502* CPU, uint16_t addr, uint8_t val);
void cpu_mem_viewer(Cpu6502* CPU);

/* Adressing Modes:
 *
 * 1. ABS = Absolute Mode
 * 2. ABSX = Absolute Mode indexed via X
 * 3. ABSY = Absolute Mode indexed via Y
 * 4. ACC = Accumulator Mode
 * 5. IMM = Immediate Mode
 * 6. IMP = Implied Mode
 * 7. IND = Indirect Mode
 * 8. INDX = Indexed Indirect Mode via X
 * 9. INDY = Indirect Index Mode via Y
 * 10. REL = Relative Mode
 * 11. ZP = Zero Page Mode
 * 12. ZPX = Zero Page Mode indexed via X
 * 13. ZPY = Zero Page Mode indexed via Y
 */
typedef enum {
	ABS,
	ABSX,
	ABSY,
	ACC,
	IMM,
	IMP,
	IND,
	INDX,
	INDY,
	REL,
	ZP,
	ZPX,
	ZPY,
} AddressMode;

#endif /* __6502_CPU__ */
