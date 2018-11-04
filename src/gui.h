#ifndef __NES_GUI__
#define __NES_GUI__

#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

const unsigned SCREEN_WIDTH;
const unsigned SCREEN_HEIGHT;
uint32_t pixels[256 * 240]; // pixels[256][240][3], [3] --> 0 = red, 1 = blue

typedef struct {
	SDL_Window* window;
	//static SDL_Surface* surface;
	SDL_Renderer* renderer;
	SDL_Texture* framebuffer;
} SCREEN;

SCREEN *nes_screen;
SCREEN *screen_init(void);
//int SDL_init(SDL_Window **window, SDL_Renderer **renderer, SDL_Texture **framebuffer);
void draw_pixels(uint32_t *pixels, SCREEN *nes); // Draws frame to screen

#endif /* __NES_GUI__ */
