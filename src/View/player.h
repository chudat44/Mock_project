#ifndef PLAYER_VIEW_H
#define PLAYER_VIEW_H

#include "component.h"
#include "Controller/player.h"

class PlayerView : public View, public PlayerInterface
{
private:
    TextComponent *currentTrackLabel;
    TextComponent *currentTimeLabel;
    TextComponent *totalTimeLabel;
    ProgressBar *progressBar;
    Button *playPauseButton;
    Button *stopButton;
    Button *previousButton;
    Button *nextButton;
    VolumeSlider *volumeSlider;
    bool isPlaying;

    PlayerController *controller;

public:
    PlayerView(PlayerController *controller);
    ~PlayerView();

    void setPlayerController(PlayerController *controller);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;
    void update() override;
    void setBounds(int x, int y, int w, int h);
    void setCurrentMedia(const std::string &trackName, const std::string &artist);

    void updatePlaybackStatus(bool playing);
    void updateProgress(int currentPosition, int duration);
    void updateVolume(int volume);
};


#endif