#ifndef PLAYLIST_VIEW_H
#define PLAYLIST_VIEW_H

#include "component.h"
#include "Controller/playlist.h"



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


#endif