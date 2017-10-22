/* Reads NES ROM and executes instructions we encounter */

#include "functions_generic.h"
#include "functions.h"
#include "opcode_debug.h"
#include <stdio.h>

const char *filename = "Balloon_Fight.nes";
/* for nestest.mes sey NES_CPU(0x) */

int main(int argc, char **argv)
{	
	NES = NES_CPU(0x00); /* haven't implimented ROM loading thus PC needs to = 0 */

	/* can turn this into a load ROM function */
	FILE *f = fopen(filename, "rb");
	if (f == NULL) {
		fprintf(stderr, "Error: Couldn't open file\n");
		exit (8);
	}
	
	/* Getting file size */
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);

	unsigned char *buffer = malloc(fsize);
	fread(buffer, fsize, 1, f);
	fclose(f);

	NES->P = 0x24;
	/* an arbituairly high numb */
	int i = 0;
	while (i < 123) {
		Debug_6502(buffer, &NES->PC);
		RET_NES_CPU();
		++i;
	}
	return 0;
}
