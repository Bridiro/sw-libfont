#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include "SDL_events.h"
#include "font.h"  // Includi il file generato con i dati del font

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Funzione per renderizzare un singolo carattere
void render_glyph(const Glyph *glyph, int x, int y, float scale, SDL_Renderer *renderer) {
    for (int j = 0; j < glyph->height * scale; j++) {
        for (int i = 0; i < glyph->width * scale; i++) {
            // Calcola le coordinate scalate nella bitmap SDF originale
            int sdf_x = i / scale;
            int sdf_y = j / scale;

            // Ottieni il valore SDF
            int sdf_value = glyph->sdf[sdf_y * glyph->width + sdf_x];

            // Controlla se il pixel è visibile
            if (sdf_value > 110) {  // Soglia di visibilità
                SDL_SetRenderDrawColor(renderer, sdf_value, sdf_value, sdf_value, 255);  // Colore in scala di grigi
                SDL_RenderDrawPoint(renderer, x + i, y + j);  // Disegna il punto scalato
            }
        }
    }
}

// Funzione per renderizzare un testo
void render_text(const char *text, int x, int y, float scale, SDL_Renderer *renderer) {
    while (*text) {
        int char_code = *text++;
        if (char_code >= 32 && char_code <= 126) {
            const Glyph *glyph = &glyphs[char_code - 32];
            render_glyph(glyph, x, y, scale, renderer);
            x += glyph->width * scale;  // Sposta il cursore per il prossimo carattere in base allo scaling
        }
    }
}

int main() {
    // Inizializzazione di SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Creazione della finestra SDL
    SDL_Window *window = SDL_CreateWindow("SDL Font Rendering",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Creazione del renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event event;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
        }
        // Pulizia della finestra (sfondo bianco)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Renderizzazione del testo
        render_text("Ciao, SDL.", 50, 50, 0.5,  renderer);
        render_text("Funzia.", 50, 100, 1, renderer);
        render_text("Maybe.", 50, 160, 1.5, renderer);

        // Presenta il rendering sullo schermo
        SDL_RenderPresent(renderer);

        // Attendere un po' per vedere il risultato
        SDL_Delay(200);
    }

    // Pulizia e chiusura
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

