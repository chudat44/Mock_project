#include "metadata.h"


// MetadataController implementation
MetadataController::MetadataController(
    MetadataInterface *mV) : metadataView(mV)
{
    metadataManager = std::make_shared<MetadataManager>();
}

void MetadataController::setMetadataView(MetadataInterface *view)
{
    metadataView = view;
}

void MetadataController::preloadMetadata(std::vector<std::shared_ptr<MediaFileModel>> mediaFiles)
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    for (auto &file : mediaFiles)
    {
        metadataManager->loadMetadata(file);
    }
}

void MetadataController::loadMetadata(std::shared_ptr<MediaFileModel> file)
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    metadataManager->loadMetadata(file);
    currentMedia = file;
    // Load metadata
    originalMetadata = currentMedia->getAllMetadata();
    editedMetadata = originalMetadata; // Make a copy for editing

    updateMetadataView();
}

bool MetadataController::saveMetadata()
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    if (!currentMedia)
        return false;

    // Apply edited metadata to the media file
    for (const auto &[key, value] : editedMetadata)
    {
        currentMedia->setMetadata(key, value);
    }

    // Save changes to file
    bool success = metadataManager->saveMetadata(currentMedia);

    if (success)
    {
        // Update original metadata with saved changes
        originalMetadata = editedMetadata;
    }

    updateMetadataView();
    return success;
}

void MetadataController::discardChanges()
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    // Restore original metadata
    editedMetadata = originalMetadata;
    updateMetadataView();
}

void MetadataController::updateField(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    // Update the field in edited metadata
    editedMetadata[key] = value;
}

void MetadataController::addNewField(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    if (!key.empty())
    {
        editedMetadata[key] = value;
    }
}

void MetadataController::removeField(const std::string &key)
{
    std::lock_guard<std::mutex> lock(metadataMutex);

    auto it = editedMetadata.find(key);
    if (it != editedMetadata.end())
    {
        editedMetadata.erase(it);
    }
}

void MetadataController::updateMetadataView()
{
    if (metadataView && currentMedia.get())
    {
        metadataView->showMetadata(currentMedia->getAllMetadata());
    }
}

void MetadataController::enterEditMode()
{
    // This would set the view to edit mode
    if (metadataView)
    {
        // metadataView->enterEditMode();
    }
}

void MetadataController::exitEditMode()
{
    // This would exit edit mode in the view
    if (metadataView)
    {
        // metadataView->exitEditMode();
    }
}

const std::map<std::string, std::string> &MetadataController::getMetadata() const
{
    return editedMetadata;
}
