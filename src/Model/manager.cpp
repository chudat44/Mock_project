#include "manager.h"

#include <mutex>
#include <filesystem>
#include <exception>
#include <iostream>
#include <sys/stat.h>

namespace fs = std::filesystem;

// MetadataManager implementations
std::shared_ptr<MediaFileModel> MetadataManager::loadMetadata(const std::string &filepath)
{
    std::shared_ptr<MediaFileModel> file;
    std::string ext = filepath.substr(filepath.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "mp3" || ext == "wav" || ext == "ogg" || ext == "flac")
    {
        file = std::make_shared<AudioFileModel>(filepath);
    }
    else if (ext == "mp4" || ext == "avi" || ext == "mkv" || ext == "mov")
    {
        file = std::make_shared<VideoFileModel>(filepath);
    }
    else
    {
        file = std::make_shared<MediaFileModel>(filepath);
    }

    return file;
}

bool MetadataManager::saveMetadata(std::shared_ptr<MediaFileModel> file)
{
    return file->saveMetadata();
}

void MetadataManager::extractMetadata(std::shared_ptr<MediaFileModel> file)
{
    file->loadMetadata();
}

void PlaylistsManager::createPlaylist(const std::string &name)
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
        savePlaylistsToFile();
    }
}

void PlaylistsManager::deletePlaylist(std::shared_ptr<PlaylistModel> playlist)
{
    std::lock_guard<std::mutex> lock(playlistsMutex);

    auto it = std::find(playlists.begin(), playlists.end(), playlist);

    if (it != playlists.end())
    {
        playlists.erase(it);
        savePlaylistsToFile();
    }
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

std::vector<std::shared_ptr<PlaylistModel>> PlaylistsManager::getAllPlaylists() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(playlistsMutex));
    return playlists;
}

void PlaylistsManager::savePlaylistsToFile()
{
    std::ofstream file(PLAYLISTS_FILE, std::ios::binary);

    if (!file)
    {
        std::cerr << "Error opening playlists file for writing" << std::endl;
        return;
    }

    // Write number of playlists
    size_t numPlaylists = playlists.size();
    file.write(reinterpret_cast<const char *>(&numPlaylists), sizeof(numPlaylists));

    // Write each playlist
    for (const auto &playlist : playlists)
    {
        // Write playlist name
        std::string name = playlist->getPlaylistName();
        size_t nameLength = name.length();
        file.write(reinterpret_cast<const char *>(&nameLength), sizeof(nameLength));
        file.write(name.c_str(), nameLength);

        // Write number of files
        auto files = playlist->getAllMediaFiles();
        size_t numFiles = files.size();
        file.write(reinterpret_cast<const char *>(&numFiles), sizeof(numFiles));

        // Write each file path
        for (const auto &mfile : files)
        {
            std::string path = mfile->getFilepath();
            size_t pathLength = path.length();
            file.write(reinterpret_cast<const char *>(&pathLength), sizeof(pathLength));
            file.write(path.c_str(), pathLength);
        }
    }
}

// PlaylistsManager implementations
void PlaylistsManager::loadPlaylistsFromFile(MediaLibrary &library)
{
    std::ifstream file(PLAYLISTS_FILE, std::ios::binary);

    if (!file)
    {
        std::cerr << "Playlists file not found, creating new file" << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(playlistsMutex);
    playlists.clear();

    // Read number of playlists
    size_t numPlaylists;
    file.read(reinterpret_cast<char *>(&numPlaylists), sizeof(numPlaylists));

    // Read each playlist
    for (size_t i = 0; i < numPlaylists; ++i)
    {
        // Read playlist name
        size_t nameLength;
        file.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));

        std::string name(nameLength, ' ');
        file.read(&name[0], nameLength);

        auto playlist = std::make_shared<PlaylistModel>(name);

        // Read number of files
        size_t numFiles;
        file.read(reinterpret_cast<char *>(&numFiles), sizeof(numFiles));

        // Read each file path
        for (size_t j = 0; j < numFiles; ++j)
        {
            size_t pathLength;
            file.read(reinterpret_cast<char *>(&pathLength), sizeof(pathLength));

            std::string path(pathLength, ' ');
            file.read(&path[0], pathLength);

            // Try to find the file in the library
            auto mediaFile = library.getMediaByFilename(fs::path(path).filename().string());

            // If not found, create a new one
            if (!mediaFile)
            {
                // Check if file exists
                if (fs::exists(path))
                {
                    mediaFile = std::make_shared<MediaFileModel>(path);
                }
                else
                {
                    // Skip missing files
                    continue;
                }
            }

            playlist->addMediaFile(mediaFile);
        }

        playlists.push_back(playlist);
    }
}

// USBManager implementation
bool USBManager::isMountPoint(const std::string &path)
{
    struct stat buf;
    if (stat(path.c_str(), &buf) != 0)
    {
        return false;
    }
    return S_ISDIR(buf.st_mode);
}

std::vector<std::string> USBManager::detectUSBDevices()
{
    std::vector<std::string> devices;

    // In a real implementation, this would use Linux-specific APIs to detect USB devices
    // For simplicity, we'll check common USB mount points
    const std::vector<std::string> commonMountPoints = {
        "/media", "/mnt", "/run/media"};

    for (const auto &basePoint : commonMountPoints)
    {
        if (!fs::exists(basePoint))
            continue;

        try
        {
            for (const auto &entry : fs::directory_iterator(basePoint))
            {
                if (fs::is_directory(entry))
                {
                    devices.push_back(entry.path().string());
                }
            }
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "Error scanning mount points: " << e.what() << std::endl;
        }
    }

    return devices;
}

bool USBManager::mountDevice(const std::string &device)
{
    // In a real implementation, this would use Linux-specific APIs to mount devices
    // For simplicity we'll assume the device is already mounted
    if (std::find(mountedDevices.begin(), mountedDevices.end(), device) == mountedDevices.end())
    {
        mountedDevices.push_back(device);
        return true;
    }
    return false;
}

bool USBManager::unmountDevice(const std::string &device)
{
    // In a real implementation, this would use Linux-specific APIs to unmount devices
    auto it = std::find(mountedDevices.begin(), mountedDevices.end(), device);
    if (it != mountedDevices.end())
    {
        // Execute umount command
        std::string command = "umount " + device + " 2>/dev/null";
        int result = system(command.c_str());

        if (result == 0)
        {
            mountedDevices.erase(it);
            return true;
        }
    }
    return false;
}

std::string USBManager::getMountPoint(const std::string &device)
{
    // In a real implementation, this would query the system for the mount point
    return device;
}

// MediaLibrary implementation

bool MediaLibrary::isAudioFile(const std::string &path)
{
    std::string ext = path.substr(path.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return std::find(supportedAudioExtensions.begin(), supportedAudioExtensions.end(), ext) != supportedAudioExtensions.end();
}

bool MediaLibrary::isVideoFile(const std::string &path)
{
    std::string ext = path.substr(path.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return std::find(supportedVideoExtensions.begin(), supportedVideoExtensions.end(), ext) != supportedVideoExtensions.end();
}

void MediaLibrary::scanDirectory(const std::string &path)
{
    std::lock_guard<std::mutex> lock(libraryMutex);

    try
    {
        for (const auto &entry : fs::recursive_directory_iterator(path))
        {
            if (entry.is_regular_file())
            {
                std::string filepath = entry.path().string();

                if (isAudioFile(filepath))
                {
                    mediaFiles.push_back(std::make_shared<AudioFileModel>(filepath));
                }
                else if (isVideoFile(filepath))
                {
                    mediaFiles.push_back(std::make_shared<VideoFileModel>(filepath));
                }
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

void MediaLibrary::scanUSBDevice(const std::string &mountPoint)
{
    scanDirectory(mountPoint);
}

std::vector<std::shared_ptr<MediaFileModel>> MediaLibrary::getMediaFiles() const
{
    return mediaFiles;
}

std::vector<std::shared_ptr<MediaFileModel>> MediaLibrary::getPagedMediaFiles(size_t page, size_t itemsPerPage) const
{
    std::vector<std::shared_ptr<MediaFileModel>> result;

    size_t startIdx = page * itemsPerPage;
    size_t endIdx = std::min(startIdx + itemsPerPage, mediaFiles.size());

    if (startIdx < mediaFiles.size())
    {
        result.assign(mediaFiles.begin() + startIdx, mediaFiles.begin() + endIdx);
    }

    return result;
}

size_t MediaLibrary::getTotalPages(size_t itemsPerPage) const
{
    return (mediaFiles.size() + itemsPerPage - 1) / itemsPerPage;
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