#include "hardware.h"

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Implementation of HardwareController

HardwareController::HardwareController()
    : adcValue(0), isRunning(false), serialFd(-1), devicePath(""), baudRate(115200)
{
}

HardwareController::~HardwareController()
{
    stopPolling();
    closeSerialPort();
}

bool HardwareController::initializeHardware(const std::string& device, int baud)
{
    devicePath = device;
    baudRate = baud;
    
    // Try to open serial port
    if (!openSerialPort()) {
        std::cerr << "Failed to open serial port: " << devicePath << std::endl;
        return false;
    }
    
    // Send initialization command to S32K144
    int result = sendCommand("INIT");
    if (result < 0) {
        std::cerr << "Failed to send initialization command" << std::endl;
        closeSerialPort();
        return false;
    }
    
    // Read initialization response
    std::string response = readResponse();
    if (response != "OK") {
        std::cerr << "Invalid initialization response: " << response << std::endl;
        closeSerialPort();
        return false;
    }
    
    std::cout << "Hardware controller initialized successfully" << std::endl;
    return true;
}

int HardwareController::readVolumeADC()
{
    std::lock_guard<std::mutex> lock(mtx);
    
    // Send ADC read command
    sendCommand("READ_ADC");
    
    // Parse response
    std::string response = readResponse();
    try {
        adcValue = std::stoi(response);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse ADC value: " << response << std::endl;
        return -1;
    }
    
    return adcValue;
}

HardwareControlEvent HardwareController::readButtonPress()
{
    std::lock_guard<std::mutex> lock(mtx);
    
    // Send button read command
    sendCommand("READ_BUTTONS");
    
    // Parse response
    std::string response = readResponse();
    
    if (response == "PLAY_PAUSE") {
        return HardwareControlEvent::BUTTON_PLAY_PAUSE;
    } else if (response == "STOP") {
        return HardwareControlEvent::BUTTON_STOP;
    } else if (response == "NEXT") {
        return HardwareControlEvent::BUTTON_NEXT;
    } else if (response == "PREV") {
        return HardwareControlEvent::BUTTON_PREVIOUS;
    } else if (response == "VOL_UP") {
        return HardwareControlEvent::BUTTON_VOLUME_UP;
    } else if (response == "VOL_DOWN") {
        return HardwareControlEvent::BUTTON_VOLUME_DOWN;
    } else {
        return HardwareControlEvent::NONE;
    }
}

void HardwareController::displayOnScreen(const std::string& info)
{
    std::lock_guard<std::mutex> lock(mtx);
    
    // Format command for display
    std::string cmd = "DISPLAY:" + info;
    sendCommand(cmd);
    
    // Check response (optional)
    std::string response = readResponse();
    if (response != "OK") {
        std::cerr << "Display command failed: " << response << std::endl;
    }
}

void HardwareController::startPolling(std::function<void(HardwareControlEvent)> callback)
{
    if (isRunning) {
        stopPolling();
    }
    
    eventCallback = callback;
    isRunning = true;
    
    // Start polling thread
    pollingThread = std::thread(&HardwareController::pollHardware, this);
}

void HardwareController::stopPolling()
{
    isRunning = false;
    
    if (pollingThread.joinable()) {
        pollingThread.join();
    }
}

void HardwareController::pollHardware()
{
    const int POLL_INTERVAL_MS = 50; // Poll every 50ms
    int lastAdcValue = -1;
    
    while (isRunning) {
        // Check for button presses
        HardwareControlEvent buttonEvent = readButtonPress();
        if (buttonEvent != HardwareControlEvent::NONE && eventCallback) {
            eventCallback(buttonEvent);
        }
        
        // Check for ADC changes
        int currentAdc = readVolumeADC();
        if (currentAdc >= 0 && std::abs(currentAdc - lastAdcValue) > 10) {
            // Only report significant changes in ADC value
            lastAdcValue = currentAdc;
            if (eventCallback) {
                eventCallback(HardwareControlEvent::ADC_VOLUME_CHANGE);
            }
        }
        
        // Sleep to prevent tight polling loop
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
    }
}

bool HardwareController::openSerialPort()
{
    // This is a simplified implementation
    // In a real application, you would use system calls to open and configure
    // the serial port (e.g., open, ioctl with termios on Linux)
    
    std::cout << "Opening serial port: " << devicePath << " at " << baudRate << " baud" << std::endl;
    
    // Simulate successful port opening
    serialFd = 42; // Dummy value
    
    return serialFd != -1;
}

void HardwareController::closeSerialPort()
{
    if (serialFd != -1) {
        // In a real application, you would close the file descriptor
        // close(serialFd);
        serialFd = -1;
        std::cout << "Serial port closed" << std::endl;
    }
}

int HardwareController::sendCommand(const std::string& cmd)
{
    if (serialFd == -1) {
        return -1;
    }
    
    // In a real application, you would use write() to send data to the serial port
    std::cout << "Sending command: " << cmd << std::endl;
    
    return cmd.length(); // Simulated bytes written
}

std::string HardwareController::readResponse()
{
    if (serialFd == -1) {
        return "";
    }
    
    // In a real application, you would use read() to receive data from the serial port
    // with appropriate timeout handling
    
    // Simulated responses for testing
    if (serialFd != -1) {
        return "OK";
    }
    
    return "";
}

int HardwareController::mapADCToVolume(int adcValue)
{
    // Map ADC value (typically 0-1023 for 10-bit ADC) to volume percentage (0-100)
    if (adcValue < 0) {
        return 0;
    }
    
    // Assuming 10-bit ADC (0-1023)
    return (adcValue * 100) / 1023;
}

// Implementation of HardwareEventAdapter

HardwareEventAdapter::HardwareEventAdapter(HardwareController* controller)
    : hardwareController(controller)
{
}

HardwareEventAdapter::~HardwareEventAdapter()
{
    stop();
}

void HardwareEventAdapter::setCommandCallback(std::function<void(UserCommand)> callback)
{
    commandCallback = callback;
}

void HardwareEventAdapter::start()
{
    if (hardwareController) {
        hardwareController->startPolling([this](HardwareControlEvent event) {
            this->onHardwareEvent(event);
        });
    }
}

void HardwareEventAdapter::stop()
{
    if (hardwareController) {
        hardwareController->stopPolling();
    }
}

void HardwareEventAdapter::onHardwareEvent(HardwareControlEvent event)
{
    if (commandCallback) {
        UserCommand cmd = mapEventToCommand(event);
        if (cmd != UserCommand::NONE) {
            commandCallback(cmd);
        }
    }
    
    // Handle ADC volume change specially
    if (event == HardwareControlEvent::ADC_VOLUME_CHANGE && hardwareController) {
        int adcValue = hardwareController->readVolumeADC();
        int volumePercentage = hardwareController->mapADCToVolume(adcValue);
        
        // Update volume display on hardware
        setHardwareVolume(volumePercentage);
        
        // Pass volume change command to application
        if (commandCallback) {
            // We use VOLUME_UP as a generic volume change event
            // The actual volume value will be read by the receiver
            commandCallback(UserCommand::VOLUME_UP);
        }
    }
}

UserCommand HardwareEventAdapter::mapEventToCommand(HardwareControlEvent event)
{
    switch (event) {
        case HardwareControlEvent::BUTTON_PLAY_PAUSE:
            return UserCommand::PLAY; // Application will toggle between play/pause
        
        case HardwareControlEvent::BUTTON_STOP:
            return UserCommand::STOP;
            
        case HardwareControlEvent::BUTTON_NEXT:
            return UserCommand::NEXT;
            
        case HardwareControlEvent::BUTTON_PREVIOUS:
            return UserCommand::PREVIOUS;
            
        case HardwareControlEvent::BUTTON_VOLUME_UP:
            return UserCommand::VOLUME_UP;
            
        case HardwareControlEvent::BUTTON_VOLUME_DOWN:
            return UserCommand::VOLUME_DOWN;
            
        case HardwareControlEvent::ADC_VOLUME_CHANGE:
            // Handled specially in onHardwareEvent
            return UserCommand::NONE;
            
        default:
            return UserCommand::NONE;
    }
}

void HardwareEventAdapter::updateHardwareDisplay(const std::string& mediaName, int position, int duration, bool playing)
{
    if (!hardwareController) {
        return;
    }
    
    // Format display string
    int minutes = position / 60;
    int seconds = position % 60;
    int totalMinutes = duration / 60;
    int totalSeconds = duration % 60;
    
    std::string statusChar = playing ? ">" : "||";
    std::string timeInfo = std::to_string(minutes) + ":" + 
                          (seconds < 10 ? "0" : "") + std::to_string(seconds) + " / " +
                          std::to_string(totalMinutes) + ":" + 
                          (totalSeconds < 10 ? "0" : "") + std::to_string(totalSeconds);
    
    std::string displayText = statusChar + " " + mediaName + "\n" + timeInfo;
    
    hardwareController->displayOnScreen(displayText);
}

void HardwareEventAdapter::setHardwareVolume(int volume)
{
    if (!hardwareController) {
        return;
    }
    
    // Format volume display
    std::string volumeText = "Volume: " + std::to_string(volume) + "%";
    hardwareController->displayOnScreen(volumeText);
}