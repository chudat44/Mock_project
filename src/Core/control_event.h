#ifndef CONTROL_EVENT_H
#define CONTROL_EVENT_H

enum class HardwareControlEvent
{
    NONE,
    BUTTON_PLAY_PAUSE,
    BUTTON_STOP,
    BUTTON_NEXT,
    BUTTON_PREVIOUS,
    BUTTON_VOLUME_UP,
    BUTTON_VOLUME_DOWN,
    ADC_VOLUME_CHANGE
};

// Enum for hardware control events
enum class ControlEvent {
    NONE,
    BUTTON_PLAY,
    BUTTON_PAUSE,
    BUTTON_STOP,
    BUTTON_NEXT,
    BUTTON_PREVIOUS,
    VOLUME_CHANGE
};

#endif