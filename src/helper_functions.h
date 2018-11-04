/*
 * Generic Functions 
 */

#ifndef __HELPER_FUNCTIONS__
#define __HELPER_FUNCTIONS__

#include "cpu.h"
#include "globals.h"
#include <stdio.h>

/*
 * ALL get_op functions have:
 * Parameters: 1. enum MODE address_mode = addressing mode
 *             2. *code = opcode
 *             3. CPU6502 *NESCPU = contains NES
 *             ... CPU registers etc.
 */
char instruction[14]; // Disassembler
char end[7]; // Disassembler
char append_int[6]; // Disassembler

/***************************
 * FETCH OPCODE            *
 * *************************/

/* get_op: fetches operand based on address modes */
void get_op_IMM(uint8_t *ptr_code);
void get_op_ZP_offset(uint8_t *ptr_code, uint8_t offset);
void get_op_ABS_offset(uint8_t *ptr_code, uint8_t offset);
void get_op_IND(uint8_t *ptr_code, CPU_6502 *NESCPU);
void get_op_INDX(uint8_t *ptr_code, CPU_6502 *NESCPU);
void get_op_INDY(uint8_t *ptr_code, CPU_6502 *NESCPU);
unsigned PAGE_CROSS(unsigned val1, unsigned val2);

/***************************
 * OTHER                   *
 * *************************/
void RET_NES_CPU(void); /* Return Status */
void transfer_cpu(void);

/***************************
 * ADD & SUB RELATED FUNCS *
 * *************************/

void Base10toBase2(uint8_t quotient, int *bin_array);
unsigned int Base2toBase10(int *bin_array, unsigned int dec_out);
void full_adder(int *bin_sum1, int *bin_sum2, int cIN, unsigned *cOUT, int *result);

/***************************
 * STACK                   *
 * *************************/

void PUSH(uint8_t value); /* Genric Push function */
uint8_t PULL(void); /* Genric Pop (Pull) function */

/***************************
 * FLAGS                   *
 * *************************/

/* Bits : 7 ----------> 0 */
/* Flags: N V - - D I Z C */

void update_FLAG_Z(uint8_t result);
void update_FLAG_N(uint8_t result);
void update_FLAG_V(int *bin_array1, int *bin_array2, int *result);
void update_FLAG_C(uint8_t cOUT);
void set_or_clear_CARRY(unsigned value);

#endif /* __HELPER_FUNCTIONS__ */
