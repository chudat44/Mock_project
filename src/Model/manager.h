#ifndef MANAGEMENT_CONTROLLER_H
#define MANAGEMENT_CONTROLLER_H

#include "playlist.h"

#include <mutex>
#include <nlohmann/json.hpp>

class MetadataManager
{
public:

    bool loadMetadata(std::shared_ptr<MediaFileModel> mediaFile);

    bool saveMetadata(std::shared_ptr<MediaFileModel> mediaFile);
};

class MediaLibrary
{
private:
    std::vector<std::shared_ptr<MediaFileModel>> mediaFiles;
    std::mutex libraryMutex;

    std::vector<std::string> supportedAudioExtensions = {
        "mp3", "wav", "ogg", "flac"};

    std::vector<std::string> supportedVideoExtensions = {
        "mp4", "avi", "mkv", "mov"};

    bool isAudioFile(std::filesystem::path &path);

    bool isVideoFile(std::filesystem::path &path);

public:
    MediaLibrary() {}

    void scanDirectory(std::filesystem::path &path);

    void scanUSBDevice(std::filesystem::path &mountPoint);

    std::shared_ptr<MediaFileModel> getMediaFile(int index) const;
    std::vector<std::shared_ptr<MediaFileModel>> getMediaFiles() const;

    std::vector<std::shared_ptr<MediaFileModel>> searchMedia(const std::string &keyword) const;

    std::shared_ptr<MediaFileModel> getMediaByFilename(const std::string &filename) const;

    std::shared_ptr<MediaFileModel> getMediaByFilepath(const std::string &filepath) const;

    void clear();
};

class PlaylistsManager
{
private:
    std::vector<std::shared_ptr<PlaylistModel>> playlists;
    std::mutex playlistsMutex;

public:
    bool createPlaylist(const std::string &name);

    bool deletePlaylist(std::shared_ptr<PlaylistModel> playlist);

    std::shared_ptr<PlaylistModel> getPlaylist(const std::string &name);
    std::shared_ptr<PlaylistModel> getPlaylist(int index);

    std::vector<std::shared_ptr<PlaylistModel>> getAllPlaylists() const;

    void parsePlaylistToJson(nlohmann::json &js, std::shared_ptr<PlaylistModel> playlist);

    void loadPlaylistFromJson(nlohmann::json &js);
};

#endif // MANAGEMENT_CONTROLLER_H