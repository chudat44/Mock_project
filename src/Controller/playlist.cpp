#include "playlist.h"


using json = nlohmann::json;
namespace fs = std::filesystem;

const std::string playlistsFilePath = "data/playlist/index.json";
const std::string scanDirFilePath = "data/scan_dir/dir.json";

// PlaylistsListController implementation
PlaylistsListController::PlaylistsListController(
    PlaylistsListInterface *plI) : playlistsListView(plI)
{
    playlistsManager = std::make_unique<PlaylistsManager>();
}

void PlaylistsListController::setPlaylistsListView(PlaylistsListInterface *view)
{
    playlistsListView = view;
}

void PlaylistsListController::createPlaylist(const std::string &name)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    playlistsManager->createPlaylist(name);
    updatePlaylistsListView();
}

void PlaylistsListController::deletePlaylist(int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    auto playlists = playlistsManager->getAllPlaylists();
    if (index >= 0 && index < playlists.size())
    {
        auto playlist = playlists[index];
        playlistsManager->deletePlaylist(playlist);
        if (index == currentPlaylistIndex)
        {
            currentPlaylistIndex = -1;
            onPlaylistSelectedCallback(std::make_shared<PlaylistModel>(nullptr));
        }
        updatePlaylistsListView();
    }
}

void PlaylistsListController::renamePlaylist(const std::string &oldName, const std::string &newName)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    auto playlist = playlistsManager->getPlaylist(oldName);
    if (playlist)
    {
        playlist->setPlaylistName(newName);
        saveAllPlaylists();
        updatePlaylistsListView();
    }
}

void PlaylistsListController::loadAllPlaylists()
{
    std::lock_guard<std::mutex> lock(playlistMutex);

    // Load playlist from file
    if (!fs::exists(playlistsFilePath))
    {
        std::cerr << "Playlists data File does not exist. No playlists found.\n";
        return;
    }

    std::ifstream file(playlistsFilePath);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file";
    }
    json indexJson;
    try
    {
        file >> indexJson;
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON parsing failed at byte " << e.byte << ": " << e.what() << "\n";
    }

    // Pass json object from file index
    for (const auto &item : indexJson)
    {
        std::string name = item["name"];
        std::string file = item["file"];

        std::ifstream in("data/playlists/" + file);
        if (!in.is_open())
            continue;

        json content;
        in >> content;

        playlistsManager->loadPlaylistFromJson(content);
    }

    updatePlaylistsListView();
}

void PlaylistsListController::moveItemUp(int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    if (index > 0 && index < playlistsManager->getAllPlaylists().size())
    {
        // This would swap the item at index with the one at index-1
        // Since this is not implemented in your model class, we'd add this functionality here
        auto playlists = playlistsManager->getAllPlaylists(); // Get a copy
        if (index > 0 && index < playlists.size())
        {
            std::swap(playlists[index], playlists[index - 1]);

            // // Recreate playlist with new order
            // auto name = currentPlaylist->getPlaylistName();
            // playlistsManager->deletePlaylist(currentPlaylist);
            // playlistsManager->createPlaylist(name);
            // currentPlaylist = playlistsManager->getPlaylist(name);

            // for (auto &file : files)
            // {
            //     currentPlaylist->addMediaFile(file);
            // }

            saveAllPlaylists();
            updatePlaylistsListView();
        }
    }
}

void PlaylistsListController::moveItemDown(int index)
{
    std::lock_guard<std::mutex> lock(playlistMutex);
    if (index > 0 && index < playlistsManager->getAllPlaylists().size())
    {
        // This would swap the item at index with the one at index-1
        // Since this is not implemented in your model class, we'd add this functionality here
        auto playlists = playlistsManager->getAllPlaylists(); // Get a copy
        if (index > 0 && index < playlists.size())
        {
            std::swap(playlists[index], playlists[index - 1]);

            // // Recreate playlist with new order
            // auto name = currentPlaylist->getPlaylistName();
            // playlistsManager->deletePlaylist(currentPlaylist);
            // playlistsManager->createPlaylist(name);
            // currentPlaylist = playlistsManager->getPlaylist(name);

            // for (auto &file : files)
            // {
            //     currentPlaylist->addMediaFile(file);
            // }

            saveAllPlaylists();
            updatePlaylistsListView();
        }
    }
}

void PlaylistsListController::updatePlaylistsListView()
{
    if (playlistsListView)
    {
        // Update the view with current playlistsstd::vector<std::string> playlistNames;
        auto playlists = playlistsManager->getAllPlaylists();
        std::vector<std::string> playlistNames;
        for (const auto &playlist : playlists)
        {
            playlistNames.push_back(playlist->getPlaylistName());
        }
        playlistsListView->setPlaylists(playlistNames);
    }
}

void PlaylistsListController::saveAllPlaylists()
{
    json indexArray;
    for (auto playlist : playlistsManager->getAllPlaylists())
    {
        std::string name = playlist->getPlaylistName();
        std::string filename = name + ".json";

        indexArray.push_back({{"name", name},
                              {"file", filename}});

        std::ofstream out("data/playlists/" + filename);
        json content;
        playlistsManager->parsePlaylistToJson(content, playlist);
        out << content.dump(4);
    }

    std::ofstream indexOut("data/playlists/index.json");
    indexOut << indexArray.dump(4);
}

std::vector<std::shared_ptr<PlaylistModel>> PlaylistsListController::getAllPlaylists() const
{
    return playlistsManager->getAllPlaylists();
}

void PlaylistsListController::setOnPlaylistSelectedCallback(std::function<void(std::shared_ptr<PlaylistModel>)> callback)
{
    onPlaylistSelectedCallback = callback;
}
void PlaylistsListController::setOnPlaylistPlayCallback(std::function<void(std::shared_ptr<PlaylistModel>)> callback)
{
    onPlaylistPlayCallback = callback;
}

void PlaylistsListController::handlePlaylistSelected(int index)
{
    if (currentPlaylistIndex != index)
    {
        if (onPlaylistSelectedCallback)
        {
            onPlaylistSelectedCallback(playlistsManager->getPlaylist(index));
        }
    }
}
void PlaylistsListController::handlePlaylistPlay(int index)
{
    if (currentPlaylistIndex != index)
    {
        if (onPlaylistPlayCallback)
        {
            onPlaylistPlayCallback(playlistsManager->getPlaylist(index));
        }
    }
}
