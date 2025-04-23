#ifndef PLAYLIST_CONTROLLER_H
#define PLAYLIST_CONTROLLER_H

#include "View/Interface/Iview.h"
#include "Model/playlist.h"
#include "Model/manager.h"

#include <nlohmann/json.hpp>
#include <iostream>

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


#endif