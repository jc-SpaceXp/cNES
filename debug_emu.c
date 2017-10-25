/* Reads NES ROM and executes instructions we encounter */

#include <stdio.h>
#include "functions_generic.h"
#include "functions.h"
#include "opcode_debug.h"
#include "cart.h"

const char *filename = "nestest.nes";
/* for nestest.mes set NES_CPU(0xC000) */

int main(int argc, char **argv)
{	
	NES = NES_CPU(0xC000);

	Cartridge* cart = malloc(sizeof(Cartridge));
	load_cart(cart, filename);
	free(cart);

	int i = 0;
	while (i < 123) {
		Debug_6502(NES->PC);
		RET_NES_CPU();
		++i;
	}
	return 0;
}
