#include "Model/playlist.h"
#include "Core/control_event.h"
#include "Core/user_command.h"

#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
/**
 * HardwareController class for interfacing with S32K144 board
 * Handles hardware initialization, reading ADC values for volume,
 * button presses, and screen display
 */
class HardwareController {
private:
    int adcValue;                      // Current ADC value for volume
    std::atomic<bool> isRunning;       // Flag to control background thread
    std::thread pollingThread;         // Thread for polling hardware
    std::mutex mtx;                    // Mutex for thread safety
    
    // Callback for hardware events
    std::function<void(HardwareControlEvent)> eventCallback;
    
    // Serial port descriptors
    int serialFd;
    
    // S32K144 connection parameters
    std::string devicePath;
    int baudRate;
    
    // Internal methods
    bool openSerialPort();
    void closeSerialPort();
    int sendCommand(const std::string& cmd);
    std::string readResponse();
    void pollHardware();
    
public:
    HardwareController();
    ~HardwareController();
    
    /**
     * Initialize hardware connection with S32K144 board
     * @param device Path to serial device (e.g., "/dev/ttyUSB0")
     * @param baud Baud rate for serial communication
     * @return true if initialization was successful
     */
    bool initializeHardware(const std::string& device = "/dev/ttyUSB0", int baud = 115200);
    
    /**
     * Read the current volume from ADC
     * @return ADC value (typically 0-1023 for 10-bit ADC)
     */
    int readVolumeADC();
    
    /**
     * Poll for button press events
     * @return HardwareControlEvent representing the button pressed
     */
    HardwareControlEvent readButtonPress();
    
    /**
     * Display information on the S32K144 screen
     * @param info Text to display on screen
     */
    void displayOnScreen(const std::string& info);
    
    /**
     * Start continuous hardware polling in a separate thread
     * @param callback Function to call when hardware events occur
     */
    void startPolling(std::function<void(HardwareControlEvent)> callback);
    
    /**
     * Stop hardware polling thread
     */
    void stopPolling();
    
    /**
     * Map ADC value to volume percentage (0-100)
     * @param adcValue Raw ADC value
     * @return Volume percentage (0-100)
     */
    int mapADCToVolume(int adcValue);
};

/**
 * HardwareEventAdapter class
 * Adapts hardware events to application commands
 * Acts as a bridge between HardwareController and PlayerController
 */
class HardwareEventAdapter {
private:
    HardwareController* hardwareController;
    std::function<void(UserCommand)> commandCallback;
    
    // Map hardware control events to user commands
    UserCommand mapEventToCommand(HardwareControlEvent event);
    
    // Hardware event handler
    void onHardwareEvent(HardwareControlEvent event);
    
public:
    HardwareEventAdapter(HardwareController* controller);
    ~HardwareEventAdapter();
    
    /**
     * Set callback for when hardware events are translated to commands
     * @param callback Function to call with translated UserCommand
     */
    void setCommandCallback(std::function<void(UserCommand)> callback);
    
    /**
     * Start listening for hardware events
     */
    void start();
    
    /**
     * Stop listening for hardware events
     */
    void stop();
    
    /**
     * Update display on hardware with current playback information
     * @param mediaName Name of the currently playing media
     * @param position Current position in seconds
     * @param duration Total duration in seconds
     * @param playing Whether playback is active
     */
    void updateHardwareDisplay(const std::string& mediaName, int position, int duration, bool playing);
    
    /**
     * Set volume on hardware display
     * @param volume Volume level (0-100)
     */
    void setHardwareVolume(int volume);
};
