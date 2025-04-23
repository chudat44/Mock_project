#ifndef MEDIA_PLAYER_VIEW_H
#define MEDIA_PLAYER_VIEW_H

#include "Model/playlist.h"
#include "View/Interface/Iview.h"
#include "Controller/controller.h"

#include "component.h"
#include "player.h"
#include "playlist.h"
#include "medialist.h"
#include "metadata.h"

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 680;


class MainWindow
{
private:
    SDL_Window *window;
    int width;
    int height;
    std::string title;
    
    int last_mouse_x, last_mouse_y;
    bool is_dragging_corner;
    bool exitRequested;

public:
    MainWindow(int width, int height, std::string title);
    ~MainWindow();

    SDL_Window *  initialize();

    bool is_mouse_in_corner(SDL_Window *window, int mouse_x, int mouse_y);
    void updatePolling(SDL_Event &event);

    int getWidth();
    int getHeight();

    bool getExitRequest() const;
    SDL_Window *getWindow();
};

class ViewManager : public ViewManagerInterface
{
private:
    SDL_Renderer *renderer;

    MainWindow mainWindow;

    std::unique_ptr<ApplicationController> appController;

    // Create views
    std::unique_ptr<MediaListView> mediaListView;
    std::unique_ptr<PlayerView> playerView;
    std::unique_ptr<PlaylistsListView> playlistsListView;
    std::unique_ptr<MetadataView> metadataView;

    // Dialog properties
    bool showingDialog;
    std::string dialogMessage;
    int dialogTimer;

public:
    ViewManager();
    ~ViewManager();

    // Initialize SDL window and renderer
    bool initialize();

    void handleEvents() override;
    void update() override;
    void render() override;
    void run() override;
    void showDialog(const std::string &message) override;

    // Additional methods
    bool shouldExit() const;
    SDL_Renderer *getRenderer() const;
};
#endif // MEDIA_PLAYER_VIEW_H