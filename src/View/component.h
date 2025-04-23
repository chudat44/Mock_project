#ifndef COMPONENT_H
#define COMPONENT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <string>
#include <vector>
#include <functional>

const SDL_Color TEXT_COLOR = {220, 220, 220, 255};
const SDL_Color BACKGROUND_COLOR = {25, 25, 25, 255};
const SDL_Color PANEL_COLOR = {35, 35, 35, 255};
const SDL_Color BUTTON_COLOR = {45, 45, 45, 255};
const SDL_Color BUTTON_HOVER_COLOR = {60, 60, 60, 255};
const SDL_Color PROGRESS_COLOR = {0, 162, 232, 255};
const SDL_Color BORDER_COLOR = {80, 80, 80, 255};
const SDL_Color HIGHLIGHT_COLOR = {70, 130, 180, 255};

class FontManager
{
public:
    static TTF_Font *GetDefaultFont()
    {
        static TTF_Font *font = TTF_OpenFont("assets/Arial-Unicode.ttf", 16);
        return font;
    }
};

// Base GUI Component Classes
class UIComponent
{
protected:
    SDL_Rect bounds;
    bool visible;
    bool enabled;

public:
    UIComponent(int x, int y, int w, int h);
    virtual ~UIComponent() = default;

    virtual void render(SDL_Renderer *renderer) = 0;
    virtual bool handleEvent(SDL_Event *event) = 0;

    void setVisible(bool isVisible);
    bool isVisible() const;
    void setEnabled(bool isEnabled);
    bool isEnabled() const;
    void setBounds(int x, int y, int w, int h);
    SDL_Rect getBounds() const;
    bool containsPoint(int x, int y) const;
};

class TextComponent : public UIComponent
{
public:
    enum class TextAlign
    {
        Left,
        Center,
        Right
    };

private:
    std::string text;
    SDL_Color textColor = TEXT_COLOR;
    TextAlign alignment;
    int nLines;
    std::vector<std::string> lines;

    std::vector<std::string> wrapText(const std::string &text, TTF_Font *font, int maxWidth);

public:
    TextComponent(int x, int y, int w, int h, const std::string &labelText);
    ~TextComponent();

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    virtual void setText(const std::string &labelText);
    void setAlign(TextAlign align);
    void setLines(int nLines);
    std::string getText() const;
    void setTextColor(SDL_Color color);
};

class ProgressBar : public UIComponent
{
private:
    float value; // 0.0 to 1.0
    SDL_Color fillColor;
    SDL_Color backgroundColor;
    bool isDraggable;
    std::function<void(float)> onValueChanged;

public:
    ProgressBar(int x, int y, int w, int h);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void setValue(float newValue);
    float getValue() const;
    void setIsDraggable(bool draggable);
    void setOnValueChanged(std::function<void(float)> callback);
    void setColors(SDL_Color fill, SDL_Color background);
};

class VolumeSlider : public UIComponent
{
private:
    int volume; // 0-100
    SDL_Color fillColor;
    SDL_Color backgroundColor;
    SDL_Color knobColor;
    bool isDragging;
    std::function<void(int)> onVolumeChanged;

public:
    VolumeSlider(int x, int y, int w, int h);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void setVolume(int newVolume);
    int getVolume() const;
    void setOnVolumeChanged(std::function<void(int)> callback);
};

class Button : public TextComponent
{
private:
    SDL_Color backgroundColor;
    SDL_Color hoverColor;
    bool isHovered;
    std::function<void()> onClick;

public:
    Button(int x, int y, int w, int h, const std::string &buttonText);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void setOnClick(std::function<void()> callback);
    void setColors(SDL_Color text, SDL_Color background, SDL_Color hover);
};

class ListView : public UIComponent
{
private:
    std::vector<std::string> items;
    int selectedIndex;
    int firstVisibleIndex;
    int visibleItemCount;
    bool hasScrollbar;
    std::function<void(int)> onSelectionChanged;
    std::function<void(int)> on2ClickSelectionChanged;

public:
    ListView(int x, int y, int w, int h);

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void addItem(const std::string &item);
    void removeItem(int index);
    void clearItems();
    size_t size() const;
    std::string getItem(int index) const;
    const std::vector<std::string> &getItems() const;
    void setItems(const std::vector<std::string> &newItems);
    int getSelectedIndex() const;
    void setSelectedIndex(int index);
    void set2ClickSelectedIndex(int index);
    void setOnSelectionChanged(std::function<void(int)> callback);
    void setOn2ClickSelectionChanged(std::function<void(int)> callback);
    void scroll(int amount);
};


class TextField : public TextComponent
{
private:
    std::string text;
    std::string placeholder;
    SDL_Color backgroundColor;
    SDL_Color borderColor;
    bool isFocused;
    int cursorPosition;
    std::function<void(const std::string &)> onTextChanged;

public:
    TextField(int x, int y, int w, int h, const std::string &initialText = "");

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void setText(const std::string &newText);
    void setPlaceholder(const std::string &placeholderText);
    void setOnTextChanged(std::function<void(const std::string &)> callback);
    void focus();
    void unfocus();
};
class Pagination : public UIComponent
{
private:
    int currentPage;
    int totalPages;
    Button *prevButton;
    Button *nextButton;
    TextField *pageField;
    TextComponent *pageLabel;
    std::function<void(int)> onPageChanged;

public:
    Pagination(int x, int y, int w, int h);
    ~Pagination();

    void render(SDL_Renderer *renderer) override;
    bool handleEvent(SDL_Event *event) override;

    void setCurrentPage(int page);
    int getCurrentPage() const;
    void setTotalPages(int pages);
    int getTotalPages();
    void setOnPageChanged(std::function<void(int)> callback);

    void updateButtonStates();
};

#endif
