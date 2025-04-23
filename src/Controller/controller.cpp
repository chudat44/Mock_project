#include "controller.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

const std::string playlistsFilePath = "data/playlist/index.json";
const std::string scanDirFilePath = "data/scan_dir/dir.json";

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

// PlaylistsListController implementation
PlaylistsListController::PlaylistsListController(
    PlaylistsListInterface *plI) : playlistsListView(plI)
{
    playlistsManager = std::make_unique<PlaylistsManager>();
}

void PlaylistsListController::setPlaylistsListView(PlaylistsListInterface *view)
{
    playlistsListView = view;
}

void PlaylistsListController::createPlaylist(const std::string &name)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    playlistsManager->createPlaylist(name);
    updatePlaylistsListView();
}

void PlaylistsListController::deletePlaylist(int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    auto playlists = playlistsManager->getAllPlaylists();
    if (index >= 0 && index < playlists.size())
    {
        auto playlist = playlists[index];
        playlistsManager->deletePlaylist(playlist);
        if (index == currentPlaylistIndex)
        {
            currentPlaylistIndex = -1;
            onPlaylistSelectedCallback(std::make_shared<PlaylistModel>(nullptr));
        }
        updatePlaylistsListView();
    }
}

void PlaylistsListController::renamePlaylist(const std::string &oldName, const std::string &newName)
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

void PlaylistsListController::loadAllPlaylists()
{
    std::lock_guard<std::mutex> lock(playlistMutex);

    // Load playlist from file
    if (!fs::exists(playlistsFilePath))
    {
        std::cerr << "Playlists data File does not exist. No playlists found.\n";
        return;
    }

    std::ifstream file(playlistsFilePath);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file";
    }
    json indexJson;
    try
    {
        file >> indexJson;
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON parsing failed at byte " << e.byte << ": " << e.what() << "\n";
    }

    // Pass json object from file index
    for (const auto &item : indexJson)
    {
        std::string name = item["name"];
        std::string file = item["file"];

        std::ifstream in("data/playlists/" + file);
        if (!in.is_open())
            continue;

        json content;
        in >> content;

        playlistsManager->loadPlaylistFromJson(content);
    }

    updatePlaylistsListView();
}

void PlaylistsListController::moveItemUp(int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    if (index > 0 && index < playlistsManager->getAllPlaylists().size())
    {
        // This would swap the item at index with the one at index-1
        // Since this is not implemented in your model class, we'd add this functionality here
        auto playlists = playlistsManager->getAllPlaylists(); // Get a copy
        if (index > 0 && index < playlists.size())
        {
            std::swap(playlists[index], playlists[index - 1]);

            // // Recreate playlist with new order
            // auto name = currentPlaylist->getPlaylistName();
            // playlistsManager->deletePlaylist(currentPlaylist);
            // playlistsManager->createPlaylist(name);
            // currentPlaylist = playlistsManager->getPlaylist(name);

            // for (auto &file : files)
            // {
            //     currentPlaylist->addMediaFile(file);
            // }

            saveAllPlaylists();
            updatePlaylistsListView();
        }
    }
}

void PlaylistsListController::moveItemDown(int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    if (index > 0 && index < playlistsManager->getAllPlaylists().size())
    {
        // This would swap the item at index with the one at index-1
        // Since this is not implemented in your model class, we'd add this functionality here
        auto playlists = playlistsManager->getAllPlaylists(); // Get a copy
        if (index > 0 && index < playlists.size())
        {
            std::swap(playlists[index], playlists[index - 1]);

            // // Recreate playlist with new order
            // auto name = currentPlaylist->getPlaylistName();
            // playlistsManager->deletePlaylist(currentPlaylist);
            // playlistsManager->createPlaylist(name);
            // currentPlaylist = playlistsManager->getPlaylist(name);

            // for (auto &file : files)
            // {
            //     currentPlaylist->addMediaFile(file);
            // }

            saveAllPlaylists();
            updatePlaylistsListView();
        }
    }
}

void PlaylistsListController::updatePlaylistsListView()
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

void PlaylistsListController::saveAllPlaylists()
{
    json indexArray;
    for (auto playlist : playlistsManager->getAllPlaylists())
    {
        std::string name = playlist->getPlaylistName();
        std::string filename = name + ".json";

        indexArray.push_back({{"name", name},
                              {"file", filename}});

        std::ofstream out("data/playlists/" + filename);
        json content;
        playlistsManager->parsePlaylistToJson(content, playlist);
        out << content.dump(4);
    }

    std::ofstream indexOut("data/playlists/index.json");
    indexOut << indexArray.dump(4);
}

std::vector<std::shared_ptr<PlaylistModel>> PlaylistsListController::getAllPlaylists() const
{
    return playlistsManager->getAllPlaylists();
}

void PlaylistsListController::setOnPlaylistSelectedCallback(std::function<void(std::shared_ptr<PlaylistModel>)> callback)
{
    onPlaylistSelectedCallback = callback;
}
void PlaylistsListController::setOnPlaylistPlayCallback(std::function<void(std::shared_ptr<PlaylistModel>)> callback)
{
    onPlaylistPlayCallback = callback;
}

void PlaylistsListController::handlePlaylistSelected(int index)
{
    if (currentPlaylistIndex != index)
    {
        if (onPlaylistSelectedCallback)
        {
            onPlaylistSelectedCallback(playlistsManager->getPlaylist(index));
        }
    }
}
void PlaylistsListController::handlePlaylistPlay(int index)
{
    if (currentPlaylistIndex != index)
    {
        if (onPlaylistPlayCallback)
        {
            onPlaylistPlayCallback(playlistsManager->getPlaylist(index));
        }
    }
}

// MediaListController
MediaListController::MediaListController(
    MediaListInterface *mlI) : mediaListView(mlI)
{
    mediaLibrary = std::make_unique<MediaLibrary>();
}

void MediaListController::setMediaListView(MediaListInterface *view)
{
    mediaListView = view;
}
std::shared_ptr<PlaylistModel> MediaListController::getCurrentPlaylist() const
{
    return currentPlaylist;
}

void MediaListController::loadPlaylist(std::shared_ptr<PlaylistModel> playlist)
{
    std::lock_guard<std::mutex> lock(mediaListMutex);
    currentPlaylist = playlist;
    currentDirectory.clear();
    updatePlaylistView();
}

void MediaListController::addToPlaylist(const std::string &playlistName, std::shared_ptr<MediaFileModel> file)
{
    std::lock_guard<std::mutex> lock(mediaListMutex);
    auto playlist = onOtherPlaylistCallback(playlistName);
    if (playlist.get())
    {
        playlist->addMediaFile(file);
        if (currentPlaylist && currentPlaylist->getPlaylistName() == playlistName)
        {
            updatePlaylistView();
        }
    }
}

void MediaListController::addToCurrentPlaylist(std::shared_ptr<MediaFileModel> file)
{
    std::lock_guard<std::mutex> lock(mediaListMutex);
    if (currentPlaylist)
    {
        currentPlaylist->addMediaFile(file);
    }
}

void MediaListController::removeFromPlaylist(const std::string &playlistName, int index)
{
    std::lock_guard<std::mutex> lock(mediaListMutex);
    auto playlist = onOtherPlaylistCallback(playlistName);
    if (playlist && index >= 0 && index < playlist->size())
    {
        playlist->removeMediaFile(index);
        if (currentPlaylist && currentPlaylist->getPlaylistName() == playlistName)
        {
            updatePlaylistView();
        }
    }
}

void MediaListController::removeFromCurrentPlaylist(int index)
{
    std::lock_guard<std::mutex> lock(mediaListMutex);
    if (currentPlaylist && index >= 0 && index < currentPlaylist->size())
    {
        currentPlaylist->removeMediaFile(index);
        updatePlaylistView();
    }
}

void MediaListController::updatePlaylistView()
{
    if (mediaListView && currentPlaylist)
    {
        std::vector<std::string> mediaFilesNames;
        for (auto &media : currentPlaylist->getAllMediaFiles())
        {
            mediaFilesNames.push_back(media->getFilename());
        }
        mediaListView->setCurrentPlaylist(currentPlaylist->getPlaylistName(), mediaFilesNames);
    }
}

void MediaListController::scanDirectoryForMedia(std::filesystem::path &path)
{
    mediaLibrary->scanDirectory(path);
    currentDirectory = path;
    std::vector<std::string> mediaFilesName;
    for (auto media : mediaLibrary->getMediaFiles())
    {
        mediaFilesName.push_back(media->getFilename());
    }
    mediaListView->setCurrentPlaylist(path.u8string(), mediaFilesName);
}

void MediaListController::setOnMediaSelectedCallback(std::function<void(std::shared_ptr<MediaFileModel>)> callback)
{
    onMediaSelectedCallback = callback;
}

void MediaListController::setOnMediaPlayCallback(std::function<void(const std::vector<std::shared_ptr<MediaFileModel>> &, int)> callback)
{
    onMediaPlayCallback = callback;
}

void MediaListController::setOnOtherPlaylistCallback(std::function<std::shared_ptr<class PlaylistModel>(const std::string &)> callback)
{
    onOtherPlaylistCallback = callback;
}

void MediaListController::handleMediaSelected(int index)
{
    if (currentMediaIndex != index)
    {
        if (onMediaSelectedCallback)
        {
            if (currentDirectory.empty())
                onMediaSelectedCallback(currentPlaylist->getMediaFile(index));
            onMediaSelectedCallback(mediaLibrary->getMediaFile(index));
        }
    }
}

void MediaListController::handleMediaPlay(int index)
{
    if (currentMediaIndex != index)
    {
        if (onMediaPlayCallback)
        {
            if (currentDirectory.empty())
                onMediaPlayCallback(currentPlaylist->getAllMediaFiles(), index);
            onMediaPlayCallback(mediaLibrary->getMediaFiles(), index);
        }
    }
}
// Static member for callback
PlayerController *PlayerController::instance = nullptr;

// PlayerController implementation
PlayerController::PlayerController(PlayerInterface *pm)
    : currentMusic(nullptr),
      isPlaying(false),
      isPaused(false),
      volume(SDL_MIX_MAXVOLUME / 2), // 50% volume
      currentPosition(0),
      totalDuration(0),
      currentPlaylistIndex(-1),
      threadRunning(false),
      playerView(pm)
{
    // Set static instance for callback
    instance = this;
    boardDriver = new S32K144PortDriver();
    serialReader = nullptr;
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

    boardDriver->start(
        [this](const std::string &portName, bool isConnected)
        {
            if (isConnected)
            {
                serialReader = new SerialPortReader(portName, 9600);
                serialReader->start(
                    [this](const std::vector<uint8_t> &buffer, size_t bytesRead)
                    {
                        for (size_t i = 0; i < bytesRead; ++i)
                        {
                            std::cout << (unsigned)buffer[i] << " ";
                            switch (buffer[i])
                            {
                            case 200:
                                if (this->isPaused)
                                    play();
                                else
                                    pause();
                                break;

                            case 201:
                                stop();
                                break;
                            case 202:
                                next();
                                break;
                            case 203:
                                previous();
                                break;
                            default:
                                setVolume(buffer[i]);
                                break;
                            }
                        }
                        std::cout << "read: " << bytesRead << '\n';
                    });
            }
            else
            {
                if (serialReader)
                {
                    serialReader->stop();
                    delete serialReader;
                    serialReader = nullptr;
                }
            }
        });

    return true;
}

void PlayerController::shutdown()
{
    // Stop playback and close audio
    Mix_HookMusicFinished(nullptr);
    Mix_HaltMusic();

    // Free music resources
    if (currentMusic)
    {
        Mix_FreeMusic(currentMusic);
        currentMusic = nullptr;
    }

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
    else if (currentPlaylist.size() > 0)
    {
        // Play first track of playlist if no current media
        currentPlaylistIndex = 0;
        auto media = currentPlaylist[0];
        if (media)
        {
            currentMedia = media;
            playCurrentMedia();
        }
    }

    playerView->updatePlaybackStatus(true);
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
    currentPlaylist.clear(); // Clear playlist context
    currentPlaylistIndex = -1;

    playCurrentMedia();
}

void PlayerController::playPlaylist(const std::vector<std::shared_ptr<MediaFileModel>> &playlist, int startIndex)
{
    if (playlist.empty())
        return;

    std::lock_guard<std::mutex> lock(playbackMutex);

    // Stop current playback
    if (isPlaying)
    {
        Mix_HaltMusic();
    }

    currentPlaylist = playlist;
    currentPlaylistIndex = (startIndex >= 0 && startIndex < playlist.size()) ? startIndex : 0;

    currentMedia = playlist[currentPlaylistIndex];
    if (currentMedia)
    {
        playCurrentMedia();
    }
}

void PlayerController::pause()
{
    std::lock_guard<std::mutex> lock(playbackMutex);

    if (isPlaying && !isPaused)
    {
        Mix_PauseMusic();
        isPaused = true;
    }
    else if (isPlaying && isPaused)
    {
        Mix_ResumeMusic();
        isPaused = false;
    }
    playerView->updatePlaybackStatus(false);
}

void PlayerController::stop()
{
    std::lock_guard<std::mutex> lock(playbackMutex);

    if (isPlaying)
    {
        isPlaying = false;
        isPaused = false;
        currentPosition = 0;
        currentMedia = nullptr;
        Mix_HaltMusic();

        // Free music resources
        if (currentMusic)
        {
            Mix_FreeMusic(currentMusic);
            currentMusic = nullptr;
        }
        if (playerView)
        {
            playerView->setCurrentMedia("", "");
            playerView->updateProgress(0, 0);
            playerView->updatePlaybackStatus(false);
        }
    }
}

void PlayerController::next()
{
    std::lock_guard<std::mutex> lock(playbackMutex);

    if (!currentPlaylist.empty() && currentPlaylistIndex >= 0)
    {
        // Move to next track in playlist
        int nextIndex = currentPlaylistIndex + 1;
        if (nextIndex < currentPlaylist.size())
        {
            currentPlaylistIndex = nextIndex;
            currentMedia = currentPlaylist[currentPlaylistIndex];

            // Start playing the new track
            if (currentMedia)
            {
                playCurrentMedia();
            }
        }
    }
}

void PlayerController::previous()
{
    std::lock_guard<std::mutex> lock(playbackMutex);

    if (!currentPlaylist.empty() && currentPlaylistIndex > 0)
    {
        // Move to previous track in playlist
        currentPlaylistIndex--;
        currentMedia = currentPlaylist[currentPlaylistIndex];

        // Start playing the new track
        if (currentMedia)
        {
            playCurrentMedia();
        }
    }
}

void PlayerController::setVolume(int vol)
{
    volume = std::clamp(vol, 0, SDL_MIX_MAXVOLUME);
    Mix_VolumeMusic(volume);
    playerView->updateVolume(volume);
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
            currentPosition = position * 2;
            playerView->updateProgress(position, totalDuration);
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
    return currentPosition / 2;
}

int PlayerController::getDuration() const
{
    return totalDuration;
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
                currentPosition = 0;
                playbackThread = std::thread(&PlayerController::playbackMonitorThread, this);
            }

            if (playerView)
            {
                if ((currentMedia->getMetadata("Title").empty()) || (currentMedia->getMetadata("Artist").empty()))
                    playerView->setCurrentMedia(currentMedia->getFilename(), "");

                else
                    playerView->setCurrentMedia(currentMedia->getMetadata("Title"), currentMedia->getMetadata("Artist"));

                playerView->updatePlaybackStatus(true);
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
    if (!currentPlaylist.empty() && currentPlaylistIndex >= 0)
    {
        int nextIndex = currentPlaylistIndex + 1;
        if (nextIndex < currentPlaylist.size())
        {
            // Move to next track
            currentPlaylistIndex = nextIndex;
            currentMedia = currentPlaylist[currentPlaylistIndex];

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
                        playerView->updateProgress(currentPosition / 2, totalDuration);
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
    if (instance->isMediaPlaying() && instance)
    {
        instance->handlePlaybackFinished();
    }
}

// MetadataController implementation
MetadataController::MetadataController(
    MetadataInterface *mV) : metadataView(mV)
{
    metadataManager = std::make_shared<MetadataManager>();
}

void MetadataController::setMetadataView(MetadataInterface *view)
{
    metadataView = view;
}

void MetadataController::preloadMetadata(std::vector<std::shared_ptr<MediaFileModel>> mediaFiles)
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    for (auto &file : mediaFiles)
    {
        metadataManager->loadMetadata(file);
    }
}

void MetadataController::loadMetadata(std::shared_ptr<MediaFileModel> file)
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    metadataManager->loadMetadata(file);
    currentMedia = file;
    // Load metadata
    originalMetadata = currentMedia->getAllMetadata();
    editedMetadata = originalMetadata; // Make a copy for editing

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
    bool success = metadataManager->saveMetadata(currentMedia);

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
    if (metadataView && currentMedia.get())
    {
        metadataView->showMetadata(currentMedia->getAllMetadata());
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

const std::map<std::string, std::string> &MetadataController::getMetadata() const
{
    return editedMetadata;
}
