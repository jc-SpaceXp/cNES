#include "gui.h"

#include <stdlib.h>
#include <stdio.h>


const unsigned SCREEN_HEIGHT = 240;
const unsigned SCREEN_WIDTH = 256;

Display* screen_init(int scale_factor)
{
	Display* nes = malloc(sizeof(Display));
	if (!nes) {
		fprintf(stderr, "Failed to allocate enough memory for Display\n");
		return nes;
	}

	nes->window = NULL;
	nes->renderer = NULL;
	nes->framebuffer = NULL;

	SDL_Init(SDL_INIT_VIDEO);
	nes->window = SDL_CreateWindow("cNES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	              SCREEN_WIDTH * scale_factor, SCREEN_HEIGHT * scale_factor, SDL_WINDOW_SHOWN);
	nes->renderer = SDL_CreateRenderer(nes->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	nes->framebuffer = SDL_CreateTexture(nes->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	nes->framebuffer = SDL_CreateTexture(nes->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Apply scaling factor
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	SDL_RenderSetLogicalSize(nes->renderer, SCREEN_WIDTH, SCREEN_HEIGHT); // preserve original aspect ratio
	SDL_RenderSetScale(nes->renderer, scale_factor, scale_factor);
	return nes;
}

void screen_clear(Display* nes)
{
	SDL_DestroyRenderer(nes->renderer);
	SDL_DestroyWindow(nes->window);
	SDL_Quit();
}

void draw_pixels(uint32_t* pixels, Display* nes)
{
	SDL_UpdateTexture(nes->framebuffer, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));

	SDL_RenderClear(nes->renderer);
	SDL_RenderCopy(nes->renderer, nes->framebuffer, NULL, NULL);
	SDL_RenderPresent(nes->renderer);
}

/*
void draw_texture(uint32_t *pixels, Display *nes)
{
	SDL_UpdateTexture(nes->framebuffer, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
}

void draw_display(Display *nes)
{
	SDL_RenderClear(nes->renderer);
	SDL_RenderCopy(nes->renderer, nes->framebuffer, NULL, NULL);
	SDL_RenderPresent(nes->renderer);
}
*/
