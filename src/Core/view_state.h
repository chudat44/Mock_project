#ifndef VIEW_STATE_H
#define VIEW_STATE_H

enum class ViewState
{
    MAIN_MENU,
    MEDIA_LIST,
    PLAYLIST_MENU,
    PLAYLIST_CONTENT,
    MEDIA_DETAILS,
    METADATA_EDIT,
    PLAYBACK,
    USB_DEVICES
};
enum class MenuItem {
    BROWSE_FILES,
    PLAYLISTS,
    SEARCH,
    USB_DEVICES,
    SETTINGS,
    EXIT
};

#endif