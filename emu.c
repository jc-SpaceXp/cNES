/* Reads NES ROM and executes instructions we encounter */

#include <stdio.h>
#include "functions_generic.h"
#include "functions.h"
#include "opcode_execute.h"
#include "cart.h"

const char *filename = "nestest.nes";
/* for nestest.mes sey NES_CPU(0x) */

int main(int argc, char **argv)
{	
	NES = NES_CPU(0x8000); /* haven't implimented ROM loading thus PC needs to = 0 */

	Cartridge* cart = malloc(sizeof(Cartridge));
	load_cart(cart, filename);
	free(cart);

	int i = 0;
	while (i < 123) {
		Execute_6502(NES->PC);
		RET_NES_CPU();
		++i;
	}
	return 0;
}
