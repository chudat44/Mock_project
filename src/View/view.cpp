#include "view.h"
#include <iostream>


#define CORNER_DETECTION_AREA 15

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