#include "controller.h"
#include <iostream>
#include <nlohmann/json.hpp>

// ApplicationController implementation
ApplicationController::ApplicationController(ViewManagerInterface *vm)
    : viewManager(vm), applicationRunning(false)
{
}

ApplicationController::~ApplicationController()
{
    // Ensure clean shutdown
    if (applicationRunning)
    {
        exit();
    }
}

bool ApplicationController::initialize(MediaListInterface *mlView, PlayerInterface *plView,
                                       PlaylistsListInterface *pllView, MetadataInterface *mdView)
{
    std::cout << "Initializing application..." << std::endl;

    try
    {
        // Initialize controllers with appropriate views and models
        playerController = std::make_unique<PlayerController>(plView);
        playlistController = std::make_unique<PlaylistsListController>(pllView);
        mediaListController = std::make_unique<MediaListController>(mlView);
        metadataController = std::make_unique<MetadataController>(mdView);

        // Initialize controllers
        if (!playerController->initialize())
        {
            std::cerr << "Failed to initialize player controller!" << std::endl;
            return false;
        }

        // Load saved playlists
        playlistController->loadAllPlaylists();

        // Callback init
        playlistController->setOnPlaylistSelectedCallback(
            [this](std::shared_ptr<PlaylistModel> playlist)
            {
                mediaListController->loadPlaylist(playlist);
            });
        playlistController->setOnPlaylistPlayCallback(
            [this](std::shared_ptr<PlaylistModel> playlist)
            {
                mediaListController->loadPlaylist(playlist);
                metadataController->preloadMetadata(playlist->getAllMediaFiles());
                playerController->playPlaylist(playlist->getAllMediaFiles());
            });

        mediaListController->setOnMediaSelectedCallback(
            [this](std::shared_ptr<MediaFileModel> media)
            {
                metadataController->loadMetadata(media);
            });
        mediaListController->setOnMediaPlayCallback(
            [this](const std::vector<std::shared_ptr<MediaFileModel>> &playlist, int index)
            {
                metadataController->preloadMetadata(playlist);
                metadataController->loadMetadata(playlist[index]); // must load first
                playerController->playPlaylist(playlist, index);
            });
        mediaListController->setOnOtherPlaylistCallback(
            [this](const std::string &name)
            {
                for (auto &playlist : playlistController->getAllPlaylists())
                {
                    if (playlist->getPlaylistName() == name)
                        return playlist;
                }
                return std::shared_ptr<PlaylistModel>(nullptr);
            });

        applicationRunning = true;
        std::cout << "Application initialized successfully!" << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error during initialization: " << e.what() << std::endl;
        return false;
    }
}

void ApplicationController::run()
{
    if (!applicationRunning)
    {
        std::cerr << "Cannot run application: not initialized!" << std::endl;
        return;
    }

    std::cout << "Starting application main loop..." << std::endl;

    // Main application loop
    while (applicationRunning)
    {
        // Let view manager handle events and rendering
        viewManager->handleEvents();

        // Check if application should exit
        if (viewManager->shouldExit())
        {
            applicationRunning = false;
        }

        // Render everything
        viewManager->render();

        // Prevent CPU overuse
        SDL_Delay(16); // ~60 fps
    }
}

void ApplicationController::exit()
{
    applicationRunning = false;

    // Clean up controllers
    playerController->shutdown();

    // Save data
    playlistController->saveAllPlaylists();

    // Additional cleanup if needed
}

PlayerController *ApplicationController::getPlayerController() const
{
    return playerController.get();
}

MediaListController *ApplicationController::getMediaListController() const
{
    return mediaListController.get();
}

PlaylistsListController *ApplicationController::getPlaylistsListController() const
{
    return playlistController.get();
}

MetadataController *ApplicationController::getMetadataController() const
{
    return metadataController.get();
}
