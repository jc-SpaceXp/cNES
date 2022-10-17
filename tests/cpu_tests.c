#include <check.h>

#include <ctype.h>

#include "mappers.h"
#include "cpu.h"
#include "ppu.h"  // needed for cpu/ppu read/write functions
#include "gui.h"  // needed for cpu/ppu read/write functions (due to ppu.h)


/* Get opcode from instruction and addressing mode
 * input is a pointer to an array of 3 chars (plus null char) e.g. LDA, BRK etc.
 */
static int reverse_opcode_lut(char (*instruction)[4], AddressMode address_mode)
{
	// Normalise input to uppercase chars
	for (int i = 0; i < 3; i++) {
		(*instruction)[i] = toupper((*instruction)[i]);
	}
	int opcode = -1;  // error for -1

	// Offical opcodes
	if (!strncmp(*instruction, "BRK", 4))        /* SPECIAL*/            { opcode = 0x00; }
	else if (!strncmp(*instruction, "ORA", 4) && (address_mode == INDX)) { opcode = 0x01; }
	else if (!strncmp(*instruction, "ORA", 4) && (address_mode == ZP)) { opcode = 0x05; }
	else if (!strncmp(*instruction, "ASL", 4) && (address_mode == ZP)) { opcode = 0x06; }
	else if (!strncmp(*instruction, "PHP", 4))   /* IMP */             { opcode = 0x08; }
	else if (!strncmp(*instruction, "ORA", 4) && (address_mode == IMM)) { opcode = 0x09; }
	else if (!strncmp(*instruction, "ASL", 4) && (address_mode == ACC)) { opcode = 0x0A; }
	else if (!strncmp(*instruction, "ORA", 4) && (address_mode == ABS)) { opcode = 0x0D; }
	else if (!strncmp(*instruction, "ASL", 4) && (address_mode == ABS)) { opcode = 0x0E; }
	// row 2
	else if (!strncmp(*instruction, "BPL", 4) && (address_mode == REL)) { opcode = 0x10; }
	else if (!strncmp(*instruction, "ORA", 4) && (address_mode == INDY)) { opcode = 0x11; }
	else if (!strncmp(*instruction, "ORA", 4) && (address_mode == ZPX)) { opcode = 0x15; }
	else if (!strncmp(*instruction, "ASL", 4) && (address_mode == ZPX)) { opcode = 0x16; }
	else if (!strncmp(*instruction, "CLC", 4))   /* IMP */               { opcode = 0x18; }
	else if (!strncmp(*instruction, "ORA", 4) && (address_mode == ABSY)) { opcode = 0x19; }
	else if (!strncmp(*instruction, "ORA", 4) && (address_mode == ABSX)) { opcode = 0x1D; }
	else if (!strncmp(*instruction, "ASL", 4) && (address_mode == ABSX)) { opcode = 0x1E; }
	// row 3
	else if (!strncmp(*instruction, "JSR", 4))   /* SPECIAL */           { opcode = 0x20; }
	else if (!strncmp(*instruction, "AND", 4) && (address_mode == INDX)) { opcode = 0x21; }
	else if (!strncmp(*instruction, "BIT", 4) && (address_mode == ZP)) { opcode = 0x24; }
	else if (!strncmp(*instruction, "AND", 4) && (address_mode == ZP)) { opcode = 0x25; }
	else if (!strncmp(*instruction, "ROL", 4) && (address_mode == ZP)) { opcode = 0x26; }
	else if (!strncmp(*instruction, "PLP", 4))   /* IMP */              { opcode = 0x28; }
	else if (!strncmp(*instruction, "AND", 4) && (address_mode == IMM)) { opcode = 0x29; }
	else if (!strncmp(*instruction, "ROL", 4) && (address_mode == ACC)) { opcode = 0x2A; }
	else if (!strncmp(*instruction, "BIT", 4) && (address_mode == ABS)) { opcode = 0x2C; }
	else if (!strncmp(*instruction, "AND", 4) && (address_mode == ABS)) { opcode = 0x2D; }
	else if (!strncmp(*instruction, "ROL", 4) && (address_mode == ABS)) { opcode = 0x2E; }
	// row 4
	else if (!strncmp(*instruction, "BMI", 4) && (address_mode == REL)) { opcode = 0x30; }
	else if (!strncmp(*instruction, "AND", 4) && (address_mode == INDY)) { opcode = 0x31; }
	else if (!strncmp(*instruction, "AND", 4) && (address_mode == ZPX)) { opcode = 0x35; }
	else if (!strncmp(*instruction, "ROL", 4) && (address_mode == ZPX)) { opcode = 0x36; }
	else if (!strncmp(*instruction, "SEC", 4))   /* IMP */               { opcode = 0x38; }
	else if (!strncmp(*instruction, "AND", 4) && (address_mode == ABSY)) { opcode = 0x39; }
	else if (!strncmp(*instruction, "AND", 4) && (address_mode == ABSX)) { opcode = 0x3D; }
	else if (!strncmp(*instruction, "ROL", 4) && (address_mode == ABSX)) { opcode = 0x3E; }
	// row 5
	else if (!strncmp(*instruction, "RTI", 4) && (address_mode == IMP)) { opcode = 0x40; }
	else if (!strncmp(*instruction, "EOR", 4) && (address_mode == INDX)) { opcode = 0x41; }
	else if (!strncmp(*instruction, "EOR", 4) && (address_mode == ZP)) { opcode = 0x45; }
	else if (!strncmp(*instruction, "LSR", 4) && (address_mode == ZP)) { opcode = 0x46; }
	else if (!strncmp(*instruction, "PHA", 4))   /* IMP */              { opcode = 0x48; }
	else if (!strncmp(*instruction, "EOR", 4) && (address_mode == IMM)) { opcode = 0x49; }
	else if (!strncmp(*instruction, "LSR", 4) && (address_mode == ACC)) { opcode = 0x4A; }
	else if (!strncmp(*instruction, "JMP", 4) && (address_mode == ABS)) { opcode = 0x4C; }
	else if (!strncmp(*instruction, "EOR", 4) && (address_mode == ABS)) { opcode = 0x4D; }
	else if (!strncmp(*instruction, "LSR", 4) && (address_mode == ABS)) { opcode = 0x4E; }
	// row 6
	else if (!strncmp(*instruction, "BVC", 4) && (address_mode == REL)) { opcode = 0x50; }
	else if (!strncmp(*instruction, "EOR", 4) && (address_mode == INDY)) { opcode = 0x51; }
	else if (!strncmp(*instruction, "EOR", 4) && (address_mode == ZPX)) { opcode = 0x55; }
	else if (!strncmp(*instruction, "LSR", 4) && (address_mode == ZPX)) { opcode = 0x56; }
	else if (!strncmp(*instruction, "CLI", 4))   /* IMP */               { opcode = 0x58; }
	else if (!strncmp(*instruction, "EOR", 4) && (address_mode == ABSY)) { opcode = 0x59; }
	else if (!strncmp(*instruction, "EOR", 4) && (address_mode == ABSX)) { opcode = 0x5D; }
	else if (!strncmp(*instruction, "LSR", 4) && (address_mode == ABSX)) { opcode = 0x5E; }
	// row 7
	else if (!strncmp(*instruction, "RTS", 4))   /* IMP */               { opcode = 0x60; }
	else if (!strncmp(*instruction, "ADC", 4) && (address_mode == INDX)) { opcode = 0x61; }
	else if (!strncmp(*instruction, "ADC", 4) && (address_mode == ZP)) { opcode = 0x65; }
	else if (!strncmp(*instruction, "ROR", 4) && (address_mode == ZP)) { opcode = 0x66; }
	else if (!strncmp(*instruction, "PLA", 4))   /* IMP */              { opcode = 0x68; }
	else if (!strncmp(*instruction, "ADC", 4) && (address_mode == IMM)) { opcode = 0x69; }
	else if (!strncmp(*instruction, "ROR", 4) && (address_mode == ACC)) { opcode = 0x6A; }
	else if (!strncmp(*instruction, "JMP", 4) && (address_mode == IND)) { opcode = 0x6C; }
	else if (!strncmp(*instruction, "ADC", 4) && (address_mode == ABS)) { opcode = 0x6D; }
	else if (!strncmp(*instruction, "ROR", 4) && (address_mode == ABS)) { opcode = 0x6E; }
	// row 8
	else if (!strncmp(*instruction, "BVS", 4) && (address_mode == REL)) { opcode = 0x70; }
	else if (!strncmp(*instruction, "ADC", 4) && (address_mode == INDY)) { opcode = 0x71; }
	else if (!strncmp(*instruction, "ADC", 4) && (address_mode == ZPX)) { opcode = 0x75; }
	else if (!strncmp(*instruction, "ROR", 4) && (address_mode == ZPX)) { opcode = 0x76; }
	else if (!strncmp(*instruction, "SEI", 4))   /* IMP */               { opcode = 0x78; }
	else if (!strncmp(*instruction, "ADC", 4) && (address_mode == ABSY)) { opcode = 0x79; }
	else if (!strncmp(*instruction, "ADC", 4) && (address_mode == ABSX)) { opcode = 0x7D; }
	else if (!strncmp(*instruction, "ROR", 4) && (address_mode == ABSX)) { opcode = 0x7E; }
	// row 9
	else if (!strncmp(*instruction, "STA", 4) && (address_mode == INDX)) { opcode = 0x81; }
	else if (!strncmp(*instruction, "STY", 4) && (address_mode == ZP)) { opcode = 0x84; }
	else if (!strncmp(*instruction, "STA", 4) && (address_mode == ZP)) { opcode = 0x85; }
	else if (!strncmp(*instruction, "STX", 4) && (address_mode == ZP)) { opcode = 0x86; }
	else if (!strncmp(*instruction, "DEY", 4))   /* IMP */              { opcode = 0x88; }
	else if (!strncmp(*instruction, "TXA", 4))   /* IMP */              { opcode = 0x8A; }
	else if (!strncmp(*instruction, "STY", 4) && (address_mode == ABS)) { opcode = 0x8C; }
	else if (!strncmp(*instruction, "STA", 4) && (address_mode == ABS)) { opcode = 0x8D; }
	else if (!strncmp(*instruction, "STX", 4) && (address_mode == ABS)) { opcode = 0x8E; }
	// row 10
	else if (!strncmp(*instruction, "BCC", 4) && (address_mode == REL)) { opcode = 0x90; }
	else if (!strncmp(*instruction, "STA", 4) && (address_mode == INDY)) { opcode = 0x91; }
	else if (!strncmp(*instruction, "STY", 4) && (address_mode == ZPX)) { opcode = 0x94; }
	else if (!strncmp(*instruction, "STA", 4) && (address_mode == ZPX)) { opcode = 0x95; }
	else if (!strncmp(*instruction, "STX", 4) && (address_mode == ZPY)) { opcode = 0x96; }
	else if (!strncmp(*instruction, "TYA", 4))   /* IMP */               { opcode = 0x98; }
	else if (!strncmp(*instruction, "STA", 4) && (address_mode == ABSY)) { opcode = 0x99; }
	else if (!strncmp(*instruction, "TXS", 4))   /* IMP */               { opcode = 0x9A; }
	else if (!strncmp(*instruction, "STA", 4) && (address_mode == ABSX)) { opcode = 0x9D; }
	// row 11
	else if (!strncmp(*instruction, "LDY", 4) && (address_mode == IMM)) { opcode = 0xA0; }
	else if (!strncmp(*instruction, "LDA", 4) && (address_mode == INDX)) { opcode = 0xA1; }
	else if (!strncmp(*instruction, "LDX", 4) && (address_mode == IMM)) { opcode = 0xA2; }
	else if (!strncmp(*instruction, "LDY", 4) && (address_mode == ZP)) { opcode = 0xA4; }
	else if (!strncmp(*instruction, "LDA", 4) && (address_mode == ZP)) { opcode = 0xA5; }
	else if (!strncmp(*instruction, "LDX", 4) && (address_mode == ZP)) { opcode = 0xA6; }
	else if (!strncmp(*instruction, "TAY", 4) && (address_mode == IMP)) { opcode = 0xA8; }
	else if (!strncmp(*instruction, "LDA", 4) && (address_mode == IMM)) { opcode = 0xA9; }
	else if (!strncmp(*instruction, "TAX", 4) && (address_mode == IMP)) { opcode = 0xAA; }
	else if (!strncmp(*instruction, "LDY", 4) && (address_mode == ABS)) { opcode = 0xAC; }
	else if (!strncmp(*instruction, "LDA", 4) && (address_mode == ABS)) { opcode = 0xAD; }
	else if (!strncmp(*instruction, "LDX", 4) && (address_mode == ABS)) { opcode = 0xAE; }
	// row 12
	else if (!strncmp(*instruction, "BCS", 4) && (address_mode == REL)) { opcode = 0xB0; }
	else if (!strncmp(*instruction, "LDA", 4) && (address_mode == INDY)) { opcode = 0xB1; }
	else if (!strncmp(*instruction, "LDY", 4) && (address_mode == ZPX)) { opcode = 0xB4; }
	else if (!strncmp(*instruction, "LDA", 4) && (address_mode == ZPX)) { opcode = 0xB5; }
	else if (!strncmp(*instruction, "LDX", 4) && (address_mode == ZPY)) { opcode = 0xB6; }
	else if (!strncmp(*instruction, "CLV", 4) && (address_mode == IMP)) { opcode = 0xB8; }
	else if (!strncmp(*instruction, "LDA", 4) && (address_mode == ABSY)) { opcode = 0xB9; }
	else if (!strncmp(*instruction, "TSX", 4))   /* IMP */               { opcode = 0xBA; }
	else if (!strncmp(*instruction, "LDY", 4) && (address_mode == ABSX)) { opcode = 0xBC; }
	else if (!strncmp(*instruction, "LDA", 4) && (address_mode == ABSX)) { opcode = 0xBD; }
	else if (!strncmp(*instruction, "LDX", 4) && (address_mode == ABSY)) { opcode = 0xBE; }
	// row 13
	else if (!strncmp(*instruction, "CPY", 4) && (address_mode == IMM)) { opcode = 0xC0; }
	else if (!strncmp(*instruction, "CMP", 4) && (address_mode == INDX)) { opcode = 0xC1; }
	else if (!strncmp(*instruction, "CPY", 4) && (address_mode == ZP)) { opcode = 0xC4; }
	else if (!strncmp(*instruction, "CMP", 4) && (address_mode == ZP)) { opcode = 0xC5; }
	else if (!strncmp(*instruction, "DEC", 4) && (address_mode == ZP)) { opcode = 0xC6; }
	else if (!strncmp(*instruction, "INY", 4))   /* IMP */              { opcode = 0xC8; }
	else if (!strncmp(*instruction, "CMP", 4) && (address_mode == IMM)) { opcode = 0xC9; }
	else if (!strncmp(*instruction, "DEX", 4))   /* IMP */              { opcode = 0xCA; }
	else if (!strncmp(*instruction, "CPY", 4) && (address_mode == ABS)) { opcode = 0xCC; }
	else if (!strncmp(*instruction, "CMP", 4) && (address_mode == ABS)) { opcode = 0xCD; }
	else if (!strncmp(*instruction, "DEC", 4) && (address_mode == ABS)) { opcode = 0xCE; }
	// row 14
	else if (!strncmp(*instruction, "BNE", 4) && (address_mode == REL)) { opcode = 0xD0; }
	else if (!strncmp(*instruction, "CMP", 4) && (address_mode == INDY)) { opcode = 0xD1; }
	else if (!strncmp(*instruction, "CMP", 4) && (address_mode == ZPX)) { opcode = 0xD5; }
	else if (!strncmp(*instruction, "DEC", 4) && (address_mode == ZPX)) { opcode = 0xD6; }
	else if (!strncmp(*instruction, "CLD", 4) && (address_mode == IMP)) { opcode = 0xD8; }
	else if (!strncmp(*instruction, "CMP", 4) && (address_mode == ABSY)) { opcode = 0xD9; }
	else if (!strncmp(*instruction, "CMP", 4) && (address_mode == ABSX)) { opcode = 0xDD; }
	else if (!strncmp(*instruction, "DEC", 4) && (address_mode == ABSX)) { opcode = 0xDE; }
	// row 15
	else if (!strncmp(*instruction, "CPX", 4) && (address_mode == IMM)) { opcode = 0xE0; }
	else if (!strncmp(*instruction, "SBC", 4) && (address_mode == INDX)) { opcode = 0xE1; }
	else if (!strncmp(*instruction, "CPX", 4) && (address_mode == ZP)) { opcode = 0xE4; }
	else if (!strncmp(*instruction, "SBC", 4) && (address_mode == ZP)) { opcode = 0xE5; }
	else if (!strncmp(*instruction, "INC", 4) && (address_mode == ZP)) { opcode = 0xE6; }
	else if (!strncmp(*instruction, "INX", 4))   /* IMP */              { opcode = 0xE8; }
	else if (!strncmp(*instruction, "SBC", 4) && (address_mode == IMM)) { opcode = 0xE9; }
	else if (!strncmp(*instruction, "NOP", 4) && (address_mode == IMP)) { opcode = 0xEA; }
	else if (!strncmp(*instruction, "CPX", 4) && (address_mode == ABS)) { opcode = 0xEC; }
	else if (!strncmp(*instruction, "SBC", 4) && (address_mode == ABS)) { opcode = 0xED; }
	else if (!strncmp(*instruction, "INC", 4) && (address_mode == ABS)) { opcode = 0xEE; }
	// row 16
	else if (!strncmp(*instruction, "BEQ", 4) && (address_mode == REL)) { opcode = 0xF0; }
	else if (!strncmp(*instruction, "SBC", 4) && (address_mode == INDY)) { opcode = 0xF1; }
	else if (!strncmp(*instruction, "SBC", 4) && (address_mode == ZPX)) { opcode = 0xF5; }
	else if (!strncmp(*instruction, "INC", 4) && (address_mode == ZPX)) { opcode = 0xF6; }
	else if (!strncmp(*instruction, "SED", 4))   /* IMP */               { opcode = 0xF8; }
	else if (!strncmp(*instruction, "SBC", 4) && (address_mode == ABSY)) { opcode = 0xF9; }
	else if (!strncmp(*instruction, "SBC", 4) && (address_mode == ABSX)) { opcode = 0xFD; }
	else if (!strncmp(*instruction, "INC", 4) && (address_mode == ABSX)) { opcode = 0xFE; }

	return opcode;
}

START_TEST (test_strcmp_reverse_opcode_lut)
{
	// take 1 from each row of the non-reversed LUT
	char (*ins)[4] = malloc(sizeof *ins);
	strncpy((char*) ins, "BRK", 4);
	ck_assert_int_eq(0x00, reverse_opcode_lut(ins, ZP));
	strncpy((char*) ins, "CLC", 4);
	ck_assert_int_eq(0x18, reverse_opcode_lut(ins, IMP));
	strncpy((char*) ins, "ROL", 4);
	ck_assert_int_eq(0x2A, reverse_opcode_lut(ins, ACC));
	strncpy((char*) ins, "AND", 4);
	ck_assert_int_eq(0x3D, reverse_opcode_lut(ins, ABSX));
	strncpy((char*) ins, "EOR", 4);
	ck_assert_int_eq(0x41, reverse_opcode_lut(ins, INDX));
	strncpy((char*) ins, "BVC", 4);
	ck_assert_int_eq(0x50, reverse_opcode_lut(ins, REL));
	strncpy((char*) ins, "JMP", 4);
	ck_assert_int_eq(0x6C, reverse_opcode_lut(ins, IND));
	strncpy((char*) ins, "ADC", 4);
	ck_assert_int_eq(0x79, reverse_opcode_lut(ins, ABSY));
	strncpy((char*) ins, "STY", 4);
	ck_assert_int_eq(0x8C, reverse_opcode_lut(ins, ABS));
	strncpy((char*) ins, "STA", 4);
	ck_assert_int_eq(0x91, reverse_opcode_lut(ins, INDY));
	strncpy((char*) ins, "LDX", 4);
	ck_assert_int_eq(0xA2, reverse_opcode_lut(ins, IMM));
	strncpy((char*) ins, "CLV", 4);
	ck_assert_int_eq(0xB8, reverse_opcode_lut(ins, IMP));
	strncpy((char*) ins, "CPY", 4);
	ck_assert_int_eq(0xCC, reverse_opcode_lut(ins, ABS));
	strncpy((char*) ins, "DEC", 4);
	ck_assert_int_eq(0xDE, reverse_opcode_lut(ins, ABSX));
	strncpy((char*) ins, "SBC", 4);
	ck_assert_int_eq(0xE1, reverse_opcode_lut(ins, INDX));
	strncpy((char*) ins, "INC", 4);
	ck_assert_int_eq(0xFE, reverse_opcode_lut(ins, ABSX));
}
END_TEST

static void set_opcode_from_address_mode_and_instruction(Cpu6502* cpu, char* input
                                                        , AddressMode address_mode)
{
	char (*ins)[4] = malloc(sizeof *ins);
	strncpy((char*) ins, input, 4);
	cpu->opcode = reverse_opcode_lut(ins, address_mode);
	free(ins);
}

static void run_logic_cycle_by_cycle(Cpu6502* cpu, void (*opcode_lut[256])(Cpu6502* cpu)
                                    , int cycles_remaining)
{
	cpu->instruction_cycles_remaining = cycles_remaining;
	for (int i = 0; i < cycles_remaining; i++) {
		opcode_lut[cpu->opcode](cpu);
		cpu->instruction_cycles_remaining -= 1;
	}
}

// globals for unit tests (as setup/teardown take void args)
Cpu6502* cpu;

void setup(void)
{
	cpu = cpu_init(0xFFFCU, NULL, NULL); // allocate memory

	if (!cpu) {
		// fail, lack of memory
		ck_abort_msg("Failed to allocate memory to cpu struct");
	}
}

void teardown(void)
{
	free(cpu);
}

START_TEST (cpu_test_addr_mode_imm)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", IMM);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xA1; // IMM byte

	cpu->instruction_cycles_remaining = 1; // 1 cycle for the IMM decoder
	decode_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xA1, cpu->operand);
}
END_TEST

START_TEST (cpu_test_addr_mode_abs_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", ABS);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xC0; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x00; // addr_hi (from cpu->PC + 1)

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0xC0, cpu->addr_lo);
	ck_assert_uint_eq(0x00, cpu->addr_hi);
	ck_assert_uint_eq(0x00C0, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_addr_mode_abs_rmw)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INC", ABS);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xC0; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x00; // addr_hi (from cpu->PC + 1)

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0xC0, cpu->addr_lo);
	ck_assert_uint_eq(0x00, cpu->addr_hi);
	ck_assert_uint_eq(0x00C0, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_addr_mode_abs_jmp)
{
	set_opcode_from_address_mode_and_instruction(cpu, "JMP", ABS);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x90; // addr_lo
	cpu->mem[cpu->PC + 1] = 0xB4; // addr_hi (from cpu->PC + 1)

	decode_opcode_lut[cpu->opcode](cpu); // setup needed for the for loop below
	run_logic_cycle_by_cycle(cpu, execute_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0x90, cpu->addr_lo);
	ck_assert_uint_eq(0xB4, cpu->addr_hi);
	ck_assert_uint_eq(0xB490, cpu->target_addr);
	ck_assert_uint_eq(0xB490, cpu->PC);
}
END_TEST

START_TEST (cpu_test_addr_mode_absx_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "EOR", ABSX);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xE1; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x07; // addr_hi (from cpu->PC + 1)
	cpu->X = 0x0F; // X offset to ABS address

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0xE1, cpu->addr_lo);
	ck_assert_uint_eq(0x07, cpu->addr_hi);
	ck_assert_uint_eq(0x07F0, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_addr_mode_absx_rmw)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ASL", ABSX);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x04; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x10; // addr_hi (from cpu->PC + 1)
	cpu->X = 0x06; // X offset to ABS address

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0x04, cpu->addr_lo);
	ck_assert_uint_eq(0x10, cpu->addr_hi);
	ck_assert_uint_eq(0x100A, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_addr_mode_absy_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDA", ABSY);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x25; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x04; // addr_hi (from cpu->PC + 1)
	cpu->Y = 0x05; // X offset to ABS address

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0x25, cpu->addr_lo);
	ck_assert_uint_eq(0x04, cpu->addr_hi);
	ck_assert_uint_eq(0x042A, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_addr_mode_zp_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ZP);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xDE; // addr_lo, addr_hi fixed to 0x00

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0xDE, cpu->addr_lo);
	ck_assert_uint_eq(0x00DE, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_addr_mode_zp_rmw)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LSR", ZP);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x02; // addr_lo, addr_hi fixed to 0x00

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0x02, cpu->addr_lo);
	ck_assert_uint_eq(0x0002, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_addr_mode_zpx_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STY", ZPX);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x04; // addr_lo, addr_hi fixed to 0x00
	cpu->X = 0xAC;

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0x04, cpu->addr_lo);
	ck_assert_uint_eq(0x00B0, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_addr_mode_zpx_rmw)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ZPX);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xFA; // addr_lo, addr_hi fixed to 0x00
	cpu->X = 0x03;

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0xFA, cpu->addr_lo);
	ck_assert_uint_eq(0x00FD, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_addr_mode_zpy_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDX", ZPY);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x63; // addr_lo, addr_hi fixed to 0x00
	cpu->Y = 0x03;

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0x63, cpu->addr_lo);
	ck_assert_uint_eq(0x0066, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_ind_jmp)
{
	set_opcode_from_address_mode_and_instruction(cpu, "JMP", IND);

	// set index_lo and index_hi for IND address mode
	// then also set the address it points from that indexed address
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x24; // index_lo
	cpu->mem[cpu->PC + 1] = 0x04; // index_hi (from cpu->PC + 1)
	cpu->mem[0x0424] = 0x11;  // addr_lo
	cpu->mem[0x0425] = 0x01;  // addr_hi

	// JMP instruction has no decode logic so loop execute function
	// and decrement instruction_cycles_remaining
	cpu->address_mode = IND;
	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, execute_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0x24, cpu->index_lo);
	ck_assert_uint_eq(0x04, cpu->index_hi);
	ck_assert_uint_eq(0x11, cpu->addr_lo);
	ck_assert_uint_eq(0x01, cpu->addr_hi);
	ck_assert_uint_eq(0x0111, cpu->PC);
}
END_TEST

START_TEST (cpu_test_ind_jmp_bug)
{
	set_opcode_from_address_mode_and_instruction(cpu, "JMP", IND);

	// set index_lo and index_hi for IND address mode
	// then also set the address it points from that indexed address
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xFF; // index_lo
	cpu->mem[cpu->PC + 1] = 0x01; // index_hi (from cpu->PC + 1)
	cpu->mem[0x01FF] = 0x20;  // addr_lo
	cpu->mem[0x0200] = 0x80;  // addr_hi (discarded)
	cpu->mem[0x0100] = 0x01;  // addr_hi (kept, JMP bug)

	// JMP instruction has no decode logic so loop execute function
	// and decrement instruction_cycles_remaining
	cpu->address_mode = IND;
	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, execute_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0xFF, cpu->index_lo);
	ck_assert_uint_eq(0x01, cpu->index_hi);
	ck_assert_uint_eq(0x20, cpu->addr_lo);
	ck_assert_uint_eq(0x01, cpu->addr_hi);
	ck_assert_uint_eq(0x0120, cpu->PC);
}
END_TEST

START_TEST (cpu_test_addr_mode_indx_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ORA", INDX);

	// set index_lo and index_hi for IND address mode
	// then also set the address it points from that indexed address
	cpu->X = 0x15;
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x24; // base_addr
	cpu->mem[0x0039] = 0xCD;  // addr_lo
	cpu->mem[0x003A] = 0x09;  // addr_hi

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0x24, cpu->base_addr);
	ck_assert_uint_eq(0xCD, cpu->addr_lo);
	ck_assert_uint_eq(0x09, cpu->addr_hi);
	ck_assert_uint_eq(0x09CD, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_addr_mode_indy_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CMP", INDY);

	// set index_lo and index_hi for IND address mode
	// then also set the address it points from that indexed address
	cpu->Y = 0x17;
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x71; // base_addr
	cpu->mem[0x0071] = 0xB1;  // addr_lo
	cpu->mem[0x0072] = 0x13;  // addr_hi

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut, max_cycles_opcode_lut[cpu->opcode] - 1);

	ck_assert_uint_eq(0x71, cpu->base_addr);
	ck_assert_uint_eq(0xB1, cpu->addr_lo);
	ck_assert_uint_eq(0x13, cpu->addr_hi);
	ck_assert_uint_eq(0x13C8, cpu->target_addr);
}
END_TEST

START_TEST (cpu_test_ram_read_non_mirrored)
{
	cpu->mem[0x0010] = 0xAA;
	cpu->mem[0x0124] = 0x83;
	cpu->mem[0x0505] = 0x52;
	cpu->mem[0x04FF] = 0x01;
	cpu->mem[0x07FF] = 0xB0;
	ck_assert_uint_eq(0xAA, read_from_cpu(cpu, 0x0010));
	ck_assert_uint_eq(0x83, read_from_cpu(cpu, 0x0124));
	ck_assert_uint_eq(0x52, read_from_cpu(cpu, 0x0505));
	ck_assert_uint_eq(0x01, read_from_cpu(cpu, 0x04FF));
	ck_assert_uint_eq(0xB0, read_from_cpu(cpu, 0x07FF));
}

START_TEST (cpu_test_ram_read_mirrored_bank_1)
{
	cpu->mem[0x0010] = 0xAA;
	cpu->mem[0x0124] = 0x83;
	cpu->mem[0x0505] = 0x52;
	cpu->mem[0x04FF] = 0x01;
	cpu->mem[0x07FF] = 0xB0;
	ck_assert_uint_eq(0xAA, read_from_cpu(cpu, 0x0010 + 0x0800));
	ck_assert_uint_eq(0x83, read_from_cpu(cpu, 0x0124 + 0x0800));
	ck_assert_uint_eq(0x52, read_from_cpu(cpu, 0x0505 + 0x0800));
	ck_assert_uint_eq(0x01, read_from_cpu(cpu, 0x04FF + 0x0800));
	ck_assert_uint_eq(0xB0, read_from_cpu(cpu, 0x07FF + 0x0800));
}

START_TEST (cpu_test_ram_read_mirrored_bank_2)
{
	cpu->mem[0x0010] = 0xAA;
	cpu->mem[0x0124] = 0x83;
	cpu->mem[0x0505] = 0x52;
	cpu->mem[0x04FF] = 0x01;
	cpu->mem[0x07FF] = 0xB0;
	ck_assert_uint_eq(0xAA, read_from_cpu(cpu, 0x0010 + 0x1000));
	ck_assert_uint_eq(0x83, read_from_cpu(cpu, 0x0124 + 0x1000));
	ck_assert_uint_eq(0x52, read_from_cpu(cpu, 0x0505 + 0x1000));
	ck_assert_uint_eq(0x01, read_from_cpu(cpu, 0x04FF + 0x1000));
	ck_assert_uint_eq(0xB0, read_from_cpu(cpu, 0x07FF + 0x1000));
}

START_TEST (cpu_test_ram_read_mirrored_bank_3)
{
	cpu->mem[0x0010] = 0xAA;
	cpu->mem[0x0124] = 0x83;
	cpu->mem[0x0505] = 0x52;
	cpu->mem[0x04FF] = 0x01;
	cpu->mem[0x07FF] = 0xB0;
	ck_assert_uint_eq(0xAA, read_from_cpu(cpu, 0x0010 + 0x1800));
	ck_assert_uint_eq(0x83, read_from_cpu(cpu, 0x0124 + 0x1800));
	ck_assert_uint_eq(0x52, read_from_cpu(cpu, 0x0505 + 0x1800));
	ck_assert_uint_eq(0x01, read_from_cpu(cpu, 0x04FF + 0x1800));
	ck_assert_uint_eq(0xB0, read_from_cpu(cpu, 0x07FF + 0x1800));
}

START_TEST (cpu_test_ram_write_non_mirrored_check_all_reads)
{
	write_to_cpu(cpu, 0x0248, 0x20);
	ck_assert_uint_eq(0x20, read_from_cpu(cpu, 0x0248));
	ck_assert_uint_eq(0x20, read_from_cpu(cpu, 0x0248 + 0x0800));
	ck_assert_uint_eq(0x20, read_from_cpu(cpu, 0x0248 + 0x1000));
	ck_assert_uint_eq(0x20, read_from_cpu(cpu, 0x0248 + 0x1800));
}

// Ensures writes to banks can also be read-back correctly from their mirrors
START_TEST (cpu_test_ram_write_mirrored_bank_1_check_all_reads)
{
	write_to_cpu(cpu, 0x0248 + 0x0800, 0x21);
	ck_assert_uint_eq(0x21, read_from_cpu(cpu, 0x0248));
	ck_assert_uint_eq(0x21, read_from_cpu(cpu, 0x0248 + 0x0800));
	ck_assert_uint_eq(0x21, read_from_cpu(cpu, 0x0248 + 0x1000));
	ck_assert_uint_eq(0x21, read_from_cpu(cpu, 0x0248 + 0x1800));
}

START_TEST (cpu_test_ram_write_mirrored_bank_2_check_all_reads)
{
	write_to_cpu(cpu, 0x0248 + 0x1000, 0x22);
	ck_assert_uint_eq(0x22, read_from_cpu(cpu, 0x0248));
	ck_assert_uint_eq(0x22, read_from_cpu(cpu, 0x0248 + 0x0800));
	ck_assert_uint_eq(0x22, read_from_cpu(cpu, 0x0248 + 0x1000));
	ck_assert_uint_eq(0x22, read_from_cpu(cpu, 0x0248 + 0x1800));
}

START_TEST (cpu_test_ram_write_mirrored_bank_3_check_all_reads)
{
	write_to_cpu(cpu, 0x0248 + 0x1800, 0x23);
	ck_assert_uint_eq(0x23, read_from_cpu(cpu, 0x0248));
	ck_assert_uint_eq(0x23, read_from_cpu(cpu, 0x0248 + 0x0800));
	ck_assert_uint_eq(0x23, read_from_cpu(cpu, 0x0248 + 0x1000));
	ck_assert_uint_eq(0x23, read_from_cpu(cpu, 0x0248 + 0x1800));
}

START_TEST (cpu_test_isa_lda_result_only_imm)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDA", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x44;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x44, cpu->A);
}

START_TEST (cpu_test_isa_lda_result_only_non_imm_mode)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDA", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x22);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x22, cpu->A);
}

START_TEST (cpu_test_isa_ldx_result_only_imm)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDX", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x44;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x44, cpu->X);
}

START_TEST (cpu_test_isa_ldx_result_only_non_imm_mode)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDX", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x22);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x22, cpu->X);
}

START_TEST (cpu_test_isa_ldy_result_only_imm)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDY", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x44;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x44, cpu->Y);
}

START_TEST (cpu_test_isa_ldy_result_only_non_imm_mode)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDY", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x22);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x22, cpu->Y);
}

START_TEST (cpu_test_isa_sta_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STA", ABSX);
	cpu->target_addr = 0x002C;
	cpu->A = 0x03;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->A, read_from_cpu(cpu, cpu->target_addr));
}

START_TEST (cpu_test_isa_stx_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STX", ZP);
	cpu->target_addr = 0x000F;
	cpu->X = 0x03;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->X, read_from_cpu(cpu, cpu->target_addr));
}

START_TEST (cpu_test_isa_sty_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STY", ZPX);
	cpu->target_addr = 0x000F;
	cpu->Y = 0x03;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->Y, read_from_cpu(cpu, cpu->target_addr));
}

START_TEST (cpu_test_isa_tax_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TAX", IMP);
	cpu->A = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->X, cpu->A);
}

START_TEST (cpu_test_isa_tay_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TAY", IMP);
	cpu->A = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->Y, cpu->A);
}

START_TEST (cpu_test_isa_tsx_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TSX", IMP);
	cpu->stack = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->X, cpu->stack);
}

START_TEST (cpu_test_isa_txa_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TXA", IMP);
	cpu->X = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->A, cpu->X);
}

START_TEST (cpu_test_isa_txs_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TXS", IMP);
	cpu->X = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->stack, cpu->X);
}

START_TEST (cpu_test_isa_tya_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TYA", IMP);
	cpu->Y = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->A, cpu->Y);
}


Suite* cpu_suite(void)
{
	Suite* s;
	TCase* tc_test_helpers;
	TCase* tc_address_modes;
	TCase* tc_cpu_reads;
	TCase* tc_cpu_writes;
	TCase* tc_cpu_isa;

	s = suite_create("Cpu Tests");
	tc_test_helpers = tcase_create("Test Helpers");
	tcase_add_test(tc_test_helpers, test_strcmp_reverse_opcode_lut);
	suite_add_tcase(s, tc_test_helpers);
	tc_address_modes = tcase_create("Address Modes Correct Address");
	tcase_add_checked_fixture(tc_address_modes, setup, teardown);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_imm);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_abs_read_store);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_abs_rmw);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_abs_jmp);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_absx_read_store);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_absx_rmw);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_absy_read_store);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_zp_read_store);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_zp_rmw);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_zpx_read_store);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_zpx_rmw);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_zpy_read_store);
	tcase_add_test(tc_address_modes, cpu_test_ind_jmp);
	tcase_add_test(tc_address_modes, cpu_test_ind_jmp_bug);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_indx_read_store);
	tcase_add_test(tc_address_modes, cpu_test_addr_mode_indy_read_store);
	suite_add_tcase(s, tc_address_modes);
	tc_cpu_reads = tcase_create("Cpu Memory Mapped Reads");
	tcase_add_checked_fixture(tc_cpu_reads, setup, teardown);
	tcase_add_test(tc_cpu_reads, cpu_test_ram_read_non_mirrored);
	tcase_add_test(tc_cpu_reads, cpu_test_ram_read_mirrored_bank_1);
	tcase_add_test(tc_cpu_reads, cpu_test_ram_read_mirrored_bank_2);
	tcase_add_test(tc_cpu_reads, cpu_test_ram_read_mirrored_bank_3);
	suite_add_tcase(s, tc_cpu_reads);
	tc_cpu_writes = tcase_create("Cpu Memory Mapped Writes");
	tcase_add_checked_fixture(tc_cpu_writes, setup, teardown);
	tcase_add_test(tc_cpu_writes, cpu_test_ram_write_non_mirrored_check_all_reads);
	tcase_add_test(tc_cpu_writes, cpu_test_ram_write_mirrored_bank_1_check_all_reads);
	tcase_add_test(tc_cpu_writes, cpu_test_ram_write_mirrored_bank_2_check_all_reads);
	tcase_add_test(tc_cpu_writes, cpu_test_ram_write_mirrored_bank_3_check_all_reads);
	suite_add_tcase(s, tc_cpu_writes);
	tc_cpu_isa = tcase_create("Cpu Instruction Set Architecture Core Results");
	tcase_add_checked_fixture(tc_cpu_isa, setup, teardown);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_lda_result_only_imm);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_lda_result_only_non_imm_mode);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_ldx_result_only_imm);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_ldx_result_only_non_imm_mode);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_ldy_result_only_imm);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_ldy_result_only_non_imm_mode);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_sta_result_only);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_stx_result_only);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_sty_result_only);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_tax_result_only);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_tay_result_only);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_tsx_result_only);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_txa_result_only);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_txs_result_only);
	tcase_add_test(tc_cpu_isa, cpu_test_isa_tya_result_only);
	suite_add_tcase(s, tc_cpu_isa);

	return s;
}
