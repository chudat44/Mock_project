#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "View/Interface/Iview.h"
#include "Model/playlist.h"
#include "Model/manager.h"
#include "hardware.h"

#include <unistd.h>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>


// Controller base class
class Controller {
protected:
    // Reference to the view manager to update UI
    ViewManagerInterface* viewManager;

public:
    Controller(ViewManagerInterface* vm) : viewManager(vm) {}
    virtual ~Controller() = default;
    
    void setViewManager(ViewManagerInterface* vm) {
        viewManager = vm;
    }
};

// Application main controller
class ApplicationController : public Controller {
private:
    // Model references
    std::shared_ptr<MediaLibrary> mediaLibrary;
    std::shared_ptr<PlaylistsManager> playlistsManager;
    std::shared_ptr<USBManager> usbManager;
    std::shared_ptr<MetadataManager> metadataManager;
    
    // Child controllers
    std::unique_ptr<class PlayerController> playerController;
    std::unique_ptr<class PlaylistController> playlistController;
    std::unique_ptr<class USBController> usbController;
    std::unique_ptr<class MetadataController> metadataController;
    
    // Current state
    std::string currentDirectory;
    bool applicationRunning;
    
    // Mutex for thread safety
    std::mutex controllerMutex;

public:
    ApplicationController(
        ViewManagerInterface* vm,
        std::shared_ptr<MediaLibrary> library,
        std::shared_ptr<PlaylistsManager> playlists,
        std::shared_ptr<USBManager> usb,
        std::shared_ptr<MetadataManager> metadata
    );
    
    ~ApplicationController();
    
    // Initialization
    void initialize();
    
    // Event handling
    void handleUserCommand(UserCommand cmd);
    void handleUserInput(const SDL_Event& event);
    
    // Navigation
    void navigateToMainMenu();
    void navigateToMediaList();
    void navigateToPlaylistsList();
    void navigateToUSBDevices();
    
    // Directory operations
    void loadDirectory(const std::string& path);
    void refreshCurrentDirectory();
    
    // Search functionality
    std::vector<std::shared_ptr<MediaFileModel>> searchMedia(const std::string& query);
    
    // Application flow
    void run();
    void exit();
    
    // Accessors
    PlayerController* getPlayerController() const;
    PlaylistController* getPlaylistController() const;
    USBController* getUSBController() const;
    MetadataController* getMetadataController() const;
    
    std::shared_ptr<MediaLibrary> getMediaLibrary() const;
    std::shared_ptr<PlaylistsManager> getPlaylistsManager() const;
};

// Controller for playlist operations
class PlaylistController : public Controller {
private:
    // Model references
    std::shared_ptr<PlaylistsManager> playlistsManager;
    std::shared_ptr<MediaLibrary> mediaLibrary;
    
    // Current state
    std::shared_ptr<PlaylistModel> currentPlaylist;
    
    // View interfaces
    PlaylistsListInterface* playlistsListView;
    PlaylistInterface* playlistView;
    
    // Mutex for thread safety
    std::mutex playlistMutex;

public:
    PlaylistController(
        ViewManagerInterface* vm,
        std::shared_ptr<PlaylistsManager> playlists,
        std::shared_ptr<MediaLibrary> library
    );
    
    // View setters
    void setPlaylistsListView(PlaylistsListInterface* view);
    void setPlaylistView(PlaylistInterface* view);
    
    // Playlist management
    void createPlaylist(const std::string& name);
    void deletePlaylist(const std::string& name);
    void deletePlaylist(int index);
    void renamePlaylist(const std::string& oldName, const std::string& newName);
    
    // Loading playlists
    void loadAllPlaylists();
    void loadPlaylist(const std::string& name);
    void loadPlaylist(int index);
    
    // Playlist content management
    void addToPlaylist(const std::string& playlistName, std::shared_ptr<MediaFileModel> file);
    void addToCurrentPlaylist(std::shared_ptr<MediaFileModel> file);
    void removeFromPlaylist(const std::string& playlistName, int index);
    void removeFromCurrentPlaylist(int index);
    
    // Playlist item ordering
    void moveItemUp(int index);
    void moveItemDown(int index);
    
    // View updates
    void updatePlaylistsListView();
    void updatePlaylistView();
    
    // Navigation
    void viewPlaylistContent(const std::string& name);
    
    // Persistence
    void saveAllPlaylists();
    void saveCurrentPlaylist();
    
    // Accessors
    std::shared_ptr<PlaylistModel> getCurrentPlaylist() const;
    std::vector<std::shared_ptr<PlaylistModel>> getAllPlaylists() const;
    
    // Directory scanning for media
    void scanDirectoryForMedia(const std::string& path);
};

// Controller for media playback
class PlayerController : public Controller {
private:
    // SDL Mixer variables
    int audioDeviceId;
    Mix_Music* currentMusic;
    
    // Playback state
    std::atomic<bool> isPlaying;
    std::atomic<bool> isPaused;
    std::atomic<int> volume;
    std::atomic<int> currentPosition;
    int totalDuration;
    
    // Current media
    std::shared_ptr<MediaFileModel> currentMedia;
    
    // Playlist context
    std::shared_ptr<PlaylistModel> currentPlaylist;
    int currentPlaylistIndex;
    
    // Thread for playback monitoring
    std::thread playbackThread;
    std::atomic<bool> threadRunning;
    std::mutex playbackMutex;
    std::condition_variable pauseCondition;
    
    // View interface
    PlayerInterface* playerView;
    
    // Callbacks
    static void musicFinishedCallback();
    static PlayerController* instance; // For callback access

public:
    PlayerController(ViewManagerInterface* vm);
    ~PlayerController();
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // View setter
    void setPlayerView(PlayerInterface* view);
    
    // Playback controls
    void play();
    void playMedia(std::shared_ptr<MediaFileModel> media);
    void playPlaylist(std::shared_ptr<PlaylistModel> playlist, int startIndex = 0);
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
    std::shared_ptr<MediaFileModel> getCurrentMedia() const;
    
    // View updates
    void updatePlayerView();
    
private:
    // Thread function for playback monitoring
    void playbackMonitorThread();
    
    // Load and prepare media for playback
    bool loadMedia(std::shared_ptr<MediaFileModel> media);
    
    // Internal playback control
    void playCurrentMedia();
    void handlePlaybackFinished();
};

// Controller for USB device operations
class USBController : public Controller {
private:
    // Model reference
    std::shared_ptr<USBManager> usbManager;
    std::shared_ptr<MediaLibrary> mediaLibrary;
    
    // Current state
    std::string currentMountPoint;
    std::vector<std::string> detectedDevices;
    
    // View interface
    USBInterface* usbView;
    
    // Mutex for thread safety
    std::mutex usbMutex;
    
    // Thread for USB detection
    std::thread detectionThread;
    std::atomic<bool> threadRunning;

public:
    USBController(
        ViewManagerInterface* vm,
        std::shared_ptr<USBManager> usb,
        std::shared_ptr<MediaLibrary> library
    );
    
    ~USBController();
    
    // Initialization
    void initialize();
    void shutdown();
    
    // View setter
    void setUSBView(USBInterface* view);
    
    // USB operations
    void detectUSBDevices();
    bool mountUSB(const std::string& device);
    bool unmountUSB(const std::string& device);
    
    // Media scanning
    void scanUSBContents(const std::string& mountPoint);
    
    // View updates
    void updateUSBView();
    
private:
    // Thread function for USB detection
    void usbDetectionThread();
};

// Controller for metadata operations
class MetadataController : public Controller {
private:
    // Model references
    std::shared_ptr<MetadataManager> metadataManager;
    
    // Current state
    std::shared_ptr<MediaFileModel> currentMedia;
    std::map<std::string, std::string> originalMetadata;
    std::map<std::string, std::string> editedMetadata;
    
    // View interface
    MetadataInterface* metadataView;
    
    // Mutex for thread safety
    std::mutex metadataMutex;

public:
    MetadataController(
        ViewManagerInterface* vm,
        std::shared_ptr<MetadataManager> metadata
    );
    
    // View setter
    void setMetadataView(MetadataInterface* view);
    
    // Metadata operations
    void loadMetadata(std::shared_ptr<MediaFileModel> file);
    bool saveMetadata();
    void discardChanges();
    
    // Field operations
    void updateField(const std::string& key, const std::string& value);
    void addNewField(const std::string& key, const std::string& value);
    void removeField(const std::string& key);
    
    // View updates
    void updateMetadataView();
    void enterEditMode();
    void exitEditMode();
    
    // Accessors
    std::shared_ptr<MediaFileModel> getCurrentMedia() const;
    const std::map<std::string, std::string>& getMetadata() const;
};
#endif