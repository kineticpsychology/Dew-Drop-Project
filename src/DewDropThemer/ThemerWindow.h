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

#ifndef _THEMERWINDOW_H_
#define _THEMERWINDOW_H_

#include "ThemePasswordWindow.h"
#include "ThemerAboutWindow.h"

#define THEMER_COMPONENT_SEQ_WINDOW     0
#define THEMER_COMPONENT_SEQ_UI_BUTTONS 1
#define THEMER_COMPONENT_SEQ_MM_BUTTONS 2
#define THEMER_COMPONENT_SEQ_MODULES    3

// Set these states (can be combination of one or more)
// before handing control over to the password window
// We will take action from the _WndProc after checking the value
#define THEMER_TRANSITION_STATE_NONE        0x0000    // Keep window open after returning from the password window
#define THEMER_TRANSITION_STATE_CLOSE       0x0100    // Close theme file after returning from the password window
#define THEMER_TRANSITION_STATE_QUIT        0x1000    // Close main window after returning from the password window
#define THEMER_TRANSITION_STATE_SAVE        0x0001    // Save with _wsThemeFilePath after returning from the password window
#define THEMER_TRANSITION_STATE_SAVEAS      0x0002    // Save As (by browsing) after returning from the password window

typedef class THEMERWINDOW
{
    private:
        // Re-usable classes
        DEWSETTINGS         _DefaultSettings;
        const DEWTHEME&     _DefaultTheme;
        LPDEWWINDOW         _lpPreviewWindow = NULL;
        DEWTHEME            _CurrentTheme, _OriginalTheme;

        // Application UI States
        static const BYTE   _UI_STATE_READY = 0x00;
        static const BYTE   _UI_STATE_NEW_NO_CHANGE = 0x01;
        static const BYTE   _UI_STATE_NEW_CHANGED = 0x02;
        static const BYTE   _UI_STATE_OPEN_NO_CHANGE = 0x03;
        static const BYTE   _UI_STATE_OPEN_CHANGED = 0x04;

        // Container Window specifics
        HINSTANCE           _hInstance = NULL;
        HWND                _hWnd = NULL;
        HDC                 _hDC = NULL;
        HWND                _hWndFocus = NULL;
        HWND                _hStsInfo = NULL;
        HWND                _hCmdPwd = NULL;
        HWND                _hCmdClose = NULL;
        HWND                _hToolTip = NULL;
        HMENU               _hMnuMain = NULL;
        HACCEL              _hAccel = NULL;
        LPTHEMEPASSWORDWINDOW   _lpPassWindow = NULL;
        LPTHEMERABOUTWINDOW _lpAboutWindow = NULL;
        LPDEWICONREPOSITORY _lpIconRepo = NULL;

        HFONT               _hFntUI = NULL, _hFntTitle = NULL, _hFntText = NULL;
        HBRUSH              _hbrBack = NULL, _hbrBack2 = NULL, _hbrText = NULL, _hbrOutline = NULL;
        BOOL                _bLivePreview = TRUE;
        int                 _iXOffset, _iYOffset, _iSBHeight; // Padding values
        int                 _iDPI = 96;
        float               _fScale = 1.0f;
        wchar_t             _wsThemeFilePath[MAX_CHAR_PATH];
        wchar_t             _wsTitlePart[DEWTHEME_ATTR_LENGTH];
        const wchar_t*      _ws_NO_BACK_IMG = L"<No Background Image>";
        const wchar_t*      _ws_DEF_APP_ICON = L"<Default (Dew Drop) Icon>";
        BYTE                _btUIState;
        DWORD               _dwTransitionState = THEMER_TRANSITION_STATE_NONE; // WARNING! Do NOT set this from _HandleUIState()!
        BOOL                _bTextChangeConsideredChange = FALSE; // Text-Handler stop gap to distinguish between automated & manual text changes

        // Icons for Buttons & Menu Items
        HIMAGELIST          _hImlBtnBrowse = NULL, _hImlBtnReset = NULL, _hImlBtnPassword = NULL, _hImlBtnClose = NULL;
        HBITMAP             _hBmp_File_New = NULL;

        // Menu/Groups
        HWND                _hGrpInfo = NULL, _hGrpColors = NULL, _hGrpFonts = NULL, _hGrpIconStyle = NULL, _hGrpImages = NULL;

        // Theme Info Section
        HWND                _hLblName = NULL, _hTxtName = NULL, _hLblAuthor = NULL, _hTxtAuthor = NULL;

        // Theme Color Section
        HWND                _hCmbColorComponent = NULL;
        HWND                _hLblColor = NULL, _hLblBackColor = NULL, _hLblBackColorPreview = NULL, _hLblBackColor2 = NULL, _hLblBackColor2Preview = NULL;
        HWND                _hLblTextColor = NULL, _hLblTextColorPreview = NULL, _hLblOutlineColor = NULL, _hLblOutlineColorPreview = NULL;

        // Theme Font Section
        HWND                _hLblTitleFont = NULL, _hCmdTitleFont = NULL, _hLblTextFont = NULL, _hCmdTextFont = NULL;

        // Theme Icon Section
        HWND                _hLblIconMode = NULL, _hOptIconModeDefault = NULL, _hOptIconModeAlternate = NULL;
        HWND                _hChkTransparentIcon = NULL, _hChkSeekbarTimes = NULL;
        HWND                _hLblIconScale = NULL, _hCmbIconScale = NULL;

        // Theme Image Section
        HWND                _hLblBackground = NULL, _hLblAppIcon = NULL;
        HWND                _hTxtBackground = NULL, _hTxtAppIcon = NULL;
        HWND                _hCmdBackground = NULL, _hCmdResetBackground = NULL, _hCmdAppIcon = NULL, _hCmdResetAppIcon = NULL;

        // UI Handler functions
        static LRESULT      CALLBACK _ThemerMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);
        void                _HandleUIState(BYTE btNewState, BOOL bReflectChanges = TRUE);
        LRESULT             _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        void                _CreateMainMenu();
        void                _HandleSizing();
        void                _SetTooltip(HWND hCtrl, LPCWSTR wsTipText);

        // Theme value application functions
        void                _ApplyUIFont();
        void                _ApplyTitleFont();
        void                _ApplyTextFont();
        void                _SetCTLColorBrushes(BOOL bDelOnly = FALSE);
        void                _SetupUIWithCurrThemeValues();

        // Direct Menu functions
        void                _File_New_Theme();
        void                _File_Open_Theme();
        void                _File_Save_Theme(LPCWSTR wsThemeFilePath);
        void                _File_SaveAs_Theme();
        void                _File_Close_Theme(); // A close will be triggered when exiting the app. Need to know the outcome.
        void                _Theme_Reset();
        void                _Theme_Default();

        // Core Functionalities
        void                _EnableMenuItem(UINT wID, BOOL bEnable = TRUE);
        void                _OpenTheme(LPCWSTR wsThemeFile, BOOL bFromMenu = FALSE);
        BOOL                _SetFont(LPWSTR wsFontName, int *piFontSize, BOOL *pBold, BOOL *pItalic);
        BOOL                _SetColor(COLORREF *pCrResult);
        BOOL                _SetBackgroundImage();
        BOOL                _ResetBackgroundImage();
        BOOL                _SetAppIcon();
        BOOL                _ResetAppIcon();
        void                _FlagChange();

    public:
                            THEMERWINDOW();
        int                 Run();
                            ~THEMERWINDOW();
} THEMERWINDOW, *LPTHEMERWINDOW;

#endif  // _THEMERWINDOW_H_
