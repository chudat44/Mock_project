#define SDL_MAIN_HANDLED
#include "View/view.h"
#include <iostream>
#include <memory>
#include <string>
#include <filesystem>

// Function to check if SDL initialization was successful
bool checkSDLError(const std::string &message)
{
    const char *error = SDL_GetError();
    if (error && *error)
    {
        std::cerr << message << " SDL Error: " << error << std::endl;
        SDL_ClearError();
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    // Initialize SDL and its subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        std::cerr << "Failed to initialize SDL_image: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0)
    {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cerr << "Failed to initialize SDL_mixer: " << Mix_GetError() << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    try
    {
        // Create model instances
        std::shared_ptr<MediaLibrary> mediaLibrary = std::make_shared<MediaLibrary>();
        std::shared_ptr<PlaylistsManager> playlistsManager = std::make_shared<PlaylistsManager>();
        std::shared_ptr<USBManager> usbManager = std::make_shared<USBManager>();
        std::shared_ptr<MetadataManager> metadataManager = std::make_shared<MetadataManager>();

        std::shared_ptr<ViewManager> viewManager = std::make_shared<ViewManager>("Media", 800, 600);

        // Create the application controller
        ApplicationController appController(viewManager.get(), mediaLibrary, playlistsManager, usbManager, metadataManager);

        // Create main window and link to controller
        MainWindow mainWindow(&appController);
        // Initialize window (set size appropriate for your UI design)
        if (!mainWindow.initialize("Media Player", 1024, 768))
        {
            std::cerr << "Failed to create window!" << std::endl;
            throw std::runtime_error("Window initialization failed");
        }

        // Create and add all views to the window
        MediaListView *mediaListView = new MediaListView(&appController);
        PlaylistView *playlistView = new PlaylistView(&appController);
        PlaylistsListView *playlistsListView = new PlaylistsListView(&appController);
        PlayerView *playerView = new PlayerView(&appController);
        MetadataView *metadataView = new MetadataView(&appController);
        USBView *usbView = new USBView(&appController);

        // Add views to the main window
        mainWindow.addView("mediaList", mediaListView);
        mainWindow.addView("playlist", playlistView);
        mainWindow.addView("playlistsList", playlistsListView);
        mainWindow.addView("player", playerView);
        mainWindow.addView("metadata", metadataView);
        mainWindow.addView("usb", usbView);

        // Initialize the controllers and link with their respective views
        appController.initialize();

        // Set specific view interfaces to their controllers
        PlayerController *playerController = appController.getPlayerController();
        if (playerController)
        {
            playerController->setPlayerView(playerView);
        }

        PlaylistController *playlistController = appController.getPlaylistController();
        if (playlistController)
        {
            playlistController->setPlaylistsListView(playlistsListView);
            playlistController->setPlaylistView(playlistView);
        }

        USBController *usbController = appController.getUSBController();
        if (usbController)
        {
            usbController->setUSBView(usbView);
            usbController->initialize();
        }

        MetadataController *metadataController = appController.getMetadataController();
        if (metadataController)
        {
            metadataController->setMetadataView(metadataView);
        }

        // Scan the current directory for media files by default
        std::string startDir = std::filesystem::current_path().string();
        mediaLibrary->scanDirectory(startDir);

        // If a directory was provided as command-line argument, use that instead
        if (argc > 1)
        {
            std::string cmdDir = argv[1];
            if (std::filesystem::exists(cmdDir))
            {
                mediaLibrary->scanDirectory(cmdDir);
            }
        }

        // Load existing playlists
        playlistsManager->loadPlaylistsFromFile(*mediaLibrary);

        // Start with the media list view
        mainWindow.switchView("mediaList");

        // Run the application
        appController.run();

        // Main event loop (runs until exit)
        while (1)
        {
            // Handle SDL events
            mainWindow.handleEvents();

            // Render the current view
            mainWindow.render();

            // Small delay to prevent maxing out CPU
            SDL_Delay(16); // approximately 60 FPS
        }

        // Cleanup before exit
        if (usbController)
        {
            usbController->shutdown();
        }

        // Save playlists
        playlistsManager->savePlaylistsToFile();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    // Cleanup all SDL subsystems
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}