#include "view.h"

// UIComponent definitions
UIComponent::UIComponent(int x, int y, int w, int h) : visible(true), enabled(true)
{
    bounds = {x, y, w, h};
}

void UIComponent::setVisible(bool isVisible)
{
    visible = isVisible;
}

bool UIComponent::isVisible() const
{
    return visible;
}

void UIComponent::setEnabled(bool isEnabled)
{
    enabled = isEnabled;
}

bool UIComponent::isEnabled() const
{
    return enabled;
}

void UIComponent::setBounds(int x, int y, int w, int h)
{
    bounds = {x, y, w, h};
}

SDL_Rect UIComponent::getBounds() const
{
    return bounds;
}

bool UIComponent::containsPoint(int x, int y) const
{
    return (x >= bounds.x && x < bounds.x + bounds.w &&
            y >= bounds.y && y < bounds.y + bounds.h);
}

// Button definitions
Button::Button(int x, int y, int w, int h, const std::string &buttonText)
    : UIComponent(x, y, w, h), text(buttonText), isHovered(false)
{
    textColor = {255, 255, 255, 255};
    backgroundColor = {100, 100, 100, 255};
    hoverColor = {150, 150, 150, 255};
}

void Button::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    // Draw background
    SDL_SetRenderDrawColor(renderer,
                           isHovered ? hoverColor.r : backgroundColor.r,
                           isHovered ? hoverColor.g : backgroundColor.g,
                           isHovered ? hoverColor.b : backgroundColor.b,
                           isHovered ? hoverColor.a : backgroundColor.a);
    SDL_RenderFillRect(renderer, &bounds);

    // Draw border
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &bounds);

    // Draw text
    if (!text.empty())
    {
        TTF_Font *font = TTF_OpenFont("fonts/default.ttf", 16);
        if (font)
        {
            SDL_Surface *textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
            if (textSurface)
            {
                SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture)
                {
                    SDL_Rect textRect = {
                        bounds.x + (bounds.w - textSurface->w) / 2,
                        bounds.y + (bounds.h - textSurface->h) / 2,
                        textSurface->w,
                        textSurface->h};
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
            TTF_CloseFont(font);
        }
    }
}

bool Button::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled)
        return false;

    if (event->type == SDL_MOUSEMOTION)
    {
        isHovered = containsPoint(event->motion.x, event->motion.y);
    }
    else if (event->type == SDL_MOUSEBUTTONDOWN)
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            if (containsPoint(event->button.x, event->button.y))
            {
                if (onClick)
                    onClick();
                return true;
            }
        }
    }
    return false;
}

void Button::setText(const std::string &buttonText)
{
    text = buttonText;
}

void Button::setOnClick(std::function<void()> callback)
{
    onClick = callback;
}

void Button::setColors(SDL_Color text, SDL_Color background, SDL_Color hover)
{
    textColor = text;
    backgroundColor = background;
    hoverColor = hover;
}

// Label definitions
Label::Label(int x, int y, int w, int h, const std::string &labelText)
    : UIComponent(x, y, w, h), text(labelText), isMultiline(false), font(nullptr)
{
    textColor = {255, 255, 255, 255};
}

Label::~Label()
{
    if (font)
    {
        TTF_CloseFont(font);
    }
}

void Label::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    if (text.empty())
        return;

    // Use a default font if none is set
    TTF_Font *fontToUse = font;
    if (!fontToUse)
    {
        fontToUse = TTF_OpenFont("fonts/default.ttf", 16);
    }

    if (fontToUse)
    {
        if (isMultiline)
        {
            // Handle multiline text
            size_t pos = 0;
            int y = bounds.y;
            while (pos < text.length())
            {
                size_t newlinePos = text.find('\n', pos);
                std::string line;
                if (newlinePos != std::string::npos)
                {
                    line = text.substr(pos, newlinePos - pos);
                    pos = newlinePos + 1;
                }
                else
                {
                    line = text.substr(pos);
                    pos = text.length();
                }

                SDL_Surface *textSurface = TTF_RenderText_Blended(fontToUse, line.c_str(), textColor);
                if (textSurface)
                {
                    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    if (textTexture)
                    {
                        SDL_Rect textRect = {
                            bounds.x,
                            y,
                            textSurface->w,
                            textSurface->h};
                        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                        SDL_DestroyTexture(textTexture);
                    }
                    y += textSurface->h;
                    SDL_FreeSurface(textSurface);
                }
            }
        }
        else
        {
            // Single line text
            SDL_Surface *textSurface = TTF_RenderText_Blended(fontToUse, text.c_str(), textColor);
            if (textSurface)
            {
                SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture)
                {
                    SDL_Rect textRect = {
                        bounds.x,
                        bounds.y,
                        textSurface->w,
                        textSurface->h};
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        }

        if (!font)
        {
            TTF_CloseFont(fontToUse);
        }
    }
}

bool Label::handleEvent(SDL_Event *event)
{
    // Labels typically don't respond to events
    return false;
}

void Label::setText(const std::string &labelText)
{
    text = labelText;
}

std::string Label::getText() const
{
    return text;
}

void Label::setTextColor(SDL_Color color)
{
    textColor = color;
}

void Label::setFont(TTF_Font *newFont)
{
    if (font && font != newFont)
    {
        TTF_CloseFont(font);
    }
    font = newFont;
}

// ProgressBar definitions
ProgressBar::ProgressBar(int x, int y, int w, int h)
    : UIComponent(x, y, w, h), value(0.0f), isDraggable(false)
{
    fillColor = {0, 128, 255, 255};
    backgroundColor = {40, 40, 40, 255};
}

void ProgressBar::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    // Draw background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &bounds);

    // Draw filled portion
    SDL_Rect fillRect = {
        bounds.x,
        bounds.y,
        static_cast<int>(bounds.w * value),
        bounds.h};
    SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    SDL_RenderFillRect(renderer, &fillRect);

    // Draw border
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &bounds);
}

bool ProgressBar::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled || !isDraggable)
        return false;

    if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEMOTION)
    {
        if (event->type == SDL_MOUSEMOTION && !(event->motion.state & SDL_BUTTON_LMASK))
        {
            return false;
        }

        if (containsPoint(event->button.x, event->button.y) ||
            (event->type == SDL_MOUSEMOTION && (event->motion.state & SDL_BUTTON_LMASK)))
        {
            float newValue = static_cast<float>(event->button.x - bounds.x) / bounds.w;
            newValue = std::max(0.0f, std::min(1.0f, newValue));
            setValue(newValue);
            return true;
        }
    }
    return false;
}

void ProgressBar::setValue(float newValue)
{
    value = std::max(0.0f, std::min(1.0f, newValue));
    if (onValueChanged)
    {
        onValueChanged(value);
    }
}

float ProgressBar::getValue() const
{
    return value;
}

void ProgressBar::setIsDraggable(bool draggable)
{
    isDraggable = draggable;
}

void ProgressBar::setOnValueChanged(std::function<void(float)> callback)
{
    onValueChanged = callback;
}

void ProgressBar::setColors(SDL_Color fill, SDL_Color background)
{
    fillColor = fill;
    backgroundColor = background;
}

// ListView definitions
ListView::ListView(int x, int y, int w, int h)
    : UIComponent(x, y, w, h), selectedIndex(-1), firstVisibleIndex(0), visibleItemCount(10), hasScrollbar(true)
{
}

void ListView::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    // Draw background
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer, &bounds);

    // Draw border
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderDrawRect(renderer, &bounds);

    // Calculate item height based on visible count
    int itemHeight = bounds.h / visibleItemCount;

    // Draw visible items
    TTF_Font *font = TTF_OpenFont("fonts/default.ttf", 14);
    if (font)
    {
        int endIndex = std::min(firstVisibleIndex + visibleItemCount, static_cast<int>(items.size()));
        for (int i = firstVisibleIndex; i < endIndex; i++)
        {
            SDL_Rect itemRect = {
                bounds.x,
                bounds.y + (i - firstVisibleIndex) * itemHeight,
                bounds.w - (hasScrollbar ? 20 : 0),
                itemHeight};

            // Draw selected item highlight
            if (i == selectedIndex)
            {
                SDL_SetRenderDrawColor(renderer, 60, 100, 160, 255);
                SDL_RenderFillRect(renderer, &itemRect);
            }

            // Draw item text
            SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface *textSurface = TTF_RenderText_Blended(font, items[i].c_str(), textColor);
            if (textSurface)
            {
                SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture)
                {
                    SDL_Rect textRect = {
                        itemRect.x + 5,
                        itemRect.y + (itemHeight - textSurface->h) / 2,
                        textSurface->w,
                        textSurface->h};
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }

            // Draw item separator
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            SDL_RenderDrawLine(renderer, itemRect.x, itemRect.y + itemHeight - 1,
                               itemRect.x + itemRect.w, itemRect.y + itemHeight - 1);
        }
        TTF_CloseFont(font);

        // Draw scrollbar if needed
        if (hasScrollbar && items.size() > visibleItemCount)
        {
            SDL_Rect scrollbarBg = {
                bounds.x + bounds.w - 20,
                bounds.y,
                20,
                bounds.h};
            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            SDL_RenderFillRect(renderer, &scrollbarBg);

            // Calculate scrollbar handle size and position
            float handleRatio = static_cast<float>(visibleItemCount) / items.size();
            int handleHeight = std::max(20, static_cast<int>(bounds.h * handleRatio));
            int handleY = bounds.y + (bounds.h - handleHeight) *
                                         (static_cast<float>(firstVisibleIndex) / (items.size() - visibleItemCount));

            SDL_Rect scrollHandle = {
                bounds.x + bounds.w - 18,
                handleY,
                16,
                handleHeight};
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
            SDL_RenderFillRect(renderer, &scrollHandle);
        }
    }
}

bool ListView::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled)
        return false;

    if (event->type == SDL_MOUSEBUTTONDOWN)
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            if (containsPoint(event->button.x, event->button.y))
            {
                if (hasScrollbar && event->button.x > bounds.x + bounds.w - 20)
                {
                    // Clicked on scrollbar
                    float clickPosition = static_cast<float>(event->button.y - bounds.y) / bounds.h;
                    int maxFirstIndex = std::max(0, static_cast<int>(items.size()) - visibleItemCount);
                    firstVisibleIndex = static_cast<int>(clickPosition * maxFirstIndex);
                    firstVisibleIndex = std::max(0, std::min(maxFirstIndex, firstVisibleIndex));
                }
                else
                {
                    // Clicked on an item
                    int itemHeight = bounds.h / visibleItemCount;
                    int clickedIndex = firstVisibleIndex + (event->button.y - bounds.y) / itemHeight;

                    if (clickedIndex >= 0 && clickedIndex < static_cast<int>(items.size()))
                    {
                        setSelectedIndex(clickedIndex);
                    }
                }
                return true;
            }
        }
    }
    else if (event->type == SDL_MOUSEWHEEL)
    {
        if (containsPoint(event->wheel.x, event->wheel.y) ||
            SDL_GetMouseFocus() == SDL_GetWindowFromID(event->wheel.windowID))
        {
            scroll(-event->wheel.y * 3); // Scroll 3 items at a time
            return true;
        }
    }
    return false;
}

void ListView::addItem(const std::string &item)
{
    items.push_back(item);
}

void ListView::removeItem(int index)
{
    if (index >= 0 && index < static_cast<int>(items.size()))
    {
        items.erase(items.begin() + index);
        if (selectedIndex >= static_cast<int>(items.size()))
        {
            selectedIndex = items.empty() ? -1 : static_cast<int>(items.size()) - 1;
        }
    }
}

void ListView::clearItems()
{
    items.clear();
    selectedIndex = -1;
    firstVisibleIndex = 0;
}

size_t ListView::size() const
{
    return items.size();
}

void ListView::setItems(const std::vector<std::string> &newItems)
{
    items = newItems;
    selectedIndex = items.empty() ? -1 : 0;
    firstVisibleIndex = 0;
}

std::string ListView::getItem(int index) const
{
    if (index >= 0 && index < static_cast<int>(items.size()))
    {
        return items[index];
    }
    return "";
}
const std::vector<std::string>& ListView::getItems() const
{
    return items;
}
int ListView::getSelectedIndex() const
{
    return selectedIndex;
}

void ListView::setSelectedIndex(int index)
{
    if (index >= -1 && index < static_cast<int>(items.size()))
    {
        if (selectedIndex != index)
        {
            selectedIndex = index;
            if (onSelectionChanged)
            {
                onSelectionChanged(selectedIndex);
            }

            // Auto-scroll to make selection visible if needed
            if (selectedIndex < firstVisibleIndex)
            {
                firstVisibleIndex = selectedIndex;
            }
            else if (selectedIndex >= firstVisibleIndex + visibleItemCount)
            {
                firstVisibleIndex = selectedIndex - visibleItemCount + 1;
            }
        }
    }
}

void ListView::setOnSelectionChanged(std::function<void(int)> callback)
{
    onSelectionChanged = callback;
}

void ListView::scroll(int amount)
{
    int maxFirstIndex = std::max(0, static_cast<int>(items.size()) - visibleItemCount);
    firstVisibleIndex = std::max(0, std::min(maxFirstIndex, firstVisibleIndex + amount));
}

// TextField definitions
TextField::TextField(int x, int y, int w, int h, const std::string &initialText)
    : UIComponent(x, y, w, h), text(initialText), placeholder("Enter text..."),
      isFocused(false), cursorPosition(initialText.length())
{
    textColor = {255, 255, 255, 255};
    backgroundColor = {40, 40, 40, 255};
    borderColor = {150, 150, 150, 255};
}

void TextField::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    // Draw background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &bounds);

    // Draw border (highlighted if focused)
    if (isFocused)
    {
        SDL_SetRenderDrawColor(renderer, 0, 120, 215, 255);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    }
    SDL_RenderDrawRect(renderer, &bounds);

    // Draw text or placeholder
    TTF_Font *font = TTF_OpenFont("fonts/default.ttf", 16);
    if (font)
    {
        std::string displayText = text.empty() && !isFocused ? placeholder : text;
        SDL_Color displayColor = text.empty() && !isFocused ? SDL_Color{150, 150, 150, 255} : textColor;

        SDL_Surface *textSurface = TTF_RenderText_Blended(font, displayText.c_str(), displayColor);
        if (textSurface)
        {
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture)
            {
                SDL_Rect textRect = {
                    bounds.x + 5, // Padding
                    bounds.y + (bounds.h - textSurface->h) / 2,
                    textSurface->w,
                    textSurface->h};
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

                // Draw cursor if focused
                if (isFocused)
                {
                    std::string textBeforeCursor = displayText.substr(0, cursorPosition);
                    int cursorX = bounds.x + 5;

                    if (!textBeforeCursor.empty())
                    {
                        SDL_Surface *cursorPosSurface = TTF_RenderText_Blended(font, textBeforeCursor.c_str(), textColor);
                        if (cursorPosSurface)
                        {
                            cursorX += cursorPosSurface->w;
                            SDL_FreeSurface(cursorPosSurface);
                        }
                    }

                    // Animate cursor blink
                    Uint32 ticks = SDL_GetTicks();
                    if ((ticks / 500) % 2 == 0)
                    {
                        SDL_SetRenderDrawColor(renderer, textColor.r, textColor.g, textColor.b, textColor.a);
                        SDL_RenderDrawLine(renderer, cursorX, bounds.y + 5, cursorX, bounds.y + bounds.h - 5);
                    }
                }

                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
        TTF_CloseFont(font);
    }
}

bool TextField::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled)
        return false;

    if (event->type == SDL_MOUSEBUTTONDOWN)
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            bool wasClicked = containsPoint(event->button.x, event->button.y);

            if (wasClicked)
            {
                focus();

                // Set cursor position based on click position (approximate)
                if (!text.empty())
                {
                    TTF_Font *font = TTF_OpenFont("fonts/default.ttf", 16);
                    if (font)
                    {
                        int clickX = event->button.x - bounds.x - 5; // Adjust for padding
                        int bestDistance = INT_MAX;
                        int bestPos = 0;

                        // Find the closest character position to the click
                        for (size_t i = 0; i <= text.length(); i++)
                        {
                            std::string textPart = text.substr(0, i);
                            SDL_Surface *surface = TTF_RenderText_Blended(font, textPart.c_str(), textColor);
                            if (surface)
                            {
                                int distance = abs(surface->w - clickX);
                                if (distance < bestDistance)
                                {
                                    bestDistance = distance;
                                    bestPos = i;
                                }
                                SDL_FreeSurface(surface);
                            }
                        }

                        cursorPosition = bestPos;
                        TTF_CloseFont(font);
                    }
                }
                else
                {
                    cursorPosition = 0;
                }
            }
            else
            {
                unfocus();
            }

            return wasClicked;
        }
    }
    else if (event->type == SDL_KEYDOWN && isFocused)
    {
        if (event->key.keysym.sym == SDLK_LEFT)
        {
            if (cursorPosition > 0)
            {
                cursorPosition--;
            }
        }
        else if (event->key.keysym.sym == SDLK_RIGHT)
        {
            if (cursorPosition < text.length())
            {
                cursorPosition++;
            }
        }
        else if (event->key.keysym.sym == SDLK_BACKSPACE)
        {
            if (cursorPosition > 0)
            {
                text.erase(cursorPosition - 1, 1);
                cursorPosition--;
                if (onTextChanged)
                {
                    onTextChanged(text);
                }
            }
        }
        else if (event->key.keysym.sym == SDLK_DELETE)
        {
            if (cursorPosition < text.length())
            {
                text.erase(cursorPosition, 1);
                if (onTextChanged)
                {
                    onTextChanged(text);
                }
            }
        }
        else if (event->key.keysym.sym == SDLK_RETURN || event->key.keysym.sym == SDLK_KP_ENTER)
        {
            unfocus();
        }
        else if (event->key.keysym.sym == SDLK_HOME)
        {
            cursorPosition = 0;
        }
        else if (event->key.keysym.sym == SDLK_END)
        {
            cursorPosition = text.length();
        }
        else if (event->key.keysym.sym == SDLK_c && (event->key.keysym.mod & KMOD_CTRL))
        {
            // Ctrl+C: Copy to clipboard
            SDL_SetClipboardText(text.c_str());
        }
        else if (event->key.keysym.sym == SDLK_v && (event->key.keysym.mod & KMOD_CTRL))
        {
            // Ctrl+V: Paste from clipboard
            char *clipboardText = SDL_GetClipboardText();
            if (clipboardText)
            {
                text.insert(cursorPosition, clipboardText);
                cursorPosition += strlen(clipboardText);
                SDL_free(clipboardText);

                if (onTextChanged)
                {
                    onTextChanged(text);
                }
            }
        }
        else if (event->key.keysym.sym == SDLK_a && (event->key.keysym.mod & KMOD_CTRL))
        {
            // Ctrl+A: Select all (move cursor to end)
            cursorPosition = text.length();
        }
        else
        {
            // Handle text input via SDL_TEXTINPUT events instead
        }
        return true;
    }
    else if (event->type == SDL_TEXTINPUT && isFocused)
    {
        // Insert the text at the cursor position
        text.insert(cursorPosition, event->text.text);
        cursorPosition += strlen(event->text.text);

        if (onTextChanged)
        {
            onTextChanged(text);
        }
        return true;
    }

    return false;
}

void TextField::setText(const std::string &newText)
{
    text = newText;
    cursorPosition = text.length();
    if (onTextChanged)
    {
        onTextChanged(text);
    }
}

std::string TextField::getText() const
{
    return text;
}

void TextField::setPlaceholder(const std::string &placeholderText)
{
    placeholder = placeholderText;
}

void TextField::setOnTextChanged(std::function<void(const std::string &)> callback)
{
    onTextChanged = callback;
}

void TextField::focus()
{
    isFocused = true;
    SDL_StartTextInput();
}

void TextField::unfocus()
{
    isFocused = false;
    SDL_StopTextInput();
}

// VolumeSlider definitions
VolumeSlider::VolumeSlider(int x, int y, int w, int h)
    : UIComponent(x, y, w, h), volume(80), isDragging(false)
{
    fillColor = {0, 128, 255, 255};
    backgroundColor = {40, 40, 40, 255};
    knobColor = {200, 200, 200, 255};
}

// Helper function for drawing filled circle in VolumeSlider
void filledCircleRGBA(SDL_Renderer *renderer, int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius))
            {
                SDL_SetRenderDrawColor(renderer, r, g, b, a);
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

void VolumeSlider::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    // Draw background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &bounds);

    // Draw filled portion
    SDL_Rect fillRect = {
        bounds.x,
        bounds.y,
        static_cast<int>(bounds.w * volume / 100),
        bounds.h};
    SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    SDL_RenderFillRect(renderer, &fillRect);

    // Draw knob
    int knobX = bounds.x + static_cast<int>(bounds.w * volume / 100);
    int knobY = bounds.y + bounds.h / 2;
    int knobRadius = bounds.h / 2;

    filledCircleRGBA(renderer, knobX, knobY, knobRadius,
                     knobColor.r, knobColor.g, knobColor.b, knobColor.a);

    // Draw border
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderDrawRect(renderer, &bounds);

    // Draw volume level indicator
    TTF_Font *font = TTF_OpenFont("fonts/default.ttf", 12);
    if (font)
    {
        SDL_Color textColor = {255, 255, 255, 255};
        std::string volumeText = std::to_string(volume) + "%";
        SDL_Surface *textSurface = TTF_RenderText_Blended(font, volumeText.c_str(), textColor);
        if (textSurface)
        {
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture)
            {
                SDL_Rect textRect = {
                    bounds.x + bounds.w + 5,
                    bounds.y + (bounds.h - textSurface->h) / 2,
                    textSurface->w,
                    textSurface->h};
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
        TTF_CloseFont(font);
    }
}

bool VolumeSlider::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled)
        return false;

    if (event->type == SDL_MOUSEBUTTONDOWN)
    {
        if (event->button.button == SDL_BUTTON_LEFT && containsPoint(event->button.x, event->button.y))
        {
            isDragging = true;
            int newVolume = (event->button.x - bounds.x) * 100 / bounds.w;
            setVolume(newVolume);
            return true;
        }
    }
    else if (event->type == SDL_MOUSEBUTTONUP)
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            isDragging = false;
        }
    }
    else if (event->type == SDL_MOUSEMOTION)
    {
        if (isDragging)
        {
            int newVolume = (event->motion.x - bounds.x) * 100 / bounds.w;
            setVolume(newVolume);
            return true;
        }
    }
    return false;
}

void VolumeSlider::setVolume(int newVolume)
{
    volume = std::max(0, std::min(100, newVolume));
    if (onVolumeChanged)
    {
        onVolumeChanged(volume);
    }
}

int VolumeSlider::getVolume() const
{
    return volume;
}

void VolumeSlider::setOnVolumeChanged(std::function<void(int)> callback)
{
    onVolumeChanged = callback;
}


// Pagination definitions
Pagination::Pagination(int x, int y, int w, int h) : UIComponent(x, y, w, h), currentPage(0), totalPages(1)
{
    int buttonWidth = 80;
    int labelWidth = w - 2 * buttonWidth;

    prevButton = new Button(x, y, buttonWidth, h, "Previous");
    nextButton = new Button(x + w - buttonWidth, y, buttonWidth, h, "Next");
    pageLabel = new Label(x + buttonWidth, y, labelWidth, h, "Page 1 of 1");

    // Center the text in the label
    pageLabel->setTextColor({255, 255, 255, 255});

    // Set button callbacks
    prevButton->setOnClick([this]()
                           {
        if (currentPage > 0) {
            setCurrentPage(currentPage - 1);
        } });

    nextButton->setOnClick([this]()
                           {
        if (currentPage < totalPages - 1) {
            setCurrentPage(currentPage + 1);
        } });

    // Update button states
    updateButtonStates();
}

Pagination::~Pagination()
{
    delete prevButton;
    delete nextButton;
    delete pageLabel;
}

void Pagination::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    prevButton->render(renderer);
    nextButton->render(renderer);
    pageLabel->render(renderer);
}

bool Pagination::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled)
        return false;

    return prevButton->handleEvent(event) ||
           nextButton->handleEvent(event) ||
           pageLabel->handleEvent(event);
}

void Pagination::setCurrentPage(int page)
{
    if (page >= 0 && page < totalPages && page != currentPage)
    {
        currentPage = page;
        updateButtonStates();

        if (onPageChanged)
        {
            onPageChanged(currentPage);
        }
    }
}

int Pagination::getCurrentPage() const
{
    return currentPage;
}

void Pagination::setTotalPages(int pages)
{
    totalPages = std::max(1, pages);

    if (currentPage >= totalPages)
    {
        currentPage = totalPages - 1;
        if (onPageChanged)
        {
            onPageChanged(currentPage);
        }
    }

    updateButtonStates();
}

void Pagination::setOnPageChanged(std::function<void(int)> callback)
{
    onPageChanged = callback;
}

void Pagination::updateButtonStates()
{
    prevButton->setEnabled(currentPage > 0);
    nextButton->setEnabled(currentPage < totalPages - 1);

    std::string pageText = "Page " + std::to_string(currentPage + 1) + " of " + std::to_string(totalPages);
    pageLabel->setText(pageText);
}

// Base View class definitions
View::View(ApplicationController *appController) : active(false), controller(appController)
{
}

View::~View()
{
    for (UIComponent *component : components)
    {
        delete component;
    }
    components.clear();
}

void View::render(SDL_Renderer *renderer)
{
    if (!active)
        return;

    // Draw background
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    // Render all components
    for (UIComponent *component : components)
    {
        if (component->isVisible())
        {
            component->render(renderer);
        }
    }
}

bool View::handleEvent(SDL_Event *event)
{
    if (!active)
        return false;

    // Pass event to all components in reverse order (top-most first)
    for (auto it = components.rbegin(); it != components.rend(); ++it)
    {
        UIComponent *component = *it;
        if (component->isVisible() && component->isEnabled())
        {
            if (component->handleEvent(event))
            {
                return true;
            }
        }
    }

    return false;
}

void View::show()
{
    active = true;
    update();
}

void View::hide()
{
    active = false;
}

void View::addComponent(UIComponent *component)
{
    components.push_back(component);
}

void View::removeComponent(UIComponent *component)
{
    auto it = std::find(components.begin(), components.end(), component);
    if (it != components.end())
    {
        components.erase(it);
    }
}

bool View::isActive() const
{
    return active;
}

// MediaListView definitions
MediaListView::MediaListView(ApplicationController *controller)
    : View(controller), itemsPerPage(25)
{

    // Create file list view
    fileListView = new ListView(20, 60, 760, 400);
    fileListView->setOnSelectionChanged([this](int index)
                                        { onFileSelected(index); });

    // Create pagination
    pagination = new Pagination(250, 470, 300, 30);
    pagination->setOnPageChanged([this](int page)
                                 { setCurrentPage(page); });

    // Create buttons
    playButton = new Button(20, 520, 150, 40, "Play");
    playButton->setOnClick([this, &controller]()
                           {
        int selectedIndex = fileListView->getSelectedIndex();
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(currentFiles.size())) {
            // Play the selected file using the controller
            auto mediaController = controller->getPlayerController();
            if (mediaController) {
                mediaController->playMedia(std::make_shared<MediaFileModel>(currentFiles[selectedIndex]));
            }
        } });

    addToPlaylistButton = new Button(190, 520, 150, 40, "Add to Playlist");
    addToPlaylistButton->setOnClick([this, &controller]()
                                    {
        int selectedIndex = fileListView->getSelectedIndex();
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(currentFiles.size())) {
            // Show context menu to select playlist
            // For simplicity, we'll just add to the current playlist
            auto playlistController = controller->getPlaylistController();
            if (playlistController) {
                playlistController->addToCurrentPlaylist(std::make_shared<MediaFileModel>(currentFiles[selectedIndex]));
            }
        } });

    viewMetadataButton = new Button(360, 520, 150, 40, "View Metadata");
    viewMetadataButton->setOnClick([this, controller]()
                                   {
        int selectedIndex = fileListView->getSelectedIndex();
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(currentFiles.size())) {
            // Show metadata view for the selected file
            auto metadataController = controller->getMetadataController();
            if (metadataController) {
                metadataController->loadMetadata(std::make_shared<MediaFileModel>(currentFiles[selectedIndex]));
                // Navigate to metadata view
                // This would be handled by the controller
            }
        } });

    // Create search field
    searchField = new TextField(520, 520, 260, 40, "");
    searchField->setPlaceholder("Search media...");
    searchField->setOnTextChanged([this, &controller](const std::string &text)
                                  {
        if (text.empty()) {
            // Reset to show all files
            update();
        } else {
            // Search and update the list
            auto searchResults = controller->searchMedia(text);
            std::vector<MediaFileModel> results;
            for (auto &file : searchResults) {
                results.push_back(*file);
            }
            updateFileList(results);
        } });

    // Add all components
    addComponent(fileListView);
    addComponent(pagination);
    addComponent(playButton);
    addComponent(addToPlaylistButton);
    addComponent(viewMetadataButton);
    addComponent(searchField);

    // Create a title label
    Label *titleLabel = new Label(20, 20, 760, 30, "Media Library");
    titleLabel->setTextColor({255, 255, 255, 255});
    addComponent(titleLabel);
}

MediaListView::~MediaListView()
{
    // Base class destructor will delete the components
}

void MediaListView::render(SDL_Renderer *renderer)
{
    View::render(renderer);

    // Additional custom rendering if needed
}

bool MediaListView::handleEvent(SDL_Event *event)
{
    return View::handleEvent(event);
}

void MediaListView::update()
{
    // Get the media files from the library
    auto mediaLibrary = controller->getMediaLibrary();
    if (mediaLibrary)
    {
        std::vector<MediaFileModel> allFiles;
        for (auto &file : mediaLibrary->getMediaFiles())
        {
            allFiles.push_back(*file);
        }
        updateFileList(allFiles);
    }
}

void MediaListView::updateFileList(const std::vector<MediaFileModel> &files)
{
    currentFiles = files;

    // Calculate total pages
    int totalItems = static_cast<int>(files.size());
    int pages = (totalItems + itemsPerPage - 1) / itemsPerPage;
    pagination->setTotalPages(pages);

    // Update the current page
    setCurrentPage(pagination->getCurrentPage());
}

void MediaListView::setCurrentPage(int page)
{
    // Get items for the current page
    int startIdx = page * itemsPerPage;
    int endIdx = std::min(startIdx + itemsPerPage, static_cast<int>(currentFiles.size()));

    // Update the list view
    std::vector<std::string> items;
    for (int i = startIdx; i < endIdx; i++)
    {
        items.push_back(currentFiles[i].getFilename());
    }

    fileListView->setItems(items);
}

int MediaListView::getCurrentPage() const
{
    return pagination->getCurrentPage();
}

int MediaListView::getTotalPages() const
{
    return (static_cast<int>(currentFiles.size()) + itemsPerPage - 1) / itemsPerPage;
}

void MediaListView::onFileSelected(int index)
{
    // Enable buttons when a file is selected
    bool hasSelection = index >= 0;
    playButton->setEnabled(hasSelection);
    addToPlaylistButton->setEnabled(hasSelection);
    viewMetadataButton->setEnabled(hasSelection);
}

void MediaListView::showFileContextMenu(int x, int y, int fileIndex)
{
    // This would typically show a popup menu with options
    // For simplicity, we'll just log the action
    SDL_Log("Context menu for file %d at %d,%d", fileIndex, x, y);
}

// PlaylistView definitions
PlaylistView::PlaylistView(ApplicationController *controller)
    : View(controller), currentPlaylist(nullptr)
{

    // Create playlist name label
    playlistNameLabel = new Label(20, 20, 760, 30, "Playlist: None");
    playlistNameLabel->setTextColor({255, 255, 255, 255});

    // Create playlist content list
    playlistContent = new ListView(20, 60, 760, 400);
    playlistContent->setOnSelectionChanged([this](int index)
                                           {
        // Enable buttons based on selection
        bool hasSelection = index >= 0;
        playButton->setEnabled(hasSelection);
        removeButton->setEnabled(hasSelection);
        moveUpButton->setEnabled(hasSelection > 0);
        moveDownButton->setEnabled(hasSelection >= 0 && index < static_cast<int>(playlistContent->getItems().size()) - 1); });

    // Create buttons
    playButton = new Button(20, 480, 150, 40, "Play");
    playButton->setEnabled(false);
    playButton->setOnClick([this, &controller]()
                           {
        int selectedIndex = playlistContent->getSelectedIndex();
        if (currentPlaylist && selectedIndex >= 0) {
            // Play the selected item and continue with the playlist
            auto playerController = controller->getPlayerController();
            if (playerController) {
                playerController->playPlaylist(std::make_shared<PlaylistModel>(*currentPlaylist), selectedIndex);
            }
        } });

    removeButton = new Button(190, 480, 150, 40, "Remove");
    removeButton->setEnabled(false);
    removeButton->setOnClick([this, &controller]()
                             {
        int selectedIndex = playlistContent->getSelectedIndex();
        if (currentPlaylist && selectedIndex >= 0) {
            // Remove the selected item from the playlist
            auto playlistController = controller->getPlaylistController();
            if (playlistController) {
                playlistController->removeFromCurrentPlaylist(selectedIndex);
                // Update the view after removing
                setCurrentPlaylist(currentPlaylist);
            }
        } });

    moveUpButton = new Button(360, 480, 150, 40, "Move Up");
    moveUpButton->setEnabled(false);
    moveUpButton->setOnClick([this, &controller]()
                             {
        int selectedIndex = playlistContent->getSelectedIndex();
        if (currentPlaylist && selectedIndex > 0) {
            // Move item up in the playlist
            auto playlistController = controller->getPlaylistController();
            if (playlistController) {
                playlistController->moveItemUp(selectedIndex);
                // Update the view after moving
                setCurrentPlaylist(currentPlaylist);
                // Keep the same item selected
                playlistContent->setSelectedIndex(selectedIndex - 1);
            }
        } });

    moveDownButton = new Button(530, 480, 150, 40, "Move Down");
    moveDownButton->setEnabled(false);
    moveDownButton->setOnClick([this, &controller]()
                               {
        int selectedIndex = playlistContent->getSelectedIndex();
        if (currentPlaylist && selectedIndex >= 0 && 
            selectedIndex < static_cast<int>(currentPlaylist->size()) - 1) {
            // Move item down in the playlist
            auto playlistController = controller->getPlaylistController();
            if (playlistController) {
                playlistController->moveItemDown(selectedIndex);
                // Update the view after moving
                setCurrentPlaylist(currentPlaylist);
                // Keep the same item selected
                playlistContent->setSelectedIndex(selectedIndex + 1);
            }
        } });

    saveButton = new Button(360, 540, 150, 40, "Save Playlist");
    saveButton->setOnClick([this, &controller]()
                           {
        if (currentPlaylist) {
            // Save the playlist
            auto playlistController = controller->getPlaylistController();
            if (playlistController) {
                playlistController->saveCurrentPlaylist();
            }
        } });

    // Add all components
    addComponent(playlistNameLabel);
    addComponent(playlistContent);
    addComponent(playButton);
    addComponent(removeButton);
    addComponent(moveUpButton);
    addComponent(moveDownButton);
    addComponent(saveButton);
}

PlaylistView::~PlaylistView()
{
    // Base class destructor will delete components
}

void PlaylistView::render(SDL_Renderer *renderer)
{
    View::render(renderer);

    // Additional custom rendering if needed
}

bool PlaylistView::handleEvent(SDL_Event *event)
{
    return View::handleEvent(event);
}

void PlaylistView::update()
{
    // This will get called when the view becomes active
    auto playlistController = controller->getPlaylistController();
    if (playlistController)
    {
        // Get the current playlist from the controller
        currentPlaylist = playlistController->getCurrentPlaylist().get();
        setCurrentPlaylist(currentPlaylist);
    }
}

void PlaylistView::setCurrentPlaylist(PlaylistModel *playlist)
{
    currentPlaylist = playlist;

    if (playlist)
    {
        // Update playlist name
        playlistNameLabel->setText("Playlist: " + playlist->getPlaylistName());

        // Update playlist content
        std::vector<std::string> items;
        for (size_t i = 0; i < playlist->size(); i++)
        {
            auto mediaFile = playlist->getMediaFile(i);
            if (mediaFile)
            {
                items.push_back(mediaFile->getFilename());
            }
        }

        playlistContent->setItems(items);
        saveButton->setEnabled(true);
    }
    else
    {
        // No playlist loaded
        playlistNameLabel->setText("Playlist: None");
        playlistContent->clearItems();
        saveButton->setEnabled(false);
    }

    // Reset button states
    playButton->setEnabled(false);
    removeButton->setEnabled(false);
    moveUpButton->setEnabled(false);
    moveDownButton->setEnabled(false);
}

void PlaylistView::highlightPlaying(int index)
{
    // This could be implemented by using a different color for the playing item
    // or by adding a special marker. For simplicity, we'll just select the item.
    playlistContent->setSelectedIndex(index);
}

void PlaylistView::enterEditMode()
{
    // Enable editing controls
    removeButton->setVisible(true);
    moveUpButton->setVisible(true);
    moveDownButton->setVisible(true);
    saveButton->setVisible(true);
}

void PlaylistView::exitEditMode()
{
    // Disable editing controls
    removeButton->setVisible(false);
    moveUpButton->setVisible(false);
    moveDownButton->setVisible(false);
    saveButton->setVisible(false);
}

// PlayerView definitions
PlayerView::PlayerView(ApplicationController *controller)
    : View(controller), currentMedia(nullptr), isPlaying(false)
{

    // Create labels for track info
    currentTrackLabel = new Label(20, 20, 760, 30, "No Track Playing");
    currentTrackLabel->setTextColor({255, 255, 255, 255});

    artistAlbumLabel = new Label(20, 50, 760, 20, "");
    artistAlbumLabel->setTextColor({200, 200, 200, 255});

    timeLabel = new Label(20, 80, 760, 20, "0:00 / 0:00");
    timeLabel->setTextColor({200, 200, 200, 255});

    // Create progress bar
    progressBar = new ProgressBar(20, 110, 760, 15);
    progressBar->setIsDraggable(true);
    progressBar->setOnValueChanged([this, &controller](float value)
                                   {
        // Seek to the position
        auto playerController = controller->getPlayerController();
        if (playerController && currentMedia) {
            int duration = playerController->getDuration();
            playerController->seek(static_cast<int>(value * duration));
        } });

    // Create playback control buttons
    playPauseButton = new Button(300, 140, 100, 40, "Play");
    playPauseButton->setOnClick([this, &controller]()
                                {
        auto playerController = controller->getPlayerController();
        if (playerController) {
            if (playerController->isMediaPlaying()) {
                if (playerController->isMediaPaused()) {
                    playerController->play();
                } else {
                    playerController->pause();
                }
            } else if (currentMedia) {
                playerController->playMedia(std::make_shared<MediaFileModel>(*currentMedia));
            }
        } });

    stopButton = new Button(410, 140, 100, 40, "Stop");
    stopButton->setOnClick([this, &controller]()
                           {
        auto playerController = controller->getPlayerController();
        if (playerController) {
            playerController->stop();
        } });

    previousButton = new Button(190, 140, 100, 40, "Previous");
    previousButton->setOnClick([this, &controller]()
                               {
        auto playerController = controller->getPlayerController();
        if (playerController) {
            playerController->previous();
        } });

    nextButton = new Button(520, 140, 100, 40, "Next");
    nextButton->setOnClick([this, &controller]()
                           {
        auto playerController = controller->getPlayerController();
        if (playerController) {
            playerController->next();
        } });

    // Create volume slider
    volumeSlider = new VolumeSlider(300, 200, 200, 20);
    volumeSlider->setOnVolumeChanged([this, &controller](int volume)
                                     {
        auto playerController = controller->getPlayerController();
        if (playerController) {
            playerController->setVolume(volume);
        } });

    // Add all components
    addComponent(currentTrackLabel);
    addComponent(artistAlbumLabel);
    addComponent(timeLabel);
    addComponent(progressBar);
    addComponent(playPauseButton);
    addComponent(stopButton);
    addComponent(previousButton);
    addComponent(nextButton);
    addComponent(volumeSlider);
}

PlayerView::~PlayerView()
{
    // Base class destructor will delete components
}

void PlayerView::render(SDL_Renderer *renderer)
{
    View::render(renderer);

    // Additional custom rendering if needed
}

bool PlayerView::handleEvent(SDL_Event *event)
{
    return View::handleEvent(event);
}

void PlayerView::update()
{
    auto playerController = controller->getPlayerController();
    if (playerController)
    {
        // Update volume
        volumeSlider->setVolume(playerController->getVolume());

        // Update current media info
        auto media = playerController->getCurrentMedia();
        if (media)
        {
            setCurrentMedia(const_cast<MediaFileModel *>(media.get()));
        }

        // Update playback status
        updatePlaybackStatus(playerController->isMediaPlaying() && !playerController->isMediaPaused());

        // Update progress
        updateProgress(playerController->getCurrentPosition(), playerController->getDuration());
    }
}

void PlayerView::setBounds(int x, int y, int w, int h) {
    // Store the bounds for the view
    viewBounds = {x, y, w, h};
}

void PlayerView::updatePlaybackStatus(bool playing)
{
    isPlaying = playing;
    playPauseButton->setText(isPlaying ? "Pause" : "Play");
}

void PlayerView::setCurrentMedia(MediaFileModel *media)
{
    currentMedia = media;

    if (media)
    {
        // Update track info
        currentTrackLabel->setText(media->getFilename());

        // Try to get artist and album from metadata
        std::string artist = media->getMetadata("Artist");
        std::string album = media->getMetadata("Album");
        std::string artInfo = "";

        if (!artist.empty())
        {
            artInfo += "Artist: " + artist;
        }

        if (!album.empty())
        {
            if (!artInfo.empty())
                artInfo += " | ";
            artInfo += "Album: " + album;
        }

        artistAlbumLabel->setText(artInfo);
    }
    else
    {
        currentTrackLabel->setText("No Track Playing");
        artistAlbumLabel->setText("");
    }
}
// Helper function to format time
std::string formatTime(int seconds)
{
    int minutes = seconds / 60;
    seconds %= 60;

    std::string result = std::to_string(minutes) + ":";
    if (seconds < 10)
        result += "0";
    result += std::to_string(seconds);

    return result;
}

void PlayerView::updateProgress(int currentPosition, int duration)
{
    // Update progress bar
    if (duration > 0)
    {
        progressBar->setValue(static_cast<float>(currentPosition) / duration);
    }
    else
    {
        progressBar->setValue(0);
    }

    // Update time label
    std::string timeText = formatTime(currentPosition) + " / " + formatTime(duration);
    timeLabel->setText(timeText);
}


void PlayerView::updateVolume(int volume)
{
    volumeSlider->setVolume(volume);
}
// MetadataView definitions
MetadataView::MetadataView(ApplicationController *controller)
    : View(controller), currentFile(nullptr), isEditing(false)
{

    // Create title label
    Label *titleLabel = new Label(20, 20, 760, 30, "Media File Metadata");
    titleLabel->setTextColor({255, 255, 255, 255});

    // Create buttons
    saveButton = new Button(580, 520, 100, 40, "Save");
    saveButton->setOnClick([this]()
                           { saveChanges(); });

    cancelButton = new Button(460, 520, 100, 40, "Cancel");
    cancelButton->setOnClick([this]()
                             { cancelChanges(); });

    addFieldButton = new Button(340, 520, 100, 40, "Add Field");
    addFieldButton->setOnClick([this]()
                               { addField(); });

    removeFieldButton = new Button(220, 520, 100, 40, "Remove");
    removeFieldButton->setOnClick([this]()
                                  { removeSelectedField(); });

    // Add components
    addComponent(titleLabel);
    addComponent(saveButton);
    addComponent(cancelButton);
    addComponent(addFieldButton);
    addComponent(removeFieldButton);

    // Initially hide edit buttons
    saveButton->setVisible(false);
    cancelButton->setVisible(false);
    addFieldButton->setVisible(false);
    removeFieldButton->setVisible(false);
}

MetadataView::~MetadataView()
{
    // Clean up key/value fields
    for (auto label : keyLabels)
    {
        delete label;
    }
    keyLabels.clear();

    for (auto field : valueFields)
    {
        delete field;
    }
    valueFields.clear();
}

void MetadataView::render(SDL_Renderer *renderer)
{
    // Set background color
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect fullRect = {0, 0, 800, 600};
    SDL_RenderFillRect(renderer, &fullRect);

    // Render metadata fields and other components
    View::render(renderer);
}

bool MetadataView::handleEvent(SDL_Event *event)
{
    // Handle events for the view components
    return View::handleEvent(event);
}

void MetadataView::update()
{
    // Update components based on controller state
    if (currentFile && controller)
    {
        auto metadataController = controller->getMetadataController();
        if (metadataController)
        {
            // Update the metadata display
            showMetadata(currentFile);
        }
    }
}

void MetadataView::showMetadata(MediaFileModel *file)
{
    currentFile = file;

    // Clear previous fields
    for (auto label : keyLabels)
    {
        removeComponent(label);
        delete label;
    }
    keyLabels.clear();

    for (auto field : valueFields)
    {
        removeComponent(field);
        delete field;
    }
    valueFields.clear();

    if (!file)
        return;

    // Create header for file information
    Label *fileNameLabel = new Label(20, 60, 760, 30, "File: " + file->getFilename());
    fileNameLabel->setTextColor({255, 255, 255, 255});
    addComponent(fileNameLabel);
    keyLabels.push_back(fileNameLabel);

    // Get metadata from file
    const auto &metadata = file->getAllMetadata();
    int yPos = 100;

    // Create key-value pairs
    for (const auto &pair : metadata)
    {
        Label *keyLabel = new Label(20, yPos, 200, 30, pair.first + ":");
        keyLabel->setTextColor({200, 200, 200, 255});
        addComponent(keyLabel);
        keyLabels.push_back(keyLabel);

        TextField *valueField = new TextField(230, yPos, 550, 30, pair.second);
        valueField->setEnabled(isEditing);
        addComponent(valueField);
        valueFields.push_back(valueField);

        yPos += 40;
    }

    // Show/hide edit buttons based on mode
    saveButton->setVisible(isEditing);
    cancelButton->setVisible(isEditing);
    addFieldButton->setVisible(isEditing);
    removeFieldButton->setVisible(isEditing);

    // Add edit button if not in edit mode
    if (!isEditing)
    {
        editButton = new Button(680, 520, 100, 40, "Edit");
        editButton->setOnClick([this]()
                               { enterEditMode(); });
        addComponent(editButton);
    }
}

void MetadataView::enterEditMode()
{
    isEditing = true;

    // Enable editing for all value fields
    for (auto field : valueFields)
    {
        field->setEnabled(true);
    }

    // Update button visibility
    saveButton->setVisible(true);
    cancelButton->setVisible(true);
    addFieldButton->setVisible(true);
    removeFieldButton->setVisible(true);

    // Remove the edit button
    for (size_t i = 0; i < keyLabels.size(); i++)
    {
        if (dynamic_cast<Button *>(keyLabels[i]))
        {
            removeComponent(keyLabels[i]);
            delete keyLabels[i];
            keyLabels.erase(keyLabels.begin() + i);
            break;
        }
    }

    // Notify controller
    if (controller)
    {
        auto metadataController = controller->getMetadataController();
        if (metadataController)
        {
            metadataController->enterEditMode();
        }
    }
}

void MetadataView::saveChanges()
{
    if (!currentFile)
        return;

    // Get current metadata
    const auto &metadata = currentFile->getAllMetadata();
    auto iter = metadata.begin();

    // Update metadata with new values
    for (size_t i = 0; i < valueFields.size() && iter != metadata.end(); i++, iter++)
    {
        currentFile->setMetadata(iter->first, valueFields[i]->getText());
    }

    // Save through controller
    if (controller)
    {
        auto metadataController = controller->getMetadataController();
        if (metadataController)
        {
            metadataController->saveMetadata();
        }
    }

    // Exit edit mode
    isEditing = false;
    showMetadata(currentFile);
}

void MetadataView::cancelChanges()
{
    isEditing = false;

    // Revert to original metadata
    if (controller)
    {
        auto metadataController = controller->getMetadataController();
        if (metadataController)
        {
            metadataController->discardChanges();
        }
    }

    // Refresh display
    showMetadata(currentFile);
}

void MetadataView::addField()
{
    if (!isEditing || !currentFile)
        return;

    // Add new key-value pair to UI
    int yPos = 100 + 40 * keyLabels.size();

    // Use a placeholder key name
    std::string newKey = "New Field";
    int counter = 1;

    // Ensure key is unique
    const auto &metadata = currentFile->getAllMetadata();
    while (metadata.find(newKey) != metadata.end())
    {
        newKey = "New Field " + std::to_string(counter++);
    }

    Label *keyLabel = new Label(20, yPos, 200, 30, newKey + ":");
    keyLabel->setTextColor({200, 200, 200, 255});
    addComponent(keyLabel);
    keyLabels.push_back(keyLabel);

    TextField *valueField = new TextField(230, yPos, 550, 30, "");
    valueField->setEnabled(true);
    addComponent(valueField);
    valueFields.push_back(valueField);

    // Add to metadata
    currentFile->setMetadata(newKey, "");
}

void MetadataView::removeSelectedField()
{
    if (!isEditing || valueFields.empty())
        return;

    // Find focused field
    int selectedIndex = -1;
    for (size_t i = 0; i < valueFields.size(); i++)
    {
        if (valueFields[i]->isEnabled() && valueFields[i]->containsPoint(
                                               SDL_GetMouseState(nullptr, nullptr), SDL_GetMouseState(nullptr, nullptr)))
        {
            selectedIndex = i;
            break;
        }
    }

    if (selectedIndex >= 0)
    {
        // Get the key to remove
        std::string keyToRemove;
        const auto &metadata = currentFile->getAllMetadata();
        auto iter = metadata.begin();
        for (int i = 0; i < selectedIndex && iter != metadata.end(); i++, iter++)
        {
        }

        if (iter != metadata.end())
        {
            keyToRemove = iter->first;

            // Remove from metadata
            if (controller)
            {
                auto metadataController = controller->getMetadataController();
                if (metadataController)
                {
                    metadataController->removeField(keyToRemove);
                }
            }

            // Remove UI elements
            removeComponent(keyLabels[selectedIndex]);
            delete keyLabels[selectedIndex];
            keyLabels.erase(keyLabels.begin() + selectedIndex);

            removeComponent(valueFields[selectedIndex]);
            delete valueFields[selectedIndex];
            valueFields.erase(valueFields.begin() + selectedIndex);

            // Reposition remaining fields
            for (size_t i = selectedIndex; i < keyLabels.size(); i++)
            {
                int yPos = 100 + 40 * i;
                keyLabels[i]->setBounds(20, yPos, 200, 30);
                valueFields[i]->setBounds(230, yPos, 550, 30);
            }
        }
    }
}

// PlaylistsListView definitions
PlaylistsListView::PlaylistsListView(ApplicationController *controller)
    : View(controller)
{

    // Create title label
    Label *titleLabel = new Label(20, 20, 760, 30, "Playlists");
    titleLabel->setTextColor({255, 255, 255, 255});

    // Create playlist list view
    playlistsList = new ListView(20, 70, 760, 400);
    playlistsList->setOnSelectionChanged([this](int index)
                                         {
        // Update button states based on selection
        openPlaylistButton->setEnabled(index >= 0);
        deletePlaylistButton->setEnabled(index >= 0); });

    // Create playlist name field for new playlists
    playlistNameField = new TextField(20, 480, 400, 30, "");
    playlistNameField->setPlaceholder("New playlist name");

    // Create buttons
    newPlaylistButton = new Button(430, 480, 120, 30, "Create Playlist");
    newPlaylistButton->setOnClick([this]()
                                  { createNewPlaylist(); });

    deletePlaylistButton = new Button(560, 480, 100, 30, "Delete");
    deletePlaylistButton->setOnClick([this]()
                                     { deleteSelectedPlaylist(); });
    deletePlaylistButton->setEnabled(false);

    openPlaylistButton = new Button(670, 480, 110, 30, "Open");
    openPlaylistButton->setOnClick([this]()
                                   { openSelectedPlaylist(); });
    openPlaylistButton->setEnabled(false);

    // Add back button
    Button *backButton = new Button(20, 530, 100, 40, "Back");
    backButton->setOnClick([this, &controller]()
                           {
        if (controller) {
            controller->navigateToMainMenu();
        } });

    // Add components
    addComponent(titleLabel);
    addComponent(playlistsList);
    addComponent(playlistNameField);
    addComponent(newPlaylistButton);
    addComponent(deletePlaylistButton);
    addComponent(openPlaylistButton);
    addComponent(backButton);
}

PlaylistsListView::~PlaylistsListView()
{
    // Components will be cleaned up by parent View destructor
}

void PlaylistsListView::render(SDL_Renderer *renderer)
{
    // Set background color
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect fullRect = {0, 0, 800, 600};
    SDL_RenderFillRect(renderer, &fullRect);

    // Render playlists list and other components
    View::render(renderer);
}

bool PlaylistsListView::handleEvent(SDL_Event *event)
{
    // Handle events for the view components
    return View::handleEvent(event);
}

void PlaylistsListView::update()
{
    // Update playlist list from controller
    if (controller)
    {
        auto playlistController = controller->getPlaylistController();
        if (playlistController)
        {
            playlistController->updatePlaylistsListView();
        }
    }
}

void PlaylistsListView::setPlaylists(const std::vector<std::string> &playlistNames)
{
    playlistsList->clearItems();

    for (const auto &name : playlistNames)
    {
        playlistsList->addItem(name);
    }
}

int PlaylistsListView::getSelectedPlaylist() const
{
    return playlistsList->getSelectedIndex();
}

void PlaylistsListView::createNewPlaylist()
{
    std::string name = playlistNameField->getText();
    if (name.empty())
    {
        // Show error or use default name
        name = "New Playlist " + std::to_string(playlistsList->size() + 1);
    }

    if (controller)
    {
        auto playlistController = controller->getPlaylistController();
        if (playlistController)
        {
            playlistController->createPlaylist(name);
            playlistNameField->setText("");

            // Refresh playlists
            update();
        }
    }
}

void PlaylistsListView::deleteSelectedPlaylist()
{
    int index = playlistsList->getSelectedIndex();
    if (index >= 0)
    {
        if (controller)
        {
            auto playlistController = controller->getPlaylistController();
            if (playlistController)
            {
                playlistController->deletePlaylist(index);

                // Refresh playlists
                update();
            }
        }
    }
}

void PlaylistsListView::openSelectedPlaylist()
{
    int index = playlistsList->getSelectedIndex();
    if (index >= 0)
    {
        if (controller)
        {
            auto playlistController = controller->getPlaylistController();
            if (playlistController)
            {
                playlistController->loadPlaylist(index);
                playlistController->viewPlaylistContent(playlistsList->getItem(index));
            }
        }
    }
}

// USBView definitions
USBView::USBView(ApplicationController *controller)
    : View(controller), deviceMounted(false)
{

    // Create title label
    Label *titleLabel = new Label(20, 20, 760, 30, "USB Devices");
    titleLabel->setTextColor({255, 255, 255, 255});

    // Create USB devices list
    usbDevicesList = new ListView(20, 70, 760, 400);
    usbDevicesList->setOnSelectionChanged([this](int index)
                                          {
        // Update button states based on selection and mount status
        if (index >= 0) {
            mountButton->setEnabled(!deviceMounted);
            unmountButton->setEnabled(deviceMounted);
        } else {
            mountButton->setEnabled(false);
            unmountButton->setEnabled(false);
        } });

    // Create status label
    statusLabel = new Label(20, 480, 760, 30, "Select a USB device to mount");
    statusLabel->setTextColor({200, 200, 200, 255});

    // Create buttons
    mountButton = new Button(580, 520, 100, 40, "Mount");
    mountButton->setEnabled(false);
    mountButton->setOnClick([this]()
                            { mountSelectedDevice(); });

    unmountButton = new Button(690, 520, 100, 40, "Unmount");
    unmountButton->setEnabled(false);
    unmountButton->setOnClick([this]()
                              { unmountDevice(); });

    // Add refresh button
    Button *refreshButton = new Button(470, 520, 100, 40, "Refresh");
    refreshButton->setOnClick([this, &controller]()
                              {
        if (controller) {
            auto usbController = controller->getUSBController();
            if (usbController) {
                usbController->detectUSBDevices();
            }
        } });

    // Add back button
    Button *backButton = new Button(20, 520, 100, 40, "Back");
    backButton->setOnClick([this, &controller]()
                           {
        if (controller) {
            controller->navigateToMainMenu();
        } });

    // Add components
    addComponent(titleLabel);
    addComponent(usbDevicesList);
    addComponent(statusLabel);
    addComponent(mountButton);
    addComponent(unmountButton);
    addComponent(refreshButton);
    addComponent(backButton);
}

USBView::~USBView()
{
    // Components will be cleaned up by parent View destructor
}

void USBView::render(SDL_Renderer *renderer)
{
    // Set background color
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect fullRect = {0, 0, 800, 600};
    SDL_RenderFillRect(renderer, &fullRect);

    // Render USB devices list and other components
    View::render(renderer);
}

bool USBView::handleEvent(SDL_Event *event)
{
    // Handle events for the view components
    return View::handleEvent(event);
}

void USBView::update()
{
    // Update from controller
    if (controller)
    {
        auto usbController = controller->getUSBController();
        if (usbController)
        {
            // Refresh USB devices list
            usbController->detectUSBDevices();
        }
    }
}

void USBView::updateDeviceList(const std::vector<std::string> &devices)
{
    usbDevicesList->clearItems();

    if (devices.empty())
    {
        usbDevicesList->addItem("No USB devices detected");
        mountButton->setEnabled(false);
        unmountButton->setEnabled(false);
        statusLabel->setText("No USB devices detected");
    }
    else
    {
        for (const auto &device : devices)
        {
            usbDevicesList->addItem(device);
        }
        statusLabel->setText("Select a USB device to mount");
    }
}

void USBView::setMountStatus(bool isMounted)
{
    deviceMounted = isMounted;

    mountButton->setEnabled(!deviceMounted && usbDevicesList->getSelectedIndex() >= 0);
    unmountButton->setEnabled(deviceMounted);

    if (deviceMounted)
    {
        statusLabel->setText("Device mounted. USB content available for browsing.");
    }
    else
    {
        statusLabel->setText("Device not mounted. Select a USB device to mount.");
    }
}

void USBView::mountSelectedDevice()
{
    int index = usbDevicesList->getSelectedIndex();
    if (index >= 0)
    {
        std::string deviceName = usbDevicesList->getItem(index);

        if (controller)
        {
            auto usbController = controller->getUSBController();
            if (usbController)
            {
                bool success = usbController->mountUSB(deviceName);

                if (success)
                {
                    setMountStatus(true);
                    // Scan USB contents
                    usbController->scanUSBContents(deviceName);
                }
                else
                {
                    statusLabel->setText("Failed to mount USB device");
                }
            }
        }
    }
}

void USBView::unmountDevice()
{
    int index = usbDevicesList->getSelectedIndex();
    if (index >= 0 && deviceMounted)
    {
        std::string deviceName = usbDevicesList->getItem(index);

        if (controller)
        {
            auto usbController = controller->getUSBController();
            if (usbController)
            {
                bool success = usbController->unmountUSB(deviceName);

                if (success)
                {
                    setMountStatus(false);
                }
                else
                {
                    statusLabel->setText("Failed to unmount USB device");
                }
            }
        }
    }
}

// MainWindow definitions
MainWindow::MainWindow(ApplicationController *appController)
    : window(nullptr), renderer(nullptr), currentView(nullptr),
      controller(appController), miniPlayerView(nullptr), showingDialog(false),
      dialogLabel(nullptr), dialogOkButton(nullptr)
{
}

MainWindow::~MainWindow()
{
    cleanup();
}

bool MainWindow::initialize(const std::string &title, int width, int height)
{
    // Initialize SDL subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0)
    {
        return false;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        return false;
    }

    // Create window
    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_SHOWN);

    if (window == nullptr)
    {
        return false;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        return false;
    }

    // Set up mini player (always visible at bottom of screen)
    miniPlayerView = new PlayerView(controller);
    miniPlayerView->setBounds(0, height - 50, width, 50);

    // Set up dialog components
    dialogLabel = new Label(width / 2 - 150, height / 2 - 40, 300, 30, "");
    dialogLabel->setVisible(false);

    dialogOkButton = new Button(width / 2 - 50, height / 2 + 10, 100, 30, "OK");
    dialogOkButton->setVisible(false);
    dialogOkButton->setOnClick([this]()
                               { hideDialog(); });

    return true;
}

void MainWindow::cleanup()
{
    // Clean up views
    for (auto &pair : views)
    {
        delete pair.second;
    }
    views.clear();

    // Clean up mini player
    delete miniPlayerView;
    miniPlayerView = nullptr;

    // Clean up dialog components
    delete dialogLabel;
    dialogLabel = nullptr;

    delete dialogOkButton;
    dialogOkButton = nullptr;

    // Clean up SDL resources
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    // Quit SDL subsystems
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void MainWindow::render()
{
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    // Render current view
    if (currentView)
    {
        currentView->render(renderer);
    }

    // Render mini player
    miniPlayerView->render(renderer);

    // Render dialog if showing
    if (showingDialog)
    {
        // Semi-transparent overlay
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect fullScreen = {0, 0, 800, 600};
        SDL_RenderFillRect(renderer, &fullScreen);

        // Dialog background
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_Rect dialogRect = {300, 200, 200, 100};
        SDL_RenderFillRect(renderer, &dialogRect);

        // Dialog border
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderDrawRect(renderer, &dialogRect);

        // Dialog components
        dialogLabel->render(renderer);
        dialogOkButton->render(renderer);
    }

    // Present rendered content
    SDL_RenderPresent(renderer);
}

void MainWindow::handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            if (controller)
            {
                controller->exit();
            }
        }

        // Handle dialog events first if showing
        if (showingDialog)
        {
            dialogOkButton->handleEvent(&event);
            continue;
        }

        // Handle mini player events
        bool handled = miniPlayerView->handleEvent(&event);

        // Handle current view events if not handled by mini player
        if (!handled && currentView)
        {
            currentView->handleEvent(&event);
        }

        // Map keyboard events to user commands
        if (event.type == SDL_KEYDOWN)
        {
            UserCommand cmd = UserCommand::NONE;

            switch (event.key.keysym.sym)
            {
            case SDLK_SPACE:
                cmd = UserCommand::PLAY;
                break;
            case SDLK_p:
                cmd = UserCommand::PAUSE;
                break;
            case SDLK_s:
                cmd = UserCommand::STOP;
                break;
            case SDLK_RIGHT:
                cmd = UserCommand::SEEK_FORWARD;
                break;
            case SDLK_LEFT:
                cmd = UserCommand::SEEK_BACKWARD;
                break;
            case SDLK_UP:
                cmd = UserCommand::VOLUME_UP;
                break;
            case SDLK_DOWN:
                cmd = UserCommand::VOLUME_DOWN;
                break;
            case SDLK_n:
                cmd = UserCommand::NEXT;
                break;
            case SDLK_b:
                cmd = UserCommand::PREVIOUS;
                break;
            case SDLK_ESCAPE:
                cmd = UserCommand::BACK;
                break;
            case SDLK_RETURN:
                cmd = UserCommand::SELECT;
                break;
            }

            if (cmd != UserCommand::NONE && controller)
            {
                controller->handleUserCommand(cmd);
            }
        }
    }
}

void MainWindow::switchView(const std::string &viewName)
{
    auto it = views.find(viewName);
    if (it != views.end())
    {
        if (currentView)
        {
            currentView->hide();
        }

        currentView = it->second;
        currentView->show();
        currentView->update();
    }
}

void MainWindow::showDialog(const std::string &message)
{
    showingDialog = true;
    dialogLabel->setText(message);
    dialogLabel->setVisible(true);
    dialogOkButton->setVisible(true);
}

void MainWindow::hideDialog()
{
    showingDialog = false;
    dialogLabel->setVisible(false);
    dialogOkButton->setVisible(false);
}

SDL_Renderer *MainWindow::getRenderer()
{
    return renderer;
}

ApplicationController *MainWindow::getController()
{
    return controller;
}

void MainWindow::addView(const std::string &name, View *view)
{
    views[name] = view;
}

View *MainWindow::getView(const std::string &name)
{
    auto it = views.find(name);
    if (it != views.end())
    {
        return it->second;
    }
    return nullptr;
}