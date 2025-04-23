#ifndef METADATA_VIEW_H
#define METADATA_VIEW_H

#include "component.h"
#include "Controller/metadata.h"

class MetadataView : public View, public MetadataInterface
{
private:
    std::vector<TextComponent *> keyLabels;
    std::vector<TextField *> valueFields;
    Button *addFieldButton;
    Button *removeFieldButton;
    Button *saveButton;
    Button *cancelButton;
    Button *editButton;
    MediaFileModel *currentFile;
    bool isEditing;

    MetadataController *controller;

public:
    MetadataView(MetadataController *controller);
    ~MetadataView();

    void setMetadataController(MetadataController *controller);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;
    void update() override;

    void showMetadata(const std::map<std::string, std::string> &metadata);
    void enterEditMode();
    void saveChanges();
    void cancelChanges();
    void addField();
    void removeSelectedField();
};

#endif