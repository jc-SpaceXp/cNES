/* Reads NES ROM and print's what instructions we encounter */
/* in Numerical order */

#include "opcode_debug.h"

const char *filename = "Balloon_Fight.nes";

int main(int argc, char **argv)
{	
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

	int pc = 0;
	while (pc < fsize) {
		pc += Disassemble6502(buffer, pc);
	}
	return 0;
}
