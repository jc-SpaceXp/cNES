/*
 * Contains CPU architechture and functions
 */
#include "extern_structs.h"
#include "cpu.h"
#include "ppu.h"  // needed for read/write functions

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Debugger */
char instruction[18]; // complete instruction i.e. LDA $2000
char end[10]; // ending of the instruction i.e. #$2000
char append_int[20]; // conversion for int to char

static void (*decode_opcode_lut[256])(Cpu6502* cpu) = {
	decode_SPECIAL        , decode_INDX_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_PUSH, decode_IMM_read_store , decode_ACC,  bad_op_code, bad_op_code           , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code,
	decode_SPECIAL        , decode_INDX_read_store, bad_op_code          , bad_op_code, decode_ZP_read_store , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_PULL, decode_IMM_read_store , decode_ACC , bad_op_code, decode_ABS_read_store , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code,
	decode_SPECIAL        , decode_INDX_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_PUSH, decode_IMM_read_store , decode_ACC , bad_op_code, decode_ABS_JMP        , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code,
	decode_RTS            , decode_INDX_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_PULL, decode_IMM_read_store , decode_ACC , bad_op_code, decode_IND_JMP        , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code,
	bad_op_code           , decode_INDX_read_store, bad_op_code          , bad_op_code, decode_ZP_read_store , decode_ZP_read_store , decode_ZP_read_store , bad_op_code, decode_IMP , bad_op_code           , decode_IMP , bad_op_code, decode_ABS_read_store , decode_ABS_read_store , decode_ABS_read_store , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, decode_ZPX_read_store, decode_ZPX_read_store, decode_ZPY_read_store, bad_op_code, decode_IMP , decode_ABSY_read_store, decode_IMP , bad_op_code, bad_op_code           , decode_ABSX_read_store, bad_op_code           , bad_op_code,
	decode_IMM_read_store , decode_INDX_read_store, decode_IMM_read_store, bad_op_code, decode_ZP_read_store , decode_ZP_read_store , decode_ZP_read_store , bad_op_code, decode_IMP , decode_IMM_read_store , decode_IMP , bad_op_code, decode_ABS_read_store , decode_ABS_read_store , decode_ABS_read_store , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, decode_ZPX_read_store, decode_ZPX_read_store, decode_ZPY_read_store, bad_op_code, decode_IMP , decode_ABSY_read_store, decode_IMP , bad_op_code, decode_ABSX_read_store, decode_ABSX_read_store, decode_ABSY_read_store, bad_op_code,
	decode_IMM_read_store , decode_INDX_read_store, bad_op_code          , bad_op_code, decode_ZP_read_store , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_IMP , decode_IMM_read_store , decode_IMP , bad_op_code, decode_ABS_read_store , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code,
	decode_IMM_read_store , decode_INDX_read_store, bad_op_code          , bad_op_code, decode_ZP_read_store , decode_ZP_read_store , decode_ZP_rmw        , bad_op_code, decode_IMP , decode_IMM_read_store , decode_IMP , bad_op_code, decode_ABS_read_store , decode_ABS_read_store , decode_ABS_rmw        , bad_op_code,
	decode_Bxx            , decode_INDY_read_store, bad_op_code          , bad_op_code, bad_op_code          , decode_ZPX_read_store, decode_ZPX_rmw       , bad_op_code, decode_IMP , decode_ABSY_read_store, bad_op_code, bad_op_code, bad_op_code           , decode_ABSX_read_store, decode_ABSX_rmw       , bad_op_code
};

static void (*execute_opcode_lut[256])(Cpu6502* cpu) = {
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
	i->suppress_nmi = false;

	return i;
}

Cpu6502* cpu_init(uint16_t pc_init, CpuPpuShare* cp)
{
	Cpu6502* i = malloc(sizeof(Cpu6502));
	if (!i) {
		fprintf(stderr, "Failed to allocate enough memory for CPU\n");
		return i;
	}

	i->cpu_ppu_io = cp;
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

	i->controller_latch = 0;
	i->player_1_controller = 0;
	i->player_2_controller = 0;

	memset(i->mem, 0, CPU_MEMORY_SIZE); // Zero out memory
	return i;
}


void init_pc(Cpu6502* cpu)
{
	cpu->PC = return_little_endian(cpu, 0xFFFC);
}

uint8_t read_from_cpu(Cpu6502* cpu, uint16_t addr)
{
	unsigned read;
	if (addr < 0x2000) {
		read = cpu->mem[addr & 0x7FF];
	} else if (addr < 0x4000) {
		read = read_ppu_reg(addr & 0x2007, cpu);
	} else if (addr == 0x4016) {
		read = read_4016(cpu);
	} else if (addr == 0x4017) {
		read = read_4017(cpu);
	} else {
		read = cpu->mem[addr]; /* catch-all */
	}

	return read;
}

/* Return 16 bit address in little endian format */
uint16_t return_little_endian(Cpu6502* cpu, uint16_t addr)
{
	return ((read_from_cpu(cpu, addr + 1) << 8) | read_from_cpu(cpu, addr));
}

void write_to_cpu(Cpu6502* cpu, uint16_t addr, uint8_t val)
{
	if (addr < 0x2000) {
		cpu->mem[addr & 0x7FF] = val;
	} else if (addr < 0x4000) {
		write_ppu_reg(addr & 0x2007, val, cpu);
		cpu->mem[addr & 0x2007] = val;
	} else if (addr == 0x4014) {
		write_ppu_reg(addr, val, cpu);
	} else if (addr == 0x4016) {
		write_4016(val, cpu);
	} else {
		cpu->mem[addr] = val;
	}
}

void write_4016(uint8_t data, Cpu6502* cpu)
{
	// Standard NES controller write
	if (data == 1) {
		cpu->controller_latch = 1;
	} else if (data == 0) {
		cpu->controller_latch = 0;
	}
}

unsigned read_4016(Cpu6502* cpu)
{
	static unsigned clock_pulse = 0;
	unsigned ret = 0;

	ret = (cpu->player_1_controller >> clock_pulse) & 0x01;

	++clock_pulse;
	if (clock_pulse == 8) { clock_pulse = 0; }

	return ret;
}


unsigned read_4017(Cpu6502* cpu)
{
	static unsigned clock_pulse = 0;
	unsigned ret = 0;

	ret = (cpu->player_2_controller >> clock_pulse) & 0x01;

	++clock_pulse;
	if (clock_pulse == 8) { clock_pulse = 0; }

	return ret;
}

// start_addr = start address of the memory you wish to insepct (16 byte boundry alligned)
// total rows is the number of rows (multiple of 16 bytes) you wish to see from the start addr
void cpu_mem_16_byte_viewer(Cpu6502* cpu, unsigned start_addr, unsigned total_rows)
{
	printf("\n##################### CPU MEM #######################\n");
	printf("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
	unsigned mem = start_addr;
	while (start_addr < total_rows) {
		printf("%.4X: ", start_addr << 4);
		for (int x = 0; x < 16; x++) {
			printf("%.2X ", cpu->mem[mem]);
			++mem;
		}
		printf("\n");
		++start_addr;
	}
}

void clock_cpu(Cpu6502* cpu)
{
	++cpu->cycle;
	--cpu->instruction_cycles_remaining;

	// Handle interrupts first
	if (cpu->cpu_ppu_io->nmi_pending && cpu->instruction_state == FETCH) {
		// print the disassembly info of the instruction just completed
		if (cpu->cpu_ppu_io->nmi_cycles_left == 7) {
#ifdef __DEBUG__
			cpu->cpu_ppu_io->write_debug = true;
			cpu_debugger(cpu);
			log_cpu_info(cpu);
			update_cpu_info(cpu);
#endif /* __DEBUG__ */
		}
		execute_NMI(cpu);
		--cpu->cpu_ppu_io->nmi_cycles_left;
	} else {
		// Fetch-decode-execute state logic
		if (cpu->instruction_state == FETCH) {
			if (cpu->cpu_ppu_io->dma_pending) {
				execute_DMA(cpu);
				return;
			}
			// if not the first instruction print its output
			if (cpu->instruction_cycles_remaining != 50) {
#ifdef __DEBUG__
				cpu->cpu_ppu_io->write_debug = true;
				cpu_debugger(cpu);
				log_cpu_info(cpu);
				update_cpu_info(cpu);
#endif /* __DEBUG__ */
			}
			fetch_opcode(cpu);
		}  else if (cpu->instruction_state == DECODE) {
			decode_opcode_lut[cpu->opcode](cpu);
		}
		if (cpu->instruction_state == EXECUTE) {
			cpu->instruction_state = FETCH;
#ifdef ENABLE_PIPELINING
			fetch_opcode(cpu);
#endif /* ENABLE_PIPELINING */
			execute_opcode_lut[cpu->opcode](cpu); // can change the PC which the early fetch made!
		}
	}
}

// true if branch not taken based on opcode
bool branch_not_taken(Cpu6502* cpu)
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

void cpu_debugger(Cpu6502* cpu)
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
		strcpy(end, ")");
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

void log_cpu_info(Cpu6502* cpu)
{
	printf("%-6.4X ", cpu->old_PC);
	printf("%-20s ", instruction);
	printf("A:%.2X ", cpu->old_A);
	printf("X:%.2X ", cpu->old_X);
	printf("Y:%.2X ", cpu->old_Y);
	printf("P:%.2X ", cpu->old_P);
	printf("SP:%.2X ", cpu->old_stack);

	if (cpu->old_cycle == 0) {
		printf("CPU:%.4u", cpu->old_cycle);
	} else { // first cycle = +1 cycles due to the tick() after the instruction executes
		printf("CPU:%.4u", cpu->old_cycle - 1);
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

bool page_cross_occurs(unsigned low_byte, unsigned offset)
{
	return ((low_byte + offset) > 0xFF) ? 1 : 0;
}

void stack_push(Cpu6502* cpu, uint8_t value)
{
	/* SP_START - 1 - as Stack = Empty Descending */
	/* FIX LIMIT CHECK */
	if (cpu->stack == 0x00) {
		/* Overflow */
		printf("Full stack - can't PUSH\n"); // Instead wrap-around
	} else {
		cpu->mem[SP_START + cpu->stack] = value;
		--cpu->stack;
	}
}


uint8_t stack_pull(Cpu6502* cpu)
{
	unsigned result = 0;
	/* FIX LIMIT CHECK */
	if (cpu->stack == SP_START) {
		/* Underflow */
		printf("Empty stack - can't PULL\n"); // Instead wrap-around
	} else {
		++cpu->stack;
		result = cpu->mem[SP_START + cpu->stack];
	}
	return result;
}

void fetch_opcode(Cpu6502* cpu)
{
	cpu->opcode = read_from_cpu(cpu, cpu->PC);
	++cpu->PC;

	cpu->instruction_cycles_remaining = max_cycles_opcode_lut[cpu->opcode];
	cpu->instruction_state = DECODE;
}

// Store operations can't skip cycles and are always fixed length unlike their load counterparts
// before skipping cycles check whether that is correct using this function
bool fixed_cycles_on_store(Cpu6502* cpu)
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


void bad_op_code(Cpu6502* cpu)
{
	printf("invalid opcode: error 404: %.2X @ %.4X \n", cpu->opcode, cpu->PC);
}

void decode_ABS_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ABS;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 3: // T1
		cpu->addr_lo = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 2: // T2
		cpu->addr_hi = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 1: //T3
		cpu->target_addr = ((cpu->addr_hi << 8) | cpu->addr_lo);
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_ABS_rmw(Cpu6502* cpu)
{
	cpu->address_mode = ABS;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		cpu->addr_lo = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 4: // T2
		cpu->addr_hi = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 3: // T3 (dummy read?)
		cpu->target_addr = ((cpu->addr_hi << 8) | cpu->addr_lo);
		break;
	case 2: // T4 (dummy write)
		cpu->unmodified_data = read_from_cpu(cpu, cpu->target_addr);
		write_to_cpu(cpu, cpu->target_addr, cpu->unmodified_data);
		break;
	case 1: // T5
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_ABSX_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ABSX;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 4: // T1
		cpu->addr_lo = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 3: // T2
		cpu->addr_hi = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 2: // T3
		cpu->target_addr = cpu->addr_hi << 8 | ((cpu->addr_lo + cpu->X) & 0xFF);
		if (!fixed_cycles_on_store(cpu) && !page_cross_occurs(cpu->addr_lo, cpu->X)) { cpu->instruction_state = EXECUTE; }
		// dummy read not implemented
		break;
	case 1: // T4 (page cross)
		// either keep the first two lines or comment out the third
		//cpu->addr_hi += 1;
		//cpu->target_addr = cpu->addr_hi << 8 | (cpu->addr_lo + cpu->X) & 0xFF;
		cpu->target_addr = ((cpu->addr_hi << 8) | cpu->addr_lo) + cpu->X;
		cpu->operand = read_from_cpu(cpu, cpu->target_addr); // comment out in execute functions (make sure it's the last commit)
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_ABSX_rmw(Cpu6502* cpu)
{
	cpu->address_mode = ABSX;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 6: // T1
		cpu->addr_lo = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 5: // T2
		cpu->addr_hi = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 4: // T3 (dummy read) [not implemented]
		cpu->target_addr = cpu->addr_hi << 8 | ((cpu->addr_lo + cpu->X) & 0xFF);
		break;
	case 3: // T4
		cpu->target_addr = ((cpu->addr_hi << 8) | cpu->addr_lo) + cpu->X; // data is discarded
		cpu->unmodified_data = read_from_cpu(cpu, cpu->target_addr);
		break;
	case 2: // T5 (dummy write)
		write_to_cpu(cpu, cpu->target_addr, cpu->unmodified_data);
		break;
	case 1: // T6
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_ABSY_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ABSY;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 4: // T1
		cpu->addr_lo = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 3: // T2
		cpu->addr_hi = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 2: // T3
		cpu->target_addr = cpu->addr_hi << 8 | ((cpu->addr_lo + cpu->Y) & 0xFF);
		if (!fixed_cycles_on_store(cpu) && !page_cross_occurs(cpu->addr_lo, cpu->Y)) { cpu->instruction_state = EXECUTE; }
		// dummy read not implemented
		break;
	case 1: // T4 (page cross)
		// either keep the first two lines or comment out the third
		//cpu->addr_hi += 1;
		//cpu->target_addr = cpu->addr_hi << 8 | (cpu->addr_lo + cpu->Y) & 0xFF;
		cpu->target_addr = ((cpu->addr_hi << 8) | cpu->addr_lo) + cpu->Y;
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_ACC(Cpu6502* cpu)
{
	cpu->address_mode = ACC;
	cpu->instruction_state = EXECUTE;
}

void decode_IMM_read_store(Cpu6502* cpu) // might only just be read
{
	cpu->address_mode = IMM;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 1: // T1
		cpu->operand = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_IMP(Cpu6502* cpu)
{
	cpu->address_mode = IMP;
	// opcode fetched: T0
	cpu->instruction_state = EXECUTE;
}

void decode_INDX_read_store(Cpu6502* cpu)
{
	cpu->address_mode = INDX;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		cpu->base_addr = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 4: // T2
		cpu->addr_lo = read_from_cpu(cpu, cpu->base_addr);  // DISCARD
		break;
	case 3: // T3
		cpu->addr_lo = read_from_cpu(cpu, cpu->base_addr + cpu->X);
		break;
	case 2: // T4
		cpu->addr_hi = read_from_cpu(cpu, cpu->base_addr + cpu->X + 1);
		break;
	case 1: // T5
		cpu->target_addr = ((cpu->addr_hi << 8) | cpu->addr_lo);
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_INDY_read_store(Cpu6502* cpu)
{
	cpu->address_mode = INDY;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		cpu->base_addr = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 4: // T2
		cpu->addr_lo = read_from_cpu(cpu, cpu->base_addr); // ZP read
		break;
	case 3: // T3
		cpu->addr_hi = read_from_cpu(cpu, cpu->base_addr + 1);
		break;
	case 2: // T4
		cpu->target_addr = cpu->addr_hi << 8 | ((cpu->addr_lo + cpu->Y) & 0xFF);
		if (!fixed_cycles_on_store(cpu) && !page_cross_occurs(cpu->addr_lo, cpu->Y)) { cpu->instruction_state = EXECUTE; }
		// dummy read not implemented
		break;
	case 1: // T5 (page cross)
		// either keep the first two lines or comment out the third
		//cpu->addr_hi += 1;
		//cpu->target_addr = cpu->addr_hi << 8 | (cpu->addr_lo + cpu->Y) & 0xFF;
		cpu->target_addr = ((cpu->addr_hi << 8) | cpu->addr_lo) + cpu->Y;
		cpu->instruction_state = EXECUTE;
	}
}

void decode_ZP_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ZP;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 2: // T1
		cpu->addr_lo = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 1: // T2
		cpu->target_addr = cpu->addr_lo;
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_ZP_rmw(Cpu6502* cpu)
{
	cpu->address_mode = ZP;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 4: // T1
		cpu->addr_lo = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 3: // T2
		cpu->target_addr = cpu->addr_lo;
		cpu->unmodified_data = read_from_cpu(cpu, cpu->target_addr);
		break;
	case 2: // T3
		write_to_cpu(cpu, cpu->target_addr, cpu->unmodified_data);
		break;
	case 1: // T4
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_ZPX_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ZPX;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 3: // T1
		cpu->addr_lo = read_from_cpu(cpu, cpu->PC); // base address
		++cpu->PC;
		break;
	case 2: // T2
		cpu->target_addr = cpu->addr_lo; // discard read occurs
		break;
	case 1: // T3
		cpu->target_addr = (cpu->addr_lo + cpu->X) & 0xFF;
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_ZPX_rmw(Cpu6502* cpu)
{
	cpu->address_mode = ZPX;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		cpu->addr_lo = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 4: // T2
		cpu->target_addr = cpu->addr_lo;
		cpu->unmodified_data = read_from_cpu(cpu, cpu->target_addr); // discard
		break;
	case 3: // T3 (dummy read)
		cpu->target_addr = (cpu->addr_lo + cpu->X) & 0xFF;
		cpu->unmodified_data = read_from_cpu(cpu, cpu->target_addr);
		break;
	case 2: // T4 (dummy write)
		write_to_cpu(cpu, cpu->target_addr, cpu->unmodified_data);
		break;
	case 1: // T5
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_ZPY_read_store(Cpu6502* cpu)
{
	cpu->address_mode = ZPY;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 3: // T1
		cpu->addr_lo = read_from_cpu(cpu, cpu->PC); // base address
		++cpu->PC;
		break;
	case 2: // T2 (dummy read)
		cpu->target_addr = cpu->addr_lo;
		break;
	case 1: // T3
		cpu->target_addr = (cpu->addr_lo + cpu->Y) & 0xFF;
		//cpu->target_addr = (uint8_t) (cpu->addr_lo + cpu->Y); either one
		cpu->instruction_state = EXECUTE;
		break;
	default:
		printf("We shouldn't be here! invalid cycle\n");
		break;
	}
}

// could have one common function and use a switch case for the opcode?
void decode_ABS_JMP(Cpu6502* cpu)
{
	cpu->address_mode = ABS;
	// opcode fetched: T0
	cpu->instruction_state = EXECUTE;
}

void decode_IND_JMP(Cpu6502* cpu)
{
	cpu->address_mode = IND;
	// opcode fetched: T0
	cpu->instruction_state = EXECUTE;
}

void decode_SPECIAL(Cpu6502* cpu)
{
	cpu->address_mode = SPECIAL;
	// opcode fetched: T0
	cpu->instruction_state = EXECUTE;
}

void decode_PUSH(Cpu6502* cpu)
{
	// Technically an IMP instruction but not handled by decode_IMP()
	cpu->address_mode = IMP; // used for debugger sets end = ""
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 2: // T1
		read_from_cpu(cpu, cpu->PC); // dummy read (next opcode)
		break;
	case 1: // T2
		cpu->instruction_state = EXECUTE;
		// push A or P onto stack w/ PHA or PHP
		break;
	}
}


void decode_PULL(Cpu6502* cpu)
{
	// Technically an IMP instruction but not handled by decode_IMP()
	cpu->address_mode = IMP; // used for debugger sets end = ""
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 3: // T1
		read_from_cpu(cpu, cpu->PC); // dummy read (next opcode)
		break;
	case 2: // T2
		read_from_cpu(cpu, SP_START + cpu->stack); // dummy read (stack pointer)
		break;
	case 1: // T2
		cpu->instruction_state = EXECUTE;
		// pull A or P via execute functions
		break;
	}
}

void decode_Bxx(Cpu6502* cpu) // branch instructions
{
	cpu->address_mode = REL;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 3: // T1
		cpu->offset = read_from_cpu(cpu, cpu->PC); // -128 + 127 offset
		++cpu->PC;

		if (branch_not_taken(cpu)) {
			cpu->target_addr = cpu->PC; // already @ PC + 2 (1 from opcode and 1 from T1)
			cpu->instruction_state = EXECUTE;
		}
		break;
	case 2: // T2
		// w/o carry --> (PCH | (PC + offset) & 0xFF)
		cpu->target_addr = (cpu->PC & 0xFF00) | ((cpu->PC + cpu->offset) & 0x00FF);
		if (!page_cross_occurs(cpu->PC & 0xFF, cpu->offset)) {
			cpu->instruction_state = EXECUTE;
		}
		break;
	case 1: // T3 (page cross)
		cpu->target_addr = cpu->PC + cpu->offset;
		cpu->instruction_state = EXECUTE;
		break;
	}
}

void decode_RTS(Cpu6502* cpu)
{
	cpu->address_mode = IMP;
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		read_from_cpu(cpu, cpu->PC); // dummy read
		break;
	case 4: // T2
		read_from_cpu(cpu, SP_START + cpu->stack); // dummy stack read
		break;
	case 3: // T3
		cpu->addr_lo = stack_pull(cpu);
		break;
	case 2: // T4
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

void update_flag_z(Cpu6502* cpu, uint8_t result)
{
	/* Zero Flag Test */
	if (!result) {
		cpu->P |= FLAG_Z; /* Set Z */
	} else {
		cpu->P &= ~FLAG_Z; /* Clear Z */
	}
}


void update_flag_n(Cpu6502* cpu, uint8_t result)
{
	/* Negative Flag Test */
	if (result >> 7) {
		cpu->P |= FLAG_N; /* Set N */
	} else {
		cpu->P &= ~FLAG_N; /* Clear N */
	}
}


/* Parameters = 2 binary operands and then the result */
void update_flag_v(Cpu6502* cpu, bool overflow)
{
	/* Overflow Flag Test */
	if (overflow) {
		cpu->P |= FLAG_V; /* Set V */
	} else {
		cpu->P &= ~FLAG_V; /* Clear V */
	}
}


void update_flag_c(Cpu6502* cpu, int carry_out)
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
void execute_LDA(Cpu6502* cpu)
{
	strcpy(instruction, "LDA ");
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
void execute_LDX(Cpu6502* cpu)
{
	strcpy(instruction, "LDX ");
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
void execute_LDY(Cpu6502* cpu)
{
	strcpy(instruction, "LDY ");
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
void execute_STA(Cpu6502* cpu)
{
	strcpy(instruction, "STA ");
	write_to_cpu(cpu, cpu->target_addr, cpu->A);
}


/* execute_STX: STX command - Store X in memory
 */
void execute_STX(Cpu6502* cpu)
{
	strcpy(instruction, "STX ");
	write_to_cpu(cpu, cpu->target_addr, cpu->X);
}


/* execute_STY: STY command - Store Y in memory
 */
void execute_STY(Cpu6502* cpu)
{
	strcpy(instruction, "STY ");
	write_to_cpu(cpu, cpu->target_addr, cpu->Y);
}


/* execute_TAX: TAX command - Transfer A to X
 */
void execute_TAX(Cpu6502* cpu)
{
	strcpy(instruction, "TAX ");
	cpu->X = cpu->A;
	update_flag_n(cpu, cpu->X);
	update_flag_z(cpu, cpu->X);
}


/* execute_TAY: TAY command - Transfer A to Y
 */
void execute_TAY(Cpu6502* cpu)
{
	strcpy(instruction, "TAY ");
	cpu->Y = cpu->A;
	update_flag_n(cpu, cpu->Y);
	update_flag_z(cpu, cpu->Y);
}


/* execute_TSX: TSX command - Transfer SP to X
 */
void execute_TSX(Cpu6502* cpu)
{
	strcpy(instruction, "TSX ");
	cpu->X = cpu->stack;
	update_flag_n(cpu, cpu->X);
	update_flag_z(cpu, cpu->X);
}


/* execute_TXA: TXA command - Transfer X to A
 */
void execute_TXA(Cpu6502* cpu)
{
	strcpy(instruction, "TXA ");
	cpu->A = cpu->X;
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


/* execute_TXS: TXS command - Transfer X to SP
 */
void execute_TXS(Cpu6502* cpu)
{
	strcpy(instruction, "TXS ");
	cpu->stack = cpu->X;
}


/* execute_TYA: TYA command - Transfer Y to A
 */
void execute_TYA(Cpu6502* cpu)
{
	strcpy(instruction, "TYA ");
	cpu->A = cpu->Y;
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}

/***************************
 * MATH                    *
 * *************************/

/* execute_ADC: ADC command - Add mem w/ A and C (A + M + C : then set flags)
 */
void execute_ADC(Cpu6502* cpu)
{
	strcpy(instruction, "ADC ");
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
void execute_DEC(Cpu6502* cpu)
{
	strcpy(instruction, "DEC ");
	cpu->operand = read_from_cpu(cpu, cpu->target_addr);
	write_to_cpu(cpu, cpu->target_addr, cpu->operand - 1);
	update_flag_n(cpu, cpu->operand - 1);
	update_flag_z(cpu, cpu->operand - 1);
}


/* execute_DEX: DEX command - Decrement X by one
 */
void execute_DEX(Cpu6502 *cpu)
{
	/* Implied Mode */
	strcpy(instruction, "DEX ");
	--cpu->X;
	update_flag_n(cpu, cpu->X);
	update_flag_z(cpu, cpu->X);
}


/* execute_DEY: DEY command - Decrement Y by one
 */
void execute_DEY(Cpu6502* cpu)
{
	/* Implied Mode */
	strcpy(instruction, "DEY ");
	--cpu->Y;
	update_flag_n(cpu, cpu->Y);
	update_flag_z(cpu, cpu->Y);
}


/* execute_INC: INC command - Increment Mem by one
 */
void execute_INC(Cpu6502* cpu)
{
	strcpy(instruction, "INC ");
	cpu->operand = read_from_cpu(cpu, cpu->target_addr);
	write_to_cpu(cpu, cpu->target_addr, cpu->operand + 1);
	update_flag_n(cpu, cpu->operand + 1);
	update_flag_z(cpu, cpu->operand + 1);
}


/* execute_INX: INX command - Increment X by one
 */
void execute_INX(Cpu6502* cpu)
{
	strcpy(instruction, "INX ");
	/* Implied Mode */
	++cpu->X;
	update_flag_n(cpu, cpu->X);
	update_flag_z(cpu, cpu->X);
}


/* execute_INY: DEY command - Increment Y by one
 */
void execute_INY(Cpu6502* cpu)
{
	strcpy(instruction, "INY ");
	/* Implied Mode */
	++cpu->Y;
	update_flag_n(cpu, cpu->Y);
	update_flag_z(cpu, cpu->Y);
}


/* execute_SBC: SBC command - Subtract mem w/ A and C (A - M - !C : then set flags)
 */
void execute_SBC(Cpu6502* cpu)

{
	strcpy(instruction, "SBC ");
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
void execute_AND(Cpu6502* cpu)
{
	strcpy(instruction, "AND ");
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
void execute_ASL(Cpu6502* cpu)
{
	strcpy(instruction, "ASL ");
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
void execute_BIT(Cpu6502* cpu)
{
	strcpy(instruction, "BIT ");
	cpu->operand = read_from_cpu(cpu, cpu->target_addr);
	/* Update Flags */
	/* N = Bit 7, V = Bit 6 (of fetched operand) & Z = 1 (if AND result = 0) */

	update_flag_n(cpu, cpu->operand);
	update_flag_v(cpu, cpu->operand & FLAG_V);
	update_flag_z(cpu, cpu->operand & cpu->A);
}


/* execute_EOR: EOR command - Exclusive OR memory with Acc
 */
void execute_EOR(Cpu6502* cpu)
{
	strcpy(instruction, "EOR ");
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
void execute_LSR(Cpu6502* cpu)
{
	strcpy(instruction, "LSR ");
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
void execute_ORA(Cpu6502* cpu)
{
	strcpy(instruction, "ORA ");
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
void execute_ROL(Cpu6502* cpu)
{
	strcpy(instruction, "ROL ");
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
void execute_ROR(Cpu6502* cpu)
{
	strcpy(instruction, "ROR ");
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
void execute_BCC(Cpu6502* cpu)
{
	strcpy(instruction, "BCC ");
	cpu->PC = cpu->target_addr;
}


/* execute_BCS: BCS command - Branch on Carry Set (C = 1)
 */
void execute_BCS(Cpu6502* cpu)
{
	strcpy(instruction, "BCS ");
	cpu->PC = cpu->target_addr;
}


/* execute_BEQ: BEQ command - Branch on Zero result (Z = 1)
 */
void execute_BEQ(Cpu6502* cpu)
{
	strcpy(instruction, "BEQ ");
	cpu->PC = cpu->target_addr;
}


/* execute_BMI: BMI command - Branch on Minus result (N = 1)
 */
void execute_BMI(Cpu6502* cpu)
{
	strcpy(instruction, "BMI ");
	cpu->PC = cpu->target_addr;
}


/* execute_BNE: BNE command - Branch on NOT Zero result (Z = 0)
 */
void execute_BNE(Cpu6502* cpu)
{
	strcpy(instruction, "BNE ");
	cpu->PC = cpu->target_addr;
}


/* execute_BPL: BPL command - Branch on Plus result (N = 0)
 */
void execute_BPL(Cpu6502* cpu)
{
	strcpy(instruction, "BPL ");
	cpu->PC = cpu->target_addr;
}


/* execute_BVC: BVC command - Branch on Overflow Clear (V = 0)
 */
void execute_BVC(Cpu6502* cpu)
{
	strcpy(instruction, "BVC ");
	cpu->PC = cpu->target_addr;
}


/* execute_BVS: BVS command - Branch on Overflow Set (V = 1)
 */
void execute_BVS(Cpu6502* cpu)
{
	strcpy(instruction, "BVS ");
	cpu->PC = cpu->target_addr;
}


/***************************
 * JUMP                    *
 * *************************/

/* execute_JMP: JMP command - JuMP to another location
 */
void execute_JMP(Cpu6502* cpu)
{
	strcpy(instruction, "JMP ");
	if (cpu->address_mode == ABS) {
		switch (cpu->instruction_cycles_remaining) {
		case 2: // T1
			cpu->addr_lo = read_from_cpu(cpu, cpu->PC);
			++cpu->PC;
			break;
		case 1: // T2
			cpu->addr_hi = read_from_cpu(cpu, cpu->PC);
			cpu->PC = (cpu->addr_hi << 8) | cpu->addr_lo; // END T2
			cpu->target_addr = (cpu->addr_hi << 8) | cpu->addr_lo; // for debugger function
			break;
		}
	} else if (cpu->address_mode == IND) {
		switch (cpu->instruction_cycles_remaining) {
		case 4: // T1
			cpu->index_lo = read_from_cpu(cpu, cpu->PC);
			++cpu->PC;
			break;
		case 3: // T2
			cpu->index_hi = read_from_cpu(cpu, cpu->PC);
			++cpu->PC;
			break;
		case 2: // T3
			cpu->addr_lo = read_from_cpu(cpu, (cpu->index_hi << 8) | cpu->index_lo);
			break;
		case 1: // T4
			if (cpu->index_lo == 0xFF) { // JMP bug
				cpu->addr_hi = read_from_cpu(cpu, cpu->index_hi << 8);
			} else {
				cpu->addr_hi = read_from_cpu(cpu, ((cpu->index_hi << 8) | cpu->index_lo) + 1);
			}

			cpu->PC = (cpu->addr_hi << 8) | cpu->addr_lo; // END T4
			cpu->target_addr = (cpu->addr_hi << 8) | cpu->addr_lo; // for debugger function
			break;
		}
	}

	if (cpu->instruction_cycles_remaining != 1) {
		cpu->instruction_state = EXECUTE;
	}
}


/* execute_JSR: JSR command - Jump to SubRoutine
 */
void execute_JSR(Cpu6502* cpu)
{
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		cpu->addr_lo = read_from_cpu(cpu, cpu->PC);
		++cpu->PC;
		break;
	case 4: // T2 (dummy read on stack)
		read_from_cpu(cpu, SP_START + cpu->stack); // dummy cpu stack read
		break;
	case 3: // T3 (PC + 2 from read_op)
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 2: // T4 (PC + 2 from read_op)
		stack_push(cpu, (uint8_t) cpu->PC); // push PCL onto stack
		break;
	case 1: // T5
		cpu->addr_hi = read_from_cpu(cpu, cpu->PC);
		cpu->target_addr = (cpu->addr_hi << 8) | cpu->addr_lo;
		cpu->PC = cpu->target_addr;

		strcpy(instruction, "JSR $");
		sprintf(append_int, "%.4X", cpu->target_addr);
		strcat(instruction, append_int);
		break;
	}

	if (cpu->instruction_cycles_remaining != 1) {
		cpu->instruction_state = EXECUTE;
	}
}


/* execute_RTI: RTI command - ReTurn from Interrupt
 */
void execute_RTI(Cpu6502* cpu)
{
	strcpy(instruction, "RTI");
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 5: // T1
		read_from_cpu(cpu, cpu->PC); // dummy read
		break;
	case 4: // T2
		read_from_cpu(cpu, SP_START + cpu->stack); // dummy stack read
		break;
	case 3: // T3
		cpu->P = stack_pull(cpu) | 0x20;
		break;
	case 2: // T4
		cpu->addr_lo = stack_pull(cpu);
		break;
	case 1: // T5
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
void execute_RTS(Cpu6502* cpu)
{
	strcpy(instruction, "RTS");
	cpu->target_addr = (cpu->addr_hi << 8) | cpu->addr_lo;
	read_from_cpu(cpu, cpu->target_addr); // dummy read
	cpu->PC = cpu->target_addr + 1;
}

/***************************
 * Registers               *
 * *************************/

/* execute_CLC: CLC command - Clear Carry flag
 */
void execute_CLC(Cpu6502* cpu)
{
	strcpy(instruction, "CLC");
	cpu->P &= ~FLAG_C;
}


/* execute_CLD: CLD command - Clear Decimal Mode (Decimal mode not supported in NES) 
 */
void execute_CLD(Cpu6502* cpu)
{
	strcpy(instruction, "CLD");
	cpu->P &= ~FLAG_D;

}


/* execute_CLI: CLI command - Clear Interrupt disable bit
 */
void execute_CLI(Cpu6502* cpu)
{
	strcpy(instruction, "CLI");
	cpu->P &= ~FLAG_I;
}


/* execute_CLV: CLV command - Clear Overflow flag
 */
void execute_CLV(Cpu6502* cpu)
{
	strcpy(instruction, "CLV");
	cpu->P &= ~FLAG_V;
}


/* execute_CMP: CMP command - Compare mem w/ A (A - M then set flags)
 */
void execute_CMP(Cpu6502* cpu)
{
	strcpy(instruction, "CMP ");
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
void execute_CPX(Cpu6502* cpu)
{
	strcpy(instruction, "CPX ");
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
void execute_CPY(Cpu6502* cpu)
{
	strcpy(instruction, "CPY ");
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
void execute_SEC(Cpu6502* cpu)
{
	strcpy(instruction, "SEC ");
	cpu->P |= FLAG_C;
}


/* execute_SED: SED command - Set Decimal Mode (Decimal mode not supported in NES)
 */
void execute_SED(Cpu6502* cpu)
{
	strcpy(instruction, "SED ");
	cpu->P |= FLAG_D;
}


/* execute_SEI: SEI command - Set Interrupt disable bit (I = 1)
 */
void execute_SEI(Cpu6502* cpu)
{
	strcpy(instruction, "SEI ");
	cpu->P |= FLAG_I;
}

/***************************
 * STACK                   *
 * *************************/

void execute_PHA(Cpu6502* cpu)
{
	strcpy(instruction, "PHA ");
	stack_push(cpu, cpu->A);
}

void execute_PHP(Cpu6502* cpu)
{
	strcpy(instruction, "PHP ");
	stack_push(cpu, cpu->P | 0x30); // set bits 4 & 5
}


void execute_PLA(Cpu6502* cpu)
{
	strcpy(instruction, "PLA ");
	cpu->A = stack_pull(cpu);
	update_flag_n(cpu, cpu->A);
	update_flag_z(cpu, cpu->A);
}


void execute_PLP(Cpu6502* cpu)
{
	strcpy(instruction, "PLP ");
	cpu->P = stack_pull(cpu) & ~ 0x10; // B flag may exist on stack but not P so it is cleared
	cpu->P |= 0x20; // bit 5 always set
}

/***************************
 * SYSTEM                  *
 * *************************/

/* execute_BRK: BRK command - Fore Break - Store PC & P (along w/ X, Y & A)
 */
void execute_BRK(Cpu6502* cpu)
{
	strcpy(instruction, "BRK ");
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 6: // T1
		read_from_cpu(cpu, cpu->PC); // dummy read
		++cpu->PC; // added (had PC + 2 before and that passed nestest)
		break;
	case 5: // T2
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 4: // T3
		stack_push(cpu, (uint8_t) cpu->PC); // push PCL onto stack
		++cpu->PC; // needed?
		break;
	case 3: // T4
		stack_push(cpu, cpu->P | 0x30); // push status reg onto stack
		cpu->P |= FLAG_I;              /* Flag I is set */
		break;
	case 2: // T5
		cpu->addr_lo = read_from_cpu(cpu, 0xFFFE);
		break;
	case 1: // T6
		cpu->addr_hi = read_from_cpu(cpu, 0xFFFF);
		cpu->PC = (cpu->addr_hi << 8) | cpu->addr_lo;
		break;
	}

	if (cpu->instruction_cycles_remaining != 1) {
		cpu->instruction_state = EXECUTE;
	}
}


/* execute_NOP: NOP command - Does nothing (No OPeration)
 */
void execute_NOP(Cpu6502* cpu)
{
	strcpy(instruction, "NOP ");
	(void) cpu; // suppress unused variable compiler warning
}


/* Non opcode interrupts */
void execute_IRQ(Cpu6502* cpu)
{
	strcpy(instruction, "IRQ ");
	// opcode fetched: T0
	switch (cpu->instruction_cycles_remaining) {
	case 6: // T1
		read_from_cpu(cpu, cpu->PC); // dummy read
		// (increment not on 64doc.txt)
		++cpu->PC; // added (had PC + 2 before and that passed nestest)
		break;
	case 5: // T2
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 4: // T3
		stack_push(cpu, (uint8_t) cpu->PC); // push PCL onto stack
		++cpu->PC;
		break;
	case 3: // T4
		stack_push(cpu, cpu->P & ~0x30); // push status reg onto stack
		cpu->P |= FLAG_I;
		break;
	case 2: // T5
		cpu->addr_lo = read_from_cpu(cpu, 0xFFFE);
		break;
	case 1: // T6
		cpu->addr_hi = read_from_cpu(cpu, 0xFFFF);
		cpu->PC = (cpu->addr_hi << 8) | cpu->addr_lo;
		break;
	}

	if (cpu->instruction_cycles_remaining != 1) {
		cpu->instruction_state = EXECUTE;
	}
}


void execute_NMI(Cpu6502* cpu)
{
	strcpy(instruction, "NMI");
	cpu->address_mode = SPECIAL;
	// opcode fetched: T0
	switch (cpu->cpu_ppu_io->nmi_cycles_left) {
	case 6: // T1
		read_from_cpu(cpu, cpu->PC); // dummy read
		break;
	case 5: // T2
		stack_push(cpu, (uint8_t) (cpu->PC >> 8)); // push PCH onto stack
		break;
	case 4: // T3
		stack_push(cpu, (uint8_t) cpu->PC); // push PCL onto stack
		++cpu->PC;
		break;
	case 3: // T4
		stack_push(cpu, cpu->P & ~0x30); // push status reg onto stack
		cpu->P |= FLAG_I;
		break;
	case 2: // T5
		cpu->addr_lo = read_from_cpu(cpu, 0xFFFA);
		break;
	case 1: // T6
		cpu->addr_hi = read_from_cpu(cpu, 0xFFFB);
		cpu->PC = (cpu->addr_hi << 8) | cpu->addr_lo;
		cpu->cpu_ppu_io->nmi_pending = false;
		break;
	}
}


void execute_DMA(Cpu6502* cpu)
{
	static bool first_cycle = true;
	if (first_cycle) {
#ifdef __DEBUG__
		cpu->cpu_ppu_io->write_debug = true;
		cpu_debugger(cpu);
		log_cpu_info(cpu);
		update_cpu_info(cpu);
		first_cycle = false;
#endif /* __DEBUG__ */
	}
	/* Triggered by PPU, CPU is suspended */
	strcpy(instruction, "DMA");
	cpu->address_mode = SPECIAL;

	//initialise static
	static unsigned cycles_left = 513;

	// + 1 cycle on an odd cycle
	if ((cpu->cycle & 1) && cycles_left == 513) { cycles_left = 514; }


	if (cycles_left == 514 || cycles_left == 513) {
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
		cycles_left = 513;
		first_cycle = true;
	}
}
