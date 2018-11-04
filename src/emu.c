/* NES emulator executes here */

#include <stdio.h>
#include "helper_functions.h"
#include "opcode_functions.h"
#include "opcode_table.h"
#include "cart.h"
#include "ppu.h"

//const char *filename = "nestest.nes";
const char *filename = "super_mario_bros.nes";
//const char *filename = "donkey_kong.nes";
//const char *filename = "balloon.nes";
//const char *filename = "nmi.nes";

#define __DEBUG__ // Add print statements for each instruction
#define __LOG__

/* comment out above or uncomment below to disable logging to a file
#undef __LOG__
*/

// Can make function take aan enum argument which takes NTSC or PAL and adjust CPU:PPU
// appropriately
void ppu_cpu_ratio(void)
{
	// 3 : 1 PPU to CPU ratio
	PPU->old_cycle = PPU->cycle;
	ppu_step(PPU, NES);
	ppu_step(PPU, NES);
	ppu_step(PPU, NES);
	CPU_6502_STEP(NES->PC);
	for (int i = 1; i < (NES->Cycle - NES->old_Cycle); i++) {
		ppu_step(PPU, NES);
		ppu_step(PPU, NES);
		ppu_step(PPU, NES);
	}
#ifdef __DEBUG__
	RET_NES_CPU();
	append_ppu_info();
#endif /* __DEBUG__ */
}

int main(int argc, char **argv)
{
	NES = NES_CPU(0xC000);
#define __RESET__
	PPU = ppu_init();

	Cartridge* cart = malloc(sizeof(Cartridge));
	load_cart(cart, filename, PPU);
	free(cart);
	NES_PC(NES); /* Set PC to reset vector */
	// Can combine above code into a NES init function

#ifdef __LOG__
	stdout = freopen("trace_log.txt", "w", stdout);
#endif /*__LOG__ */

	int i = 0;
	//5005 - nestest, 104615 - SMB1, 42360 Donkey Kong @ 5 frames
	//NES->PC = 0xC000; //nes test
	nes_screen = screen_init();

	while (i < 419123) { // 5 Frames DK
		ppu_cpu_ratio();
		++i;
	}
	//OLD LOOOOP PRIOR TO SDL

	/* SDL LOOOOOOP */
	/*
	SDL_init();
	int quit = 0;
	SDL_Event e;
	while (!quit) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
		}
		ppu_cpu_ratio();
		printf(" PPU_CYC: %-3d", PPU->old_cycle);
		printf(" SL: %d\n", (PPU->scanline));
	}

	*/
	PPU_MEM_DEBUG(); // PPU memory viewer
	SDL_Delay(5000);
	SDL_Quit();
	return 0;
}
