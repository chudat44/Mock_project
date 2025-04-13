#ifndef MANAGEMENT_CONTROLLER_H
#define MANAGEMENT_CONTROLLER_H

#include "playlist.h"

#include <mutex>
// #include <mntent.h>

class MetadataManager
{
public:
    std::shared_ptr<MediaFileModel> loadMetadata(const std::string &filepath);

    bool saveMetadata(std::shared_ptr<MediaFileModel> file);

    void extractMetadata(std::shared_ptr<MediaFileModel> file);
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

    bool isAudioFile(const std::string &path);

    bool isVideoFile(const std::string &path);

public:
    MediaLibrary() {}

    void scanDirectory(const std::string &path);

    void scanUSBDevice(const std::string &mountPoint);

    std::vector<std::shared_ptr<MediaFileModel>> getMediaFiles() const;

    std::vector<std::shared_ptr<MediaFileModel>> getPagedMediaFiles(size_t page, size_t itemsPerPage = 25) const;

    size_t getTotalPages(size_t itemsPerPage = 25) const;

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
    const std::string PLAYLISTS_FILE = "playlists.dat";

public:
    void createPlaylist(const std::string &name);

    void deletePlaylist(std::shared_ptr<PlaylistModel> playlist);

    std::shared_ptr<PlaylistModel> getPlaylist(const std::string &name);

    std::vector<std::shared_ptr<PlaylistModel>> getAllPlaylists() const;

    void savePlaylistsToFile();

    void loadPlaylistsFromFile(MediaLibrary &library);
};

class USBManager
{
private:
    std::vector<std::string> mountedDevices;

    bool isMountPoint(const std::string &path);

public:
    // std::vector<std::string> detectUSBDevices()
    // {
    //     std::vector<std::string> devices;
    //     FILE *mtab = setmntent("/proc/mounts", "r");
    //     if (mtab)
    //     {
    //         struct mntent *mnt;
    //         while ((mnt = getmntent(mtab)) != nullptr)
    //         {
    //             std::string fsname = mnt->mnt_fsname;
    //             std::string dir = mnt->mnt_dir;

    //             // Check if it's a USB device (simplified)
    //             if (fsname.find("/dev/sd") != std::string::npos && isMountPoint(dir))
    //             {
    //                 devices.push_back(dir);
    //             }
    //         }
    //         endmntent(mtab);
    //     }
    //     return devices;
    // }

    // bool mountDevice(const std::string &device)
    // {
    //     // In Linux, USB drives are often auto-mounted
    //     // This is a simplified implementation that assumes the device is already mounted
    //     if (std::find(mountedDevices.begin(), mountedDevices.end(), device) == mountedDevices.end())
    //     {
    //         mountedDevices.push_back(device);
    //         return true;
    //     }
    //     return false;
    // }

    // bool unmountDevice(const std::string &device)
    // {
    //     auto it = std::find(mountedDevices.begin(), mountedDevices.end(), device);
    //     if (it != mountedDevices.end())
    //     {
    //         mountedDevices.erase(it);
    //         // In a real application, you'd call the umount command
    //         // system(("umount " + device).c_str());
    //         return true;
    //     }
    //     return false;
    // }

    // std::string getMountPoint(const std::string &device)
    // {
    //     // For simplicity, return the device itself as mount point
    //     // In a real implementation, you'd parse /proc/mounts to find the actual mount point
    //     return device;
    // }

    // New ********************

    std::vector<std::string> detectUSBDevices();

    bool mountDevice(const std::string &device);

    bool unmountDevice(const std::string &device);

    std::string getMountPoint(const std::string &device);
};

#endif // MANAGEMENT_CONTROLLER_H