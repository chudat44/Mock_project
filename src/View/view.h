#ifndef MEDIA_PLAYER_VIEW_H
#define MEDIA_PLAYER_VIEW_H

#include "Model/playlist.h"
#include "Controller/controller.h"
#include "View/Interface/Iview.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>

// Base GUI Component Classes
class UIComponent
{
protected:
    SDL_Rect bounds;
    bool visible;
    bool enabled;

public:
    UIComponent(int x, int y, int w, int h);
    virtual ~UIComponent() = default;

    virtual void render(SDL_Renderer *renderer) = 0;
    virtual bool handleEvent(SDL_Event *event) = 0;

    void setVisible(bool isVisible);
    bool isVisible() const;
    void setEnabled(bool isEnabled);
    bool isEnabled() const;
    void setBounds(int x, int y, int w, int h);
    SDL_Rect getBounds() const;
    bool containsPoint(int x, int y) const;
};

class Button : public UIComponent
{
private:
    std::string text;
    SDL_Color textColor;
    SDL_Color backgroundColor;
    SDL_Color hoverColor;
    bool isHovered;
    std::function<void()> onClick;

public:
    Button(int x, int y, int w, int h, const std::string &buttonText);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void setText(const std::string &buttonText);
    void setOnClick(std::function<void()> callback);
    void setColors(SDL_Color text, SDL_Color background, SDL_Color hover);
};

class Label : public UIComponent
{
private:
    std::string text;
    SDL_Color textColor;
    TTF_Font *font;
    bool isMultiline;

public:
    Label(int x, int y, int w, int h, const std::string &labelText);
    ~Label();

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void setText(const std::string &labelText);
    std::string getText() const;
    void setTextColor(SDL_Color color);
    void setFont(TTF_Font *newFont);
};

class ProgressBar : public UIComponent
{
private:
    float value; // 0.0 to 1.0
    SDL_Color fillColor;
    SDL_Color backgroundColor;
    bool isDraggable;
    std::function<void(float)> onValueChanged;

public:
    ProgressBar(int x, int y, int w, int h);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void setValue(float newValue);
    float getValue() const;
    void setIsDraggable(bool draggable);
    void setOnValueChanged(std::function<void(float)> callback);
    void setColors(SDL_Color fill, SDL_Color background);
};

class ListView : public UIComponent
{
private:
    std::vector<std::string> items;
    int selectedIndex;
    int firstVisibleIndex;
    int visibleItemCount;
    bool hasScrollbar;
    std::function<void(int)> onSelectionChanged;

public:
    ListView(int x, int y, int w, int h);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void addItem(const std::string &item);
    void removeItem(int index);
    void clearItems();
    size_t size() const;
    std::string getItem(int index) const;
    const std::vector<std::string> &getItems() const;
    void setItems(const std::vector<std::string> &newItems);
    int getSelectedIndex() const;
    void setSelectedIndex(int index);
    void setOnSelectionChanged(std::function<void(int)> callback);
    void scroll(int amount);
};

class TextField : public UIComponent
{
private:
    std::string text;
    std::string placeholder;
    SDL_Color textColor;
    SDL_Color backgroundColor;
    SDL_Color borderColor;
    bool isFocused;
    int cursorPosition;
    std::function<void(const std::string &)> onTextChanged;

public:
    TextField(int x, int y, int w, int h, const std::string &initialText = "");

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void setText(const std::string &newText);
    std::string getText() const;
    void setPlaceholder(const std::string &placeholderText);
    void setOnTextChanged(std::function<void(const std::string &)> callback);
    void focus();
    void unfocus();
};

class VolumeSlider : public UIComponent
{
private:
    int volume; // 0-100
    SDL_Color fillColor;
    SDL_Color backgroundColor;
    SDL_Color knobColor;
    bool isDragging;
    std::function<void(int)> onVolumeChanged;

public:
    VolumeSlider(int x, int y, int w, int h);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void setVolume(int newVolume);
    int getVolume() const;
    void setOnVolumeChanged(std::function<void(int)> callback);
};

class Pagination : public UIComponent
{
private:
    int currentPage;
    int totalPages;
    Button *prevButton;
    Button *nextButton;
    Label *pageLabel;
    std::function<void(int)> onPageChanged;

public:
    Pagination(int x, int y, int w, int h);
    ~Pagination();

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void setCurrentPage(int page);
    int getCurrentPage() const;
    void setTotalPages(int pages);
    void setOnPageChanged(std::function<void(int)> callback);

    void updateButtonStates();
};

// Main View class (parent for all views)
class View
{
protected:
    std::vector<UIComponent *> components;
    bool active;
    ApplicationController *controller;

public:
    View(ApplicationController *appController);
    virtual ~View();

    virtual void render(SDL_Renderer *renderer);
    virtual bool handleEvent(SDL_Event *event);

    virtual void show();
    virtual void hide();
    virtual void update() = 0;

    void addComponent(UIComponent *component);
    void removeComponent(UIComponent *component);
    bool isActive() const;
};

// Specific View Implementations
class MediaListView : public View, public MediaListInterface
{
private:
    ListView *fileListView;
    Pagination *pagination;
    Button *playButton;
    Button *addToPlaylistButton;
    Button *viewMetadataButton;
    TextField *searchField;
    std::vector<MediaFileModel> currentFiles;
    int itemsPerPage;

public:
    MediaListView(ApplicationController *controller);
    ~MediaListView();

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;
    void update() override;

    void updateFileList(const std::vector<MediaFileModel> &files);
    void setCurrentPage(int page);
    int getCurrentPage() const;
    int getTotalPages() const;
    void onFileSelected(int index);
    void showFileContextMenu(int x, int y, int fileIndex);
};

class PlaylistView : public View, public PlaylistInterface
{
private:
    ListView *playlistContent;
    Label *playlistNameLabel;
    Button *playButton;
    Button *removeButton;
    Button *moveUpButton;
    Button *moveDownButton;
    Button *saveButton;
    PlaylistModel *currentPlaylist;

public:
    PlaylistView(ApplicationController *controller);
    ~PlaylistView();

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;
    void update() override;

    void setCurrentPlaylist(PlaylistModel *playlist);
    void highlightPlaying(int index);
    void enterEditMode();
    void exitEditMode();
};

class PlayerView : public View, public PlayerInterface
{
private:
    Label *currentTrackLabel;
    Label *artistAlbumLabel;
    Label *timeLabel;
    ProgressBar *progressBar;
    Button *playPauseButton;
    Button *stopButton;
    Button *previousButton;
    Button *nextButton;
    VolumeSlider *volumeSlider;
    MediaFileModel *currentMedia;
    bool isPlaying;
    SDL_Rect viewBounds;

public:
    PlayerView(ApplicationController *controller);
    ~PlayerView();

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;
    void update() override;
    void setBounds(int x, int y, int w, int h);
    void setCurrentMedia(MediaFileModel *media);

    void updatePlaybackStatus(bool playing);
    void updateProgress(int currentPosition, int duration);
    void updateVolume(int volume);
};

class MetadataView : public View, public MetadataInterface
{
private:
    std::vector<Label *> keyLabels;
    std::vector<TextField *> valueFields;
    Button *addFieldButton;
    Button *removeFieldButton;
    Button *saveButton;
    Button *cancelButton;
    Button *editButton;
    MediaFileModel *currentFile;
    bool isEditing;

public:
    MetadataView(ApplicationController *controller);
    ~MetadataView();

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;
    void update() override;

    void showMetadata(MediaFileModel *file);
    void enterEditMode();
    void saveChanges();
    void cancelChanges();
    void addField();
    void removeSelectedField();
};

class PlaylistsListView : public View, public PlaylistsListInterface
{
private:
    ListView *playlistsList;
    Button *newPlaylistButton;
    Button *deletePlaylistButton;
    Button *openPlaylistButton;
    TextField *playlistNameField;

public:
    PlaylistsListView(ApplicationController *controller);
    ~PlaylistsListView();

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;
    void update() override;

    void setPlaylists(const std::vector<std::string> &playlistNames);
    int getSelectedPlaylist() const;
    void createNewPlaylist();
    void deleteSelectedPlaylist();
    void openSelectedPlaylist();
};

class USBView : public View, public USBInterface
{
private:
    ListView *usbDevicesList;
    Button *mountButton;
    Button *unmountButton;
    Label *statusLabel;
    bool deviceMounted;

public:
    USBView(ApplicationController *controller);
    ~USBView();

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;
    void update() override;

    void updateDeviceList(const std::vector<std::string> &devices);
    void setMountStatus(bool isMounted);
    void mountSelectedDevice();
    void unmountDevice();
};

class MainWindow 
{
private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    std::map<std::string, View *> views;
    View *currentView;
    ApplicationController *controller;

    // Common widgets that appear in multiple views
    PlayerView *miniPlayerView;

    // Modal dialog support
    bool showingDialog;
    Label *dialogLabel;
    Button *dialogOkButton;

public:
    MainWindow(ApplicationController *appController);
    ~MainWindow();

    bool initialize(const std::string &title, int width, int height);
    void cleanup();

    void render();
    void handleEvents();
    void switchView(const std::string &viewName);

    void showDialog(const std::string &message);
    void hideDialog();

    SDL_Renderer *getRenderer();
    ApplicationController *getController();

    // Add views to the window
    void addView(const std::string &name, View *view);
    View *getView(const std::string &name);
};


class ViewManager : public ViewManagerInterface
{
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* headingFont;
    TTF_Font* bodyFont;
    TTF_Font* smallFont;
    SDL_Color textColor;
    SDL_Color bgColor;

    ViewState currentState;
    std::map<ViewState, View*> views;
    
    // Controllers
    ApplicationController* appController;
    PlayerController* playerController;
    PlaylistController* playlistController;
    USBController* usbController;
    MetadataController* metadataController;
    
    // Model references
    std::shared_ptr<MediaLibrary> mediaLibrary;
    std::shared_ptr<PlaylistsManager> playlistsManager;
    std::shared_ptr<USBManager> usbManager;
    std::shared_ptr<MetadataManager> metadataManager;

    // Modal dialog support
    bool showingDialog;
    Label* dialogLabel;
    Button* dialogOkButton;

    // Current context objects
    std::shared_ptr<MediaFileModel> currentMedia;
    std::shared_ptr<PlaylistModel> currentPlaylist;

public:
    ViewManager(const std::string& title, int width, int height)
        : window(nullptr), renderer(nullptr), headingFont(nullptr),
          bodyFont(nullptr), smallFont(nullptr), currentState(ViewState::MAIN_MENU),
          appController(nullptr), playerController(nullptr), playlistController(nullptr),
          usbController(nullptr), metadataController(nullptr), showingDialog(false),
          dialogLabel(nullptr), dialogOkButton(nullptr)
    {
        // Initialize SDL
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
        TTF_Init();
        IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

        // Create window and renderer
        window = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_SHOWN);

        renderer = SDL_CreateRenderer(
            window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        // Set up default colors
        textColor = {240, 240, 240, 255};
        bgColor = {20, 20, 20, 255};

        // Load fonts
        headingFont = TTF_OpenFont("assets/fonts/OpenSans-Bold.ttf", 24);
        bodyFont = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 18);
        smallFont = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 14);

        // Fallback to system fonts if loading fails
        if (!headingFont)
            headingFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24);
        if (!bodyFont)
            bodyFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 18);
        if (!smallFont)
            smallFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14);

        // Initialize models
        mediaLibrary = std::make_shared<MediaLibrary>();
        playlistsManager = std::make_shared<PlaylistsManager>();
        usbManager = std::make_shared<USBManager>();
        metadataManager = std::make_shared<MetadataManager>();

        // Set up dialog components
        dialogLabel = new Label(width/2 - 150, height/2 - 50, 300, 60, "");
        dialogOkButton = new Button(width/2 - 50, height/2 + 20, 100, 30, "OK");
        dialogOkButton->setOnClick([this]() {
            hideDialog();
        });
    }

    ~ViewManager()
    {
        // Clean up resources
        delete dialogLabel;
        delete dialogOkButton;
        
        // Clean up views
        for (auto& [state, view] : views) {
            delete view;
        }

        // Clean up controllers - ApplicationController will delete the others
        delete appController;
        
        if (headingFont)
            TTF_CloseFont(headingFont);
        if (bodyFont)
            TTF_CloseFont(bodyFont);
        if (smallFont)
            TTF_CloseFont(smallFont);

        if (renderer)
            SDL_DestroyRenderer(renderer);
        if (window)
            SDL_DestroyWindow(window);

        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
    }

    void initialize()
    {
        // Create the application controller first
        appController = new ApplicationController(
            this, mediaLibrary, playlistsManager, usbManager, metadataManager
        );
        
        // Initialize application controller which creates other controllers
        appController->initialize();
        
        // Get pointers to the child controllers
        playerController = appController->getPlayerController();
        playlistController = appController->getPlaylistController();
        usbController = appController->getUSBController();
        metadataController = appController->getMetadataController();

        // Create all views using the ViewState enum from tempview.cpp
        views[ViewState::MEDIA_LIST] = new MediaListView(appController);
        views[ViewState::PLAYLIST_MENU] = new PlaylistsListView(appController);
        views[ViewState::PLAYLIST_CONTENT] = new PlaylistView(appController);
        views[ViewState::MEDIA_DETAILS] = new MetadataView(appController);
        views[ViewState::METADATA_EDIT] = new MetadataView(appController); // Same view in edit mode
        views[ViewState::PLAYBACK] = new PlayerView(appController);
        views[ViewState::USB_DEVICES] = new USBView(appController);

        // Register views with controllers
        if (playerController) {
            PlayerView* playerView = dynamic_cast<PlayerView*>(views[ViewState::PLAYBACK]);
            if (playerView) {
                playerController->setPlayerView(playerView);
            }
        }
        
        if (playlistController) {
            PlaylistsListView* playlistsListView = dynamic_cast<PlaylistsListView*>(views[ViewState::PLAYLIST_MENU]);
            PlaylistView* playlistView = dynamic_cast<PlaylistView*>(views[ViewState::PLAYLIST_CONTENT]);
            if (playlistsListView && playlistView) {
                playlistController->setPlaylistsListView(playlistsListView);
                playlistController->setPlaylistView(playlistView);
            }
        }
        
        if (usbController) {
            USBView* usbView = dynamic_cast<USBView*>(views[ViewState::USB_DEVICES]);
            if (usbView) {
                usbController->setUSBView(usbView);
            }
        }
        
        if (metadataController) {
            MetadataView* metadataView = dynamic_cast<MetadataView*>(views[ViewState::MEDIA_DETAILS]);
            if (metadataView) {
                metadataController->setMetadataView(metadataView);
            }
        }

        // Initialize all views
        for (auto& [state, view] : views) {
            view->update();
        }

        // Set initial state
        changeState(ViewState::MAIN_MENU);
    }

    // ViewManagerInterface methods
    void changeState(ViewState state) override {
        // Hide current view
        if (views.find(currentState) != views.end() && views[currentState]) {
            views[currentState]->hide();
        }
        
        // Update state
        currentState = state;
        
        // Show and update new view
        if (views.find(state) != views.end() && views[state]) {
            views[state]->show();
            views[state]->update();
        }
        
        // Context passing logic
        updateViewContext(state);
    }
    
    void updateViewContext(ViewState state) {
        // Update view context based on state change
        if (state == ViewState::MEDIA_DETAILS && currentMedia) {
            MetadataView* metadataView = dynamic_cast<MetadataView*>(views[state]);
            if (metadataView) {
                metadataController->loadMetadata(currentMedia);
                metadataView->update();
            }
        }
        else if (state == ViewState::METADATA_EDIT && currentMedia) {
            MetadataView* editView = dynamic_cast<MetadataView*>(views[state]);
            if (editView) {
                metadataController->loadMetadata(currentMedia);
                metadataController->enterEditMode();
                editView->update();
            }
        }
        else if (state == ViewState::PLAYLIST_CONTENT && currentPlaylist) {
            PlaylistView* contentView = dynamic_cast<PlaylistView*>(views[state]);
            if (contentView) {
                contentView->setCurrentPlaylist(currentPlaylist.get());
            }
        }
        else if (state == ViewState::PLAYBACK && currentMedia) {
            PlayerView* playbackView = dynamic_cast<PlayerView*>(views[state]);
            if (playbackView) {
                playerController->playMedia(currentMedia);
                playbackView->update();
            }
        }
    }

    void showDialog(const std::string& message) {
        showingDialog = true;
        dialogLabel->setText(message);
    }

    void hideDialog() {
        showingDialog = false;
    }

    SDL_Renderer* getRenderer() {
        return renderer;
    }

    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                appController->exit();
                return;
            }

            // Handle dialog events if showing
            if (showingDialog) {
                dialogOkButton->handleEvent(&event);
                continue;
            }

            // Let current view handle the event
            if (views.find(currentState) != views.end() && views[currentState]) {
                views[currentState]->handleEvent(&event);
            }
            
            // Also let application controller handle input for global commands
            appController->handleUserInput(event);
        }
    }

    void update() {
        if (views.find(currentState) != views.end() && views[currentState]) {
            views[currentState]->update();
        }
    }

    void render() {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderClear(renderer);

        // Render current view
        if (views.find(currentState) != views.end() && views[currentState]) {
            views[currentState]->render(renderer);
        }

        // Render dialog if showing
        if (showingDialog) {
            // Render semi-transparent background
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 192);
            SDL_Rect overlay = {0, 0, 800, 600}; // Assume window size
            SDL_RenderFillRect(renderer, &overlay);

            // Render dialog components
            dialogLabel->render(renderer);
            dialogOkButton->render(renderer);
        }

        // Present renderer
        SDL_RenderPresent(renderer);
    }

    void run() {
        bool running = true;
        while (running) {
            handleEvents();
            update();
            render();
            SDL_Delay(16); // Cap at roughly 60 FPS
        }
    }

    // Context setters for passing data between views
    void setCurrentMedia(std::shared_ptr<MediaFileModel> media) {
        currentMedia = media;
    }
    
    void setCurrentPlaylist(std::shared_ptr<PlaylistModel> playlist) {
        currentPlaylist = playlist;
    }
    
    // Getters for context objects
    std::shared_ptr<MediaFileModel> getCurrentMedia() const {
        return currentMedia;
    }
    
    std::shared_ptr<PlaylistModel> getCurrentPlaylist() const {
        return currentPlaylist;
    }
    
    // Getter for application controller
    ApplicationController* getAppController() const {
        return appController;
    }
};
#endif // MEDIA_PLAYER_VIEW_H