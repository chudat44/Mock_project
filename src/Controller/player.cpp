#include "player.h"

#include <iostream>
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

                playerView->update();

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