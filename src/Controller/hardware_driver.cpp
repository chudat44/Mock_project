#include "hardware_driver.h"

#include <algorithm>
#include <mutex>
#include <filesystem>
#include <exception>
#include <iostream>
#include <sys/stat.h>

namespace fs = std::filesystem;

// Serial Port Reader implement
#ifdef _WIN32

#define UNICODE
#include <map>
#include <windows.h>
#include <dbt.h>
#include <devguid.h>
#include <setupapi.h>
#include <tchar.h>
#include <codecvt>

extern std::string wstring_to_utf8(const std::wstring &wstr);
extern std::wstring utf8_to_wstring(const std::string &str);


class SerialPortReaderImpl
{
private:
    HANDLE serialHandle;
    OVERLAPPED overlapped;

public:
    SerialPortReaderImpl() : serialHandle(INVALID_HANDLE_VALUE) {}
    bool start(const std::string &portName, int baudRate)
    {
        overlapped = {0};
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        // Open the serial port
        std::string fullPortName = "\\\\.\\" + portName;
        serialHandle = CreateFileA(
            fullPortName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,                    // No sharing
            NULL,                 // No security attributes
            OPEN_EXISTING,        // Must use OPEN_EXISTING
            FILE_FLAG_OVERLAPPED, // Overlapped I/O for async operations
            NULL                  // Must be NULL for comm devices
        );

        if (serialHandle == INVALID_HANDLE_VALUE)
        {
            std::cerr << "Error opening serial port: " << GetLastError() << std::endl;
            return false;
        }

        // Configure the serial port
        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

        if (!GetCommState(serialHandle, &dcbSerialParams))
        {
            std::cerr << "Error getting serial port state: " << GetLastError() << std::endl;
            CloseHandle(serialHandle);
            CloseHandle(overlapped.hEvent);
            serialHandle = INVALID_HANDLE_VALUE;
            return false;
        }

        // Set serial port parameters
        dcbSerialParams.BaudRate = baudRate;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;

        if (!SetCommState(serialHandle, &dcbSerialParams))
        {
            std::cerr << "Error setting serial port state: " << GetLastError() << std::endl;
            CloseHandle(serialHandle);
            CloseHandle(overlapped.hEvent);
            serialHandle = INVALID_HANDLE_VALUE;
            return false;
        }

        // Set timeouts
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        if (!SetCommTimeouts(serialHandle, &timeouts))
        {
            std::cerr << "Error setting timeouts: " << GetLastError() << std::endl;
            CloseHandle(serialHandle);
            CloseHandle(overlapped.hEvent);
            serialHandle = INVALID_HANDLE_VALUE;
            return false;
        }
        return true;
    }

    void stop()
    {
        if (serialHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(serialHandle);
            CloseHandle(overlapped.hEvent);
            serialHandle = INVALID_HANDLE_VALUE;
        }
    }

    unsigned long ReadFunc(std::vector<uint8_t> &buffer, const size_t bufferSize)
    {
        DWORD bytesRead = 0;

        // Set up the overlapped read operation
        if (!ReadFile(serialHandle, buffer.data(), bufferSize, &bytesRead, &overlapped))
        {
            if (GetLastError() != ERROR_IO_PENDING)
            {
                std::cerr << "Error reading from serial port: " << GetLastError() << std::endl;
                return 0;
            }

            // Wait for the read to complete or timeout
            DWORD result = WaitForSingleObject(overlapped.hEvent, 100);
            if (result == WAIT_OBJECT_0)
            {
                // Read completed
                if (!GetOverlappedResult(serialHandle, &overlapped, &bytesRead, FALSE))
                {
                    std::cerr << "Error getting overlapped result: " << GetLastError() << std::endl;
                    return 0;
                }
            }
            else if (result == WAIT_TIMEOUT)
            {
                // Timeout, check if we should continue running
                return bytesRead;
            }
            else
            {
                // Error waiting for event
                std::cerr << "Error waiting for read event: " << GetLastError() << std::endl;
                return 0;
            }
        }

        return bytesRead;
    }
};

#endif

SerialPortReader::SerialPortReader(const std::string &port, int baud)
    : portName(port),
      baudRate(baud),
      running(false)
{
    sprImpl = std::make_unique<SerialPortReaderImpl>();
}

SerialPortReader::~SerialPortReader()
{
    stop();
}

void SerialPortReader::setPort(const std::string &port)
{
    portName = port;
}

bool SerialPortReader::start(DataCallback callback)
{
    onDataReceived = callback;

    sprImpl->start(portName, baudRate);
    // Start the read thread
    running = true;
    readThread = std::thread(&SerialPortReader::ReadThreadFunc, this);

    std::cout << "Serial port " << portName << " opened successfully at " << baudRate << " baud" << std::endl;
    return true;
}

void SerialPortReader::stop()
{
    if (running)
    {
        running = false;
        if (readThread.joinable())
        {
            readThread.join();
        }
    }
    sprImpl->stop();
}

void SerialPortReader::ReadThreadFunc()
{
    const size_t bufferSize = 1024;
    std::vector<uint8_t> buffer(bufferSize);
    unsigned long bytesRead;

    while (running)
    {
        bytesRead = sprImpl->ReadFunc(buffer, bufferSize);
        // Process the data if we read anything
        if (bytesRead && onDataReceived)
        {
            onDataReceived(buffer, bytesRead);
        }
    }
}

// S32K port plug in detector

#ifdef _WIN32
class S32K144PortDriverImpl
{
private:
    HWND messageWindow;
    using DeviceChangeCallback = std::function<void(const std::string &, bool)>;
    DeviceChangeCallback implCallback;

    void ListSerialPorts(std::map<std::wstring, std::wstring> &newPorts) 
    {
        HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
        if (hDevInfo == INVALID_HANDLE_VALUE)
            return;

        SP_DEVINFO_DATA devInfo;
        devInfo.cbSize = sizeof(devInfo);
        for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfo); ++i)
        {
            TCHAR name[256];
            if (SetupDiGetDeviceRegistryProperty(hDevInfo, &devInfo, SPDRP_FRIENDLYNAME, NULL, (PBYTE)name, sizeof(name), NULL))
            {
                newPorts[name] = L"";

                HKEY hDeviceRegistryKey = SetupDiOpenDevRegKey(hDevInfo, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
                if (hDeviceRegistryKey != INVALID_HANDLE_VALUE)
                {
                    TCHAR portName[256];
                    DWORD size = sizeof(portName);
                    DWORD type = 0;
                    if (RegQueryValueEx(hDeviceRegistryKey, TEXT("PortName"), NULL, &type, (LPBYTE)portName, &size) == ERROR_SUCCESS)
                    {

                        newPorts[name] = portName;
                    }
                    RegCloseKey(hDeviceRegistryKey);
                }
            }
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);
    }


    // Static instance pointer for the window procedure
    static S32K144PortDriverImpl *instance;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_DEVICECHANGE && instance)
        {
            std::cout << "Device change message received\n";

            if (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE)
            {
                PDEV_BROADCAST_HDR broadcastHeader = (PDEV_BROADCAST_HDR)lParam;

                if (broadcastHeader && broadcastHeader->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                {
                    std::map<std::wstring, std::wstring> newPorts;
                    instance->ListSerialPorts(newPorts);

                    std::string portName;
                    bool isConnected = false;
                    for (const auto &[key,value] : newPorts){
                        if (key.find(L"OpenSDA") != key.npos){
                            isConnected = true;
                            portName = wstring_to_utf8(value);
                        }
                    }

                    // Call the callback with the port name and connection state
                    if (instance->implCallback)
                    {
                        instance->implCallback(portName, isConnected);
                    }
                }
            }
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    

public:
    S32K144PortDriverImpl() : messageWindow(NULL) {}
    void start(DeviceChangeCallback callback)
    {
        implCallback = callback;
        instance = this;

        const wchar_t CLASS_NAME[] = L"COMDetectWindowClass";
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = CLASS_NAME;
        RegisterClass(&wc);

        HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"COM Detector",
                                   0, 0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandle(NULL), NULL);

        // Subscribe to device notifications (for COM ports)
        GUID GUID_DEVINTERFACE_COMPORT =
            {0x86E0D1E0, 0x8089, 0x11D0, {0x9C, 0xE4, 0x08, 0x00, 0x3E, 0x30, 0x1F, 0x73}};

        DEV_BROADCAST_DEVICEINTERFACE NotificationFilter = {};
        NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
        NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_COMPORT;

        HDEVNOTIFY hDevNotify = RegisterDeviceNotification(
            hwnd,
            &NotificationFilter,
            DEVICE_NOTIFY_WINDOW_HANDLE);

        if (!hDevNotify)
        {
            std::cerr << "RegisterDeviceNotification failed.\n";
        }

        // Message loop
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void stop()
    {
        if (messageWindow)
        {
            // Send a quit message to stop the message loop
            PostMessage(messageWindow, WM_QUIT, 0, 0);
            DestroyWindow(messageWindow);
            messageWindow = NULL;
        }
    }
};

S32K144PortDriverImpl *S32K144PortDriverImpl::instance = nullptr;

#endif

S32K144PortDriver::S32K144PortDriver()
{
    spdImpl = std::make_unique<S32K144PortDriverImpl>();
}

S32K144PortDriver::~S32K144PortDriver()
{
    stop();
}

bool S32K144PortDriver::start(DeviceChangeCallback callback)
{
    // Start the read thread
    onDeviceChange = callback;
    running = true;
    detectThread = std::thread(&detectFunc, this);

    return true;
}

void S32K144PortDriver::stop()
{
    if (running)
    {
        running = false;
        if (detectThread.joinable())
        {
            detectThread.join();
        }
    }
    spdImpl->stop();
}
void S32K144PortDriver::detectFunc()
{
    spdImpl->start(onDeviceChange);
}

// USBPortDriver implementation
bool USBPortDriver::isMountPoint(std::filesystem::path &path)
{
    struct stat buf;
    if (stat(path.string().c_str(), &buf) != 0)
    {
        return false;
    }
    return S_ISDIR(buf.st_mode);
}

std::vector<std::filesystem::path> USBPortDriver::detectUSBDevices()
{
    std::vector<std::filesystem::path> devices;

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

bool USBPortDriver::mountDevice(std::filesystem::path &device)
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

bool USBPortDriver::unmountDevice(std::filesystem::path &device)
{
    // In a real implementation, this would use Linux-specific APIs to unmount devices
    auto it = std::find(mountedDevices.begin(), mountedDevices.end(), device);
    if (it != mountedDevices.end())
    {
        // Execute umount command
        std::string command = "umount " + device.string() + " 2>/dev/null";
        int result = system(command.c_str());

        if (result == 0)
        {
            mountedDevices.erase(it);
            return true;
        }
    }
    return false;
}

std::filesystem::path USBPortDriver::getMountPoint(std::filesystem::path &device)
{
    // In a real implementation, this would query the system for the mount point
    return device;
}