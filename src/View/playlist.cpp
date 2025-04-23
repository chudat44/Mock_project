#include "playlist.h"

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
