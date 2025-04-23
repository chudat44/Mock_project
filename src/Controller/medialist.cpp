#include "medialist.h"


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
