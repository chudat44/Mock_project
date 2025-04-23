#ifndef HARDWARE_DRIVER
#define HARDWARE_DRIVER

#include "Model/playlist.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>


class SerialPortReaderImpl;

class SerialPortReader
{
private:
    std::unique_ptr<SerialPortReaderImpl> sprImpl;

    std::atomic<bool> running;
    std::thread readThread;
    std::string portName;
    int baudRate;

    // Callback function type for received data
    using DataCallback = std::function<void(const std::vector<uint8_t> &, size_t)>;
    DataCallback onDataReceived;

public:
    SerialPortReader(const std::string &port, int baud);

    ~SerialPortReader();

    void setPort(const std::string &port);

    bool start(DataCallback callback);

    void stop();

private:
    void ReadThreadFunc();
};

class S32K144PortDriverImpl;
// Manager for S32K144 device
class S32K144PortDriver
{
private:
    std::unique_ptr<S32K144PortDriverImpl> spdImpl;

    std::atomic<bool> running;
    std::thread detectThread;

    // Callback function type for device change events
    using DeviceChangeCallback = std::function<void(const std::string &, bool)>;
    DeviceChangeCallback onDeviceChange;

public:
    S32K144PortDriver();

    ~S32K144PortDriver();

    bool start(DeviceChangeCallback callback);
    void stop();

private:
    void detectFunc();
};

// Controller for USB device operations
class USBPortDriver
{
private:
    std::vector<std::filesystem::path> mountedDevices;

    bool isMountPoint(std::filesystem::path &path);

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

    std::vector<std::filesystem::path> detectUSBDevices();

    bool mountDevice(std::filesystem::path &device);

    bool unmountDevice(std::filesystem::path &device);

    std::filesystem::path getMountPoint(std::filesystem::path &device);
};

#endif