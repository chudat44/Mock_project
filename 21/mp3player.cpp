#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 150

typedef struct {
    char* filename;
    Mix_Music* music;
    bool isPlaying;
    bool isPaused;
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool quit;
} AudioPlayer;

void initSDL(AudioPlayer* player) {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    // Initialize SDL_mixer with MP3 support
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        SDL_Quit();
        exit(1);
    }

    // Initialize formats we want to use
    int formats = MIX_INIT_MP3;
    if ((Mix_Init(formats) & formats) != formats) {
        printf("SDL_mixer couldn't initialize MP3 support! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        exit(1);
    }

    // Create window
    player->window = SDL_CreateWindow("MP3 Player", 
                                     SDL_WINDOWPOS_UNDEFINED, 
                                     SDL_WINDOWPOS_UNDEFINED, 
                                     WINDOW_WIDTH, WINDOW_HEIGHT, 
                                     SDL_WINDOW_SHOWN);
    if (player->window == NULL) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        exit(1);
    }

    // Create renderer
    player->renderer = SDL_CreateRenderer(player->window, -1, SDL_RENDERER_ACCELERATED);
    if (player->renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(player->window);
        Mix_CloseAudio();
        SDL_Quit();
        exit(1);
    }
}

void loadMusic(AudioPlayer* player, const char* filename) {
    // Free existing music if loaded
    if (player->music != NULL) {
        Mix_FreeMusic(player->music);
        player->music = NULL;
    }

    // Load the music file
    player->music = Mix_LoadMUS(filename);
    if (player->music == NULL) {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }
    
    player->filename = (char*)filename;
    player->isPlaying = false;
    player->isPaused = false;
    
    printf("Loaded music: %s\n", filename);
}

void playMusic(AudioPlayer* player) {
    if (player->music != NULL) {
        if (player->isPaused) {
            Mix_ResumeMusic();
            player->isPaused = false;
        } else if (!player->isPlaying) {
            Mix_PlayMusic(player->music, 0); // Play once
            player->isPlaying = true;
        }
    }
}

void pauseMusic(AudioPlayer* player) {
    if (player->isPlaying && !player->isPaused) {
        Mix_PauseMusic();
        player->isPaused = true;
    }
}

void stopMusic(AudioPlayer* player) {
    Mix_HaltMusic();
    player->isPlaying = false;
    player->isPaused = false;
}

void drawControls(AudioPlayer* player) {
    // Set drawing color to light gray for background
    SDL_SetRenderDrawColor(player->renderer, 240, 240, 240, 255);
    SDL_RenderClear(player->renderer);

    // Drawing buttons and status is simplified here
    // A real implementation would use textures and proper UI rendering
    
    // Set drawing colors for buttons
    SDL_Rect playRect = {50, 50, 60, 30};
    SDL_Rect pauseRect = {120, 50, 60, 30};
    SDL_Rect stopRect = {190, 50, 60, 30};
    SDL_Rect quitRect = {260, 50, 60, 30};
    
    // Draw play button (green)
    SDL_SetRenderDrawColor(player->renderer, 0, 200, 0, 255);
    SDL_RenderFillRect(player->renderer, &playRect);
    
    // Draw pause button (yellow)
    SDL_SetRenderDrawColor(player->renderer, 200, 200, 0, 255);
    SDL_RenderFillRect(player->renderer, &pauseRect);
    
    // Draw stop button (red)
    SDL_SetRenderDrawColor(player->renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(player->renderer, &stopRect);
    
    // Draw quit button (gray)
    SDL_SetRenderDrawColor(player->renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(player->renderer, &quitRect);
    
    // Draw filename at the top
    SDL_SetRenderDrawColor(player->renderer, 0, 0, 0, 255);
    SDL_Rect textRect = {10, 10, WINDOW_WIDTH - 20, 30};
    SDL_RenderDrawRect(player->renderer, &textRect);
    
    // Present the renderer
    SDL_RenderPresent(player->renderer);
}

void handleEvents(AudioPlayer* player) {
    SDL_Event e;
    
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            player->quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_SPACE:
                    if (player->isPaused || !player->isPlaying) {
                        playMusic(player);
                    } else {
                        pauseMusic(player);
                    }
                    break;
                case SDLK_s:
                    stopMusic(player);
                    break;
                case SDLK_ESCAPE:
                    player->quit = true;
                    break;
            }
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                int x = e.button.x;
                int y = e.button.y;
                
                // Check if mouse click was on any button
                if (y >= 50 && y <= 80) {
                    if (x >= 50 && x <= 110) {
                        // Play button
                        playMusic(player);
                    } else if (x >= 120 && x <= 180) {
                        // Pause button
                        pauseMusic(player);
                    } else if (x >= 190 && x <= 250) {
                        // Stop button
                        stopMusic(player);
                    } else if (x >= 260 && x <= 320) {
                        // Quit button
                        player->quit = true;
                    }
                }
            }
        }
    }
}

void cleanup(AudioPlayer* player) {
    // Free the music
    if (player->music != NULL) {
        Mix_FreeMusic(player->music);
        player->music = NULL;
    }
    
    // Destroy renderer
    SDL_DestroyRenderer(player->renderer);
    
    // Destroy window
    SDL_DestroyWindow(player->window);
    
    // Quit SDL_mixer
    Mix_CloseAudio();
    Mix_Quit();
    
    // Quit SDL
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    AudioPlayer player;
    
    // Initialize all fields to 0/NULL
    player.filename = NULL;
    player.music = NULL;
    player.isPlaying = false;
    player.isPaused = false;
    player.window = NULL;
    player.renderer = NULL;
    player.quit = false;
    
    // Initialize SDL
    initSDL(&player);
    
    // Check for filename argument
    if (argc < 2) {
        printf("Usage: %s <mp3_file>\n", argv[0]);
        cleanup(&player);
        return 1;
    }
    
    // Load the music
    loadMusic(&player, argv[1]);
    
    // Main loop
    while (!player.quit) {
        // Handle events
        handleEvents(&player);
        
        // Update status
        player.isPlaying = Mix_PlayingMusic() == 1;
        
        // Draw controls
        drawControls(&player);
        
        // Small delay to prevent high CPU usage
        SDL_Delay(16);
    }
    
    // Clean up
    cleanup(&player);
    
    return 0;
}