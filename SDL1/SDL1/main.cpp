#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

extern "C" {
#include "./SDL2-2.0.10/include/SDL.h"
#include "./SDL2-2.0.10/include/SDL_main.h"
}

#ifdef __cplusplus
extern "C"
#endif

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
struct timer {
	int  t1=SDL_GetTicks() , t2, frames=0;
	double delta, worldTime=0, fpsTimer=0, fps=0, distance=0, etiSpeed;
};
struct stan_gry {
	timer time;
	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Surface* eti;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;

};
void funkcjetimer(timer	&time)
{
	time.t2 = SDL_GetTicks();
	time.delta = (time.t2 - time.t1) * 0.001;
	time.t1 = time.t2;

	time.worldTime += time.delta;
	time.fpsTimer += time.delta;
	if (time.fpsTimer > 0.5) {
		time.fps = time.frames * 2;
		time.frames = 0;
		time.fpsTimer -= 0.5;
	};
}
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie

void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};

void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)

void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};

// rysowanie prostokπta o d≥ugoúci bokÛw l i k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};
void inicjalizacja(stan_gry& gra)
{
	int rc;
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,&gra.window, &gra.renderer); //okno i renderer
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear"); //jakosc 
	SDL_RenderSetLogicalSize(gra.renderer, SCREEN_WIDTH, SCREEN_HEIGHT); //skalowanie ekranu
	SDL_SetRenderDrawColor(gra.renderer, 0, 0, 0, 255); //ustaiwenie koloru renderera
	SDL_SetWindowTitle(gra.window, "SNAKE"); //tytul gry
	gra.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000); //tworzenie ekranu i ustalenie parametrÛw
	gra.scrtex = SDL_CreateTexture(gra.renderer, SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,SCREEN_WIDTH, SCREEN_HEIGHT); //tworzenie tekstury
	SDL_ShowCursor(SDL_DISABLE); // wy≥πczenie kursora
	gra.charset = SDL_LoadBMP("./cs8x8.bmp"); //bitmapa do tekstu
	if (gra.charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gra.screen);
		SDL_DestroyTexture(gra.scrtex);
		SDL_DestroyWindow(gra.window);
		SDL_DestroyRenderer(gra.renderer);
		SDL_Quit();
	};
	SDL_SetColorKey(gra.charset, true, 0x000000); //przezroczystosc
}
int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	timer time;
	int quit=0;
	stan_gry gra;
	inicjalizacja(gra);
	char text[128];
	int czarny = SDL_MapRGB(gra.screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(gra.screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(gra.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(gra.screen->format, 0x11, 0x11, 0xCC);
	while (!quit) {
		funkcjetimer(time);
		DrawRectangle(gra.screen, 4, 4, SCREEN_WIDTH - 8, 50, czerwony, niebieski);  //maluje okno wyúwietlania tekstu
		sprintf(text, "Szsablasfsdon drugiego zadania, czas trwania = %.1lf s  %.0lf klatek / s", time.worldTime, time.fps);
		DrawString(gra.screen, gra.screen->w / 2 - strlen(text) * 8 / 2, 10, text, gra.charset);
		sprintf(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie");
		DrawString(gra.screen, gra.screen->w / 2 - strlen(text) * 8 / 2, 26, text, gra.charset);
		SDL_UpdateTexture(gra.scrtex, NULL, gra.screen->pixels, gra.screen->pitch);
		SDL_RenderCopy(gra.renderer, gra.scrtex, NULL, NULL);
		SDL_RenderPresent(gra.renderer);
	}
}