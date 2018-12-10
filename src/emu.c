/* NES emulator executes here */

#include <stdio.h>
#include "helper_functions.h"
#include "opcode_functions.h"
#include "opcode_table.h"
#include "cart.h"
#include "ppu.h"

//const char* filename = "nestest.nes";
const char* filename = "super_mario_bros.nes";
//const char* filename = "donkey_kong.nes";
//const char* filename = "balloon.nes";
//const char* filename = "nmi.nes";

#define __DEBUG__ // Add print statements for each instruction
#define __LOG__

/* comment out above or uncomment below to disable logging to a file
#undef __LOG__
*/

void ppu_cpu_ratio(void)
{
	PPU->old_cycle = PPU->cycle;
	update_cpu_info(CPU);

	// 3 : 1 PPU to CPU ratio
	cpu_step(CPU->PC, CPU);
	for (unsigned i = 0; i < (CPU->Cycle - CPU->old_Cycle); i++) {
		ppu_step(PPU, CPU);
		ppu_step(PPU, CPU);
		ppu_step(PPU, CPU);
	}
#ifdef __DEBUG__
	log_cpu_info(CPU);
	append_ppu_info();
#endif /* __DEBUG__ */
}

int main(int argc, char** argv)
{
#define __RESET__
	CPU = cpu_init(0xC000);
	PPU = ppu_init();

	Cartridge* cart = malloc(sizeof(Cartridge));
	load_cart(cart, filename, CPU, PPU);
	free(cart);
	cart = NULL;

	set_pc(CPU); /* Set PC to reset vector */

#ifdef __LOG__
	stdout = freopen("trace_log.txt", "w", stdout);
#endif /*__LOG__ */

	unsigned i = 0;
	nes_screen = screen_init();

	
	//1000000 for quick SMB1 test
	//while (i < 80000000) { // 5 Frames DK
	while (i < 10000000) { // SMB1 start of demo
	//while (CPU->Cycle < 22040403) { // DK overflow?
	//while (CPU->Cycle < 8198933) { // Balloon fight overflow
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
	screen_clear(nes_screen);
	nes_screen = NULL;

	//PPU_MEM_DEBUG(); // PPU memory viewer
	//cpu_ram_viewer(NES);
	//OAM_viewer(PRIMARY_OAM);
	OAM_viewer(SECONDARY_OAM);
	free(PPU);
	free(CPU);
	return 0;
}
