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

static void run_hw_interrupt_cycle_by_cycle(Cpu6502* cpu
                                           , void (*hardware_interrupts[3])(Cpu6502* cpu)
                                           , int array_index
                                           , int cycles_remaining, InstructionStates stop_condition)
{
	cpu->instruction_cycles_remaining = cycles_remaining;
	for (int i = 0; i < cycles_remaining; i++) {
		if (cpu->instruction_state != stop_condition) {
			hardware_interrupts[array_index](cpu);
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

START_TEST (addr_mode_absx_read_store_page_cross)
{
	set_opcode_from_address_mode_and_instruction(cpu, "EOR", ABSX);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0xE1; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x07; // addr_hi (from cpu->PC + 1)
	cpu->X = 0x2F; // X offset to ABS address

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0xE1, cpu->addr_lo);
	ck_assert_uint_eq(0x07, cpu->addr_hi);
	ck_assert_uint_eq(0x0810, cpu->target_addr);
}
END_TEST

/* Test that calculation of page cross address doesn't affect
 * STx type instructions when a page isn't crossed (as for
 * non-STx type instructions the last cycle is skipped if a page
 * isn't crossed, yielding a different result to the page cross address)
 */
START_TEST (addr_mode_absx_read_store_STx_no_page_cross)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STA", ABSX);

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

START_TEST (addr_mode_absy_read_store_page_cross)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDA", ABSY);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x25; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x04; // addr_hi (from cpu->PC + 1)
	cpu->Y = 0xF3; // X offset to ABS address

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0x25, cpu->addr_lo);
	ck_assert_uint_eq(0x04, cpu->addr_hi);
	ck_assert_uint_eq(0x0518, cpu->target_addr);
}
END_TEST

/* Test that calculation of page cross address doesn't affect
 * STx type instructions when a page isn't crossed (as for
 * non-STx type instructions the last cycle is skipped if a page
 * isn't crossed, yielding a different result to the page cross address)
 */
START_TEST (addr_mode_absy_read_store_STx_no_page_cross)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STA", ABSY);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x25; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x04; // addr_hi (from cpu->PC + 1)
	cpu->Y = 0x03; // X offset to ABS address

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0x25, cpu->addr_lo);
	ck_assert_uint_eq(0x04, cpu->addr_hi);
	ck_assert_uint_eq(0x0428, cpu->target_addr);
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

START_TEST (addr_mode_zpx_read_store_page_cross)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STY", ZPX);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x08; // addr_lo, addr_hi fixed to 0x00
	cpu->X = 0xFD;

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0x08, cpu->addr_lo);
	ck_assert_uint_eq(0x0005, cpu->target_addr); // address is always a zero page one
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

START_TEST (addr_mode_zpy_read_store_page_cross)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDX", ZPY);

	cpu->PC = 0x8000;
	cpu->mem[cpu->PC] = 0x63; // addr_lo, addr_hi fixed to 0x00
	cpu->Y = 0xFA;

	// minus one as we skip the fetch cycle
	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);

	ck_assert_uint_eq(0x63, cpu->addr_lo);
	ck_assert_uint_eq(0x005D, cpu->target_addr); // address is always a zero page one
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

START_TEST (addr_mode_indy_read_store_page_cross)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CMP", INDY);

	// set index_lo and index_hi for IND address mode
	// then also set the address it points from that indexed address
	cpu->Y = 0x7A;
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
	ck_assert_uint_eq(0x142B, cpu->target_addr);
}
END_TEST

/* Test that calculation of page cross address doesn't affect
 * STx type instructions when a page isn't crossed (as for
 * non-STx type instructions the last cycle is skipped if a page
 * isn't crossed, yielding a different result to the page cross address)
 */
START_TEST (addr_mode_indy_read_store_STx_no_page_cross)
{
	set_opcode_from_address_mode_and_instruction(cpu, "STA", INDY);

	// set index_lo and index_hi for IND address mode
	// then also set the address it points from that indexed address
	cpu->Y = 0x2A;
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
	ck_assert_uint_eq(0x13DB, cpu->target_addr);
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

START_TEST (generic_read_x_reg)
{
	cpu->X = 0xC4;
	ck_assert_uint_eq(0xC4, cpu_generic_read(cpu, INTERNAL_REG, ZPX, 0x0000, &cpu->X));
}

START_TEST (generic_read_y_reg)
{
	cpu->Y = 0xF9;
	ck_assert_uint_eq(0xF9, cpu_generic_read(cpu, INTERNAL_REG, ABS, 0x00FF, &cpu->Y));
}

START_TEST (generic_read_a_reg)
{
	cpu->A = 0xA3;
	ck_assert_uint_eq(0xA3, cpu_generic_read(cpu, INTERNAL_REG, INDY, 0x4112, &cpu->A));
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


/* ISA unit tests
 *
 * Storage type instructions
 */

START_TEST (isa_lda_imm_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDA", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x44;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x44, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_lda_non_imm_mode_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDA", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x22);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x22, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}

START_TEST (isa_lda_imm_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDA", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x80;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x80, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_lda_imm_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDA", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x00;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_ldx_imm_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDX", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x44;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x44, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_ldx_non_imm_mode_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDX", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x22);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x22, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}

START_TEST (isa_ldx_non_imm_mode_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDX", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x89);
	cpu->P = FLAG_V | FLAG_I | FLAG_Z;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x89, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}

START_TEST (isa_ldx_non_imm_mode_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDX", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x00);
	cpu->P = FLAG_N | FLAG_V | FLAG_I;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}

START_TEST (isa_ldy_imm_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDY", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x44;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x44, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_ldy_non_imm_mode_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDY", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x22);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x22, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}

START_TEST (isa_ldy_imm_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDY", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x8C;
	cpu->P = FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x8C, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_ldy_imm_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LDY", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x00;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
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

START_TEST (isa_tax_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TAX", IMP);
	cpu->A = 0x38;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->X, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_tax_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TAX", IMP);
	cpu->A = 0x91;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->X, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_tax_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TAX", IMP);
	cpu->A = 0x00;
	cpu->P = FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->X, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_tay_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TAY", IMP);
	cpu->A = 0x48;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->Y, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_tay_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TAY", IMP);
	cpu->A = 0x91;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->Y, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_tay_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TAY", IMP);
	cpu->A = 0x00;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->Y, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_tsx_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TSX", IMP);
	cpu->stack = 0x55;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->X, cpu->stack);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_tsx_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TSX", IMP);
	cpu->stack = 0x91;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->X, cpu->stack);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_tsx_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TSX", IMP);
	cpu->stack = 0x00;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->X, cpu->stack);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_txa_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TXA", IMP);
	cpu->X = 0x61;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->A, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_txa_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TXA", IMP);
	cpu->X = 0x91;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->A, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_txa_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TXA", IMP);
	cpu->X = 0x00;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->A, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_txs_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TXS", IMP);
	cpu->X = 0x91;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->stack, cpu->X);
	// No flag updates for this specific transfer instruction
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_tya_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TYA", IMP);
	cpu->Y = 0x7F;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->A, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_tya_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TYA", IMP);
	cpu->Y = 0x91;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->A, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_tya_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "TYA", IMP);
	cpu->Y = 0x00;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(cpu->A, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

/* ISA unit tests
 *
 * Arithmetic type instructions
 */

/* ADC impossible flags combinations:
 *  1) V set (C or N would also be set)
 *  2) NZ set (contradictory)
 *  3) NVC set (NV only happens when 2 pos numbers are added, C can't be set in this case)
 * Current test cases (set flags)
 *  1) none
 *  2) C
 *  3) VZC
 *  4) NC
 *  5) NV
 *  6) ZC
 */
START_TEST (isa_adc_imm_no_carry_in_clear_nvzc_flags)
{
	// Result of ADC: is A + M + C (where M is a value from memory)
	// Signed addition from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 9;
	cpu->A = 8;
	cpu->P = FLAG_I;
	int expected_result = cpu->operand + cpu->A + (cpu->P & FLAG_C);

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq((uint8_t) expected_result, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_adc_imm_no_carry_in_set_c_clear_nvz_flags)
{
	// Result of ADC: is A + M + C (where M is a value from memory)
	// Signed addition from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 3;
	cpu->A = -1;
	cpu->P = FLAG_I;
	// Works because we have 1111... added to 10
	// which propagates a 1 in the carry to the carry out bit (setting the C flag)
	int expected_result = cpu->operand + cpu->A + (cpu->P & FLAG_C);

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq((uint8_t) expected_result, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_adc_imm_no_carry_in_set_zc_clear_nv_flags)
{
	// Result of ADC: is A + M + C (where M is a value from memory)
	// Signed addition from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 1;
	cpu->A = -1;
	cpu->P = FLAG_I;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_adc_imm_no_carry_in_set_vzc_clear_n_flags)
{
	// Result of ADC: is A + M + C (where M is a value from memory)
	// Signed addition from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = -128;
	cpu->A = -128;
	cpu->P = FLAG_I;
	// -256 is 0xFF00 (9th bit is also sent to C flag)
	int expected_result = cpu->operand + cpu->A + (cpu->P & FLAG_C);

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq((uint8_t) expected_result, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_adc_imm_carry_in_set_nc_clear_vz_flags)
{
	// Result of ADC: is A + M + C (where M is a value from memory)
	// Signed addition from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = -1;
	cpu->A = -16;
	cpu->P = FLAG_I | FLAG_C;
	int expected_result = cpu->operand + cpu->A + (cpu->P & FLAG_C);

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq((uint8_t) expected_result, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_adc_imm_carry_in_set_nv_clear_zc_flags)
{
	// Result of ADC: is A + M + C (where M is a value from memory)
	// Signed addition from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x40;
	cpu->A = 0x40;
	cpu->P = FLAG_I | FLAG_C;
	int expected_result = cpu->operand + cpu->A + (cpu->P & FLAG_C);

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq((uint8_t) expected_result, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_adc_non_imm_mode_carry_in_set_nv_clear_zc_flags)
{
	// Result of ADC: is A + M + C (where M is a value from memory)
	// Signed addition from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "ADC", INDY);
	cpu->address_mode = INDY;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x40);
	cpu->A = 0x40;
	cpu->P = FLAG_I | FLAG_C;
	int expected_result = read_from_cpu(cpu, cpu->target_addr) + cpu->A + (cpu->P & FLAG_C);

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq((uint8_t) expected_result, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_dec_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEC", ABSX);
	cpu->target_addr = 0x0C43;
	write_to_cpu(cpu, cpu->target_addr, 0x19);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x18, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_dec_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEC", ABSX);
	cpu->target_addr = 0x0C43;
	write_to_cpu(cpu, cpu->target_addr, 0xA9);
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xA8, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_dec_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEC", ABSX);
	cpu->target_addr = 0x0C43;
	write_to_cpu(cpu, cpu->target_addr, 0x01);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_dex_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEX", IMP);
	cpu->X = 0x80;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x7F, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_dex_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEX", IMP);
	cpu->X = 0xFF;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xFE, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_dex_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEX", IMP);
	cpu->X = 0x01;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_dey_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEY", IMP);
	cpu->Y = 0x7F;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x7E, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_dey_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEY", IMP);
	cpu->Y = 0xFF;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xFE, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_dey_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "DEY", IMP);
	cpu->Y = 0x01;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_inc_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INC", ABS);
	cpu->target_addr = 0x0C43;
	write_to_cpu(cpu, cpu->target_addr, 0x2D);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x2E, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_inc_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INC", ABS);
	cpu->target_addr = 0x0C43;
	write_to_cpu(cpu, cpu->target_addr, 0xA9);
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xAA, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_inc_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INC", ABS);
	cpu->target_addr = 0x0C43;
	write_to_cpu(cpu, cpu->target_addr, 0xFF);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_inx_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INX", IMP);
	cpu->X = 0x67;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x68, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_inx_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INX", IMP);
	cpu->X = 0xFE;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xFF, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_inx_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INX", IMP);
	cpu->X = 0xFF;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, cpu->X);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_iny_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INY", IMP);
	cpu->Y = 0x7E;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x7F, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_iny_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INY", IMP);
	cpu->Y = 0x7F;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x80, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_iny_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "INY", IMP);
	cpu->Y = 0xFF;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, cpu->Y);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

/* SBC impossible flags combinations
 *  1) NVZC clear (C is only clear if result is negative)
 *  2) NZ set (contradictory)
 *  3) NC set (C is only set when N is clear)
 * Current test cases (set flags)
 *  1) C
 *  2) NV
 *  3) N
 *  4) VZC
 *  5) ZC
 */
START_TEST (isa_sbc_imm_carry_in_set_c_clear_nvz_flags)
{
	// Result of SBC: is A - M - !C (where M is a value from memory)
	// Signed subtraction from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 17;
	cpu->A = 79;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C; // Borrow is set when carry in isn't
	int expected_result = cpu->A - cpu->operand - !(cpu->P & FLAG_C);

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq((uint8_t) expected_result, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C); // Carry is set on positive results
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_sbc_imm_no_carry_in_set_nv_clear_zc_flags)
{
	// Result of SBC: is A - M + !C (where M is a value from memory)
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", IMM);
	cpu->address_mode = IMM;

	// SBC uses signed addition, so the inputs are limited to -128 to 127
	// Overflow occurs only when both operands have the same sign
	// For overflow the result will have a different sign to the inputs
	cpu->operand = -24;
	cpu->A = 127;
	cpu->P = FLAG_I | FLAG_C;
	cpu->P &= ~FLAG_C;
	int expected_result = cpu->A - cpu->operand - !(cpu->P & FLAG_C);

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq((uint8_t) expected_result, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}
END_TEST

START_TEST (isa_sbc_imm_no_carry_in_set_n_clear_vzc_flags)
{
	// Result of SBC: is A - M - !C (where M is a value from memory)
	// Signed subtraction from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x40;
	cpu->A = 0x40;
	cpu->P = FLAG_I | FLAG_C;
	cpu->P &= ~FLAG_C; // Borrow is set when carry in isn't
	int expected_result = cpu->A - cpu->operand - !(cpu->P & FLAG_C);

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq((uint8_t) expected_result, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_sbc_non_imm_mode_no_carry_in_set_n_clear_vzc_flags)
{
	// Result of SBC: is A - M - !C (where M is a value from memory)
	// Signed subtraction from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", INDY);
	cpu->address_mode = INDY;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x40);
	cpu->A = 0x40;
	cpu->P = FLAG_I | FLAG_C;
	cpu->P &= ~FLAG_C; // Borrow is set when carry in isn't
	int expected_result = cpu->A - read_from_cpu(cpu, cpu->target_addr) - !(cpu->P & FLAG_C);

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq((uint8_t) expected_result, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_sbc_non_imm_mode_carry_in_set_vzc_clear_n_flags)
{
	// Result of SBC: is A - M - !C (where M is a value from memory)
	// Signed subtraction from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", INDY);
	cpu->address_mode = INDY;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 127);
	cpu->A = -128;
	cpu->P = FLAG_I | FLAG_C;
	cpu->P &= ~FLAG_C; // Borrow is set when carry in isn't
	// When going more negative than -128 we wrap back around to 127
	// so minus 1 to get to 127 (via Borrow) and then minus 127 to get to 0

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C); // Carry is set on positive results
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_sbc_non_imm_mode_no_carry_in_set_zc_clear_nv_flags)
{
	// Result of SBC: is A - M - !C (where M is a value from memory)
	// Signed subtraction from -128 to 127 w/o carry
	set_opcode_from_address_mode_and_instruction(cpu, "SBC", INDY);
	cpu->address_mode = INDY;
	cpu->target_addr = 0x01F0;
	write_to_cpu(cpu, cpu->target_addr, 0x3F);
	cpu->A = 0x40;
	cpu->P = FLAG_I | FLAG_C;
	cpu->P &= ~FLAG_C; // Borrow is set when carry in isn't

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C); // Carry is set on positive results
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

/* ISA unit tests
 *
 * Bitwise type instructions
 */

START_TEST (isa_and_imm_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "AND", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x1F;
	cpu->A = 0x10;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x10, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_and_non_imm_mode_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "AND", ZP);
	cpu->address_mode = ZP;
	cpu->target_addr = 0x00F2;
	write_to_cpu(cpu, cpu->target_addr, 0x1F);
	cpu->A = 0x10;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x10, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_and_non_imm_mode_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "AND", ZP);
	cpu->address_mode = ZP;
	cpu->target_addr = 0x00F2;
	write_to_cpu(cpu, cpu->target_addr, 0x8F);
	cpu->A = 0x80;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x80, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_and_non_imm_mode_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "AND", ZP);
	cpu->address_mode = ZP;
	cpu->target_addr = 0x00F2;
	write_to_cpu(cpu, cpu->target_addr, 0xAF);
	cpu->A = 0x10;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_asl_acc_clear_nzc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ASL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x03;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x06, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_asl_acc_set_n_clear_zc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ASL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x7F;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xFE, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_asl_non_acc_mode_set_n_clear_zc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ASL", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00F2;
	write_to_cpu(cpu, cpu->target_addr, 0x7F);
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xFE, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_asl_acc_set_z_clear_nc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ASL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x00;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_asl_acc_set_c_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ASL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x81;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x02, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_asl_acc_set_zc_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ASL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x80;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_asl_acc_set_nc_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ASL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0xC0;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x80, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_bit_clear_nvz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ABS);

	cpu->P = 0;
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 0x01);
	cpu->A = 0x01;
	cpu->P = FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N); // Bit 7 from mem
	ck_assert((cpu->P & FLAG_V) != FLAG_V); // Bit 6 from mem
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z); // Set if A & mem == 0
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}
END_TEST

START_TEST (isa_bit_set_nvz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ABS);

	cpu->P = 0;
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 0xFF);
	cpu->A = 0x00;
	cpu->P = FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N); // Bit 7 from mem
	ck_assert((cpu->P & FLAG_V) == FLAG_V); // Bit 6 from mem
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z); // Set if A & mem == 0
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}
END_TEST

START_TEST (isa_bit_set_vn_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ABS);

	cpu->P = FLAG_N | FLAG_I | FLAG_Z | FLAG_C;
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 0xC1);
	cpu->A = 0x11;

	execute_opcode_lut[cpu->opcode](cpu);

	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N); // Bit 7 from mem
	ck_assert((cpu->P & FLAG_V) == FLAG_V); // Bit 6 from mem
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z); // Set if A & mem == 0
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}
END_TEST

START_TEST (isa_bit_set_v_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ABS);

	cpu->P = FLAG_N | FLAG_I | FLAG_Z | FLAG_C;
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 0xFF & ~FLAG_N);
	cpu->A = 0xFF & ~FLAG_N;

	execute_opcode_lut[cpu->opcode](cpu);

	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N); // Bit 7 from mem
	ck_assert((cpu->P & FLAG_V) == FLAG_V); // Bit 6 from mem
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z); // Set if A & mem == 0
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}
END_TEST

START_TEST (isa_bit_set_n_clear_vz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ABS);

	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 0xFF & ~FLAG_V);
	cpu->A = 0xFF & ~FLAG_V;

	execute_opcode_lut[cpu->opcode](cpu);

	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N); // Bit 7 from mem
	ck_assert((cpu->P & FLAG_V) != FLAG_V); // Bit 6 from mem
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z); // Set if A & mem == 0
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}
END_TEST

START_TEST (isa_bit_set_z_clear_nv_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BIT", ABS);

	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;
	cpu->target_addr = 0x1000;
	write_to_cpu(cpu, cpu->target_addr, 0xFF & ~FLAG_N & ~FLAG_V);
	cpu->A = FLAG_N; // make sure no matching bits

	execute_opcode_lut[cpu->opcode](cpu);

	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N); // Bit 7 from mem
	ck_assert((cpu->P & FLAG_V) != FLAG_V); // Bit 6 from mem
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z); // Set if A & mem == 0
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}
END_TEST

START_TEST (isa_eor_imm_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "EOR", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x1F;
	cpu->A = 0x10;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x0F, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_eor_non_imm_mode_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "EOR", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x0C12;
	write_to_cpu(cpu, cpu->target_addr, 0x1F);
	cpu->A = 0x10;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x0F, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_eor_non_imm_mode_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "EOR", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x0C12;
	write_to_cpu(cpu, cpu->target_addr, 0x8F);
	cpu->A = 0x10;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x9F, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_eor_non_imm_mode_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "EOR", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x0C12;
	write_to_cpu(cpu, cpu->target_addr, 0xFF);
	cpu->A = 0xFF;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_lsr_acc_clear_nzc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LSR", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x1E;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x0F, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_lsr_non_acc_mode_clear_nzc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LSR", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	write_to_cpu(cpu, cpu->target_addr, 0x1E);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x0F, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N); // should always be cleared 8th bit is shifted down
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_lsr_non_acc_mode_set_z_clear_nc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LSR", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	write_to_cpu(cpu, cpu->target_addr, 0x00);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N); // should always be cleared 8th bit is shifted down
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_lsr_non_acc_mode_set_c_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LSR", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	write_to_cpu(cpu, cpu->target_addr, 0x41);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x20, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N); // should always be cleared 8th bit is shifted down
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C); // C is set when input has a 1 in lowest bit
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_lsr_non_acc_mode_set_zc_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "LSR", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	write_to_cpu(cpu, cpu->target_addr, 0x01);
	cpu->P = FLAG_N | FLAG_V | FLAG_I;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N); // should always be cleared 8th bit is shifted down
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C); // C is set when input has a 1 in lowest bit
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_ora_imm_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ORA", IMM);
	cpu->address_mode = IMM;
	cpu->operand = 0x1F;
	cpu->A = 0x10;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x1F, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_ora_non_imm_mode_clear_nz_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ORA", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x0C12;
	write_to_cpu(cpu, cpu->target_addr, 0x1F);
	cpu->A = 0x10;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x1F, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_ora_non_imm_mode_set_n_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ORA", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x0C12;
	write_to_cpu(cpu, cpu->target_addr, 0x22);
	cpu->A = 0x80;
	cpu->P = FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xA2, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_ora_non_imm_mode_set_z_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ORA", ABS);
	cpu->address_mode = ABS;
	cpu->target_addr = 0x0C12;
	write_to_cpu(cpu, cpu->target_addr, 0x00);
	cpu->A = 0x00;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z); // can only be set if both inputs are 0
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_rol_acc_no_carry_in_clear_nzc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x3F;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z; // can't set carry, used for result

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x7E, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_rol_acc_no_carry_in_set_n_clear_zc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x7F;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z; // can't set carry, used for result

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xFE, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_rol_non_acc_mode_no_carry_in_set_n_clear_zc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z; // can't set carry, used for result

	write_to_cpu(cpu, cpu->target_addr, 0x7F);
	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xFE, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_rol_non_acc_mode_no_carry_in_set_zc_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z; // can't set carry, used for result

	write_to_cpu(cpu, cpu->target_addr, 0x80);
	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x00, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_rol_non_acc_mode_carry_in_set_nc_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C; // Carry moves into vacated LSB spot

	write_to_cpu(cpu, cpu->target_addr, 0xC0);
	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x81, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_rol_acc_carry_in_set_n_clear_zc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0x7F;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C; // Carry moves into vacated LSB spot

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xFF, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_rol_non_acc_mode_no_carry_in_set_z_clear_nc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROL", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z; // can't set carry, used for result

	write_to_cpu(cpu, cpu->target_addr, 0x00);
	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_ror_acc_no_carry_in_clear_nzc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROR", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0xFE;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z; // can't set carry, used for result

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x7F, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_ror_acc_mode_no_carry_in_clear_nzc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROR", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	write_to_cpu(cpu, cpu->target_addr, 0xFE);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z; // can't set carry, used for result

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0x7F, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_ror_acc_mode_no_carry_in_set_z_clear_nc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROR", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	write_to_cpu(cpu, cpu->target_addr, 0x00);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z; // can't set carry, used for result

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_ror_acc_mode_no_carry_in_set_zc_clear_n_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROR", ZPX);
	cpu->address_mode = ZPX;
	cpu->target_addr = 0x00A0;
	write_to_cpu(cpu, cpu->target_addr, 0x01);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z; // can't set carry, used for result

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0, read_from_cpu(cpu, cpu->target_addr));
	// Flag checks
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_ror_acc_carry_in_set_n_clear_zc_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROR", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0xFE;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C; // Carry moves into vacated MSB spot

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xFF, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N); // only set when Carry in is set too
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

START_TEST (isa_ror_acc_carry_in_set_nc_clear_z_flags)
{
	set_opcode_from_address_mode_and_instruction(cpu, "ROR", ACC);
	cpu->address_mode = ACC;
	cpu->A = 0xF1;
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C; // Carry moves into vacated MSB spot

	execute_opcode_lut[cpu->opcode](cpu);

	ck_assert_uint_eq(0xF8, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N); // only set when Carry in is set too
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
}

/* ISA unit tests
 *
 * Jump type instructions
 */

START_TEST (isa_jsr_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "JSR", IMP);
	cpu->PC = 0x9000;
	cpu->mem[cpu->PC] = 0x00; // addr_lo
	cpu->mem[cpu->PC + 1] = 0x80; // addr_hi
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;


	decode_opcode_lut[cpu->opcode](cpu); // setup needed for the for loop below
	run_logic_cycle_by_cycle(cpu, execute_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, FETCH);

	ck_assert_uint_eq(0x00, cpu->addr_lo);
	ck_assert_uint_eq(0x80, cpu->addr_hi);
	ck_assert_uint_eq(0x8000, cpu->target_addr);
	ck_assert_uint_eq(0x8000, cpu->PC);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_rti_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "RTI", IMP);
	// inverse of pull operations
	stack_push(cpu, 0x80); // push addr_hi onto stack
	stack_push(cpu, 0x01); // push addr_lo onto stack
	stack_push(cpu, 0x40); // push status reg onto stack (Flag_V)

	decode_opcode_lut[cpu->opcode](cpu); // setup needed for the for loop below
	run_logic_cycle_by_cycle(cpu, execute_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, FETCH);

	ck_assert_uint_eq(0x01, cpu->addr_lo);
	ck_assert_uint_eq(0x80, cpu->addr_hi);
	ck_assert_uint_eq(0x8001, cpu->PC);
	// Flags depend on previous stack
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}

START_TEST (isa_rts_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "RTS", IMP);
	// inverse of pull operations
	stack_push(cpu, 0x80); // push addr_hi onto stack
	stack_push(cpu, 0x02); // push addr_lo onto stack
	cpu->P = 0;

	run_logic_cycle_by_cycle(cpu, decode_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, EXECUTE);
	execute_opcode_lut[cpu->opcode](cpu); // set PC from logic above

	ck_assert_uint_eq(0x02, cpu->addr_lo);
	ck_assert_uint_eq(0x80, cpu->addr_hi);
	ck_assert_uint_eq(0x8002, cpu->target_addr);
	ck_assert_uint_eq(0x8003, cpu->PC); // PC == PCH, PCL + 1 for RTS
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}

/* ISA unit tests
 *
 * Register type instructions
 */

START_TEST (isa_clc_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CLC", IMP);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_C) != FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
}

START_TEST (isa_cld_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CLD", IMP);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_D) != FLAG_D);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_cli_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CLI", IMP);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_I) != FLAG_I);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_clv_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "CLV", IMP);
	cpu->P = FLAG_N | FLAG_V | FLAG_I | FLAG_Z | FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

/* CMP, CPX & CPY impossible status reg flag combinations
 *  1) NZ set (contradictory)
 *  2) NC set (C is set if register >= mem, therefore result is +ve and N can't be set)
 */
START_TEST (isa_cmp_set_n_clear_zc_flags)
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
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
}
END_TEST

// Carry is also set when Z flag is set
START_TEST (isa_cmp_set_zc_clear_n_flags)
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
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
}
END_TEST

START_TEST (isa_cmp_set_c_clear_nz_flags)
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
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
}
END_TEST

START_TEST (isa_cpx_set_n_clear_zc_flags)
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
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
}
END_TEST

// Carry is also set when Z flag is set
START_TEST (isa_cpx_set_zc_clear_n_flags)
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
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
}
END_TEST

START_TEST (isa_cpx_set_c_clear_nz_flags)
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
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
}
END_TEST

START_TEST (isa_cpy_set_n_clear_zc_flags)
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
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
}
END_TEST

// Carry is also set when Z flag is set
START_TEST (isa_cpy_set_zc_clear_n_flags)
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
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
}
END_TEST

START_TEST (isa_cpy_set_c_clear_nz_flags)
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
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
}
END_TEST

START_TEST (isa_sec_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "SEC", IMP);
	cpu->P = 0xFF & ~FLAG_C;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_C) == FLAG_C);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
}

START_TEST (isa_sed_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "SED", IMP);
	cpu->P = 0xFF & ~FLAG_D;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_D) == FLAG_D);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

START_TEST (isa_sei_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "SEI", IMP);
	cpu->P = 0xFF & ~FLAG_I;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
}

/* ISA unit tests
 *
 * Stack type instructions
 */

START_TEST (isa_pha_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "PHA", IMP);
	cpu->A = 0x54;
	cpu->P = 0;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert_uint_eq(cpu->A, stack_pull(cpu));
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
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
	cpu->P = 0;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert_uint_eq(0xA4, cpu->A);
	// Flag checks
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
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

/* ISA unit tests
 *
 * System/misc type instructions
 */

START_TEST (isa_brk_result_only)
{
	set_opcode_from_address_mode_and_instruction(cpu, "BRK", IMP);
	cpu->mem[BRK_VECTOR] = 0x0A; // addr_lo
	cpu->mem[BRK_VECTOR + 1] = 0x90; // addr_hi
	cpu->P = FLAG_N | FLAG_V | FLAG_Z | FLAG_C;
	cpu->instruction_state = EXECUTE;

	run_logic_cycle_by_cycle(cpu, execute_opcode_lut
	                        , max_cycles_opcode_lut[cpu->opcode] - 1, FETCH);

	ck_assert_uint_eq(0x0A, cpu->addr_lo);
	ck_assert_uint_eq(0x90, cpu->addr_hi);
	ck_assert_uint_eq(0x900A, cpu->PC);
	// Flag checks
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) == FLAG_C);
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
	cpu->P = 0;
	cpu->instruction_state = EXECUTE;

	execute_opcode_lut[cpu->opcode](cpu); // only a single cycle instruction

	ck_assert_uint_eq(old_PC, cpu->PC);
	ck_assert_uint_eq(old_A, cpu->A);
	ck_assert_uint_eq(old_X, cpu->X);
	ck_assert_uint_eq(old_Y, cpu->Y);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) != FLAG_N);
	ck_assert((cpu->P & FLAG_V) != FLAG_V);
	ck_assert((cpu->P & FLAG_I) != FLAG_I);
	ck_assert((cpu->P & FLAG_Z) != FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}
END_TEST


START_TEST (irq_correct_interrupt_vector)
{
	cpu->mem[IRQ_VECTOR] = 0xFA; // addr_lo
	cpu->mem[IRQ_VECTOR + 1] = 0xC4; // addr_hi
	cpu->P = FLAG_N | FLAG_V | FLAG_Z;
	int total_cycles = 7;
	cpu->instruction_state = EXECUTE;

	run_hw_interrupt_cycle_by_cycle(cpu, hardware_interrupts, IRQ_INDEX
	                               , total_cycles - 1, FETCH);

	ck_assert_uint_eq(0xFA, cpu->addr_lo);
	ck_assert_uint_eq(0xC4, cpu->addr_hi);
	ck_assert_uint_eq(0xC4FA, cpu->PC);
	// Flag checks
	ck_assert((cpu->P & FLAG_I) == FLAG_I);
	// Flags that should be unaffected
	ck_assert((cpu->P & FLAG_N) == FLAG_N);
	ck_assert((cpu->P & FLAG_V) == FLAG_V);
	ck_assert((cpu->P & FLAG_Z) == FLAG_Z);
	ck_assert((cpu->P & FLAG_C) != FLAG_C);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_adc)
{
	char ins[4] = "ADC";
	uint8_t adc_opcodes[8] = { reverse_opcode_lut(&ins, IMM)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)
	                         , reverse_opcode_lut(&ins, ABSY)
	                         , reverse_opcode_lut(&ins, INDX)
	                         , reverse_opcode_lut(&ins, INDY)};

	execute_opcode_lut[adc_opcodes[_i]](cpu);

	ck_assert_str_eq("ADC ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_and)
{
	char ins[4] = "AND";
	uint8_t and_opcodes[8] = { reverse_opcode_lut(&ins, IMM)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)
	                         , reverse_opcode_lut(&ins, ABSY)
	                         , reverse_opcode_lut(&ins, INDX)
	                         , reverse_opcode_lut(&ins, INDY)};

	execute_opcode_lut[and_opcodes[_i]](cpu);

	ck_assert_str_eq("AND ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_asl)
{
	char ins[4] = "ASL";
	uint8_t asl_opcodes[5] = { reverse_opcode_lut(&ins, ACC)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)};

	execute_opcode_lut[asl_opcodes[_i]](cpu);

	ck_assert_str_eq("ASL ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_bcc)
{
	char ins[4] = "BCC";
	uint8_t bcc_opcode  = reverse_opcode_lut(&ins, REL);

	execute_opcode_lut[bcc_opcode](cpu);

	ck_assert_str_eq("BCC ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_bcs)
{
	char ins[4] = "BCS";
	uint8_t bcs_opcode  = reverse_opcode_lut(&ins, REL);

	execute_opcode_lut[bcs_opcode](cpu);

	ck_assert_str_eq("BCS ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_beq)
{
	char ins[4] = "BEQ";
	uint8_t beq_opcode  = reverse_opcode_lut(&ins, REL);

	execute_opcode_lut[beq_opcode](cpu);

	ck_assert_str_eq("BEQ ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_bit)
{
	char ins[4] = "BIT";
	uint8_t bit_opcodes[2] = { reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ABS)};

	execute_opcode_lut[bit_opcodes[_i]](cpu);

	ck_assert_str_eq("BIT ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_bmi)
{
	char ins[4] = "BMI";
	uint8_t bmi_opcode  = reverse_opcode_lut(&ins, REL);

	execute_opcode_lut[bmi_opcode](cpu);

	ck_assert_str_eq("BMI ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_bne)
{
	char ins[4] = "BNE";
	uint8_t bne_opcode  = reverse_opcode_lut(&ins, REL);

	execute_opcode_lut[bne_opcode](cpu);

	ck_assert_str_eq("BNE ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_bpl)
{
	char ins[4] = "BPL";
	uint8_t bpl_opcode  = reverse_opcode_lut(&ins, REL);

	execute_opcode_lut[bpl_opcode](cpu);

	ck_assert_str_eq("BPL ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_brk)
{
	char ins[4] = "BRK";
	uint8_t brk_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[brk_opcode](cpu);

	ck_assert_str_eq("BRK ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_bvc)
{
	char ins[4] = "BVC";
	uint8_t bvc_opcode  = reverse_opcode_lut(&ins, REL);

	execute_opcode_lut[bvc_opcode](cpu);

	ck_assert_str_eq("BVC ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_bvs)
{
	char ins[4] = "BVS";
	uint8_t bvs_opcode  = reverse_opcode_lut(&ins, REL);

	execute_opcode_lut[bvs_opcode](cpu);

	ck_assert_str_eq("BVS ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_clc)
{
	char ins[4] = "CLC";
	uint8_t clc_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[clc_opcode](cpu);

	ck_assert_str_eq("CLC", cpu->instruction); // no space for implied address mode
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_cld)
{
	char ins[4] = "CLD";
	uint8_t cld_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[cld_opcode](cpu);

	ck_assert_str_eq("CLD", cpu->instruction); // no space for implied address mode
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_cli)
{
	char ins[4] = "CLI";
	uint8_t cli_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[cli_opcode](cpu);

	ck_assert_str_eq("CLI", cpu->instruction); // no space for implied address mode
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_clv)
{
	char ins[4] = "CLV";
	uint8_t clv_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[clv_opcode](cpu);

	ck_assert_str_eq("CLV", cpu->instruction); // no space for implied address mode
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_cmp)
{
	char ins[4] = "CMP";
	uint8_t cmp_opcodes[8] = { reverse_opcode_lut(&ins, IMM)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)
	                         , reverse_opcode_lut(&ins, ABSY)
	                         , reverse_opcode_lut(&ins, INDX)
	                         , reverse_opcode_lut(&ins, INDY)};

	execute_opcode_lut[cmp_opcodes[_i]](cpu);

	ck_assert_str_eq("CMP ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_cpx)
{
	char ins[4] = "CPX";
	uint8_t cpx_opcodes[3] = { reverse_opcode_lut(&ins, IMM)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ABS)};

	execute_opcode_lut[cpx_opcodes[_i]](cpu);

	ck_assert_str_eq("CPX ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_cpy)
{
	char ins[4] = "CPY";
	uint8_t cpy_opcodes[3] = { reverse_opcode_lut(&ins, IMM)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ABS)};

	execute_opcode_lut[cpy_opcodes[_i]](cpu);

	ck_assert_str_eq("CPY ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_dec)
{
	char ins[4] = "DEC";
	uint8_t dec_opcodes[4] = { reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)};

	execute_opcode_lut[dec_opcodes[_i]](cpu);

	ck_assert_str_eq("DEC ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_dex)
{
	char ins[4] = "DEX";
	uint8_t dex_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[dex_opcode](cpu);

	ck_assert_str_eq("DEX ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_dey)
{
	char ins[4] = "DEY";
	uint8_t dey_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[dey_opcode](cpu);

	ck_assert_str_eq("DEY ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_eor)
{
	char ins[4] = "EOR";
	uint8_t eor_opcodes[8] = { reverse_opcode_lut(&ins, IMM)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)
	                         , reverse_opcode_lut(&ins, ABSY)
	                         , reverse_opcode_lut(&ins, INDX)
	                         , reverse_opcode_lut(&ins, INDY)};

	execute_opcode_lut[eor_opcodes[_i]](cpu);

	ck_assert_str_eq("EOR ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_inc)
{
	char ins[4] = "INC";
	uint8_t inc_opcodes[4] = { reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)};

	execute_opcode_lut[inc_opcodes[_i]](cpu);

	ck_assert_str_eq("INC ", cpu->instruction);
}

START_TEST (log_correct_instruction_mnemonic_inx)
{
	char ins[4] = "INX";
	uint8_t inx_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[inx_opcode](cpu);

	ck_assert_str_eq("INX ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_iny)
{
	char ins[4] = "INY";
	uint8_t iny_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[iny_opcode](cpu);

	ck_assert_str_eq("INY ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_jmp)
{
	char ins[4] = "JMP";
	uint8_t jmp_opcodes[2] = { reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, IND)};

	execute_opcode_lut[jmp_opcodes[_i]](cpu);

	ck_assert_str_eq("JMP ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_and_address_jsr)
{
	char ins[4] = "JSR";
	uint8_t jsr_opcode  = reverse_opcode_lut(&ins, ABS);
	cpu->instruction_cycles_remaining = 1; // last cycle sets the trace logger strings
	cpu->addr_lo = 0x04; // set @ earlier cycles, read from PC
	execute_opcode_lut[jsr_opcode](cpu);

	ck_assert_str_eq("JSR $0004", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_lda)
{
	char ins[4] = "LDA";
	uint8_t lda_opcodes[8] = { reverse_opcode_lut(&ins, IMM)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)
	                         , reverse_opcode_lut(&ins, ABSY)
	                         , reverse_opcode_lut(&ins, INDX)
	                         , reverse_opcode_lut(&ins, INDY)};

	execute_opcode_lut[lda_opcodes[_i]](cpu);

	ck_assert_str_eq("LDA ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_ldx)
{
	char ins[4] = "LDX";
	uint8_t ldx_opcodes[5] = { reverse_opcode_lut(&ins, IMM)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPY)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSY)};

	execute_opcode_lut[ldx_opcodes[_i]](cpu);

	ck_assert_str_eq("LDX ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_ldy)
{
	char ins[4] = "LDY";
	uint8_t ldy_opcodes[5] = { reverse_opcode_lut(&ins, IMM)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)};

	execute_opcode_lut[ldy_opcodes[_i]](cpu);

	ck_assert_str_eq("LDY ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_lsr)
{
	char ins[4] = "LSR";
	uint8_t lsr_opcodes[5] = { reverse_opcode_lut(&ins, ACC)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)};

	execute_opcode_lut[lsr_opcodes[_i]](cpu);

	ck_assert_str_eq("LSR ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_nop)
{
	char ins[4] = "NOP";
	uint8_t nop_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[nop_opcode](cpu);

	ck_assert_str_eq("NOP ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_ora)
{
	char ins[4] = "ORA";
	uint8_t ora_opcodes[8] = { reverse_opcode_lut(&ins, IMM)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)
	                         , reverse_opcode_lut(&ins, ABSY)
	                         , reverse_opcode_lut(&ins, INDX)
	                         , reverse_opcode_lut(&ins, INDY)};

	execute_opcode_lut[ora_opcodes[_i]](cpu);

	ck_assert_str_eq("ORA ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_pha)
{
	char ins[4] = "PHA";
	uint8_t pha_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[pha_opcode](cpu);

	ck_assert_str_eq("PHA ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_php)
{
	char ins[4] = "PHP";
	uint8_t php_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[php_opcode](cpu);

	ck_assert_str_eq("PHP ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_pla)
{
	char ins[4] = "PLA";
	uint8_t pla_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[pla_opcode](cpu);

	ck_assert_str_eq("PLA ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_plp)
{
	char ins[4] = "PLP";
	uint8_t plp_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[plp_opcode](cpu);

	ck_assert_str_eq("PLP ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_rol)
{
	char ins[4] = "ROL";
	uint8_t rol_opcodes[5] = { reverse_opcode_lut(&ins, ACC)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)};

	execute_opcode_lut[rol_opcodes[_i]](cpu);

	ck_assert_str_eq("ROL ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_ror)
{
	char ins[4] = "ROR";
	uint8_t ror_opcodes[5] = { reverse_opcode_lut(&ins, ACC)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)};

	execute_opcode_lut[ror_opcodes[_i]](cpu);

	ck_assert_str_eq("ROR ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_rti)
{
	char ins[4] = "RTI";
	uint8_t rti_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[rti_opcode](cpu);

	ck_assert_str_eq("RTI", cpu->instruction); // no extra space for this instruction
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_rts)
{
	char ins[4] = "RTS";
	uint8_t rts_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[rts_opcode](cpu);

	ck_assert_str_eq("RTS", cpu->instruction); // no extra space for this instruction
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_sbc)
{
	char ins[4] = "SBC";
	uint8_t sbc_opcodes[8] = { reverse_opcode_lut(&ins, IMM)
	                         , reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)
	                         , reverse_opcode_lut(&ins, ABSY)
	                         , reverse_opcode_lut(&ins, INDX)
	                         , reverse_opcode_lut(&ins, INDY)};

	execute_opcode_lut[sbc_opcodes[_i]](cpu);

	ck_assert_str_eq("SBC ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_sec)
{
	char ins[4] = "SEC";
	uint8_t sec_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[sec_opcode](cpu);

	ck_assert_str_eq("SEC ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_sed)
{
	char ins[4] = "SED";
	uint8_t sed_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[sed_opcode](cpu);

	ck_assert_str_eq("SED ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_sei)
{
	char ins[4] = "SEI";
	uint8_t sei_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[sei_opcode](cpu);

	ck_assert_str_eq("SEI ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_sta)
{
	char ins[4] = "STA";
	uint8_t sta_opcodes[7] = { reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)
	                         , reverse_opcode_lut(&ins, ABSX)
	                         , reverse_opcode_lut(&ins, ABSY)
	                         , reverse_opcode_lut(&ins, INDX)
	                         , reverse_opcode_lut(&ins, INDY)};

	execute_opcode_lut[sta_opcodes[_i]](cpu);

	ck_assert_str_eq("STA ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_stx)
{
	char ins[4] = "STX";
	uint8_t stx_opcodes[3] = { reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPY)
	                         , reverse_opcode_lut(&ins, ABS)};

	execute_opcode_lut[stx_opcodes[_i]](cpu);

	ck_assert_str_eq("STX ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_sty)
{
	char ins[4] = "STY";
	uint8_t sty_opcodes[3] = { reverse_opcode_lut(&ins, ZP)
	                         , reverse_opcode_lut(&ins, ZPX)
	                         , reverse_opcode_lut(&ins, ABS)};

	execute_opcode_lut[sty_opcodes[_i]](cpu);

	ck_assert_str_eq("STY ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_tax)
{
	char ins[4] = "TAX";
	uint8_t tax_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[tax_opcode](cpu);

	ck_assert_str_eq("TAX ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_tay)
{
	char ins[4] = "TAY";
	uint8_t tay_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[tay_opcode](cpu);

	ck_assert_str_eq("TAY ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_tsx)
{
	char ins[4] = "TSX";
	uint8_t tsx_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[tsx_opcode](cpu);

	ck_assert_str_eq("TSX ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_txa)
{
	char ins[4] = "TXA";
	uint8_t txa_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[txa_opcode](cpu);

	ck_assert_str_eq("TXA ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_txs)
{
	char ins[4] = "TXS";
	uint8_t txs_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[txs_opcode](cpu);

	ck_assert_str_eq("TXS ", cpu->instruction);
}
END_TEST

START_TEST (log_correct_instruction_mnemonic_tya)
{
	char ins[4] = "TYA";
	uint8_t tya_opcode  = reverse_opcode_lut(&ins, IMP);

	execute_opcode_lut[tya_opcode](cpu);

	ck_assert_str_eq("TYA ", cpu->instruction);
}
END_TEST


START_TEST (abs_store_only_writes_on_last_cycle)
{
	char ins[3][4] = { "STA", "STX", "STY" };
	uint8_t store_opcodes[3] = { reverse_opcode_lut(&ins[0], ABS)
	                           , reverse_opcode_lut(&ins[1], ABS)
	                           , reverse_opcode_lut(&ins[2], ABS)};
	cpu->instruction_cycles_remaining = 1;
	cpu->operand = 0xFF; // should remain unchanged
	// target address is (addr_hi | addr_lo)
	cpu->addr_hi = 0x58;
	cpu->addr_lo = 0xC8;
	cpu->A = 0x32;
	cpu->X = 0x32;
	cpu->Y = 0x32;

	decode_opcode_lut[store_opcodes[_i]](cpu);
	execute_opcode_lut[store_opcodes[_i]](cpu);

	// verify a read didn't happen
	ck_assert_uint_eq(0xFF, cpu->operand);
	// verify a write occured
	ck_assert_uint_eq(0x32, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (absx_store_only_writes_on_last_cycle)
{
	// STX/STY don't support this address mode
	char ins[4] = "STA";
	uint8_t sta_opcodes[3] = { reverse_opcode_lut(&ins, ABSX)
	                         , reverse_opcode_lut(&ins, ABSX)
	                         , reverse_opcode_lut(&ins, ABSX)};
	cpu->instruction_cycles_remaining = 1;
	cpu->operand = 0xFF; // should remain unchanged
	// target address is (addr_hi | addr_lo)
	cpu->addr_hi = 0x68;
	cpu->addr_lo = 0x00;
	cpu->A = 0x91;
	cpu->X = 0x91;
	cpu->Y = 0x91;

	decode_opcode_lut[sta_opcodes[_i]](cpu);
	execute_opcode_lut[sta_opcodes[_i]](cpu);

	// verify a read didn't happen
	ck_assert_uint_eq(0xFF, cpu->operand);
	// verify a write occured
	ck_assert_uint_eq(0x91, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (absy_store_only_writes_on_last_cycle)
{
	// STX/STY don't support this address mode
	char ins[4] = "STA";
	uint8_t sta_opcodes[3] = { reverse_opcode_lut(&ins, ABSY)
	                         , reverse_opcode_lut(&ins, ABSY)
	                         , reverse_opcode_lut(&ins, ABSY)};
	cpu->instruction_cycles_remaining = 1;
	cpu->operand = 0xFF; // should remain unchanged
	// target address is (addr_hi | addr_lo)
	cpu->addr_hi = 0x10;
	cpu->addr_lo = 0x15;
	cpu->A = 0xC7;
	cpu->X = 0xC7;
	cpu->Y = 0xC7;

	decode_opcode_lut[sta_opcodes[_i]](cpu);
	execute_opcode_lut[sta_opcodes[_i]](cpu);

	// verify a read didn't happen
	ck_assert_uint_eq(0xFF, cpu->operand);
	// verify a write occured
	ck_assert_uint_eq(0xC7, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (indx_store_only_writes_on_last_cycle)
{
	// STX/STY don't support this address mode
	char ins[4] = "STA";
	uint8_t sta_opcodes[3] = { reverse_opcode_lut(&ins, INDX)
	                         , reverse_opcode_lut(&ins, INDX)
	                         , reverse_opcode_lut(&ins, INDX)};
	cpu->instruction_cycles_remaining = 1;
	cpu->operand = 0xFF; // should remain unchanged
	// target address is (addr_hi | addr_lo)
	cpu->addr_hi = 0x00;
	cpu->addr_lo = 0x08;
	cpu->A = 0x62;
	cpu->X = 0x62;
	cpu->Y = 0x62;

	decode_opcode_lut[sta_opcodes[_i]](cpu);
	execute_opcode_lut[sta_opcodes[_i]](cpu);

	// verify a read didn't happen
	ck_assert_uint_eq(0xFF, cpu->operand);
	// verify a write occured
	ck_assert_uint_eq(0x62, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (indy_store_only_writes_on_last_cycle)
{
	// STX/STY don't support this address mode
	char ins[4] = "STA";
	uint8_t sta_opcodes[3] = { reverse_opcode_lut(&ins, INDY)
	                         , reverse_opcode_lut(&ins, INDY)
	                         , reverse_opcode_lut(&ins, INDY)};
	cpu->instruction_cycles_remaining = 1;
	cpu->operand = 0xFF; // should remain unchanged
	// target address is (addr_hi | addr_lo)
	cpu->addr_hi = 0x0B;
	cpu->addr_lo = 0xC8;
	cpu->A = 0x77;
	cpu->X = 0x77;
	cpu->Y = 0x77;

	decode_opcode_lut[sta_opcodes[_i]](cpu);
	execute_opcode_lut[sta_opcodes[_i]](cpu);

	// verify a read didn't happen
	ck_assert_uint_eq(0xFF, cpu->operand);
	// verify a write occured
	ck_assert_uint_eq(0x77, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (zp_store_only_writes_on_last_cycle)
{
	char ins[3][4] = { "STA", "STX", "STY" };
	uint8_t store_opcodes[3] = { reverse_opcode_lut(&ins[0], ZP)
	                           , reverse_opcode_lut(&ins[1], ZP)
	                           , reverse_opcode_lut(&ins[2], ZP)};
	cpu->instruction_cycles_remaining = 1;
	cpu->operand = 0xFF; // should remain unchanged
	// target address is (addr_lo)
	cpu->addr_lo = 0xDC;
	cpu->A = 0x14;
	cpu->X = 0x14;
	cpu->Y = 0x14;

	decode_opcode_lut[store_opcodes[_i]](cpu);
	execute_opcode_lut[store_opcodes[_i]](cpu);

	// verify a read didn't happen
	ck_assert_uint_eq(0xFF, cpu->operand);
	// verify a write occured
	ck_assert_uint_eq(0x14, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (zpx_store_only_writes_on_last_cycle)
{
	char ins[2][4] = { "STA", "STY" };
	uint8_t store_opcodes[2] = { reverse_opcode_lut(&ins[0], ZPX)
	                           , reverse_opcode_lut(&ins[1], ZPX)};
	cpu->instruction_cycles_remaining = 1;
	cpu->operand = 0xFF; // should remain unchanged
	// target address is (addr_lo + X)
	cpu->addr_lo = 0xA0;
	cpu->A = 0x22;
	cpu->X = 0x0A;
	cpu->Y = 0x22;

	decode_opcode_lut[store_opcodes[_i]](cpu);
	execute_opcode_lut[store_opcodes[_i]](cpu);

	// verify a read didn't happen
	ck_assert_uint_eq(0xFF, cpu->operand);
	// verify a write occured
	ck_assert_uint_eq(0x22, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (zpy_store_only_writes_on_last_cycle)
{
	char ins[4] = "STX";
	uint8_t stx_opcode = reverse_opcode_lut(&ins, ZPY);
	cpu->instruction_cycles_remaining = 1;
	cpu->operand = 0xFF; // should remain unchanged
	// target address is (addr_lo + Y)
	cpu->addr_lo = 0x41;
	cpu->X = 0x83;
	cpu->Y = 0x01;

	decode_opcode_lut[stx_opcode](cpu);
	execute_opcode_lut[stx_opcode](cpu);

	// verify a read didn't happen
	ck_assert_uint_eq(0xFF, cpu->operand);
	// verify a write occured
	ck_assert_uint_eq(0x83, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST


START_TEST (abs_read_store_t1)
{
	char ins[4] = "EOR";
	cpu->instruction_cycles_remaining = 3;
	cpu->PC = 0x003B;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x81);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABS)](cpu);

	// addr_lo should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x81, cpu->addr_lo);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (abs_read_store_t2)
{
	char ins[4] = "EOR";
	cpu->instruction_cycles_remaining = 2;
	cpu->PC = 0x003C;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x05);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABS)](cpu);

	// addr_hi should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x05, cpu->addr_hi);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (abs_read_store_t3)
{
	char ins[4] = "EOR";
	cpu->instruction_cycles_remaining = 1;
	cpu->addr_hi = 0x05;
	cpu->addr_lo = 0x81;

	decode_opcode_lut[reverse_opcode_lut(&ins, ABS)](cpu);

	// target_addr should be a concat of addr_hi and addr_lo
	ck_assert_uint_eq(0x0581, cpu->target_addr);
}
END_TEST

START_TEST (abs_rmw_t1)
{
	char ins[4] = "DEC";
	cpu->instruction_cycles_remaining = 5;
	cpu->PC = 0x0035;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x51);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABS)](cpu);

	// addr_lo should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x51, cpu->addr_lo);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (abs_rmw_t2)
{
	char ins[4] = "DEC";
	cpu->instruction_cycles_remaining = 4;
	cpu->PC = 0x0036;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x13);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABS)](cpu);

	// addr_hi should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x13, cpu->addr_hi);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (abs_rmw_t3_dummy_read)
{
	char ins[4] = "DEC";
	cpu->instruction_cycles_remaining = 3;
	cpu->addr_hi = 0x13;
	cpu->addr_lo = 0x51;
	write_to_cpu(cpu, 0x1351, 0x10);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABS)](cpu);

	// target_addr is the absolute address (addr_hi | addr_lo)
	// a read occurs at that address too (dummy read)
	ck_assert_uint_eq(0x1351, cpu->target_addr);
	ck_assert_uint_eq(0x10, cpu->unmodified_data);
}
END_TEST

START_TEST (abs_rmw_t4_dummy_write)
{
	char ins[4] = "DEC";
	cpu->instruction_cycles_remaining = 2;
	cpu->target_addr = 0x1351;
	cpu->unmodified_data = 0xF3;

	decode_opcode_lut[reverse_opcode_lut(&ins, ABS)](cpu);

	// read from T3 is written back to target_addr here
	ck_assert_uint_eq(0xF3, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (absx_read_store_t1)
{
	char ins[4] = "LDA";
	cpu->instruction_cycles_remaining = 4;
	cpu->PC = 0x004B;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x52);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSX)](cpu);

	// addr_lo should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x52, cpu->addr_lo);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (absx_read_store_t2)
{
	char ins[4] = "LDA";
	cpu->instruction_cycles_remaining = 3;
	cpu->PC = 0x004C;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x11);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSX)](cpu);

	// addr_hi should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x11, cpu->addr_hi);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (absx_read_store_t3_no_page_cross)
{
	char ins[4] = "LDA";
	cpu->instruction_cycles_remaining = 2;
	cpu->addr_hi = 0x11;
	cpu->addr_lo = 0x52;
	cpu->X = 0;

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSX)](cpu);

	// target_addr should be a concat of addr_hi and addr_lo
	ck_assert_uint_eq(0x1152, cpu->target_addr);
}
END_TEST

START_TEST (absx_read_store_t3_page_cross)
{
	char ins[4] = "LDA";
	cpu->instruction_cycles_remaining = 2;
	cpu->addr_hi = 0x11;
	cpu->addr_lo = 0x52;
	cpu->X = 0xFF;
	write_to_cpu(cpu, 0x1152 + cpu->X - 0x0100, 0xB5); // minus the page cross

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSX)](cpu);

	// target_addr should be a concat of addr_hi and addr_lo
	ck_assert_uint_eq(0x1152 + cpu->X - 0x0100, cpu->target_addr);
	// page cross means a dummy read at the incorrect (non-paged crossed) address occurs here
	ck_assert_uint_eq(cpu->operand, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (absx_read_store_t4)
{
	char ins[4] = "LDA";
	cpu->instruction_cycles_remaining = 1;
	cpu->addr_hi = 0x11;
	cpu->addr_lo = 0x52;
	cpu->X = 0xFF;

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSX)](cpu);

	// target_addr should be a concat of addr_hi and addr_lo
	ck_assert_uint_eq(0x1152 + cpu->X, cpu->target_addr);
}
END_TEST

START_TEST (absx_rmw_t1)
{
	char ins[4] = "DEC";
	cpu->instruction_cycles_remaining = 6;
	cpu->PC = 0x0075;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x27);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSX)](cpu);

	// addr_lo should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x27, cpu->addr_lo);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (absx_rmw_t2)
{
	char ins[4] = "DEC";
	cpu->instruction_cycles_remaining = 5;
	cpu->PC = 0x0056;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x17);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSX)](cpu);

	// addr_hi should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x17, cpu->addr_hi);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (absx_rmw_t4_2nd_absx_read)
{
	char ins[4] = "DEC";
	cpu->instruction_cycles_remaining = 3;
	cpu->addr_hi = 0x17;
	cpu->addr_lo = 0x27;
	// actual read from page-crossed address if needed
	// otherwise the T3 read was in fact the correct read
	cpu->X = 0x51;
	write_to_cpu(cpu, 0x1727 + cpu->X, 0x20);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSX)](cpu);

	// target_addr is the absolute address plus the X register
	// a read occurs at that address too
	ck_assert_uint_eq(0x1727 + cpu->X, cpu->target_addr);
	ck_assert_uint_eq(0x20, cpu->unmodified_data);
}
END_TEST

START_TEST (absx_rmw_t5_dummy_write)
{
	char ins[4] = "DEC";
	cpu->instruction_cycles_remaining = 2;
	cpu->target_addr = 0x1351;
	cpu->unmodified_data = 0xF3;

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSX)](cpu);

	// read from T4 is written back to target_addr here
	ck_assert_uint_eq(0xF3, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (absy_read_store_t1)
{
	char ins[4] = "ORA";
	cpu->instruction_cycles_remaining = 4;
	cpu->PC = 0x004B;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x05);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSY)](cpu);

	// addr_lo should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x05, cpu->addr_lo);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (absy_read_store_t2)
{
	char ins[4] = "ORA";
	cpu->instruction_cycles_remaining = 3;
	cpu->PC = 0x004C;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x18);

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSY)](cpu);

	// addr_hi should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x18, cpu->addr_hi);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (absy_read_store_t3_no_page_cross)
{
	char ins[4] = "ORA";
	cpu->instruction_cycles_remaining = 2;
	cpu->addr_hi = 0x18;
	cpu->addr_lo = 0x05;
	cpu->Y = 0;

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSY)](cpu);

	// target_addr should be a concat of addr_hi and addr_lo
	ck_assert_uint_eq(0x1805, cpu->target_addr);
}
END_TEST

START_TEST (absy_read_store_t3_page_cross)
{
	char ins[4] = "ORA";
	cpu->instruction_cycles_remaining = 2;
	cpu->addr_hi = 0x18;
	cpu->addr_lo = 0x05;
	cpu->Y = 0xFF;
	write_to_cpu(cpu, 0x1805 + cpu->Y - 0x0100, 0x35); // minus the page cross

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSY)](cpu);

	// target_addr should be a concat of addr_hi and addr_lo
	ck_assert_uint_eq(0x1805 + cpu->Y - 0x0100, cpu->target_addr);
	// page cross means a dummy read at the incorrect (non-paged crossed) address occurs here
	ck_assert_uint_eq(cpu->operand, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (absy_read_store_t4)
{
	char ins[4] = "ORA";
	cpu->instruction_cycles_remaining = 1;
	cpu->addr_hi = 0x18;
	cpu->addr_lo = 0x05;
	cpu->Y = 0xFF;

	decode_opcode_lut[reverse_opcode_lut(&ins, ABSY)](cpu);

	// target_addr should be a concat of addr_hi and addr_lo
	ck_assert_uint_eq(0x1805 + cpu->Y, cpu->target_addr);
}
END_TEST

START_TEST (indx_read_store_t1)
{
	char ins[4] = "CMP";
	cpu->instruction_cycles_remaining = 5;
	cpu->PC = 0x00B7;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x0C);

	decode_opcode_lut[reverse_opcode_lut(&ins, INDX)](cpu);

	// base_addr should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x0C, cpu->base_addr);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (indx_read_store_t2_dummy_read)
{
	char ins[4] = "CMP";
	cpu->instruction_cycles_remaining = 4;
	cpu->base_addr = 0x0C;
	write_to_cpu(cpu, cpu->base_addr, 0x46);

	decode_opcode_lut[reverse_opcode_lut(&ins, INDX)](cpu);

	// addr_lo should be set by reading from the base_addr
	ck_assert_uint_eq(0x46, cpu->addr_lo);
}
END_TEST

START_TEST (indx_read_store_t3)
{
	char ins[4] = "CMP";
	cpu->instruction_cycles_remaining = 3;
	cpu->base_addr = 0x0C;
	cpu->X = 0x02;
	write_to_cpu(cpu, cpu->base_addr + cpu->X, 0x48);

	decode_opcode_lut[reverse_opcode_lut(&ins, INDX)](cpu);

	// addr_lo should be set by reading from the zero page address
	// of base_addr + X register
	ck_assert_uint_eq(0x48, cpu->addr_lo);
}
END_TEST

START_TEST (indx_read_store_t4)
{
	char ins[4] = "CMP";
	cpu->instruction_cycles_remaining = 2;
	cpu->base_addr = 0x0C;
	cpu->X = 0x02;
	write_to_cpu(cpu, cpu->base_addr + cpu->X + 1, 0x19);

	decode_opcode_lut[reverse_opcode_lut(&ins, INDX)](cpu);

	// addr_hi should be set by reading from the zero page address
	// of base_addr + X register + 1
	ck_assert_uint_eq(0x19, cpu->addr_hi);
}
END_TEST

START_TEST (indx_read_store_t5)
{
	char ins[4] = "CMP";
	cpu->instruction_cycles_remaining = 1;
	cpu->addr_hi = 0x19;
	cpu->addr_lo = 0x48;

	decode_opcode_lut[reverse_opcode_lut(&ins, INDX)](cpu);

	// target_addr should be a concat of addr_hi and addr_lo
	ck_assert_uint_eq(0x1948, cpu->target_addr);
}
END_TEST

START_TEST (indy_read_store_t1)
{
	char ins[4] = "ADC";
	cpu->instruction_cycles_remaining = 5;
	cpu->PC = 0x0013;
	write_to_cpu(cpu, cpu->PC, 0x20);

	decode_opcode_lut[reverse_opcode_lut(&ins, INDY)](cpu);

	// base_addr should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x20, cpu->base_addr);
}
END_TEST

START_TEST (indy_read_store_t2)
{
	char ins[4] = "ADC";
	cpu->instruction_cycles_remaining = 4;
	cpu->base_addr = 0x0020;
	write_to_cpu(cpu, cpu->base_addr, 0xFE);

	decode_opcode_lut[reverse_opcode_lut(&ins, INDY)](cpu);

	// addr_lo should be set by reading from the zero page address
	// of base_addr
	ck_assert_uint_eq(0xFE, cpu->addr_lo);
}
END_TEST

START_TEST (indy_read_store_t3)
{
	char ins[4] = "ADC";
	cpu->instruction_cycles_remaining = 3;
	cpu->base_addr = 0x0020;
	write_to_cpu(cpu, cpu->base_addr + 1, 0x03);

	decode_opcode_lut[reverse_opcode_lut(&ins, INDY)](cpu);

	// addr_hi should be set by reading from the zero page address
	// of base_addr + 1
	ck_assert_uint_eq(0x03, cpu->addr_hi);
}
END_TEST

START_TEST (indy_read_store_t4_no_page_cross)
{
	// This is a dummy read if T5 is executed
	char ins[4] = "ADC";
	cpu->instruction_cycles_remaining = 2;
	cpu->addr_hi = 0x03;
	cpu->addr_lo = 0xFE;
	cpu->Y = 1;

	decode_opcode_lut[reverse_opcode_lut(&ins, INDY)](cpu);

	// target_addr should be a concat of addr_hi and addr_lo
	// Add Y register offset to the traget_addr
	// With this calc being the non-page cross address
	// (adding Y to addr_lo w/o adding a potentital carry to addr_hi)
	ck_assert_uint_eq(0x03FE + cpu->Y, cpu->target_addr);
}
END_TEST

START_TEST (indy_read_store_t4_page_cross)
{
	// This is a dummy read if T5 is executed
	char ins[4] = "ADC";
	cpu->instruction_cycles_remaining = 2;
	cpu->addr_hi = 0x03;
	cpu->addr_lo = 0xFE;
	cpu->Y = 18;
	write_to_cpu(cpu, 0x03FE + cpu->Y - 0x0100, 0xE5);

	decode_opcode_lut[reverse_opcode_lut(&ins, INDY)](cpu);

	// target_addr should be a concat of addr_hi and addr_lo
	// Add Y register offset to the traget_addr
	// With this calc being the non-page cross address
	// (adding Y to addr_lo w/o adding a potentital carry to addr_hi)
	ck_assert_uint_eq(0x03FE + cpu->Y - 0x0100, cpu->target_addr); // minus page cross
	// page cross means a dummy read at the incorrect (non-paged crossed) address occurs here
	ck_assert_uint_eq(cpu->operand, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (indy_read_store_t5)
{
	char ins[4] = "ADC";
	cpu->instruction_cycles_remaining = 2;
	cpu->addr_hi = 0x03;
	cpu->addr_lo = 0xFE;
	cpu->Y = 1;

	decode_opcode_lut[reverse_opcode_lut(&ins, INDY)](cpu);

	// target_addr should be a concat of addr_hi and addr_lo
	// A carry is added to addr_hi here if needed
	// (when addr_lo + Y crosses a page)
	ck_assert_uint_eq(0x03FE + cpu->Y, cpu->target_addr);
}
END_TEST

START_TEST (zp_read_store_t1)
{
	char ins[4] = "BIT";
	cpu->instruction_cycles_remaining = 2;
	cpu->PC = 0x06D1;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x90);

	decode_opcode_lut[reverse_opcode_lut(&ins, ZP)](cpu);

	// addr_lo should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x90, cpu->addr_lo);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (zp_read_store_t2)
{
	char ins[4] = "BIT";
	cpu->instruction_cycles_remaining = 1;
	cpu->addr_lo = 0x90;

	decode_opcode_lut[reverse_opcode_lut(&ins, ZP)](cpu);

	// target_addr is just the zero page address of addr_lo
	ck_assert_uint_eq(0x90, cpu->target_addr);
}
END_TEST

START_TEST (zp_rmw_t1)
{
	char ins[4] = "ROR";
	cpu->instruction_cycles_remaining = 4;
	cpu->PC = 0x0601;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x94);

	decode_opcode_lut[reverse_opcode_lut(&ins, ZP)](cpu);

	// addr_lo should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x94, cpu->addr_lo);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (zp_rmw_t2)
{
	char ins[4] = "ROR";
	cpu->instruction_cycles_remaining = 3;
	cpu->addr_lo = 0x94;
	write_to_cpu(cpu, cpu->addr_lo, 0xA8);

	decode_opcode_lut[reverse_opcode_lut(&ins, ZP)](cpu);

	// target_addr is just the zero page address of addr_lo
	// a read occurs at that address too
	ck_assert_uint_eq(0x94, cpu->target_addr);
	ck_assert_uint_eq(0xA8, cpu->unmodified_data);
}
END_TEST

START_TEST (zp_rmw_t3_dummy_write)
{
	char ins[4] = "ROR";
	cpu->instruction_cycles_remaining = 2;
	cpu->target_addr = 0x94;
	cpu->unmodified_data = 0xA8;

	decode_opcode_lut[reverse_opcode_lut(&ins, ZP)](cpu);

	// read from T3 is written back to target_addr here
	ck_assert_uint_eq(0xA8, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (zpx_read_store_t1)
{
	char ins[4] = "AND";
	cpu->instruction_cycles_remaining = 3;
	cpu->PC = 0x0081;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0x83);

	decode_opcode_lut[reverse_opcode_lut(&ins, ZPX)](cpu);

	// addr_lo should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0x83, cpu->addr_lo);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (zpx_read_store_t3)
{
	char ins[4] = "AND";
	cpu->instruction_cycles_remaining = 1;
	cpu->addr_lo = 0x83;
	cpu->X = 0x21;

	decode_opcode_lut[reverse_opcode_lut(&ins, ZPX)](cpu);

	// target_addr is just the zero page address of addr_lo + X register
	ck_assert_uint_eq((uint8_t) (cpu->addr_lo + cpu->X), cpu->target_addr);
}
END_TEST

START_TEST (zpx_rmw_t1)
{
	char ins[4] = "ROL";
	cpu->instruction_cycles_remaining = 5;
	cpu->PC = 0x0601;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0xB4);

	decode_opcode_lut[reverse_opcode_lut(&ins, ZPX)](cpu);

	// addr_lo should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0xB4, cpu->addr_lo);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (zpx_rmw_t2_dummy_read)
{
	char ins[4] = "ROL";
	cpu->instruction_cycles_remaining = 4;
	cpu->addr_lo = 0xB4;
	write_to_cpu(cpu, cpu->addr_lo, 0x09);

	decode_opcode_lut[reverse_opcode_lut(&ins, ZPX)](cpu);

	// target_addr is just the zero page address of addr_lo
	// a read occurs at that address too
	ck_assert_uint_eq(0xB4, cpu->target_addr);
	ck_assert_uint_eq(0x09, cpu->unmodified_data);
}
END_TEST

START_TEST (zpx_rmw_t3)
{
	char ins[4] = "ROL";
	cpu->instruction_cycles_remaining = 3;
	cpu->addr_lo = 0xB4;
	cpu->X = 0x01;
	write_to_cpu(cpu, (uint8_t) (cpu->addr_lo + cpu->X), 0x92);

	decode_opcode_lut[reverse_opcode_lut(&ins, ZPX)](cpu);

	// target_addr is just the zero page address of addr_lo + X register
	// a read occurs here too
	ck_assert_uint_eq((uint8_t) (cpu->addr_lo + cpu->X), cpu->target_addr);
	ck_assert_uint_eq(0x92, cpu->unmodified_data);
}
END_TEST

START_TEST (zpx_rmw_t4_dummy_write)
{
	char ins[4] = "ROL";
	cpu->instruction_cycles_remaining = 2;
	cpu->target_addr = 0x94;
	cpu->unmodified_data = 0x92;

	decode_opcode_lut[reverse_opcode_lut(&ins, ZPX)](cpu);

	// read from T3 is written back to target_addr here
	ck_assert_uint_eq(0x92, read_from_cpu(cpu, cpu->target_addr));
}
END_TEST

START_TEST (zpy_read_store_t1)
{
	char ins[4] = "LDX";
	cpu->instruction_cycles_remaining = 3;
	cpu->PC = 0x00E1;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 0xE3);

	decode_opcode_lut[reverse_opcode_lut(&ins, ZPY)](cpu);

	// addr_lo should be set by reading from the PC
	// PC should then be incremented
	ck_assert_uint_eq(0xE3, cpu->addr_lo);
	ck_assert_uint_eq(start_PC + 1, cpu->PC);
}
END_TEST

START_TEST (zpy_read_store_t3)
{
	char ins[4] = "LDX";
	cpu->instruction_cycles_remaining = 1;
	cpu->addr_lo = 0xE3;
	cpu->Y = 0x21;

	decode_opcode_lut[reverse_opcode_lut(&ins, ZPY)](cpu);

	// target_addr is just the zero page address of addr_lo + Y register
	ck_assert_uint_eq((uint8_t) (cpu->addr_lo + cpu->Y), cpu->target_addr);
}
END_TEST

START_TEST (branch_ops_t1_branch_not_taken)
{
	char ins[4] = "BCS";
	cpu->instruction_cycles_remaining = 3;
	cpu->P = ~FLAG_C;
	cpu->PC = 0x0100;
	uint16_t start_PC = cpu->PC;
	write_to_cpu(cpu, cpu->PC, 8); // offset of +8
	uint8_t opcode = reverse_opcode_lut(&ins, REL);
	cpu->opcode = opcode; // needed for branch_not_taken() function

	decode_opcode_lut[opcode](cpu);

	ck_assert_uint_eq(start_PC + 1, cpu->target_addr);
}
END_TEST

START_TEST (branch_ops_t2_branch_taken_no_page_cross_pending)
{
	char ins[4] = "BCS";
	cpu->instruction_cycles_remaining = 2;
	cpu->P = FLAG_C;
	cpu->PC = 0x0100;
	cpu->offset = 8;

	decode_opcode_lut[reverse_opcode_lut(&ins, REL)](cpu);

	ck_assert_uint_eq(cpu->PC + cpu->offset, cpu->target_addr);
}
END_TEST

START_TEST (branch_ops_t2_branch_taken_page_cross_pending)
{
	char ins[4] = "BCS";
	cpu->instruction_cycles_remaining = 2;
	cpu->P = FLAG_C;
	cpu->PC = 0x01FF;
	cpu->offset = 8;

	decode_opcode_lut[reverse_opcode_lut(&ins, REL)](cpu);

	// minus the page cross, emulates [PCH | (uint8_t) (PCL + offset)]
	ck_assert_uint_eq(cpu->PC + cpu->offset - 0x0100, cpu->target_addr);
}
END_TEST

START_TEST (branch_ops_t3_branch_taken_page_cross)
{
	char ins[4] = "BCS";
	cpu->instruction_cycles_remaining = 1;
	cpu->P = FLAG_C;
	cpu->PC = 0x01FF;
	cpu->offset = 8;

	decode_opcode_lut[reverse_opcode_lut(&ins, REL)](cpu);

	// here the correct page cross address is used
	ck_assert_uint_eq(cpu->PC + cpu->offset, cpu->target_addr);
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
	TCase* tc_cpu_hardware_interrupts;
	TCase* tc_cpu_instruction_trace_logger;
	TCase* tc_cpu_address_modes_rw_logic;
	TCase* tc_cpu_address_modes_cycles;

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
	tcase_add_test(tc_address_modes, addr_mode_absx_read_store_page_cross);
	tcase_add_test(tc_address_modes, addr_mode_absx_read_store_STx_no_page_cross);
	tcase_add_test(tc_address_modes, addr_mode_absx_rmw);
	tcase_add_test(tc_address_modes, addr_mode_absy_read_store);
	tcase_add_test(tc_address_modes, addr_mode_absy_read_store_page_cross);
	tcase_add_test(tc_address_modes, addr_mode_absy_read_store_STx_no_page_cross);
	tcase_add_test(tc_address_modes, addr_mode_zp_read_store);
	tcase_add_test(tc_address_modes, addr_mode_zp_rmw);
	tcase_add_test(tc_address_modes, addr_mode_zpx_read_store);
	tcase_add_test(tc_address_modes, addr_mode_zpx_read_store_page_cross);
	tcase_add_test(tc_address_modes, addr_mode_zpx_rmw);
	tcase_add_test(tc_address_modes, addr_mode_zpy_read_store);
	tcase_add_test(tc_address_modes, addr_mode_zpy_read_store_page_cross);
	tcase_add_test(tc_address_modes, ind_jmp);
	tcase_add_test(tc_address_modes, ind_jmp_bug);
	tcase_add_test(tc_address_modes, addr_mode_indx_read_store);
	tcase_add_test(tc_address_modes, addr_mode_indy_read_store);
	tcase_add_test(tc_address_modes, addr_mode_indy_read_store_page_cross);
	tcase_add_test(tc_address_modes, addr_mode_indy_read_store_STx_no_page_cross);
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
	tcase_add_test(tc_cpu_reads, generic_read_x_reg);
	tcase_add_test(tc_cpu_reads, generic_read_y_reg);
	tcase_add_test(tc_cpu_reads, generic_read_a_reg);
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
	tc_cpu_isa = tcase_create("Cpu Instruction Set Architecture Results & Flag Updates");
	tcase_add_checked_fixture(tc_cpu_isa, setup, teardown);
	tcase_add_test(tc_cpu_isa, isa_lda_imm_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_lda_non_imm_mode_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_lda_imm_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_lda_imm_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_ldx_imm_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_ldx_non_imm_mode_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_ldx_non_imm_mode_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_ldx_non_imm_mode_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_ldy_imm_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_ldy_non_imm_mode_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_ldy_imm_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_ldy_imm_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_sta_result_only);
	tcase_add_test(tc_cpu_isa, isa_stx_result_only);
	tcase_add_test(tc_cpu_isa, isa_sty_result_only);
	tcase_add_test(tc_cpu_isa, isa_tax_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_tax_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_tax_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_tay_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_tay_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_tay_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_tsx_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_tsx_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_tsx_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_txa_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_txa_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_txa_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_txs_result_only);
	tcase_add_test(tc_cpu_isa, isa_tya_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_tya_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_tya_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_adc_imm_no_carry_in_clear_nvzc_flags);
	tcase_add_test(tc_cpu_isa, isa_adc_imm_no_carry_in_set_c_clear_nvz_flags);
	tcase_add_test(tc_cpu_isa, isa_adc_imm_no_carry_in_set_zc_clear_nv_flags);
	tcase_add_test(tc_cpu_isa, isa_adc_imm_no_carry_in_set_vzc_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_adc_imm_carry_in_set_nc_clear_vz_flags);
	tcase_add_test(tc_cpu_isa, isa_adc_imm_carry_in_set_nv_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_adc_non_imm_mode_carry_in_set_nv_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_dec_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_dec_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_dec_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_dex_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_dex_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_dex_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_dey_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_dey_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_dey_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_inc_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_inc_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_inc_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_inx_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_inx_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_inx_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_iny_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_iny_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_iny_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_sbc_imm_carry_in_set_c_clear_nvz_flags);
	tcase_add_test(tc_cpu_isa, isa_sbc_imm_no_carry_in_set_nv_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_sbc_imm_no_carry_in_set_n_clear_vzc_flags);
	tcase_add_test(tc_cpu_isa, isa_sbc_non_imm_mode_no_carry_in_set_n_clear_vzc_flags);
	tcase_add_test(tc_cpu_isa, isa_sbc_non_imm_mode_carry_in_set_vzc_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_sbc_non_imm_mode_no_carry_in_set_zc_clear_nv_flags);
	tcase_add_test(tc_cpu_isa, isa_and_imm_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_and_non_imm_mode_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_and_non_imm_mode_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_and_non_imm_mode_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_asl_acc_clear_nzc_flags);
	tcase_add_test(tc_cpu_isa, isa_asl_acc_set_n_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_asl_non_acc_mode_set_n_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_asl_acc_set_z_clear_nc_flags);
	tcase_add_test(tc_cpu_isa, isa_asl_acc_set_c_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_asl_acc_set_zc_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_asl_acc_set_nc_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_bit_clear_nvz_flags);
	tcase_add_test(tc_cpu_isa, isa_bit_set_nvz_flags);
	tcase_add_test(tc_cpu_isa, isa_bit_set_vn_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_bit_set_v_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_bit_set_n_clear_vz_flags);
	tcase_add_test(tc_cpu_isa, isa_bit_set_z_clear_nv_flags);
	tcase_add_test(tc_cpu_isa, isa_eor_imm_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_eor_non_imm_mode_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_eor_non_imm_mode_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_eor_non_imm_mode_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_lsr_acc_clear_nzc_flags);
	tcase_add_test(tc_cpu_isa, isa_lsr_non_acc_mode_clear_nzc_flags);
	tcase_add_test(tc_cpu_isa, isa_lsr_non_acc_mode_set_z_clear_nc_flags);
	tcase_add_test(tc_cpu_isa, isa_lsr_non_acc_mode_set_c_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_lsr_non_acc_mode_set_zc_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_ora_imm_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_ora_non_imm_mode_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_ora_non_imm_mode_set_n_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_ora_non_imm_mode_set_z_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_rol_acc_no_carry_in_clear_nzc_flags);
	tcase_add_test(tc_cpu_isa, isa_rol_acc_no_carry_in_set_n_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_rol_non_acc_mode_no_carry_in_set_n_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_rol_non_acc_mode_no_carry_in_set_z_clear_nc_flags);
	tcase_add_test(tc_cpu_isa, isa_rol_non_acc_mode_no_carry_in_set_zc_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_rol_non_acc_mode_carry_in_set_nc_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_rol_acc_carry_in_set_n_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_ror_acc_no_carry_in_clear_nzc_flags);
	tcase_add_test(tc_cpu_isa, isa_ror_acc_mode_no_carry_in_clear_nzc_flags);
	tcase_add_test(tc_cpu_isa, isa_ror_acc_mode_no_carry_in_set_z_clear_nc_flags);
	tcase_add_test(tc_cpu_isa, isa_ror_acc_mode_no_carry_in_set_zc_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_ror_acc_carry_in_set_n_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_ror_acc_carry_in_set_nc_clear_z_flags);
	tcase_add_test(tc_cpu_isa, isa_jsr_result_only);
	tcase_add_test(tc_cpu_isa, isa_rti_result_only);
	tcase_add_test(tc_cpu_isa, isa_rts_result_only);
	tcase_add_test(tc_cpu_isa, isa_clc_result_only);
	tcase_add_test(tc_cpu_isa, isa_cld_result_only);
	tcase_add_test(tc_cpu_isa, isa_cli_result_only);
	tcase_add_test(tc_cpu_isa, isa_clv_result_only);
	tcase_add_test(tc_cpu_isa, isa_cmp_set_n_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_cmp_set_zc_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_cmp_set_c_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_cpx_set_n_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_cpx_set_zc_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_cpx_set_c_clear_nz_flags);
	tcase_add_test(tc_cpu_isa, isa_cpy_set_n_clear_zc_flags);
	tcase_add_test(tc_cpu_isa, isa_cpy_set_zc_clear_n_flags);
	tcase_add_test(tc_cpu_isa, isa_cpy_set_c_clear_nz_flags);
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
	tc_cpu_hardware_interrupts = tcase_create("Cpu Hardware Interrupts (no opcodes e.g. IRQ)");
	tcase_add_checked_fixture(tc_cpu_hardware_interrupts, setup, teardown);
	tcase_add_test(tc_cpu_hardware_interrupts, irq_correct_interrupt_vector);
	suite_add_tcase(s, tc_cpu_hardware_interrupts);
	tc_cpu_instruction_trace_logger = tcase_create("Cpu Instruction Trace Logger");
	tcase_add_checked_fixture(tc_cpu_instruction_trace_logger, setup, teardown);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_adc, 0, 8);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_and, 0, 8);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_asl, 0, 5);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_bcc);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_bcs);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_beq);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_bit, 0, 2);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_bmi);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_bne);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_bpl);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_brk);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_bvc);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_bvs);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_clc);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_cld);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_cli);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_clv);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_cmp, 0, 8);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_cpx, 0, 3);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_cpy, 0, 3);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_dec, 0, 4);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_dex);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_dey);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_eor, 0, 8);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_inc, 0, 4);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_inx);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_iny);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_jmp, 0, 2);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_and_address_jsr);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_lda, 0, 8);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_ldx, 0, 5);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_ldy, 0, 5);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_lsr, 0, 5);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_nop);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_ora, 0, 8);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_pha);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_php);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_pla);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_plp);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_rol, 0, 5);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_ror, 0, 5);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_rti);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_rts);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_sbc, 0, 8);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_sec);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_sed);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_sei);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_sta, 0, 7);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_stx, 0, 3);
	tcase_add_loop_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_sty, 0, 3);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_tax);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_tay);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_tsx);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_txa);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_txs);
	tcase_add_test(tc_cpu_instruction_trace_logger, log_correct_instruction_mnemonic_tya);
	suite_add_tcase(s, tc_cpu_instruction_trace_logger);
	tc_cpu_address_modes_rw_logic = tcase_create("Cpu Address Modes R/W Logic");
	tcase_add_checked_fixture(tc_cpu_address_modes_rw_logic, setup, teardown);
	tcase_add_loop_test(tc_cpu_address_modes_rw_logic, abs_store_only_writes_on_last_cycle, 0, 3);
	tcase_add_test(tc_cpu_address_modes_rw_logic, absx_store_only_writes_on_last_cycle);
	tcase_add_test(tc_cpu_address_modes_rw_logic, absy_store_only_writes_on_last_cycle);
	tcase_add_test(tc_cpu_address_modes_rw_logic, indx_store_only_writes_on_last_cycle);
	tcase_add_test(tc_cpu_address_modes_rw_logic, indy_store_only_writes_on_last_cycle);
	tcase_add_loop_test(tc_cpu_address_modes_rw_logic, zp_store_only_writes_on_last_cycle, 0, 3);
	tcase_add_loop_test(tc_cpu_address_modes_rw_logic, zpx_store_only_writes_on_last_cycle, 0, 2);
	tcase_add_test(tc_cpu_address_modes_rw_logic, zpy_store_only_writes_on_last_cycle);
	suite_add_tcase(s, tc_cpu_address_modes_rw_logic);
	tc_cpu_address_modes_cycles = tcase_create("Cpu Address Modes Cycle-By-Cycle Verification");
	tcase_add_checked_fixture(tc_cpu_address_modes_cycles, setup, teardown);
	tcase_add_test(tc_cpu_address_modes_cycles, abs_read_store_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, abs_read_store_t2);
	tcase_add_test(tc_cpu_address_modes_cycles, abs_read_store_t3);
	tcase_add_test(tc_cpu_address_modes_cycles, abs_rmw_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, abs_rmw_t2);
	tcase_add_test(tc_cpu_address_modes_cycles, abs_rmw_t3_dummy_read);
	tcase_add_test(tc_cpu_address_modes_cycles, abs_rmw_t4_dummy_write);
	tcase_add_test(tc_cpu_address_modes_cycles, absx_read_store_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, absx_read_store_t2);
	tcase_add_test(tc_cpu_address_modes_cycles, absx_read_store_t3_no_page_cross);
	tcase_add_test(tc_cpu_address_modes_cycles, absx_read_store_t3_page_cross);
	tcase_add_test(tc_cpu_address_modes_cycles, absx_read_store_t4);
	tcase_add_test(tc_cpu_address_modes_cycles, absx_rmw_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, absx_rmw_t2);
	tcase_add_test(tc_cpu_address_modes_cycles, absx_rmw_t4_2nd_absx_read);
	tcase_add_test(tc_cpu_address_modes_cycles, absx_rmw_t5_dummy_write);
	tcase_add_test(tc_cpu_address_modes_cycles, absy_read_store_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, absy_read_store_t2);
	tcase_add_test(tc_cpu_address_modes_cycles, absy_read_store_t3_no_page_cross);
	tcase_add_test(tc_cpu_address_modes_cycles, absy_read_store_t3_page_cross);
	tcase_add_test(tc_cpu_address_modes_cycles, absy_read_store_t4);
	// IMM is a single cycle and already has a unit test for it
	tcase_add_test(tc_cpu_address_modes_cycles, indx_read_store_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, indx_read_store_t2_dummy_read);
	tcase_add_test(tc_cpu_address_modes_cycles, indx_read_store_t3);
	tcase_add_test(tc_cpu_address_modes_cycles, indx_read_store_t4);
	tcase_add_test(tc_cpu_address_modes_cycles, indx_read_store_t5);
	tcase_add_test(tc_cpu_address_modes_cycles, indy_read_store_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, indy_read_store_t2);
	tcase_add_test(tc_cpu_address_modes_cycles, indy_read_store_t3);
	tcase_add_test(tc_cpu_address_modes_cycles, indy_read_store_t4_no_page_cross);
	tcase_add_test(tc_cpu_address_modes_cycles, indy_read_store_t4_page_cross);
	tcase_add_test(tc_cpu_address_modes_cycles, indy_read_store_t5);
	tcase_add_test(tc_cpu_address_modes_cycles, zp_read_store_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, zp_read_store_t2);
	tcase_add_test(tc_cpu_address_modes_cycles, zp_rmw_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, zp_rmw_t2);
	tcase_add_test(tc_cpu_address_modes_cycles, zp_rmw_t3_dummy_write);
	tcase_add_test(tc_cpu_address_modes_cycles, zpx_read_store_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, zpx_read_store_t3);
	tcase_add_test(tc_cpu_address_modes_cycles, zpx_rmw_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, zpx_rmw_t2_dummy_read);
	tcase_add_test(tc_cpu_address_modes_cycles, zpx_rmw_t3);
	tcase_add_test(tc_cpu_address_modes_cycles, zpx_rmw_t4_dummy_write);
	tcase_add_test(tc_cpu_address_modes_cycles, zpy_read_store_t1);
	tcase_add_test(tc_cpu_address_modes_cycles, zpy_read_store_t3);
	tcase_add_test(tc_cpu_address_modes_cycles, branch_ops_t1_branch_not_taken);
	tcase_add_test(tc_cpu_address_modes_cycles, branch_ops_t2_branch_taken_no_page_cross_pending);
	tcase_add_test(tc_cpu_address_modes_cycles, branch_ops_t2_branch_taken_page_cross_pending);
	tcase_add_test(tc_cpu_address_modes_cycles, branch_ops_t3_branch_taken_page_cross);
	suite_add_tcase(s, tc_cpu_address_modes_cycles);

	return s;
}
