#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include "View/Interface/Iview.h"
#include "Model/playlist.h"
#include "Model/manager.h"
#include "hardware_driver.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
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

#endif