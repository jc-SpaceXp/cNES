#include "gui.h"

#include <stdlib.h>
#include <stdio.h>


const unsigned SCREEN_HEIGHT = 240;
const unsigned SCREEN_WIDTH = 256;

Sdl2Display* screen_init(int scale_factor)
{
	Sdl2Display* nes = malloc(sizeof(Sdl2Display));
	if (!nes) {
		fprintf(stderr, "Failed to allocate enough memory for Display\n");
		return nes;
	}

	nes->window = NULL;
	nes->renderer = NULL;
	nes->framebuffer = NULL;

	if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "Failed to initialise the SDL library: %s\n", SDL_GetError());
	}

	nes->window = SDL_CreateWindow("cNES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED
	                              , SCREEN_WIDTH * scale_factor
	                              , SCREEN_HEIGHT * scale_factor, SDL_WINDOW_SHOWN);
	if (nes->window == NULL) {
		fprintf(stderr, "SDL failed to create window: %s\n", SDL_GetError());
	}

	nes->renderer = SDL_CreateRenderer(nes->window, -1
	                                  , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (nes->renderer == NULL) {
		fprintf(stderr, "SDL failed to create 2D renderer for the window: %s\n", SDL_GetError());
	}

	nes->framebuffer = SDL_CreateTexture(nes->renderer, SDL_PIXELFORMAT_ARGB8888
	                                    , SDL_TEXTUREACCESS_STREAMING
	                                    , SCREEN_WIDTH, SCREEN_HEIGHT);
	if (nes->framebuffer == NULL) {
		fprintf(stderr, "SDL failed to create a texture for the window: %s\n", SDL_GetError());
	}

	// Apply scaling factor
	if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest") == SDL_FALSE) {
		fprintf(stderr, "SDL failed to set a scaling method/quality for the renderer: %s\n", SDL_GetError());
	}

	// preserve original aspect ratio
	if (SDL_RenderSetLogicalSize(nes->renderer, SCREEN_WIDTH, SCREEN_HEIGHT)) {
		fprintf(stderr, "SDL failed to set a new resolution for the renderer: %s\n", SDL_GetError());
	}

	if (SDL_RenderSetScale(nes->renderer, scale_factor, scale_factor)) {
		fprintf(stderr, "SDL failed to apply a scaling factor to the renderer: %s\n", SDL_GetError());
	}

	return nes;
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
