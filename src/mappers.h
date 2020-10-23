#ifndef __MAPPERS__
#define __MAPPERS__

#include "extern_structs.h"

void set_prg_rom_bank_1(Cpu6502* cpu, unsigned prg_bank_offset, unsigned kib_size);
void set_prg_rom_bank_2(Cpu6502* cpu, unsigned prg_bank_offset);
void set_chr_bank_1(Cpu6502* cpu, unsigned chr_bank_offset, unsigned kib_size);
void set_chr_bank_2(Cpu6502* cpu, unsigned chr_bank_offset);
void mapper_write(Cpu6502* cpu, uint16_t addr, uint8_t val);
void change_nt_mirroring(Cpu6502* cpu);
void init_mapper(Cartridge* cart, Cpu6502* cpu, Ppu2A03* ppu);

void mapper_000(struct Cartridge* cart, Cpu6502* cpu, Ppu2A03* ppu); /* NROM mapper */


void mmc1_reg_write(Cpu6502* cpu, uint16_t addr, uint8_t val);
void mapper_001(struct Cartridge* cart, Cpu6502* cpu, Ppu2A03* ppu); /* MMC1 mapper */
//void mapper_003(struct Cartridge* cart); /* CROM mapper */

#endif /* __MAPPERS__ */
