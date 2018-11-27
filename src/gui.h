#ifndef __NES_GUI__
#define __NES_GUI__

#include "SDL2/SDL.h"
#include <stdint.h>
#include <stdlib.h>

const unsigned SCREEN_WIDTH;
const unsigned SCREEN_HEIGHT;
uint32_t pixels[256 * 240];

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* framebuffer;
} Display;

Display* nes_screen;
Display* screen_init(void);
void screen_clear(Display* nes);
//int SDL_init(SDL_Window **window, SDL_Renderer **renderer, SDL_Texture **framebuffer);
void draw_pixels(uint32_t* pixels, Display* nes); // Draws frame to screen
/*
void draw_texture(uint32_t *pixels, Display *nes); // Draws frame to screen
void draw_display(Display *nes); // Draws frame to screen
*/

#endif /* __NES_GUI__ */
