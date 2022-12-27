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


void clock_all_units(Cpu6502* cpu, Ppu2C02* ppu, Sdl2DisplayOutputs* cnes_windows, const bool no_logging)
{
	// 3 : 1 PPU to CPU ratio
	clock_cpu(cpu, no_logging);
	clock_ppu(ppu, cpu, cnes_windows, no_logging);
	clock_ppu(ppu, cpu, cnes_windows, no_logging);
	clock_ppu(ppu, cpu, cnes_windows, no_logging);
}

void emu_usuage(const char* program_name)
{
	fprintf(stderr, "\nUSAGE: %s [options]\n", program_name);
	fprintf(stderr, "OPTIONS:\n");
	fprintf(stderr, "\t-h\n\tShows all the possible command-line options\n\n");
	fprintf(stderr, "\t-l\n\tEnable logging to a file\n\n");
	fprintf(stderr, "\t-s\n\tSuppress logging to file or terminal\n\n");
	fprintf(stderr, "\t-o FILE\n\tOpen the provided file\n\n");
	fprintf(stderr, "\t-c CYCLES\n\tRun the CPU up to the specified number of cycles\n\n");
	fprintf(stderr, "\t-u UI_SCALE_FACTOR\n\tScaling factor (integer) to be applied to the displayed output\n");
}

void process_player_1_input(SDL_Event e, Cpu6502* cpu)
{
	// detect player 1 key presses
	switch (e.type) {
	case SDL_KEYDOWN:
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
		break; // SDL_KEYDOWN
	case SDL_KEYUP:
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
		break; // SDL_KEYUP
	}
}

void process_window_events(SDL_Event e, Sdl2Display* cnes_screen)
{
	if ((cnes_screen) && (e.window.windowID == cnes_screen->window_id)) {
		switch (e.type) {
		case SDL_WINDOWEVENT:
			switch (e.window.event) {
			case SDL_WINDOWEVENT_CLOSE:
				SDL_DestroyRenderer(cnes_screen->renderer);
				SDL_DestroyWindow(cnes_screen->window);
				cnes_screen->window = NULL;
				break;
			default:
				break;
			}
			break; // SDL_WINDOWEVENT
		}
	}
}

int main(int argc, char** argv)
{
	int ret = -1;

	const char* program_name = "cnes";
	char* filename = "dummy.nes";  // default: forces user to submit a file to open
	unsigned long max_cycles = 0;
	bool help = false;
	bool log_to_file = false;
	bool no_logging = false;
	int ui_scale_factor = 1;

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
			// disable the side effect of overwritting the log file when switching to release builds
			// and providing the -l option
#ifdef __DEBUG__
			log_to_file = true;
#endif /* __DEBUG__ */
			break;
		case 's': // s - silent output (no instructionlogging, either to terminal or file)
			no_logging = true;
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
		case 'u': // u - change ui scale factor of emulator
			if (argc < 3 || (argv[2][0] == '-')) {
				fprintf(stderr, "Please provide an integer\n");
				help = true;
				break;
			}
			--argc;
			++argv;
			ui_scale_factor = atoi(&argv[1][0]);
			break;
		}
		// increment argv and decrement argc
		--argc;
		++argv;
	}

	// only display help if the user requested it (or if they didn't supply a filename too)
	if (help) {
		emu_usuage(program_name);
		goto early_return;
	}

#define __RESET__

	Cartridge* cart = cart_init();
	CpuMapperShare* cpu_mapper = cpu_mapper_init(cart);
	CpuPpuShare* cpu_ppu = mmio_init();
	Cpu6502* cpu = cpu_init(0xC000, cpu_ppu, cpu_mapper);
	Ppu2C02* ppu = ppu_init(cpu_ppu);
	Sdl2DisplayOutputs cnes_windows;
	cnes_windows.cnes_main = sdl2_display_allocator();

	if (!cart || !cpu_mapper || !cpu_ppu || !cpu || !ppu || !cnes_windows.cnes_main) {
		goto program_exit;
	}

	if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "Failed to initialise the SDL library: %s\n", SDL_GetError());
	}

	if (screen_init(cnes_windows.cnes_main, "cNES"
	               , DEFAULT_WIDTH, DEFAULT_HEIGHT, ui_scale_factor)) {
		fprintf(stderr, "Error when initialsing the SDL2 display\n");
	}

#ifdef __DEBUG__
	cnes_windows.cnes_nt_viewer = sdl2_display_allocator();
	if (!cnes_windows.cnes_nt_viewer) {
		goto program_exit;
	}
	if (screen_init(cnes_windows.cnes_nt_viewer, "cNES Nametable Viewer"
	               , DEFAULT_WIDTH * 2, DEFAULT_HEIGHT * 2, ui_scale_factor)) {
		fprintf(stderr, "Error when initialsing the SDL2 display\n");
	}
#endif /* __DEBUG__ */

	if (parse_nes_cart_file(cart, filename, cpu, ppu)) {
		goto program_exit;
	}

	init_pc(cpu); // Initialise PC to reset vector
	update_cpu_info(cpu);

	if (log_to_file) {
		stdout = freopen("trace_log.txt", "w", stdout);
	}

	// run for a fixed number of cycles if specified by the user
	if (max_cycles) {
		while (cpu->cycle < max_cycles) {
			clock_all_units(cpu, ppu, &cnes_windows, no_logging);
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
					process_player_1_input(e, cpu);

					process_window_events(e, cnes_windows.cnes_main);
#ifdef __DEBUG__
					process_window_events(e, cnes_windows.cnes_nt_viewer);
#endif/* __DEBUG__ */
				}
			}
			clock_all_units(cpu, ppu, &cnes_windows, no_logging);
		}
	}

	SDL_Quit();

	//cpu_mem_hexdump_addr_range(cpu, 0x0000, 0x2000);
	//ppu_mem_hexdump_addr_range(ppu, VRAM, 0x0000, 0x2000);

	ret = 0;

program_exit:
	free(cart);
	free(cnes_windows.cnes_main);
#ifdef __DEBUG__
	free(cnes_windows.cnes_nt_viewer);
#endif /* __DEBUG__ */
	free(ppu);
	free(cpu);
	free(cpu_ppu);

early_return:
	return ret;
}
