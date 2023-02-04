#ifndef __MAPPERS__
#define __MAPPERS__

#include "cart_fwd.h"
#include "cpu_fwd.h"
#include "ppu_fwd.h"

#include <stdint.h>

void mapper_write(Cpu6502* cpu, uint16_t addr, uint8_t val);
void init_mapper(Cartridge* cart, Cpu6502* cpu, Ppu2C02* ppu);

#endif /* __MAPPERS__ */
