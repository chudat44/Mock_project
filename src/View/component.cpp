#include "component.h"

#include <iostream>

// Base UI Component implementation
UIComponent::UIComponent(int x, int y, int w, int h)
    : bounds{x, y, w, h}, visible(true), enabled(true) {}

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

// Button implementation
Button::Button(int x, int y, int w, int h, const std::string &buttonText)
    : TextComponent(x, y, w, h, buttonText), isHovered(false)
{
    setAlign(TextAlign::Center);
}

void Button::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    // Set button color based on hover state
    SDL_Color currentColor = isHovered ? BUTTON_HOVER_COLOR : BUTTON_COLOR;

    // Draw button background
    SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
    SDL_RenderFillRect(renderer, &bounds);

    // Draw button border
    SDL_SetRenderDrawColor(renderer, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b, BORDER_COLOR.a); // Black border
    SDL_RenderDrawRect(renderer, &bounds);

    // Render text if not empty
    if (!getText().empty())
    {
        TextComponent::render(renderer);
    }
}

bool Button::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled)
        return false;

    bool handled = false;

    switch (event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            int x = event->button.x;
            int y = event->button.y;

            if (containsPoint(x, y))
            {
                if (onClick)
                {
                    onClick();
                }
                handled = true;
            }
        }
        break;
    }
    case SDL_MOUSEMOTION:
    {
        int x = event->motion.x;
        int y = event->motion.y;
        bool wasHovered = isHovered;
        isHovered = containsPoint(x, y);

        // Return true if hover state changed
        handled = (wasHovered != isHovered);
        break;
    }
    }

    return handled;
}

void Button::setOnClick(std::function<void()> callback)
{
    onClick = callback;
}

void Button::setColors(SDL_Color text, SDL_Color background, SDL_Color hover)
{
    TextComponent::setTextColor(text);
    backgroundColor = background;
    hoverColor = hover;
}

// TextComponent implementation
TextComponent::TextComponent(int x, int y, int w, int h, const std::string &labelText)
    : UIComponent(x, y, w, h), nLines(1), alignment(TextAlign::Left)
{
    setText(labelText);
}

TextComponent::~TextComponent()
{
}

std::vector<std::string> TextComponent::wrapText(const std::string &text, TTF_Font *font, int maxWidth)
{
    std::vector<std::string> lines;
    std::vector<std::string> words;

    size_t start = 0;
    size_t end = 0;

    // Split into words manually
    while ((end = text.find(' ', start)) != std::string::npos)
    {
        if (end > start)
            words.emplace_back(text.substr(start, end - start));
        start = end + 1;
    }
    if (start < text.length())
        words.emplace_back(text.substr(start));

    // Build lines from words
    std::string line;
    for (const std::string &word : words)
    {
        std::string testLine = line.empty() ? word : line + " " + word;
        int w = 0;
        TTF_SizeUTF8(font, testLine.c_str(), &w, nullptr);

        if (w > maxWidth && !line.empty())
        {
            lines.push_back(line);
            line = word;
        }
        else
        {
            line = testLine;
        }
    }

    if (!line.empty())
        lines.push_back(line);

    return lines;
}

void TextComponent::render(SDL_Renderer *renderer)
{
    if (!visible || text.empty())
        return;

    TTF_Font *font = FontManager::GetDefaultFont();

    // Render each line
    int y_offset = 0;
    if (nLines == 1)
    {
        // Single line rendering
        SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, text.c_str(), textColor);
        if (textSurface)
        {
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture)
            {
                // Center text vertically, align left
                int lineHeight = textSurface->h;
                int lineWidth = textSurface->w;

                SDL_Rect dstRect;
                dstRect.y = bounds.y + (bounds.h - lineHeight) / 2;
                dstRect.w = lineWidth;
                dstRect.h = lineHeight;

                // Horizontal alignment
                switch (alignment)
                {
                case TextAlign::Left:
                    dstRect.x = bounds.x + 5;
                    break;
                case TextAlign::Center:
                    dstRect.x = bounds.x + (bounds.w - lineWidth) / 2;
                    break;
                case TextAlign::Right:
                    dstRect.x = bounds.x + (bounds.w - lineWidth);
                    break;
                }

                SDL_RenderCopy(renderer, textTexture, NULL, &dstRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
    }
    else
    {
        for (const std::string &line : lines)
        {
            SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, line.c_str(), textColor);
            if (!textSurface)
                continue;

            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (!textTexture)
            {
                SDL_FreeSurface(textSurface);
                continue;
            }

            int lineHeight = textSurface->h;
            int lineWidth = textSurface->w;

            SDL_Rect dstRect;
            dstRect.y = bounds.y + y_offset;
            dstRect.w = lineWidth;
            dstRect.h = lineHeight;

            // Horizontal alignment
            switch (alignment)
            {
            case TextAlign::Left:
                dstRect.x = bounds.x;
                break;
            case TextAlign::Center:
                dstRect.x = bounds.x + (bounds.w - lineWidth) / 2;
                break;
            case TextAlign::Right:
                dstRect.x = bounds.x + (bounds.w - lineWidth);
                break;
            }

            SDL_RenderCopy(renderer, textTexture, nullptr, &dstRect);

            y_offset += lineHeight;
            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
    }
}

bool TextComponent::handleEvent(SDL_Event *event)
{
    return false; // TextComponents don't typically handle events
}

void TextComponent::setText(const std::string &labelText)
{
    text = labelText;

    lines.clear();
    TTF_Font *font = FontManager::GetDefaultFont();

    if (nLines == 1) // Multi line
    {
        int textWidth = 0;
        TTF_SizeUTF8(font, text.c_str(), &textWidth, nullptr);
        if (textWidth > bounds.w)
        {
            lines = wrapText(text, font, bounds.w);
        }
        else
        {
            lines.push_back(text);
        }
    }
    else
    {
        // Basic newline-based line splitting
        size_t start = 0;
        size_t end = 0;

        while ((end = text.find('\n', start)) != std::string::npos)
        {
            std::string line = text.substr(start, end - start);
            auto wrapped = wrapText(line, font, bounds.w);
            lines.insert(lines.end(), wrapped.begin(), wrapped.end());
            start = end + 1;
        }

        // Handle the final line (or if there's no newline at all)
        if (start < text.length())
        {
            std::string line = text.substr(start);
            auto wrapped = wrapText(line, font, bounds.w);
            lines.insert(lines.end(), wrapped.begin(), wrapped.end());
        }
    }
}

void TextComponent::setAlign(TextAlign align)
{
    alignment = align;
    TextComponent::setText(text);
}
void TextComponent::setLines(int numberLines)
{
    if (numberLines > 0)
        nLines = numberLines;
    TextComponent::setText(text);
}

std::string TextComponent::getText() const
{
    return text;
}

void TextComponent::setTextColor(SDL_Color color)
{
    textColor = color;
}

// ProgressBar implementation
ProgressBar::ProgressBar(int x, int y, int w, int h)
    : UIComponent(x, y, w, h), value(0.0f), isDraggable(false)
{
}

void ProgressBar::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    // Draw background
    SDL_SetRenderDrawColor(renderer, PANEL_COLOR.r, PANEL_COLOR.g, PANEL_COLOR.b, PANEL_COLOR.a);
    SDL_RenderFillRect(renderer, &bounds);

    // Draw filled portion
    int fillWidth = static_cast<int>(bounds.w * value);
    SDL_Rect fillRect = {bounds.x, bounds.y, fillWidth, bounds.h};
    SDL_SetRenderDrawColor(renderer, PROGRESS_COLOR.r, PROGRESS_COLOR.g, PROGRESS_COLOR.b, PROGRESS_COLOR.a);
    SDL_RenderFillRect(renderer, &fillRect);

    // Draw border
    SDL_SetRenderDrawColor(renderer, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b, BORDER_COLOR.a);
    SDL_RenderDrawRect(renderer, &bounds);
}

bool ProgressBar::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled || !isDraggable)
        return false;

    switch (event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            int x = event->button.x;
            int y = event->button.y;

            if (containsPoint(x, y))
            {
                // Calculate new value based on click position
                float newValue = static_cast<float>(x - bounds.x) / bounds.w;
                // Clamp value between 0 and 1
                newValue = (newValue < 0) ? 0 : (newValue > 1) ? 1
                                                               : newValue;

                setValue(newValue);
                if (onValueChanged)
                {
                    onValueChanged(newValue);
                }
                return true;
            }
        }
        break;
    }
    case SDL_MOUSEMOTION:
    {
        if (event->motion.state & SDL_BUTTON_LMASK)
        {
            int x = event->motion.x;

            // Check if drag started inside the component
            if (containsPoint(x, event->motion.y) ||
                (x >= bounds.x - 100 && x <= bounds.x + bounds.w + 100))
            {

                // Calculate new value based on drag position, clamped to [0,1]
                float newValue = static_cast<float>(x - bounds.x) / bounds.w;
                newValue = (newValue < 0) ? 0 : (newValue > 1) ? 1
                                                               : newValue;
                setValue(newValue);
                if (onValueChanged)
                {
                    onValueChanged(newValue);
                }
                return true;
            }
        }
        break;
    }
    }

    return false;
}

void ProgressBar::setValue(float newValue)
{
    // Clamp value between 0 and 1
    value = (newValue < 0) ? 0 : (newValue > 1) ? 1
                                                : newValue;
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

// ListView implementation
ListView::ListView(int x, int y, int w, int h)
    : UIComponent(x, y, w, h), selectedIndex(-1), firstVisibleIndex(0), visibleItemCount(0), hasScrollbar(false)
{
}

void ListView::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    // Calculate item height (assuming fixed height)
    int itemHeight = 30; // Adjust as needed
    visibleItemCount = bounds.h / itemHeight;

    // Render visible items
    int endIndex = std::min(static_cast<int>(items.size()), firstVisibleIndex + visibleItemCount);
    for (int i = firstVisibleIndex; i < endIndex; i++)
    {
        SDL_Rect itemRect = {
            bounds.x,
            bounds.y + (i - firstVisibleIndex) * itemHeight,
            bounds.w - (hasScrollbar ? 15 : 0), // Leave space for scrollbar
            itemHeight};

        // Draw selection background if this item is selected
        if (i == selectedIndex)
        {
            SDL_SetRenderDrawColor(renderer, PROGRESS_COLOR.r, PROGRESS_COLOR.g, PROGRESS_COLOR.b, PROGRESS_COLOR.a); // Highlight color
            SDL_RenderFillRect(renderer, &itemRect);
        }

        // Draw item text
        SDL_Surface *textSurface = TTF_RenderUTF8_Blended(FontManager::GetDefaultFont(), items[i].c_str(), TEXT_COLOR);
        if (textSurface)
        {
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture)
            {
                // Center text vertically in its row, align left with a small margin
                SDL_Rect textRect = {
                    itemRect.x + 5, // 5 pixels margin
                    itemRect.y + (itemRect.h - textSurface->h) / 2,
                    textSurface->w,
                    textSurface->h};

                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }

        // Draw horizontal separator line
        SDL_SetRenderDrawColor(renderer, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b, BORDER_COLOR.a); // Light gray
        SDL_RenderDrawLine(
            renderer,
            itemRect.x,
            itemRect.y + itemRect.h - 1,
            itemRect.x + itemRect.w,
            itemRect.y + itemRect.h - 1);
    }

    // Draw scrollbar if needed
    if (hasScrollbar && items.size() > visibleItemCount)
    {
        int scrollbarWidth = 20;
        SDL_Rect scrollbarRect = {
            bounds.x + bounds.w - scrollbarWidth,
            bounds.y,
            scrollbarWidth,
            bounds.h};

        // Draw scrollbar background
        SDL_SetRenderDrawColor(renderer, PANEL_COLOR.r, PANEL_COLOR.g, PANEL_COLOR.b, PANEL_COLOR.a); // Light gray
        SDL_RenderFillRect(renderer, &scrollbarRect);

        // Draw scrollbar handle
        float handleRatio = static_cast<float>(visibleItemCount) / items.size();
        float handlePosition = static_cast<float>(firstVisibleIndex) / items.size();

        int handleHeight = static_cast<int>(bounds.h * handleRatio);
        handleHeight = (handleHeight < 20) ? 20 : handleHeight; // Minimum handle size

        int handleY = static_cast<int>(bounds.y + bounds.h * handlePosition);

        SDL_Rect handleRect = {
            scrollbarRect.x,
            handleY,
            scrollbarWidth,
            handleHeight};

        SDL_SetRenderDrawColor(renderer, BUTTON_HOVER_COLOR.r, BUTTON_HOVER_COLOR.g, BUTTON_HOVER_COLOR.b, BUTTON_HOVER_COLOR.a); // Darker gray
        SDL_RenderFillRect(renderer, &handleRect);
    }
}

bool ListView::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled || items.empty())
        return false;

    switch (event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            int x = event->button.x;
            int y = event->button.y;

            if (containsPoint(x, y))
            {
                // Calculate item height
                int itemHeight = 30; // Should match the one in render

                // Calculate which item was clicked
                int clickedIndex = firstVisibleIndex + (y - bounds.y) / itemHeight; // Make sure the index is valid
                if (clickedIndex >= 0 && clickedIndex < static_cast<int>(items.size()))
                {
                    if (event->button.clicks >= 2)
                    {
                        set2ClickSelectedIndex(clickedIndex);
                    }
                    else
                    {
                        setSelectedIndex(clickedIndex);
                    }
                    return true;
                }
            }
        }
        break;
    }
    case SDL_MOUSEWHEEL:
    {
        int x = event->wheel.mouseX;
        int y = event->wheel.mouseY;

        if (containsPoint(x, y))
        {
            // Scroll up (positive y) or down (negative y)
            int scrollAmount = -event->wheel.y;
            scroll(scrollAmount);
            return true;
        }
        break;
    }
    case SDL_KEYDOWN:
    {
        switch (event->key.keysym.sym)
        {
        case SDLK_UP:
            if (selectedIndex > 0)
            {
                setSelectedIndex(selectedIndex - 1);

                // Adjust scrolling if needed
                if (selectedIndex < firstVisibleIndex)
                {
                    firstVisibleIndex = selectedIndex;
                }
                return true;
            }
            break;
        case SDLK_DOWN:
            if (selectedIndex < static_cast<int>(items.size()) - 1)
            {
                setSelectedIndex(selectedIndex + 1);

                // Adjust scrolling if needed
                if (selectedIndex >= firstVisibleIndex + visibleItemCount)
                {
                    firstVisibleIndex = selectedIndex - visibleItemCount + 1;
                }
                return true;
            }
            break;
        }
        break;
    }
    }

    return false;
}

void ListView::addItem(const std::string &item)
{
    items.push_back(item);
    hasScrollbar = (items.size() > visibleItemCount);
}

void ListView::removeItem(int index)
{
    if (index >= 0 && index < static_cast<int>(items.size()))
    {
        items.erase(items.begin() + index);

        // Update selected index if necessary
        if (selectedIndex >= static_cast<int>(items.size()))
        {
            selectedIndex = items.empty() ? -1 : static_cast<int>(items.size()) - 1;
        }

        // Update scrollbar state
        hasScrollbar = (items.size() > visibleItemCount);

        // Adjust firstVisibleIndex if necessary
        if (firstVisibleIndex > static_cast<int>(items.size()) - visibleItemCount &&
            firstVisibleIndex > 0)
        {
            firstVisibleIndex = std::max(0, static_cast<int>(items.size()) - visibleItemCount);
        }
    }
}

void ListView::clearItems()
{
    items.clear();
    selectedIndex = -1;
    firstVisibleIndex = 0;
    hasScrollbar = false;
}

size_t ListView::size() const
{
    return items.size();
}

std::string ListView::getItem(int index) const
{
    if (index >= 0 && index < static_cast<int>(items.size()))
    {
        return items[index];
    }
    return "";
}

const std::vector<std::string> &ListView::getItems() const
{
    return items;
}

void ListView::setItems(const std::vector<std::string> &newItems)
{
    items = newItems;
    selectedIndex = items.empty() ? -1 : 0;
    firstVisibleIndex = 0;
    hasScrollbar = (items.size() > visibleItemCount);
}

int ListView::getSelectedIndex() const
{
    return selectedIndex;
}

void ListView::setSelectedIndex(int index)
{
    if (index >= -1 && index < static_cast<int>(items.size()))
    {

        if (index != selectedIndex && onSelectionChanged)
        {
            selectedIndex = index;
            onSelectionChanged(selectedIndex);
        }
    }
}
void ListView::set2ClickSelectedIndex(int index)
{
    if (index >= -1 && index < static_cast<int>(items.size()))
    {

        if (onSelectionChanged)
        {
            selectedIndex = index;
            on2ClickSelectionChanged(selectedIndex);
        }
    }
}

void ListView::setOnSelectionChanged(std::function<void(int)> callback)
{
    onSelectionChanged = callback;
}
void ListView::setOn2ClickSelectionChanged(std::function<void(int)> callback)
{
    on2ClickSelectionChanged = callback;
}

void ListView::scroll(int amount)
{
    // Calculate max first visible index
    int maxFirstVisible = std::max(0, static_cast<int>(items.size()) - visibleItemCount);

    // Update first visible index
    firstVisibleIndex += amount;

    // Clamp to valid range
    if (firstVisibleIndex < 0)
    {
        firstVisibleIndex = 0;
    }
    else if (firstVisibleIndex > maxFirstVisible)
    {
        firstVisibleIndex = maxFirstVisible;
    }
}

// TextField implementation
TextField::TextField(int x, int y, int w, int h, const std::string &initialText)
    : TextComponent(x, y, w, h, initialText), text(initialText), isFocused(false), cursorPosition(0)
{
    placeholder = ""; // Empty placeholder by default
}

void TextField::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    // Draw background
    SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
    SDL_RenderFillRect(renderer, &bounds);

    // Draw border (thicker if focused)
    SDL_SetRenderDrawColor(renderer, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b, BORDER_COLOR.a);
    SDL_RenderDrawRect(renderer, &bounds);

    if (isFocused)
    {
        SDL_Rect innerBorder = {
            bounds.x + 1,
            bounds.y + 1,
            bounds.w - 2,
            bounds.h - 2};
        SDL_RenderDrawRect(renderer, &innerBorder);
    }

    // Determine text to display (actual text or placeholder)
    std::string displayText = text;

    if (displayText.empty() && !placeholder.empty())
    {
        displayText = placeholder;
    }

    // Render text if not empty
    if (!displayText.empty())
    {
        TextComponent::setText(displayText);
        TextComponent::render(renderer);
    }

    // Draw cursor if focused
    if (isFocused)
    {
        // Calculate cursor position
        int cursorX = bounds.x + 5; // Start with left margin

        if (!displayText.empty() && cursorPosition > 0)
        {
            // Calculate width of text up to cursor position
            std::string textBeforeCursor = displayText.substr(0, cursorPosition);
            int w, h;
            TTF_SizeText(FontManager::GetDefaultFont(), textBeforeCursor.c_str(), &w, &h);
            cursorX += w;
        }

        // Draw cursor line
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black cursor
        SDL_RenderDrawLine(
            renderer,
            cursorX,
            bounds.y + 5,
            cursorX,
            bounds.y + bounds.h - 5);
    }
}

bool TextField::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled)
        return false;

    switch (event->type)
    {
    case SDL_MOUSEMOTION:
    {
        int x = event->motion.x;
        int y = event->motion.y;

        if (containsPoint(x, y))
        {
            SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM));
            // optionally store & reuse cursor instead of creating every time
        }
        break;
    }
    case SDL_MOUSEBUTTONDOWN:
    {
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            int x = event->button.x;
            int y = event->button.y;

            bool wasFocused = isFocused;
            isFocused = containsPoint(x, y);

            if (isFocused)
            {
                // Set cursor position based on click (simplified)
                cursorPosition = text.length(); // Just go to the end for now

                // TODO: For better UX, place cursor based on click position relative to text
                return true;
            }
            unfocus();

            return wasFocused != isFocused; // Return true if focus changed
        }
        break;
    }
    case SDL_KEYDOWN:
    {
        if (!isFocused)
            return false;

        switch (event->key.keysym.sym)
        {
        case SDLK_BACKSPACE:
        {
            if (!text.empty() && cursorPosition > 0)
            {
                text.erase(cursorPosition - 1, 1);
                cursorPosition--;
                return true;
            }
            break;
        }
        case SDLK_DELETE:
        {
            if (!text.empty() && cursorPosition < text.length())
            {
                text.erase(cursorPosition, 1);

                return true;
            }
            break;
        }
        case SDLK_LEFT:
        {
            if (cursorPosition > 0)
            {
                cursorPosition--;
                return true;
            }
            break;
        }
        case SDLK_RIGHT:
        {
            if (cursorPosition < text.length())
            {
                cursorPosition++;
                return true;
            }
            break;
        }
        case SDLK_HOME:
        {
            cursorPosition = 0;
            return true;
        }
        case SDLK_END:
        {
            cursorPosition = text.length();
            return true;
        }
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        {
            isFocused = false;
            // Trigger text changed and unfocus
            if (onTextChanged)
            {
                onTextChanged(text);
            }
            return true;
        }
        case SDLK_ESCAPE:
        {
            // Cancel and unfocus
            isFocused = false;
            return true;
        }
        }
        break;
    }
    case SDL_TEXTINPUT:
    {
        if (!isFocused)
            return false;

        // Insert text at cursor position
        text.insert(cursorPosition, event->text.text);
        cursorPosition += strlen(event->text.text);
        return true;
    }
    }

    return false;
}

void TextField::setText(const std::string &newText)
{
    text = newText;
    cursorPosition = newText.length(); // Move cursor to end
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
    cursorPosition = text.length(); // Place cursor at end when focused
    SDL_StartTextInput();
}

void TextField::unfocus()
{
    isFocused = false;
    SDL_StopTextInput();
}
void VolumeSlider::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    // Draw background track
    SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
    SDL_Rect trackRect = {
        bounds.x,
        bounds.y + bounds.h / 2 - 2,
        bounds.w,
        4};
    SDL_RenderFillRect(renderer, &trackRect);

    // Draw filled portion
    int fillWidth = static_cast<int>(bounds.w * volume / 100);
    SDL_Rect fillRect = {
        bounds.x,
        trackRect.y,
        fillWidth,
        trackRect.h};
    SDL_SetRenderDrawColor(renderer, BUTTON_COLOR.r, BUTTON_COLOR.g, BUTTON_COLOR.b, BUTTON_COLOR.a);
    SDL_RenderFillRect(renderer, &fillRect);

    // Draw knob
    int knobX = bounds.x + fillWidth - 5;
    int knobSize = 10;
    SDL_Rect knobRect = {
        knobX,
        bounds.y + bounds.h / 2 - knobSize / 2,
        knobSize,
        knobSize};
    SDL_SetRenderDrawColor(renderer, knobColor.r, knobColor.g, knobColor.b, knobColor.a);
    SDL_RenderFillRect(renderer, &knobRect);

    // Draw border around knob for better visibility
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &knobRect);
}

VolumeSlider::VolumeSlider(int x, int y, int w, int h)
    : UIComponent(x, y, w, h), volume(50), isDragging(false)
{
    // Default colors
    fillColor = {0, 122, 255, 255};         // Blue
    backgroundColor = {200, 200, 200, 255}; // Light gray
    knobColor = {255, 255, 255, 255};       // White
}

bool VolumeSlider::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled)
        return false;

    switch (event->type)
    {
    case SDL_MOUSEBUTTONDOWN:
        if (event->button.button == SDL_BUTTON_LEFT && containsPoint(event->button.x, event->button.y))
        {
            isDragging = true;
            // Update volume based on click position
            int relativeX = event->button.x - bounds.x;
            int newVolume = (relativeX * 100) / bounds.w;
            setVolume(newVolume);
            // Trigger callback if set
            if (onVolumeChanged)
                onVolumeChanged(volume);
            return true;
        }
        break;

    case SDL_MOUSEMOTION:
        if (isDragging)
        {
            // Update volume while dragging
            int relativeX = event->motion.x - bounds.x;
            int newVolume = (relativeX * 100) / bounds.w;
            setVolume(newVolume);
            // Trigger callback if set
            if (onVolumeChanged)
                onVolumeChanged(volume);
            return true;
        }
        break;

    case SDL_MOUSEBUTTONUP:
        if (isDragging && event->button.button == SDL_BUTTON_LEFT)
        {
            isDragging = false;
            return true;
        }
        break;
    }

    return false;
}

void VolumeSlider::setVolume(int newVolume)
{
    // Clamp volume between 0 and 100
    volume = (newVolume < 0) ? 0 : (newVolume > 100) ? 100
                                                     : newVolume;
}

int VolumeSlider::getVolume() const
{
    return volume;
}

void VolumeSlider::setOnVolumeChanged(std::function<void(int)> callback)
{
    onVolumeChanged = callback;
}

Pagination::Pagination(int x, int y, int w, int h)
    : UIComponent(x, y, w, h), currentPage(0), totalPages(1)
{
    int buttonWidth = 70;
    int buttonHeight = h;
    int labelWidth = 35;
    int spacing = 5;

    prevButton = new Button(x, y, buttonWidth, buttonHeight, "Previous");
    prevButton->setOnClick([this]()
                           {
        if (currentPage > 0) {
            setCurrentPage(currentPage - 1);
        } });

    int fieldWidth = w - 2 * buttonWidth - 2 * spacing - labelWidth;
    pageField = new TextField(x + buttonWidth + spacing, y, fieldWidth, buttonHeight, "1");
    pageField->setOnTextChanged(
        [this](const std::string &text)
        {
            if (text.empty())
                return;

            int page = std::stoi(text);
            if (std::to_string(page) == text)
            {
                if (page < 1)
                {
                    page = 1;
                    pageField->setText("1");
                }
                else if (page > totalPages)
                {
                    page = totalPages;
                    pageField->setText(std::to_string(totalPages));
                }
                onPageChanged(page - 1);
            }
        });

    pageLabel = new TextComponent(x + buttonWidth + spacing + fieldWidth, y, labelWidth, buttonHeight, " of 1");
    pageLabel->setAlign(TextField::TextAlign::Left);

    nextButton = new Button(x + buttonWidth + spacing + labelWidth + spacing + fieldWidth, y, buttonWidth, buttonHeight, "Next");
    nextButton->setOnClick([this]()
                           {
        if (currentPage < totalPages - 1) {
            setCurrentPage(currentPage + 1);
        } });

    updateButtonStates();
}

Pagination::~Pagination()
{
    delete prevButton;
    delete nextButton;
    delete pageLabel;
    delete pageField;
}

void Pagination::render(SDL_Renderer *renderer)
{
    if (!visible)
        return;

    prevButton->render(renderer);
    pageField->render(renderer);
    pageLabel->render(renderer);
    nextButton->render(renderer);
}

bool Pagination::handleEvent(SDL_Event *event)
{
    if (!visible || !enabled)
        return false;

    if (prevButton->handleEvent(event))
        return true;

    if (nextButton->handleEvent(event))
        return true;

    if (pageField->handleEvent(event))
        return true;

    return false;
}

void Pagination::setCurrentPage(int page)
{
    if (page >= 0 && page < totalPages)
    {
        currentPage = page;
        updateButtonStates();
        // Update label
        std::string text = std::to_string(currentPage + 1);
        pageField->setText(text);

        // Trigger callback if set
        if (onPageChanged)
            onPageChanged(currentPage);
    }
}

int Pagination::getCurrentPage() const
{
    return currentPage;
}

void Pagination::setTotalPages(int pages)
{
    totalPages = (pages < 1) ? 1 : pages;

    // Adjust current page if needed
    if (currentPage >= totalPages)
        setCurrentPage(totalPages - 1);
    else
        updateButtonStates();

    // Update label
    std::string text = " of " + std::to_string(totalPages);
    pageLabel->setText(text);

    // Update visibility
    setVisible(totalPages > 1);
}

int Pagination::getTotalPages()
{
    return totalPages;
}

void Pagination::setOnPageChanged(std::function<void(int)> callback)
{
    onPageChanged = callback;
}

void Pagination::updateButtonStates()
{
    prevButton->setEnabled(currentPage > 0);
    nextButton->setEnabled(currentPage < totalPages - 1);
}