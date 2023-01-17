/*
 * Contains CPU architechture and functions
 */
#include "extern_structs.h"
#include "cpu.h"
#include "ppu.h"  // needed for read/write functions
#include "mappers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// CPU memory map
#define ADDR_RAM_START        0x0000U
#define ADDR_RAM_END          0x1FFFU
#define ADDR_PPU_REG_START    0x2000U
#define ADDR_PPU_REG_END      0x3FFFU
#define ADDR_OAM_DMA          0x4014U
#define ADDR_JOY1             0x4016U
#define ADDR_JOY2             0x4017U

// Address masks
#define RAM_NON_MIRROR_MASK     0x07FFU
#define PPU_REG_NON_MIRROR_MASK 0x2007U

// static prototype functions
static uint16_t return_little_endian(Cpu6502* cpu, uint16_t addr);
static void write_4016(uint8_t data, Cpu6502* cpu);
static unsigned read_4016(Cpu6502* cpu);
static unsigned read_4017(Cpu6502* cpu);
static void cpu_debugger(const Cpu6502* cpu, char* instruction, char* append_int, char* end);  // warning unused function, currently hidden behind conditional execution
static void log_cpu_info(Cpu6502* cpu, const bool no_logging);  // warning unused function, currently hidden behind conditional execution
static void fetch_opcode(Cpu6502* cpu);
static bool fixed_cycles_on_store(const Cpu6502* cpu);
static bool page_cross_occurs(const unsigned low_byte, const unsigned offset);
static void update_flag_z(Cpu6502* cpu, uint8_t result);
static void update_flag_n(Cpu6502* cpu, uint8_t result);
static void update_flag_v(Cpu6502* cpu, bool overflow);
static void update_flag_c(Cpu6502* cpu, int carry_out);
static void bad_op_code(Cpu6502* cpu);  // needed for function pointer
static void decode_ABS_read_store(Cpu6502* cpu);
static void decode_ABS_rmw(Cpu6502* cpu);
static void decode_ABSX_read_store(Cpu6502* cpu);
static void decode_ABSX_rmw(Cpu6502* cpu);
static void decode_ABSY_read_store(Cpu6502* cpu);
static void decode_ACC(Cpu6502* cpu);
static void decode_IMM_read(Cpu6502* cpu);
static void decode_IMP(Cpu6502* cpu);
static void decode_INDX_read_store(Cpu6502* cpu);
static void decode_INDY_read_store(Cpu6502* cpu);
static void decode_ZP_read_store(Cpu6502* cpu);
static void decode_ZP_rmw(Cpu6502* cpu);
static void decode_ZPX_read_store(Cpu6502* cpu);
static void decode_ZPX_rmw(Cpu6502* cpu);
static void decode_ABS_JMP(Cpu6502* cpu);
static void decode_ZPY_read_store(Cpu6502* cpu);
static void decode_IND_JMP(Cpu6502* cpu);
static void decode_SPECIAL(Cpu6502* cpu); // no decoding happens here, execute_X() handles the complete instruction
static void decode_PUSH(Cpu6502* cpu);
static void decode_PULL(Cpu6502* cpu);
static void decode_Bxx(Cpu6502* cpu); // branch instructions (REL address mode)
static void decode_RTS(Cpu6502* cpu);
static void execute_LDA(Cpu6502* cpu);
static void execute_LDX(Cpu6502* cpu);
static void execute_LDY(Cpu6502* cpu);
static void execute_STA(Cpu6502* cpu);
static void execute_STX(Cpu6502* cpu);
static void execute_STY(Cpu6502* cpu);
static void execute_TAX(Cpu6502* cpu);
static void execute_TAY(Cpu6502* cpu);
static void execute_TSX(Cpu6502* cpu);
static void execute_TXA(Cpu6502* cpu);
static void execute_TXS(Cpu6502* cpu);
static void execute_TYA(Cpu6502* cpu);
static void execute_ADC(Cpu6502* cpu);
static void execute_DEC(Cpu6502* cpu);
static void execute_DEX(Cpu6502* cpu);
static void execute_DEY(Cpu6502* cpu);
static void execute_INC(Cpu6502* cpu);
static void execute_INX(Cpu6502* cpu);
static void execute_INY(Cpu6502* cpu);
static void execute_SBC(Cpu6502* cpu);
static void execute_AND(Cpu6502* cpu);
static void execute_ASL(Cpu6502* cpu);
static void execute_BIT(Cpu6502* cpu);
static void execute_EOR(Cpu6502* cpu);
static void execute_LSR(Cpu6502* cpu);
static void execute_ORA(Cpu6502* cpu);
static void execute_ROL(Cpu6502* cpu);
static void execute_ROR(Cpu6502* cpu);
static void execute_BCC(Cpu6502* cpu);
static void execute_BCS(Cpu6502* cpu);
static void execute_BEQ(Cpu6502* cpu);
static void execute_BMI(Cpu6502* cpu);
static void execute_BNE(Cpu6502* cpu);
static void execute_BPL(Cpu6502* cpu);
static void execute_BVC(Cpu6502* cpu);
static void execute_BVS(Cpu6502* cpu);
static void execute_JMP(Cpu6502* cpu);
static void execute_JSR(Cpu6502* cpu);
static void execute_RTI(Cpu6502* cpu);
static void execute_RTS(Cpu6502* cpu);
static void execute_CLC(Cpu6502* cpu);
static void execute_CLD(Cpu6502* cpu);
static void execute_CLI(Cpu6502* cpu);
static void execute_CLV(Cpu6502* cpu);
static void execute_CMP(Cpu6502* cpu);
static void execute_CPX(Cpu6502* cpu);
static void execute_CPY(Cpu6502* cpu);
static void execute_SEC(Cpu6502* cpu);
static void execute_SED(Cpu6502* cpu);
static void execute_SEI(Cpu6502* cpu);
static void execute_PHA(Cpu6502* cpu);
static void execute_PHP(Cpu6502* cpu);
static void execute_PLA(Cpu6502* cpu);
static void execute_PLP(Cpu6502* cpu);
static void execute_BRK(Cpu6502* cpu);
static void execute_NOP(Cpu6502* cpu);
static void execute_IRQ(Cpu6502* cpu);  // warning unused function, needed later for audio or other mappers I believe
static void execute_NMI(Cpu6502* cpu);
static void execute_DMA(Cpu6502* cpu, const bool no_logging);

// 0 denotes illegal op codes
const uint8_t max_cycles_opcode_lut[256] = {
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

void (*decode_opcode_lut[256])(Cpu6502* cpu) = {
	decode_SPECIAL        , decode_INDX_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_PUSH, decode_IMM_read       , decode_ACC,  bad_op_code, bad_op_code           , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code,
	decode_SPECIAL        , decode_INDX_read_store, bad_op_code          , bad_op_code, decode_ZP_read_store , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_PULL, decode_IMM_read       , decode_ACC , bad_op_code, decode_ABS_read_store , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code,
	decode_SPECIAL        , decode_INDX_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_PUSH, decode_IMM_read       , decode_ACC , bad_op_code, decode_ABS_JMP        , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code,
	decode_RTS            , decode_INDX_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_PULL, decode_IMM_read       , decode_ACC , bad_op_code, decode_IND_JMP        , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code,
	bad_op_code           , decode_INDX_read_store, bad_op_code          , bad_op_code, decode_ZP_read_store , decode_ZP_read_store , decode_ZP_read_store , bad_op_code, decode_IMP , bad_op_code           , decode_IMP , bad_op_code, decode_ABS_read_store , decode_ABS_read_store , decode_ABS_read_store , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, decode_ZPX_read_store, decode_ZPX_read_store, decode_ZPY_read_store, bad_op_code, decode_IMP , decode_ABSY_read_store, decode_IMP , bad_op_code, bad_op_code           , decode_ABSX_read_store, bad_op_code           , bad_op_code,
	decode_IMM_read       , decode_INDX_read_store, decode_IMM_read      , bad_op_code, decode_ZP_read_store , decode_ZP_read_store , decode_ZP_read_store , bad_op_code, decode_IMP , decode_IMM_read       , decode_IMP , bad_op_code, decode_ABS_read_store , decode_ABS_read_store , decode_ABS_read_store , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, decode_ZPX_read_store, decode_ZPX_read_store, decode_ZPY_read_store, bad_op_code, decode_IMP , decode_ABSY_read_store, decode_IMP , bad_op_code, decode_ABSX_read_store, decode_ABSX_read_store, decode_ABSY_read_store, bad_op_code,
	decode_IMM_read       , decode_INDX_read_store, bad_op_code          , bad_op_code, decode_ZP_read_store , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_IMP , decode_IMM_read       , decode_IMP , bad_op_code, decode_ABS_read_store , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code,
	decode_IMM_read       , decode_INDX_read_store, bad_op_code          , bad_op_code, decode_ZP_read_store , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_IMP , decode_IMM_read       , decode_IMP , bad_op_code, decode_ABS_read_store , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code
};

void (*execute_opcode_lut[256])(Cpu6502* cpu) = {
	execute_BRK, execute_ORA, bad_op_code, bad_op_code, bad_op_code, execute_ORA, execute_ASL, bad_op_code, execute_PHP, execute_ORA, execute_ASL, bad_op_code, bad_op_code, execute_ORA, execute_ASL, bad_op_code,
	execute_BPL, execute_ORA, bad_op_code, bad_op_code, bad_op_code, execute_ORA, execute_ASL, bad_op_code, execute_CLC, execute_ORA, bad_op_code, bad_op_code, bad_op_code, execute_ORA, execute_ASL, bad_op_code,
	execute_JSR, execute_AND, bad_op_code, bad_op_code, execute_BIT, execute_AND, execute_ROL, bad_op_code, execute_PLP, execute_AND, execute_ROL, bad_op_code, execute_BIT, execute_AND, execute_ROL, bad_op_code,
	execute_BMI, execute_AND, bad_op_code, bad_op_code, bad_op_code, execute_AND, execute_ROL, bad_op_code, execute_SEC, execute_AND, bad_op_code, bad_op_code, bad_op_code, execute_AND, execute_ROL, bad_op_code,
	execute_RTI, execute_EOR, bad_op_code, bad_op_code, bad_op_code, execute_EOR, execute_LSR, bad_op_code, execute_PHA, execute_EOR, execute_LSR, bad_op_code, execute_JMP, execute_EOR, execute_LSR, bad_op_code,
	execute_BVC, execute_EOR, bad_op_code, bad_op_code, bad_op_code, execute_EOR, execute_LSR, bad_op_code, execute_CLI, execute_EOR, bad_op_code, bad_op_code, bad_op_code, execute_EOR, execute_LSR, bad_op_code,
	execute_RTS, execute_ADC, bad_op_code, bad_op_code, bad_op_code, execute_ADC, execute_ROR, bad_op_code, execute_PLA, execute_ADC, execute_ROR, bad_op_code, execute_JMP, execute_ADC, execute_ROR, bad_op_code,
	execute_BVS, execute_ADC, bad_op_code, bad_op_code, bad_op_code, execute_ADC, execute_ROR, bad_op_code, execute_SEI, execute_ADC, bad_op_code, bad_op_code, bad_op_code, execute_ADC, execute_ROR, bad_op_code,
	bad_op_code, execute_STA, bad_op_code, bad_op_code, execute_STY, execute_STA, execute_STX, bad_op_code, execute_DEY, bad_op_code, execute_TXA, bad_op_code, execute_STY, execute_STA, execute_STX, bad_op_code,
	execute_BCC, execute_STA, bad_op_code, bad_op_code, execute_STY, execute_STA, execute_STX, bad_op_code, execute_TYA, execute_STA, execute_TXS, bad_op_code, bad_op_code, execute_STA, bad_op_code, bad_op_code,
	execute_LDY, execute_LDA, execute_LDX, bad_op_code, execute_LDY, execute_LDA, execute_LDX, bad_op_code, execute_TAY, execute_LDA, execute_TAX, bad_op_code, execute_LDY, execute_LDA, execute_LDX, bad_op_code,
	execute_BCS, execute_LDA, bad_op_code, bad_op_code, execute_LDY, execute_LDA, execute_LDX, bad_op_code, execute_CLV, execute_LDA, execute_TSX, bad_op_code, execute_LDY, execute_LDA, execute_LDX, bad_op_code,
	execute_CPY, execute_CMP, bad_op_code, bad_op_code, execute_CPY, execute_CMP, execute_DEC, bad_op_code, execute_INY, execute_CMP, execute_DEX, bad_op_code, execute_CPY, execute_CMP, execute_DEC, bad_op_code,
	execute_BNE, execute_CMP, bad_op_code, bad_op_code, bad_op_code, execute_CMP, execute_DEC, bad_op_code, execute_CLD, execute_CMP, bad_op_code, bad_op_code, bad_op_code, execute_CMP, execute_DEC, bad_op_code,
	execute_CPX, execute_SBC, bad_op_code, bad_op_code, execute_CPX, execute_SBC, execute_INC, bad_op_code, execute_INX, execute_SBC, execute_NOP, bad_op_code, execute_CPX, execute_SBC, execute_INC, bad_op_code,
	execute_BEQ, execute_SBC, bad_op_code, bad_op_code, bad_op_code, execute_SBC, execute_INC, bad_op_code, execute_SED, execute_SBC, bad_op_code, bad_op_code, bad_op_code, execute_SBC, execute_INC, bad_op_code
};

void (*hardware_interrupts[3])(Cpu6502* cpu) = {
	execute_DMA, execute_IRQ, execute_NMI,
};

CpuMapperShare* cpu_mapper_init(Cartridge* cart)
{
	CpuMapperShare* i = malloc(sizeof(CpuMapperShare));
	if (!i) {
		fprintf(stderr, "Failed to allocate enough memory for CpuMapperShare\n");
		return i;
	}

	// assign mirrors
	i->prg_rom = &cart->prg_rom;
	i->prg_ram = &cart->prg_ram;
	i->chr = &cart->chr;

	i->mapper_number = 0;
	i->prg_rom_bank_size = 0;
	i->chr_bank_size = 0;

	i->prg_low_bank_fixed = false;
	i->prg_high_bank_fixed = false;
	i->enable_prg_ram = false;

	return i;
}

CpuPpuShare* mmio_init(void)
{
	CpuPpuShare* i = malloc(sizeof(CpuPpuShare));
	if (!i) {
		fprintf(stderr, "Failed to allocate enough memory for PPU I/O\n");
		return i;
	}

	i->ppu_ctrl = 0;
	i->ppu_mask = 0;
	i->ppu_status = 0;
	i->oam_addr = 0;
	i->oam_data = 0;
	i->ppu_scroll = 0;
	i->ppu_addr = 0;
	i->ppu_data = 0;
	i->oam_dma = 0;

	i->nmi_pending = false;
	i->dma_pending = false;
	i->suppress_nmi_flag = false;
	i->ignore_nmi = false;
	i->nmi_lookahead = false;

	i->nmi_cycles_left = 7;

	i->buffer_write = false;
	i->buffer_address = 0;
	i->buffer_counter = 0;
	i->buffer_value = 0;

	return i;
}

Cpu6502* cpu_init(uint16_t pc_init, CpuPpuShare* cp, CpuMapperShare* cm)
{
	Cpu6502* i = malloc(sizeof(Cpu6502));
	if (!i) {
		fprintf(stderr, "Failed to allocate enough memory for CPU\n");
		return i;
	}

	i->cpu_ppu_io = cp;
	i->cpu_mapper_io = cm;
	i->PC = pc_init;
	i->stack = 0xFD; // After startup stack pointer is FD
	i->cycle = 0;
	i->P = 0x24;
	i->A = 0;
	i->X = 0;
	i->Y = 0;
	i->old_cycle = 0;
	i->instruction_state = FETCH;
	i->instruction_cycles_remaining = 51; // initial value doesn't matter as LUT will set it after first instruction is read

	i->delay_nmi = false;
	i->cpu_ignore_fetch_on_nmi = false;
	i->process_interrupt = false;

	i->controller_latch = 0;
	i->player_1_controller = 0;
	i->player_2_controller = 0;

	memset(i->mem, 0, CPU_MEMORY_SIZE); // Zero out memory
	return i;
}


void init_pc(Cpu6502* cpu)
{
	cpu->PC = return_little_endian(cpu, RST_VECTOR);
}

uint8_t cpu_generic_read(Cpu6502* cpu, enum CpuMemType mem_type
                        , AddressMode address_mode
                        , uint16_t read_address, const uint8_t* internal_reg)
{
	uint8_t read_val = read_from_cpu(cpu, read_address); // default val

	if ((mem_type == INTERNAL_REG) && (internal_reg != NULL)) {
		read_val = *internal_reg; // should point to X, Y or A registers
	}

	return read_val;
}

uint8_t read_from_cpu(Cpu6502* cpu, uint16_t addr)
{
	unsigned read;
	if (addr < (ADDR_RAM_END + 1)) { // read from RAM (non-mirrored)
		read = cpu->mem[addr & RAM_NON_MIRROR_MASK];
	} else if (addr < (ADDR_PPU_REG_END + 1)) { // read from PPU registers (non-mirrored)
		read = read_ppu_reg(addr & PPU_REG_NON_MIRROR_MASK, cpu);
	} else if (addr == ADDR_JOY1) {
		read = read_4016(cpu);
	} else if (addr == ADDR_JOY2) {
		read = read_4017(cpu);
	} else {
		read = cpu->mem[addr]; /* catch-all */
	}

	return read;
}

void set_address_bus_bytes(Cpu6502* cpu, uint8_t adh, uint8_t adl)
{
	cpu->address_bus = (adh << 8) | adl;
}

void set_address_bus(Cpu6502* cpu, uint16_t target_address)
{
	cpu->address_bus = target_address;
}

void set_data_bus_via_read(Cpu6502* cpu, uint16_t target_address, enum DataBusType data_type)
{
	uint8_t data = read_from_cpu(cpu, target_address);
	cpu->data_bus = data;

	// Set internal signals too
	if (data_type == ADL) {
		cpu->addr_lo = data;
	} else if (data_type == ADH) {
		cpu->addr_hi = data;
	} else if (data_type == BAL) {
		cpu->base_addr = data;
	} else if (data_type == INL) {
		cpu->index_lo = data;
	} else if (data_type == INH) {
		cpu->index_hi = data;
	} else if (data_type == BRANCH) {
		cpu->offset = data;
	}
}

void set_data_bus_via_write(Cpu6502* cpu, uint8_t data)
{
	cpu->data_bus = data;
}

/* Return 16 bit address in little endian format */
static uint16_t return_little_endian(Cpu6502* cpu, uint16_t addr)
{
	return ((read_from_cpu(cpu, addr + 1) << 8) | read_from_cpu(cpu, addr));
}

static uint16_t concat_address_bus_bytes(uint8_t adh, uint8_t adl)
{
	return ((adh << 8) | adl);
}

void write_to_cpu(Cpu6502* cpu, uint16_t addr, uint8_t val)
{
	if (addr < (ADDR_RAM_END + 1)) { // write to RAM (non-mirrored)
		cpu->mem[addr & RAM_NON_MIRROR_MASK] = val;
	} else if (addr < (ADDR_PPU_REG_END + 1)) { // write to PPU registers (non-mirrored)
		delay_write_ppu_reg(addr & PPU_REG_NON_MIRROR_MASK, val, cpu);
		cpu->mem[addr & PPU_REG_NON_MIRROR_MASK] = val;
	} else if (addr == ADDR_OAM_DMA) {
		write_ppu_reg(addr, val, cpu);
	} else if (addr == ADDR_JOY1) {
		write_4016(val, cpu);
	} else if (addr >= 0x8000) { // currently hard-coded for mappers 0 and 1
		mapper_write(cpu, addr, val);
	} else {
		cpu->mem[addr] = val;
	}
}

static void write_4016(uint8_t data, Cpu6502* cpu)
{
	// Standard NES controller write
	if (data == 1) {
		cpu->controller_latch = 1;
	} else if (data == 0) {
		cpu->controller_latch = 0;
	}
}

static unsigned read_4016(Cpu6502* cpu)
{
	static unsigned clock_pulse = 0;
	unsigned ret = 0;

	ret = (cpu->player_1_controller >> clock_pulse) & 0x01;

	++clock_pulse;
	if (clock_pulse == 8) { clock_pulse = 0; }

	return ret;
}


static unsigned read_4017(Cpu6502* cpu)
{
	static unsigned clock_pulse = 0;
	unsigned ret = 0;

	ret = (cpu->player_2_controller >> clock_pulse) & 0x01;

	++clock_pulse;
	if (clock_pulse == 8) { clock_pulse = 0; }

	return ret;
}

void cpu_mem_hexdump_addr_range(const Cpu6502* cpu, uint16_t start_addr, uint16_t end_addr)
{
	if (end_addr <= start_addr) {
		fprintf(stderr, "Hexdump failed, need more than 1 byte to read (end_addr must be greater than start_addr)\n");
		return;
	}

	printf("\n########################  CPU  MEM  ##################\n");
	// print header for memory addresses, e.g. 0x00 through 0x0F
	printf("      ");
	for (int h = 0; h < 16; h++) {
		printf("%.2X ", (start_addr & 0x0F) + h);
		// halfway point, print extra space for readability
		if (h == 7) {
			printf(" ");
		}
	}
	printf("\n");

	// acutally perform hexdump here
	while (start_addr < end_addr) {
		printf("%.4X: ", start_addr);
		for (int x = 0; x < 16; x++) {
			if ((start_addr + x) > end_addr) {
				break; // early stop
			}
			printf("%.2X ", cpu->mem[start_addr + x]);
			// halfway point, print extra space for readability
			if (x == 7) {
				printf(" ");
			}
		}
		start_addr += 16;
		printf("\n");
	}
}

void clock_cpu(Cpu6502* cpu, const bool no_logging)
{
	++cpu->cycle;
	--cpu->instruction_cycles_remaining;

	// disable any pending interrupts when suppressing an NMI
	if (cpu->cpu_ppu_io->ignore_nmi) {
		cpu->process_interrupt = false;
		cpu->cpu_ppu_io->ignore_nmi = false;
	}

	// Handle interrupts first
	if (!cpu->delay_nmi && cpu->process_interrupt && cpu->instruction_state == FETCH) {
		// print the disassembly info of the instruction just completed
		if (cpu->cpu_ppu_io->nmi_cycles_left == 7) {
#ifdef __DEBUG__
			cpu->cpu_ppu_io->write_debug = true;
			cpu_debugger(cpu, cpu->instruction, cpu->append_int, cpu->end);
			log_cpu_info(cpu, no_logging);
			update_cpu_info(cpu);
#endif /* __DEBUG__ */
		}
		execute_NMI(cpu);
		--cpu->cpu_ppu_io->nmi_cycles_left;
	} else {
		// Fetch-decode-execute state logic
		if (cpu->instruction_state == FETCH) {
			if (cpu->cpu_ppu_io->dma_pending) {
				execute_DMA(cpu, no_logging);
				return;
			}
			// if not the first instruction print its output
			if (cpu->instruction_cycles_remaining != 50) {
#ifdef __DEBUG__
				cpu->cpu_ppu_io->write_debug = true;
				cpu_debugger(cpu, cpu->instruction, cpu->append_int, cpu->end);
				log_cpu_info(cpu, no_logging);
				update_cpu_info(cpu);
#endif /* __DEBUG__ */
			}
			fetch_opcode(cpu);
			cpu->delay_nmi = false; // reset after returning from NMI
		}  else if (cpu->instruction_state == DECODE) {
			decode_opcode_lut[cpu->opcode](cpu);
		}
		if (cpu->instruction_state == EXECUTE) {
			cpu->instruction_state = FETCH;
			execute_opcode_lut[cpu->opcode](cpu); // can change the PC which the early fetch made!

			if (cpu->cpu_ppu_io->nmi_pending) {
				cpu->process_interrupt = true;
			}

			if (cpu->cpu_ppu_io->nmi_lookahead) {
				cpu->delay_nmi = true;
			}

			if (cpu->cpu_ppu_io->nmi_lookahead && cpu->cpu_ignore_fetch_on_nmi) {
				cpu->delay_nmi = false;
			}
			cpu->cpu_ignore_fetch_on_nmi = false;
		}
	}
}

// true if branch not taken based on opcode
static bool branch_not_taken(const Cpu6502* cpu)
{
	bool result;
	switch (cpu->opcode) {
	case 0x90: // BCC
		result = !(cpu->P & FLAG_C) ? 0 : 1;
		break;
	case 0xB0: // BCS
		result = cpu->P & FLAG_C ? 0 : 1;
		break;
	case 0x10: // BPL
		result = !(cpu->P & FLAG_N) ? 0 : 1;
		break;
	case 0x30: // BMI
		result = cpu->P & FLAG_N ? 0 : 1;
		break;
	case 0x50: // BVC
		result = !(cpu->P & FLAG_V) ? 0 : 1;
		break;
	case 0x70: // BVS
		result = cpu->P & FLAG_V ? 0 : 1;
		break;
	case 0xD0: // BNE
		result = !(cpu->P & FLAG_Z) ? 0 : 1;
		break;
	case 0xF0: // BEQ
		result = cpu->P & FLAG_Z ? 0 : 1;
		break;
	default:
		printf("invalid opcode\n");
		result = 0;
		break;
	}

	return result;
}

static void cpu_debugger(const Cpu6502* cpu, char* instruction, char* append_int, char* end)
{
	switch(cpu->address_mode) {
	case ABS:
		sprintf(append_int, "%.4X", cpu->target_addr);
		strcpy(end, "$");
		strcat(end, append_int);
		break;
	case ABSX:   // 02. Absolute Mode indexed via X
		sprintf(append_int, "%.4X", cpu->target_addr - cpu->X);
		strcpy(end, "$");
		strcat(end, append_int);
		strcat(end, ",X");
		break;
	case ABSY:   // 03. Absolute Mode indexed via Y
		sprintf(append_int, "%.4X", cpu->target_addr - cpu->Y);
		strcpy(end, "$");
		strcat(end, append_int);
		strcat(end, ",Y");
		break;
	case ACC:    // 04. Accumulator Mode
		strcpy(end, "A");
		break;
	case IMM:    // 05. Immediate Mode
		strcpy(end, "#$");
		sprintf(append_int, "%.2X", cpu->operand);
		strcat(end, append_int);
		break;
	case IMP: // 06. Implied Mode
	case SPECIAL:
		strcpy(end, "");
		break;
	case IND:    // 07. Indirect Mode
		sprintf(append_int, "%.4X", cpu->target_addr);  // previously used %.X
		strcpy(end, "($");
		strcat(end, append_int);
		strcat(end, ")");
		break;
	case INDX:   // 08. Indexed Indirect Mode via X
		sprintf(append_int, "%.2X", cpu->base_addr);
		strcpy(end, "($");
		strcat(end, append_int);
		strcat(end, ",X)");
		break;
	case INDY:   // 09. Indirect Index Mode via Y
		sprintf(append_int, "%.2X", cpu->base_addr);
		strcpy(end, "($");
		strcat(end, append_int);
		strcat(end, "),Y");
		break;
	case REL:    // 10. Relative Mode (branch instructions)
		sprintf(append_int, "%.4X", cpu->old_PC + cpu->offset + 2); // use old_cycle as we print after the instruction has completed
		strcpy(end, "$");
		strcat(end, append_int);
		break;
	case ZP:     // 11. Zero Page Mode
		sprintf(append_int, "%.2X", cpu->target_addr);
		strcpy(end, "$");
		strcat(end, append_int);
		break;
	case ZPX:    // 12. Zero Page Mode indexed via X
		sprintf(append_int, "%.2X", cpu->addr_lo);
		strcpy(end, "$");
		strcat(end, append_int);
		strcat(end, ",X");
		break;
	case ZPY:    // 13. Zero Page Mode indexed via Y
		sprintf(append_int, "%.2X", cpu->addr_lo);
		strcpy(end, "$");
		strcat(end, append_int);
		strcat(end, ",Y");
		break;
	default:
		printf("Invalid address mode\n");
		break;
	}
	strcat(instruction, end); // execute_* functions provide the instruction string
}

static void log_cpu_info(Cpu6502* cpu, const bool no_logging)
{
	if (!no_logging) {
		printf("%-6.4X ", cpu->old_PC);
		printf("%-20s ", cpu->instruction);
		printf("A:%.2X ", cpu->old_A);
		printf("X:%.2X ", cpu->old_X);
		printf("Y:%.2X ", cpu->old_Y);
		printf("P:%.2X ", cpu->old_P);
		printf("SP:%.2X ", cpu->old_stack);

		if (cpu->old_cycle == 0) {
			printf("CPU:%-10u", cpu->old_cycle);
		} else { // first cycle = +1 cycles due to the tick() after the instruction executes
			printf("CPU:%-10u", cpu->old_cycle - 1);
		}
	}
}

void update_cpu_info(Cpu6502* cpu)
{
	cpu->old_A = cpu->A;
	cpu->old_X = cpu->X;
	cpu->old_Y = cpu->Y;
	cpu->old_P = cpu->P;
	cpu->old_stack = cpu->stack;
	cpu->old_PC = cpu->PC;
	cpu->old_cycle = cpu->cycle;
}

static bool page_cross_occurs(const unsigned low_byte, const unsigned offset)
{
	return ((low_byte + offset) > 0xFF) ? 1 : 0;
}

void stack_push(Cpu6502* cpu, const uint8_t value)
{
	cpu->mem[SP_START + cpu->stack] = value;
	--cpu->stack; // automatically wraps around (8-bit variable)
}


uint8_t stack_pull(Cpu6502* cpu)
{
	unsigned result = 0;
	++cpu->stack; // automatically wraps around (8-bit variable)
	result = cpu->mem[SP_START + cpu->stack];
	return result;
}

static void fetch_opcode(Cpu6502* cpu)
{
	cpu->opcode = read_from_cpu(cpu, cpu->PC);
	++cpu->PC;

	cpu->instruction_cycles_remaining = max_cycles_opcode_lut[cpu->opcode];
	cpu->instruction_state = DECODE;
}

// Store operations can't skip cycles and are always fixed length unlike their load counterparts
// before skipping cycles check whether that is correct using this function
static bool fixed_cycles_on_store(const Cpu6502* cpu)
{
	bool result;
	switch(cpu->opcode) {
	case 0x91: /* STA, INDY */
	case 0x99: /* STA, ABSY */
	case 0x9D: /* STA, ASBX */
		result = true;
		break;
	default:
		result = false;
		break;
	}

	return result;
}


// not a const pointer as the function pointer that uses this is non-const
static void bad_op_code(Cpu6502* cpu)
{
	printf("invalid opcode: error 404: %.2X @ %.4X \n", cpu->opcode, cpu->PC);
}

static void decode_ABS_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ABS;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 3: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADL);
		++cpu->PC;
		break;
	case 2: // T2
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADH);
		++cpu->PC;
		break;
	case 1: //T3
		set_address_bus_bytes(cpu, cpu->addr_hi, cpu->addr_lo);
		cpu->target_addr = concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo);
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_ABS_rmw(Cpu6502* cpu)
{
	cpu->address_mode = ABS;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADL);
		++cpu->PC;
		break;
	case 4: // T2
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADH);
		++cpu->PC;
		break;
	case 3: // T3 (dummy read)
		set_address_bus_bytes(cpu, cpu->addr_hi, cpu->addr_lo);
		cpu->target_addr = concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo);
		set_data_bus_via_read(cpu, cpu->target_addr, DATA);
		break;
	case 2: // T4 (dummy write)
		write_to_cpu(cpu, cpu->target_addr, cpu->data_bus);
		break;
	case 1: // T5
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_ABSX_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ABSX;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 4: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADL);
		++cpu->PC;
		break;
	case 3: // T2
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADH);
		++cpu->PC;
		break;
	case 2: // T3 (non-page cross address)
		set_address_bus_bytes(cpu, cpu->addr_hi, cpu->addr_lo + cpu->X);
		cpu->target_addr = concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo + cpu->X);
		if (!fixed_cycles_on_store(cpu) && !page_cross_occurs(cpu->addr_lo, cpu->X)) {
			cpu->instruction_state = EXECUTE;
			break;
		}
		// dummy read (only if T4 executes next)
		set_data_bus_via_read(cpu, cpu->target_addr, DATA);
		break;
	case 1: // T4 (correct address, page crossed or not)
		set_address_bus(cpu, concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo) + cpu->X);
		cpu->target_addr = concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo) + cpu->X;
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_ABSX_rmw(Cpu6502* cpu)
{
	cpu->address_mode = ABSX;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 6: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADL);
		++cpu->PC;
		break;
	case 5: // T2
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADH);
		++cpu->PC;
		break;
	case 4: // T3 (dummy read)
		set_address_bus_bytes(cpu, cpu->addr_hi, cpu->addr_lo + cpu->X);
		cpu->target_addr = concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo + cpu->X);
		set_data_bus_via_read(cpu, cpu->target_addr, DATA);
		break;
	case 3: // T4
		set_address_bus(cpu, concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo) + cpu->X);
		cpu->target_addr = concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo) + cpu->X;
		set_data_bus_via_read(cpu, cpu->target_addr, DATA);
		break;
	case 2: // T5 (dummy write)
		write_to_cpu(cpu, cpu->target_addr, cpu->data_bus);
		break;
	case 1: // T6
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_ABSY_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ABSY;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 4: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADL);
		++cpu->PC;
		break;
	case 3: // T2
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADH);
		++cpu->PC;
		break;
	case 2: // T3 (non-page cross address)
		set_address_bus_bytes(cpu, cpu->addr_hi, cpu->addr_lo + cpu->Y);
		cpu->target_addr = concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo + cpu->Y);
		if (!fixed_cycles_on_store(cpu) && !page_cross_occurs(cpu->addr_lo, cpu->Y)) {
			cpu->instruction_state = EXECUTE;
			break;
		}
		// dummy read (only if T4 executes next)
		set_data_bus_via_read(cpu, cpu->target_addr, DATA);
		break;
	case 1: // T4 (correct address, page crossed or not)
		set_address_bus(cpu, concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo) + cpu->Y);
		cpu->target_addr = concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo) + cpu->Y;
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_ACC(Cpu6502* cpu)
{
	// opcode fetched: T0
	cpu->address_mode = ACC;
	// T1: fetch the next opcode (re-fetched on next T0 cycle)
	set_address_bus(cpu, cpu->PC);
	set_data_bus_via_read(cpu, cpu->PC, DATA);
	cpu->instruction_state = EXECUTE;
}

static void decode_IMM_read(Cpu6502* cpu)
{
	cpu->address_mode = IMM;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 1: // T1
		set_address_bus(cpu, cpu->PC);
		cpu->operand = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_IMP(Cpu6502* cpu)
{
	// opcode fetched: T0
	cpu->address_mode = IMP;
	// T1: fetch the next opcode (re-fetched on next T0 cycle)
	set_address_bus(cpu, cpu->PC);
	set_data_bus_via_read(cpu, cpu->PC, DATA);
	cpu->instruction_state = EXECUTE;
}

static void decode_INDX_read_store(Cpu6502* cpu)
{
	cpu->address_mode = INDX;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, BAL);
		++cpu->PC;
		break;
	case 4: // T2 (dummy read)
		set_address_bus_bytes(cpu, 0x00, cpu->base_addr);
		set_data_bus_via_read(cpu, cpu->base_addr, DATA);
		break;
	case 3: // T3
		set_address_bus_bytes(cpu, 0x00, cpu->base_addr + cpu->X);
		set_data_bus_via_read(cpu, (uint8_t) (cpu->base_addr + cpu->X), ADL);
		break;
	case 2: // T4
		set_address_bus_bytes(cpu, 0x00, cpu->base_addr + cpu->X + 1);
		set_data_bus_via_read(cpu, (uint8_t) (cpu->base_addr + cpu->X + 1), ADH);
		break;
	case 1: // T5
		set_address_bus_bytes(cpu, cpu->addr_hi, cpu->addr_lo);
		cpu->target_addr = concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo);
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_INDY_read_store(Cpu6502* cpu)
{
	cpu->address_mode = INDY;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, BAL);
		++cpu->PC;
		break;
	case 4: // T2
		set_address_bus_bytes(cpu, 0x00, cpu->base_addr);
		set_data_bus_via_read(cpu, cpu->base_addr, ADL); // ZP read
		break;
	case 3: // T3
		set_address_bus_bytes(cpu, 0x00, cpu->base_addr + 1);
		set_data_bus_via_read(cpu, (uint8_t) (cpu->base_addr + 1), ADH); // ZP read
		break;
	case 2: // T4 (non-page cross address)
		set_address_bus_bytes(cpu, cpu->addr_hi, cpu->addr_lo + cpu->Y);
		cpu->target_addr = concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo + cpu->Y);
		if (!fixed_cycles_on_store(cpu) && !page_cross_occurs(cpu->addr_lo, cpu->Y)) {
			cpu->instruction_state = EXECUTE;
			break;
		}
		// dummy read (only if T5 executes next)
		set_data_bus_via_read(cpu, cpu->target_addr, DATA);
		break;
	case 1: // T5 (correct address, page crossed or not)
		set_address_bus(cpu, concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo) + cpu->Y);
		cpu->target_addr = concat_address_bus_bytes(cpu->addr_hi, cpu->addr_lo) + cpu->Y;
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_ZP_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ZP;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 2: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADL);
		++cpu->PC;
		break;
	case 1: // T2
		set_address_bus_bytes(cpu, 0x00, cpu->addr_lo);
		cpu->target_addr = cpu->addr_lo;
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_ZP_rmw(Cpu6502* cpu)
{
	cpu->address_mode = ZP;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 4: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADL);
		++cpu->PC;
		break;
	case 3: // T2
		set_address_bus_bytes(cpu, 0x00, cpu->addr_lo);
		cpu->target_addr = cpu->addr_lo;
		set_data_bus_via_read(cpu, cpu->target_addr, DATA);
		break;
	case 2: // T3
		write_to_cpu(cpu, cpu->target_addr, cpu->data_bus);
		break;
	case 1: // T4
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_ZPX_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ZPX;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 3: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADL); // base address
		++cpu->PC;
		break;
	case 2: // T2 (dummy read)
		set_address_bus_bytes(cpu, 0x00, cpu->addr_lo);
		cpu->target_addr = cpu->addr_lo;
		set_data_bus_via_read(cpu, cpu->target_addr, DATA);
		break;
	case 1: // T3
		set_address_bus_bytes(cpu, 0x00, cpu->addr_lo + cpu->X);
		cpu->target_addr = (uint8_t) (cpu->addr_lo + cpu->X);
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_ZPX_rmw(Cpu6502* cpu)
{
	cpu->address_mode = ZPX;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADL);
		++cpu->PC;
		break;
	case 4: // T2 (dummy read)
		set_address_bus_bytes(cpu, 0x00, cpu->addr_lo);
		cpu->target_addr = cpu->addr_lo;
		set_data_bus_via_read(cpu, cpu->target_addr, DATA);
		break;
	case 3: // T3
		set_address_bus_bytes(cpu, 0x00, cpu->addr_lo + cpu->X);
		cpu->target_addr = (uint8_t) (cpu->addr_lo + cpu->X);
		set_data_bus_via_read(cpu, cpu->target_addr, DATA);
		break;
	case 2: // T4 (dummy write)
		write_to_cpu(cpu, cpu->target_addr, cpu->data_bus);
		break;
	case 1: // T5
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_ZPY_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ZPY;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 3: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADL); // base address
		++cpu->PC;
		break;
	case 2: // T2 (dummy read)
		set_address_bus_bytes(cpu, 0x00, cpu->addr_lo);
		cpu->target_addr = cpu->addr_lo;
		set_data_bus_via_read(cpu, cpu->target_addr, DATA);
		break;
	case 1: // T3
		set_address_bus_bytes(cpu, 0x00, cpu->addr_lo + cpu->Y);
		cpu->target_addr = (uint8_t) (cpu->addr_lo + cpu->Y);
		cpu->instruction_state = EXECUTE;
		break;
	default:
		printf("We shouldn't be here! invalid cycle\n");
		break;
	}
}

static void decode_ABS_JMP(Cpu6502* cpu)
{
	cpu->address_mode = ABS;
	cpu->cpu_ignore_fetch_on_nmi = true;
	// opcode fetched: T0
	cpu->instruction_state = EXECUTE;
}

static void decode_IND_JMP(Cpu6502* cpu)
{
	cpu->address_mode = IND;
	// opcode fetched: T0
	cpu->instruction_state = EXECUTE;
}

static void decode_SPECIAL(Cpu6502* cpu)
{
	cpu->address_mode = SPECIAL;
	// opcode fetched: T0
	cpu->instruction_state = EXECUTE;
}

static void decode_PUSH(Cpu6502* cpu)
{
	// Technically an IMP instruction but not handled by decode_IMP()
	cpu->address_mode = IMP; // used for debugger sets end = ""
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 2: // T1 (dummy read)
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, DATA);
		break;
	case 1: // T2
		cpu->instruction_state = EXECUTE;
		// push A or P onto stack w/ PHA or PHP
		break;
	}
}


static void decode_PULL(Cpu6502* cpu)
{
	// Technically an IMP instruction but not handled by decode_IMP()
	cpu->address_mode = IMP; // used for debugger sets end = ""
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 3: // T1 (dummy read)
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, DATA);
		break;
	case 2: // T2 (dummy read on stack)
		set_address_bus(cpu, SP_START + cpu->stack);
		set_data_bus_via_read(cpu, SP_START + cpu->stack, DATA);
		break;
	case 1: // T2
		cpu->instruction_state = EXECUTE;
		// pull A or P via execute functions
		break;
	}
}

static void decode_Bxx(Cpu6502* cpu) // branch instructions
{
	cpu->address_mode = REL;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 3: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, BRANCH); // -128 to +127 offset
		++cpu->PC;

		if (branch_not_taken(cpu)) {
			cpu->target_addr = cpu->PC; // already @ PC + 2 (1 from opcode and 1 from T1)
			cpu->instruction_state = EXECUTE;
		}
		break;
	case 2: // T2
		// w/o carry --> (PCH | (PC + offset) & 0xFF)
		set_address_bus_bytes(cpu, cpu->PC >> 8, (cpu->PC & 0x00FF) + cpu->offset);
		cpu->target_addr = (cpu->PC & 0xFF00) | ((cpu->PC + cpu->offset) & 0x00FF);
		if (!page_cross_occurs(cpu->PC & 0xFF, cpu->offset)) {
			cpu->instruction_state = EXECUTE;
		}
		break;
	case 1: // T3 (page cross)
		set_address_bus(cpu, cpu->PC + cpu->offset);
		cpu->target_addr = cpu->PC + cpu->offset;
		cpu->instruction_state = EXECUTE;
		break;
	}
}

static void decode_RTS(Cpu6502* cpu)
{
	cpu->address_mode = IMP;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1 (dummy read)
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, DATA);
		break;
	case 4: // T2 (dummy read on stack)
		set_address_bus(cpu, SP_START + cpu->stack);
		set_data_bus_via_read(cpu, SP_START + cpu->stack, DATA);
		break;
	case 3: // T3
		set_address_bus(cpu, SP_START + cpu->stack + 1); // pull increments SP to non-empty slot
		cpu->addr_lo = stack_pull(cpu);
		break;
	case 2: // T4
		set_address_bus(cpu, SP_START + cpu->stack + 1); // pull increments SP to non-empty slot
		cpu->addr_hi = stack_pull(cpu);
		break;
	case 1: // T5
		cpu->instruction_state = EXECUTE;
		break;
	}
}

/***************************
 * FLAGS                   *
 * *************************/

/* Bits : 7 ----------> 0 */
/* Flags: N V - - D I Z C */

static void update_flag_z(Cpu6502* cpu, const uint8_t result)
{
	/* Zero Flag Test */
	if (!result) {
		cpu->P |= FLAG_Z; /* Set Z */
	} else {
		cpu->P &= ~FLAG_Z; /* Clear Z */
	}
}


static void update_flag_n(Cpu6502* cpu, const uint8_t result)
{
	/* Negative Flag Test */
	if (result >> 7) {
		cpu->P |= FLAG_N; /* Set N */
	} else {
		cpu->P &= ~FLAG_N; /* Clear N */
	}
}


/* Parameters = 2 binary operands and then the result */
static void update_flag_v(Cpu6502* cpu, const bool overflow)
{
	/* Overflow Flag Test */
	if (overflow) {
		cpu->P |= FLAG_V; /* Set V */
	} else {
		cpu->P &= ~FLAG_V; /* Clear V */
	}
}


static void update_flag_c(Cpu6502* cpu, const int carry_out)
{
	if (carry_out) { // Carry out = result >> 8 (9th bit in ADC / SBC calc
		cpu->P |= FLAG_C; /* Set C */
	} else {
		cpu->P &= ~FLAG_C; /* Clear C */
	}
}


/*
 * Execute Functions (Opcode functions)
 */

/***************************
 * STORAGE                 *
 * *************************/

/* execute_LDA: LDA command - Load A with memory
 */
static void execute_LDA(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "LDA ");
	if (cpu->address_mode == IMM) {
		cpu->A = cpu->operand;
	} else {
		cpu->A = read_from_cpu(cpu, cpu->target_addr);
	}
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


/* execute_LDX: LDX command - Load X with memory
 */
static void execute_LDX(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "LDX ");
	if (cpu->address_mode == IMM) {
		cpu->X = cpu->operand;
	} else {
		cpu->X = read_from_cpu(cpu, cpu->target_addr);
	}
	update_flag_n(cpu, cpu->X);
	update_flag_z(cpu, cpu->X);
}


/* execute_LDY: LDY command - Load Y with memory
 */
static void execute_LDY(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "LDY ");
	if (cpu->address_mode == IMM) {
		cpu->Y = cpu->operand;
	} else {
		cpu->Y = read_from_cpu(cpu, cpu->target_addr);
	}
	update_flag_n(cpu, cpu->Y);
	update_flag_z(cpu, cpu->Y);
}


/* execute_STA: STA command - Store A in memory
 */
static void execute_STA(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "STA ");
	write_to_cpu(cpu, cpu->target_addr, cpu->A);
}


/* execute_STX: STX command - Store X in memory
 */
static void execute_STX(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "STX ");
	write_to_cpu(cpu, cpu->target_addr, cpu->X);
}


/* execute_STY: STY command - Store Y in memory
 */
static void execute_STY(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "STY ");
	write_to_cpu(cpu, cpu->target_addr, cpu->Y);
}


/* execute_TAX: TAX command - Transfer A to X
 */
static void execute_TAX(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "TAX ");
	cpu->X = cpu->A;
	update_flag_n(cpu, cpu->X);
	update_flag_z(cpu, cpu->X);
}


/* execute_TAY: TAY command - Transfer A to Y
 */
static void execute_TAY(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "TAY ");
	cpu->Y = cpu->A;
	update_flag_n(cpu, cpu->Y);
	update_flag_z(cpu, cpu->Y);
}


/* execute_TSX: TSX command - Transfer SP to X
 */
static void execute_TSX(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "TSX ");
	cpu->X = cpu->stack;
	update_flag_n(cpu, cpu->X);
	update_flag_z(cpu, cpu->X);
}


/* execute_TXA: TXA command - Transfer X to A
 */
static void execute_TXA(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "TXA ");
	cpu->A = cpu->X;
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


/* execute_TXS: TXS command - Transfer X to SP
 */
static void execute_TXS(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "TXS ");
	cpu->stack = cpu->X;
}


/* execute_TYA: TYA command - Transfer Y to A
 */
static void execute_TYA(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "TYA ");
	cpu->A = cpu->Y;
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}

/***************************
 * MATH                    *
 * *************************/

/* execute_ADC: ADC command - Add mem w/ A and C (A + M + C : then set flags)
 */
static void execute_ADC(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "ADC ");
	if (cpu->address_mode != IMM) {
		cpu->operand = read_from_cpu(cpu, cpu->target_addr);
	}

	int result = cpu->A + cpu->operand + (cpu->P & FLAG_C);
	bool overflow = ((cpu->A >> 7) == (cpu->operand >> 7))  // Overflow can only occur if MSBs are equal
				  && ((cpu->A >> 7) != ((result & 0xFF) >> 7));  // result narrowed to 8 bits
	cpu->A = result;  // Result is narrowed to 8 bits
	update_flag_n(cpu, cpu->A);
	update_flag_v(cpu, overflow);
	update_flag_z(cpu, cpu->A);
	update_flag_c(cpu, result >> 8);
}

/* execute_DEC: DEC command - Decrement Mem by one
 */
static void execute_DEC(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "DEC ");
	cpu->operand = read_from_cpu(cpu, cpu->target_addr);
	write_to_cpu(cpu, cpu->target_addr, cpu->operand - 1);
	update_flag_n(cpu, cpu->operand - 1);
	update_flag_z(cpu, cpu->operand - 1);
}


/* execute_DEX: DEX command - Decrement X by one
 */
static void execute_DEX(Cpu6502* cpu)
{
	/* Implied Mode */
	strcpy(cpu->instruction, "DEX ");
	--cpu->X;
	update_flag_n(cpu, cpu->X);
	update_flag_z(cpu, cpu->X);
}


/* execute_DEY: DEY command - Decrement Y by one
 */
static void execute_DEY(Cpu6502* cpu)
{
	/* Implied Mode */
	strcpy(cpu->instruction, "DEY ");
	--cpu->Y;
	update_flag_n(cpu, cpu->Y);
	update_flag_z(cpu, cpu->Y);
}


/* execute_INC: INC command - Increment Mem by one
 */
static void execute_INC(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "INC ");
	cpu->operand = read_from_cpu(cpu, cpu->target_addr);
	write_to_cpu(cpu, cpu->target_addr, cpu->operand + 1);
	update_flag_n(cpu, cpu->operand + 1);
	update_flag_z(cpu, cpu->operand + 1);
}


/* execute_INX: INX command - Increment X by one
 */
static void execute_INX(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "INX ");
	/* Implied Mode */
	++cpu->X;
	update_flag_n(cpu, cpu->X);
	update_flag_z(cpu, cpu->X);
}


/* execute_INY: INY command - Increment Y by one
 */
static void execute_INY(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "INY ");
	/* Implied Mode */
	++cpu->Y;
	update_flag_n(cpu, cpu->Y);
	update_flag_z(cpu, cpu->Y);
}


/* execute_SBC: SBC command - Subtract mem w/ A and C (A - M - !C : then set flags)
 */
static void execute_SBC(Cpu6502* cpu)

{
	strcpy(cpu->instruction, "SBC ");
	if (cpu->address_mode != IMM) {
		cpu->operand = read_from_cpu(cpu, cpu->target_addr);
	}

	int result = cpu->A - cpu->operand - !(cpu->P & FLAG_C);
	bool overflow = ((cpu->A >> 7) != (cpu->operand >> 7))  // Overflow can occur if MSBs are different
				  && ((cpu->A >> 7) != ((result & 0xFF) >> 7));  // narrow result to 8 bits
	cpu->A = result;  // Narrowed to 8 bits
	update_flag_n(cpu, cpu->A);
	update_flag_v(cpu, overflow);
	update_flag_z(cpu, cpu->A);
	update_flag_c(cpu, (result >= 0) ? 1 : 0);
}

/***************************
 * BITWISE                 *
 * *************************/

/* execute_AND: AND command - AND memory with Acc
 */
static void execute_AND(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "AND ");
	if (cpu->address_mode == IMM) {
		cpu->A &= cpu->operand;
	} else {
		cpu->A &= read_from_cpu(cpu, cpu->target_addr);
	}
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


/* execute_ASL: ASL command - Arithmetic Shift Left one bit (Acc or mem)
 * ASL == LSL
 */
static void execute_ASL(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "ASL ");
	unsigned high_bit = 0;
	if (cpu->address_mode == ACC) {
		high_bit = cpu->A & 0x80; /* Mask 7th bit */
		cpu->A = cpu->A << 1;
		update_flag_n(cpu, cpu->A);
		update_flag_z(cpu, cpu->A);
	} else {
		/* Shift value @ address 1 bit to the left */
		cpu->operand = read_from_cpu(cpu, cpu->target_addr);
		high_bit = cpu->operand & 0x80; /* Mask 7th bit */
		write_to_cpu(cpu, cpu->target_addr, cpu->operand << 1);
		update_flag_n(cpu, cpu->operand << 1);
		update_flag_z(cpu, cpu->operand << 1);
	}
	/* Update Carry */
	update_flag_c(cpu, high_bit >> 7);
}


/* execute_BIT: BIT command - BIT test (AND) between mem and Acc
 */
static void execute_BIT(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "BIT ");
	cpu->operand = read_from_cpu(cpu, cpu->target_addr);
	/* Update Flags */
	/* N = Bit 7, V = Bit 6 (of fetched operand) & Z = 1 (if AND result = 0) */

	update_flag_n(cpu, cpu->operand);
	update_flag_v(cpu, cpu->operand & FLAG_V);
	update_flag_z(cpu, cpu->operand & cpu->A);
}


/* execute_EOR: EOR command - Exclusive OR memory with Acc
 */
static void execute_EOR(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "EOR ");
	if (cpu->address_mode == IMM) {
		cpu->A ^= cpu->operand;
	} else {
		cpu->A ^= read_from_cpu(cpu, cpu->target_addr);
	}
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


/* execute_LSR: LSR command - Logical Shift Right by one bit (Acc or mem)
 */
static void execute_LSR(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "LSR ");
	unsigned low_bit = 0;
	if (cpu->address_mode == ACC) {
		low_bit = cpu->A & 0x01; /* Mask 0th bit */
		cpu->A = cpu->A >> 1;
		update_flag_n(cpu, cpu->A); /* Should always clear N flag */
		update_flag_z(cpu, cpu->A);
	} else {
		cpu->operand = read_from_cpu(cpu, cpu->target_addr);
		low_bit = cpu->operand & 0x01; /* Mask 0th bit */
		write_to_cpu(cpu, cpu->target_addr, cpu->operand >> 1);
		update_flag_n(cpu, cpu->operand >> 1); /* Should always clear N flag */
		update_flag_z(cpu, cpu->operand >> 1);
	}
	/* Update Carry */
	update_flag_c(cpu, low_bit);
}


/* execute_ORA: ORA command - OR memory with Acc
 */
static void execute_ORA(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "ORA ");
	if (cpu->address_mode == IMM) {
		cpu->A |= cpu->operand;
	} else {
		cpu->A |= read_from_cpu(cpu, cpu->target_addr);
	}
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


/* execute_ROL: ROL command - Rotate Shift Left one bit (Acc or mem)
 * ROL == LSL (execpt Carry Flag is copied into LSB & Carry = MSB after shift)
 */
static void execute_ROL(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "ROL ");
	unsigned high_bit = 0;
	if (cpu->address_mode == ACC) {
		high_bit = cpu->A & 0x80; /* Mask 7th bit */
		cpu->A = cpu->A << 1;
		/* Testing if Status Reg has a 1 in Carry Flag */
		if (cpu->P & FLAG_C) {
			cpu->A |= (cpu->P & FLAG_C);
		} /* if carry = 0 then do nothing as that still leaves a zero in the 0th bit */
		update_flag_n(cpu, cpu->A);
		update_flag_z(cpu, cpu->A);
	} else {
		cpu->operand = read_from_cpu(cpu, cpu->target_addr);
		high_bit = cpu->operand & 0x80; /* Mask 7th bit */
		unsigned result = cpu->operand << 1;
		write_to_cpu(cpu, cpu->target_addr, result);
		if (cpu->P & FLAG_C) {
			result |= 0x01;
			write_to_cpu(cpu, cpu->target_addr, result);
		}
		update_flag_n(cpu, result);
		update_flag_z(cpu, result);
	}
	/* Update Flag */
	update_flag_c(cpu, high_bit >> 7);
}


/* execute_ROR: ROR command - Rotate Shift Right one bit (Acc or mem)
 * ROR == LSR (execpt MSB = carry & LSB copied into carry)
 */
static void execute_ROR(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "ROR ");
	unsigned low_bit = 0;
	if (cpu->address_mode == ACC) {
		low_bit = cpu->A & 0x01; /* Mask 0th bit */
		cpu->A = cpu->A >> 1; /* Shift right */
		if (cpu->P & FLAG_C) {
			cpu->A |= 0x80; /* Set 7th bit to 1 - if carry = 1 */
		} /* if carry = 0 then do nothing as that still leaves a zero in the 0th bit */
		update_flag_n(cpu, cpu->A);
		update_flag_z(cpu, cpu->A);
	} else {
		cpu->operand = read_from_cpu(cpu, cpu->target_addr);
		low_bit = cpu->operand & 0x01;
		unsigned result = cpu->operand >> 1;
		write_to_cpu(cpu, cpu->target_addr, result);
		if (cpu->P & FLAG_C) {
			/* Set 7th bit to 1 - if carry = 1 */
			result |= 0x80;
			write_to_cpu(cpu, cpu->target_addr, result);
		} /* if carry = 0 then do nothing as that still leaves a zero in the 0th bit */
		update_flag_n(cpu, result);
		update_flag_z(cpu, result);
	}
	/* Update Carry */
	update_flag_c(cpu, low_bit);
}

/***************************
 * BRANCH                  *
 * *************************/
/* all are in RELATIVE address mode : -126 to +129 on pc (due to +2 @ end) */
/* No flag changes */

/* execute_BCC: BCC command - Branch on Carry Clear (C = 0)
 */
static void execute_BCC(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "BCC ");
	cpu->PC = cpu->target_addr;
}


/* execute_BCS: BCS command - Branch on Carry Set (C = 1)
 */
static void execute_BCS(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "BCS ");
	cpu->PC = cpu->target_addr;
}


/* execute_BEQ: BEQ command - Branch on Zero result (Z = 1)
 */
static void execute_BEQ(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "BEQ ");
	cpu->PC = cpu->target_addr;
}


/* execute_BMI: BMI command - Branch on Minus result (N = 1)
 */
static void execute_BMI(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "BMI ");
	cpu->PC = cpu->target_addr;
}


/* execute_BNE: BNE command - Branch on NOT Zero result (Z = 0)
 */
static void execute_BNE(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "BNE ");
	cpu->PC = cpu->target_addr;
}


/* execute_BPL: BPL command - Branch on Plus result (N = 0)
 */
static void execute_BPL(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "BPL ");
	cpu->PC = cpu->target_addr;
}


/* execute_BVC: BVC command - Branch on Overflow Clear (V = 0)
 */
static void execute_BVC(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "BVC ");
	cpu->PC = cpu->target_addr;
}


/* execute_BVS: BVS command - Branch on Overflow Set (V = 1)
 */
static void execute_BVS(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "BVS ");
	cpu->PC = cpu->target_addr;
}


/***************************
 * JUMP                    *
 * *************************/

/* execute_JMP: JMP command - JuMP to another location
 */
static void execute_JMP(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "JMP ");
	if (cpu->address_mode == ABS) {
		switch (cpu->instruction_cycles_remaining) {
		case 2: // T1
			set_address_bus(cpu, cpu->PC);
			set_data_bus_via_read(cpu, cpu->PC, ADL);
			++cpu->PC;
			break;
		case 1: // T2
			set_address_bus(cpu, cpu->PC);
			set_data_bus_via_read(cpu, cpu->PC, ADH);
			cpu->PC = (cpu->addr_hi << 8) | cpu->addr_lo; // END T2
			cpu->target_addr = (cpu->addr_hi << 8) | cpu->addr_lo; // for debugger function
			break;
		}
	} else if (cpu->address_mode == IND) {
		switch (cpu->instruction_cycles_remaining) {
		case 4: // T1
			set_address_bus(cpu, cpu->PC);
			set_data_bus_via_read(cpu, cpu->PC, INL);
			++cpu->PC;
			break;
		case 3: // T2
			set_address_bus(cpu, cpu->PC);
			set_data_bus_via_read(cpu, cpu->PC, INH);
			++cpu->PC;
			break;
		case 2: // T3
			set_address_bus_bytes(cpu, cpu->index_hi, cpu->index_lo);
			set_data_bus_via_read(cpu, (cpu->index_hi << 8) | cpu->index_lo, ADL);
			break;
		case 1: // T4
			set_address_bus(cpu, concat_address_bus_bytes(cpu->index_hi, cpu->index_lo) + 1);
			set_data_bus_via_read(cpu, ((cpu->index_hi << 8) | cpu->index_lo) + 1, ADH);
			if (cpu->index_lo == 0xFF) { // JMP bug
				set_address_bus_bytes(cpu, cpu->index_hi, 0x00);
				set_data_bus_via_read(cpu, cpu->index_hi << 8, ADH);
			}

			cpu->PC = (cpu->addr_hi << 8) | cpu->addr_lo; // END T4
			cpu->target_addr = (cpu->index_hi << 8) | cpu->index_lo; // for debugger function
			break;
		}
	}

	if (cpu->instruction_cycles_remaining != 1) {
		cpu->instruction_state = EXECUTE;
	}
}


/* execute_JSR: JSR command - Jump to SubRoutine
 */
static void execute_JSR(Cpu6502* cpu)
{
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADL);
		++cpu->PC;
		break;
	case 4: // T2 (dummy read on stack)
		set_address_bus(cpu, SP_START + cpu->stack);
		set_data_bus_via_read(cpu, SP_START + cpu->stack, DATA);
		break;
	case 3: // T3 (PC + 2 from read_op)
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 2: // T4 (PC + 2 from read_op)
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		stack_push(cpu, (uint8_t) cpu->PC); // push PCL onto stack
		break;
	case 1: // T5
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, ADH);
		cpu->target_addr = (cpu->addr_hi << 8) | cpu->addr_lo;
		cpu->PC = cpu->target_addr;

		strcpy(cpu->instruction, "JSR $");
		sprintf(cpu->append_int, "%.4X", cpu->target_addr);
		strcat(cpu->instruction, cpu->append_int);
		break;
	}

	if (cpu->instruction_cycles_remaining != 1) {
		cpu->instruction_state = EXECUTE;
	}
}


/* execute_RTI: RTI command - ReTurn from Interrupt
 */
static void execute_RTI(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "RTI");
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1 (dummy read)
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, DATA);
		break;
	case 4: // T2 (dummy read on stack)
		set_address_bus(cpu, SP_START + cpu->stack);
		set_data_bus_via_read(cpu, SP_START + cpu->stack, DATA);
		break;
	case 3: // T3
		set_address_bus(cpu, SP_START + cpu->stack + 1); // pull increments SP to non-empty slot
		cpu->P = stack_pull(cpu) | 0x20;
		break;
	case 2: // T4
		set_address_bus(cpu, SP_START + cpu->stack + 1); // pull increments SP to non-empty slot
		cpu->addr_lo = stack_pull(cpu);
		break;
	case 1: // T5
		set_address_bus(cpu, SP_START + cpu->stack + 1); // pull increments SP to non-empty slot
		cpu->addr_hi = stack_pull(cpu);
		cpu->PC = (cpu->addr_hi << 8) | cpu->addr_lo;
		break;
	}

	if (cpu->instruction_cycles_remaining != 1) {
		cpu->instruction_state = EXECUTE;
	}
}


/* execute_RTS: RTS command - ReTurn from Sub-routine
 */
static void execute_RTS(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "RTS");
	cpu->target_addr = (cpu->addr_hi << 8) | cpu->addr_lo;
	set_data_bus_via_read(cpu, cpu->target_addr, DATA); // dummy read
	cpu->PC = cpu->target_addr + 1;
}

/***************************
 * Registers               *
 * *************************/

/* execute_CLC: CLC command - Clear Carry flag
 */
static void execute_CLC(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "CLC");
	cpu->P &= ~FLAG_C;
}


/* execute_CLD: CLD command - Clear Decimal Mode (Decimal mode not supported in NES) 
 */
static void execute_CLD(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "CLD");
	cpu->P &= ~FLAG_D;

}


/* execute_CLI: CLI command - Clear Interrupt disable bit
 */
static void execute_CLI(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "CLI");
	cpu->P &= ~FLAG_I;
}


/* execute_CLV: CLV command - Clear Overflow flag
 */
static void execute_CLV(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "CLV");
	cpu->P &= ~FLAG_V;
}


/* execute_CMP: CMP command - Compare mem w/ A (A - M then set flags)
 */
static void execute_CMP(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "CMP ");
	/* CMP - same as SBC except result isn't stored and V flag isn't changed */
	if (cpu->address_mode != IMM) {
		cpu->operand = read_from_cpu(cpu, cpu->target_addr);
	}

	int result = cpu->A - cpu->operand;
	update_flag_n(cpu, result);
	update_flag_z(cpu, result);
	update_flag_c(cpu, (cpu->operand <= cpu->A) ? 1 : 0); // Borrow is ! of carry
}


/* execute_CPX: CPX command - Compare mem w/ X (X - M then set flags)
 */
static void execute_CPX(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "CPX ");
	if (cpu->address_mode != IMM) {
		cpu->operand = read_from_cpu(cpu, cpu->target_addr);
	}
	int result = cpu->X - cpu->operand;
	update_flag_n(cpu, result);
	update_flag_z(cpu, result);
	update_flag_c(cpu, (cpu->operand <= cpu->X) ? 1 : 0); // Borrow is ! of carry
}


/* execute_CPY: CPY command - Compare mem w/ Y (Y - M then set flags)
 */
static void execute_CPY(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "CPY ");
	if (cpu->address_mode != IMM) {
		cpu->operand = read_from_cpu(cpu, cpu->target_addr);
	}

	int result = cpu->Y - cpu->operand;
	update_flag_n(cpu, result);
	update_flag_z(cpu, result);
	update_flag_c(cpu, (cpu->operand <= cpu->Y) ? 1 : 0); // Borrow is ! of carry
}


/* execute_SEC: SEC command - Set Carry flag (C = 1)
 */
static void execute_SEC(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "SEC ");
	cpu->P |= FLAG_C;
}


/* execute_SED: SED command - Set Decimal Mode (Decimal mode not supported in NES)
 */
static void execute_SED(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "SED ");
	cpu->P |= FLAG_D;
}


/* execute_SEI: SEI command - Set Interrupt disable bit (I = 1)
 */
static void execute_SEI(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "SEI ");
	cpu->P |= FLAG_I;
}

/***************************
 * STACK                   *
 * *************************/

static void execute_PHA(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "PHA ");
	stack_push(cpu, cpu->A);
}

static void execute_PHP(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "PHP ");
	stack_push(cpu, cpu->P | 0x30); // set bits 4 & 5
}


static void execute_PLA(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "PLA ");
	cpu->A = stack_pull(cpu);
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


static void execute_PLP(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "PLP ");
	cpu->P = stack_pull(cpu) & ~ 0x10; // B flag may exist on stack but not P so it is cleared
	cpu->P |= 0x20; // bit 5 always set
}

/***************************
 * SYSTEM                  *
 * *************************/

/* execute_BRK: BRK command - Fore Break - Store PC & P (along w/ X, Y & A)
 */
static void execute_BRK(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "BRK ");
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 6: // T1 (dummy read)
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, DATA);
		++cpu->PC;
		break;
	case 5: // T2
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 4: // T3
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		stack_push(cpu, (uint8_t) cpu->PC); // push PCL onto stack
		++cpu->PC; // needed?
		break;
	case 3: // T4
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		stack_push(cpu, cpu->P | 0x30); // push status reg onto stack
		cpu->P |= FLAG_I;              /* Flag I is set */
		break;
	case 2: // T5
		set_address_bus(cpu, BRK_VECTOR);
		set_data_bus_via_read(cpu, BRK_VECTOR, ADL);
		break;
	case 1: // T6
		set_address_bus(cpu, BRK_VECTOR + 1);
		set_data_bus_via_read(cpu, BRK_VECTOR + 1, ADH);
		cpu->PC = (cpu->addr_hi << 8) | cpu->addr_lo;
		break;
	}

	if (cpu->instruction_cycles_remaining != 1) {
		cpu->instruction_state = EXECUTE;
	}
}


/* execute_NOP: NOP command - Does nothing (No OPeration)
 */
static void execute_NOP(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "NOP ");
	(void) cpu; // suppress unused variable compiler warning
}


/* Non opcode interrupts */
static void execute_IRQ(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "IRQ ");
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 6: // T1 (dummy read)
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, DATA);
		break;
	case 5: // T2
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 4: // T3
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		stack_push(cpu, (uint8_t) cpu->PC); // push PCL onto stack
		++cpu->PC;
		break;
	case 3: // T4
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		stack_push(cpu, cpu->P & ~0x30); // push status reg onto stack
		cpu->P |= FLAG_I;
		break;
	case 2: // T5
		set_address_bus(cpu, IRQ_VECTOR);
		set_data_bus_via_read(cpu, IRQ_VECTOR, ADL);
		break;
	case 1: // T6
		set_address_bus(cpu, IRQ_VECTOR + 1);
		set_data_bus_via_read(cpu, IRQ_VECTOR + 1, ADH);
		cpu->PC = (cpu->addr_hi << 8) | cpu->addr_lo;
		break;
	}

	if (cpu->instruction_cycles_remaining != 1) {
		cpu->instruction_state = EXECUTE;
	}
}


static void execute_NMI(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "NMI");
	cpu->address_mode = SPECIAL;
	// opcode fetched: T0
	switch (cpu->cpu_ppu_io->nmi_cycles_left) {
	case 6: // T1 (dummy read)
		set_address_bus(cpu, cpu->PC);
		set_data_bus_via_read(cpu, cpu->PC, DATA);
		break;
	case 5: // T2
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 4: // T3
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		stack_push(cpu, (uint8_t) cpu->PC); // push PCL onto stack
		++cpu->PC;
		break;
	case 3: // T4
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		stack_push(cpu, cpu->P & ~0x30); // push status reg onto stack
		cpu->P |= FLAG_I;
		break;
	case 2: // T5
		set_address_bus(cpu, NMI_VECTOR);
		set_data_bus_via_read(cpu, NMI_VECTOR, ADL);
		break;
	case 1: // T6
		set_address_bus(cpu, NMI_VECTOR + 1);
		set_data_bus_via_read(cpu, NMI_VECTOR + 1, ADH);
		cpu->PC = (cpu->addr_hi << 8) | cpu->addr_lo;
		cpu->cpu_ppu_io->nmi_pending = false;
		cpu->process_interrupt = false;
		cpu->cpu_ppu_io->nmi_cycles_left = 8;  // 8 as a decrement occurs after this function is called
		break;
	}
}


static void execute_DMA(Cpu6502* cpu, const bool no_logging)
{
	static bool first_cycle = true;
	if (first_cycle) {
#ifdef __DEBUG__
		cpu->cpu_ppu_io->write_debug = true;
		cpu_debugger(cpu, cpu->instruction, cpu->append_int, cpu->end);
		log_cpu_info(cpu, no_logging);
		update_cpu_info(cpu);
		first_cycle = false;
#endif /* __DEBUG__ */
	}
	/* Triggered by PPU, CPU is suspended */
	strcpy(cpu->instruction, "DMA");
	cpu->address_mode = SPECIAL;

	// initialise static (actually takes 513 cycles to complete)
	// this is due to the first idle tick(s) and the if (cycles < 513) check
	// which nullifies the last cycle skip
	static unsigned cycles_left = 514;

	// + 1 cycle on an odd cycle
	if (((cpu->cycle - 1) & 1) && cycles_left == 514) {  cycles_left += 1;  }

	if (cycles_left >= 513) {
		// dummy read
		--cycles_left;
	}

	// copy to dma, alternating read and writes
	unsigned index = 0;
	//static unsigned read = 0; // read comments below
	if (cycles_left < 513) {
		index = (512 - cycles_left) / 2; // index starts from 256 and ends on 0
		if (cycles_left % 2) { // read (on the first cycle)
			// have weird behaviour using static variable
			// just write the expected read on the second cycle
			//read = cpu->mem[(cpu->base_addr << 8) + index];
		} else { // write (on the second)
			cpu->cpu_ppu_io->oam[cpu->cpu_ppu_io->oam_addr + index] = cpu->mem[(cpu->base_addr << 8) + index];
		}
		--cycles_left;
	}

	// reset static
	if (cycles_left == 0) {
		cpu->instruction_state = FETCH;
		cpu->cpu_ppu_io->dma_pending = false;
		cycles_left = 514;
		first_cycle = true;
	}
}
