#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "View/Interface/Iview.h"
#include "Model/playlist.h"
#include "Model/manager.h"
#include "hardware_driver.h"

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

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

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

// Controller for playlist operations
class PlaylistsListController
{
private:
    // Model references
    std::unique_ptr<class PlaylistsManager> playlistsManager;

    // Current state
    int currentPlaylistIndex;

    // View interfaces
    PlaylistsListInterface *playlistsListView;

    // Mutex for thread safety
    std::mutex playlistMutex;

    // Callbacks
    std::function<void(std::shared_ptr<class PlaylistModel>)> onPlaylistSelectedCallback;
    std::function<void(std::shared_ptr<class PlaylistModel>)> onPlaylistPlayCallback;

public:
    PlaylistsListController(PlaylistsListInterface *plI);

    // View setters
    void setPlaylistsListView(PlaylistsListInterface *view);

    // Playlist management
    void createPlaylist(const std::string &name);
    void deletePlaylist(int index);
    void renamePlaylist(const std::string &oldName, const std::string &newName);

    // Playlist item ordering
    void moveItemUp(int index);
    void moveItemDown(int index);

    // View updates
    void updatePlaylistsListView();

    // Loading playlists
    void loadAllPlaylists();

    // Persistence
    void saveAllPlaylists();
    // void saveCurrentPlaylist();

    // Accessors
    std::vector<std::shared_ptr<class PlaylistModel>> getAllPlaylists() const;

    // Player Callback set
    void setOnPlaylistSelectedCallback(std::function<void(std::shared_ptr<class PlaylistModel>)> callback);
    void setOnPlaylistPlayCallback(std::function<void(std::shared_ptr<class PlaylistModel>)> callback);

    void handlePlaylistSelected(int index); 
    void handlePlaylistPlay(int index); 
};

class MediaListController
{
private:
    // Model references
    std::unique_ptr<class MediaLibrary> mediaLibrary;

    // Current state
    std::filesystem::path currentDirectory;
    std::shared_ptr<class PlaylistModel> currentPlaylist;
    int currentMediaIndex;
    
    // View interfaces
    MediaListInterface *mediaListView;

    // Mutex for thread safety
    std::mutex mediaListMutex;

    // Callbacks
    std::function<void(std::shared_ptr<class MediaFileModel>)> onMediaSelectedCallback;
    std::function<void(const std::vector<std::shared_ptr<class MediaFileModel>>&, int)> onMediaPlayCallback;
    std::function<std::shared_ptr<class PlaylistModel>(const std::string&)> onOtherPlaylistCallback;

public:
    MediaListController(MediaListInterface *mlI);

    void setMediaListView(MediaListInterface *view);

    // Playlist content management
    void addToPlaylist(const std::string &playlistName, std::shared_ptr<class MediaFileModel> file);
    void addToCurrentPlaylist(std::shared_ptr<class MediaFileModel> file);
    void removeFromPlaylist(const std::string &playlistName, int index);
    void removeFromCurrentPlaylist(int index);

    // Playlist item ordering
    void moveItemUp(int index);
    void moveItemDown(int index);

    void loadPlaylist(std::shared_ptr<class PlaylistModel> playlist);

    // View update    
    void updatePlaylistView();

    // Accessors
    std::shared_ptr<class PlaylistModel> getCurrentPlaylist() const;

    
    // Directory scanning for media
    void scanDirectoryForMedia(std::filesystem::path &path);

    // Player Callback set
    void setOnMediaSelectedCallback(std::function<void(std::shared_ptr<class MediaFileModel>)> callback);
    void setOnMediaPlayCallback(std::function<void(const std::vector<std::shared_ptr<class MediaFileModel>>&, int)> callback);
    void setOnOtherPlaylistCallback(std::function<std::shared_ptr<class PlaylistModel>(const std::string&)> callback);

    void handleMediaSelected(int index);
    void handleMediaPlay(int index);
};

// Controller for media playback
class PlayerController
{
private:
    // SDL Mixer variables
    int audioDeviceId;
    Mix_Music *currentMusic;

    // Playback state
    std::atomic<bool> isPlaying;
    std::atomic<bool> isPaused;
    std::atomic<int> volume;
    std::atomic<int> currentPosition;
    int totalDuration;

    // Current media
    std::shared_ptr<class MediaFileModel> currentMedia;

    // Playlist context
    std::vector<std::shared_ptr<class MediaFileModel>> currentPlaylist;
    int currentPlaylistIndex;

    // Thread for playback monitoring
    std::thread playbackThread;
    std::atomic<bool> threadRunning;
    std::mutex playbackMutex;
    std::condition_variable pauseCondition;

    // View interface
    PlayerInterface *playerView;

    // S32K144 controller
    S32K144PortDriver *boardDriver;
    SerialPortReader *serialReader;

    // Callbacks
    static void musicFinishedCallback();
    static PlayerController *instance; // For callback access

public:
    PlayerController(PlayerInterface *pm);
    ~PlayerController();

    // Initialization
    bool initialize();
    void shutdown();

    // View setter
    void setPlayerView(PlayerInterface *view);

    // Playback controls
    void play();
    void playMedia(std::shared_ptr<class MediaFileModel> media);
    void playPlaylist(const std::vector<std::shared_ptr<class MediaFileModel>> &playlist, int startIndex = 0);
    void pause();
    void stop();
    void next();
    void previous();

    // Volume control
    void setVolume(int vol);
    int getVolume() const;
    void volumeUp();
    void volumeDown();

    // Seeking
    void seek(int position);
    void seekForward(int seconds = 10);
    void seekBackward(int seconds = 10);

    // Status information
    bool isMediaPlaying() const;
    bool isMediaPaused() const;
    int getCurrentPosition() const;
    int getDuration() const;

private:
    // Thread function for playback monitoring
    void playbackMonitorThread();

    // Load and prepare media for playback
    bool loadMedia(std::shared_ptr<class MediaFileModel> media);

    // Internal playback control
    void playCurrentMedia();
    void handlePlaybackFinished();
};

// Controller for metadata operations
class MetadataController
{
private:
    // Model references
    std::shared_ptr<class MetadataManager> metadataManager;

    // Current state
    std::shared_ptr<class MediaFileModel> currentMedia;
    std::map<std::string, std::string> originalMetadata;
    std::map<std::string, std::string> editedMetadata;

    // View interface
    MetadataInterface *metadataView;

    // Mutex for thread safety
    std::mutex metadataMutex;

public:
    MetadataController( MetadataInterface *mV);

    // View setter
    void setMetadataView(MetadataInterface *view);

    // Metadata operations
    const std::map<std::string, std::string> &getMetadata() const;
    void preloadMetadata(std::vector<std::shared_ptr<MediaFileModel>> mediaFiles);
    void loadMetadata(std::shared_ptr<class MediaFileModel> file);
    bool saveMetadata();
    void discardChanges();

    // Field operations
    void updateField(const std::string &key, const std::string &value);
    void addNewField(const std::string &key, const std::string &value);
    void removeField(const std::string &key);

    // View updates
    void updateMetadataView();
    void enterEditMode();
    void exitEditMode();

    // Accessors
};
#endif