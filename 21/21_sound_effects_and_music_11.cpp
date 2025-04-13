#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <audio.mp3>\n", argv[0]);
        return 1;
    }

    // Initialize SDL with video and audio
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create a simple SDL window
    SDL_Window* window = SDL_CreateWindow("SDL2 MP3 Player",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          400, 200,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Load the MP3 file
    Mix_Music* music = Mix_LoadMUS(argv[1]);
    if (!music) {
        printf("Mix_LoadMUS failed: %s\n", Mix_GetError());
        Mix_CloseAudio();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Play the music (loop once)
    Mix_PlayMusic(music, 1);
    printf("Playing: %s\nPress ESC or close the window to quit.\n", argv[1]);

    // Main event loop
    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        SDL_Delay(50); // Avoid CPU spin
    }

    // Clean up
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
