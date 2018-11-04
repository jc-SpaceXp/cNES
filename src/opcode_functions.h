/*
 * Execute Functions (Opcode functions)
 *
 */

#ifndef __6502_FUNCTIONS__
#define __6502_FUNCTIONS__

#include "cpu.h"
#include <string.h>

/***************************
 * STORAGE                 *
 * *************************/

void execute_LDA(enum MODES address_mode, CPU_6502* CPU);
void execute_LDX(enum MODES address_mode, CPU_6502* CPU);
void execute_LDY(enum MODES address_mode, CPU_6502* CPU);
void execute_STA(CPU_6502* CPU);
void execute_STX(CPU_6502* CPU);
void execute_STY(CPU_6502* CPU);
void execute_TAX(CPU_6502* CPU);
void execute_TAY(CPU_6502* CPU);
void execute_TSX(CPU_6502* CPU);
void execute_TXA(CPU_6502* CPU);
void execute_TXS(CPU_6502* CPU);
void execute_TYA(CPU_6502* CPU);

/***************************
 * MATH                    *
 * *************************/

void execute_ADC(enum MODES address_mode, CPU_6502* CPU);
void execute_DEC(CPU_6502* CPU);
void execute_DEX(CPU_6502* CPU);
void execute_DEY(CPU_6502* CPU);
void execute_INC(CPU_6502* CPU);
void execute_INX(CPU_6502* CPU);
void execute_INY(CPU_6502* CPU);
void execute_SBC(enum MODES address_mode, CPU_6502* CPU);

/***************************
 * BITWISE                 *
 * *************************/

void execute_AND(enum MODES address_mode, CPU_6502* CPU);
void execute_ASL(enum MODES address_mode, CPU_6502* CPU);
void execute_BIT(CPU_6502* CPU);
void execute_EOR(enum MODES address_mode, CPU_6502* CPU);
void execute_LSR(enum MODES address_mode, CPU_6502* CPU);
void execute_ORA(enum MODES address_mode, CPU_6502* CPU);
void execute_ROL(enum MODES address_mode, CPU_6502* CPU);
void execute_ROR(enum MODES address_mode, CPU_6502* CPU);

/***************************
 * BRANCH                  *
 * *************************/

void execute_BCC(uint8_t *ptr_code);
void execute_BCS(uint8_t *ptr_code);
void execute_BEQ(uint8_t *ptr_code);
void execute_BMI(uint8_t *ptr_code);
void execute_BNE(uint8_t *ptr_code);
void execute_BPL(uint8_t *ptr_code);
void execute_BVC(uint8_t *ptr_code);
void execute_BVS(uint8_t *ptr_code);

/***************************
 * JUMP                    *
 * *************************/
void execute_JMP(CPU_6502* COU);
void execute_JSR(CPU_6502* CPU);
void execute_RTI(CPU_6502* CPU);
void execute_RTS(CPU_6502* CPU);

/***************************
 * Registers               *
 * *************************/

void execute_CLC(CPU_6502* CPU);
void execute_CLD(CPU_6502* CPU);
void execute_CLI(CPU_6502* CPU);
void execute_CLV(CPU_6502* CPU);
void execute_CMP(enum MODES address_mode, CPU_6502* CPU);
void execute_CPX(enum MODES address_mode, CPU_6502* CPU);
void execute_CPY(enum MODES address_mode, CPU_6502* CPU);
void execute_SEC(CPU_6502* CPU);
void execute_SED(CPU_6502* CPU);
void execute_SEI(CPU_6502* CPU);

/***************************
 * SYSTEM                  *
 * *************************/

void execute_BRK(void);
void execute_NOP(void);
void execute_IRQ(void);
void execute_NMI(void); // Not an official opcode
void execute_DMA(void); // Not an official opcode

#endif /* __6502_FUNCTIONS__ */
