#ifndef METADATA_CONTROLLER_H
#define METADATA_CONTROLLER_H

#include "View/Interface/Iview.h"
#include "Model/playlist.h"
#include "Model/manager.h"

// Controller for metadata operations
class MetadataController
{
private:
    // Model references
    std::shared_ptr<class MetadataManager> metadataManager;

    // Current state
    std::shared_ptr<class MediaFileModel> currentMedia;
    std::map<std::string, std::string> originalMetadata;
    std::map<std::string, std::string> editedMetadata;

    // View interface
    MetadataInterface *metadataView;

    // Mutex for thread safety
    std::mutex metadataMutex;

public:
    MetadataController(MetadataInterface *mV);

    // View setter
    void setMetadataView(MetadataInterface *view);

    // Metadata operations
    const std::map<std::string, std::string> &getMetadata() const;
    void preloadMetadata(std::vector<std::shared_ptr<MediaFileModel>> mediaFiles);
    void loadMetadata(std::shared_ptr<class MediaFileModel> file);
    bool saveMetadata();
    void discardChanges();

    // Field operations
    void updateField(const std::string &key, const std::string &value);
    void addNewField(const std::string &key, const std::string &value);
    void removeField(const std::string &key);

    // View updates
    void updateMetadataView();
    void enterEditMode();
    void exitEditMode();

    // Accessors
};


#endif