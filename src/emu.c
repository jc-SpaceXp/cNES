/* NES emulator executes here */

#include <stdio.h>
#include "helper_functions.h"
#include "opcode_functions.h"
#include "opcode_table.h"
#include "cart.h"
#include "ppu.h"

const char* filename = "milk_nuts.nes";
//const char* filename = "nestest.nes";
//const char* filename = "super_mario_bros.nes";
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

int main(void)
{
	int ret = -1;
	Cartridge* cart = NULL;
#define __RESET__

	CpuPpuShare* cpu_ppu = mmio_init();
	if (!cpu_ppu)
		goto program_exit;
	CPU = cpu_init(0xC000, cpu_ppu);
	if (!CPU)
		goto program_exit;
	PPU = ppu_init(cpu_ppu);
	if (!PPU)
		goto program_exit;

	cart = malloc(sizeof(Cartridge));
	if (!cart) {
		fprintf(stderr, "Failed to allocate memory for Cartridge\n");
		goto program_exit;
	}
	if (load_cart(cart, filename, CPU, PPU))
		goto program_exit;

	free(cart);
	cart = NULL;

	set_pc(CPU); /* Set PC to reset vector */

#ifdef __LOG__
	stdout = freopen("trace_log.txt", "w", stdout);
#endif /*__LOG__ */

	nes_screen = screen_init();
	if (!nes_screen)
		goto program_exit;

	unsigned i = 0;
	while (i < 10000000) { // SMB1 start of demo
	//while (i < 23507) { // milk and nuts munmap_chunck() error 23500 = no error w/ quit
	//while (CPU->Cycle < 6373063) { // NMI test end
	//while (CPU->Cycle < 22040403) { // DK demo
	//while (CPU->Cycle < 8198933) { // Balloon fight demo
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
	//screen_clear(nes_screen);  //seg fault rn
	//nes_screen = NULL;  //seg fault rn

	//PPU_MEM_DEBUG(); // PPU memory viewer
	//cpu_mem_viewer(CPU);
	//OAM_viewer(PRIMARY_OAM);
	OAM_viewer(SECONDARY_OAM);

	ret = 0;
program_exit:
	free(cart);
	if (nes_screen)
		screen_clear(nes_screen);
	free(nes_screen);
	free(PPU);
	free(CPU);
	free(cpu_ppu);

	return ret;
}
