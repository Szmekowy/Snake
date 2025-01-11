#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <time.h>

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
	int  t1=SDL_GetTicks() , t2=0, frames=0;
	double delta, worldTime=0, fpsTimer=0, fps=0, distance=0, snake_speed=0.008, snake_speed_licznik=0;
	double czas_zmiany = 0.4, licznik_zmiany = 0.4; 
	double speedup_limit = 5.0;
	double speedup = 1.15;
	double progres_delay = 0.019;
	double progres_rand = 1.00;
};
struct czesci_weza {
	int x=500;
	int y=300;
	int kierunek=1;
	int pom = 1;
};
struct wisnie {
	int x = 700;
	int y = 300;
};
struct stan_gry {
	timer time;
	SDL_Event event;
	SDL_Surface* screen, * charset, *obramowanie;
	SDL_Surface* eti, *eti2, *wisnia, *progres, *bonus;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	int quit = 0;
	int total_progres = 1;
	int points = 0;
	int czerwony_bonus = 0;
	int czy_odczyt = 0;
	wisnie wisnia_powieksz;
	wisnie wisnia_bonusowa;
	struct {
		int aktualny_rozmiar=5;
		czesci_weza cialo_weza[200];
	}snake;

};
void funkcjetimer(stan_gry	&gra)
{
	int czas= SDL_GetTicks() -gra.czy_odczyt;
	gra.time.t2 = czas;
	gra.time.delta = (gra.time.t2 - gra.time.t1) * 0.001;
	gra.time.t1 = gra.time.t2;
	gra.time.snake_speed_licznik += gra.time.delta;
	gra.time.licznik_zmiany += gra.time.delta;
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
	gra.eti = SDL_LoadBMP("./glowa.bmp"); //bitmapa do tekstu
	if (gra.eti == NULL) {
		printf("SDL_LoadBMP(glowa.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gra.screen);
		SDL_DestroyTexture(gra.scrtex);
		SDL_DestroyWindow(gra.window);
		SDL_DestroyRenderer(gra.renderer);
		SDL_Quit();
	};
	SDL_SetColorKey(gra.eti, true, 0x000000);
	gra.eti2 = SDL_LoadBMP("./cialo.bmp"); //bitmapa do tekstu
	if (gra.eti2 == NULL) {
		printf("SDL_LoadBMP(cialo.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gra.screen);
		SDL_DestroyTexture(gra.scrtex);
		SDL_DestroyWindow(gra.window);
		SDL_DestroyRenderer(gra.renderer);
		SDL_Quit();
	};
	SDL_SetColorKey(gra.eti2, true, 0x000000);
	gra.wisnia = SDL_LoadBMP("./wisnia.bmp"); //bitmapa do tekstu
	if (gra.wisnia == NULL) {
		printf("SDL_LoadBMP(wisnia.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gra.screen);
		SDL_DestroyTexture(gra.scrtex);
		SDL_DestroyWindow(gra.window);
		SDL_DestroyRenderer(gra.renderer);
		SDL_Quit();
	};
	gra.progres = SDL_LoadBMP("./progres_bar.bmp"); //bitmapa do tekstu
	if (gra.progres == NULL) {
		printf("SDL_LoadBMP(progres_bar.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gra.screen);
		SDL_DestroyTexture(gra.scrtex);
		SDL_DestroyWindow(gra.window);
		SDL_DestroyRenderer(gra.renderer);
		SDL_Quit();
	};
	gra.bonus = SDL_LoadBMP("./bonus.bmp"); //bitmapa do tekstu
	if (gra.progres == NULL) {
		printf("SDL_LoadBMP(bonus.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gra.screen);
		SDL_DestroyTexture(gra.scrtex);
		SDL_DestroyWindow(gra.window);
		SDL_DestroyRenderer(gra.renderer);
		SDL_Quit();
	};
}
void zmiana_pozycji(stan_gry &gra)
{
	if (gra.time.snake_speed_licznik >= gra.time.snake_speed)
	{
		if (gra.snake.cialo_weza[0].kierunek == 1)
		{
			if (gra.snake.cialo_weza[0].x + gra.eti->w / 2 == 1270)
			{
				if (gra.snake.cialo_weza[0].y - gra.eti->h / 2 == 683)
					gra.snake.cialo_weza[0].kierunek = 3;
				else
					gra.snake.cialo_weza[0].kierunek = 4;
			}
			else
				gra.snake.cialo_weza[0].x++;
		}
		else if (gra.snake.cialo_weza[0].kierunek == 2)
		{
			if (gra.snake.cialo_weza[0].x - gra.eti->w / 2 == 10)
			{
				if (gra.snake.cialo_weza[0].y - gra.eti->h / 2 == 66)
					gra.snake.cialo_weza[0].kierunek = 4;
				else
					gra.snake.cialo_weza[0].kierunek = 3;

			}
			else
				gra.snake.cialo_weza[0].x--;
		}
		else if (gra.snake.cialo_weza[0].kierunek == 3)
		{
			if (gra.snake.cialo_weza[0].y - gra.eti->h / 2 == 66)
			{
				if (gra.snake.cialo_weza[0].x + gra.eti->w / 2 == 1270)
					gra.snake.cialo_weza[0].kierunek = 2;
				else
					gra.snake.cialo_weza[0].kierunek = 1;

			}
			else
				gra.snake.cialo_weza[0].y--;
		}
		else if (gra.snake.cialo_weza[0].kierunek == 4)
		{
			if (gra.snake.cialo_weza[0].y - gra.eti->h / 2 == 683)
			{
				if (gra.snake.cialo_weza[0].x - gra.eti->w / 2 == 10)
					gra.snake.cialo_weza[0].kierunek = 1;
				else
					gra.snake.cialo_weza[0].kierunek = 2;

			}
			else
				gra.snake.cialo_weza[0].y++;
		}
	}
}
void zmiana_pozycji_ciala(stan_gry& gra, int i)
{
	
	if (gra.time.snake_speed_licznik >= gra.time.snake_speed)
	{
		
		if (gra.snake.cialo_weza[i].kierunek == 1)
		{
			if (gra.snake.cialo_weza[i].x + gra.eti2->w / 2 == 1270) gra.snake.cialo_weza[i].pom = 0;
			else
				gra.snake.cialo_weza[i].x++;
		}
		else if (gra.snake.cialo_weza[i].kierunek == 2)
		{
			if (gra.snake.cialo_weza[i].x - gra.eti2->w / 2 == 10) gra.snake.cialo_weza[i].pom = 0;
			else
				gra.snake.cialo_weza[i].x--;
		}
		else if (gra.snake.cialo_weza[i].kierunek == 3)
		{
			if (gra.snake.cialo_weza[i].y - gra.eti2->h / 2 == 66) gra.snake.cialo_weza[i].pom = 0;
			else
				gra.snake.cialo_weza[i].y--;
		}
		else if (gra.snake.cialo_weza[i].kierunek == 4)
		{
			if (gra.snake.cialo_weza[i].y - gra.eti2->h / 2 == 683) gra.snake.cialo_weza[i].pom = 0;
			else
				gra.snake.cialo_weza[i].y++;
		}
		if(gra.snake.cialo_weza[i].kierunek!=gra.snake.cialo_weza[i-1].kierunek)
		if (((gra.snake.cialo_weza[i].y == gra.snake.cialo_weza[i - 1].y + 30 || gra.snake.cialo_weza[i].y == gra.snake.cialo_weza[i - 1].y - 30) && (gra.snake.cialo_weza[i].x == gra.snake.cialo_weza[i - 1].x)) || ((gra.snake.cialo_weza[i].x == gra.snake.cialo_weza[i - 1].x + 30 || gra.snake.cialo_weza[i].x == gra.snake.cialo_weza[i - 1].x - 30) && (gra.snake.cialo_weza[i].y == gra.snake.cialo_weza[i - 1].y)))
		{
			if (i == 1)
				gra.snake.cialo_weza[i].kierunek = gra.snake.cialo_weza[i - 1].kierunek;
			else if (gra.snake.cialo_weza[i].pom == 0)
			{
				gra.snake.cialo_weza[i].kierunek = gra.snake.cialo_weza[i - 1].kierunek;
				gra.snake.cialo_weza[i].pom = 1;
			}
			else
				gra.snake.cialo_weza[i].pom = 0;
		}
		if(i==gra.snake.aktualny_rozmiar-1)
		gra.time.snake_speed_licznik = 0;
	}
}
void init_cialo(stan_gry& gra)
{
	for (int i = 1; i < gra.snake.aktualny_rozmiar; i++)
	{
		gra.snake.cialo_weza[i].x = gra.snake.cialo_weza[i - 1].x - 30;
		gra.snake.cialo_weza[i].y = gra.snake.cialo_weza[i - 1].y;
		gra.snake.cialo_weza[i].kierunek = 1;
		gra.snake.cialo_weza[i].pom = 1;
	}
}
int czy_kolizja(stan_gry& gra)
{
	for (int i = 1; i < gra.snake.aktualny_rozmiar; i++)
	{
		if (gra.snake.cialo_weza[0].kierunek == 1)
		{
			if (gra.snake.cialo_weza[0].x + 30 == gra.snake.cialo_weza[i].x)
				if ((gra.snake.cialo_weza[0].y + 30 >= gra.snake.cialo_weza[i].y) && (gra.snake.cialo_weza[0].y - 30 <= gra.snake.cialo_weza[i].y))
					return 1;
		}
		else if (gra.snake.cialo_weza[0].kierunek == 2)
		{
			if (gra.snake.cialo_weza[0].x - 30 == gra.snake.cialo_weza[i].x)
				if ((gra.snake.cialo_weza[0].y + 30 >= gra.snake.cialo_weza[i].y) && (gra.snake.cialo_weza[0].y - 30 <= gra.snake.cialo_weza[i].y))
					return 1;
		}
		else if (gra.snake.cialo_weza[0].kierunek == 3)
		{
			if (gra.snake.cialo_weza[0].y - 30 == gra.snake.cialo_weza[i].y)
				if ((gra.snake.cialo_weza[0].x + 30 >= gra.snake.cialo_weza[i].x) && (gra.snake.cialo_weza[0].x - 30 <= gra.snake.cialo_weza[i].x))
					return 1;
		}
		else if (gra.snake.cialo_weza[0].kierunek == 4)
		{
			if (gra.snake.cialo_weza[0].y + 30 == gra.snake.cialo_weza[i].y)
				if ((gra.snake.cialo_weza[0].x + 30 >= gra.snake.cialo_weza[i].x) && (gra.snake.cialo_weza[0].x - 30 <= gra.snake.cialo_weza[i].x))
					return 1;
		}
	}
	return 0;
}
void rozpocznij_gre(stan_gry& gra)
{
	char text[128];
	int czarny = SDL_MapRGB(gra.screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(gra.screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(gra.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(gra.screen->format, 0x11, 0x11, 0xCC);
	if (czy_kolizja(gra))
	{
		SDL_SetRenderDrawColor(gra.renderer, 0, 0, 0, 255);
		SDL_RenderClear(gra.renderer);
		SDL_RenderPresent(gra.renderer);
		DrawRectangle(gra.screen, 0, 4, SCREEN_WIDTH, 50, czerwony, niebieski);  //maluje okno wyúwietlania tekstu
		sprintf(text, "nacisniej ESCAPE aby zakonczyc nacisnij n aby rozpoczac ponownie");
		DrawString(gra.screen, gra.screen->w / 2 - strlen(text) * 8 / 2, 10, text, gra.charset);
		for (int i = 0; i < gra.snake.aktualny_rozmiar; i++)
		{
			DrawSurface(gra.screen, gra.eti2, gra.snake.cialo_weza[i].x, gra.snake.cialo_weza[i].y);
		}
		SDL_UpdateTexture(gra.scrtex, NULL, gra.screen->pixels, gra.screen->pitch);
		SDL_RenderCopy(gra.renderer, gra.scrtex, NULL, NULL);
		SDL_RenderPresent(gra.renderer);
		int przycisk = 0;
		while (!przycisk)
		{
			if (SDL_WaitEvent(&gra.event)) {
				switch (gra.event.type) {
				case SDL_QUIT:
					gra.quit = 1;
					przycisk = 1;
					break;

				case SDL_KEYDOWN:
					if (gra.event.key.keysym.sym == SDLK_n)
					{
						gra.snake.cialo_weza[0].x = 500;
						gra.snake.cialo_weza[0].y = 300;
						gra.snake.cialo_weza[0].kierunek = 1;
						gra.time.worldTime = 0;
						gra.snake.aktualny_rozmiar = 5;
						init_cialo(gra);
						przycisk = 1;
						gra.time.snake_speed_licznik = 0;
						gra.points = 0;
						gra.time.snake_speed = 0.008;
						gra.total_progres = 1;
						gra.time.speedup_limit = 5;
						gra.time.licznik_zmiany = 0.4;
						gra.time.czas_zmiany = 0.4;
						gra.czerwony_bonus = 0;
					}
					else if (gra.event.key.keysym.sym == SDLK_ESCAPE)
					{
						gra.quit = 1;
						przycisk = 1;
					}
					break;
				}
			}
		}
	}
}
int zebranie_wisni(stan_gry& gra)
{
	if (gra.snake.cialo_weza[0].kierunek == 1)
	{
		if (gra.snake.cialo_weza[0].x + 25 == gra.wisnia_powieksz.x)
			if ((gra.snake.cialo_weza[0].y + 25 >= gra.wisnia_powieksz.y) && (gra.snake.cialo_weza[0].y - 25 <= gra.wisnia_powieksz.y))
				return 1;
	}
	else if (gra.snake.cialo_weza[0].kierunek == 2)
	{
		if (gra.snake.cialo_weza[0].x - 25 == gra.wisnia_powieksz.x)
			if ((gra.snake.cialo_weza[0].y + 25 >= gra.wisnia_powieksz.y) && (gra.snake.cialo_weza[0].y - 25 <= gra.wisnia_powieksz.y))
				return 1;
	}
	else if (gra.snake.cialo_weza[0].kierunek == 3)
	{
		if (gra.snake.cialo_weza[0].y - 25 == gra.wisnia_powieksz.y)
			if ((gra.snake.cialo_weza[0].x + 25 >= gra.wisnia_powieksz.x) && (gra.snake.cialo_weza[0].x - 25 <= gra.wisnia_powieksz.x))
				return 1;
	}
	else if (gra.snake.cialo_weza[0].kierunek == 4)
	{
		if (gra.snake.cialo_weza[0].y + 25 == gra.wisnia_powieksz.y)
			if ((gra.snake.cialo_weza[0].x + 25 >= gra.wisnia_powieksz.x) && (gra.snake.cialo_weza[0].x - 25 <= gra.wisnia_powieksz.x))
				return 1;
	}
	return 0;
}
int zebranie_wisni2(stan_gry& gra)
{
	if (gra.snake.cialo_weza[0].kierunek == 1)
	{
		if (gra.snake.cialo_weza[0].x + 25 == gra.wisnia_bonusowa.x)
			if ((gra.snake.cialo_weza[0].y + 25 >= gra.wisnia_bonusowa.y) && (gra.snake.cialo_weza[0].y - 25 <= gra.wisnia_bonusowa.y))
				return 1;
	}
	else if (gra.snake.cialo_weza[0].kierunek == 2)
	{
		if (gra.snake.cialo_weza[0].x - 25 == gra.wisnia_bonusowa.x)
			if ((gra.snake.cialo_weza[0].y + 25 >= gra.wisnia_bonusowa.y) && (gra.snake.cialo_weza[0].y - 25 <= gra.wisnia_bonusowa.y))
				return 1;
	}
	else if (gra.snake.cialo_weza[0].kierunek == 3)
	{
		if (gra.snake.cialo_weza[0].y - 25 == gra.wisnia_bonusowa.y)
			if ((gra.snake.cialo_weza[0].x + 25 >= gra.wisnia_bonusowa.x) && (gra.snake.cialo_weza[0].x - 25 <= gra.wisnia_bonusowa.x))
				return 1;
	}
	else if (gra.snake.cialo_weza[0].kierunek == 4)
	{
		if (gra.snake.cialo_weza[0].y + 25 == gra.wisnia_bonusowa.y)
			if ((gra.snake.cialo_weza[0].x + 25 >= gra.wisnia_bonusowa.x) && (gra.snake.cialo_weza[0].x - 25 <= gra.wisnia_bonusowa.x))
				return 1;
	}
	return 0;
}
void zmniejsz(stan_gry& gra, int bonus)
{
	
	if (zebranie_wisni2(gra))
	{
		int pom = rand() % 2 + 1;
		if (pom == 1)
		{
			gra.time.snake_speed *= 1.5;
			gra.time.czas_zmiany *= 1.5;
		}
		else if(gra.snake.aktualny_rozmiar>1)
		{
			gra.snake.aktualny_rozmiar--;
		}
		
		gra.wisnia_bonusowa.x = rand() % 1200 + 30;
		gra.wisnia_bonusowa.y = rand() % 550 + 130;
		gra.points++;
	}
}
int czy_bonus()
{
	int pom = rand() % 100;
	if (pom <= 50)
		return 1;
	return 0;
}
void powieksz(stan_gry& gra)
{
	if (zebranie_wisni(gra))
	{
		gra.wisnia_powieksz.x = rand() % 1200 + 30;
		gra.wisnia_powieksz.y = rand() % 550 + 130;
		gra.snake.aktualny_rozmiar++;
		if (gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].kierunek == 1)
		{
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].x = gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].x - 30;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].y = gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].y;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].kierunek = 1;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].pom = 1;
		}
		else if (gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].kierunek == 2)
		{
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].x = gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].x + 30;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].y = gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].y;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].kierunek = 2;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].pom = 1;
		}
		else if (gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].kierunek == 3)
		{
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].x = gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].x;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].y = gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].y + 30;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].kierunek = 3;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].pom = 1;
		}
		else if (gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].kierunek == 4)
		{
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].x = gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].x;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].y = gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 2].y - 30;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].kierunek = 4;
			gra.snake.cialo_weza[gra.snake.aktualny_rozmiar - 1].pom = 1;
		}
		gra.points++;
	}
}
void licznik_progresu(stan_gry& gra, int& bonus)
{
	if (gra.time.progres_rand <= gra.time.worldTime)
	{
		gra.time.progres_rand++;
		if (czy_bonus())
		{
			bonus = 1;

		}
	}
	if (gra.time.progres_delay <= gra.time.worldTime)
	{
		gra.time.progres_delay += 0.019;
		if (bonus == 1)
		{
			gra.total_progres++;
		}
	}

	if (gra.total_progres >= 260)
	{
		bonus = 0;
		gra.total_progres = 1;
		gra.wisnia_bonusowa.x = rand() % 1200 + 30;
		gra.wisnia_bonusowa.y = rand() % 550 + 130;
	}
}
void zapisz(stan_gry& gra)
{
	FILE* plik;
	plik = fopen("zapisgry.txt", "r+");
	if (plik == NULL)
	{
		printf("zapis error: \n");
	}
	fprintf(plik, "%d \n", gra.time.t1);
	fprintf(plik, "%d \n", gra.time.t2);
	fprintf(plik, "%d \n", gra.time.frames);
	fprintf(plik, "%lf \n", gra.time.delta);
	fprintf(plik, "%lf \n", gra.time.worldTime);
	fprintf(plik, "%lf \n", gra.time.fpsTimer);
	fprintf(plik, "%lf \n", gra.time.fps);
	fprintf(plik, "%lf \n", gra.time.snake_speed);
	fprintf(plik, "%lf \n", gra.time.snake_speed_licznik);
	fprintf(plik, "%lf \n", gra.time.czas_zmiany);
	fprintf(plik, "%lf \n", gra.time.licznik_zmiany);
	fprintf(plik, "%lf \n", gra.time.speedup_limit);
	fprintf(plik, "%lf \n", gra.time.speedup);
	fprintf(plik, "%lf \n", gra.time.progres_delay);
	fprintf(plik, "%lf \n", gra.time.progres_rand);
	fprintf(plik, "%d \n", gra.quit);
	fprintf(plik, "%d \n", gra.total_progres);
	fprintf(plik, "%d \n", gra.points);
	fprintf(plik, "%d \n", gra.czerwony_bonus);
	fprintf(plik, "%d \n", gra.czy_odczyt);
	fprintf(plik, "%d \n", gra.wisnia_powieksz.x);
	fprintf(plik, "%d \n", gra.wisnia_powieksz.y);
	fprintf(plik, "%d \n", gra.wisnia_bonusowa.x);
	fprintf(plik, "%d \n", gra.wisnia_bonusowa.x);
	fprintf(plik, "%d \n", gra.snake.aktualny_rozmiar);
	for (int i = 0; i < gra.snake.aktualny_rozmiar; i++)
	{
		fprintf(plik, "%d \n", gra.snake.cialo_weza[i].x);
		fprintf(plik, "%d \n", gra.snake.cialo_weza[i].y);
		fprintf(plik, "%d \n", gra.snake.cialo_weza[i].pom);
		fprintf(plik, "%d \n", gra.snake.cialo_weza[i].kierunek);
	}

}
void odczyt(stan_gry& gra)
{
	FILE* plik;
	plik = fopen("zapisgry.txt", "r+");
	if (plik == NULL)
	{
		printf("odczyt error: \n");
	}
	fscanf(plik, "%d ", &gra.time.t1);
	fscanf(plik, "%d ", &gra.time.t2);
	fscanf(plik, "%d ", &gra.time.frames);
	fscanf(plik, "%lf ", &gra.time.delta);
	fscanf(plik, "%lf ", &gra.time.worldTime);
	fscanf(plik, "%lf ", &gra.time.fpsTimer);
	fscanf(plik, "%lf ", &gra.time.fps);
	fscanf(plik, "%lf ", &gra.time.snake_speed);
	fscanf(plik, "%lf ", &gra.time.snake_speed_licznik);
	fscanf(plik, "%lf ", &gra.time.czas_zmiany);
	fscanf(plik, "%lf ", &gra.time.licznik_zmiany);
	fscanf(plik, "%lf ", &gra.time.speedup_limit);
	fscanf(plik, "%lf ", &gra.time.speedup);
	fscanf(plik, "%lf ", &gra.time.progres_delay);
	fscanf(plik, "%lf ", &gra.time.progres_rand);
	fscanf(plik, "%d ", &gra.quit);
	fscanf(plik, "%d ", &gra.total_progres);
	fscanf(plik, "%d ", &gra.points);
	fscanf(plik, "%d ", &gra.czerwony_bonus);
	fscanf(plik, "%d ", &gra.czy_odczyt);
	fscanf(plik, "%d ", &gra.wisnia_powieksz.x);
	fscanf(plik, "%d ", &gra.wisnia_powieksz.y);
	fscanf(plik, "%d ", &gra.wisnia_bonusowa.x);
	fscanf(plik, "%d ", &gra.wisnia_bonusowa.x);
	fscanf(plik, "%d ", &gra.snake.aktualny_rozmiar);
	for (int i = 0; i < gra.snake.aktualny_rozmiar; i++)
	{
		fscanf(plik, "%d ", &gra.snake.cialo_weza[i].x);
		fscanf(plik, "%d ", &gra.snake.cialo_weza[i].y);
		fscanf(plik, "%d ", &gra.snake.cialo_weza[i].pom);
		fscanf(plik, "%d ", &gra.snake.cialo_weza[i].kierunek);
	}
}
void zdarzenie(stan_gry& gra)
{
	while (SDL_PollEvent(&gra.event)) {
		switch (gra.event.type) {
		case SDL_KEYDOWN:
			if (gra.event.key.keysym.sym == SDLK_ESCAPE) gra.quit = 1;
			else if (gra.event.key.keysym.sym == SDLK_n)
			{
				gra.snake.cialo_weza[0].x = 500;
				gra.snake.cialo_weza[0].y = 300;
				gra.time.worldTime = 0;
				gra.snake.cialo_weza[0].kierunek = 1;
			}
			else if (gra.event.key.keysym.sym == SDLK_d)
			{
				gra.snake.cialo_weza[0].kierunek = 1;
			}
			else if (gra.event.key.keysym.sym == SDLK_a)
			{
				gra.snake.cialo_weza[0].kierunek = 2;
			}
			else if (gra.event.key.keysym.sym == SDLK_w)
			{
				gra.snake.cialo_weza[0].kierunek = 3;
			}
			else if (gra.event.key.keysym.sym == SDLK_s)
			{
				gra.snake.cialo_weza[0].kierunek = 4;
			}
			else if (gra.event.key.keysym.sym == SDLK_p)
			{
				zapisz(gra);
			}
			else if (gra.event.key.keysym.sym == SDLK_i)
			{
				odczyt(gra);
				gra.czy_odczyt = gra.time.t2;
				gra.time.t1 = 0;
			}
			break;
		case SDL_QUIT:
			gra.quit = 1;
			break;
		};
	};
}
int main(int argc, char* argv[]) {
	srand(time(NULL));
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	int a=0;
	stan_gry gra;
	inicjalizacja(gra);
	char text[128];
	int czarny = SDL_MapRGB(gra.screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(gra.screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(gra.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(gra.screen->format, 0x11, 0x11, 0xCC);
	DrawSurface(gra.screen, gra.eti, SCREEN_WIDTH/2, (SCREEN_HEIGHT+54)/2);
	init_cialo(gra);
	gra.wisnia_bonusowa.x = rand() % 1200 + 30;
	gra.wisnia_bonusowa.y = rand() % 550 + 130;
	while (!gra.quit) {
		SDL_FillRect(gra.screen, NULL, czarny);
		DrawSurface(gra.screen, gra.obramowanie, SCREEN_WIDTH / 2, (SCREEN_HEIGHT + 54) / 2);
		funkcjetimer(gra);
		rozpocznij_gre(gra);
		powieksz(gra);
		DrawSurface(gra.screen, gra.eti, gra.snake.cialo_weza[0].x, gra.snake.cialo_weza[0].y);
		if (gra.time.czas_zmiany <= gra.time.licznik_zmiany)
		{
			zdarzenie(gra);
			gra.time.licznik_zmiany = 0;
		}
		zmiana_pozycji(gra);
		

		for (int i = 1; i < gra.snake.aktualny_rozmiar; i++)
		{
			DrawSurface(gra.screen, gra.eti2, gra.snake.cialo_weza[i].x, gra.snake.cialo_weza[i].y);
			zmiana_pozycji_ciala(gra, i);
		}
		DrawSurface(gra.screen, gra.wisnia, gra.wisnia_powieksz.x, gra.wisnia_powieksz.y);
		DrawRectangle(gra.screen, 0, 4, SCREEN_WIDTH , 50, czerwony, niebieski); 
		//maluje okno wyúwietlania tekstu
		sprintf(text, "Szsablasfsdon , czas trwania = %.1lf s  %.0lf klatek / s  %d    ", gra.time.worldTime, gra.time.fps, gra.points );
		DrawString(gra.screen, gra.screen->w / 4 - strlen(text) * 8 / 2, 10, text, gra.charset);
		sprintf(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie");
		DrawString(gra.screen, gra.screen->w / 2 - strlen(text) * 8 / 2, 26, text, gra.charset);
		DrawSurface(gra.screen, gra.progres, 1000, 25);
		DrawRectangle(gra.screen, 870, 8, gra.total_progres, 34, czerwony, czerwony);
		if (gra.czerwony_bonus == 1)
		{
			zmniejsz(gra,gra.czerwony_bonus);
			DrawSurface(gra.screen, gra.bonus, gra.wisnia_bonusowa.x, gra.wisnia_bonusowa.y);
		}
		SDL_UpdateTexture(gra.scrtex, NULL, gra.screen->pixels, gra.screen->pitch);
		SDL_RenderCopy(gra.renderer, gra.scrtex, NULL, NULL);
		SDL_RenderPresent(gra.renderer);
		if (gra.time.speedup_limit <= gra.time.worldTime)
		{
			gra.time.snake_speed /= gra.time.speedup;
			gra.time.speedup_limit+=5;
			if(gra.time.czas_zmiany>0.2)
			gra.time.czas_zmiany /= gra.time.speedup;
		}
		licznik_progresu(gra, gra.czerwony_bonus);
		

	}
}