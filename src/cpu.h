/*
 * Contains CPU architechture and functions to help with
 * fetching and decoding CPU instructions
 */
#ifndef __6502_CPU__
#define __6502_CPU__

#include "extern_structs.h"

#include <stdint.h>
#include <stdbool.h>

/* Status_Flags
 * Bits : 7 ----------> 0
 * Flags: N V - - D I Z C
 * Note:  Bit 5 = Breakpoint (used in debugging - not used in NES)
 */
#define FLAG_C  0x01 /* Carry */
#define FLAG_Z  0x02 /* Zero */
#define FLAG_I  0x04 /* Interupt Enabled */
#define FLAG_D  0x08 /* Decimal mode - not supported on NES */
#define FLAG_V  0x40 /* Overflow */
#define FLAG_N  0x80 /* Negative */

/* Stack pointer definitions (empty descending stack) */
#define SP_START   0x0100U /* Stack pointer upper byte is fixed to 0x01 */
#define SP_OFFSET  0xFFU

/* Interrupt Vectors (lo address) */
#define BRK_VECTOR 0xFFFEU  // IRQ and BRK vector: 0xFFFE and 0xFFFF
#define IRQ_VECTOR 0xFFFEU  // IRQ and BRK vector: 0xFFFE and 0xFFFF
#define NMI_VECTOR 0xFFFAU  // NMI vector: 0xFFFA and 0xFFFB
#define RST_VECTOR 0xFFFCU  // Reset vector: 0xFFFC and 0xFFFD


// 0 denotes illegal op codes
static const uint8_t max_cycles_opcode_lut[256] = {
	0x0007, 0x0006,      0,      0,      0, 0x0003, 0x0005,      0, 0x0003, 0x0002, 0x0002,      0,      0, 0x0004, 0x0006,      0,
	0x0004, 0x0006,      0,      0,      0, 0x0004, 0x0006,      0, 0x0002, 0x0005,      0,      0,      0, 0x0005, 0x0007,      0,
	0x0006, 0x0006,      0,      0, 0x0003, 0x0003, 0x0005,      0, 0x0004, 0x0002, 0x0002,      0, 0x0004, 0x0004, 0x0006,      0,
	0x0004, 0x0006,      0,      0,      0, 0x0004, 0x0006,      0, 0x0002, 0x0005,      0,      0,      0, 0x0005, 0x0007,      0,
	0x0006, 0x0006,      0,      0,      0, 0x0003, 0x0005,      0, 0x0003, 0x0002, 0x0002,      0, 0x0003, 0x0004, 0x0006,      0,
	0x0004, 0x0006,      0,      0,      0, 0x0004, 0x0006,      0, 0x0002, 0x0005,      0,      0,      0, 0x0005, 0x0007,      0,
	0x0006, 0x0006,      0,      0,      0, 0x0003, 0x0005,      0, 0x0004, 0x0002, 0x0002,      0, 0x0005, 0x0004, 0x0006,      0,
	0x0004, 0x0006,      0,      0,      0, 0x0004, 0x0006,      0, 0x0002, 0x0005,      0,      0,      0, 0x0005, 0x0007,      0,
	     0, 0x0006,      0,      0, 0x0003, 0x0003, 0x0003,      0, 0x0002,      0, 0x0002,      0, 0x0004, 0x0004, 0x0004,      0,
	0x0004, 0x0006,      0,      0, 0x0004, 0x0004, 0x0004,      0, 0x0002, 0x0005, 0x0002,      0,      0, 0x0005,      0,      0,
	0x0002, 0x0006, 0x0002,      0, 0x0003, 0x0003, 0x0003,      0, 0x0002, 0x0002, 0x0002,      0, 0x0004, 0x0004, 0x0004,      0,
	0x0004, 0x0006,      0,      0, 0x0004, 0x0004, 0x0004,      0, 0x0002, 0x0005, 0x0002,      0, 0x0005, 0x0005, 0x0005,      0,
	0x0002, 0x0006,      0,      0, 0x0003, 0x0003, 0x0005,      0, 0x0002, 0x0002, 0x0002,      0, 0x0004, 0x0004, 0x0006,      0,
	0x0004, 0x0006,      0,      0,      0, 0x0004, 0x0006,      0, 0x0002, 0x0005,      0,      0,      0, 0x0005, 0x0007,      0,
	0x0002, 0x0006,      0,      0, 0x0003, 0x0003, 0x0005,      0, 0x0002, 0x0002, 0x0002,      0, 0x0004, 0x0004, 0x0006,      0,
	0x0004, 0x0006,      0,      0,      0, 0x0004, 0x0006,      0, 0x0002, 0x0005,      0,      0,      0, 0x0005, 0x0007,      0
};


CpuPpuShare* mmio_init(void);
Cpu6502* cpu_init(uint16_t pc_init, CpuPpuShare* cp); /* NES_CPU : Type 6501 CPU, used to initialise CPU */
void cpu_tick(Cpu6502* CPU);
void cpu_step(Cpu6502* CPU);

// Helper functions
void init_pc(Cpu6502* CPU); /* Set PC via reset vector */
uint8_t read_from_cpu(Cpu6502* CPU, uint16_t addr);  // Read byte from CPU mempry
uint16_t return_little_endian(Cpu6502* CPU, uint16_t addr); // Returns 2 byte
void write_to_cpu(Cpu6502* CPU, uint16_t addr, uint8_t val);
void cpu_mem_16_byte_viewer(Cpu6502* CPU, unsigned start_addr, unsigned total_rows);
void cpu_debugger(Cpu6502* CPU);
bool branch_not_taken(Cpu6502* CPU);
void fetch_opcode(Cpu6502* CPU);
bool fixed_cycles_on_store(Cpu6502* CPU);
void log_cpu_info(Cpu6502* CPU);
void update_cpu_info(Cpu6502* CPU);
bool page_cross_occurs(unsigned low_byte, unsigned offset);
/* Flags */
void update_flag_z(Cpu6502* CPU, uint8_t result);
void update_flag_n(Cpu6502* CPU, uint8_t result);
void update_flag_v(Cpu6502* CPU, bool overflow);
void update_flag_c(Cpu6502* CPU, int carry_out);
/* Stack */
void stack_push(Cpu6502* CPU, uint8_t value);
uint8_t stack_pull(Cpu6502* CPU);

// Decoders: generic / address mode decoders
void bad_op_code(Cpu6502* CPU);  // needed for function pointer of illegal op codes
void decode_ABS_read_store(Cpu6502* CPU);
void decode_ABS_rmw(Cpu6502* CPU);
void decode_ABSX_read_store(Cpu6502* CPU);
void decode_ABSX_rmw(Cpu6502* CPU);
void decode_ABSY_read_store(Cpu6502* CPU);
void decode_ACC(Cpu6502* CPU);
void decode_IMM_read_store(Cpu6502* CPU);
void decode_IMP(Cpu6502* CPU);  // no decoding happens
void decode_INDX_read_store(Cpu6502* CPU);
void decode_INDY_read_store(Cpu6502* CPU);
void decode_ZP_read_store(Cpu6502* CPU);
void decode_ZP_rmw(Cpu6502* CPU);
void decode_ZPX_read_store(Cpu6502* CPU);
void decode_ZPX_rmw(Cpu6502* CPU);
void decode_ZPY_read_store(Cpu6502* CPU);
void decode_ABS_JMP(Cpu6502* CPU);
void decode_IND_JMP(Cpu6502* CPU);
void decode_SPECIAL(Cpu6502* CPU);  // no decoding happens (when specific decoders and generic ones don't apply)
/* Specific decoders */
void decode_PUSH(Cpu6502* CPU);
void decode_PULL(Cpu6502* CPU);
void decode_Bxx(Cpu6502* CPU); // branch instructions (REL address mode)
void decode_RTS(Cpu6502* CPU);


// Execute functions for op code
// Storage:
void execute_LDA(Cpu6502* CPU);
void execute_LDX(Cpu6502* CPU);
void execute_LDY(Cpu6502* CPU);
void execute_STA(Cpu6502* CPU);
void execute_STX(Cpu6502* CPU);
void execute_STY(Cpu6502* CPU);
void execute_TAX(Cpu6502* CPU);
void execute_TAY(Cpu6502* CPU);
void execute_TSX(Cpu6502* CPU);
void execute_TXA(Cpu6502* CPU);
void execute_TXS(Cpu6502* CPU);
void execute_TYA(Cpu6502* CPU);

// Math:
void execute_ADC(Cpu6502* CPU);
void execute_DEC(Cpu6502* CPU);
void execute_DEX(Cpu6502* CPU);
void execute_DEY(Cpu6502* CPU);
void execute_INC(Cpu6502* CPU);
void execute_INX(Cpu6502* CPU);
void execute_INY(Cpu6502* CPU);
void execute_SBC(Cpu6502* CPU);

// Bitwise:
void execute_AND(Cpu6502* CPU);
void execute_ASL(Cpu6502* CPU);
void execute_BIT(Cpu6502* CPU);
void execute_EOR(Cpu6502* CPU);
void execute_LSR(Cpu6502* CPU);
void execute_ORA(Cpu6502* CPU);
void execute_ROL(Cpu6502* CPU);
void execute_ROR(Cpu6502* CPU);

// Branch
void execute_BCC(Cpu6502* CPU);
void execute_BCS(Cpu6502* CPU);
void execute_BEQ(Cpu6502* CPU);
void execute_BMI(Cpu6502* CPU);
void execute_BNE(Cpu6502* CPU);
void execute_BPL(Cpu6502* CPU);
void execute_BVC(Cpu6502* CPU);
void execute_BVS(Cpu6502* CPU);

// JMP
void execute_JMP(Cpu6502* CPU);
void execute_JSR(Cpu6502* CPU);
void execute_RTI(Cpu6502* CPU);
void execute_RTS(Cpu6502* CPU);

// Registers
void execute_CLC(Cpu6502* CPU);
void execute_CLD(Cpu6502* CPU);
void execute_CLI(Cpu6502* CPU);
void execute_CLV(Cpu6502* CPU);
void execute_CMP(Cpu6502* CPU);
void execute_CPX(Cpu6502* CPU);
void execute_CPY(Cpu6502* CPU);
void execute_SEC(Cpu6502* CPU);
void execute_SED(Cpu6502* CPU);
void execute_SEI(Cpu6502* CPU);

// Stack
void execute_PHA(Cpu6502* CPU);
void execute_PHP(Cpu6502* CPU);
void execute_PLA(Cpu6502* CPU);
void execute_PLP(Cpu6502* CPU);

// System
void execute_BRK(Cpu6502* CPU);
void execute_NOP(Cpu6502* CPU);
void execute_IRQ(Cpu6502* CPU);
void execute_NMI(Cpu6502* CPU); // Not an official opcode
void execute_DMA(Cpu6502* CPU); // Not an official opcode


#endif /* __6502_CPU__ */
