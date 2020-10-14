/* NES emulator executes here */

#include "emu.h"
#include "cart.h"
#include "cpu.h"
#include "ppu.h"
#include "gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_keycode.h>

#define A_BUTTON      0x01U
#define B_BUTTON      0x02U
#define SELECT_BUTTON 0x04U
#define START_BUTTON  0x08U
#define UP_BUTTON     0x10U
#define DOWN_BUTTON   0x20U
#define LEFT_BUTTON   0x40U
#define RIGHT_BUTTON  0x80U

void clock_all_units(Cpu6502* CPU, PPU_Struct* PPU, Display* nes_screen)
{
	PPU->old_cycle = PPU->cycle;

	// 3 : 1 PPU to CPU ratio
	clock_cpu(CPU);
	clock_ppu(PPU, CPU, nes_screen);
	clock_ppu(PPU, CPU, nes_screen);
	clock_ppu(PPU, CPU, nes_screen);
}

void usuage(const char* program_name)
{
	fprintf(stderr, "\nUSAGE: %s [options]\n", program_name);
	fprintf(stderr, "OPTIONS:\n");
	fprintf(stderr, "\t-h\n\tShows all the possible command-line options\n\n");
	fprintf(stderr, "\t-l\n\tEnable logging to a file\n\n");
	fprintf(stderr, "\t-o FILE\n\tOpen the provided file\n\n");
	fprintf(stderr, "\t-c CYCLES\n\tRun the CPU up to the specified number of cycles\n");
}

int main(int argc, char** argv)
{
	int ret = -1;

	const char* program_name = "cnes";
	char* filename = "dummy.nes";  // default: forces user to submit a file to open
	unsigned long max_cycles = 0;
	bool help = false;
	bool log = false;

	// process command line arguments
	while ((argc > 1) && (argv[1][0] == '-')) {
		// make sure -x isn't the same as -xxxxxxx (where x is any command line option)
		if (strlen(argv[1]) > 2) {
			fprintf(stderr, "Command line option must be a single character when using the '-' option\n");
			break;
		}

		switch (argv[1][1]) {
		case 'h': // h - display help message
			help = true;
			break;
		case 'o': // o - open file
			if (argc < 3 || (argv[2][0] == '-')) {
				fprintf(stderr, "Please provide a filename\n");
				help = true;
				break;
			}
			--argc;
			++argv;
			filename = &argv[1][0];
			break;
		case 'l': // l - enable logging to a file
			log = true;
			break;
		case 'c': // c - execute emulator for a fixed number of cpu clock cycles
			if (argc < 3 || (argv[2][0] == '-')) {
				fprintf(stderr, "Please provide an unsigned integer\n");
				help = true;
				break;
			}
			--argc;
			++argv;
			max_cycles = atoi(&argv[1][0]);
			break;
		}
		// increment argv and decrement argc
		--argc;
		++argv;
	}

	// only display help if the user requested it (or if they didn't supply a filename too)
	if (help) {
		usuage(program_name);
		goto early_return;
	}

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

	if (log) {
		stdout = freopen("trace_log.txt", "w", stdout);
	}

	Display* nes_screen = screen_init();
	if (!nes_screen)
		goto program_exit;


	// run for a fixed number of cycles if specified by the user
	if (max_cycles) {
		while (CPU->Cycle < max_cycles) {
			clock_all_units(CPU, PPU, nes_screen);
		}
	} else {
		/* SDL GAME LOOOOOOP */
		int quit = 0;
		SDL_Event e;
		while (!quit) {
			if (PPU->cpu_ppu_io->ppu_status & 0x80) { // process input when ppu is idle
				while (SDL_PollEvent(&e)) {
					if (e.type == SDL_QUIT) {
						quit = 1;
					}

					// detect player 1 key presses (roll into its own function)
					switch (e.type) {
					case SDL_KEYDOWN:
						// store into array and set to 1
						switch (e.key.keysym.sym) {
						case SDLK_m:
							CPU->player_1_controller |= A_BUTTON;
							break;
						case SDLK_n:
							CPU->player_1_controller |= B_BUTTON;
							break;
						case SDLK_q:
							CPU->player_1_controller |= SELECT_BUTTON;
							break;
						case SDLK_e:
							CPU->player_1_controller |= START_BUTTON;
							break;
						case SDLK_w:
							CPU->player_1_controller |= UP_BUTTON;
							break;
						case SDLK_s:
							CPU->player_1_controller |= DOWN_BUTTON;
							break;
						case SDLK_a:
							CPU->player_1_controller |= LEFT_BUTTON;
							break;
						case SDLK_d:
							CPU->player_1_controller |= RIGHT_BUTTON;
							break;
						default:
							break;
						}
						break;
					case SDL_KEYUP:
						// store into array and set to 0
						switch (e.key.keysym.sym) {
						case SDLK_m:
							CPU->player_1_controller &= ~A_BUTTON;
							break;
						case SDLK_n:
							CPU->player_1_controller &= ~B_BUTTON;
							break;
						case SDLK_q:
							CPU->player_1_controller &= ~SELECT_BUTTON;
							break;
						case SDLK_e:
							CPU->player_1_controller &= ~START_BUTTON;
							break;
						case SDLK_w:
							CPU->player_1_controller &= ~UP_BUTTON;
							break;
						case SDLK_s:
							CPU->player_1_controller &= ~DOWN_BUTTON;
							break;
						case SDLK_a:
							CPU->player_1_controller &= ~LEFT_BUTTON;
							break;
						case SDLK_d:
							CPU->player_1_controller &= ~RIGHT_BUTTON;
							break;
						default:
							break;
						}
						break;
					}
				}
			}
			clock_all_units(CPU, PPU, nes_screen);
		}
	}

	screen_clear(nes_screen);
	nes_screen = NULL;

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

early_return:
	return ret;
}
