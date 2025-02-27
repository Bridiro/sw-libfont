#include <SDL.h>
#include <string.h>
#include "SDL_events.h"
#include "SDL_render.h"
#include "font.h"
#include "libfont.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 480

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} SDLContext;

void draw_line_fun(uint16_t x, uint16_t y, uint16_t lenght, uint32_t col) {
    SDL_SetRenderDrawColor(SDL_GetRenderer(SDL_GetWindowFromID(1)), col >> 16 & 0xff, col >> 8 & 0xff, col & 0xff, col >> 24 & 0xff);
    for (int i=0; i<lenght; i++) {
        SDL_RenderDrawPoint(SDL_GetRenderer(SDL_GetWindowFromID(1)), x+i, y);
    }
}

int main() {
    int size=0;
    int memory_occupied = 0;
    for (int i=0; i<94; i++) {
        size += glyphs[i].height * glyphs[i].width;
        memory_occupied += glyphs[i].size;
    }
    printf("Size of font: %d\n", size);
    printf("Memory occupied: %d bytes\n", memory_occupied);

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return 1;
    }

    SDLContext sdl_ctx = {
        .window = SDL_CreateWindow("Text Demo",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   WINDOW_WIDTH,
                                   WINDOW_HEIGHT,
                                   SDL_WINDOW_SHOWN),
        .renderer = NULL};

    if (!sdl_ctx.window) {
        SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    sdl_ctx.renderer = SDL_CreateRenderer(sdl_ctx.window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");

    if (!sdl_ctx.renderer) {
        SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(sdl_ctx.window);
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawBlendMode(SDL_GetRenderer(SDL_GetWindowFromID(1)), SDL_BLENDMODE_BLEND);

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }
        SDL_SetRenderDrawColor(sdl_ctx.renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdl_ctx.renderer);
        // Pulizia della finestra (sfondo bianco)
        // Renderizzazione del testo
        draw_text(50, 50, LEFT, "CIAO, SDL", 0xffffffff, 0.3, draw_line_fun);
        draw_text(50, 80, LEFT, "Funziona", 0xffffffff, 1, draw_line_fun);
        draw_text(50, 160, LEFT, "Maybe", 0xffffffff, 1.5, draw_line_fun);

        // Presenta il rendering sullo schermo
        SDL_RenderPresent(sdl_ctx.renderer);

        // Attendere un po' per vedere il risultato
        SDL_Delay(16);
    }

    // Pulizia e chiusura
    SDL_DestroyRenderer(sdl_ctx.renderer);
    SDL_DestroyWindow(sdl_ctx.window);
    SDL_Quit();

    return 0;
}

