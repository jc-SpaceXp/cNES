#include "gui.h"

#include <stdlib.h>
#include <stdio.h>


const unsigned SCREEN_HEIGHT = 240;
const unsigned SCREEN_WIDTH = 256;

Sdl2Display* sdl2_display_allocator(void)
{
	Sdl2Display* cnes_screen = malloc(sizeof(Sdl2Display));
	if (!cnes_screen) {
		fprintf(stderr, "Failed to allocate enough memory for Sdl2Display struct\n");
	}

	return cnes_screen; // either valid or NULL
}

int screen_init(Sdl2Display* cnes_screen, const char* window_name, int scale_factor)
{
	int error_code = 0;
	cnes_screen->window = NULL;
	cnes_screen->renderer = NULL;
	cnes_screen->framebuffer = NULL;

	if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "Failed to initialise the SDL library: %s\n", SDL_GetError());
		error_code = 1;
	}

	cnes_screen->window = SDL_CreateWindow(window_name
	                                      , SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED
	                                      , SCREEN_WIDTH * scale_factor
	                                      , SCREEN_HEIGHT * scale_factor, SDL_WINDOW_SHOWN);
	if (cnes_screen->window == NULL) {
		fprintf(stderr, "SDL failed to create window: %s\n", SDL_GetError());
		error_code = 1;
	}

	cnes_screen->renderer = SDL_CreateRenderer(cnes_screen->window, -1
	                                          , SDL_RENDERER_ACCELERATED
	                                            | SDL_RENDERER_PRESENTVSYNC);
	if (cnes_screen->renderer == NULL) {
		fprintf(stderr, "SDL failed to create 2D renderer for the window: %s\n", SDL_GetError());
		error_code = 1;
	}

	cnes_screen->framebuffer = SDL_CreateTexture(cnes_screen->renderer, SDL_PIXELFORMAT_ARGB8888
	                                            , SDL_TEXTUREACCESS_STREAMING
	                                            , SCREEN_WIDTH, SCREEN_HEIGHT);
	if (cnes_screen->framebuffer == NULL) {
		fprintf(stderr, "SDL failed to create a texture for the window: %s\n", SDL_GetError());
		error_code = 1;
	}

	// Apply scaling factor
	if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest") == SDL_FALSE) {
		fprintf(stderr, "SDL failed to set a scaling method/quality for the renderer: %s\n", SDL_GetError());
		error_code = 1;
	}

	// preserve original aspect ratio
	if (SDL_RenderSetLogicalSize(cnes_screen->renderer, SCREEN_WIDTH, SCREEN_HEIGHT)) {
		fprintf(stderr, "SDL failed to set a new resolution for the renderer: %s\n", SDL_GetError());
		error_code = 1;
	}

	if (SDL_RenderSetScale(cnes_screen->renderer, scale_factor, scale_factor)) {
		fprintf(stderr, "SDL failed to apply a scaling factor to the renderer: %s\n", SDL_GetError());
		error_code = 1;
	}

	return error_code;
}

void screen_clear(Sdl2Display* nes)
{
	SDL_DestroyRenderer(nes->renderer);
	SDL_DestroyWindow(nes->window);
	SDL_Quit();
}

void draw_pixels(uint32_t* pixels, Sdl2Display* nes)
{
	SDL_UpdateTexture(nes->framebuffer, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));

	SDL_RenderClear(nes->renderer);
	SDL_RenderCopy(nes->renderer, nes->framebuffer, NULL, NULL);
	SDL_RenderPresent(nes->renderer);
}
