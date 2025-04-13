#ifndef INTERFACE_H
#define INTERFACE_H

#include "Core/user_command.h"
#include "Core/view_state.h"

#include <string>
#include <vector>

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
    virtual void update() = 0;
};

// Playlists List view
class PlaylistsListInterface
{
public:
    virtual void setPlaylists(const std::vector<std::string> &playlistNames)=0;
    // virtual void initialize() = 0;
    virtual void update() = 0;
};

// PlaylistModel content view
class PlaylistInterface
{
public:
    virtual void setCurrentPlaylist(PlaylistModel *playlist)=0;
    // virtual void initialize() = 0;
    virtual void update() = 0;
};

// Playback view for media playing interface
class PlayerInterface
{
public:
    // virtual void initialize() = 0;
    virtual void setCurrentMedia(MediaFileModel *media) = 0;
    virtual void update() = 0;
};

// USB devices view
class USBInterface
{
public:
    // virtual void initialize() = 0;
    virtual void updateDeviceList(const std::vector<std::string> &devices) = 0;
    virtual void update() = 0;
};

// Detail view for metadata
class MetadataInterface
{
public:
    // virtual void initialize() = 0;
    virtual void showMetadata(MediaFileModel *media) = 0;
    // virtual void saveMetadata() = 0;
    virtual void update() = 0;
};

class ViewManagerInterface
{
public:
    virtual void changeState(ViewState newState) =0;

    virtual void handleEvents() =0;
    virtual void update() =0;

    virtual void render() =0;

    virtual void run() =0;
    virtual void showDialog(const std::string& message) = 0;
};


#endif // MEDIA_PLAYER_VIEW_H