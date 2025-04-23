#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "View/Interface/Iview.h"
#include "Model/playlist.h"
#include "Model/manager.h"

#include "player.h"
#include "playlist.h"
#include "medialist.h"
#include "metadata.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <unistd.h>
#include <thread>
#include <mutex>
#include <atomic>

#include <functional>
#include <chrono>
#include <condition_variable>
#include <filesystem>


// Application main controller
class ApplicationController
{
private:
    // Child controllers
    std::unique_ptr<class PlayerController> playerController;
    std::unique_ptr<class PlaylistsListController> playlistController;
    std::unique_ptr<class MediaListController> mediaListController;
    std::unique_ptr<class MetadataController> metadataController;

    // Current state
    bool applicationRunning;

    ViewManagerInterface *viewManager;

    // Mutex for thread safety
    std::mutex controllerMutex;

public:
    ApplicationController(ViewManagerInterface *vm);

    ~ApplicationController();

    // Initialization
    bool initialize(MediaListInterface *mlView, PlayerInterface *plView,
                    PlaylistsListInterface *pllView, MetadataInterface *mdView);

    // Application flow
    void run();
    void exit();

    // Accessors
    PlayerController *getPlayerController() const;
    MediaListController *getMediaListController() const;
    PlaylistsListController *getPlaylistsListController() const;
    MetadataController *getMetadataController() const;
};

#endif