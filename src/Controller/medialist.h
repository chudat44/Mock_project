#ifndef MEDIA_LIST_CONTROLLER_H
#define MEDIA_LIST_CONTROLLER_H

#include "View/Interface/Iview.h"
#include "Model/playlist.h"
#include "Model/manager.h"

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


#endif