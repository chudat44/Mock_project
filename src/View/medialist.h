#ifndef MEDIA_LIST_VIEW_H
#define MEDIA_LIST_VIEW_H


#include "component.h"
#include "Controller/medialist.h"
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

#endif