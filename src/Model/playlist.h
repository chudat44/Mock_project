#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "media.h"
#include <vector>
#include <memory>

#include <fstream>
#include <filesystem>

// List of media files = Playlist
class PlaylistModel
{
private:
    std::vector<std::shared_ptr<MediaFileModel>> playlist;
    std::string name;

public:
    PlaylistModel();
    PlaylistModel(const std::string &name);
    virtual ~PlaylistModel();

    void addMediaFile(const std::string &folder_path);
    void addMediaFile(std::shared_ptr<MediaFileModel> file);

    std::shared_ptr<MediaFileModel> getMediaFile(size_t index) const;

    void removeMediaFile(size_t index);
    void removeMediaFile(const std::string &filepath);
    void clear();

    void setPlaylistName(const std::string &name);
    const std::string &getPlaylistName() const;

    const std::vector<std::shared_ptr<MediaFileModel>> &getAllMediaFiles() const;
    size_t size() const;
};

#endif // PLAYLIST_H