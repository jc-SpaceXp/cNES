/*
 * Contains CPU architechture and functions to help with
 * fetching and decoding CPU instructions
 */
#ifndef __6502_CPU__
#define __6502_CPU__

#include "cpu_fwd.h"
#include "cpu_ppu_interface_fwd.h"
#include "cart_fwd.h"
#include "cpu_mapper_interface_fwd.h"

#include <stdint.h>
#include <stdbool.h>

#ifndef KiB
#define KiB (1024U)
#endif /* KiB*/

#define CPU_MEMORY_SIZE  64 * KiB // Total memory available to the CPU

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

enum DataBusType {DATA, ADL, ADH, BAL, INL, INH, BRANCH};

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
	POST_EXECUTE,
} InstructionStates;

struct Cpu6502 {
	// Memory mapped I/O
	CpuPpuShare* cpu_ppu_io;
	CpuMapperShare* cpu_mapper_io;

	// Registers
	uint8_t A; // Accumulator
	uint8_t X; // X reg
	uint8_t Y; // Y reg
	// Special Registers
	uint8_t P; // Program status register
	uint16_t PC; // Program counter (instruction pointer)
	uint8_t stack;
	unsigned cycle; // Helper variable, logs how many cpu cycles have elapsed
	// Memory
	uint8_t mem[CPU_MEMORY_SIZE];

	// Bus signals
	uint16_t address_bus;
	uint16_t data_bus;

	// Decoders
	uint8_t addr_lo;
	uint8_t addr_hi;
	uint8_t index_lo;
	uint8_t index_hi;
	uint8_t base_addr;
	uint16_t target_addr;
	uint8_t operand;
	uint8_t opcode;
	int8_t offset;  // used in branch and indexed addressing modes
	unsigned instruction_cycles_remaining; // initial value = max number of cycles

	bool delay_nmi;  // only true when enabling NMI via $2000 during VBlank
	bool cpu_ignore_fetch_on_nmi;

	// Instruction trace logger
	char instruction[18]; // complete instruction e.g. LDA $2000
	char end[10]; // ending of the instruction e.g. #$2000
	char append_int[20]; // conversion for int to char

	// NES controller
	unsigned controller_latch; // latch signal for controller shift register
	uint8_t player_1_controller;
	uint8_t player_2_controller;

	InstructionStates instruction_state;
	AddressMode address_mode;

	// Previous values for trace logging
	uint8_t old_A;
	uint8_t old_X;
	uint8_t old_Y;
	uint8_t old_P;
	uint16_t old_PC;
	int old_stack;
	unsigned old_cycle;
	bool process_interrupt;
};

struct InstructionDetails {
	const char mnemonic[5];
	void (*decode_opcode)(Cpu6502* cpu);
	void (*execute_opcode)(Cpu6502* cpu);
	const uint8_t max_cycles;
};
extern struct InstructionDetails isa_info[256];

Cpu6502* cpu_allocator(void);
int cpu_init(Cpu6502* cpu, uint16_t pc, CpuPpuShare* cp, CpuMapperShare* cm);

void clock_cpu(Cpu6502* cpu, const bool no_logging);
extern void (*hardware_interrupts[3])(Cpu6502* cpu); // used for unit tests of DMA/IRQ/NMI (non opcode interrupts)

// Helper functions
void init_pc(Cpu6502* cpu); /* Set PC via reset vector */
uint8_t cpu_generic_read(Cpu6502* cpu, enum CpuMemType mem_type
                        , AddressMode address_mode
                        , uint16_t read_address, const uint8_t* internal_reg);
uint8_t read_from_cpu(Cpu6502* cpu, uint16_t addr);  // Read byte from CPU mempry
void cpu_generic_write(Cpu6502* cpu, enum CpuMemType mem_type
                      , AddressMode address_mode
                      , uint16_t write_address, uint8_t* internal_reg
                      , uint8_t data);
void write_to_cpu(Cpu6502* cpu, uint16_t addr, uint8_t val);
void set_address_bus_bytes(Cpu6502* cpu, uint8_t adh, uint8_t adl);
void set_address_bus(Cpu6502* cpu, uint16_t target_address);
void set_data_bus_via_read(Cpu6502* cpu, uint16_t target_address, enum DataBusType data_type);
void set_data_bus_via_write(Cpu6502* cpu, uint8_t data);
void stack_push(Cpu6502* cpu, uint8_t val);
uint8_t stack_pull(Cpu6502* cpu);
void cpu_mem_hexdump_addr_range(const Cpu6502* cpu, uint16_t start_addr, uint16_t end_addr);
void cpu_debugger(const Cpu6502* cpu, char* instruction, char* append_int, char* end);
void update_cpu_info(Cpu6502* cpu);


// Memory mapped reads/writes
uint8_t read_ppu_reg(const uint16_t addr, Cpu6502* cpu);
void write_ppu_reg(const uint16_t addr, const uint8_t data, Cpu6502* cpu);
void delay_write_ppu_reg(const uint16_t addr, const uint8_t data, Cpu6502* cpu);


#endif /* __6502_CPU__ */
