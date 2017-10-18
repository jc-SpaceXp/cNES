/* Reads NES ROM and executes instructions we encounter */

#include "functions_generic.h"
#include "functions.h"
#include "opcode_execute.h"
#include <stdio.h>

const char *filename = "Balloon_Fight.nes";

int main(int argc, char **argv)
{	
	NES = NES_CPU(0xC000);  /* Initialise CPU - causes error */
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

	/* an arbituairly high numb */
	int i = 0;
	while (i < 5) {
		Execute_6502(buffer, &NES->PC);
		RET_NES_CPU();
		++i;
	}
	return 0;
}
