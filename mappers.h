#ifndef __MAPPERS__
#define __MAPPERS__

#include "cpu.h"

struct Cartridge; /* can't include cart.h as breaks compiler include in .c file */

void mapper_000(struct Cartridge* cart); /* NROM mapper */
void mapper_000_V2(struct Cartridge* cart); /* NROM mapper */

#endif /* __MAPPERS__ */
