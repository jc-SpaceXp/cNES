/* NES emulator executes here */

#include "emu.h"
#include "cart.h"
#include "cpu.h"
#include "ppu.h"
#include "gui.h"

#include <stdio.h>
#include <stdlib.h>

//const char* filename = "milk_nuts.nes";
//const char* filename = "nestest.nes";
//const char* filename = "super_mario_bros.nes";
const char* filename = "donkey_kong.nes";
//const char* filename = "balloon.nes";
//const char* filename = "nmi.nes";

#define __LOG__

/* comment out above or uncomment below to disable logging to a file
#undef __LOG__
*/

void ppu_cpu_ratio(Cpu6502* CPU, PPU_Struct* PPU, Display* nes_screen)
{
	PPU->old_cycle = PPU->cycle;

	// 3 : 1 PPU to CPU ratio
	cpu_step(CPU);
	//printf("CPU %X    PPU: %X\n", CPU->cpu_ppu_io->ppu_mask, PPU->cpu_ppu_io->ppu_mask);
	ppu_step(PPU, CPU, nes_screen);
	ppu_step(PPU, CPU, nes_screen);
	ppu_step(PPU, CPU, nes_screen);
}

int main(void)
{
	int ret = -1;
	Cartridge* cart = NULL;
#define __RESET__

	CpuPpuShare* cpu_ppu = mmio_init();
	if (!cpu_ppu)
		goto program_exit;
	Cpu6502* CPU = cpu_init(0xC000, cpu_ppu);
	if (!CPU)
		goto program_exit;
	PPU_Struct* PPU = ppu_init(cpu_ppu);
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

	init_pc(CPU); // Initialise PC to reset vector
	//CPU->PC = 0xC000; // nestest
	update_cpu_info(CPU);

#ifdef __LOG__
	stdout = freopen("trace_log.txt", "w", stdout);
#endif /*__LOG__ */

	Display* nes_screen = screen_init();
	if (!nes_screen)
		goto program_exit;

#if 1
	unsigned i = 0;
	// 5005 nestest, 104615 SMB1, 42360 Donkey Kong @ 5 frames
	while (i < 10000000) { // SMB1 start of demo
	//while (i < 23507) { // milk and nuts munmap_chunck() error 23500 = no error w/ quit
	//while (i < 200) {
	//while (CPU->Cycle < 6373063) { // NMI test end
	//while (CPU->Cycle < 22040403) { // DK demo
	//while (CPU->Cycle < 8198933) { // Balloon fight demo
		ppu_cpu_ratio(CPU, PPU, nes_screen);
		++i;
	}
#endif

#if 0
	/* SDL LOOOOOOP */
	int quit = 0;
	SDL_Event e;
	while (!quit) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
		}
		ppu_cpu_ratio(CPU, PPU, nes_screen);
	}

	screen_clear(nes_screen);  //seg fault rn
	nes_screen = NULL;  //seg fault rn
#endif

	//cpu_mem_16_byte_viewer(CPU, 0, 2048);
	//ppu_mem_16_byte_viewer(PPU, 0, 2048);
	//OAM_viewer(PPU, PRIMARY_OAM);
	//OAM_viewer(PPU, SECONDARY_OAM);

	ret = 0;

program_exit:
	free(cart);
	if (nes_screen) { screen_clear(nes_screen); }
	free(nes_screen);
	free(PPU);
	free(CPU);
	free(cpu_ppu);

	return ret;
}
