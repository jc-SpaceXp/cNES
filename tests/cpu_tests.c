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
	else if (!strncmp(*instruction, "RTI", 4))   /* IMP */               { opcode = 0x40; }
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
                                    , int cycles_remaining, InstructionStates stop_condition)
{
	cpu->instruction_cycles_remaining = cycles_remaining;
	for (int i = 0; i < cycles_remaining; i++) {
		if (cpu->instruction_state != stop_condition) {
			opcode_lut[cpu->opcode](cpu);
			cpu->instruction_cycles_remaining -= 1;
		}
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

START_TEST (addr_mode_imm)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", IMM);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xA1; // IMM byte

	cpu->instruction_cycles_remaining = 1; // 1 cycle for the IMM decoder
	decode_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xA1, cpu->operand);
}
END_TEST

START_TEST (addr_mode_abs_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", ABS);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xC0; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x00; // addr_hi (from cpu->PC + 1)

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0xC0, cpu->addr_lo);
	ck_assert_uint_eq(0x00, cpu->addr_hi);
	ck_assert_uint_eq(0x00C0, cpu->target_addr);
}
END_TEST

START_TEST (addr_mode_abs_rmw)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INC", ABS);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xC0; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x00; // addr_hi (from cpu->PC + 1)

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0xC0, cpu->addr_lo);
	ck_assert_uint_eq(0x00, cpu->addr_hi);
	ck_assert_uint_eq(0x00C0, cpu->target_addr);
}
END_TEST

START_TEST (addr_mode_abs_jmp)
{
	set_opcode_from_address_mode_and_instruction(cpu, "JMP", ABS);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x90; // addr_lo
	cpu->mem[cpu->PC + 1] = 0xB4; // addr_hi (from cpu->PC + 1)

	decode_opcode_lut[cpu->opcode](cpu); // setup needed for the for loop below
	run_logic_cycle_by_cycle(cpu, execute_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, FETCH);

	ck_assert_uint_eq(0x90, cpu->addr_lo);
	ck_assert_uint_eq(0xB4, cpu->addr_hi);
	ck_assert_uint_eq(0xB490, cpu->target_addr);
	ck_assert_uint_eq(0xB490, cpu->PC);
}
END_TEST

START_TEST (addr_mode_absx_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "EOR", ABSX);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xE1; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x07; // addr_hi (from cpu->PC + 1)
	cpu->X = 0x0F; // X offset to ABS address

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0xE1, cpu->addr_lo);
	ck_assert_uint_eq(0x07, cpu->addr_hi);
	ck_assert_uint_eq(0x07F0, cpu->target_addr);
}
END_TEST

START_TEST (addr_mode_absx_rmw)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ASL", ABSX);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x04; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x10; // addr_hi (from cpu->PC + 1)
	cpu->X = 0x06; // X offset to ABS address

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0x04, cpu->addr_lo);
	ck_assert_uint_eq(0x10, cpu->addr_hi);
	ck_assert_uint_eq(0x100A, cpu->target_addr);
}
END_TEST

START_TEST (addr_mode_absy_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDA", ABSY);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x25; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x04; // addr_hi (from cpu->PC + 1)
	cpu->Y = 0x05; // X offset to ABS address

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0x25, cpu->addr_lo);
	ck_assert_uint_eq(0x04, cpu->addr_hi);
	ck_assert_uint_eq(0x042A, cpu->target_addr);
}
END_TEST

START_TEST (addr_mode_zp_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ZP);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xDE; // addr_lo, addr_hi fixed to 0x00

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0xDE, cpu->addr_lo);
	ck_assert_uint_eq(0x00DE, cpu->target_addr);
}
END_TEST

START_TEST (addr_mode_zp_rmw)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LSR", ZP);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x02; // addr_lo, addr_hi fixed to 0x00

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0x02, cpu->addr_lo);
	ck_assert_uint_eq(0x0002, cpu->target_addr);
}
END_TEST

START_TEST (addr_mode_zpx_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STY", ZPX);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x04; // addr_lo, addr_hi fixed to 0x00
	cpu->X = 0xAC;

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0x04, cpu->addr_lo);
	ck_assert_uint_eq(0x00B0, cpu->target_addr);
}
END_TEST

START_TEST (addr_mode_zpx_rmw)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ZPX);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xFA; // addr_lo, addr_hi fixed to 0x00
	cpu->X = 0x03;

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0xFA, cpu->addr_lo);
	ck_assert_uint_eq(0x00FD, cpu->target_addr);
}
END_TEST

START_TEST (addr_mode_zpy_read_store)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDX", ZPY);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x63; // addr_lo, addr_hi fixed to 0x00
	cpu->Y = 0x03;

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0x63, cpu->addr_lo);
	ck_assert_uint_eq(0x0066, cpu->target_addr);
}
END_TEST

START_TEST (ind_jmp)
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
	decode_opcode_lut[cpu->opcode](cpu); // setup needed for the for loop below
	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, execute_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, FETCH);

	ck_assert_uint_eq(0x24, cpu->index_lo);
	ck_assert_uint_eq(0x04, cpu->index_hi);
	ck_assert_uint_eq(0x11, cpu->addr_lo);
	ck_assert_uint_eq(0x01, cpu->addr_hi);
	ck_assert_uint_eq(0x0111, cpu->PC);
}
END_TEST

START_TEST (ind_jmp_bug)
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
	decode_opcode_lut[cpu->opcode](cpu); // setup needed for the for loop below
	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, execute_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, FETCH);

	ck_assert_uint_eq(0xFF, cpu->index_lo);
	ck_assert_uint_eq(0x01, cpu->index_hi);
	ck_assert_uint_eq(0x20, cpu->addr_lo);
	ck_assert_uint_eq(0x01, cpu->addr_hi);
	ck_assert_uint_eq(0x0120, cpu->PC);
}
END_TEST

START_TEST (addr_mode_indx_read_store)
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
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0x24, cpu->base_addr);
	ck_assert_uint_eq(0xCD, cpu->addr_lo);
	ck_assert_uint_eq(0x09, cpu->addr_hi);
	ck_assert_uint_eq(0x09CD, cpu->target_addr);
}
END_TEST

START_TEST (addr_mode_indy_read_store)
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
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0x71, cpu->base_addr);
	ck_assert_uint_eq(0xB1, cpu->addr_lo);
	ck_assert_uint_eq(0x13, cpu->addr_hi);
	ck_assert_uint_eq(0x13C8, cpu->target_addr);
}
END_TEST

START_TEST (bcc_not_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BCC", REL);
	cpu->P |= FLAG_C;

	// set offset to be non-zero to detect any errors if the branch is taken
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 cycle, cycles T2 and onwards should be skipped
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(old_PC + 1, cpu->target_addr); // T0 increments PC and so does T1
}
END_TEST

START_TEST (bcs_not_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BCS", REL);
	cpu->P &= ~FLAG_C;

	// set offset to be non-zero to detect any errors if the branch is taken
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 cycle, cycles T2 and onwards should be skipped
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(old_PC + 1, cpu->target_addr); // T0 increments PC and so does T1
}
END_TEST

START_TEST (beq_not_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BEQ", REL);
	cpu->P &= ~FLAG_Z;

	// set offset to be non-zero to detect any errors if the branch is taken
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 cycle, cycles T2 and onwards should be skipped
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(old_PC + 1, cpu->target_addr); // T0 increments PC and so does T1
}
END_TEST

START_TEST (bmi_not_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BMI", REL);
	cpu->P &= ~FLAG_N;

	// set offset to be non-zero to detect any errors if the branch is taken
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 cycle, cycles T2 and onwards should be skipped
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(old_PC + 1, cpu->target_addr); // T0 increments PC and so does T1
}
END_TEST

START_TEST (bne_not_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BNE", REL);
	cpu->P |= FLAG_Z;

	// set offset to be non-zero to detect any errors if the branch is taken
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 cycle, cycles T2 and onwards should be skipped
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(old_PC + 1, cpu->target_addr); // T0 increments PC and so does T1
}
END_TEST

START_TEST (bpl_not_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BPL", REL);
	cpu->P |= FLAG_N;

	// set offset to be non-zero to detect any errors if the branch is taken
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 cycle, cycles T2 and onwards should be skipped
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(old_PC + 1, cpu->target_addr); // T0 increments PC and so does T1
}
END_TEST

START_TEST (bvc_not_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BVC", REL);
	cpu->P |= FLAG_V;

	// set offset to be non-zero to detect any errors if the branch is taken
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 cycle, cycles T2 and onwards should be skipped
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(old_PC + 1, cpu->target_addr); // T0 increments PC and so does T1
}
END_TEST

START_TEST (bvs_not_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BVS", REL);
	cpu->P &= ~FLAG_V;

	// set offset to be non-zero to detect any errors if the branch is taken
	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 cycle, cycles T2 and onwards should be skipped
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(old_PC + 1, cpu->target_addr); // T0 increments PC and so does T1
}
END_TEST

START_TEST (bcc_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BCC", REL);
	cpu->P &= ~FLAG_C;

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 and T2 cycles, T3 cycle should be skipped (page cross)
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bcs_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BCS", REL);
	cpu->P |= FLAG_C;

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 and T2 cycles, T3 cycle should be skipped (page cross)
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (beq_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BEQ", REL);
	cpu->P |= FLAG_Z;

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 and T2 cycles, T3 cycle should be skipped (page cross)
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bmi_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BMI", REL);
	cpu->P |= FLAG_N;

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 and T2 cycles, T3 cycle should be skipped (page cross)
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bne_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BNE", REL);
	cpu->P &= ~FLAG_Z;

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 and T2 cycles, T3 cycle should be skipped (page cross)
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bpl_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BPL", REL);
	cpu->P &= ~FLAG_N;

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 and T2 cycles, T3 cycle should be skipped (page cross)
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bvc_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BVC", REL);
	cpu->P &= ~FLAG_V;

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 and T2 cycles, T3 cycle should be skipped (page cross)
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bvs_taken_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BVS", REL);
	cpu->P |= FLAG_V;

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1 and T2 cycles, T3 cycle should be skipped (page cross)
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bcc_take_page_cross_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BCC", REL);
	cpu->P &= ~FLAG_C;

	cpu->PC = 0x80FF;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1-T3 cycles
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bcs_take_page_cross_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BCS", REL);
	cpu->P |= FLAG_C;

	cpu->PC = 0x80FF;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1-T3 cycles
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (beq_take_page_cross_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BEQ", REL);
	cpu->P |= FLAG_Z;

	cpu->PC = 0x80FF;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1-T3 cycles
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bmi_take_page_cross_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BMI", REL);
	cpu->P |= FLAG_N;

	cpu->PC = 0x80FF;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1-T3 cycles
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bne_take_page_cross_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BNE", REL);
	cpu->P &= ~FLAG_Z;

	cpu->PC = 0x80FF;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1-T3 cycles
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bpl_take_page_cross_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BPL", REL);
	cpu->P &= ~FLAG_N;

	cpu->PC = 0x80FF;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1-T3 cycles
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bvc_take_page_cross_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BVC", REL);
	cpu->P &= ~FLAG_V;

	cpu->PC = 0x80FF;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1-T3 cycles
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST

START_TEST (bvs_take_page_cross_correct_addr)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BVS", REL);
	cpu->P |= FLAG_V;

	cpu->PC = 0x80FF;
	cpu->mem[cpu->PC] = 0x50;
	uint16_t old_PC = cpu->PC;

	// Simulate T1-T3 cycles
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	// T0 increments PC and so does T1
	ck_assert_uint_eq(old_PC + 1 + (int8_t) 0x50, cpu->target_addr);
}
END_TEST


START_TEST (ram_read_non_mirrored)
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

START_TEST (ram_read_mirrored_bank_1)
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

START_TEST (ram_read_mirrored_bank_2)
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

START_TEST (ram_read_mirrored_bank_3)
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

START_TEST (ram_write_non_mirrored_check_all_reads)
{
	write_to_cpu(cpu, 0x0248, 0x20);
	ck_assert_uint_eq(0x20, read_from_cpu(cpu, 0x0248));
	ck_assert_uint_eq(0x20, read_from_cpu(cpu, 0x0248 + 0x0800));
	ck_assert_uint_eq(0x20, read_from_cpu(cpu, 0x0248 + 0x1000));
	ck_assert_uint_eq(0x20, read_from_cpu(cpu, 0x0248 + 0x1800));
}

// Ensures writes to banks can also be read-back correctly from their mirrors
START_TEST (ram_write_mirrored_bank_1_check_all_reads)
{
	write_to_cpu(cpu, 0x0248 + 0x0800, 0x21);
	ck_assert_uint_eq(0x21, read_from_cpu(cpu, 0x0248));
	ck_assert_uint_eq(0x21, read_from_cpu(cpu, 0x0248 + 0x0800));
	ck_assert_uint_eq(0x21, read_from_cpu(cpu, 0x0248 + 0x1000));
	ck_assert_uint_eq(0x21, read_from_cpu(cpu, 0x0248 + 0x1800));
}

START_TEST (ram_write_mirrored_bank_2_check_all_reads)
{
	write_to_cpu(cpu, 0x0248 + 0x1000, 0x22);
	ck_assert_uint_eq(0x22, read_from_cpu(cpu, 0x0248));
	ck_assert_uint_eq(0x22, read_from_cpu(cpu, 0x0248 + 0x0800));
	ck_assert_uint_eq(0x22, read_from_cpu(cpu, 0x0248 + 0x1000));
	ck_assert_uint_eq(0x22, read_from_cpu(cpu, 0x0248 + 0x1800));
}

START_TEST (ram_write_mirrored_bank_3_check_all_reads)
{
	write_to_cpu(cpu, 0x0248 + 0x1800, 0x23);
	ck_assert_uint_eq(0x23, read_from_cpu(cpu, 0x0248));
	ck_assert_uint_eq(0x23, read_from_cpu(cpu, 0x0248 + 0x0800));
	ck_assert_uint_eq(0x23, read_from_cpu(cpu, 0x0248 + 0x1000));
	ck_assert_uint_eq(0x23, read_from_cpu(cpu, 0x0248 + 0x1800));
}

START_TEST (stack_push_no_overflow)
{
	cpu->stack = SP_OFFSET; // End of stack is SP_OFFSET (0xFF)
	stack_push(cpu, 0x01);
	ck_assert_uint_eq(0xFF - 0x01, cpu->stack);
	ck_assert_uint_eq(0x01, read_from_cpu(cpu, SP_START + cpu->stack + 1));

	stack_push(cpu, 0x02);
	ck_assert_uint_eq(0xFF - 0x02, cpu->stack);
	ck_assert_uint_eq(0x02, read_from_cpu(cpu, SP_START + cpu->stack + 1));

	stack_push(cpu, 0xFF);
	ck_assert_uint_eq(0xFF - 0x03, cpu->stack);
	ck_assert_uint_eq(0xFF, read_from_cpu(cpu, SP_START + cpu->stack + 1));
	// make sure we don't overwrite previous writes too
	ck_assert_uint_eq(0x02, read_from_cpu(cpu, SP_START + cpu->stack + 2));
	ck_assert_uint_eq(0x01, read_from_cpu(cpu, SP_START + cpu->stack + 3));
	// make sure we don't have a off by one error too
	ck_assert_uint_ne(0xFF, read_from_cpu(cpu, SP_START + cpu->stack));
}

START_TEST (stack_push_overflow)
{
	cpu->stack = 0;
	stack_push(cpu, 0x0F);
	ck_assert_uint_eq(0xFF, cpu->stack); // stack pointer should wrap back around to 0xFF
	ck_assert_uint_eq(0x0F, read_from_cpu(cpu, SP_START));

	// make sure we don't have a off by one error too
	ck_assert_uint_ne(0x0F, read_from_cpu(cpu, SP_START + 1));
	ck_assert_uint_ne(0x0F, read_from_cpu(cpu, SP_START + SP_OFFSET));
}

START_TEST (stack_pull_no_underflow)
{
	// note stack pulls will automatically increment the stack pointer
	cpu->stack = 0x8C; // End of stack is SP_OFFSET (0xFF)
	stack_push(cpu, 0x01);
	stack_push(cpu, 0x02);
	stack_push(cpu, 0xFF);

	// Inversion of stack pushes
	ck_assert_uint_eq(0xFF, stack_pull(cpu));
	ck_assert_uint_eq(0x8C - 0x02, cpu->stack);
	ck_assert_uint_eq(0x02, stack_pull(cpu));
	ck_assert_uint_eq(0x8C - 0x01, cpu->stack);
	ck_assert_uint_eq(0x01, stack_pull(cpu));
	ck_assert_uint_eq(0x8C - 0x00, cpu->stack);
}

START_TEST (stack_pull_underflow)
{
	// note stack pulls will automatically increment the stack pointer
	cpu->stack = 0x02;
	stack_push(cpu, 0xA1);
	stack_push(cpu, 0xA2);
	stack_push(cpu, 0xA3);
	cpu->stack = 0xFF;
	stack_push(cpu, 0x01);
	stack_push(cpu, 0x02);


	// Inversion of stack pushes
	ck_assert_uint_eq(0x02, stack_pull(cpu));
	ck_assert_uint_eq(0xFE, cpu->stack);
	ck_assert_uint_eq(0x01, stack_pull(cpu));
	ck_assert_uint_eq(0xFF, cpu->stack);
	// Stack pointer should wrap back around to 0x00
	ck_assert_uint_eq(0xA3, stack_pull(cpu));
	ck_assert_uint_eq(0x00, cpu->stack);
	ck_assert_uint_eq(0xA2, stack_pull(cpu));
	ck_assert_uint_eq(0x01, cpu->stack);
}

START_TEST (isa_lda_result_only_imm)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDA", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x44;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x44, cpu->A);
}

START_TEST (isa_lda_result_only_non_imm_mode)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDA", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x22);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x22, cpu->A);
}

START_TEST (isa_ldx_result_only_imm)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDX", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x44;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x44, cpu->X);
}

START_TEST (isa_ldx_result_only_non_imm_mode)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDX", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x22);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x22, cpu->X);
}

START_TEST (isa_ldy_result_only_imm)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDY", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x44;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x44, cpu->Y);
}

START_TEST (isa_ldy_result_only_non_imm_mode)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDY", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x22);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x22, cpu->Y);
}

START_TEST (isa_sta_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STA", ABSX);
	cpu->target_addr = 0x002C;
	cpu->A = 0x03;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->A, read_from_cpu(cpu, cpu->target_addr));
}

START_TEST (isa_stx_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STX", ZP);
	cpu->target_addr = 0x000F;
	cpu->X = 0x03;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->X, read_from_cpu(cpu, cpu->target_addr));
}

START_TEST (isa_sty_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STY", ZPX);
	cpu->target_addr = 0x000F;
	cpu->Y = 0x03;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->Y, read_from_cpu(cpu, cpu->target_addr));
}

START_TEST (isa_tax_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TAX", IMP);
	cpu->A = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->X, cpu->A);
}

START_TEST (isa_tay_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TAY", IMP);
	cpu->A = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->Y, cpu->A);
}

START_TEST (isa_tsx_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TSX", IMP);
	cpu->stack = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->X, cpu->stack);
}

START_TEST (isa_txa_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TXA", IMP);
	cpu->X = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->A, cpu->X);
}

START_TEST (isa_txs_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TXS", IMP);
	cpu->X = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->stack, cpu->X);
}

START_TEST (isa_tya_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TYA", IMP);
	cpu->Y = 0x91;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(cpu->A, cpu->Y);
}

START_TEST (isa_adc_result_only_imm)
{
	// Result of ADC: is A + M + C (where M is a value from memory)
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x40;
	cpu->A = 0x40;
	cpu->P |= FLAG_C;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x81, cpu->A);
}

START_TEST (isa_adc_result_only_non_imm_mode)
{
	// Result of ADC: is A + M + C (where M is a value from memory)
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", INDY);
	cpu->address_mode = INDY;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x40);
	cpu->A = 0x40;
	cpu->P |= FLAG_C;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x81, cpu->A);
}

START_TEST (isa_adc_result_only_imm_overflow)
{
	// Result of ADC: is A + M + C (where M is a value from memory)
	// When result is greater than 0xFF it will be constrained to 8-bits
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0xFF;
	cpu->A = 0xF0;
	cpu->P |= FLAG_C;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xF0, cpu->A);
}

START_TEST (isa_dec_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEC", ABSX);
	cpu->target_addr = 0x0C43;
	write_to_cpu(cpu, cpu->target_addr, 0xA9);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xA8, read_from_cpu(cpu, cpu->target_addr));
}

START_TEST (isa_dex_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEX", IMP);
	cpu->X = 0xFF;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFE, cpu->X);
}

START_TEST (isa_dey_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEY", IMP);
	cpu->Y = 0xFF;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFE, cpu->Y);
}

START_TEST (isa_inc_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INC", ABS);
	cpu->target_addr = 0x0C43;
	write_to_cpu(cpu, cpu->target_addr, 0xA9);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xAA, read_from_cpu(cpu, cpu->target_addr));
}

START_TEST (isa_inx_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INX", IMP);
	cpu->X = 0xFE;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFF, cpu->X);
}

START_TEST (isa_iny_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INY", IMP);
	cpu->Y = 0xFE;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFF, cpu->Y);
}

START_TEST (isa_sbc_result_only_imm)
{
	// Result of ADC: is A - M - !C (where M is a value from memory)
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x40;
	cpu->A = 0x40;
	cpu->P &= ~FLAG_C;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFF, cpu->A);
}

START_TEST (isa_sbc_result_only_non_imm_mode)
{
	// Result of ADC: is A - M - !C (where M is a value from memory)
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", INDY);
	cpu->address_mode = INDY;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x40);
	cpu->A = 0x40;
	cpu->P &= ~FLAG_C;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFF, cpu->A);
}

START_TEST (isa_sbc_result_only_imm_underflow)
{
	// Result of ADC: is A - M - !C (where M is a value from memory)
	// When result is less than than 0x00 it will be constrained to 8-bits
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0xF1;
	cpu->A = 0xF0;
	cpu->P &= ~FLAG_C;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFE, cpu->A);
}

START_TEST (isa_and_result_only_imm)
{
	set_opcode_from_address_mode_and_instruction(cpu, "AND", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x1F;
	cpu->A = 0x10;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x10, cpu->A);
}

START_TEST (isa_and_result_only_non_imm_mode)
{
	set_opcode_from_address_mode_and_instruction(cpu, "AND", ZP);
	cpu->address_mode = ZP;
	cpu->target_addr = 0x00F2;
	write_to_cpu(cpu, cpu->target_addr, 0x1F);
	cpu->A = 0x10;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x10, cpu->A);
}

START_TEST (isa_asl_result_only_acc)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ASL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x7F;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFE, cpu->A);
}

START_TEST (isa_asl_result_only_non_acc_mode)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ASL", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00F2;
	write_to_cpu(cpu, cpu->target_addr, 0x7F);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFE, read_from_cpu(cpu, cpu->target_addr));
}

// Skip BIT as it only updates the flags which will be a separate test case

START_TEST (isa_eor_result_only_imm)
{
	set_opcode_from_address_mode_and_instruction(cpu, "EOR", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x1F;
	cpu->A = 0x10;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x0F, cpu->A);
}

START_TEST (isa_eor_result_only_non_imm_mode)
{
	set_opcode_from_address_mode_and_instruction(cpu, "EOR", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x0C12;
	write_to_cpu(cpu, cpu->target_addr, 0x1F);
	cpu->A = 0x10;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x0F, cpu->A);
}

START_TEST (isa_lsr_result_only_acc)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LSR", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x1E;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x0F, cpu->A);
}

START_TEST (isa_lsr_result_only_non_acc_mode)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LSR", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	write_to_cpu(cpu, cpu->target_addr, 0x1E);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x0F, read_from_cpu(cpu, cpu->target_addr));
}

START_TEST (isa_ora_result_only_imm)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ORA", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x1F;
	cpu->A = 0x10;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x1F, cpu->A);
}

START_TEST (isa_ora_result_only_non_imm_mode)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ORA", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x0C12;
	write_to_cpu(cpu, cpu->target_addr, 0x1F);
	cpu->A = 0x10;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x1F, cpu->A);
}

START_TEST (isa_rol_result_only_acc_carry_not_set)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x7F;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFE, cpu->A);
}

START_TEST (isa_rol_result_only_non_acc_mode_carry_not_set)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	write_to_cpu(cpu, cpu->target_addr, 0x7F);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFE, read_from_cpu(cpu, cpu->target_addr));
}

START_TEST (isa_rol_result_only_acc_carry_set)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x7F;
	cpu->P |= FLAG_C; // Carry moves into vacated LSB spot
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFF, cpu->A);
}

START_TEST (isa_ror_result_only_acc_carry_not_set)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROR", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0xFE;
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x7F, cpu->A);
}

START_TEST (isa_ror_result_only_non_acc_mode_carry_not_set)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROR", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	write_to_cpu(cpu, cpu->target_addr, 0xFE);
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0x7F, read_from_cpu(cpu, cpu->target_addr));
}

START_TEST (isa_ror_result_only_acc_carry_set)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROR", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0xFE;
	cpu->P |= FLAG_C; // Carry moves into vacated MSB spot
	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert_uint_eq(0xFF, cpu->A);
}

START_TEST (isa_jsr_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "JSR", IMP);
	cpu->PC = 0x9000;
	cpu->mem[cpu->PC] = 0x00; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x80; // addr_hi


	decode_opcode_lut[cpu->opcode](cpu); // setup needed for the for loop below
	run_logic_cycle_by_cycle(cpu, execute_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, FETCH);

	ck_assert_uint_eq(0x00, cpu->addr_lo);
	ck_assert_uint_eq(0x80, cpu->addr_hi);
	ck_assert_uint_eq(0x8000, cpu->target_addr);
	ck_assert_uint_eq(0x8000, cpu->PC);
}

START_TEST (isa_rti_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "RTI", IMP);
	// inverse of pull operations
	stack_push(cpu, 0x80); // push addr_hi onto stack
	stack_push(cpu, 0x01); // push addr_lo onto stack
	stack_push(cpu, 0x40); // push status reg onto stack

	decode_opcode_lut[cpu->opcode](cpu); // setup needed for the for loop below
	run_logic_cycle_by_cycle(cpu, execute_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, FETCH);

	ck_assert_uint_eq(0x01, cpu->addr_lo);
	ck_assert_uint_eq(0x80, cpu->addr_hi);
	ck_assert_uint_eq(0x8001, cpu->PC);
}

START_TEST (isa_rts_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "RTS", IMP);
	// inverse of pull operations
	stack_push(cpu, 0x80); // push addr_hi onto stack
	stack_push(cpu, 0x02); // push addr_lo onto stack

	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);
	execute_opcode_lut[cpu->opcode](cpu); // set PC from logic above

	ck_assert_uint_eq(0x02, cpu->addr_lo);
	ck_assert_uint_eq(0x80, cpu->addr_hi);
	ck_assert_uint_eq(0x8002, cpu->target_addr);
	ck_assert_uint_eq(0x8003, cpu->PC); // PC == PCH, PCL + 1 for RTS
}

START_TEST (isa_clc_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CLC", IMP);
	cpu->P = 0xBF;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}

START_TEST (isa_cld_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CLD", IMP);
	cpu->P = 0xBF;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_D) != FLAG_D);
}

START_TEST (isa_cli_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CLI", IMP);
	cpu->P = 0xBF;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_I) != FLAG_I);
}

START_TEST (isa_clv_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CLV", IMP);
	cpu->P = 0xBF;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_V) != FLAG_V);
}

START_TEST (isa_sec_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "SEC", IMP);
	cpu->P = 0xBF & ~FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_sed_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "SED", IMP);
	cpu->P = 0xBF & ~FLAG_D;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_D) == FLAG_D);
}

START_TEST (isa_sei_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "SEI", IMP);
	cpu->P = 0xBF & ~FLAG_I;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_pha_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "PHA", IMP);
	cpu->A = 0x54;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert_uint_eq(cpu->A, stack_pull(cpu));
}

START_TEST (isa_php_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "PHP", IMP);
	cpu->P = FLAG_N | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	// PHP will push bits 5 and 4 of the status reg onto the stack
	// as they don't exist on the status reg
	ck_assert_uint_eq(cpu->P | 0x30, stack_pull(cpu));
}

START_TEST (isa_pla_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "PLA", IMP);
	cpu->A = 0x54;
	stack_push(cpu, 0xA0);
	stack_push(cpu, 0xA4);

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert_uint_eq(0xA4, cpu->A);
}

START_TEST (isa_plp_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "PLP", IMP);
	stack_push(cpu, 0xA0);
	stack_push(cpu, FLAG_N | FLAG_C);

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	// Read last entry into stack and store into status reg
	// The unused bit (bit 5) is always set to 1 due to its open state behaviour
	ck_assert_uint_eq(FLAG_N | FLAG_C | 0x20, cpu->P);
}

START_TEST (isa_brk_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BRK", IMP);
	cpu->mem[BRK_VECTOR] = 0x0A; // addr_lo
	cpu->mem[BRK_VECTOR + 1] = 0x90; // addr_hi

	cpu->instruction_state = EXECUTE;
	run_logic_cycle_by_cycle(cpu, execute_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, FETCH);

	ck_assert_uint_eq(0x0A, cpu->addr_lo);
	ck_assert_uint_eq(0x90, cpu->addr_hi);
	ck_assert_uint_eq(0x900A, cpu->PC);
}
END_TEST

START_TEST (isa_nop_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "NOP", IMP);

	// Not really much to check for a NOP
	// So looking if any of the important internal registers change (excluding status reg)
	// PC should be unchanged, PC would be incremented by fetch stage
	uint16_t old_PC = cpu->PC;
	uint16_t old_A = cpu->A;
	uint16_t old_X = cpu->X;
	uint16_t old_Y = cpu->Y;

	cpu->instruction_state = EXECUTE;
	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert_uint_eq(old_PC, cpu->PC);
	ck_assert_uint_eq(old_A, cpu->A);
	ck_assert_uint_eq(old_X, cpu->X);
	ck_assert_uint_eq(old_Y, cpu->Y);
}
END_TEST

START_TEST (isa_flag_update_adc_imm_overflow)
{
	// Result of ADC: is A + M + C (where M is a value from memory)
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", IMM);
	cpu->address_mode = IMM;

	// ADC uses signed addition, so the inputs are limited to -128 to 127
	// Overflow occurs only when both operands have the same sign
	// For overflow the result will have a different sign to the inputs
	cpu->operand = 3;
	cpu->A = 127;
	cpu->P &= ~FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	// Extra flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}
END_TEST

START_TEST (isa_flag_update_sbc_imm_overflow)
{
	// Result of SBC: is A - M + !C (where M is a value from memory)
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", IMM);
	cpu->address_mode = IMM;

	// SBC uses signed addition, so the inputs are limited to -128 to 127
	// Overflow occurs only when both operands have the same sign
	// For overflow the result will have a different sign to the inputs
	cpu->operand = -24;
	cpu->A = 127;
	cpu->P &= ~FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	// Extra flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}
END_TEST

START_TEST (isa_flag_update_bit_set_all_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ABS);

	cpu->P = 0;
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 0xFF);
	cpu->A = 0x00;

	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
}
END_TEST

START_TEST (isa_flag_update_bit_set_overflow)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ABS);

	cpu->P = 0;
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 0xFF & ~FLAG_N);
	cpu->A = 0xFF & ~FLAG_N;

	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
}
END_TEST

START_TEST (isa_flag_update_bit_set_negative)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ABS);

	cpu->P = 0;
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 0xFF & ~FLAG_V);
	cpu->A = 0xFF & ~FLAG_V;

	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
}
END_TEST

START_TEST (isa_flag_update_bit_set_zero)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ABS);

	cpu->P = 0;
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 0xFF & ~FLAG_N & ~FLAG_V);
	cpu->A = FLAG_N; // make sure no matching bits

	execute_opcode_lut[cpu->opcode](cpu);
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
}
END_TEST

START_TEST (isa_flag_update_cmp_set_negative)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CMP", IMM);
	cpu->address_mode = IMM;

	cpu->P = 0;
	// Similar to SBC, signed subtraction from -128 to 127 w/o carry
	cpu->operand = 50;
	cpu->A = 10;

	execute_opcode_lut[cpu->opcode](cpu);
	// note comparisons which set the flags are unsigned comparisons
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C); // C is set if register >= memory
}
END_TEST

// Carry is also set when Z flag is set
START_TEST (isa_flag_update_cmp_set_zero_and_carry)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CMP", ZP);
	cpu->address_mode = ZP;

	cpu->P = 0;
	// Similar to SBC, signed subtraction from -128 to 127 w/o carry
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, -113);
	cpu->A = -113;

	execute_opcode_lut[cpu->opcode](cpu);
	// note comparisons which set the flags are unsigned comparisons
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C); // C is set if register >= memory
}
END_TEST

START_TEST (isa_flag_update_cmp_set_carry_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CMP", ABSX);
	cpu->address_mode = ABSX;

	cpu->P = 0;
	// Similar to SBC, signed subtraction from -128 to 127 w/o carry
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 20);
	cpu->A = -113;

	execute_opcode_lut[cpu->opcode](cpu);
	// note comparisons which set the flags are unsigned comparisons
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C); // C is set if register >= memory
}
END_TEST

START_TEST (isa_flag_update_cpx_set_negative)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CPX", IMM);
	cpu->address_mode = IMM;

	cpu->P = 0;
	// Similar to SBC, signed subtraction from -128 to 127 w/o carry
	cpu->operand = 150;
	cpu->X = 77;

	execute_opcode_lut[cpu->opcode](cpu);
	// note comparisons which set the flags are unsigned comparisons
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C); // C is set if register >= memory
}
END_TEST

// Carry is also set when Z flag is set
START_TEST (isa_flag_update_cpx_set_zero_and_carry)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CPX", ZP);
	cpu->address_mode = ZP;

	cpu->P = 0;
	// Similar to SBC, signed subtraction from -128 to 127 w/o carry
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, -113);
	cpu->X = -113;

	execute_opcode_lut[cpu->opcode](cpu);
	// note comparisons which set the flags are unsigned comparisons
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C); // C is set if register >= memory
}
END_TEST

START_TEST (isa_flag_update_cpx_set_carry_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CPX", ABS);
	cpu->address_mode = ABS;

	cpu->P = 0;
	// Similar to SBC, signed subtraction from -128 to 127 w/o carry
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 20);
	cpu->X = -113;

	execute_opcode_lut[cpu->opcode](cpu);
	// note comparisons which set the flags are unsigned comparisons
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C); // C is set if register >= memory
}
END_TEST

START_TEST (isa_flag_update_cpy_set_negative)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CPY", IMM);
	cpu->address_mode = IMM;

	cpu->P = 0;
	// Similar to SBC, signed subtraction from -128 to 127 w/o carry
	cpu->operand = 150;
	cpu->Y = 40;

	execute_opcode_lut[cpu->opcode](cpu);
	// note comparisons which set the flags are unsigned comparisons
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C); // C is set if register >= memory
}
END_TEST

// Carry is also set when Z flag is set
START_TEST (isa_flag_update_cpy_set_zero_and_carry)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CPY", ZP);
	cpu->address_mode = ZP;

	cpu->P = 0;
	// Similar to SBC, signed subtraction from -128 to 127 w/o carry
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, -80);
	cpu->Y = -80;

	execute_opcode_lut[cpu->opcode](cpu);
	// note comparisons which set the flags are unsigned comparisons
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C); // C is set if register >= memory
}
END_TEST

START_TEST (isa_flag_update_cpy_set_carry_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CPY", ABS);
	cpu->address_mode = ABS;

	cpu->P = 0;
	// Similar to SBC, signed subtraction from -128 to 127 w/o carry
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, -20);
	cpu->Y = -1; // -1 is 0xFF

	execute_opcode_lut[cpu->opcode](cpu);
	// note comparisons which set the flags are unsigned comparisons
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C); // C is set if register >= memory
}
END_TEST


Suite* cpu_suite(void)
{
	Suite* s;
	TCase* tc_test_helpers;
	TCase* tc_address_modes;
	TCase* tc_branch_not_taken_addr;
	TCase* tc_branch_taken_addr;
	TCase* tc_branch_take_page_cross;
	TCase* tc_cpu_reads;
	TCase* tc_cpu_writes;
	TCase* tc_cpu_stack_op;
	TCase* tc_cpu_isa;
	TCase* tc_cpu_isa_flags;

	s = suite_create("Cpu Tests");
	tc_test_helpers = tcase_create("Test Helpers");
	tcase_add_test(tc_test_helpers, test_strcmp_reverse_opcode_lut);
	suite_add_tcase(s, tc_test_helpers);
	tc_address_modes = tcase_create("Address Modes Correct Address");
	tcase_add_checked_fixture(tc_address_modes, setup, teardown);
	tcase_add_test(tc_address_modes, addr_mode_imm);
	tcase_add_test(tc_address_modes, addr_mode_abs_read_store);
	tcase_add_test(tc_address_modes, addr_mode_abs_rmw);
	tcase_add_test(tc_address_modes, addr_mode_abs_jmp);
	tcase_add_test(tc_address_modes, addr_mode_absx_read_store);
	tcase_add_test(tc_address_modes, addr_mode_absx_rmw);
	tcase_add_test(tc_address_modes, addr_mode_absy_read_store);
	tcase_add_test(tc_address_modes, addr_mode_zp_read_store);
	tcase_add_test(tc_address_modes, addr_mode_zp_rmw);
	tcase_add_test(tc_address_modes, addr_mode_zpx_read_store);
	tcase_add_test(tc_address_modes, addr_mode_zpx_rmw);
	tcase_add_test(tc_address_modes, addr_mode_zpy_read_store);
	tcase_add_test(tc_address_modes, ind_jmp);
	tcase_add_test(tc_address_modes, ind_jmp_bug);
	tcase_add_test(tc_address_modes, addr_mode_indx_read_store);
	tcase_add_test(tc_address_modes, addr_mode_indy_read_store);
	suite_add_tcase(s, tc_address_modes);
	tc_branch_not_taken_addr = tcase_create("Branch Not Taken Correct Address");
	tcase_add_checked_fixture(tc_branch_not_taken_addr, setup, teardown);
	tcase_add_test(tc_branch_not_taken_addr, bcc_not_taken_correct_addr);
	tcase_add_test(tc_branch_not_taken_addr, bcs_not_taken_correct_addr);
	tcase_add_test(tc_branch_not_taken_addr, beq_not_taken_correct_addr);
	tcase_add_test(tc_branch_not_taken_addr, bmi_not_taken_correct_addr);
	tcase_add_test(tc_branch_not_taken_addr, bne_not_taken_correct_addr);
	tcase_add_test(tc_branch_not_taken_addr, bpl_not_taken_correct_addr);
	tcase_add_test(tc_branch_not_taken_addr, bvc_not_taken_correct_addr);
	tcase_add_test(tc_branch_not_taken_addr, bvs_not_taken_correct_addr);
	suite_add_tcase(s, tc_branch_not_taken_addr);
	tc_branch_taken_addr = tcase_create("Branch Taken Correct Address");
	tcase_add_checked_fixture(tc_branch_taken_addr, setup, teardown);
	tcase_add_test(tc_branch_taken_addr, bcc_taken_correct_addr);
	tcase_add_test(tc_branch_taken_addr, bcs_taken_correct_addr);
	tcase_add_test(tc_branch_taken_addr, beq_taken_correct_addr);
	tcase_add_test(tc_branch_taken_addr, bmi_taken_correct_addr);
	tcase_add_test(tc_branch_taken_addr, bne_taken_correct_addr);
	tcase_add_test(tc_branch_taken_addr, bpl_taken_correct_addr);
	tcase_add_test(tc_branch_taken_addr, bvc_taken_correct_addr);
	tcase_add_test(tc_branch_taken_addr, bvs_taken_correct_addr);
	suite_add_tcase(s, tc_branch_taken_addr);
	tc_branch_take_page_cross = tcase_create("Branch Taken Correct Page Cross Address");
	tcase_add_checked_fixture(tc_branch_take_page_cross, setup, teardown);
	tcase_add_test(tc_branch_take_page_cross, bcc_take_page_cross_correct_addr);
	tcase_add_test(tc_branch_take_page_cross, bcs_take_page_cross_correct_addr);
	tcase_add_test(tc_branch_take_page_cross, beq_take_page_cross_correct_addr);
	tcase_add_test(tc_branch_take_page_cross, bmi_take_page_cross_correct_addr);
	tcase_add_test(tc_branch_take_page_cross, bne_take_page_cross_correct_addr);
	tcase_add_test(tc_branch_take_page_cross, bpl_take_page_cross_correct_addr);
	tcase_add_test(tc_branch_take_page_cross, bvc_take_page_cross_correct_addr);
	tcase_add_test(tc_branch_take_page_cross, bvs_take_page_cross_correct_addr);
	suite_add_tcase(s, tc_branch_take_page_cross);
	tc_cpu_reads = tcase_create("Cpu Memory Mapped Reads");
	tcase_add_checked_fixture(tc_cpu_reads, setup, teardown);
	tcase_add_test(tc_cpu_reads, ram_read_non_mirrored);
	tcase_add_test(tc_cpu_reads, ram_read_mirrored_bank_1);
	tcase_add_test(tc_cpu_reads, ram_read_mirrored_bank_2);
	tcase_add_test(tc_cpu_reads, ram_read_mirrored_bank_3);
	suite_add_tcase(s, tc_cpu_reads);
	tc_cpu_writes = tcase_create("Cpu Memory Mapped Writes");
	tcase_add_checked_fixture(tc_cpu_writes, setup, teardown);
	tcase_add_test(tc_cpu_writes, ram_write_non_mirrored_check_all_reads);
	tcase_add_test(tc_cpu_writes, ram_write_mirrored_bank_1_check_all_reads);
	tcase_add_test(tc_cpu_writes, ram_write_mirrored_bank_2_check_all_reads);
	tcase_add_test(tc_cpu_writes, ram_write_mirrored_bank_3_check_all_reads);
	suite_add_tcase(s, tc_cpu_writes);
	tc_cpu_stack_op = tcase_create("Cpu Stack Operations");
	tcase_add_checked_fixture(tc_cpu_stack_op, setup, teardown);
	tcase_add_test(tc_cpu_stack_op, stack_push_no_overflow);
	tcase_add_test(tc_cpu_stack_op, stack_push_overflow);
	tcase_add_test(tc_cpu_stack_op, stack_pull_no_underflow);
	tcase_add_test(tc_cpu_stack_op, stack_pull_underflow);
	suite_add_tcase(s, tc_cpu_stack_op);
	tc_cpu_isa = tcase_create("Cpu Instruction Set Architecture Core Results");
	tcase_add_checked_fixture(tc_cpu_isa, setup, teardown);
	tcase_add_test(tc_cpu_isa, isa_lda_result_only_imm);
	tcase_add_test(tc_cpu_isa, isa_lda_result_only_non_imm_mode);
	tcase_add_test(tc_cpu_isa, isa_ldx_result_only_imm);
	tcase_add_test(tc_cpu_isa, isa_ldx_result_only_non_imm_mode);
	tcase_add_test(tc_cpu_isa, isa_ldy_result_only_imm);
	tcase_add_test(tc_cpu_isa, isa_ldy_result_only_non_imm_mode);
	tcase_add_test(tc_cpu_isa, isa_sta_result_only);
	tcase_add_test(tc_cpu_isa, isa_stx_result_only);
	tcase_add_test(tc_cpu_isa, isa_sty_result_only);
	tcase_add_test(tc_cpu_isa, isa_tax_result_only);
	tcase_add_test(tc_cpu_isa, isa_tay_result_only);
	tcase_add_test(tc_cpu_isa, isa_tsx_result_only);
	tcase_add_test(tc_cpu_isa, isa_txa_result_only);
	tcase_add_test(tc_cpu_isa, isa_txs_result_only);
	tcase_add_test(tc_cpu_isa, isa_tya_result_only);
	tcase_add_test(tc_cpu_isa, isa_adc_result_only_imm);
	tcase_add_test(tc_cpu_isa, isa_adc_result_only_non_imm_mode);
	tcase_add_test(tc_cpu_isa, isa_adc_result_only_imm_overflow);
	tcase_add_test(tc_cpu_isa, isa_dec_result_only);
	tcase_add_test(tc_cpu_isa, isa_dex_result_only);
	tcase_add_test(tc_cpu_isa, isa_dey_result_only);
	tcase_add_test(tc_cpu_isa, isa_inc_result_only);
	tcase_add_test(tc_cpu_isa, isa_inx_result_only);
	tcase_add_test(tc_cpu_isa, isa_iny_result_only);
	tcase_add_test(tc_cpu_isa, isa_sbc_result_only_imm);
	tcase_add_test(tc_cpu_isa, isa_sbc_result_only_non_imm_mode);
	tcase_add_test(tc_cpu_isa, isa_sbc_result_only_imm_underflow);
	tcase_add_test(tc_cpu_isa, isa_and_result_only_imm);
	tcase_add_test(tc_cpu_isa, isa_and_result_only_non_imm_mode);
	tcase_add_test(tc_cpu_isa, isa_asl_result_only_acc);
	tcase_add_test(tc_cpu_isa, isa_asl_result_only_non_acc_mode);
	tcase_add_test(tc_cpu_isa, isa_eor_result_only_imm);
	tcase_add_test(tc_cpu_isa, isa_eor_result_only_non_imm_mode);
	tcase_add_test(tc_cpu_isa, isa_lsr_result_only_acc);
	tcase_add_test(tc_cpu_isa, isa_lsr_result_only_non_acc_mode);
	tcase_add_test(tc_cpu_isa, isa_ora_result_only_imm);
	tcase_add_test(tc_cpu_isa, isa_ora_result_only_non_imm_mode);
	tcase_add_test(tc_cpu_isa, isa_rol_result_only_acc_carry_not_set);
	tcase_add_test(tc_cpu_isa, isa_rol_result_only_non_acc_mode_carry_not_set);
	tcase_add_test(tc_cpu_isa, isa_rol_result_only_acc_carry_set);
	tcase_add_test(tc_cpu_isa, isa_ror_result_only_acc_carry_not_set);
	tcase_add_test(tc_cpu_isa, isa_ror_result_only_non_acc_mode_carry_not_set);
	tcase_add_test(tc_cpu_isa, isa_ror_result_only_acc_carry_set);
	tcase_add_test(tc_cpu_isa, isa_jsr_result_only);
	tcase_add_test(tc_cpu_isa, isa_rti_result_only);
	tcase_add_test(tc_cpu_isa, isa_rts_result_only);
	tcase_add_test(tc_cpu_isa, isa_clc_result_only);
	tcase_add_test(tc_cpu_isa, isa_cld_result_only);
	tcase_add_test(tc_cpu_isa, isa_cli_result_only);
	tcase_add_test(tc_cpu_isa, isa_clv_result_only);
	tcase_add_test(tc_cpu_isa, isa_sec_result_only);
	tcase_add_test(tc_cpu_isa, isa_sed_result_only);
	tcase_add_test(tc_cpu_isa, isa_sei_result_only);
	tcase_add_test(tc_cpu_isa, isa_pha_result_only);
	tcase_add_test(tc_cpu_isa, isa_php_result_only);
	tcase_add_test(tc_cpu_isa, isa_pla_result_only);
	tcase_add_test(tc_cpu_isa, isa_plp_result_only);
	tcase_add_test(tc_cpu_isa, isa_brk_result_only);
	tcase_add_test(tc_cpu_isa, isa_nop_result_only);
	suite_add_tcase(s, tc_cpu_isa);
	tc_cpu_isa_flags = tcase_create("Cpu Instruction Set Architecture Flag Updates");
	tcase_add_checked_fixture(tc_cpu_isa_flags, setup, teardown);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_adc_imm_overflow);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_sbc_imm_overflow);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_bit_set_all_flags);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_bit_set_overflow);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_bit_set_negative);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_bit_set_zero);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_cmp_set_negative);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_cmp_set_zero_and_carry);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_cmp_set_carry_only);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_cpx_set_negative);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_cpx_set_zero_and_carry);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_cpx_set_carry_only);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_cpy_set_negative);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_cpy_set_zero_and_carry);
	tcase_add_test(tc_cpu_isa_flags, isa_flag_update_cpy_set_carry_only);
	suite_add_tcase(s, tc_cpu_isa_flags);

	return s;
}
