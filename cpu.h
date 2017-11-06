/*
 * Contains CPU Architechture
 */
#ifndef __6502_CPU__
#define __6502_CPU__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE  65536
/* Status_Flags */
#define FLAG_C  0x01 /* Carry */
#define FLAG_Z  0x02 /* Zero */
#define FLAG_I  0x04 /* Interupt Enabled */
#define FLAG_D  0x08 /* Decimal mode - not supported on NES */
#define FLAG_V  0x40 /* Overflow */
#define FLAG_N  0x80 /* Negative */
/* Stack Pointer Definitions - Empty Descending Stack */
#define SP_START   0x0100 /* Stack pointer upper byte is fixed to 1 */
#define SP_OFFSET  0xFF

typedef struct {
	/* Registers */
	uint8_t A; /* Accumulator */
	uint8_t X; /* X Reg */
	uint8_t Y; /* Y Reg */
	/* Special Registers */
	uint8_t P; /* Program status register - contains flags */
	uint8_t *SP; /* Stack Pointer */
	/** Want to replace SP w/ Stack but doing so breaks my execution 
	 * SP isn't called (see no reason why my code should break)
	 * -- check if SP is used anywhere
	 */
	int Stack; /* only beign used for debugging */
	uint16_t PC; /* Program counter (Instruction Pointer) */
	/* Memory */
	uint8_t RAM[MEMORY_SIZE]; /* 2 Kb internal RAM */
} CPU_6502;


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
CPU_6502* NES_CPU(uint16_t pc_init); /* NES_CPU : Type 6501 CPU, used to initialise CPU */
CPU_6502 *NES; /* Global NES CPU Pointer */

uint8_t read_addr(CPU_6502* NES, uint16_t addr);
uint16_t fetch_16(CPU_6502* NES, uint16_t addr);
uint16_t fetch_16_IND(CPU_6502* NES, uint16_t addr);
void write_addr(CPU_6502* NES, uint16_t addr, uint8_t val);

/* Includes addressing modes
 * Adressing Modes:
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

enum MODES {
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
} address_mode;

#endif /* __6502_CPU__ */
