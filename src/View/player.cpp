#include "player.h"


// PlayerView Implementation
PlayerView::PlayerView(PlayerController *controller)
    : isPlaying(false), controller(controller)
{
    viewBounds = {20, 550, 960, 100};
    // Create components for the player view
    currentTrackLabel = new TextComponent(280, 575, 610, 30, "");
    currentTimeLabel = new TextComponent(210, 605, 50, 30, "0:00");
    totalTimeLabel = new TextComponent(910, 605, 50, 30, "0:00");

    playPauseButton = new Button(40, 600, 30, 30, "▶");

    stopButton = new Button(80, 600, 30, 30, "■");

    previousButton = new Button(120, 600, 30, 30, "◀◀");

    nextButton = new Button(160, 600, 30, 30, "▶▶");

    progressBar = new ProgressBar(270, 615, 630, 10);

    volumeSlider = new VolumeSlider(50, 570, 150, 15);

    // Add components to view
    addComponent(currentTrackLabel);
    addComponent(currentTimeLabel);
    addComponent(totalTimeLabel);
    addComponent(progressBar);
    addComponent(playPauseButton);
    addComponent(stopButton);
    addComponent(previousButton);
    addComponent(nextButton);
    addComponent(volumeSlider);

    totalTimeLabel->setAlign(TextComponent::TextAlign::Left);
    currentTimeLabel->setAlign(TextComponent::TextAlign::Right);
    currentTrackLabel->setAlign(TextComponent::TextAlign::Center);

    progressBar->setEnabled(false);

    volumeSlider->setVolume(20);

    show();
}

PlayerView::~PlayerView()
{
    // Base destructor will handle component deletion
}

void PlayerView::setPlayerController(PlayerController *controller)
{
    this->controller = controller;
    progressBar->setIsDraggable(true);
    progressBar->setOnValueChanged([this](float value)
                                   { this->controller->seek(static_cast<int>(
                                         value * this->controller->getDuration())); });
    playPauseButton->setOnClick([this]()
                                {
        if (isPlaying) {
            this->controller->pause();
        } else {
            this->controller->play();
        } });

    stopButton->setOnClick([this]()
                           { this->controller->stop(); });

    previousButton->setOnClick([this]()
                               { this->controller->previous(); });

    nextButton->setOnClick([this]()
                           { this->controller->next(); });

    volumeSlider->setOnVolumeChanged([this](int vol)
                                     { this->controller->setVolume(vol); });
}

void PlayerView::render(SDL_Renderer *renderer)
{
    // Render components
    View::render(renderer);
}

bool PlayerView::handleEvent(SDL_Event *event)
{
    return View::handleEvent(event);
}

void PlayerView::update()
{
    if (isPlaying)
    {
        updateProgress(controller->getCurrentPosition(), controller->getDuration());

        // Update volume slider
        volumeSlider->setVolume(controller->getVolume());
    }
}

void PlayerView::setBounds(int x, int y, int w, int h)
{
    viewBounds = {x, y, w, h};
}

void PlayerView::setCurrentMedia(const std::string &trackName, const std::string &artist)
{
    std::string displayName = trackName;
    if (!artist.empty())
    {
        displayName += " - " + artist;
    }
    currentTrackLabel->setText(displayName);

    progressBar->setEnabled(true);
}

void PlayerView::updatePlaybackStatus(bool playing)
{
    isPlaying = playing;
    // Update UI based on current playback state
    playPauseButton->setText(isPlaying ? "||" : "▶");
}

void PlayerView::updateProgress(int currentPosition, int duration)
{
    // Format time as mm:ss
    int currMin = currentPosition / 60;
    int currSec = currentPosition % 60;

    int totalMin = duration / 60;
    int totalSec = duration % 60;
    std::string timeText = std::to_string(currMin) + ":" +
                           (currSec < 10 ? "0" : "") + std::to_string(currSec);
    currentTimeLabel->setText(timeText);

    timeText = std::to_string(totalMin) + ":" +
               (totalSec < 10 ? "0" : "") + std::to_string(totalSec);
    totalTimeLabel->setText(timeText);

    // Update progress bar if not being dragged

    // -1,-1 is a proxy for "not being dragged"

    if (duration == 0)
        progressBar->setValue(0);
    else
        progressBar->setValue(static_cast<float>(currentPosition) / duration);
}

void PlayerView::updateVolume(int volume)
{
    volumeSlider->setVolume(volume);
}
