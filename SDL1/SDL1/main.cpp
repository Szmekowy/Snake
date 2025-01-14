///SZYMON DRYWA s203668 PROJEKT 2 SNAKE korzystam z szablonu projekt 2 SNAKE ///
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

#define SCREEN_WIDTH	1280 //szerokoœæ planszy
#define SCREEN_HEIGHT	720 //wysokoœæ planszy

//////////										
////////// STRUKTURY							
////////// 
struct timer {
	int  t1=SDL_GetTicks() , t2=0, frames=0;
	double delta, worldTime=0, fpsTimer=0, fps=0, distance=0, snake_speed=0.008, snake_speed_licznik=0;
	double czas_zmiany = 0.4, licznik_zmiany = 0.4; 
	double speedup_limit = 5.0;
	double speedup = 1.15;
	double progres_delay = 0.019;
	double progres_rand = 1.00;
};
struct teleporty {
	int x;
	int y;
	int x2;
	int y2;
};
struct czesci_weza {
	int x=510;
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
	SDL_Surface* eti, *eti2, *wisnia, *progres, *bonus, *cialo2, *ogon2;
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
		int zmiana_kierunku_glowy = 1;
		int aktualny_rozmiar=5;
		czesci_weza cialo_weza[200];
	}snake;
	teleporty teleport[2];

};
//////////										
////////// STRUKTURY							
////////// 

//////////										
////////// FUNKCJE TIMERA							
////////// 
void funkcjetimer(stan_gry	&gra)
{
	gra.time.t2 = SDL_GetTicks();
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
//////////										
////////// FUNKCJE TIMERA-koniec							
////////// 

//////////										
////////// FUNCKJE MALOWANIA SZABLON							
////////// 
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
// (x, y) to punkt œrodka obrazka sprite na ekranie

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


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)

void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};

// rysowanie prostok¹ta o d³ugoœci boków l i k
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
//////////										
////////// FUNCKJE MALOWANIA SZABLON -koniec							
////////// 

//////////										
////////// INICJALIZACJA SDL EKRANU RENDERERA 							
////////// 
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
	gra.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000); //tworzenie ekranu i ustalenie parametrów
	gra.scrtex = SDL_CreateTexture(gra.renderer, SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,SCREEN_WIDTH, SCREEN_HEIGHT); //tworzenie tekstury
	SDL_ShowCursor(SDL_DISABLE); // wy³¹czenie kursora
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
	gra.cialo2 = SDL_LoadBMP("./cialo2.bmp"); //bitmapa do tekstu
	if (gra.progres == NULL) {
		printf("SDL_LoadBMP(cialo2.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gra.screen);
		SDL_DestroyTexture(gra.scrtex);
		SDL_DestroyWindow(gra.window);
		SDL_DestroyRenderer(gra.renderer);
		SDL_Quit();
	};
	SDL_SetColorKey(gra.cialo2, true, 0x000000);
	gra.ogon2 = SDL_LoadBMP("./ogon2.bmp"); //bitmapa do tekstu
	if (gra.progres == NULL) {
		printf("SDL_LoadBMP(ogon2.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gra.screen);
		SDL_DestroyTexture(gra.scrtex);
		SDL_DestroyWindow(gra.window);
		SDL_DestroyRenderer(gra.renderer);
		SDL_Quit();
	};
	SDL_SetColorKey(gra.ogon2, true, 0x000000);
}

//////////										
////////// INICJALIZACJA SDL EKRANU RENDERERA itp-koniec							
////////// 

//////////										
////////// PO£O¯ENIE WÊ¯A NA MAPIE								
////////// 
void zmiana_pozycji(stan_gry &gra) /// zmiana kierunku poruszania siê g³owy i reakcja na œciany
{
	if (gra.time.snake_speed_licznik >= gra.time.snake_speed)
	{
		
		if (gra.snake.cialo_weza[0].kierunek == 1)
		{
			if (gra.snake.cialo_weza[0].x + gra.eti->w / 2 == 1245)
			{
				if (gra.snake.cialo_weza[0].y - gra.eti->h / 2 == 675)
					gra.snake.zmiana_kierunku_glowy = 3;
				else
					gra.snake.zmiana_kierunku_glowy = 4;
			}
			else
				gra.snake.cialo_weza[0].x++;
		}
		else if (gra.snake.cialo_weza[0].kierunek == 2)
		{
			if (gra.snake.cialo_weza[0].x + gra.eti->w / 2 == 45)
			{
				if (gra.snake.cialo_weza[0].y - gra.eti->h / 2 == 75)
					gra.snake.zmiana_kierunku_glowy = 4;
				else
					gra.snake.zmiana_kierunku_glowy = 3;

			}
			else
				gra.snake.cialo_weza[0].x--;
		}
		else if (gra.snake.cialo_weza[0].kierunek == 3)
		{
			if (gra.snake.cialo_weza[0].y - gra.eti->h / 2 == 75)
			{
				if (gra.snake.cialo_weza[0].x + gra.eti->w / 2 == 1245)
					gra.snake.zmiana_kierunku_glowy = 2;
				else
					gra.snake.zmiana_kierunku_glowy = 1;

			}
			else
				gra.snake.cialo_weza[0].y--;
		}
		else if (gra.snake.cialo_weza[0].kierunek == 4)
		{
			if (gra.snake.cialo_weza[0].y - gra.eti->h / 2 == 675)
			{
				if (gra.snake.cialo_weza[0].x + gra.eti->w / 2 == 45)
					gra.snake.zmiana_kierunku_glowy = 1;
				else
					gra.snake.zmiana_kierunku_glowy = 2;

			}
			else
				gra.snake.cialo_weza[0].y++;
		}
	}
}
void zmiana_pozycji_ciala(stan_gry& gra, int i) /// zmiana kierunku poruszania siê kolejnych czêœci cia³a w zale¿noœci od poprzednika
{
	
	if (gra.time.snake_speed_licznik >= gra.time.snake_speed)
	{
		
		if (gra.snake.cialo_weza[i].kierunek == 1)
		{
			if (gra.snake.cialo_weza[i].x + gra.eti2->w / 2 == 1245) gra.snake.cialo_weza[i].pom = 0;
			else
				gra.snake.cialo_weza[i].x++;
		}
		else if (gra.snake.cialo_weza[i].kierunek == 2)
		{
			if (gra.snake.cialo_weza[i].x + gra.eti2->w / 2 == 45) gra.snake.cialo_weza[i].pom = 0;
			else
				gra.snake.cialo_weza[i].x--;
		}
		else if (gra.snake.cialo_weza[i].kierunek == 3)
		{
			if (gra.snake.cialo_weza[i].y - gra.eti2->h / 2 == 75) gra.snake.cialo_weza[i].pom = 0;
			else
				gra.snake.cialo_weza[i].y--;
		}
		else if (gra.snake.cialo_weza[i].kierunek == 4)
		{
			if (gra.snake.cialo_weza[i].y - gra.eti2->h / 2 == 675) gra.snake.cialo_weza[i].pom = 0;
			else
				gra.snake.cialo_weza[i].y++;
		}
		if(gra.snake.cialo_weza[i].kierunek!=gra.snake.cialo_weza[i-1].kierunek)
		if (((gra.snake.cialo_weza[i].y == gra.snake.cialo_weza[i - 1].y + 30 || gra.snake.cialo_weza[i].y == gra.snake.cialo_weza[i - 1].y - 30) && (gra.snake.cialo_weza[i].x == gra.snake.cialo_weza[i - 1].x)) || ((gra.snake.cialo_weza[i].x == gra.snake.cialo_weza[i - 1].x + 30 || gra.snake.cialo_weza[i].x == gra.snake.cialo_weza[i - 1].x - 30) && (gra.snake.cialo_weza[i].y == gra.snake.cialo_weza[i - 1].y))) /// czy jest mo¿liwa zmiana kierunku
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
void init_cialo(stan_gry& gra) /// inicjalizacja pocz¹tkowych pozycji cia³a wê¿a
{
	for (int i = 1; i < gra.snake.aktualny_rozmiar; i++)
	{
		gra.snake.cialo_weza[i].x = gra.snake.cialo_weza[i - 1].x - 30;
		gra.snake.cialo_weza[i].y = gra.snake.cialo_weza[i - 1].y;
		gra.snake.cialo_weza[i].kierunek = 1;
		gra.snake.cialo_weza[i].pom = 1;
	}
}
int czy_kolizja(stan_gry& gra) /// sprawdzanie kolizji g³owy z cia³em wê¿a
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
//////////										
////////// PO£O¯ENIE WÊ¯A NA MAPIE-koniec									
////////// 

void nowa_gra(stan_gry& gra)
{
	gra.snake.zmiana_kierunku_glowy = 1;
	gra.snake.cialo_weza[0].x = 510;
	gra.snake.cialo_weza[0].y = 300;
	gra.snake.cialo_weza[0].kierunek = 1;
	gra.time.worldTime = 0;
	gra.snake.aktualny_rozmiar = 5;
	gra.time.snake_speed_licznik = 0;
	gra.points = 0;
	gra.time.snake_speed = 0.008;
	gra.total_progres = 1;
	gra.time.speedup_limit = 5;
	gra.time.licznik_zmiany = 0.4;
	gra.time.czas_zmiany = 0.4;
	gra.czerwony_bonus = 0;
	gra.time.t1 = SDL_GetTicks();
	init_cialo(gra);
}
//////////										
////////// ZAKONCZENIE ROZGRYWKI-KONIEC //////////										
////////// popraw
//////////
//////////
void wpisz_imie_gracza(stan_gry& gra, char imiegracza[])
{
	char text[128];
	int czarny = SDL_MapRGB(gra.screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(gra.screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(gra.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(gra.screen->format, 0x11, 0x11, 0xCC);
	int przycisk = 0;
		DrawRectangle(gra.screen, 0, 4, SCREEN_WIDTH, 50, czerwony, niebieski);  
		sprintf(text, " Napisz 5 literowe imie gracza");
		DrawString(gra.screen, gra.screen->w / 2 - strlen(text) * 8 / 2, 30, text, gra.charset);
		SDL_UpdateTexture(gra.scrtex, NULL, gra.screen->pixels, gra.screen->pitch);
		SDL_RenderCopy(gra.renderer, gra.scrtex, NULL, NULL);
		SDL_RenderPresent(gra.renderer);
		for (int i = 0; i < gra.snake.aktualny_rozmiar; i++)
		{
			DrawSurface(gra.screen, gra.eti2, gra.snake.cialo_weza[i].x, gra.snake.cialo_weza[i].y);
		}
		int czy_imie = 1;
		SDL_StartTextInput();
		while (przycisk == 0) {
			while (SDL_PollEvent(&gra.event)) {
				switch (gra.event.type) {
				case SDL_QUIT:
					przycisk = 1;
					break;
				case SDL_TEXTINPUT:
					if (czy_imie) {
						if (strlen(imiegracza) + strlen(gra.event.text.text) < sizeof(imiegracza) - 3) {
							strcat(imiegracza, gra.event.text.text);
						}
						else
						{
							przycisk = 1;
							strcat(imiegracza, gra.event.text.text);
						}
						DrawRectangle(gra.screen, 0, 4, SCREEN_WIDTH, 50, czerwony, niebieski);
						sprintf(text, " Napisz 5 literowe imie gracza %s", imiegracza);
						DrawString(gra.screen, gra.screen->w / 2 - strlen(text) * 8 / 2, 30, text, gra.charset);
						SDL_UpdateTexture(gra.scrtex, NULL, gra.screen->pixels, gra.screen->pitch);
						SDL_RenderCopy(gra.renderer, gra.scrtex, NULL, NULL);
						SDL_RenderPresent(gra.renderer);
					}
					break;
				}
			}
		}
		SDL_StopTextInput();
	
}
void wypisz_ranking(stan_gry& gra, char gracz1[], char gracz2[], char gracz3[], int wynik1, int wynik2, int wynik3)
{
	char text[128];
	int czarny = SDL_MapRGB(gra.screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(gra.screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(gra.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(gra.screen->format, 0x11, 0x11, 0xCC);
	DrawRectangle(gra.screen, 0, 4, SCREEN_WIDTH, 50, czerwony, niebieski);
	sprintf(text, "Najlepsze wyniki: 1. %s  %d  2. %s  %d  3. %s  %d ", gracz1, wynik1, gracz2, wynik2, gracz3, wynik3);
	DrawString(gra.screen, gra.screen->w / 2 - strlen(text) * 8 / 2, 10, text, gra.charset);
	sprintf(text, "N-nowa gra ESC-zakoncz ");
	DrawString(gra.screen, gra.screen->w / 2 - strlen(text) * 8 / 2, 30, text, gra.charset);
	SDL_UpdateTexture(gra.scrtex, NULL, gra.screen->pixels, gra.screen->pitch);
	SDL_RenderCopy(gra.renderer, gra.scrtex, NULL, NULL);
	SDL_RenderPresent(gra.renderer);
}
void zapisz_do_scores(stan_gry& gra)
{
	char imiegracza[6] = "";
	FILE* plik;
	plik = fopen("scores.txt", "r+");
	if (plik == NULL)
	{
		printf("scores error: \n");
	}
	char gracz1[6], gracz2[6], gracz3[6];
	int wynik1, wynik2, wynik3;
	fscanf(plik, "%s", &gracz1);
	fscanf(plik, "%d", &wynik1);
	fscanf(plik, "%s", &gracz2);
	fscanf(plik, "%d", &wynik2);
	fscanf(plik, "%s", &gracz3);
	fscanf(plik, "%d", &wynik3);
	if (gra.points > wynik3)
	{
		wpisz_imie_gracza(gra, imiegracza);
		if (gra.points > wynik3 && gra.points <= wynik2)
		{
			wynik3 = gra.points;
			memcpy(gracz3, imiegracza, sizeof(imiegracza));
		}
		else if (gra.points > wynik2 && gra.points <= wynik1)
		{
			wynik3 = wynik2;
			memcpy(gracz3, gracz2, sizeof(imiegracza));
			wynik2 = gra.points;
			memcpy(gracz2, imiegracza, sizeof(imiegracza));
		}
		else if (gra.points > wynik1)
		{
			wynik3 = wynik2;
			memcpy(gracz3, gracz2, sizeof(imiegracza));
			wynik2 = wynik1;
			memcpy(gracz2, gracz1, sizeof(imiegracza));
			wynik1 = gra.points;
			memcpy(gracz1, imiegracza, sizeof(imiegracza));
		}
		fseek(plik, 0, SEEK_SET);
		fprintf(plik, "%s ", gracz1);
		fprintf(plik, "  %d \n", wynik1);
		fprintf(plik, "%s ", gracz2);
		fprintf(plik, "  %d \n", wynik2);
		fprintf(plik, "%s ", gracz3);
		fprintf(plik, "  %d \n", wynik3);
	}
	wypisz_ranking(gra, gracz1, gracz2, gracz3, wynik1, wynik2, wynik3);
	fclose(plik);
}
void rozpocznij_gre(stan_gry& gra)
{
	if (czy_kolizja(gra))
	{
		zapisz_do_scores(gra);
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
							nowa_gra(gra);
							przycisk = 1;
							
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
//////////										
////////// ZAKONCZENIE ROZGRYWKI-KONIEC //////////										
////////// popraw
//////////
//////////

//////////										
////////// ZEBRANIE WISNI
//////////
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
//////////										
////////// ZEBRANIE WISNI-KONIEC
//////////

//////////										
////////// ZMIANY WÊ¯A
//////////
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
//////////										
////////// ZMIANY WÊ¯A-KONIEC
//////////

//////////										
////////// POJAWIANIE SIE BONUSU
//////////
int czy_bonus() ///szansa na pojawienie siê bonusu
{
	int pom = rand() % 100;
	if (pom <= 50)
		return 1;
	return 0;
}
void licznik_progresu(stan_gry& gra, int& bonus) ///sprawdzanie czy pojawi³ siê bonus i zmiany progresu bonusu
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
//////////										
////////// POJAWIANIE SIE BONUSU-KONIEC
//////////

//////////										
////////// ZAPISYWANIE I ODCZYTANIE STANU GRY
//////////
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
	fclose(plik);

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
	fclose(plik);
}
//////////										
////////// ZAPISYWANIE I ODCZYTANIE STANU GRY -KONIEC
//////////

//////////										
////////// INTERAKCJA UZYTKOWNIKA
//////////
void zdarzenie(stan_gry& gra)
{
	while (SDL_PollEvent(&gra.event)) {
		switch (gra.event.type) {
		case SDL_KEYDOWN:
			if (gra.event.key.keysym.sym == SDLK_ESCAPE) gra.quit = 1;
			else if (gra.event.key.keysym.sym == SDLK_n)
			{
				nowa_gra(gra);
			}
			else if (gra.event.key.keysym.sym == SDLK_d)
			{
				if (gra.snake.cialo_weza[0].x + gra.eti2->w / 2 != 1245)
				if(gra.snake.cialo_weza[0].kierunek!=2)
				gra.snake.zmiana_kierunku_glowy = 1;
			}
			else if (gra.event.key.keysym.sym == SDLK_a)
			{
				if (gra.snake.cialo_weza[0].x + gra.eti2->w / 2 != 45)
				if (gra.snake.cialo_weza[0].kierunek != 1)
				gra.snake.zmiana_kierunku_glowy = 2;
			}
			else if (gra.event.key.keysym.sym == SDLK_w)
			{
				if (gra.snake.cialo_weza[0].y - gra.eti2->h / 2 != 75)
				if (gra.snake.cialo_weza[0].kierunek != 4)
				gra.snake.zmiana_kierunku_glowy = 3;
			}
			else if (gra.event.key.keysym.sym == SDLK_s)
			{
				if (gra.snake.cialo_weza[0].kierunek != 3)
					if (gra.snake.cialo_weza[0].y - gra.eti2->h / 2 != 675)
				gra.snake.zmiana_kierunku_glowy = 4;
			}
			else if (gra.event.key.keysym.sym == SDLK_p)
			{
				zapisz(gra);
			}
			else if (gra.event.key.keysym.sym == SDLK_i)
			{
				odczyt(gra);
				gra.time.t1 = SDL_GetTicks();
			}
			break;
		case SDL_QUIT:
			gra.quit = 1;
			break;
		};
	};
}
//////////										
////////// INTERAKCJA UZYTKOWNIKA -koniec
//////////
void init_teleport(stan_gry& gra)
{
	for (int i = 0; i < 2; i++)
	{
		gra.teleport[i].x = (rand() % 40 +2)*30 ;
		gra.teleport[i].y = (rand() % 20 + 2) * 30+60;
		gra.teleport[i].x2 = (rand() % 40 + 2) * 30;
		gra.teleport[i].y2 = (rand() % 20 + 2) * 30+60;
	}
}
void rysuj_teleport(stan_gry& gra)
{
	char text[128];
	int czarny = SDL_MapRGB(gra.screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(gra.screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(gra.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(gra.screen->format, 0x11, 0x11, 0xCC);
	int inny = SDL_MapRGB(gra.screen->format, 0xF2, 0x21, 0xCC);

	DrawRectangle(gra.screen, gra.teleport[0].x, gra.teleport[0].y-15, 2, 30, czerwony, niebieski);
	DrawRectangle(gra.screen, gra.teleport[0].x2, gra.teleport[0].y2-15, 2, 30, zielony, niebieski);
	DrawRectangle(gra.screen, gra.teleport[1].x, gra.teleport[1].y-15, 2, 30, niebieski, niebieski);
	DrawRectangle(gra.screen, gra.teleport[1].x2, gra.teleport[1].y2-15, 2, 30, inny, niebieski);
	sprintf(text, "1");
	DrawString(gra.screen, gra.teleport[0].x-1, gra.teleport[0].y-30, text, gra.charset);
	sprintf(text, "1");
	DrawString(gra.screen, gra.teleport[0].x2 - 1, gra.teleport[0].y2 - 30, text, gra.charset);
	sprintf(text, "2");
	DrawString(gra.screen, gra.teleport[1].x - 1, gra.teleport[1].y - 30, text, gra.charset);
	sprintf(text, "2");
	DrawString(gra.screen, gra.teleport[1].x2 - 1, gra.teleport[1].y2 - 30, text, gra.charset);
}
void teleportacja(stan_gry& gra)
{

	
		for (int i = 0; i < gra.snake.aktualny_rozmiar; i++)
		{
			if (gra.snake.cialo_weza[i].kierunek != 3 && gra.snake.cialo_weza[i].kierunek != 4)
			{
				for (int j = 0; j < 2; j++)
				{
					if (gra.snake.cialo_weza[i].x == gra.teleport[j].x && gra.snake.cialo_weza[i].y == gra.teleport[j].y)
					{

						if (gra.teleport[j].x > gra.teleport[j].x2)
						{
							if (gra.snake.cialo_weza[i].kierunek == 1)
							{
								gra.snake.cialo_weza[i].x -= (gra.teleport[j].x - gra.teleport[j].x2) - 1;
								gra.snake.cialo_weza[i].y -= (gra.teleport[j].y - gra.teleport[j].y2);
							}
							else
							{
								gra.snake.cialo_weza[i].x -= (gra.teleport[j].x - gra.teleport[j].x2) + 1;
								gra.snake.cialo_weza[i].y -= (gra.teleport[j].y - gra.teleport[j].y2);
							}
						}
						else
						{
							if (gra.snake.cialo_weza[i].kierunek == 1)
							{
								gra.snake.cialo_weza[i].x += (gra.teleport[j].x2 - gra.teleport[j].x) + 1;
								gra.snake.cialo_weza[i].y += (gra.teleport[j].y2 - gra.teleport[j].y);
							}
							else
							{
								gra.snake.cialo_weza[i].x += (gra.teleport[j].x2 - gra.teleport[j].x) - 1;
								gra.snake.cialo_weza[i].y += (gra.teleport[j].y2 - gra.teleport[j].y);
							}
						}
					}
					else if (gra.snake.cialo_weza[i].x == gra.teleport[j].x2 && gra.snake.cialo_weza[i].y == gra.teleport[j].y2)
					{
						if (gra.teleport[j].x > gra.teleport[j].x2)
						{
							if (gra.snake.cialo_weza[i].kierunek == 1)
							{
								gra.snake.cialo_weza[i].x += (gra.teleport[j].x - gra.teleport[j].x2) + 1;
								gra.snake.cialo_weza[i].y += (gra.teleport[j].y - gra.teleport[j].y2);
							}
							else
							{
								gra.snake.cialo_weza[i].x += (gra.teleport[j].x - gra.teleport[j].x2) - 1;
								gra.snake.cialo_weza[i].y += (gra.teleport[j].y - gra.teleport[j].y2);
							}
						}
						else
						{
							if (gra.snake.cialo_weza[i].kierunek == 1)
							{
								gra.snake.cialo_weza[i].x -= (gra.teleport[j].x2 - gra.teleport[j].x) - 1;
								gra.snake.cialo_weza[i].y -= (gra.teleport[j].y2 - gra.teleport[j].y);
							}
							else
							{
								gra.snake.cialo_weza[i].x -= (gra.teleport[j].x2 - gra.teleport[j].x) + 1;
								gra.snake.cialo_weza[i].y -= (gra.teleport[j].y2 - gra.teleport[j].y);
							}
						}
					}
				}
				 
			}
		}
	
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
	init_cialo(gra);
	gra.wisnia_bonusowa.x = rand() % 1200 + 30;
	gra.wisnia_bonusowa.y = rand() % 550 + 130;
	init_teleport(gra);
	while (!gra.quit) {
		
		SDL_FillRect(gra.screen, NULL, czarny);
		rysuj_teleport(gra);
		teleportacja(gra);
		DrawSurface(gra.screen, gra.obramowanie, SCREEN_WIDTH / 2, (SCREEN_HEIGHT + 54) / 2);
		funkcjetimer(gra);
		rozpocznij_gre(gra);
		powieksz(gra);
		DrawSurface(gra.screen, gra.eti, gra.snake.cialo_weza[0].x, gra.snake.cialo_weza[0].y);
		if (gra.time.czas_zmiany <= gra.time.licznik_zmiany)
		{
			zdarzenie(gra);
		}
		if (gra.snake.cialo_weza[0].x % 30 == 0 && gra.snake.cialo_weza[0].y % 30 == 0)
		{
			gra.snake.cialo_weza[0].kierunek = gra.snake.zmiana_kierunku_glowy;
			if (gra.time.czas_zmiany <= gra.time.licznik_zmiany)
			gra.time.licznik_zmiany = 0;
		}
		zmiana_pozycji(gra);
		

		for (int i = 1; i < gra.snake.aktualny_rozmiar; i++)
		{
			if(i==gra.snake.aktualny_rozmiar-1)
				DrawSurface(gra.screen, gra.ogon2, gra.snake.cialo_weza[i].x, gra.snake.cialo_weza[i].y);
			else if(i%2==0)
				DrawSurface(gra.screen, gra.eti2, gra.snake.cialo_weza[i].x, gra.snake.cialo_weza[i].y);
			else
				DrawSurface(gra.screen, gra.cialo2, gra.snake.cialo_weza[i].x, gra.snake.cialo_weza[i].y);
			zmiana_pozycji_ciala(gra, i);
		}
		DrawSurface(gra.screen, gra.wisnia, gra.wisnia_powieksz.x, gra.wisnia_powieksz.y);
		DrawRectangle(gra.screen, 0, 4, SCREEN_WIDTH , 50, czerwony, niebieski); 
		sprintf(text, "Szsablasfsdon , czas trwania = %.1lf s  %.0lf klatek / s  %d   %d    %d %d %d ", gra.time.worldTime, gra.time.fps, gra.points, gra.snake.cialo_weza[0].x, gra.snake.cialo_weza[0].y, gra.teleport[0].x, gra.teleport[0].y);
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