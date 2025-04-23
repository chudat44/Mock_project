#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <map>

class MediaFileModel;
class PlaylistModel;

// Forward declarations

// Media list view
class MediaListInterface
{
public:
    // virtual void setDirectory(const std::string &directory)=0;
    // virtual void setMediaFiles(const std::vector<MediaFileModel *> &files)=0;
    // virtual void initialize() = 0;
    // virtual void update() = 0;
    virtual void setCurrentPlaylist(const std::string &playlistName, const std::vector<std::string> &mediaFilesNames) = 0;
};

// Playlists List view
class PlaylistsListInterface
{
public:
    virtual void setPlaylists(const std::vector<std::string> &playlistNames) = 0;
    // virtual void initialize() = 0;
    // virtual void update() = 0;
};

// Playback view for media playing interface
class PlayerInterface
{
public:
    // virtual void initialize() = 0;
    virtual void setCurrentMedia(const std::string &trackName, const std::string &artist) = 0;
    virtual void update() = 0;
    virtual void updateVolume(int volume) = 0;
    virtual void updatePlaybackStatus(bool playing) = 0;
    virtual void updateProgress(int currentPosition, int duration) = 0;
};

// USB devices view
class USBInterface
{
public:
    // virtual void initialize() = 0;
    virtual void updateDeviceList(const std::vector<std::string> &devices) = 0;
    // virtual void update() = 0;
};

// Detail view for metadata
class MetadataInterface
{
public:
    // virtual void initialize() = 0;
    virtual void showMetadata(const std::map<std::string, std::string> &metadata) = 0;
    // virtual void saveMetadata() = 0;
    // virtual void update() = 0;
};

class ViewManagerInterface
{
public:
    virtual void handleEvents() = 0;
    virtual void update() = 0;

    virtual void render() = 0;

    virtual void run() = 0;
    virtual void showDialog(const std::string &message) = 0;

    virtual bool shouldExit() const = 0;
};

#endif // MEDIA_PLAYER_VIEW_H