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

#ifndef _DEWSETTINGS_H_
#define _DEWSETTINGS_H_

#include "DewTheme.h"
#include "DewIconRepository.h"

#define DEWOPT_CLOSE_QUIT           0
#define DEWOPT_CLOSE_TRAY           1

#define DEWOPT_MINIMIZE_MINIMIZE    0
#define DEWOPT_MINIMIZE_TRAY        1

#define DEWOPT_REPEAT_OFF           0
#define DEWOPT_REPEAT_ALL           1
#define DEWOPT_REPEAT_ONE           2

typedef class DEWSETTINGS
{
    protected:

        #pragma region CORE SETTINGS SECTION

        // The _bXXX (non _btXXX) BYTE values are actually BOOL, but BYTE is
        // guaranteed to take 1 byte only, unlike BOOL, which might turn out to
        // be a DWORD. Marking them as BYTE is more space-efficient
        typedef struct _SETTINGS
        {
            BYTE        _btMinimizeAction;
            BYTE        _btCloseAction;
            DWORD       _dwVolLevel;
            BYTE        _btRepeatLevel;
            BYTE        _btPlaylistVisible;
            wchar_t     _wsThemeFile[MAX_CHAR_PATH];
            BYTE        _bRememberHistory;
            DWORD       _dwHistoryFileCount;
            BYTE        _bTopmostInDewMode;
            BYTE        _bNotifyOnSongChange;
            BYTE        _bDeepScan;
            // Refer to DewUICommon.h file's '// Notification Posters' section
            // for the explanation. That same sequence is replicated here
            DWORD       _lpdwComponentPos[DEWUI_OBJECT_COUNT];
            BOOL        _lpbComponentVisibility[DEWUI_OBJECT_COUNT];
            BYTE        _bShowTitle;
            BYTE        _bDewMode;
        } SETTINGS, *LPSETTINGS;

        SETTINGS        _settings;
        DEWTHEME        _dewTheme;
        LPFILELIST      _lpSongList = NULL;
        const UINT      _WM_APPLY = WM_USER + 0x0A;
        const UINT      _WM_CANCEL = WM_USER + 0x0B;
        BOOL            _bThemeNeedsRefresh = FALSE;

        const wchar_t*  _wsSettingsFileName = L"DewDrop.Settings";
        wchar_t         _wsSettingsFile[MAX_CHAR_PATH];
        const wchar_t*  _wsDefaultThemeRepr = L"<Default Theme>";
        const char*     _lpDefPass = "D3W.$3771n9s.D3f.P@$$";

        virtual void    _SaveSettings();
        virtual void    _TrimPathForLocalPath(LPWSTR wsPath);
        virtual void    _ReadSettingsWithDefaults();

        #pragma endregion

        #pragma region SETTINGS UI SECTION

        typedef struct _ENQUEQUE_WAIT_OPTIONS
        {
            DWORD       dwWaitTime;
            wchar_t     wsCaption[64];
        } ENQUEQUE_WAIT_OPTIONS, *LPENQUEQUE_WAIT_OPTIONS;

        HINSTANCE       _hInstance = NULL;
        HWND            _hWndParent = NULL;
        HWND            _hWndSettings = NULL;

        // Child window controls
        HWND            _hLblCloseAction = NULL, _hLblMinAction = NULL, _hLblThemeFile = NULL;
        HWND            _hOptCloseQuit = NULL, _hOptCloseTray = NULL;
        HWND            _hOptMinMinimize = NULL, _hOptMinTray = NULL;
        HWND            _hChkRememberHist = NULL, _hChkDewModeTopmost = NULL;
        HWND            _hChkNotifyOnSongChange = NULL, _hChkDeepScan = NULL;
        HWND            _hLblThemeName = NULL, _hLblThemeAuthor = NULL;
        HWND            _hTxtThemeFile = NULL, _hCmdBrowseThemeFile = NULL;
        HWND            _hGrpCloseAction = NULL, _hGrpMinAction = NULL, _hGrpTheme = NULL;
        HWND            _hGrpPlaylist = NULL, _hGrpDewMode = NULL, _hGrpEngine = NULL;
        HWND            _hCmdResetTheme = NULL, _hCmdApply = NULL, _hCmdCancel = NULL, _hCmdOK = NULL;
        HWND            _hLstHotkeys = NULL;
        HWND            _hToolTip = NULL;
        HWND            _hTabSettings = NULL;
        HFONT           _hFntUI = NULL, _hFntTab = NULL;
        LPDEWICONREPOSITORY _lpIconRepo = NULL;

        int             _iDPI = 96;
        float           _fScale = 1.0f;
        int             _iMinWidth;
        int             _iMinHeight;
        BOOL            _bIsCreated = FALSE;
        const int       _GRP_HOTKEY_ALL_MODE = 10;
        const int       _GRP_HOTKEY_DEW_MODE = 20;
        const int       _GRP_HOTKEY_LAYOUT_MODE = 30;
        const int       _GRP_HOTKEY_PLAYLIST = 40;
        const int       _GRP_CMDLINE = 50;

        static LRESULT  CALLBACK _StgsMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);
        virtual void    _ApplyChildFont();
        virtual void    _SetTip(HWND hCtrl, LPCWSTR wsTip);
        virtual void    _AddHotkey(int iGroupID, LPWSTR wsHotkey, LPWSTR wsAction, BOOL bFlushAll = FALSE);
        virtual void    _HandleSizing();
        virtual void    _HandleTabContents();
        virtual void    _DeleteUIComponents();
        virtual LRESULT _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

        #pragma endregion

    public:

        #pragma region CORE SETTINGS SECTION

        // Restrict the settings with 'const' for the attributes that
        // DEWWINDOW will not modify
        const DEWTHEME& Theme;
        const BYTE&     MinimizeAction;
        const BYTE&     CloseAction;
        DWORD&          VolumeLevel;
        BYTE&           RepeatLevel;
        BYTE&           PlaylistVisible;
        wchar_t*        ThemeFile;
        const BYTE&     RememberHistory;
        DWORD&          HistoryFileCount;
        const BYTE&     TopmostInDewMode;
        const BYTE&     NotifyOnSongChange;
        const BYTE&     DeepScan;
        LPFILELIST&     SongList;
        HWND            FocusWindowHandle = NULL;
        LPDWORD         ComponentPos;
        LPBOOL          ComponentVisibility;
        BYTE&           ShowTitle;
        BYTE&           DewMode;

                        DEWSETTINGS();
                        ~DEWSETTINGS();

        #pragma endregion

        #pragma region SETTINGS UI SECTION

        HWND            ShowSettings(HWND hWndParent);

        #pragma endregion

} DEWSETTINGS, *LPDEWSETTINGS;

#endif // _DEWSETTINGS_H_
