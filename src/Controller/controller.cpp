#include "controller.h"

#include <iostream>

// ApplicationController implementation
ApplicationController::ApplicationController(
    ViewManagerInterface *vm,
    std::shared_ptr<MediaLibrary> library,
    std::shared_ptr<PlaylistsManager> playlists,
    std::shared_ptr<USBManager> usb,
    std::shared_ptr<MetadataManager> metadata) : Controller(vm),
                                                 mediaLibrary(library),
                                                 playlistsManager(playlists),
                                                 usbManager(usb),
                                                 metadataManager(metadata),
                                                 applicationRunning(false)
{
    // Create child controllers
    playerController = std::make_unique<PlayerController>(vm);
    playlistController = std::make_unique<PlaylistController>(vm, playlists, library);
    usbController = std::make_unique<USBController>(vm, usb, library);
    metadataController = std::make_unique<MetadataController>(vm, metadata);
}

ApplicationController::~ApplicationController()
{
    // Ensure clean shutdown
    if (applicationRunning)
    {
        exit();
    }
}

void ApplicationController::initialize()
{
    // Initialize SDL subsystems if not already initialized
    if (SDL_WasInit(SDL_INIT_AUDIO) == 0)
    {
        SDL_InitSubSystem(SDL_INIT_AUDIO);
    }

    // Initialize controllers
    playerController->initialize();
    usbController->initialize();

    // Load saved playlists
    playlistController->loadAllPlaylists();

    // Set default directory to current directory
    char cwd[FILENAME_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        currentDirectory = std::string(cwd);
        loadDirectory(currentDirectory);
    }
    else
    {
        // Default to home directory if current directory can't be determined
        currentDirectory = getenv("HOME");
        loadDirectory(currentDirectory);
    }

    applicationRunning = true;
}

void ApplicationController::handleUserCommand(UserCommand cmd)
{
    switch (cmd)
    {
    case UserCommand::PLAY:
        playerController->play();
        break;
    case UserCommand::PAUSE:
        playerController->pause();
        break;
    case UserCommand::STOP:
        playerController->stop();
        break;
    case UserCommand::NEXT:
        playerController->next();
        break;
    case UserCommand::PREVIOUS:
        playerController->previous();
        break;
    case UserCommand::VOLUME_UP:
        playerController->volumeUp();
        break;
    case UserCommand::VOLUME_DOWN:
        playerController->volumeDown();
        break;
    case UserCommand::SEEK_FORWARD:
        playerController->seekForward();
        break;
    case UserCommand::SEEK_BACKWARD:
        playerController->seekBackward();
        break;
    case UserCommand::EXIT:
        exit();
        break;
    default:
        break;
    }
}

void ApplicationController::handleUserInput(const SDL_Event &event)
{
    // Handle keyboard shortcuts
    if (event.type == SDL_KEYDOWN)
    {
        switch (event.key.keysym.sym)
        {
        case SDLK_SPACE:
            if (playerController->isMediaPlaying() && !playerController->isMediaPaused())
            {
                playerController->pause();
            }
            else
            {
                playerController->play();
            }
            break;
        case SDLK_RIGHT:
            if (SDL_GetModState() & KMOD_CTRL)
            {
                playerController->next();
            }
            else
            {
                playerController->seekForward();
            }
            break;
        case SDLK_LEFT:
            if (SDL_GetModState() & KMOD_CTRL)
            {
                playerController->previous();
            }
            else
            {
                playerController->seekBackward();
            }
            break;
        case SDLK_UP:
            playerController->volumeUp();
            break;
        case SDLK_DOWN:
            playerController->volumeDown();
            break;
        case SDLK_ESCAPE:
            // Handle escape key for navigation
            break;
        }
    }
}

void ApplicationController::loadDirectory(const std::string &path)
{
    std::lock_guard<std::mutex> lock(controllerMutex);
    currentDirectory = path;
    mediaLibrary->scanDirectory(path);

    // Notify views to update
    if (viewManager)
    {
        viewManager->update();
    }
}

void ApplicationController::refreshCurrentDirectory()
{
    loadDirectory(currentDirectory);
}

std::vector<std::shared_ptr<MediaFileModel>> ApplicationController::searchMedia(const std::string &query)
{
    std::lock_guard<std::mutex> lock(controllerMutex);
    return mediaLibrary->searchMedia(query);
}

void ApplicationController::run()
{
    applicationRunning = true;

    // Main application loop would be here in a real application
    // but this will be handled by the SDL event loop and view manager
}

void ApplicationController::exit()
{
    applicationRunning = false;

    // Clean up controllers
    playerController->shutdown();
    usbController->shutdown();

    // Save data
    playlistController->saveAllPlaylists();

    // Additional cleanup if needed
}

void ApplicationController::navigateToMainMenu() {
    std::lock_guard<std::mutex> lock(controllerMutex);
    
    // Log navigation action
    std::cout << "Navigating to main menu" << std::endl;
    
    // Save any necessary state before changing views
    if (viewManager) {
        // Change the view state to main menu
        viewManager->changeState(ViewState::MAIN_MENU);
    } else {
        std::cerr << "Error: View manager not initialized!" << std::endl;
    }
}

void ApplicationController::navigateToMediaList() {
    std::lock_guard<std::mutex> lock(controllerMutex);
    
    // Log navigation action
    std::cout << "Navigating to media list" << std::endl;
    
    // Make sure we have at least some media files loaded
    if (mediaLibrary && mediaLibrary->getMediaFiles().empty()) {
        // If no directory is loaded yet, load the current directory
        if (currentDirectory.empty()) {
            currentDirectory = "."; // Default to current directory
        }
        
        // Load the media files from the current directory
        mediaLibrary->scanDirectory(currentDirectory);
        
        // If still empty, show a message
        if (mediaLibrary->getMediaFiles().empty()) {
            if (viewManager) {
                viewManager->showDialog("No media files found in the current directory.");
            }
            // Still proceed to the view so user can navigate to other directories
        }
    }
    
    // Change the view state to media list
    if (viewManager) {
        viewManager->changeState(ViewState::MEDIA_LIST);
    } else {
        std::cerr << "Error: View manager not initialized!" << std::endl;
    }
}

void ApplicationController::navigateToPlaylistsList() {
    std::lock_guard<std::mutex> lock(controllerMutex);
    
    // Log navigation action
    std::cout << "Navigating to playlists menu" << std::endl;
    
    // Make sure the playlist controller is initialized
    if (!playlistController) {
        std::cerr << "Error: Playlist controller not initialized!" << std::endl;
        return;
    }
    
    // Update the playlists list before showing it
    if (playlistsManager) {
        // Load playlists if they haven't been loaded yet
        if (playlistsManager->getAllPlaylists().empty()) {
            playlistsManager->loadPlaylistsFromFile(*mediaLibrary);
        }
        
        // Update the playlist controller with current playlists
        playlistController->loadAllPlaylists();
    }
    
    // Change the view state to playlists menu
    if (viewManager) {
        viewManager->changeState(ViewState::PLAYLIST_MENU);
    } else {
        std::cerr << "Error: View manager not initialized!" << std::endl;
    }
}

void ApplicationController::navigateToUSBDevices() {
    std::lock_guard<std::mutex> lock(controllerMutex);
    
    // Log navigation action
    std::cout << "Navigating to USB devices" << std::endl;
    
    // Make sure the USB controller is initialized
    if (!usbController) {
        std::cerr << "Error: USB controller not initialized!" << std::endl;
        return;
    }
    
    // Check if USB manager is available
    if (!usbManager) {
        std::cerr << "Error: USB manager not initialized!" << std::endl;
        if (viewManager) {
            viewManager->showDialog("USB manager not available. Cannot detect USB devices.");
        }
        return;
    }
    
    try {
        // Trigger USB detection
        usbController->detectUSBDevices();
        
        // This might take a moment, so we could show a loading indicator
        // For now, just add a small delay to simulate detection
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Change the view state to USB devices
        if (viewManager) {
            viewManager->changeState(ViewState::USB_DEVICES);
        } else {
            std::cerr << "Error: View manager not initialized!" << std::endl;
        }
    } catch (const std::exception& e) {
        // Handle any errors that occur during USB detection
        std::cerr << "Error detecting USB devices: " << e.what() << std::endl;
        if (viewManager) {
            viewManager->showDialog("Error detecting USB devices: " + std::string(e.what()));
        }
    }
}

PlayerController *ApplicationController::getPlayerController() const
{
    return playerController.get();
}

PlaylistController *ApplicationController::getPlaylistController() const
{
    return playlistController.get();
}

USBController *ApplicationController::getUSBController() const
{
    return usbController.get();
}

MetadataController *ApplicationController::getMetadataController() const
{
    return metadataController.get();
}

std::shared_ptr<MediaLibrary> ApplicationController::getMediaLibrary() const
{
    return mediaLibrary;
}

std::shared_ptr<PlaylistsManager> ApplicationController::getPlaylistsManager() const
{
    return playlistsManager;
}

// PlaylistController implementation
PlaylistController::PlaylistController(
    ViewManagerInterface *vm,
    std::shared_ptr<PlaylistsManager> playlists,
    std::shared_ptr<MediaLibrary> library) : Controller(vm), playlistsManager(playlists), mediaLibrary(library),
                                             playlistsListView(nullptr), playlistView(nullptr)
{
}

void PlaylistController::setPlaylistsListView(PlaylistsListInterface *view)
{
    playlistsListView = view;
}

void PlaylistController::setPlaylistView(PlaylistInterface *view)
{
    playlistView = view;
}

void PlaylistController::createPlaylist(const std::string &name)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    playlistsManager->createPlaylist(name);
    updatePlaylistsListView();
}

void PlaylistController::deletePlaylist(const std::string &name)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    auto playlist = playlistsManager->getPlaylist(name);
    if (playlist)
    {
        playlistsManager->deletePlaylist(playlist);
        if (currentPlaylist && currentPlaylist->getPlaylistName() == name)
        {
            currentPlaylist = nullptr;
        }
        updatePlaylistsListView();
    }
}

void PlaylistController::deletePlaylist(int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    auto playlists = playlistsManager->getAllPlaylists();
    if (index >= 0 && index < playlists.size())
    {
        auto playlist = playlists[index];
        playlistsManager->deletePlaylist(playlist);
        if (currentPlaylist && currentPlaylist == playlist)
        {
            currentPlaylist = nullptr;
        }
        updatePlaylistsListView();
    }
}

void PlaylistController::renamePlaylist(const std::string &oldName, const std::string &newName)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    auto playlist = playlistsManager->getPlaylist(oldName);
    if (playlist)
    {
        playlist->setPlaylistName(newName);
        saveAllPlaylists();
        updatePlaylistsListView();
    }
}

void PlaylistController::loadAllPlaylists()
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    // This would call playlistsManager->loadPlaylistsFromFile() in a real implementation
    updatePlaylistsListView();
}

void PlaylistController::loadPlaylist(const std::string &name)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    currentPlaylist = playlistsManager->getPlaylist(name);
    updatePlaylistView();
}

void PlaylistController::loadPlaylist(int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    auto playlists = playlistsManager->getAllPlaylists();
    if (index >= 0 && index < playlists.size())
    {
        currentPlaylist = playlists[index];
        updatePlaylistView();
    }
}

void PlaylistController::addToPlaylist(const std::string &playlistName, std::shared_ptr<MediaFileModel> file)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    auto playlist = playlistsManager->getPlaylist(playlistName);
    if (playlist)
    {
        playlist->addMediaFile(file);
        saveAllPlaylists();
        if (currentPlaylist && currentPlaylist->getPlaylistName() == playlistName)
        {
            updatePlaylistView();
        }
    }
}

void PlaylistController::addToCurrentPlaylist(std::shared_ptr<MediaFileModel> file)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    if (currentPlaylist)
    {
        currentPlaylist->addMediaFile(file);
        saveAllPlaylists();
        updatePlaylistView();
    }
}

void PlaylistController::removeFromPlaylist(const std::string &playlistName, int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    auto playlist = playlistsManager->getPlaylist(playlistName);
    if (playlist && index >= 0 && index < playlist->size())
    {
        playlist->removeMediaFile(index);
        saveAllPlaylists();
        if (currentPlaylist && currentPlaylist->getPlaylistName() == playlistName)
        {
            updatePlaylistView();
        }
    }
}

void PlaylistController::removeFromCurrentPlaylist(int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    if (currentPlaylist && index >= 0 && index < currentPlaylist->size())
    {
        currentPlaylist->removeMediaFile(index);
        saveAllPlaylists();
        updatePlaylistView();
    }
}

void PlaylistController::moveItemUp(int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    if (currentPlaylist && index > 0 && index < currentPlaylist->size())
    {
        // This would swap the item at index with the one at index-1
        // Since this is not implemented in your model class, we'd add this functionality here
        auto files = currentPlaylist->getAllMediaFiles(); // Get a copy
        if (index > 0 && index < files.size())
        {
            std::swap(files[index], files[index - 1]);

            // Recreate playlist with new order
            auto name = currentPlaylist->getPlaylistName();
            playlistsManager->deletePlaylist(currentPlaylist);
            playlistsManager->createPlaylist(name);
            currentPlaylist = playlistsManager->getPlaylist(name);

            for (auto &file : files)
            {
                currentPlaylist->addMediaFile(file);
            }

            saveAllPlaylists();
            updatePlaylistView();
        }
    }
}

void PlaylistController::moveItemDown(int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    if (currentPlaylist && index >= 0 && index < currentPlaylist->size() - 1)
    {
        // This would swap the item at index with the one at index+1
        auto files = currentPlaylist->getAllMediaFiles(); // Get a copy
        if (index >= 0 && index < files.size() - 1)
        {
            std::swap(files[index], files[index + 1]);

            // Recreate playlist with new order
            auto name = currentPlaylist->getPlaylistName();
            playlistsManager->deletePlaylist(currentPlaylist);
            playlistsManager->createPlaylist(name);
            currentPlaylist = playlistsManager->getPlaylist(name);

            for (auto &file : files)
            {
                currentPlaylist->addMediaFile(file);
            }

            saveAllPlaylists();
            updatePlaylistView();
        }
    }
}

void PlaylistController::updatePlaylistsListView()
{
    if (playlistsListView)
    {
        // Update the view with current playlistsstd::vector<std::string> playlistNames;
        auto playlists = playlistsManager->getAllPlaylists();
        std::vector<std::string> playlistNames;
        for (const auto &playlist : playlists)
        {
            playlistNames.push_back(playlist->getPlaylistName());
        }
        playlistsListView->setPlaylists(playlistNames);
    }
}

void PlaylistController::updatePlaylistView()
{
    if (playlistView && currentPlaylist)
    {
        playlistView->setCurrentPlaylist(currentPlaylist.get());
    }
}

void PlaylistController::viewPlaylistContent(const std::string &name)
{
    loadPlaylist(name);
    if (viewManager && currentPlaylist)
    {
        // This would navigate to the playlist content view
        // viewManager->changeState(ViewState::PLAYLIST_CONTENT);
    }
}

void PlaylistController::saveAllPlaylists()
{
    playlistsManager->savePlaylistsToFile();
}

void PlaylistController::saveCurrentPlaylist()
{
    saveAllPlaylists(); // Since we don't have a specific method for one playlist
}

std::shared_ptr<PlaylistModel> PlaylistController::getCurrentPlaylist() const
{
    return currentPlaylist;
}

std::vector<std::shared_ptr<PlaylistModel>> PlaylistController::getAllPlaylists() const
{
    return playlistsManager->getAllPlaylists();
}

void PlaylistController::scanDirectoryForMedia(const std::string &path)
{
    mediaLibrary->scanDirectory(path);
}

// Static member for callback
PlayerController *PlayerController::instance = nullptr;

// PlayerController implementation
PlayerController::PlayerController(ViewManagerInterface *vm)
    : Controller(vm),
      currentMusic(nullptr),
      isPlaying(false),
      isPaused(false),
      volume(SDL_MIX_MAXVOLUME / 2), // 50% volume
      currentPosition(0),
      totalDuration(0),
      currentPlaylistIndex(-1),
      threadRunning(false),
      playerView(nullptr)
{
    // Set static instance for callback
    instance = this;
}

PlayerController::~PlayerController()
{
    shutdown();
}

bool PlayerController::initialize()
{
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        return false;
    }

    // Set volume
    Mix_VolumeMusic(volume);

    // Set up music finished callback
    Mix_HookMusicFinished(musicFinishedCallback);

    return true;
}

void PlayerController::shutdown()
{
    // Stop playback and close audio
    stop();

    // Stop thread
    if (threadRunning)
    {
        threadRunning = false;
        if (playbackThread.joinable())
        {
            playbackThread.join();
        }
    }

    // Close audio
    Mix_CloseAudio();
}

void PlayerController::setPlayerView(PlayerInterface *view)
{
    playerView = view;
}

void PlayerController::play()
{
    std::lock_guard<std::mutex> lock(playbackMutex);

    if (currentMedia)
    {
        if (isPaused)
        {
            // Resume paused playback
            Mix_ResumeMusic();
            isPaused = false;
            isPlaying = true;
        }
        else if (!isPlaying)
        {
            // Start new playback
            playCurrentMedia();
        }
    }
    else if (currentPlaylist && currentPlaylist->size() > 0)
    {
        // Play first track of playlist if no current media
        currentPlaylistIndex = 0;
        auto media = currentPlaylist->getMediaFile(0);
        if (media)
        {
            currentMedia = media;
            playCurrentMedia();
        }
    }

    updatePlayerView();
}

void PlayerController::playMedia(std::shared_ptr<MediaFileModel> media)
{
    if (!media)
        return;

    std::lock_guard<std::mutex> lock(playbackMutex);

    // Stop current playback
    if (isPlaying)
    {
        Mix_HaltMusic();
    }

    currentMedia = media;
    currentPlaylist = nullptr; // Clear playlist context
    currentPlaylistIndex = -1;

    playCurrentMedia();
    updatePlayerView();
}

void PlayerController::playPlaylist(std::shared_ptr<PlaylistModel> playlist, int startIndex)
{
    if (!playlist || playlist->size() == 0)
        return;

    std::lock_guard<std::mutex> lock(playbackMutex);

    // Stop current playback
    if (isPlaying)
    {
        Mix_HaltMusic();
    }

    currentPlaylist = playlist;
    currentPlaylistIndex = (startIndex >= 0 && startIndex < playlist->size()) ? startIndex : 0;

    currentMedia = playlist->getMediaFile(currentPlaylistIndex);
    if (currentMedia)
    {
        playCurrentMedia();
    }

    updatePlayerView();
}

void PlayerController::pause()
{
    std::lock_guard<std::mutex> lock(playbackMutex);

    if (isPlaying && !isPaused)
    {
        Mix_PauseMusic();
        isPaused = true;
        updatePlayerView();
    }
    else if (isPlaying && isPaused)
    {
        Mix_ResumeMusic();
        isPaused = false;
        updatePlayerView();
    }
}

void PlayerController::stop()
{
    std::lock_guard<std::mutex> lock(playbackMutex);

    if (isPlaying)
    {
        Mix_HaltMusic();
        isPlaying = false;
        isPaused = false;
        currentPosition = 0;

        // Free music resources
        if (currentMusic)
        {
            Mix_FreeMusic(currentMusic);
            currentMusic = nullptr;
        }

        updatePlayerView();
    }
}

void PlayerController::next()
{
    std::lock_guard<std::mutex> lock(playbackMutex);

    if (currentPlaylist && currentPlaylistIndex >= 0)
    {
        // Move to next track in playlist
        int nextIndex = currentPlaylistIndex + 1;
        if (nextIndex < currentPlaylist->size())
        {
            currentPlaylistIndex = nextIndex;
            currentMedia = currentPlaylist->getMediaFile(currentPlaylistIndex);

            // Start playing the new track
            if (currentMedia)
            {
                playCurrentMedia();
            }
        }
    }

    updatePlayerView();
}

void PlayerController::previous()
{
    std::lock_guard<std::mutex> lock(playbackMutex);

    if (currentPlaylist && currentPlaylistIndex > 0)
    {
        // Move to previous track in playlist
        currentPlaylistIndex--;
        currentMedia = currentPlaylist->getMediaFile(currentPlaylistIndex);

        // Start playing the new track
        if (currentMedia)
        {
            playCurrentMedia();
        }
    }

    updatePlayerView();
}

void PlayerController::setVolume(int vol)
{
    volume = std::clamp(vol, 0, SDL_MIX_MAXVOLUME);
    Mix_VolumeMusic(volume);
    updatePlayerView();
}

int PlayerController::getVolume() const
{
    return volume;
}

void PlayerController::volumeUp()
{
    setVolume(volume + SDL_MIX_MAXVOLUME / 10); // Increase by 10%
}

void PlayerController::volumeDown()
{
    setVolume(volume - SDL_MIX_MAXVOLUME / 10); // Decrease by 10%
}

void PlayerController::seek(int position)
{
    // SDL_mixer doesn't support direct seeking, so we'd need to use Mix_SetMusicPosition
    // which works for certain formats
    if (isPlaying && currentMedia)
    {
        // Clamp position to valid range
        position = std::clamp(position, 0, totalDuration);

        // Try to seek (may not
        // Clamp position to valid range
        position = std::clamp(position, 0, totalDuration);

        // Try to seek (may not work with all formats)
        if (Mix_SetMusicPosition(static_cast<double>(position)) == 0)
        {
            currentPosition = position;
            updatePlayerView();
        }
    }
}

void PlayerController::seekForward(int seconds)
{
    seek(currentPosition + seconds);
}

void PlayerController::seekBackward(int seconds)
{
    seek(currentPosition - seconds);
}

bool PlayerController::isMediaPlaying() const
{
    return isPlaying;
}

bool PlayerController::isMediaPaused() const
{
    return isPaused;
}

int PlayerController::getCurrentPosition() const
{
    return currentPosition;
}

int PlayerController::getDuration() const
{
    return totalDuration;
}

std::shared_ptr<MediaFileModel> PlayerController::getCurrentMedia() const
{
    return currentMedia;
}

void PlayerController::updatePlayerView()
{
    if (playerView && currentMedia)
    {
        playerView->setCurrentMedia(currentMedia.get());
        // Additional view updates would happen here
    }
}

bool PlayerController::loadMedia(std::shared_ptr<MediaFileModel> media)
{
    if (!media)
        return false;

    // Clean up previous music if any
    if (currentMusic)
    {
        Mix_FreeMusic(currentMusic);
        currentMusic = nullptr;
    }

    // Load the music file
    currentMusic = Mix_LoadMUS(media->getFilepath().c_str());
    if (!currentMusic)
    {
        return false;
    }

    // Set duration
    totalDuration = media->getDuration();
    currentPosition = 0;

    return true;
}

void PlayerController::playCurrentMedia()
{
    if (loadMedia(currentMedia))
    {
        // Start playback
        if (Mix_PlayMusic(currentMusic, 1) == 0)
        {
            isPlaying = true;
            isPaused = false;

            // Start the playback monitoring thread if not already running
            if (!threadRunning)
            {
                threadRunning = true;
                playbackThread = std::thread(&PlayerController::playbackMonitorThread, this);
            }
        }
        else
        {
            // Failed to play
            isPlaying = false;
        }
    }
}

void PlayerController::handlePlaybackFinished()
{
    // This is called when a track finishes playing

    // Try to play the next track if in a playlist
    if (currentPlaylist && currentPlaylistIndex >= 0)
    {
        int nextIndex = currentPlaylistIndex + 1;
        if (nextIndex < currentPlaylist->size())
        {
            // Move to next track
            currentPlaylistIndex = nextIndex;
            currentMedia = currentPlaylist->getMediaFile(currentPlaylistIndex);

            // Start playing the new track
            if (currentMedia)
            {
                playCurrentMedia();
            }
        }
        else
        {
            // End of playlist
            isPlaying = false;
            currentPosition = 0;
        }
    }
    else
    {
        // Single track playback ended
        isPlaying = false;
        currentPosition = 0;
    }

    updatePlayerView();
}

void PlayerController::playbackMonitorThread()
{
    using namespace std::chrono;

    while (threadRunning)
    {
        // Update current position every 500ms
        {
            std::unique_lock<std::mutex> lock(playbackMutex);

            if (isPlaying && !isPaused)
            {
                if (Mix_PlayingMusic())
                {
                    // Update current position
                    currentPosition++;

                    // Update view every second
                    if (currentPosition % 2 == 0)
                    {
                        updatePlayerView();
                    }
                }
                else
                {
                    // Music finished playing
                    handlePlaybackFinished();
                }
            }
        }

        // Sleep for 500ms
        std::this_thread::sleep_for(milliseconds(500));
    }
}

void PlayerController::musicFinishedCallback()
{
    // This is called by SDL_mixer when music finishes
    if (instance)
    {
        instance->handlePlaybackFinished();
    }
}

// USBController implementation
USBController::USBController(
    ViewManagerInterface *vm,
    std::shared_ptr<USBManager> usb,
    std::shared_ptr<MediaLibrary> library) : Controller(vm),
                                             usbManager(usb),
                                             mediaLibrary(library),
                                             threadRunning(false),
                                             usbView(nullptr)
{
}

USBController::~USBController()
{
    shutdown();
}

void USBController::initialize()
{
    // Start USB detection thread
    threadRunning = true;
    detectionThread = std::thread(&USBController::usbDetectionThread, this);

    // Initial detection
    detectUSBDevices();
}

void USBController::shutdown()
{
    // Stop thread
    if (threadRunning)
    {
        threadRunning = false;
        if (detectionThread.joinable())
        {
            detectionThread.join();
        }
    }

    // Unmount any mounted device
    if (!currentMountPoint.empty())
    {
        usbManager->unmountDevice(currentMountPoint);
        currentMountPoint.clear();
    }
}

void USBController::setUSBView(USBInterface *view)
{
    usbView = view;
}

void USBController::detectUSBDevices()
{
    std::lock_guard<std::mutex> lock(usbMutex);
    detectedDevices = usbManager->detectUSBDevices();
    updateUSBView();
}

bool USBController::mountUSB(const std::string &device)
{
    std::lock_guard<std::mutex> lock(usbMutex);

    // Unmount current device if any
    if (!currentMountPoint.empty())
    {
        usbManager->unmountDevice(currentMountPoint);
        currentMountPoint.clear();
    }

    // Mount the selected device
    bool result = usbManager->mountDevice(device);
    if (result)
    {
        currentMountPoint = usbManager->getMountPoint(device);
        // Scan for media files
        scanUSBContents(currentMountPoint);
    }

    updateUSBView();
    return result;
}

bool USBController::unmountUSB(const std::string &device)
{
    std::lock_guard<std::mutex> lock(usbMutex);

    bool result = usbManager->unmountDevice(device);
    if (result && device == currentMountPoint)
    {
        currentMountPoint.clear();
    }

    updateUSBView();
    return result;
}

void USBController::scanUSBContents(const std::string &mountPoint)
{
    if (!mountPoint.empty())
    {
        mediaLibrary->scanUSBDevice(mountPoint);
    }
}

void USBController::updateUSBView()
{
    if (usbView)
    {
        usbView->updateDeviceList(usbManager->detectUSBDevices());
    }
}

void USBController::usbDetectionThread()
{
    using namespace std::chrono;

    while (threadRunning)
    {
        // Check for USB changes every 2 seconds
        {
            std::unique_lock<std::mutex> lock(usbMutex);
            auto currentDevices = usbManager->detectUSBDevices();

            // Check if device list changed
            if (currentDevices != detectedDevices)
            {
                detectedDevices = currentDevices;
                updateUSBView();
            }
        }

        // Sleep for 2 seconds
        std::this_thread::sleep_for(seconds(2));
    }
}

// MetadataController implementation
MetadataController::MetadataController(
    ViewManagerInterface *vm,
    std::shared_ptr<MetadataManager> metadata) : Controller(vm),
                                                 metadataManager(metadata),
                                                 metadataView(nullptr)
{
}

void MetadataController::setMetadataView(MetadataInterface *view)
{
    metadataView = view;
}

void MetadataController::loadMetadata(std::shared_ptr<MediaFileModel> file)
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    currentMedia = file;

    if (currentMedia)
    {
        // Load metadata
        originalMetadata = currentMedia->getAllMetadata();
        editedMetadata = originalMetadata; // Make a copy for editing
    }
    else
    {
        originalMetadata.clear();
        editedMetadata.clear();
    }

    updateMetadataView();
}

bool MetadataController::saveMetadata()
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    if (!currentMedia)
        return false;

    // Apply edited metadata to the media file
    for (const auto &[key, value] : editedMetadata)
    {
        currentMedia->setMetadata(key, value);
    }

    // Save changes to file
    bool success = currentMedia->saveMetadata();

    if (success)
    {
        // Update original metadata with saved changes
        originalMetadata = editedMetadata;
    }

    updateMetadataView();
    return success;
}

void MetadataController::discardChanges()
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    // Restore original metadata
    editedMetadata = originalMetadata;
    updateMetadataView();
}

void MetadataController::updateField(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    // Update the field in edited metadata
    editedMetadata[key] = value;
}

void MetadataController::addNewField(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    if (!key.empty())
    {
        editedMetadata[key] = value;
    }
}

void MetadataController::removeField(const std::string &key)
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    auto it = editedMetadata.find(key);
    if (it != editedMetadata.end())
    {
        editedMetadata.erase(it);
    }
}

void MetadataController::updateMetadataView()
{
    if (metadataView && currentMedia)
    {
        metadataView->showMetadata(currentMedia.get());
    }
}

void MetadataController::enterEditMode()
{
    // This would set the view to edit mode
    if (metadataView)
    {
        // metadataView->enterEditMode();
    }
}

void MetadataController::exitEditMode()
{
    // This would exit edit mode in the view
    if (metadataView)
    {
        // metadataView->exitEditMode();
    }
}

std::shared_ptr<MediaFileModel> MetadataController::getCurrentMedia() const
{
    return currentMedia;
}

const std::map<std::string, std::string> &MetadataController::getMetadata() const
{
    return editedMetadata;
}
