/*
 * Contains CPU architechture and functions
 */
#include "cpu.h"
#include "ppu.h"  // needed for read/write functions
#include "cart.h"
#include "cpu_mapper_interface.h"
#include "mappers.h"
#include "cpu_ppu_interface.h"

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
#define ADDR_MAPPER_START     0x4020U

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

struct InstructionDetails isa_info[256] = {
	/* 0x00 */ {"BRK", decode_SPECIAL,         execute_BRK, 7 },
	/* 0x01 */ {"ORA", decode_INDX_read_store, execute_ORA, 6 },
	/* 0x02 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x03 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x04 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x05 */ {"ORA", decode_ZP_read_store,   execute_ORA, 3 },
	/* 0x06 */ {"ASL", decode_ZP_rmw,          execute_ASL, 5 },
	/* 0x07 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x08 */ {"PHP", decode_PUSH,            execute_PHP, 3 },
	/* 0x09 */ {"ORA", decode_IMM_read,        execute_ORA, 2 },
	/* 0x0A */ {"ASL", decode_ACC,             execute_ASL, 2 },
	/* 0x0B */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x0C */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x0D */ {"ORA", decode_ABS_read_store,  execute_ORA, 4 },
	/* 0x0E */ {"ASL", decode_ABS_rmw,         execute_ASL, 6 },
	/* 0x0F */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0x10 */ {"BPL", decode_Bxx,             execute_BPL, 4 },
	/* 0x11 */ {"ORA", decode_INDY_read_store, execute_ORA, 6 },
	/* 0x12 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x13 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x14 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x15 */ {"ORA", decode_ZPX_read_store,  execute_ORA, 4 },
	/* 0x16 */ {"ASL", decode_ZPX_rmw,         execute_ASL, 6 },
	/* 0x17 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x18 */ {"CLC", decode_IMP,             execute_CLC, 2 },
	/* 0x19 */ {"ORA", decode_ABSY_read_store, execute_ORA, 5 },
	/* 0x1A */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x1B */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x1C */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x1D */ {"ORA", decode_ABSX_read_store, execute_ORA, 5 },
	/* 0x1E */ {"ASL", decode_ABSX_rmw,        execute_ASL, 7 },
	/* 0x1F */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0x20 */ {"JSR", decode_SPECIAL,         execute_JSR, 6 },
	/* 0x21 */ {"AND", decode_INDX_read_store, execute_AND, 6 },
	/* 0x22 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x23 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x24 */ {"BIT", decode_ZP_read_store,   execute_BIT, 3 },
	/* 0x25 */ {"AND", decode_ZP_read_store,   execute_AND, 3 },
	/* 0x26 */ {"ROL", decode_ZP_rmw,          execute_ROL, 5 },
	/* 0x27 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x28 */ {"PLP", decode_PULL,            execute_PLP, 4 },
	/* 0x29 */ {"AND", decode_IMM_read,        execute_AND, 2 },
	/* 0x2A */ {"ROL", decode_ACC,             execute_ROL, 2 },
	/* 0x2B */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x2C */ {"BIT", decode_ABS_read_store,  execute_BIT, 4 },
	/* 0x2D */ {"AND", decode_ABS_read_store,  execute_AND, 4 },
	/* 0x2E */ {"ROL", decode_ABS_rmw,         execute_ROL, 6 },
	/* 0x2F */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0x30 */ {"BMI", decode_Bxx,             execute_BMI, 4 },
	/* 0x31 */ {"AND", decode_INDY_read_store, execute_AND, 6 },
	/* 0x32 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x33 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x34 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x35 */ {"AND", decode_ZPX_read_store,  execute_AND, 4 },
	/* 0x36 */ {"ROL", decode_ZPX_rmw,         execute_ROL, 6 },
	/* 0x37 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x38 */ {"SEC", decode_IMP,             execute_SEC, 2 },
	/* 0x39 */ {"AND", decode_ABSY_read_store, execute_AND, 5 },
	/* 0x3A */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x3B */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x3C */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x3D */ {"AND", decode_ABSX_read_store, execute_AND, 5 },
	/* 0x3E */ {"ROL", decode_ABSX_rmw,        execute_ROL, 7 },
	/* 0x3F */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0x40 */ {"RTI", decode_SPECIAL,         execute_RTI, 6 },
	/* 0x41 */ {"EOR", decode_INDX_read_store, execute_EOR, 6 },
	/* 0x42 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x43 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x44 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x45 */ {"EOR", decode_ZP_read_store,   execute_EOR, 3 },
	/* 0x46 */ {"LSR", decode_ZP_rmw,          execute_LSR, 5 },
	/* 0x47 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x48 */ {"PHA", decode_PUSH,            execute_PHA, 3 },
	/* 0x49 */ {"EOR", decode_IMM_read,        execute_EOR, 2 },
	/* 0x4A */ {"LSR", decode_ACC,             execute_LSR, 2 },
	/* 0x4B */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x4C */ {"JMP", decode_ABS_JMP,         execute_JMP, 3 },
	/* 0x4D */ {"EOR", decode_ABS_read_store,  execute_EOR, 4 },
	/* 0x4E */ {"LSR", decode_ABS_rmw,         execute_LSR, 6 },
	/* 0x4F */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0x50 */ {"BVC", decode_Bxx,             execute_BVC, 4 },
	/* 0x51 */ {"EOR", decode_INDY_read_store, execute_EOR, 6 },
	/* 0x52 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x53 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x54 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x55 */ {"EOR", decode_ZPX_read_store,  execute_EOR, 4 },
	/* 0x56 */ {"LSR", decode_ZPX_rmw,         execute_LSR, 6 },
	/* 0x57 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x58 */ {"CLI", decode_IMP,             execute_CLI, 2 },
	/* 0x59 */ {"EOR", decode_ABSY_read_store, execute_EOR, 5 },
	/* 0x5A */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x5B */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x5C */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x5D */ {"EOR", decode_ABSX_read_store, execute_EOR, 5 },
	/* 0x5E */ {"LSR", decode_ABSX_rmw,        execute_LSR, 7 },
	/* 0x5F */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0x60 */ {"RTS", decode_RTS,             execute_RTS, 6 },
	/* 0x61 */ {"ADC", decode_INDX_read_store, execute_ADC, 6 },
	/* 0x62 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x63 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x64 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x65 */ {"ADC", decode_ZP_read_store,   execute_ADC, 3 },
	/* 0x66 */ {"ROR", decode_ZP_rmw,          execute_ROR, 5 },
	/* 0x67 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x68 */ {"PLA", decode_PULL,            execute_PLA, 4 },
	/* 0x69 */ {"ADC", decode_IMM_read,        execute_ADC, 2 },
	/* 0x6A */ {"ROR", decode_ACC,             execute_ROR, 2 },
	/* 0x6B */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x6C */ {"JMP", decode_IND_JMP,         execute_JMP, 5 },
	/* 0x6D */ {"ADC", decode_ABS_read_store,  execute_ADC, 4 },
	/* 0x6E */ {"ROR", decode_ABS_rmw,         execute_ROR, 6 },
	/* 0x6F */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0x70 */ {"BVS", decode_Bxx,             execute_BVS, 4 },
	/* 0x71 */ {"ADC", decode_INDY_read_store, execute_ADC, 6 },
	/* 0x72 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x73 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x74 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x75 */ {"ADC", decode_ZPX_read_store,  execute_ADC, 4 },
	/* 0x76 */ {"ROR", decode_ZPX_rmw,         execute_ROR, 6 },
	/* 0x77 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x78 */ {"SEI", decode_IMP,             execute_SEI, 2 },
	/* 0x79 */ {"ADC", decode_ABSY_read_store, execute_ADC, 5 },
	/* 0x7A */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x7B */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x7C */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x7D */ {"ADC", decode_ABSX_read_store, execute_ADC, 5 },
	/* 0x7E */ {"ROR", decode_ABSX_rmw,        execute_ROR, 7 },
	/* 0x7F */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0x80 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x81 */ {"STA", decode_INDX_read_store, execute_STA, 6 },
	/* 0x82 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x83 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x84 */ {"STY", decode_ZP_read_store,   execute_STY, 3 },
	/* 0x85 */ {"STA", decode_ZP_read_store,   execute_STA, 3 },
	/* 0x86 */ {"STX", decode_ZP_read_store,   execute_STX, 3 },
	/* 0x87 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x88 */ {"DEY", decode_IMP,             execute_DEY, 2 },
	/* 0x89 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x8A */ {"TXA", decode_IMP,             execute_TXA, 2 },
	/* 0x8B */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x8C */ {"STY", decode_ABS_read_store,  execute_STY, 4 },
	/* 0x8D */ {"STA", decode_ABS_read_store,  execute_STA, 4 },
	/* 0x8E */ {"STX", decode_ABS_read_store,  execute_STX, 4 },
	/* 0x8F */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0x90 */ {"BCC", decode_Bxx,             execute_BCC, 4 },
	/* 0x91 */ {"STA", decode_INDY_read_store, execute_STA, 6 },
	/* 0x92 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x93 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x94 */ {"STY", decode_ZPX_read_store,  execute_STY, 4 },
	/* 0x95 */ {"STA", decode_ZPX_read_store,  execute_STA, 4 },
	/* 0x96 */ {"STX", decode_ZPY_read_store,  execute_STX, 4 },
	/* 0x97 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x98 */ {"TYA", decode_IMP,             execute_TYA, 2 },
	/* 0x99 */ {"STA", decode_ABSY_read_store, execute_STA, 5 },
	/* 0x9A */ {"TXS", decode_IMP,             execute_TXS, 2 },
	/* 0x9B */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x9C */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x9D */ {"STA", decode_ABSX_read_store, execute_STA, 5 },
	/* 0x9E */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0x9F */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0xA0 */ {"LDY", decode_IMM_read,        execute_LDY, 2 },
	/* 0xA1 */ {"LDA", decode_INDX_read_store, execute_LDA, 6 },
	/* 0xA2 */ {"LDX", decode_IMM_read,        execute_LDX, 2 },
	/* 0xA3 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xA4 */ {"LDY", decode_ZP_read_store,   execute_LDY, 3 },
	/* 0xA5 */ {"LDA", decode_ZP_read_store,   execute_LDA, 3 },
	/* 0xA6 */ {"LDX", decode_ZP_read_store,   execute_LDX, 3 },
	/* 0xA7 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xA8 */ {"TAY", decode_IMP,             execute_TAY, 2 },
	/* 0xA9 */ {"LDA", decode_IMM_read,        execute_LDA, 2 },
	/* 0xAA */ {"TAX", decode_IMP,             execute_TAX, 2 },
	/* 0xAB */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xAC */ {"LDY", decode_ABS_read_store,  execute_LDY, 4 },
	/* 0xAD */ {"LDA", decode_ABS_read_store,  execute_LDA, 4 },
	/* 0xAE */ {"LDX", decode_ABS_read_store,  execute_LDX, 4 },
	/* 0xAF */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0xB0 */ {"BCS", decode_Bxx,             execute_BCS, 4 },
	/* 0xB1 */ {"LDA", decode_INDY_read_store, execute_LDA, 6 },
	/* 0xB2 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xB3 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xB4 */ {"LDY", decode_ZPX_read_store,  execute_LDY, 4 },
	/* 0xB5 */ {"LDA", decode_ZPX_read_store,  execute_LDA, 4 },
	/* 0xB6 */ {"LDX", decode_ZPY_read_store,  execute_LDX, 4 },
	/* 0xB7 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xB8 */ {"CLV", decode_IMP,             execute_CLV, 2 },
	/* 0xB9 */ {"LDA", decode_ABSY_read_store, execute_LDA, 5 },
	/* 0xBA */ {"TSX", decode_IMP,             execute_TSX, 2 },
	/* 0xBB */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xBC */ {"LDY", decode_ABSX_read_store, execute_LDY, 5 },
	/* 0xBD */ {"LDA", decode_ABSX_read_store, execute_LDA, 5 },
	/* 0xBE */ {"LDX", decode_ABSY_read_store, execute_LDX, 5 },
	/* 0xBF */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0xC0 */ {"CPY", decode_IMM_read,        execute_CPY, 2 },
	/* 0xC1 */ {"CMP", decode_INDX_read_store, execute_CMP, 6 },
	/* 0xC2 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xC3 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xC4 */ {"CPY", decode_ZP_read_store,   execute_CPY, 3 },
	/* 0xC5 */ {"CMP", decode_ZP_read_store,   execute_CMP, 3 },
	/* 0xC6 */ {"DEC", decode_ZP_rmw,          execute_DEC, 5 },
	/* 0xC7 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xC8 */ {"INY", decode_IMP,             execute_INY, 2 },
	/* 0xC9 */ {"CMP", decode_IMM_read,        execute_CMP, 2 },
	/* 0xCA */ {"DEX", decode_IMP,             execute_DEX, 2 },
	/* 0xCB */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xCC */ {"CPY", decode_ABS_read_store,  execute_CPY, 4 },
	/* 0xCD */ {"CMP", decode_ABS_read_store,  execute_CMP, 4 },
	/* 0xCE */ {"DEC", decode_ABS_rmw,         execute_DEC, 6 },
	/* 0xCF */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0xD0 */ {"BNE", decode_Bxx,             execute_BNE, 4 },
	/* 0xD1 */ {"CMP", decode_INDY_read_store, execute_CMP, 6 },
	/* 0xD2 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xD3 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xD4 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xD5 */ {"CMP", decode_ZPX_read_store,  execute_CMP, 4 },
	/* 0xD6 */ {"DEC", decode_ZPX_rmw,         execute_DEC, 6 },
	/* 0xD7 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xD8 */ {"CLD", decode_IMP,             execute_CLD, 2 },
	/* 0xD9 */ {"CMP", decode_ABSY_read_store, execute_CMP, 5 },
	/* 0xDA */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xDB */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xDC */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xDD */ {"CMP", decode_ABSX_read_store, execute_CMP, 5 },
	/* 0xDE */ {"DEC", decode_ABSX_rmw,        execute_DEC, 7 },
	/* 0xDF */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0xE0 */ {"CPX", decode_IMM_read,        execute_CPX, 2 },
	/* 0xE1 */ {"SBC", decode_INDX_read_store, execute_SBC, 6 },
	/* 0xE2 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xE3 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xE4 */ {"CPX", decode_ZP_read_store,   execute_CPX, 3 },
	/* 0xE5 */ {"SBC", decode_ZP_read_store,   execute_SBC, 3 },
	/* 0xE6 */ {"INC", decode_ZP_rmw,          execute_INC, 5 },
	/* 0xE7 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xE8 */ {"INX", decode_IMP,             execute_INX, 2 },
	/* 0xE9 */ {"SBC", decode_IMM_read,        execute_SBC, 2 },
	/* 0xEA */ {"NOP", decode_IMP,             execute_NOP, 2 },
	/* 0xEB */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xEC */ {"CPX", decode_ABS_read_store,  execute_CPX, 4 },
	/* 0xED */ {"SBC", decode_ABS_read_store,  execute_SBC, 4 },
	/* 0xEE */ {"INC", decode_ABS_rmw,         execute_INC, 6 },
	/* 0xEF */ {"",    bad_op_code,            bad_op_code, 0 },

	/* 0xF0 */ {"BEQ", decode_Bxx,             execute_BEQ, 4 },
	/* 0xF1 */ {"SBC", decode_INDY_read_store, execute_SBC, 6 },
	/* 0xF2 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xF3 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xF4 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xF5 */ {"SBC", decode_ZPX_read_store,  execute_SBC, 4 },
	/* 0xF6 */ {"INC", decode_ZPX_rmw,         execute_INC, 6 },
	/* 0xF7 */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xF8 */ {"SED", decode_IMP,             execute_SED, 2 },
	/* 0xF9 */ {"SBC", decode_ABSY_read_store, execute_SBC, 5 },
	/* 0xFA */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xFB */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xFC */ {"",    bad_op_code,            bad_op_code, 0 },
	/* 0xFD */ {"SBC", decode_ABSX_read_store, execute_SBC, 5 },
	/* 0xFE */ {"INC", decode_ABSX_rmw,        execute_INC, 7 },
	/* 0xFF */ {"",    bad_op_code,            bad_op_code, 0 }
};

void (*hardware_interrupts[3])(Cpu6502* cpu) = {
	execute_DMA, execute_IRQ, execute_NMI,
};


Cpu6502* cpu_allocator(void)
{
	Cpu6502* cpu = malloc(sizeof(Cpu6502));
	if (!cpu) {
		fprintf(stderr, "Failed to allocate enough memory for CPU\n");
	}
	return cpu; // either returns a valid or NULL pointer
}

int cpu_init(Cpu6502* cpu, uint16_t pc_init, CpuPpuShare* cp, CpuMapperShare* cm)
{
	int return_code = -1;

	cpu->cpu_ppu_io = cp;
	cpu->cpu_mapper_io = cm;

	cpu->PC = pc_init;

	cpu->stack = 0xFD; // After startup stack pointer is FD
	cpu->cycle = 0;
	cpu->P = 0x24;
	cpu->A = 0;
	cpu->X = 0;
	cpu->Y = 0;
	cpu->old_cycle = 0;
	cpu->instruction_state = FETCH;
	cpu->instruction_cycles_remaining = 51; // initial value doesn't matter as LUT will set it after first instruction is read

	cpu->delay_nmi = false;
	cpu->cpu_ignore_fetch_on_nmi = false;
	cpu->process_interrupt = false;

	cpu->controller_latch = 0;
	cpu->player_1_controller = 0;
	cpu->player_2_controller = 0;

	memset(cpu->mem, 0, CPU_MEMORY_SIZE); // Zero out memory

	return_code = 0;

	return return_code;
}


void init_pc(Cpu6502* cpu)
{
	cpu->PC = return_little_endian(cpu, RST_VECTOR);
}

/* Read from memory or indexed registers and accumulator (X/Y/A)
 *
 * mem_type determines where we read from
 *   INTERNAL_MEM, read from memory address regardless of address mode
 *   ADDRESS_MODE_DEP, read from memory or accumulator depending on address mode
 *   INTERNAL_REG, read from indexed registers or accumulator (X/Y/A)
 */
uint8_t cpu_generic_read(Cpu6502* cpu, enum CpuMemType mem_type
                        , AddressMode address_mode
                        , uint16_t read_address, const uint8_t* internal_reg)
{
	uint8_t read_val = 0; // default value, if reg w/ a NULL

	if (mem_type == ADDRESS_MODE_DEP) {
		mem_type = INTERNAL_MEM;
		if (address_mode == ACC) {
			mem_type = INTERNAL_REG;
			internal_reg = &cpu->A;
		}
	}

	// avoid any side effects by only reading from memory if necessary
	if (mem_type == INTERNAL_MEM) {
		read_val = read_from_cpu(cpu, read_address);
	}

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
	} else if (addr >= ADDR_MAPPER_START) {
		read = mapper_read(cpu, addr);
	} else {
		read = cpu->mem[addr]; /* catch-all */
	}

	return read;
}

uint8_t read_ppu_reg(const uint16_t addr, Cpu6502* cpu)
{
	uint8_t data = 0;
	switch (addr) {
	case 0x2002:
		// PPU_STATUS
		read_2002(cpu->cpu_ppu_io);
		data = cpu->cpu_ppu_io->return_value;
		break;
	case 0x2004:
		// OAM_DATA
		read_2004(cpu->cpu_ppu_io);
		data = cpu->cpu_ppu_io->return_value;
		break;
	case 0x2007:
		// PPU_DATA
		read_2007(cpu->cpu_ppu_io);
		data = cpu->cpu_ppu_io->return_value;
		break;
	}
	return data;
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

void cpu_generic_write(Cpu6502* cpu, enum CpuMemType mem_type
                      , AddressMode address_mode
                      , uint16_t write_address, uint8_t* internal_reg
                      , uint8_t data)
{
	if (mem_type == ADDRESS_MODE_DEP) {
		mem_type = INTERNAL_MEM;
		if (address_mode == ACC) {
			mem_type = INTERNAL_REG;
			internal_reg = &cpu->A;
		}
	}

	// avoid any side effects by only writing to memory if necessary
	if (mem_type == INTERNAL_MEM) {
		write_to_cpu(cpu, write_address, data);
	}

	if ((mem_type == INTERNAL_REG) && (internal_reg != NULL)) {
		*internal_reg = data; // should point to X, Y or A registers
	}
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
	} else if (addr >= 0x4020) { // Mapper space/region
		mapper_write(cpu, addr, val);
	} else {
		cpu->mem[addr] = val;
	}
}

void write_ppu_reg(const uint16_t addr, const uint8_t data, Cpu6502* cpu)
{
	switch (addr) {
	case 0x2000:
		// PPU_CTRL
		if (ppu_status_vblank_bit_set(cpu->cpu_ppu_io)
		    && !ppu_ctrl_gen_nmi_bit_set(cpu->cpu_ppu_io)
		    && (data & 0x80)) {
			cpu->cpu_ppu_io->nmi_pending = true;
			cpu->delay_nmi = true;
		}

		cpu->cpu_ppu_io->ppu_ctrl = data;
		write_2000(data, cpu->cpu_ppu_io);
		break;
	case 0x2001:
		// PPU_MASK
		cpu->cpu_ppu_io->ppu_mask = data;
		break;
	case 0x2003:
		// OAM_ADDR
		write_2003(data, cpu->cpu_ppu_io);
		break;
	case 0x2004:
		// OAM_DATA
		cpu->cpu_ppu_io->oam_data = data;
		write_2004(data, cpu->cpu_ppu_io);
		break;
	case 0x2005:
		// PPU_SCROLL (write * 2)
		write_2005(data, cpu->cpu_ppu_io);
		break;
	case 0x2006:
		// PPU_ADDR (write * 2)
		write_2006(data, cpu->cpu_ppu_io);
		break;
	case 0x2007:
		// OAM_DATA
		cpu->cpu_ppu_io->ppu_data = data;
		write_2007(data, cpu->cpu_mapper_io->chr->ram_size, cpu->cpu_ppu_io);
		break;
	case 0x4014:
		// DMA
		write_4014(data, cpu);
		break;
	}
}

void delay_write_ppu_reg(const uint16_t addr, const uint8_t data, Cpu6502* cpu)
{
	cpu->cpu_ppu_io->buffer_write = true;
	cpu->cpu_ppu_io->buffer_counter = 2;
	if (addr == 0x2001) {
		cpu->cpu_ppu_io->buffer_counter += 3; // 3 dot delay for background/sprite rendering
	}
	cpu->cpu_ppu_io->buffer_address = addr;
	cpu->cpu_ppu_io->buffer_value = data;

	cpu->cpu_ppu_io->ppu_status &= ~0x1F;
	cpu->cpu_ppu_io->ppu_status |= (data & 0x1F);
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
			isa_info[cpu->opcode].decode_opcode(cpu);
		}
		if (cpu->instruction_state == EXECUTE) {
			cpu->instruction_state = FETCH;
			isa_info[cpu->opcode].execute_opcode(cpu); // can change the PC which the early fetch made!

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
	set_data_bus_via_write(cpu, cpu->opcode);
	++cpu->PC;

	cpu->instruction_cycles_remaining = isa_info[cpu->opcode].max_cycles;
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
		cpu->target_addr = cpu->address_bus;
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
		set_address_bus_bytes(cpu, cpu->PC >> 8, (uint8_t) (cpu->PC + cpu->offset));
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
		set_data_bus_via_write(cpu, cpu->addr_lo);
		break;
	case 2: // T4
		set_address_bus(cpu, SP_START + cpu->stack + 1); // pull increments SP to non-empty slot
		cpu->addr_hi = stack_pull(cpu);
		set_data_bus_via_write(cpu, cpu->addr_hi);
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
	cpu->A = read_from_cpu(cpu, cpu->target_addr);
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


/* execute_LDX: LDX command - Load X with memory
 */
static void execute_LDX(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "LDX ");
	cpu->X = read_from_cpu(cpu, cpu->target_addr);
	update_flag_n(cpu, cpu->X);
	update_flag_z(cpu, cpu->X);
}


/* execute_LDY: LDY command - Load Y with memory
 */
static void execute_LDY(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "LDY ");
	cpu->Y = read_from_cpu(cpu, cpu->target_addr);
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
	cpu->operand = read_from_cpu(cpu, cpu->target_addr);

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
	cpu->operand = read_from_cpu(cpu, cpu->target_addr);

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
	cpu->A &= read_from_cpu(cpu, cpu->target_addr);
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


/* execute_ASL: ASL command - Arithmetic Shift Left one bit (Acc or mem)
 * ASL == LSL
 */
static void execute_ASL(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "ASL ");
	// read from memory or accumulator
	uint8_t operand = cpu_generic_read(cpu, ADDRESS_MODE_DEP
	                                  , cpu->address_mode
	                                  , cpu->target_addr
	                                  , NULL);
	unsigned high_bit = operand & 0x80; // Mask 7th bit
	uint8_t asl_result = operand << 1;
	cpu_generic_write(cpu, ADDRESS_MODE_DEP, cpu->address_mode
	                 , cpu->target_addr, NULL, asl_result);

	update_flag_n(cpu, asl_result);
	update_flag_z(cpu, asl_result);
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
	cpu->A ^= read_from_cpu(cpu, cpu->target_addr);
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


/* execute_LSR: LSR command - Logical Shift Right by one bit (Acc or mem)
 */
static void execute_LSR(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "LSR ");
	// read from memory or accumulator
	uint8_t operand = cpu_generic_read(cpu, ADDRESS_MODE_DEP
	                                  , cpu->address_mode
	                                  , cpu->target_addr
	                                  , NULL);
	unsigned low_bit = operand & 0x01; // Mask 0th bit
	uint8_t lsr_result = operand >> 1;
	cpu_generic_write(cpu, ADDRESS_MODE_DEP, cpu->address_mode
	                 , cpu->target_addr, NULL, lsr_result);

	update_flag_n(cpu, lsr_result);
	update_flag_z(cpu, lsr_result);
	update_flag_c(cpu, low_bit);
}


/* execute_ORA: ORA command - OR memory with Acc
 */
static void execute_ORA(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "ORA ");
	cpu->A |= read_from_cpu(cpu, cpu->target_addr);
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


/* execute_ROL: ROL command - Rotate Shift Left one bit (Acc or mem)
 * ROL == LSL (execpt Carry Flag is copied into LSB & Carry = MSB after shift)
 */
static void execute_ROL(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "ROL ");
	// read from memory or accumulator
	uint8_t operand = cpu_generic_read(cpu, ADDRESS_MODE_DEP
	                                  , cpu->address_mode
	                                  , cpu->target_addr
	                                  , NULL);
	unsigned high_bit = operand & 0x80; // Mask 7th bit
	uint8_t rol_result = operand << 1;
	if (cpu->P & FLAG_C) {
		rol_result |= FLAG_C; // Copy carry into LSB (bit 0)
	} // if carry = 0 do nothing as that still leaves a zero in the 0th bit
	cpu_generic_write(cpu, ADDRESS_MODE_DEP, cpu->address_mode
	                 , cpu->target_addr, NULL, rol_result);

	update_flag_n(cpu, rol_result);
	update_flag_z(cpu, rol_result);
	update_flag_c(cpu, high_bit >> 7);
}


/* execute_ROR: ROR command - Rotate Shift Right one bit (Acc or mem)
 * ROR == LSR (execpt MSB = carry & LSB copied into carry)
 */
static void execute_ROR(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "ROR ");
	// read from memory or accumulator
	uint8_t operand = cpu_generic_read(cpu, ADDRESS_MODE_DEP
	                                  , cpu->address_mode
	                                  , cpu->target_addr
	                                  , NULL);
	unsigned low_bit = operand & 0x01; // Mask 0th bit
	uint8_t ror_result = operand >> 1;
	if (cpu->P & FLAG_C) {
		ror_result |= 0x80; // Copy carry into MSB (bit 7)
	} // if carry = 0 do nothing as that still leaves a zero in the 7th bit
	cpu_generic_write(cpu, ADDRESS_MODE_DEP, cpu->address_mode
	                 , cpu->target_addr, NULL, ror_result);

	update_flag_n(cpu, ror_result);
	update_flag_z(cpu, ror_result);
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
		set_data_bus_via_write(cpu, cpu->PC >> 8);
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 2: // T4 (PC + 2 from read_op)
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		set_data_bus_via_write(cpu, cpu->PC);
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
		set_data_bus_via_read(cpu, SP_START + cpu->stack, DATA);
		break;
	case 2: // T4
		set_address_bus(cpu, SP_START + cpu->stack + 1); // pull increments SP to non-empty slot
		cpu->addr_lo = stack_pull(cpu);
		set_data_bus_via_write(cpu, cpu->addr_lo);
		break;
	case 1: // T5
		set_address_bus(cpu, SP_START + cpu->stack + 1); // pull increments SP to non-empty slot
		cpu->addr_hi = stack_pull(cpu);
		set_data_bus_via_write(cpu, cpu->addr_hi);
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
	cpu->operand = read_from_cpu(cpu, cpu->target_addr);

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
	cpu->operand = read_from_cpu(cpu, cpu->target_addr);
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
	cpu->operand = read_from_cpu(cpu, cpu->target_addr);

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
	set_data_bus_via_write(cpu, cpu->A);
	stack_push(cpu, cpu->A);
}

static void execute_PHP(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "PHP ");
	set_data_bus_via_write(cpu, cpu->P | 0x30);
	stack_push(cpu, cpu->P | 0x30); // set bits 4 & 5
}


static void execute_PLA(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "PLA ");
	cpu->A = stack_pull(cpu);
	set_data_bus_via_write(cpu, cpu->A);
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


static void execute_PLP(Cpu6502* cpu)
{
	strcpy(cpu->instruction, "PLP ");
	cpu->P = stack_pull(cpu) & ~ 0x10; // B flag may exist on stack but not P so it is cleared
	cpu->P |= 0x20; // bit 5 always set
	set_data_bus_via_read(cpu, SP_START + cpu->stack, DATA);
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
		set_data_bus_via_write(cpu, cpu->PC >> 8);
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 4: // T3
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		set_data_bus_via_write(cpu, cpu->PC);
		stack_push(cpu, (uint8_t) cpu->PC); // push PCL onto stack
		++cpu->PC; // needed?
		break;
	case 3: // T4
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		set_data_bus_via_write(cpu, cpu->P | 0x30);
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
		set_data_bus_via_write(cpu, cpu->PC >> 8);
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 4: // T3
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		set_data_bus_via_write(cpu, cpu->PC);
		stack_push(cpu, (uint8_t) cpu->PC); // push PCL onto stack
		++cpu->PC;
		break;
	case 3: // T4
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		set_data_bus_via_write(cpu, cpu->P & ~0x30);
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
		set_data_bus_via_write(cpu, cpu->PC >> 8);
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 4: // T3
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		set_data_bus_via_write(cpu, cpu->PC);
		stack_push(cpu, (uint8_t) cpu->PC); // push PCL onto stack
		++cpu->PC;
		break;
	case 3: // T4
		set_address_bus(cpu, SP_START + cpu->stack); // SP points to an empty slot
		set_data_bus_via_write(cpu, cpu->P & ~0x30);
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
