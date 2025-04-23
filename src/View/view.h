#ifndef MEDIA_PLAYER_VIEW_H
#define MEDIA_PLAYER_VIEW_H

#include "Model/playlist.h"
#include "View/Interface/Iview.h"
#include "Controller/controller.h"
#include "component.h"

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 680;
// Main View class (parent for all views)
class View
{
protected:
    std::vector<UIComponent *> components;
    bool active;
    SDL_Rect viewBounds;

public:
    View();
    virtual ~View();

    virtual void render(SDL_Renderer *renderer);
    virtual bool handleEvent(SDL_Event *event);

    virtual void show();
    virtual void hide();
    virtual void update() = 0;

    void addComponent(UIComponent *component);
    void removeComponent(UIComponent *component);
    bool isActive() const;
    bool isInViewRect(int x, int y);
};

class PlaylistsListView : public View, public PlaylistsListInterface
{
private:
    TextComponent *titleLabel;
    ListView *playlistsList;
    Pagination *pagination;
    TextField *playlistNameField;
    Button *createButton;

    PlaylistsListController *controller;

public:
    PlaylistsListView(PlaylistsListController *controller);
    ~PlaylistsListView();

    void setPlaylistsListController(PlaylistsListController *controller);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;
    void update() override;

    void setPlaylists(const std::vector<std::string> &playlistNames);
    int getSelectedPlaylist() const;
    void createNewPlaylist();
    void deleteSelectedPlaylist();
    void openSelectedPlaylist();
};

// Specific View Implementations
class MediaListView : public View, public MediaListInterface
{
private:
    ListView *fileListView;
    Pagination *pagination;
    TextComponent *titleLabel;
    Button *openFolderButton;

    std::vector<std::string> currentFilesName;
    int itemsPerPage;
    MediaListController *controller;

public:
    MediaListView(MediaListController *controller);
    ~MediaListView();
    void setMediaListController(MediaListController *controller);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;
    void update() override;

    void scanDirectoryForMedia();

    void setCurrentPlaylist(const std::string &playlistName, const std::vector<std::string> &mediaFilesNames);
    void setCurrentPage(int page);
    int getCurrentPage() const;
    int getTotalPages() const;

    void onFileSelected(int index);
    void onFile2ClickSelected(int index);
    void showFileContextMenu(int x, int y, int fileIndex);
};

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

class MetadataView : public View, public MetadataInterface
{
private:
    std::vector<TextComponent *> keyLabels;
    std::vector<TextField *> valueFields;
    Button *addFieldButton;
    Button *removeFieldButton;
    Button *saveButton;
    Button *cancelButton;
    Button *editButton;
    MediaFileModel *currentFile;
    bool isEditing;

    MetadataController *controller;

public:
    MetadataView(MetadataController *controller);
    ~MetadataView();

    void setMetadataController(MetadataController *controller);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;
    void update() override;

    void showMetadata(const std::map<std::string, std::string> &metadata);
    void enterEditMode();
    void saveChanges();
    void cancelChanges();
    void addField();
    void removeSelectedField();
};

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