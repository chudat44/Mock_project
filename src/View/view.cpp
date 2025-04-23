#include "view.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <shobjidl.h> // For IFileDialog
#include <optional>
#endif

#define CORNER_DETECTION_AREA 15

// Implementation for View class
View::View() : active(false) {}

View::~View()
{
    // Clean up all components
    for (auto &component : components)
    {
        delete component;
    }
    components.clear();
}

// Render all the componenet
void View::render(SDL_Renderer *renderer)
{
    // Set background for playlist area
    SDL_SetRenderDrawColor(renderer, PANEL_COLOR.r, PANEL_COLOR.g, PANEL_COLOR.b, PANEL_COLOR.a);
    SDL_RenderFillRect(renderer, &viewBounds);
    // Panel border
    SDL_SetRenderDrawColor(renderer, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b, BORDER_COLOR.a);
    SDL_RenderDrawRect(renderer, &viewBounds);
    if (!active)
        return;

    // Render all components
    for (auto &component : components)
    {
        if (component->isVisible())
        {
            component->render(renderer);
        }
    }
}

bool View::handleEvent(SDL_Event *event)
{
    if (!active)
        return false;

    // Handle events for all components in reverse order (top-most first)
    for (auto it = components.rbegin(); it != components.rend(); ++it)
    {
        if ((*it)->isVisible() && (*it)->isEnabled())
        {
            if ((*it)->handleEvent(event))
            {
                return true; // Event was handled
            }
        }
    }
    return false;
}

void View::show()
{
    active = true;
}

void View::hide()
{
    active = false;
}

void View::addComponent(UIComponent *component)
{
    components.push_back(component);
}

void View::removeComponent(UIComponent *component)
{
    auto it = std::find(components.begin(), components.end(), component);
    if (it != components.end())
    {

        components.erase(it);
    }
}

bool View::isActive() const
{
    return active;
}
bool View::isInViewRect(int x, int y)
{
    return (x >= viewBounds.x && x < viewBounds.x + viewBounds.w &&
            y >= viewBounds.y && y < viewBounds.y + viewBounds.h);
}

// MediaListView Implementation
MediaListView::MediaListView(MediaListController *controller)
    : controller(controller), itemsPerPage(25)
{
    viewBounds = {240, 20, 500, 500};
    // Create components
    fileListView = new ListView(245, 50, 490, 430);
    pagination = new Pagination(385, 480, 210, 30);
    // Create title label
    titleLabel = new TextComponent(450, 25, 90, 15, "Media List");
    // Create scan directory button
    openFolderButton = new Button(25, 485, 190, 30, "Open Folder");

    // Add components to view
    addComponent(fileListView);
    addComponent(pagination);
    addComponent(titleLabel);
    addComponent(openFolderButton);

    titleLabel->setAlign(TextComponent::TextAlign::Center);

    pagination->setVisible(false);

    show();
}

MediaListView::~MediaListView()
{
    // Base destructor will handle component deletion
}

void MediaListView::setMediaListController(MediaListController *controller)
{
    this->controller = controller;
    // Setup UI components
    fileListView->setOnSelectionChanged([this](int index)
                                        { onFileSelected(index); });
    fileListView->setOn2ClickSelectionChanged([this](int index)
                                              { onFile2ClickSelected(index); });

    pagination->setOnPageChanged([this](int page)
                                 { setCurrentPage(page); });

    openFolderButton->setOnClick([this]()
                                 { scanDirectoryForMedia(); });
}

void MediaListView::render(SDL_Renderer *renderer)
{
    // Default rendering done by base class
    View::render(renderer);
}

bool MediaListView::handleEvent(SDL_Event *event)
{
    // Handle right-click on list items
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_RIGHT)
    {
        int x = event->button.x;
        int y = event->button.y;

        if (fileListView->containsPoint(x, y))
        {
            // Calculate which item was clicked
            SDL_Rect bounds = fileListView->getBounds();
            int itemHeight = 30; // Assuming each item is 30px high
            int itemIndex = fileListView->getSelectedIndex();

            if (itemIndex >= 0 && itemIndex < static_cast<int>(fileListView->size()))
            {
                showFileContextMenu(x, y, itemIndex);
                return true;
            }
        }
    }

    return View::handleEvent(event);
}

void MediaListView::update()
{
}

void MediaListView::scanDirectoryForMedia()
{
    std::filesystem::path result;

#ifdef _WIN32

    HWND owner = nullptr;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        return;
    }

    IFileDialog *pFileDialog = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
    if (FAILED(hr))
    {
        CoUninitialize();
        return;
    }

    // Set the options on the dialog to select folders only
    DWORD options;
    pFileDialog->GetOptions(&options);
    pFileDialog->SetOptions(options | FOS_PICKFOLDERS);

    // Show the dialog
    hr = pFileDialog->Show(owner);
    if (SUCCEEDED(hr))
    {
        IShellItem *pItem;
        hr = pFileDialog->GetResult(&pItem);
        if (SUCCEEDED(hr))
        {
            PWSTR folderPath = nullptr;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &folderPath);

            if (SUCCEEDED(hr))
            {
                result.assign(folderPath);
                CoTaskMemFree(folderPath);
            }

            pItem->Release();
            pFileDialog->Release();
            CoUninitialize();
            return controller->scanDirectoryForMedia(result);
        }
    }

    pFileDialog->Release();
    CoUninitialize();
    return;

#endif
}

void MediaListView::setCurrentPlaylist(const std::string &playlistName, const std::vector<std::string> &mediaFilesNames)
{
    currentFilesName.clear();
    for (const auto &file : mediaFilesNames)
    {
        currentFilesName.push_back(file);
    }

    fileListView->clearItems();
    for (const auto &name : currentFilesName)
    {
        fileListView->addItem(name);
    }

    titleLabel->setText(playlistName);

    // Update pagination
    int totalFiles = currentFilesName.size();
    int totalPages = (totalFiles + itemsPerPage - 1) / itemsPerPage;
    pagination->setTotalPages(totalPages);
    pagination->setCurrentPage(0);

    // Update visibility of pagination
    pagination->setVisible(totalPages > 1);

    update();
}

void MediaListView::setCurrentPage(int page)
{
    int startIdx = page * itemsPerPage;
    int endIdx = std::min(startIdx + itemsPerPage, static_cast<int>(currentFilesName.size()));

    fileListView->clearItems();
    for (int i = startIdx; i < endIdx; ++i)
    {
        fileListView->addItem(currentFilesName[i]);
    }
}

int MediaListView::getCurrentPage() const
{
    return pagination->getCurrentPage();
}

int MediaListView::getTotalPages() const
{
    return pagination->getTotalPages();
}

void MediaListView::onFileSelected(int index)
{
    controller->handleMediaSelected(index);
}
void MediaListView::onFile2ClickSelected(int index)
{
    controller->handleMediaPlay(index);
}

void MediaListView::showFileContextMenu(int x, int y, int fileIndex)
{
    // This would create a context menu at the given position
    // Implementation depends on how you want to implement context menus
    // For now, just a placeholder
}

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

// MetadataView Implementation
MetadataView::MetadataView(MetadataController *controller)
    : controller(controller), isEditing(false), currentFile(nullptr)
{
    viewBounds = {760, 20, 220, 500};
    // Create title labels
    TextComponent *titleLabel = new TextComponent(770, 30, 200, 15, "Metadata");

    // Add "Edit" button to enter edit mode
    editButton = new Button(925, 485, 50, 30, "Edit");

    // Create buttons for edit mode (initially hidden)
    saveButton = new Button(880, 485, 95, 30, "Save");

    cancelButton = new Button(765, 485, 95, 30, "Cancel");

    addFieldButton = new Button(880, 450, 95, 30, "Add key");

    removeFieldButton = new Button(765, 450, 95, 30, "Remove key");

    addComponent(titleLabel);
    addComponent(editButton);
    addComponent(saveButton);
    addComponent(cancelButton);
    addComponent(addFieldButton);
    addComponent(removeFieldButton);

    titleLabel->setAlign(TextComponent::TextAlign::Center);

    saveButton->setVisible(false);
    cancelButton->setVisible(false);
    addFieldButton->setVisible(false);
    removeFieldButton->setVisible(false);
}

MetadataView::~MetadataView()
{
    // Base destructor will handle component deletion
}

void MetadataView::setMetadataController(MetadataController *controller)
{
    this->controller = controller;
    editButton->setOnClick(
        [this]()
        {
            enterEditMode();
            this->controller->enterEditMode();
        });
    saveButton->setOnClick(
        [this]()
        {
            saveChanges();
            this->controller->saveMetadata();
            this->controller->exitEditMode();
        });
    cancelButton->setOnClick(
        [this]()
        {
            cancelChanges();
            this->controller->discardChanges();
            this->controller->exitEditMode();
        });
    addFieldButton->setOnClick([this]()
                               { addField(); });
    removeFieldButton->setOnClick([this]()
                                  { removeSelectedField(); });
}

void MetadataView::render(SDL_Renderer *renderer)
{
    // Render components
    View::render(renderer);
}

bool MetadataView::handleEvent(SDL_Event *event)
{
    return View::handleEvent(event);
}

void MetadataView::update()
{
    // Nothing specific to update regularly
}

void MetadataView::showMetadata(const std::map<std::string, std::string> &metadata)
{
    show();
    // Clear previous metadata fields
    for (auto &label : keyLabels)
    {
        delete label;
        removeComponent(label);
    }
    for (auto &field : valueFields)
    {
        delete field;
        removeComponent(field);
    }
    keyLabels.clear();
    valueFields.clear();

    // Add metadata key-value pairs
    int yPos = 60;

    // Title
    {
        TextComponent *keyLabel = new TextComponent(770, yPos, 60, 30, "Title");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(840, yPos, 130, 30, metadata.at("Title"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(770, yPos, 60, 30, "Artist");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(840, yPos, 130, 30, metadata.at("Artist"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(770, yPos, 60, 30, "Album");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(840, yPos, 130, 30, metadata.at("Album"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(770, yPos, 60, 30, "Comment");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(840, yPos, 130, 30, metadata.at("Comment"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(770, yPos, 60, 30, "Genre");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(840, yPos, 130, 30, metadata.at("Genre"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(770, yPos, 60, 30, "Year");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(840, yPos, 130, 30, metadata.at("Year"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(770, yPos, 60, 30, "Track");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(840, yPos, 130, 30, metadata.at("Track"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(770, yPos, 60, 30, "Bitrate");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(840, yPos, 130, 30, metadata.at("Bitrate"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(770, yPos, 60, 30, "Channels");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(840, yPos, 130, 30, metadata.at("Channels"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(770, yPos, 60, 30, "Sample Rate");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(840, yPos, 130, 30, metadata.at("Sample Rate"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    // Reset edit mode
    isEditing = false;
    saveButton->setVisible(false);
    cancelButton->setVisible(false);
    addFieldButton->setVisible(false);
    removeFieldButton->setVisible(false);
    editButton->setVisible(true);
}

void MetadataView::enterEditMode()
{
    isEditing = true;

    // Enable all text fields for editing
    for (auto field = valueFields.begin(); field < valueFields.begin() + 7; ++field)
    {
        (*field)->setEnabled(true);
    }

    // Show edit mode buttons
    saveButton->setVisible(true);
    cancelButton->setVisible(true);
    addFieldButton->setVisible(true);
    removeFieldButton->setVisible(true);

    // Hide edit button
    editButton->setVisible(false);
}

void MetadataView::saveChanges()
{
    // Switch back to view mode
    isEditing = false;

    // Disable all text fields
    for (auto field = valueFields.begin(); field < valueFields.begin() + 7; ++field)
    {
        (*field)->setEnabled(false);
    }

    // Hide edit mode buttons
    saveButton->setVisible(false);
    cancelButton->setVisible(false);
    addFieldButton->setVisible(false);
    removeFieldButton->setVisible(false);

    // Show edit button
    editButton->setVisible(true);
}

void MetadataView::cancelChanges()
{
    // Switch back to view mode without saving
    isEditing = false;

    // Disable all text fields
    for (auto &field : valueFields)
    {
        field->setEnabled(false);
    }

    // Hide edit mode buttons
    saveButton->setVisible(false);
    cancelButton->setVisible(false);
    addFieldButton->setVisible(false);
    removeFieldButton->setVisible(false);

    // Show edit button
    editButton->setVisible(true);
}

void MetadataView::addField()
{
    // This would create a dialog to add a new field
    // For simplicity, we'll just add a new field at the end
    int yPos = 200 + (keyLabels.size() - 1) * 40;

    TextComponent *newKeyLabel = new TextComponent(770, yPos, 150, 30, "New Key");
    addComponent(newKeyLabel);
    keyLabels.push_back(newKeyLabel);

    TextField *newValueField = new TextField(870, yPos, 150, 30, "New Value");
    newValueField->setEnabled(true);
    addComponent(newValueField);
    valueFields.push_back(newValueField);
}

void MetadataView::removeSelectedField()
{
    // This would show a dialog to select which field to remove
    // For simplicity, we'll just show a message
    // In a real implementation, this would prompt for which field to edit/remove
}

// PlaylistsListView Implementation
PlaylistsListView::PlaylistsListView(PlaylistsListController *controller) : controller(controller)
{
    viewBounds = {20, 20, 200, 500};
    // Create list view for playlists
    playlistsList = new ListView(25, 50, 190, 430);

    // Create title label
    titleLabel = new TextComponent(90, 25, 65, 15, "Playlists");

    // Create pagination (initially hidden)
    pagination = new Pagination(25, 410, 190, 30);

    playlistNameField = new TextField(25, 410, 190, 30, "");

    // Create Create playlist button
    createButton = new Button(25, 445, 190, 30, "Create");

    addComponent(titleLabel);
    addComponent(playlistsList);
    addComponent(pagination);
    addComponent(createButton);
    addComponent(playlistNameField);

    titleLabel->setAlign(TextComponent::TextAlign::Center);

    playlistNameField->setPlaceholder("New playlist name");
    playlistNameField->setVisible(false);

    pagination->setVisible(false);

    show();
}

PlaylistsListView::~PlaylistsListView()
{
    // Base destructor will handle component deletion
}

void PlaylistsListView::setPlaylistsListController(PlaylistsListController *controller)
{
    this->controller = controller;
    playlistsList->setOnSelectionChanged(
        [this](int index)
        { this->controller->handlePlaylistSelected(index); });
    createButton->setOnClick(
        [this]()
        {
            if (playlistNameField->isVisible())
                playlistNameField->setVisible(false);
            else
                this->createNewPlaylist();
        });
    // Set callback for when the user presses Enter
    playlistNameField->setOnTextChanged(
        [this](const std::string &text)
        {
            if (!text.empty())
            {
                // Call controller to create playlist
                playlistNameField->setVisible(false);
                this->controller->createPlaylist(text);
                playlistNameField->setPlaceholder("New playlist name");
            }
        });
}

void PlaylistsListView::render(SDL_Renderer *renderer)
{

    // Render components
    View::render(renderer);
}

bool PlaylistsListView::handleEvent(SDL_Event *event)
{
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_RIGHT)
    {
        int x = event->button.x;
        int y = event->button.y;

        if (playlistsList->containsPoint(x, y))
        {
            // Show context menu for the playlist
            // This would be implemented based on your context menu system
            return true;
        }
    }

    return View::handleEvent(event);
}

void PlaylistsListView::update()
{
    // Nothing specific to update regularly
}

void PlaylistsListView::setPlaylists(const std::vector<std::string> &playlistNames)
{
    playlistsList->clearItems();
    for (const auto &name : playlistNames)
    {
        playlistsList->addItem(name);
    }
}

int PlaylistsListView::getSelectedPlaylist() const
{
    return playlistsList->getSelectedIndex();
}

void PlaylistsListView::createNewPlaylist()
{
    // Create text field for new playlist name
    // Show text field for entering new playlist name
    playlistNameField->setVisible(true);
    playlistNameField->focus();
}

void PlaylistsListView::deleteSelectedPlaylist()
{
    int selectedIndex = playlistsList->getSelectedIndex();
    if (selectedIndex >= 0)
    {
        // Call controller to delete playlist
        // controller->deletePlaylist(selectedIndex);
        playlistsList->removeItem(selectedIndex);
    }
}

void PlaylistsListView::openSelectedPlaylist()
{
    int selectedIndex = playlistsList->getSelectedIndex();
    if (selectedIndex >= 0)
    {
    }
}

// Main Window implementation

MainWindow::MainWindow(int width, int height, std::string title)
    : width(width), height(height), title(title),
      window(nullptr), exitRequested(false), is_dragging_corner(false)
{
}
MainWindow::~MainWindow()
{
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}
SDL_Window *MainWindow::initialize()
{
    // Create window
    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    return window;
}
int MainWindow::getWidth() { return width; }
int MainWindow::getHeight() { return height; }
bool MainWindow::getExitRequest() const { return exitRequested; }
SDL_Window *MainWindow::getWindow() { return window; }

bool MainWindow::is_mouse_in_corner(SDL_Window *window, int mouse_x, int mouse_y)
{
    int window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);

    // Check bottom-right corner
    return (mouse_x >= window_width - CORNER_DETECTION_AREA &&
            mouse_y >= window_height - CORNER_DETECTION_AREA);
}

void MainWindow::updatePolling(SDL_Event &event)
{
    switch (event.type)
    {
    case SDL_QUIT:
        exitRequested = true;
        break;

    case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT)
        {
            int mouse_x = event.button.x;
            int mouse_y = event.button.y;

            if (is_mouse_in_corner(window, mouse_x, mouse_y))
            {
                is_dragging_corner = true;
                last_mouse_x = mouse_x;
                last_mouse_y = mouse_y;

                // Set a custom cursor if desired
                SDL_Cursor *arrowCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
                SDL_SetCursor(arrowCursor);
            }
            else
            {
            }
        }
        break;

    case SDL_MOUSEBUTTONUP:
        if (event.button.button == SDL_BUTTON_LEFT && is_dragging_corner)
        {
            is_dragging_corner = false;

            // Reset cursor
            SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW));
        }
        break;

    case SDL_MOUSEMOTION:
        if (is_dragging_corner)
        {
            int mouse_x = event.motion.x;
            int mouse_y = event.motion.y;

            // Calculate the change in position
            int dx = mouse_x - last_mouse_x;
            int dy = mouse_y - last_mouse_y;

            // Update window size
            width += dx;
            height += dy;

            // Enforce minimum size
            if (width < 200)
                width = 200;
            if (height < 200)
                height = 200;

            // Resize the window
            SDL_SetWindowSize(window, width, height);

            // Update last mouse position
            last_mouse_x = mouse_x;
            last_mouse_y = mouse_y;
        }
        else if (is_mouse_in_corner(window, event.motion.x, event.motion.y))
        {
            // Show resize cursor when hovering over corner
            SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE));
        }
        else
        {
            // Reset cursor
            SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW));
        }
    }
}

// View Manager implementation
ViewManager::ViewManager()
    : renderer(nullptr),
      mainWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Media Player"),
      showingDialog(false),
      dialogTimer(0)
{
}

ViewManager::~ViewManager()
{

    // Destroy SDL components
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
}

bool ViewManager::initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0)
    {
        std::cerr << "SDL_ttf could not initialize! TTF Error: " << TTF_GetError() << std::endl;
        return false;
    }
    if (!mainWindow.initialize())
    {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(mainWindow.getWindow(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    // Initialize renderer color
    SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);

    // Create views
    mediaListView = std::make_unique<MediaListView>(nullptr);         // Will be set later
    playerView = std::make_unique<PlayerView>(nullptr);               // Will be set later
    playlistsListView = std::make_unique<PlaylistsListView>(nullptr); // Will be set later
    metadataView = std::make_unique<MetadataView>(nullptr);           // Will be set later

    appController = std::make_unique<ApplicationController>(this);
    appController->initialize(mediaListView.get(), playerView.get(), playlistsListView.get(), metadataView.get());

    mediaListView->setMediaListController(appController->getMediaListController());
    playerView->setPlayerController(appController->getPlayerController());
    playlistsListView->setPlaylistsListController(appController->getPlaylistsListController());
    metadataView->setMetadataController(appController->getMetadataController());

    return true;
}

void ViewManager::handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        mainWindow.updatePolling(event);
        if (shouldExit())
            break;

        mediaListView->handleEvent(&event);
        playerView->handleEvent(&event);
        playlistsListView->handleEvent(&event);
        metadataView->handleEvent(&event);
        // Handle dialog events first if showing
        if (showingDialog)
        {
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                // Any click dismisses dialog
                showingDialog = false;
                dialogTimer = 0;
                continue;
            }
            // Don't process other events while dialog is showing
            continue;
        }
    }
}

void ViewManager::update()
{
    // Update dialog timer if showing
    if (showingDialog)
    {
        dialogTimer--;
        if (dialogTimer <= 0)
        {
            showingDialog = false;
        }
    }
}

void ViewManager::render()
{

    // Clear screen
    SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a); // Light gray background
    SDL_RenderClear(renderer);

    // Render all active views
    if (playlistsListView && playlistsListView->isActive())
        playlistsListView->render(renderer);

    if (mediaListView && mediaListView->isActive())
        mediaListView->render(renderer);

    if (playerView && playerView->isActive())
        playerView->render(renderer);

    if (metadataView && metadataView->isActive())
        metadataView->render(renderer);

    // Render dialog if showing
    if (showingDialog)
    {
        // Dialog background
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 220);
        SDL_Rect dialogRect = {
            mainWindow.getWidth() / 4,
            mainWindow.getHeight() / 3,
            mainWindow.getWidth() / 2,
            mainWindow.getHeight() / 4};
        SDL_RenderFillRect(renderer, &dialogRect);

        // Dialog border
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &dialogRect);

        // Render dialog text using SDL_ttf (implementation omitted for brevity)
        // You would need to render the dialogMessage using TTF_Font
    }

    // Update screen
    SDL_RenderPresent(renderer);
}

void ViewManager::run()
{
    appController->run();
}

void ViewManager::showDialog(const std::string &message)
{
    dialogMessage = message;
    showingDialog = true;
    dialogTimer = 180; // Show for about 3 seconds (60 frames/sec * 3)
}

bool ViewManager::shouldExit() const
{
    return mainWindow.getExitRequest();
}

SDL_Renderer *ViewManager::getRenderer() const
{
    return renderer;
}