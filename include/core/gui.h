#ifndef __NES_GUI__
#define __NES_GUI__

#include "SDL2/SDL.h"
#include "gui_fwd.h"

#include <stdint.h>

#define DEFAULT_HEIGHT 240U
#define DEFAULT_WIDTH  256U

struct Sdl2Display {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* framebuffer;

	uint32_t window_id;
};

struct Sdl2DisplayOutputs {
	Sdl2Display* cnes_main;
	Sdl2Display* cnes_nt_viewer;
};

Sdl2Display* sdl2_display_allocator(void);
int screen_init(Sdl2Display* cnes_screen, const char* window_name
                , const unsigned int width, const unsigned int height
                , int scale_factor);
void kill_screen(Sdl2Display* cnes_screen);
void draw_pixels(uint32_t* pixels, const unsigned int width, Sdl2Display* cnes_screen); // Draws frame to screen

#endif /* __NES_GUI__ */
