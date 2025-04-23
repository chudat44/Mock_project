#include "manager.h"

#include <mutex>
#include <filesystem>
#include <exception>
#include <iostream>
#include <sys/stat.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/mp4file.h>
#include <taglib/flacfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/wavfile.h>

#ifdef _WIN32
#include <windows.h>
#include <codecvt>

std::string wstring_to_utf8(const std::wstring &wstr)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(wstr);
}
std::wstring utf8_to_wstring(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(str);
}
#endif

using json = nlohmann::json;
namespace fs = std::filesystem;

// MetadataManager implementations
bool MetadataManager::loadMetadata(std::shared_ptr<MediaFileModel> mediaFile)
{
#ifdef _WIN32
    TagLib::FileRef f(utf8_to_wstring(mediaFile->getFilepath()).c_str());
#else
    TagLib::FileRef f(mediaFile->getFilepath().c_str());
#endif

    if (f.isNull())
        return false;

    if (f.tag())
    {
        TagLib::Tag *tag = f.tag();

        mediaFile->setMetadata("Title", tag->title().to8Bit(true));
        mediaFile->setMetadata("Artist", tag->artist().to8Bit(true));
        mediaFile->setMetadata("Album", tag->album().to8Bit(true));
        mediaFile->setMetadata("Comment", tag->comment().to8Bit(true));
        mediaFile->setMetadata("Genre", tag->genre().to8Bit(true));
        mediaFile->setMetadata("Year", std::to_string(tag->year()));
        mediaFile->setMetadata("Track", std::to_string(tag->track()));
    }

    if (f.audioProperties())
    {
        TagLib::AudioProperties *props = f.audioProperties();
        mediaFile->setDuration(props->lengthInSeconds());
        mediaFile->setMetadata("Bitrate", std::to_string(props->bitrate()) + " kbps");
        mediaFile->setMetadata("Channels", std::to_string(props->channels()));
        mediaFile->setMetadata("Sample Rate", std::to_string(props->sampleRate()) + " Hz");
    }

    return true;
}

bool MetadataManager::saveMetadata(std::shared_ptr<MediaFileModel> mediaFile)
{
#ifdef _WIN32
    TagLib::FileRef f(utf8_to_wstring(mediaFile->getFilepath()).c_str());
#else
    TagLib::FileRef f(mediaFile->getFilepath().c_str());
#endif

    if (f.isNull())
        return false;

    TagLib::Tag *tag = f.tag();
    if (!tag)
        return false;

    if (!mediaFile->getMetadata("Title").empty())
        tag->setTitle(TagLib::String(mediaFile->getMetadata("Title"), TagLib::String::UTF8));
    if (!mediaFile->getMetadata("Artist").empty())
        tag->setArtist(TagLib::String(mediaFile->getMetadata("Artist"), TagLib::String::UTF8));
    if (!mediaFile->getMetadata("Album").empty())
        tag->setAlbum(TagLib::String(mediaFile->getMetadata("Album"), TagLib::String::UTF8));
    if (!mediaFile->getMetadata("Comment").empty())
        tag->setComment(TagLib::String(mediaFile->getMetadata("Comment"), TagLib::String::UTF8));
    if (!mediaFile->getMetadata("Genre").empty())
        tag->setGenre(TagLib::String(mediaFile->getMetadata("Genre"), TagLib::String::UTF8));
    if (!mediaFile->getMetadata("Year").empty())
        tag->setYear(std::stoi(mediaFile->getMetadata("Year")));
    if (!mediaFile->getMetadata("Track").empty())
        tag->setTrack(std::stoi(mediaFile->getMetadata("Track")));

    return f.save();
}

bool PlaylistsManager::createPlaylist(const std::string &name)
{
    std::lock_guard<std::mutex> lock(playlistsMutex);

    // Check if playlist with same name already exists
    auto it = std::find_if(playlists.begin(), playlists.end(),
                           [&name](const std::shared_ptr<PlaylistModel> &p)
                           {
                               return p->getPlaylistName() == name;
                           });

    if (it == playlists.end())
    {
        playlists.push_back(std::make_shared<PlaylistModel>(name));
        return true;
    }
    return false;
}

bool PlaylistsManager::deletePlaylist(std::shared_ptr<PlaylistModel> playlist)
{
    std::lock_guard<std::mutex> lock(playlistsMutex);

    auto it = std::find(playlists.begin(), playlists.end(), playlist);

    if (it != playlists.end())
    {
        playlists.erase(it);
        return true;
    }
    return false;
}

std::shared_ptr<PlaylistModel> PlaylistsManager::getPlaylist(const std::string &name)
{
    std::lock_guard<std::mutex> lock(playlistsMutex);

    for (const auto &playlist : playlists)
    {
        if (playlist->getPlaylistName() == name)
        {
            return playlist;
        }
    }

    return nullptr;
}

std::shared_ptr<PlaylistModel> PlaylistsManager::getPlaylist(int index)
{
    std::lock_guard<std::mutex> lock(playlistsMutex);

    if (index < playlists.size() && index >= 0)
        return playlists[index];

    return nullptr;
}

std::vector<std::shared_ptr<PlaylistModel>> PlaylistsManager::getAllPlaylists() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(playlistsMutex));
    return playlists;
}

void PlaylistsManager::parsePlaylistToJson(json &js, std::shared_ptr<PlaylistModel> playlist)
{
    js["playlist_name"] = playlist->getPlaylistName();

    nlohmann::json mediaArray = nlohmann::json::array();
    for (const auto &media : playlist->getAllMediaFiles())
    {
        nlohmann::json mediaJson;
        mediaJson["filepath"] = media->getFilepath();

        // Handle metadata as additional keys
        nlohmann::json additionalKeys = nlohmann::json::array();
        for (const auto &[key, value] : media->getAllAddMetadata())
        {
            nlohmann::json metadataItem;
            metadataItem[key] = value;
            additionalKeys.push_back(metadataItem);
        }
        mediaJson["additional key"] = additionalKeys;

        mediaArray.push_back(mediaJson);
    }

    js["media"] = mediaArray;
}

// PlaylistsManager implementations
void PlaylistsManager::loadPlaylistFromJson(json &js)
{
    std::lock_guard<std::mutex> lock(playlistsMutex);
    auto playlist = std::make_shared<PlaylistModel>();
    // Set playlist name
    if (js.contains("playlist_name"))
    {
        playlist->setPlaylistName(js["playlist_name"].get<std::string>());
    }

    // Process media files
    if (js.contains("media") && js["media"].is_array())
    {
        for (const auto &mediaJson : js["media"])
        {

            // Set filepath
            if (mediaJson.contains("filepath"))
            {
                std::shared_ptr<MediaFileModel> media =
                    std::make_shared<MediaFileModel>(mediaJson["filepath"].get<std::string>());
                // Process additional keys (metadata)
                if (mediaJson.contains("additional key") && mediaJson["additional key"].is_array())
                {
                    for (const auto &additionalItem : mediaJson["additional key"])
                    {
                        for (auto it = additionalItem.begin(); it != additionalItem.end(); ++it)
                        {
                            media->setMetadata(it.key(), it.value().get<std::string>());
                        }
                    }
                }

                playlist->addMediaFile(media);
            }
        }
    }
}

// MediaLibrary implementation

bool MediaLibrary::isAudioFile(std::filesystem::path &path)
{
    std::string u8path = path.u8string();
    std::string ext = u8path.substr(u8path.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return std::find(supportedAudioExtensions.begin(), supportedAudioExtensions.end(), ext) != supportedAudioExtensions.end();
}

bool MediaLibrary::isVideoFile(std::filesystem::path &path)
{
    std::string u8path = path.u8string();
    std::string ext = u8path.substr(u8path.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return std::find(supportedVideoExtensions.begin(), supportedVideoExtensions.end(), ext) != supportedVideoExtensions.end();
}

void MediaLibrary::scanDirectory(fs::path &path)
{
    std::lock_guard<std::mutex> lock(libraryMutex);
    mediaFiles.clear();
    try
    {
        for (const auto &entry : fs::recursive_directory_iterator(path))
        {
            if (entry.is_regular_file())
            {
                fs::path filepath = entry.path();
#ifdef _WIN32
                if (isAudioFile(filepath))
                {
                    mediaFiles.push_back(std::make_shared<AudioFileModel>(wstring_to_utf8(filepath.wstring())));
                }
                else if (isVideoFile(filepath))
                {
                    mediaFiles.push_back(std::make_shared<VideoFileModel>(wstring_to_utf8(filepath.wstring())));
                }
#else
                if (isAudioFile(filepath))
                {
                    mediaFiles.push_back(std::make_shared<AudioFileModel>(filepath.string()));
                }
                else if (isVideoFile(filepath))
                {
                    mediaFiles.push_back(std::make_shared<VideoFileModel>(filepath.string()));
                }
#endif
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

void MediaLibrary::scanUSBDevice(fs::path &mountPoint)
{
    scanDirectory(mountPoint);
}

std::shared_ptr<MediaFileModel> MediaLibrary::getMediaFile(int index) const
{
    return mediaFiles[index];
}
std::vector<std::shared_ptr<MediaFileModel>> MediaLibrary::getMediaFiles() const
{
    return mediaFiles;
}

std::vector<std::shared_ptr<MediaFileModel>> MediaLibrary::searchMedia(const std::string &keyword) const
{
    std::vector<std::shared_ptr<MediaFileModel>> result;
    std::string lowerKeyword = keyword;
    std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);

    for (const auto &file : mediaFiles)
    {
        std::string filename = file->getFilename();
        std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

        if (filename.find(lowerKeyword) != std::string::npos)
        {
            result.push_back(file);
        }
        else
        {
            // Search in metadata
            auto metadata = file->getAllMetadata();
            for (const auto &[key, value] : metadata)
            {
                std::string lowerValue = value;
                std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);

                if (lowerValue.find(lowerKeyword) != std::string::npos)
                {
                    result.push_back(file);
                    break;
                }
            }
        }
    }

    return result;
}

std::shared_ptr<MediaFileModel> MediaLibrary::getMediaByFilename(const std::string &filename) const
{
    for (const auto &file : mediaFiles)
    {
        if (file->getFilename() == filename)
            return file;
    }
    return nullptr;
}

std::shared_ptr<MediaFileModel> MediaLibrary::getMediaByFilepath(const std::string &filepath) const
{
    for (const auto &file : mediaFiles)
    {
        if (file->getFilepath() == filepath)
            return file;
    }
    return nullptr;
}

void MediaLibrary::clear()
{
    std::lock_guard<std::mutex> lock(libraryMutex);
    mediaFiles.clear();
}