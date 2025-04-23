#include "metadata.h"


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
        TextComponent *keyLabel = new TextComponent(765, yPos, 60, 30, "Title");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(850, yPos, 125, 30, metadata.at("Title"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(765, yPos, 60, 30, "Artist");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(850, yPos, 125, 30, metadata.at("Artist"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(765, yPos, 60, 30, "Album");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(850, yPos, 125, 30, metadata.at("Album"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(765, yPos, 60, 30, "Comment");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(850, yPos, 125, 30, metadata.at("Comment"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(765, yPos, 60, 30, "Genre");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(850, yPos, 125, 30, metadata.at("Genre"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(765, yPos, 60, 30, "Year");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(850, yPos, 125, 30, metadata.at("Year"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(765, yPos, 60, 30, "Track");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(850, yPos, 125, 30, metadata.at("Track"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(765, yPos, 60, 30, "Bitrate");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(850, yPos, 125, 30, metadata.at("Bitrate"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(765, yPos, 60, 30, "Channels");
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(850, yPos, 125, 30, metadata.at("Channels"));
        valueField->setEnabled(false);
        addComponent(valueField);
        valueFields.push_back(valueField);
        yPos += 35;
    }
    {
        TextComponent *keyLabel = new TextComponent(765, yPos-5, 60, 30, "Sample Rate");
        keyLabel->setLines(2);
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(850, yPos, 125, 30, metadata.at("Sample Rate"));
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
