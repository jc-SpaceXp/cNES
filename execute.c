/* Reads NES ROM and executes instructions we encounter */

#include "execute.h"

const char *filename = "Balloon_Fight.nes";

int main(int argc, char **argv)
{	
	NES = NES_CPU(0);  /* Initialise CPU - causes error */
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

	while (NES->PC < 220) {
		printf("NES PC = %d\n", NES->PC);
		Execute_6502(buffer, &NES->PC);
	}
	return 0;
}
