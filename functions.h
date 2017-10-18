/*
 * Execute Functions (Opcode functions)
 *
 */

#ifndef __6502_FUNCTIONS__
#define __6502_FUNCTIONS__

#include "cpu.h"

/***************************
 * STORAGE                 *
 * *************************/

void execute_LDA(enum MODES address_mode, size_t operand);
void execute_LDX(enum MODES address_mode, size_t operand);
void execute_LDY(enum MODES address_mode, size_t operand);
void execute_STA(size_t operand);
void execute_STX(size_t operand);
void execute_STY(size_t operand);
void execute_TAX(void);
void execute_TAY(void);
void execute_TSX(void);
void execute_TXA(void);
void execute_TXS(void);
void execute_TYA(void);

/***************************
 * MATH                    *
 * *************************/

void execute_ADC(enum MODES address_mode, size_t operand);
void execute_DEC(size_t operand);
void execute_DEX(void);
void execute_DEY(void);
void execute_INC(size_t operand);
void execute_INX(void);
void execute_INY(void);
void execute_SBC(enum MODES address_mode, size_t operand);

/***************************
 * BITWISE                 *
 * *************************/

void execute_AND(enum MODES address_mode, size_t operand);
void execute_ASL(enum MODES address_mode, size_t operand);
void execute_BIT(size_t operand);
void execute_EOR(enum MODES address_mode, size_t operand);
void execute_LSR(enum MODES address_mode, size_t operand);
void execute_ORA(enum MODES address_mode, size_t operand);
void execute_ROL(enum MODES address_mode, size_t operand);
void execute_ROR(enum MODES address_mode, size_t operand);

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
void execute_JMP(size_t operand);
void execute_JSR(size_t operand);
void execute_RTI(void);
void execute_RTS(void);

/***************************
 * Registers               *
 * *************************/

void execute_CLC(void);
void execute_CLD(void);
void execute_CLI(void);
void execute_CLV(void);
void execute_CMP(enum MODES address_mode, size_t operand);
void execute_CPX(enum MODES address_mode, size_t operand);
void execute_CPY(enum MODES address_mode, size_t operand);
void execute_SEC(CPU_6502 *NESCPU);
void execute_SED(CPU_6502 *NESCPU);
void execute_SEI(CPU_6502 *NESCPU);

/***************************
 * SYSTEM                  *
 * *************************/

void execute_BRK(void);
void execute_NOP(void);

#endif /* __6502_FUNCTIONS__ */
