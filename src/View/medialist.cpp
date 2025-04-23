#include "medialist.h"


#ifdef _WIN32
#include <windows.h>
#include <shobjidl.h> // For IFileDialog
#endif

// MediaListView Implementation
MediaListView::MediaListView(MediaListController *controller)
    : controller(controller), itemsPerPage(25)
{
    viewBounds = {240, 20, 500, 500};
    // Create components
    fileListView = new ListView(245, 50, 490, 430);
    pagination = new Pagination(380, 480, 220, 30);
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
    contextMenu = new ListView(x, y, 80, 100);

    contextMenu->addItem("Add media to");
    contextMenu->addItem("Remove Media");
}
