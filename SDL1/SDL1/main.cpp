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

#define SCREEN_WIDTH	1280
#define SCREEN_HEIGHT	720
struct timer {
	int  t1=SDL_GetTicks() , t2, frames=0;
	double delta, worldTime=0, fpsTimer=0, fps=0, distance=0, snake_speed=0.01, snake_speed_licznik=0;
	double czas_zmiany = 0.9, licznik_zmiany = 0;
};
struct czesci_weza {
	int x=500-90;
	int y=300;
	int kierunek=1;
};
struct stan_gry {
	timer time;
	SDL_Event event;
	SDL_Surface* screen, * charset, *obramowanie;
	SDL_Surface* eti, *eti2;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	int quit = 0;
	struct {
		int pozycja_x=500;
		int pozycja_y=300;
		int kierunek = 1;
		int aktualny_rozmiar=1;
		czesci_weza cialo_weza[1];
	}snake;

};
void funkcjetimer(stan_gry	&gra)
{
	gra.time.t2 = SDL_GetTicks();
	gra.time.delta = (gra.time.t2 - gra.time.t1) * 0.001;
	gra.time.t1 = gra.time.t2;
	gra.time.snake_speed_licznik += gra.time.delta;
	gra.time.worldTime += gra.time.delta;
	gra.time.fpsTimer += gra.time.delta;
	if (gra.time.fpsTimer > 0.5) {
		gra.time.fps = gra.time.frames * 2;
		gra.time.frames = 0;
		gra.time.fpsTimer -= 0.5;
	};
	gra.time.frames++;
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
	gra.obramowanie = SDL_LoadBMP("./obramowanie.bmp"); //bitmapa do tekstu
	if (gra.obramowanie == NULL) {
		printf("SDL_LoadBMP(obramowanie.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gra.screen);
		SDL_DestroyTexture(gra.scrtex);
		SDL_DestroyWindow(gra.window);
		SDL_DestroyRenderer(gra.renderer);
		SDL_Quit();
	};
	SDL_SetColorKey(gra.obramowanie, true, 0x000000); //przezroczystosc
	gra.eti = SDL_LoadBMP("./eti.bmp"); //bitmapa do tekstu
	if (gra.eti == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gra.screen);
		SDL_DestroyTexture(gra.scrtex);
		SDL_DestroyWindow(gra.window);
		SDL_DestroyRenderer(gra.renderer);
		SDL_Quit();
	};
	gra.eti2 = SDL_LoadBMP("./eti.bmp"); //bitmapa do tekstu
	if (gra.eti2 == NULL) {
		printf("SDL_LoadBMP(eti2.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gra.screen);
		SDL_DestroyTexture(gra.scrtex);
		SDL_DestroyWindow(gra.window);
		SDL_DestroyRenderer(gra.renderer);
		SDL_Quit();
	};
}
void zdarzenie(stan_gry& gra)
{
	while (SDL_PollEvent(&gra.event)) {
		switch (gra.event.type) {
		case SDL_KEYDOWN:
			if (gra.event.key.keysym.sym == SDLK_ESCAPE) gra.quit = 1;
			else if (gra.event.key.keysym.sym == SDLK_n)
			{
				gra.snake.pozycja_x = 0;
				gra.time.worldTime = 0;
				gra.snake.kierunek = 1;
			}
			else if (gra.event.key.keysym.sym == SDLK_d)
			{
				gra.snake.kierunek = 1;
			}
			else if (gra.event.key.keysym.sym == SDLK_a)
			{
				gra.snake.kierunek = 2;
			}
			else if (gra.event.key.keysym.sym == SDLK_w)
			{
				gra.snake.kierunek = 3;
			}
			else if (gra.event.key.keysym.sym == SDLK_s)
			{
				gra.snake.kierunek = 4;
			}
			break;
		case SDL_QUIT:
			gra.quit = 1;
			break;
		};
	};
}
void zmiana_pozycji(stan_gry &gra)
{
	if (gra.time.snake_speed_licznik >= gra.time.snake_speed)
	{
		if (gra.snake.kierunek == 1)
		{
			if (gra.snake.pozycja_x + gra.eti->w / 2 == 1270)
			{
				if (gra.snake.pozycja_y - gra.eti->h / 2 == 610)
					gra.snake.kierunek = 3;
				else
					gra.snake.kierunek = 4;
			}
			else
				gra.snake.pozycja_x++;
		}
		else if (gra.snake.kierunek == 2)
		{
			if (gra.snake.pozycja_x - gra.eti->w / 2 == 10)
			{
				if (gra.snake.pozycja_y - gra.eti->h / 2 == 66)
					gra.snake.kierunek = 4;
				else
					gra.snake.kierunek = 3;

			}
			else
				gra.snake.pozycja_x--;
		}
		else if (gra.snake.kierunek == 3)
		{
			if (gra.snake.pozycja_y - gra.eti->h / 2 == 66)
			{
				if (gra.snake.pozycja_x + gra.eti->w / 2 == 1270)
					gra.snake.kierunek = 2;
				else
					gra.snake.kierunek = 1;

			}
			else
				gra.snake.pozycja_y--;
		}
		else if (gra.snake.kierunek == 4)
		{
			if (gra.snake.pozycja_y - gra.eti->h / 2 == 610)
			{
				if (gra.snake.pozycja_x - gra.eti->w / 2 == 10)
					gra.snake.kierunek = 1;
				else
					gra.snake.kierunek = 2;

			}
			else
				gra.snake.pozycja_y++;
		}
	}
}
void zmiana_pozycji_ciala(stan_gry& gra)
{
	if (gra.time.snake_speed_licznik >= gra.time.snake_speed)
	{
		
		if (gra.snake.cialo_weza->kierunek == 1)
		{
			if (gra.snake.cialo_weza[0].x + gra.eti2->w / 2 == 1270);
			else
				gra.snake.cialo_weza[0].x++;
		}
		else if (gra.snake.cialo_weza->kierunek == 2)
		{
			if (gra.snake.cialo_weza[0].x - gra.eti2->w / 2 == 10);
			else
				gra.snake.cialo_weza[0].x--;
		}
		else if (gra.snake.cialo_weza->kierunek == 3)
		{
			if (gra.snake.cialo_weza[0].y - gra.eti2->h / 2 == 66);
			else
				gra.snake.cialo_weza[0].y--;
		}
		else if (gra.snake.cialo_weza->kierunek == 4)
		{
			if (gra.snake.cialo_weza[0].y - gra.eti2->h / 2 == 610);
			else
				gra.snake.cialo_weza[0].y++;
		}
		if (((gra.snake.cialo_weza[0].y==gra.snake.pozycja_y+90|| gra.snake.cialo_weza[0].y == gra.snake.pozycja_y - 90)&&(gra.snake.cialo_weza[0].x == gra.snake.pozycja_x))|| ((gra.snake.cialo_weza[0].x == gra.snake.pozycja_x + 90 || gra.snake.cialo_weza[0].x == gra.snake.pozycja_x - 90) && (gra.snake.cialo_weza[0].y == gra.snake.pozycja_y)))
		{
				gra.snake.cialo_weza[0].kierunek = gra.snake.kierunek;	
		}
		gra.time.snake_speed_licznik = 0;
	}
}
int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	stan_gry gra;
	inicjalizacja(gra);
	char text[128];
	int czarny = SDL_MapRGB(gra.screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(gra.screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(gra.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(gra.screen->format, 0x11, 0x11, 0xCC);
	DrawSurface(gra.screen, gra.eti, SCREEN_WIDTH/2, (SCREEN_HEIGHT+54)/2);
	while (!gra.quit) {
		SDL_FillRect(gra.screen, NULL, czarny);
		DrawSurface(gra.screen, gra.obramowanie, SCREEN_WIDTH / 2, (SCREEN_HEIGHT + 54) / 2);
		funkcjetimer(gra);
		DrawSurface(gra.screen, gra.eti, gra.snake.pozycja_x, gra.snake.pozycja_y);
		DrawSurface(gra.screen, gra.eti2, gra.snake.cialo_weza[0].x, gra.snake.cialo_weza[0].y);
		DrawRectangle(gra.screen, 0, 4, SCREEN_WIDTH , 50, czerwony, niebieski);  //maluje okno wyúwietlania tekstu
		sprintf(text, "Szsablasfsdon , czas trwania = %.1lf s  %.0lf klatek / s  %d    %d", gra.time.worldTime, gra.time.fps, gra.snake.pozycja_x, gra.eti->w);
		DrawString(gra.screen, gra.screen->w / 2 - strlen(text) * 8 / 2, 10, text, gra.charset);
		sprintf(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie");
		DrawString(gra.screen, gra.screen->w / 2 - strlen(text) * 8 / 2, 26, text, gra.charset);
		SDL_UpdateTexture(gra.scrtex, NULL, gra.screen->pixels, gra.screen->pitch);
		SDL_RenderCopy(gra.renderer, gra.scrtex, NULL, NULL);
		SDL_RenderPresent(gra.renderer);
		zdarzenie(gra);
		zmiana_pozycji(gra);
		zmiana_pozycji_ciala(gra);
	}

}