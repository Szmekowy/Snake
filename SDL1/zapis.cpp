#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480


#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
    int a = 50, b = 30;
    int t1, t2, quit, frames, rc;
    double delta, worldTime, fpsTimer, fps, distance, etiSpeed;
    SDL_Event event;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Surface* screen, * wisnia;
    SDL_Texture* scrtex;

    rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    if (rc != 0) {
        SDL_Quit();
        printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetWindowTitle(window, "przykladowa_gdsfuhweiuhreopjehrnpher");

    scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);


    wisnia = SDL_LoadBMP("./wisnia.bmp");
    if (!wisnia) {
        printf("SDL_LoadBMP(wisnia.bmp) error: %s\n", SDL_GetError());
        return 1;
    }

    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
    int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
    int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

    t1 = SDL_GetTicks();

    frames = 0;
    fpsTimer = 0;
    fps = 0;
    quit = 0;
    worldTime = 0;
    distance = 0;
    etiSpeed = 2;
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    while (!quit) {
        t2 = SDL_GetTicks();
        delta = (t2 - t1) * 0.001;
        t1 = t2;

        worldTime += delta;
        distance += etiSpeed * delta;
        SDL_FillRect(screen, NULL, zielony);
        SDL_Delay(2);
        SDL_Rect dest;
        dest.x = distance;
        dest.y = 100;
        dest.w = wisnia->w;
        dest.h = wisnia->h;
        if (distance > 680)
            distance = 1;
        SDL_BlitSurface(wisnia, NULL, screen, &dest);
        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
                else if (event.key.keysym.sym == SDLK_UP) { etiSpeed *= 2; }
                else if (event.key.keysym.sym == SDLK_DOWN) { etiSpeed /= 2; }
                break;

            case SDL_QUIT:
                quit = 1;
                break;
            }
        }
        frames++;
    }

    SDL_FreeSurface(wisnia);
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(scrtex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
