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

enum CpuMemType {
	INTERNAL_REG,
	INTERNAL_MEM,
	ADDRESS_MODE_DEP,
};

enum HardwareInterruptsArrayIndexes {
	DMA_INDEX = 0,
	IRQ_INDEX = 1,
	NMI_INDEX = 2,
};

CpuMapperShare* cpu_mapper_init(Cartridge* cart);
CpuPpuShare* mmio_init(void);
Cpu6502* cpu_init(uint16_t pc_init, CpuPpuShare* cp, CpuMapperShare* cm); /* initialise CPU struct */
void clock_cpu(Cpu6502* cpu, const bool no_logging);
extern const uint8_t max_cycles_opcode_lut[256];
extern void (*decode_opcode_lut[256])(Cpu6502* cpu);
extern void (*execute_opcode_lut[256])(Cpu6502* cpu);
extern void (*hardware_interrupts[3])(Cpu6502* cpu); // used for unit tests of DMA/IRQ/NMI (non opcode interrupts)

// Helper functions
void init_pc(Cpu6502* cpu); /* Set PC via reset vector */
uint8_t cpu_generic_read(Cpu6502* cpu, enum CpuMemType mem_type
                        , AddressMode address_mode
                        , uint16_t read_address, const uint8_t* internal_reg);
uint8_t read_from_cpu(Cpu6502* cpu, uint16_t addr);  // Read byte from CPU mempry
void write_to_cpu(Cpu6502* cpu, uint16_t addr, uint8_t val);
void stack_push(Cpu6502* cpu, uint8_t val);
uint8_t stack_pull(Cpu6502* cpu);
void cpu_mem_hexdump_addr_range(const Cpu6502* cpu, uint16_t start_addr, uint16_t end_addr);
void update_cpu_info(Cpu6502* cpu);


#endif /* __6502_CPU__ */
