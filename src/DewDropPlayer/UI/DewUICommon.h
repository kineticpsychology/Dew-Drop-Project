/*******************************************************************************
*                                                                              *
* COPYRIGHT (C) Aug, 2020 | Polash Majumdar                                    *
*                                                                              *
* This file is part of the Dew Drop Player project. The file content comes     *
* "as is" without any warranty of definitive functionality. The author of the  *
* code is not to be held liable for any improper functionality, broken code,   *
* bug and otherwise unspecified error that might cause damage or might break   *
* the application where this code is imported. In accordance with the          *
* Zero-Clause BSD licence model of the Dew Drop project, the end-user/adopter  *
* of this code is hereby granted the right to use, copy, modify, and/or        *
* distribute this code with/without keeping this copyright notice.             *
*                                                                              *
* TL;DR - It's a free world. All of us should give back to the world that we   *
*         get so much from. I have tried doing my part by making this code     *
*         free (as in free beer). Have fun. Just don't vandalize the code      *
*         or morph it into a malicious one.                                    *
*                                                                              *
*******************************************************************************/

#ifndef _DEWUICOMMON_H_
#define _DEWUICOMMON_H_

#ifndef UNICODE
#define UNICODE
#endif

#define _WIN32_WINNT    0x0700  // Advanced WIN32 methods // Already defined in DewTheme.h
#define GDIPVER         0x0110  // Advanced GDI+ functionalities
#define _WIN32_IE       0x0500  // RTE control

#include "../Common/DewCommon.h"
#include <commctrl.h>
#include <gdiplus.h>
#include <shobjidl.h>
#include <ShlObj.h>
#include <PathCch.h>
#include "DewDropPlayerResource.h"
#ifdef _DEBUG
#include <stdio.h> // Debug exe is a console application. Include this to put out handy logs to the console
#endif

// Libs neeed for UI. Other libs are already included in the common.h file
#pragma comment(lib, "comctl32")
#pragma comment(lib, "shell32")
#pragma comment(lib, "pathcch")
#pragma comment(lib, "gdiplus")
#pragma comment(lib, "version")

// #pragma for enabling visual styles
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace Gdiplus;

#define DEWUI_MAX_TITLE             0x200   // 512 characters should be adequate
#define _F(X)                       (int)((float)X * _fScale)

// Notification Messages (For ProcessMessages)
#define DEWUI_MSG_INVALID           0x00    // Stub
#define DEWUI_MSG_CLICKED           0x01    // Any button click
#define DEWUI_MSG_HIDDEN            0x02    // Object has hidden
#define DEWUI_MSG_SHOWN             0x03    // Object has restored from hidden
#define DEWUI_MSG_AUDIO_LOADED      0x04    // Audio file has loaded
#define DEWUI_MSG_AUDIO_COMPLETE    0x05    // Playback complete
#define DEWUI_MSG_SEEKED            0x06    // Audio has been seeked by the user
#define DEWUI_MSG_PL_PARSE_COMPLETE 0x07    // Parsing and queuing of playlist files is complete
#define DEWUI_MSG_PL_SELECTION      0x08    // An item has been activated from the playlist
#define DEWUI_MSG_PL_EXPORT         0x09    // Export option has been selected from playlist (dwValue is a BOOL indicating export all (true) or selected)
#define DEWUI_MSG_VOL_CHANGED       0x0A    // The volume has been changed


// Notification Messages (For WndProc directly)
// These are also custom messages. But they are names with the WM_ Convention
#define WM_DEWMSG_TRAY              (WM_USER + 0xBEAD)  // Tray message notification
#define WM_DEWMSG_STYLE_CHANGED     (WM_USER + 0x05E7)  // Theme/Style changed (1337-speak for 'SET')

// User has selected a valid playlist for export
// wParam: The dwValue sent during DEWUI_MSG_PL_EXPORT message
// lParam: Pointer to a 'DEWPLAYLISTEXPORTINFO' structure containing the requisite information
#define WM_DEWMSG_PL_EXPORT_SEL     (WM_USER + 0x0914)

// A specific (child) object has been activated (in layout mode)
// WAPRAM: ObjectID
// LPARAM: Handle of the object (if sent from child control, 0 if sent from layout viewer)
#define WM_DEWMSG_LAYOUT_OBJ_ACTIVATED  (WM_USER + 0x0C1D)

// A child window position has changed
// WPARAM: Object ID
// LPARAM: LOWORD = x position, HIWORD = y position
#define WM_DEWMSG_CHILD_POS_CHANGED (WM_USER + 0xC9ED)

// A child window has been closed out (Settings/Extra Tag info windows etc)
// WPARAM: Not used
// LPARAM: HWND of the 'closing' child window
#define WM_DEWMSG_CHILD_CLOSED      (WM_USER + 0xBA5E)

// A Custom message for window to be able to queue (by reading memory map)
// WPARAM: Enqueuing sequence:
//         DEWUI_ENQUEUE_SEQ_MORE means more songs to come
//         DEWUI_ENQUEUE_SEQ_LAST means this is the last song
// LPARAM: Ignored
#define WM_DEWMSG_ENQUEUE           (WM_USER + 0x0ADD)
#define WM_DEWMSG_FLUSH_PLAYLIST    (WM_USER + 0x0C17)

// A change has happened to the value in DewUpDownControl
// WPARAM: New (changed) value
// LPARAM: Handle of the sender
#define WM_DEWMSG_UPDOWN_VALUE_CHANGED  (WM_USER + 0xC09) // 'COG' - like the cogwheel (bad joke!)

// The Audio Timer has completed. The window needs to close
#define WM_DEWMSG_COUNTDOWN_OVER    (WM_USER + 0x0F19)

// The user has made a (audio) disc selection
// WPARAM: ignored
// LPARAM: wchar_t casted path to the disc
#define WM_DEWMSG_DISC_SEL          (WM_USER + 0xD15C)

// Enqueuing Sources
#define DEWUI_ENQUEUE_SEQ_MORE      0x01
#define DEWUI_ENQUEUE_SEQ_LAST      0x00

// Notification Posters
// Do NOT modify these values or sequence!
// These will be used to populate the sub-component array in the window.
// If you do need to modify, pay VERY careful attention to the components array
// in the DEWWINDOW class!
#define DEWUI_OBJECT_COUNT          0x15    // A total of 20 components
#define DEWUI_OBJ_NONE              0x00    // Stub
#define DEWUI_OBJ_WINDOW            0xFF    // Window (Parent)

#define DEWUI_OBJ_MIN_BUTTON        0x01    // Minimize button
#define DEWUI_OBJ_CLOSE_BUTTON      0x02    // Close button ** Messages from here will have to be cascaded from the window to the application manager **

#define DEWUI_OBJ_ALBUM_ART         0x03    // Album Art module
#define DEWUI_OBJ_SEEKBAR           0x04    // Seekbar module
#define DEWUI_OBJ_INFO_TITLE        0x05    // Title information label module, with legend
#define DEWUI_OBJ_INFO_ARTIST       0x06    // Artist information label module, with legend
#define DEWUI_OBJ_INFO_ALBUM        0x07    // Album information label module, with legend

#define DEWUI_OBJ_PLAY_PAUSE_BUTTON 0x08    // Play/Pause button
#define DEWUI_OBJ_STOP_BUTTON       0x09    // Stop button
#define DEWUI_OBJ_PREV_BUTTON       0x0A    // Previous button
#define DEWUI_OBJ_NEXT_BUTTON       0x0B    // Next button
#define DEWUI_OBJ_VOLUME_BUTTON     0x0C    // Volume button
#define DEWUI_OBJ_REPEAT_BUTTON     0x0D    // Repeat button
#define DEWUI_OBJ_PLAYLIST_BUTTON   0x0E    // Playlist button
#define DEWUI_OBJ_BROWSE_BUTTON     0x0F    // Browse button
#define DEWUI_OBJ_SETTINGS_BUTTON   0x10    // Settings
#define DEWUI_OBJ_EXINFO_BUTTON     0x11    // Additional Tag/Info button
#define DEWUI_OBJ_TIMER_BUTTON      0x12    // Additional Tag/Info button
#define DEWUI_OBJ_DEWMODE_BUTTON    0x13    // 'Dew Mode' button

#define DEWUI_OBJ_PLAYLIST          0x14    // Playlist module
#define DEWUI_OBJ_LAYOUT_VIEWER     0x15    // Layout Position Viewer module
#define DEWUI_OBJ_DM_PLAY_PAUSE_BUTTON  0x16 // The 'Play/Pause' button in Dew Mode

// Default Symbol Font to be used throughout
#define DEWUI_SYMBOL_FONT_NAME      L"DewDrop"
// Caption for each of the component controls
#define DEWUI_CAP_DEFAULT           L"Dew Drop Player"
#define DEWCTL_CAP_MIN_BUTTON       L"Minimize Button"
#define DEWCTL_CAP_CLOSE_BUTTON     L"Close Button"
#define DEWCTL_CAP_ALBUM_ART        L"Album Art Viewer"
#define DEWCTL_CAP_SEEKBAR          L"Seekbar"
#define DEWCTL_CAP_INFO_TITLE       L"Song Title"
#define DEWCTL_CAP_INFO_ARTIST      L"Song Artist"
#define DEWCTL_CAP_INFO_ALBUM       L"Song Album"
#define DEWCTL_CAP_PLAY_PAUSE_BUTTON L"Play/Pause Button"
#define DEWCTL_CAP_STOP_BUTTON      L"Stop Button"
#define DEWCTL_CAP_PREV_BUTTON      L"Previous Button"
#define DEWCTL_CAP_NEXT_BUTTON      L"Next Button"
#define DEWCTL_CAP_VOLUME_BUTTON    L"Volume Button"
#define DEWCTL_CAP_REPEAT_BUTTON    L"Repeat Button"
#define DEWCTL_CAP_PLAYLIST_BUTTON  L"Playlist Button"
#define DEWCTL_CAP_BROWSE_BUTTON    L"Browse Button"
#define DEWCTL_CAP_SETTINGS_BUTTON  L"Settings Button"
#define DEWCTL_CAP_EXINFO_BUTTON    L"Additional Properties Button"
#define DEWCTL_CAP_TIMER_BUTTON     L"Audio Timer Button"
#define DEWCTL_CAP_DEWMODE_BUTTON   L"'Dew Mode' Button"
#define DEWCTL_CAP_PLAYLIST         L"Playlist"
#define DEWCTL_CAP_LAYOUT_VIEWER    L"Layout Viewer"

// Notification Codes
#define DEWUI_NCODE_DEFAULT             0x00    // Stub
#define DEWUI_NCODE_NEXT_LOGICAL_VALUE  0xCD    // Supposed value if the MM operation succeeds. This will typically be posted by state-based buttons like Play/Pause, Volume, Playlist, Repeat

// Object Types (Base Class Types)
#define DEWUI_OBJ_TYPE_NONE         0x00    // Stub
#define DEWUI_OBJ_TYPE_WINDOW       0x01    // Any window type
#define DEWUI_OBJ_TYPE_UIBUTTON     0x02    // Any UI Button (like minimize, close etc.)
#define DEWUI_OBJ_TYPE_MMBUTTON     0x03    // Audio action button
#define DEWUI_OBJ_TYPE_MODULE       0x04    // Any other non-button module (like playlist, album art, seekbar etc)

// States (Do not try to re-use the DEWS_* values)
// This is more of a UI-definition of the states
// (which can be either playing or not playing)
#define DEWUI_STATE_PLAYING         0x01
#define DEWUI_STATE_NOT_PLAYING     0x02

// Pre-defined base dimensions of UI elements
#define DEWUI_DIM_WINDOW_X          480
#define DEWUI_DIM_WINDOW_Y_NOPL     300
#define DEWUI_DIM_WINDOW_Y_DEWMODE  25
#define DEWUI_DIM_WINDOW_Y          640
#define DEWUI_DIM_SYSBUTTON         25
#define DEWUI_DIM_BUTTON            29  // Must be an odd value for nice circles & centered drawings!
#define DEWUI_DIM_VOL_BUTTON_X      129 // Rectangular (extended) length of the volume button
#define DEWUI_DIM_PNS_BUTTON        37  // Previous/Next/Stop Buttons (a tad bigger)
#define DEWUI_DIM_PLAY_BUTTON       65  // Main Play button
#define DEWUI_DIM_ALBUM_ART         152 // Album art will be a square (2 pixel offset for a 1 pixel boundary)
#define DEWUI_DIM_INFO_Y            22  // The height of the info labels. A 'DEWUI_DIM_INFO_X' is not needed as width will be dynamic
#define DEWUI_DIM_INFO_TEXT_PADDING 24  // The space from left, after which the text would start
#define DEWUI_DIM_PADDING_X         20  // Padding from the left from the window
#define DEWUI_DIM_PADDING_Y_HEADER  DEWUI_DIM_SYSBUTTON  // Height of the title bar
#define DEWUI_DIM_PADDING_Y_FOOTER  10  // Padding from bottom
#define DEWUI_DIM_SEEK_RECT_HEIGHT  8   // The height of the seekbar 'rectangle'
#define DEWUI_DIM_SEEKBAR_HEIGHT    32  // The overall height of the seekbar control
#define DEWUI_DIM_PL_ACTIVE_IND     16  // The dimension of the indicator for the currently playing/activated playlist item
#define DEWUI_DIM_DM_PLAY_BUTTON    (DEWUI_DIM_WINDOW_Y_DEWMODE - 2)
#define DEWUI_DIM_SYM_BUTTON_FONT   12  // The font size to be used for creating button icons from the Symbol font
#define DEWUI_DIM_SYM_BUTTON_ICON   16  // The icon size to be used for creating button icons from the Symbol font
#define DEWUI_DIM_SYM_MENU_FONT     12  // The font size to be used for creating menu icons from the Symbol font
#define DEWUI_DIM_SYM_MENU_ICON     24  // The icon size to be used for creating menu icons from the Symbol font
#define DEWUI_DIM_SYM_DISC_FONT     48  // The font size of the disc list in the disc selection screen
#define DEWUI_DIM_SYM_DISC_ICON     64  // The icon size of the disc list in the disc selection screen

// Pre-defined font sizes of buttons & Bold indicator
// The bold flag is set at 7th bit. So a max size of 63 point font is allowed
// Binary '&' with 0x40 (64) to get the BOLD flag
#define DEWUI_FNTSPEC_SYS_BUTTON    75  // 64 & 11
#define DEWUI_FNTSPEC_BUTTON        12  // 00 & 12
#define DEWUI_FNTSPEC_PNS_BUTTON    78  // 64 & 14
#define DEWUI_FNTSPEC_PLAY_BUTTON   96  // 64 & 36
// This is an exception. The 64th bit 'AND'ing will not apply for BOLD
#define DEWUI_FNTSPEC_ALBUM_ART     48
#define DEWUI_FNTSPEC_INFO_SYMBOL   14

#define IDM_OPEN_FILE               0x11
#define IDM_OPEN_DIR                0x12
#define IDM_OPEN_DISC               0x13
// These tray Menu IDs could be moved to window/uiobject
// but keeping them here, in case any child needs to access it by any chance
#define IDM_TRAY_PLAYPAUSE          0x71
#define IDM_TRAY_STOP               0x72
#define IDM_TRAY_NEXT               0x73
#define IDM_TRAY_PREV               0x74
#define IDM_TRAY_SEP                0x70
#define IDM_TRAY_RESTORE            0x75
#define IDM_TRAY_QUIT               0x76
#define IDM_CTX_LAYOUT              0x81
#define IDM_CTX_RESET_LAYOUT        0x82
#define IDM_CTX_DEWMODE             0x83
#define IDM_CTX_ABOUT               0x84
// Reserve the 0x9x & 0xAx series for the Hotkeys/Accelerators
#define IDM_HK_LAYOUT_UP            0x91
#define IDM_HK_LAYOUT_DOWN          0x92
#define IDM_HK_LAYOUT_LEFT          0x93
#define IDM_HK_LAYOUT_RIGHT         0x94
#define IDM_HK_LAYOUT_ACCEL_UP      0x95
#define IDM_HK_LAYOUT_ACCEL_DOWN    0x96
#define IDM_HK_LAYOUT_ACCEL_LEFT    0x97
#define IDM_HK_LAYOUT_ACCEL_RIGHT   0x98
#define IDM_HK_DEWSNAP_UP           0x99
#define IDM_HK_DEWSNAP_DOWN         0x9A
#define IDM_HK_DEWSNAP_LEFT         0x9B
#define IDM_HK_DEWSNAP_RIGHT        0x9C
#define IDM_HK_DEWMODE_TOGGLE       0x9D
#define IDM_HK_LAYOUT_TOGGLE        0x9E
#define IDM_HK_REPEAT_CYCLE         0x9F
#define IDM_HK_PLAYLIST_TOGGLE      0xA0
#define IDM_HK_OPEN_FILE_DLG        0xA1
#define IDM_HK_OPEN_DIR_DLG         0xA2
#define IDM_HK_EXINFO               0xA3
#define IDM_HK_TIMER                0xA4
#define IDM_HK_SETTINGS             0xA5
#define IDM_HK_ABOUT                0xA6
#define IDM_HK_CONTEXT_MENU         0xA7
#define IDM_HK_PLAYLIST_SEL_ALL     0xA8
#define IDM_HK_PLAYLIST_EXPORT_SEL  0xA9
#define IDM_HK_PLAYLIST_EXPORT_ALL  0xAA
// No explicit code for quit. We'll reuse the IDM_TRAY_QUIT code


// Do NOT use the range between 0x20 & 0x60 anywhere
// Note that these are ranges of 32 (0x20). So these are adequately spaced
// to contain the entirety of the DEWUI_OBJECT_COUNT
#define IDM_CTX_HIDE                0x20    // Messages will be added with DEW_OBJ ID
#define IDM_CTX_SHOW                0x40    // Messages will be added with DEW_OBJ ID
#define IDM_CTX_HIDE_TITLE          0x61    // Special message to hide the title
#define IDM_CTX_SHOW_TITLE          0x62    // Special message to show the title
// Commands for the Playlist context menu. It could be placed under the
// DewPlaylist.h header, but we want to make sure those constants do not
// collide with the IDM constants declared above. We'll reserve the 0xD<x> lineup
#define IDM_CTX_PL_PLAY             0xD1
#define IDM_CTX_PL_DELETE           0xD2
#define IDM_CTX_PL_EXPLORE_LOCATION 0xD3
#define IDM_CTX_PL_EXPORT_SEL       0xD4
#define IDM_CTX_PL_EXPORT_ALL       0xD5
#define IDM_CTX_PL_EXPORT_FMT_M3U   0xD6
#define IDM_CTX_PL_EXPORT_FMT_PLS   0xD7

// Playlist export format enumeration
#define DEWUI_PL_EXPORT_FMT_M3U8    0x00
#define DEWUI_PL_EXPORT_FMT_PLS     0x01

// Alpha & numeric key codes
#define VK_A                        0x41
#define VK_B                        0x42
#define VK_C                        0x43
#define VK_D                        0x44
#define VK_E                        0x45
#define VK_F                        0x46
#define VK_G                        0x47
#define VK_H                        0x48
#define VK_I                        0x49
#define VK_J                        0x4A
#define VK_K                        0x4B
#define VK_L                        0x4C
#define VK_M                        0x4D
#define VK_N                        0x4E
#define VK_O                        0x4F
#define VK_P                        0x50
#define VK_Q                        0x51
#define VK_R                        0x52
#define VK_S                        0x53
#define VK_T                        0x54
#define VK_U                        0x55
#define VK_V                        0x56
#define VK_W                        0x57
#define VK_X                        0x58
#define VK_Y                        0x59
#define VK_Z                        0x5A
#define VK_0                        0x30
#define VK_1                        0x31
#define VK_2                        0x32
#define VK_3                        0x33
#define VK_4                        0x34
#define VK_5                        0x35
#define VK_6                        0x36
#define VK_7                        0x37
#define VK_8                        0x38
#define VK_9                        0x39

// Text symbols (icons) for buttons
#define DEWUI_SYMBOL_MAIN_APP       L"p"
#define DEWUI_SYMBOL_MINIMIZE       L"A"
#define DEWUI_SYMBOL_CLOSE          L"E"
#define DEWUI_SYMBOL_ALBUM_ART      L"a"
#define DEWUI_SYMBOL_SONG_TITLE     L"b"
#define DEWUI_SYMBOL_SONG_ARTIST    L"d"
#define DEWUI_SYMBOL_SONG_ALBUM     L"e"
#define DEWUI_SYMBOL_PLAY           L"1"
#define DEWUI_SYMBOL_PAUSE          L"2"
#define DEWUI_SYMBOL_STOP           L"3"
#define DEWUI_SYMBOL_PREV           L"4"
#define DEWUI_SYMBOL_NEXT           L"5"
#define DEWUI_SYMBOL_VOL00          L"w"
#define DEWUI_SYMBOL_VOL33          L"x"
#define DEWUI_SYMBOL_VOL67          L"y"
#define DEWUI_SYMBOL_VOL100         L"z"
#define DEWUI_SYMBOL_REPEAT         L"0"
#define DEWUI_SYMBOL_PLAYLIST       L"I"
#define DEWUI_SYMBOL_BROWSE         L"l"
#define DEWUI_SYMBOL_SETTINGS       L"j"
#define DEWUI_SYMBOL_EX_INFO        L"h"
#define DEWUI_SYMBOL_TIMER          L"t"
#define DEWUI_SYMBOL_DEW_MODE       L"o"
#define DEWUI_SYMBOL_ABOUT          L":"

// Review these glyphs. Some may be ok. Most of the others: not
// Menu (HBITMAP) specific symbols
#define DEWUI_SYMBOL_LAYOUT_MODE    L"\\"
#define DEWUI_SYMBOL_RESET_LAYUOT   L"0"
#define DEWUI_SYMBOL_DELETE         L"_"
#define DEWUI_SYMBOL_EXPORT_SEL     L"H"
#define DEWUI_SYMBOL_EXPORT_ALL     L"I"
#define DEWUI_SYMBOL_OPEN_FILE      L"G"
#define DEWUI_SYMBOL_OPEN_DIR       L"l"
#define DEWUI_SYMBOL_OPEN_DISC      L"f"
#define DEWUI_SYMBOL_RESTORE        L"B"

#ifdef DEW_THEMER_MODE
#define DEWUI_SYMBOL_SAVE           L"v"
#define DEWUI_SYMBOL_DEFAULT        L"-"
#endif

// ListView, Button, Tab (HIMAGELIST) specific symbols
#define DEWUI_SYMBOL_ACTIVE_IND     L"1"
#define DEWUI_SYMBOL_CONFIG         L"i"
#define DEWUI_SYMBOL_HOTKEY         L","

#define DEWUI_SYMBOL_BUTTON_OK      L";"
#define DEWUI_SYMBOL_BUTTON_CANCEL  L"E"
#define DEWUI_SYMBOL_BUTTON_START   L"t"
#define DEWUI_SYMBOL_BUTTON_ABORT   L"E"
#define DEWUI_SYMBOL_BUTTON_RESET   L"<"

#define DEWUI_CMDLINE_OPT_PLAY      0x01
#define DEWUI_CMDLINE_OPT_ENQUEUE   0x02

#define DEWUI_MUTEX                 L"DEWDROP.SINGLEINST.WIN32.C.MUTEX"
#define DEWUI_MAP                   L"DEWDROP.SHARED.MEM.MAP"
#define DEWUI_ENQUEUE_EVENT         L"DEWDROP.SHARED.MEM.ENQUEUE.EVENT"

typedef struct _MAPCONTENT
{
    HWND        hWnd;
    HANDLE      hEvtStartEnqueue;
    BYTE        btCmdlineOpt;
    wchar_t     wsMediaPath[MAX_CHAR_PATH];
} MAPCONTENT, *LPMAPCONTENT;

// This can be used both as array or a Linked List
typedef struct _FILELIST
{
    wchar_t             wsFileName[MAX_CHAR_PATH];
    struct _FILELIST    *next;
} FILELIST, *LPFILELIST;

typedef struct _DEWPLAYLISTEXPORTINFO
{
    wchar_t     wsExportFile[MAX_CHAR_PATH];
    BYTE        DEWUI_PL_EXPORT_FMT;
    BOOL        bUseRelativePath;
    BOOL        bOperationValid;
} DEWPLAYLISTEXPORTINFO, *LPDEWPLAYLISTEXPORTINFO;

#endif // _DEWUICOMMON_H_
