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

void clock_all_units(Cpu6502* cpu, Ppu2A03* ppu, Display* nes_screen)
{
	// 3 : 1 PPU to CPU ratio
	clock_cpu(cpu);
	clock_ppu(ppu, cpu, nes_screen);
	clock_ppu(ppu, cpu, nes_screen);
	clock_ppu(ppu, cpu, nes_screen);
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

#define __RESET__

	Cartridge* cart = cart_init();
	CpuMapperShare* cpu_mapper = cpu_mapper_init(cart);
	CpuPpuShare* cpu_ppu = mmio_init();
	Cpu6502* cpu = cpu_init(0xC000, cpu_ppu, cpu_mapper);
	Ppu2A03* ppu = ppu_init(cpu_ppu);
	Display* nes_screen = screen_init();

	if (!cart || !cpu_mapper || !cpu_ppu || !cpu || !ppu || !nes_screen) {
		goto program_exit;
	}

	if (load_cart(cart, filename, cpu, ppu)) {
		goto program_exit;
	}

	init_pc(cpu); // Initialise PC to reset vector
	update_cpu_info(cpu);

	if (log) {
		stdout = freopen("trace_log.txt", "w", stdout);
	}

	// run for a fixed number of cycles if specified by the user
	if (max_cycles) {
		while (cpu->cycle < max_cycles) {
			clock_all_units(cpu, ppu, nes_screen);
		}
	} else {
		/* SDL GAME LOOOOOOP */
		int quit = 0;
		SDL_Event e;
		while (!quit) {
			if (ppu->cpu_ppu_io->ppu_status & 0x80) { // process input when ppu is idle
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
							cpu->player_1_controller |= A_BUTTON;
							break;
						case SDLK_n:
							cpu->player_1_controller |= B_BUTTON;
							break;
						case SDLK_q:
							cpu->player_1_controller |= SELECT_BUTTON;
							break;
						case SDLK_e:
							cpu->player_1_controller |= START_BUTTON;
							break;
						case SDLK_w:
							cpu->player_1_controller |= UP_BUTTON;
							break;
						case SDLK_s:
							cpu->player_1_controller |= DOWN_BUTTON;
							break;
						case SDLK_a:
							cpu->player_1_controller |= LEFT_BUTTON;
							break;
						case SDLK_d:
							cpu->player_1_controller |= RIGHT_BUTTON;
							break;
						default:
							break;
						}
						break;
					case SDL_KEYUP:
						// store into array and set to 0
						switch (e.key.keysym.sym) {
						case SDLK_m:
							cpu->player_1_controller &= ~A_BUTTON;
							break;
						case SDLK_n:
							cpu->player_1_controller &= ~B_BUTTON;
							break;
						case SDLK_q:
							cpu->player_1_controller &= ~SELECT_BUTTON;
							break;
						case SDLK_e:
							cpu->player_1_controller &= ~START_BUTTON;
							break;
						case SDLK_w:
							cpu->player_1_controller &= ~UP_BUTTON;
							break;
						case SDLK_s:
							cpu->player_1_controller &= ~DOWN_BUTTON;
							break;
						case SDLK_a:
							cpu->player_1_controller &= ~LEFT_BUTTON;
							break;
						case SDLK_d:
							cpu->player_1_controller &= ~RIGHT_BUTTON;
							break;
						default:
							break;
						}
						break;
					}
				}
			}
			clock_all_units(cpu, ppu, nes_screen);
		}
	}

	screen_clear(nes_screen);
	nes_screen = NULL;

	//cpu_mem_16_byte_viewer(cpu, 0, 2048);
	//ppu_mem_16_byte_viewer(ppu, 0, 2048);
	//OAM_viewer(ppu, PRIMARY_OAM);
	//OAM_viewer(ppu, SECONDARY_OAM);

	ret = 0;

program_exit:
	free(cart);
	free(nes_screen);
	free(ppu);
	free(cpu);
	free(cpu_ppu);

early_return:
	return ret;
}
