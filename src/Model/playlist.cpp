#include "playlist.h"

#include <algorithm>

namespace fs = std::filesystem;

// PlaylistModel implementation
PlaylistModel::PlaylistModel() {}
PlaylistModel::PlaylistModel(const std::string &name) : name(name) {}

PlaylistModel::~PlaylistModel() { playlist.clear(); }

void PlaylistModel::addMediaFile(const std::string &folder_path)
{
    playlist.push_back(std::make_shared<MediaFileModel>(folder_path));
}
void PlaylistModel::addMediaFile(std::shared_ptr<MediaFileModel> file)
{
    playlist.push_back(file);
}
std::shared_ptr<MediaFileModel> PlaylistModel::getMediaFile(size_t index) const
{
    if (index < playlist.size())
        return playlist[index];
    return nullptr;
}
void PlaylistModel::removeMediaFile(size_t index)
{
    if (index < playlist.size())
    {
        playlist.erase(playlist.begin() + index);
    }
}
void PlaylistModel::removeMediaFile(const std::string &filepath)
{
    auto it = std::find_if(playlist.begin(), playlist.end(),
                           [&filepath](const std::shared_ptr<MediaFileModel> &file)
                           {
                               return file->getFilepath() == filepath;
                           });

    if (it != playlist.end())
        playlist.erase(it);
}
void PlaylistModel::clear() { playlist.clear(); }

void PlaylistModel::setPlaylistName(const std::string &name) { this->name = name; }
const std::string &PlaylistModel::getPlaylistName() const { return name; }

const std::vector<std::shared_ptr<MediaFileModel>> &PlaylistModel::getAllMediaFiles() const { return playlist; }

size_t PlaylistModel::size() const { return playlist.size(); }
