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

void ppu_cpu_ratio(void)
{
	// 3 : 1 PPU to CPU ratio
	PPU->old_cycle = PPU->cycle;
	ppu_step(PPU, NES);
	ppu_step(PPU, NES);
	ppu_step(PPU, NES);
	cpu_step(NES->PC);
	for (unsigned i = 1; i < (NES->Cycle - NES->old_Cycle); i++) {
		ppu_step(PPU, NES);
		ppu_step(PPU, NES);
		ppu_step(PPU, NES);
	}
#ifdef __DEBUG__
	log_cpu_info();
	append_ppu_info();
#endif /* __DEBUG__ */
}

int main(int argc, char **argv)
{
#define __RESET__
	NES = cpu_init(0xC000);
	PPU = ppu_init();

	Cartridge* cart = malloc(sizeof(Cartridge));
	load_cart(cart, filename, NES, PPU);
	free(cart);

	set_pc(NES); /* Set PC to reset vector */

#ifdef __LOG__
	stdout = freopen("trace_log.txt", "w", stdout);
#endif /*__LOG__ */

	int i = 0;
	nes_screen = screen_init();

	
	while (i < 1000000) { // 5 Frames DK
	//while (NES->Cycle < 980972) { // 5 Frames DK
		ppu_cpu_ratio();
		++i;
	}

	/* SDL LOOOOOOP */
	/*
	nes_screen = screen_init();
	int quit = 0;
	SDL_Event e;
	while (!quit) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
		}
		ppu_cpu_ratio();
	}
	*/

	//PPU_MEM_DEBUG(); // PPU memory viewer
	//cpu_ram_viewer();
	//OAM_viewer(PRIMARY_OAM);
	OAM_viewer(SECONDARY_OAM);
	free(PPU);
	free(NES);
	SDL_Delay(5000);
	SDL_Quit();
	return 0;
}
