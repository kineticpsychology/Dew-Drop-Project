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

#include "DewSettings.h"

LRESULT CALLBACK DEWSETTINGS::_StgsMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    LRESULT     lrRetVal;
    HWND        hWndParent = NULL;
    HWND        hWndThis = NULL;
    UINT        nCode_WM_APPLY = 0;
    UINT        nCode_WM_CANCEL = 0;
    BOOL        bThemeNeedsRefresh = FALSE;

    lrRetVal = ((LPDEWSETTINGS)dwRefData)->_WndProc(hWnd, nMsg, wParam, lParam);
    if (nMsg == WM_COMMAND)
    {
        hWndParent = ((LPDEWSETTINGS)dwRefData)->_hWndParent;
        hWndThis = ((LPDEWSETTINGS)dwRefData)->_hWndSettings;
        nCode_WM_APPLY = ((LPDEWSETTINGS)dwRefData)->_WM_APPLY;
        nCode_WM_CANCEL = ((LPDEWSETTINGS)dwRefData)->_WM_CANCEL;
        bThemeNeedsRefresh = ((LPDEWSETTINGS)dwRefData)->_bThemeNeedsRefresh;
        DEWTHEME& dewTheme = ((LPDEWSETTINGS)dwRefData)->_dewTheme;

        if (lParam == 0 && LOWORD(wParam) == IDCANCEL)
        {
            SendMessage(hWndThis, WM_CLOSE, 0, 0);
            return lrRetVal;
        }

        // Just a friendly notification to the parent window that
        // the style has changed and needs to be applied to all the (sub)components
        if ((UINT)lrRetVal == nCode_WM_APPLY && hWndParent != NULL && bThemeNeedsRefresh)
        {
            PostMessage(hWndParent, WM_DEWMSG_STYLE_CHANGED, 0, 0);
        }
        if (((UINT)lrRetVal == nCode_WM_APPLY || (UINT)lrRetVal == nCode_WM_CANCEL)
            && hWndParent != NULL)
        {
            PostMessage(hWndParent, WM_DEWMSG_CHILD_CLOSED, 0, (LPARAM)hWndThis);
            SetForegroundWindow(hWndParent);
        }
    }
    return lrRetVal;
}

void DEWSETTINGS::_ApplyChildFont()
{
    HWND        hWndChild = NULL;
    LOGFONT     lgfFont { 0 };

    if (_hFntTab) { DeleteObject(_hFntTab); _hFntTab = NULL; }
    if (_hFntUI) { DeleteObject(_hFntUI); _hFntUI = NULL; }

    lgfFont.lfHeight = -MulDiv(8, _iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = FW_NORMAL;
    lgfFont.lfItalic = 0;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, L"Tahoma", 32 * sizeof(wchar_t));
    _hFntUI = CreateFontIndirect(&lgfFont);

    lgfFont.lfWeight = FW_BOLD;
    lgfFont.lfItalic = FALSE;
    _hFntTab = CreateFontIndirect(&lgfFont);

    hWndChild = FindWindowEx(_hWndSettings, NULL, NULL, NULL);
    if(!hWndChild) return;

    do
    {
        hWndChild = FindWindowEx(_hWndSettings, hWndChild, NULL, NULL);
        if(hWndChild)
            SendMessage(hWndChild, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));
    }while(hWndChild);

    // Specialized font for tab strips to highlight them
    SendMessage(_hTabSettings, WM_SETFONT, (WPARAM)_hFntTab, MAKELPARAM(TRUE, 0));

    return;
}

void DEWSETTINGS::_SetTip(HWND hCtrl, LPCWSTR wsTip)
{
    TOOLINFO ti { 0 };

    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hCtrl;
    ti.hinst = _hInstance;
    ti.lpszText = (LPWSTR)wsTip;
    GetClientRect(hCtrl, &ti.rect);

    SendMessage(_hToolTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    return;
}

void DEWSETTINGS::_AddHotkey(int iGroupID, LPWSTR wsHotkey, LPWSTR wsAction, BOOL bFlushAll)
{
    LVITEM      lvItem { 0 };
    int         iIndex;

    if (bFlushAll) ListView_DeleteAllItems(_hLstHotkeys);

    iIndex = ListView_GetItemCount(_hLstHotkeys);
    lvItem.mask = LVIF_TEXT | LVIF_GROUPID;
    lvItem.iItem = iIndex;
    lvItem.iSubItem = 0;
    lvItem.pszText = wsHotkey;
    lvItem.iGroupId = iGroupID;
    SendMessage(_hLstHotkeys, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = iIndex;
    lvItem.iSubItem = 1;
    lvItem.pszText = wsAction;
    lvItem.cchTextMax = lstrlen(wsAction);
    SendMessage(_hLstHotkeys, LVM_SETITEM, 0, (LPARAM)&lvItem);

    ListView_SetColumnWidth(_hLstHotkeys, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(_hLstHotkeys, 1, LVSCW_AUTOSIZE);

    return;
}

void DEWSETTINGS::_HandleSizing()
{
    int         iX, iY, iW, iH;
    RECT        rctClient { 0 };
    wchar_t     wsText[MAX_CHAR_PATH];

    GetClientRect(_hWndSettings, &rctClient);

    // Tab Section -------------------------------------------------------------
    SetWindowPos(_hTabSettings, 0, rctClient.left, rctClient.top, rctClient.right,
                 rctClient.bottom, SWP_NOZORDER);
    // -------------------------------------------------------------------------

    // Minimize Section --------------------------------------------------------
    iX = _F(10); iY = _F(50); iW = rctClient.right - (_F(20)); iH = _F(40);
    SetWindowPos(_hGrpMinAction, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(20); iY = _F(65); iW = _F(200); iH = _F(20);
    SetWindowPos(_hLblMinAction, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(230); iY = _F(65); iW = _F(105); iH = _F(20);
    SetWindowPos(_hOptMinMinimize, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(345); iY = _F(65); iW = _F(105); iH = _F(20);
    SetWindowPos(_hOptMinTray, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------

    // Close Section -----------------------------------------------------------
    iX = _F(10); iY = _F(100); iW = rctClient.right - (_F(20)); iH = _F(40);
    SetWindowPos(_hGrpCloseAction, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(20); iY = _F(115); iW = _F(200); iH = _F(20);
    SetWindowPos(_hLblCloseAction, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(230); iY = _F(115); iW = _F(105); iH = _F(20);
    SetWindowPos(_hOptCloseQuit, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(345); iY = _F(115); iW = _F(105); iH = _F(20);
    SetWindowPos(_hOptCloseTray, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------

    // Theme Section -----------------------------------------------------------
    // Theme Name & Author Labels
    iX = _F(10); iY = _F(150); iW = rctClient.right - (_F(20)); iH = _F(100);
    SetWindowPos(_hGrpTheme, 0, iX, iY, iW, iH, SWP_NOZORDER);
    StringCchPrintf(wsText, MAX_CHAR_PATH, L"Theme name: %s", _dewTheme.Name);
    iX = _F(20); iY = _F(170); iW = rctClient.right - (_F(40)); iH = _F(20);
    SetWindowText(_hLblThemeName, wsText);
    SetWindowPos(_hLblThemeName, 0, iX, iY, iW, iH, SWP_NOZORDER);
    StringCchPrintf(wsText, MAX_CHAR_PATH, L"Author: %s", _dewTheme.Author);
    iX = _F(20); iY = _F(195); iW = rctClient.right - _F(120); iH = _F(20);
    SetWindowText(_hLblThemeAuthor, wsText);
    SetWindowPos(_hLblThemeAuthor, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // Theme Browser Controls
    iX = _F(20); iY = _F(220); iW = _F(75); iH = _F(20);
    SetWindowPos(_hLblThemeFile, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(100); iY = _F(220); iW = rctClient.right - _F(180) - _F(50); iH = _F(20);
    SetWindowPos(_hTxtThemeFile, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = rctClient.right - _F(124); iY -= _F(2); iW = iH = _F(24);
    SetWindowPos(_hCmdBrowseThemeFile, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // Reset Theme Button
    iX = rctClient.right - _F(95); iY = _F(212); iW = _F(75); iH = _F(30);
    SetWindowPos(_hCmdResetTheme, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------

    // Playlist Section --------------------------------------------------------
    iX = _F(10); iY = _F(260); iW = rctClient.right - _F(20); iH = _F(40);
    SetWindowPos(_hGrpPlaylist, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(20); iY = _F(275); iW = rctClient.right - _F(40); iH = _F(20);
    SetWindowPos(_hChkRememberHist, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------

    // Dew Mode Section --------------------------------------------------------
    iX = _F(10); iY = _F(310); iW = rctClient.right - _F(20); iH = _F(40);
    SetWindowPos(_hGrpDewMode, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(20); iY = _F(325); iW = rctClient.right - _F(40); iH = _F(20);
    SetWindowPos(_hChkDewModeTopmost, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------

    // Engine Section ----------------------------------------------------------
    iX = _F(10); iY = _F(360); iW = rctClient.right - _F(20); iH = _F(70);
    SetWindowPos(_hGrpEngine, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(20); iY = _F(375); iW = rctClient.right - _F(40); iH = _F(20);
    SetWindowPos(_hChkNotifyOnSongChange, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _F(20); iY = _F(400); iW = rctClient.right - _F(40); iH = _F(20);
    SetWindowPos(_hChkDeepScan, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------

    // Apply/Cancel Section -------------------------------------------------------
    iX = rctClient.right - _F(85); iY = rctClient.bottom - _F(40); iW = _F(75); iH = _F(30);
    SetWindowPos(_hCmdApply, 0, iX, iY, iW, iH, SWP_NOZORDER);

    iX = _F(10); iY = rctClient.bottom - _F(40); iW = _F(75); iH = _F(30);
    SetWindowPos(_hCmdCancel, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------


    // View-only Hotkeys section -----------------------------------------------
    iX = _F(10); iY = _F(60); iW = rctClient.right - (_F(20)); iH = rctClient.bottom - _F(110);
    SetWindowPos(_hLstHotkeys, 0, iX, iY, iW, iH, SWP_NOZORDER);

    // OK Button will be in the same spot as the 'Apply' button
    iX = rctClient.right - _F(85); iY = rctClient.bottom - _F(40); iW = _F(75); iH = _F(30);
    SetWindowPos(_hCmdOK, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------

    // Tooltips:
    this->_SetTip(_hOptMinMinimize, L"Select to minimize Dew Drop Player to taskbar");
    this->_SetTip(_hOptMinTray, L"Select to minimize Dew Drop Player to the system tray");
    this->_SetTip(_hOptCloseQuit, L"Select to quit Dew Drop Player completely");
    this->_SetTip(_hOptCloseTray, L"Select to hide Dew Drop Player to the system tray");
    this->_SetTip(_hCmdResetTheme, L"Remove currently applied theme and revert to the default style");
    this->_SetTip(_hCmdBrowseThemeFile, L"Browse and select a compatible theme(skin) file");
    this->_SetTip(_hChkRememberHist, L"Select to make Dew Drop Player remember the list of songs before exiting");
    this->_SetTip(_hChkDewModeTopmost, L"Select to make Dew Drop Player stay on top of all windows (Dew Mode only)");
    this->_SetTip(_hChkNotifyOnSongChange, L"Select to get a notification, everytime a song starts playing (only when in tray)");
    this->_SetTip(_hChkDeepScan, L"Select to read a few bytes of the file while enqueuing (more accurate, performance intensive)");
    this->_SetTip(_hCmdApply, L"Apply your changes and close the settings dialog");
    this->_SetTip(_hCmdCancel, L"Discard your changes and close the settings dialog");
    // -------------------------------------------------------------------------

    return;
}

void DEWSETTINGS::_HandleTabContents()
{
    BOOL    bConfigTab;
    HWND    hWndChild = NULL;

    bConfigTab = TabCtrl_GetCurSel(_hTabSettings) == 0;
    do
    {
        hWndChild = FindWindowEx(_hWndSettings, hWndChild, NULL, NULL);
        if (hWndChild)
            ShowWindow(hWndChild, bConfigTab ? SW_SHOW : SW_HIDE);
    } while(hWndChild);

    ShowWindow(_hTabSettings, SW_SHOW);
    ShowWindow(_hLstHotkeys, bConfigTab ? SW_HIDE : SW_SHOW);
    ShowWindow(_hCmdOK, bConfigTab ? SW_HIDE : SW_SHOW);
    return;
}

LRESULT DEWSETTINGS::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    wchar_t             wsText[MAX_CHAR_PATH] { 0 };
    wchar_t             wsThemeName[DEWTHEME_ATTR_LENGTH] { 0 };
    wchar_t             wsThemeAuthor[DEWTHEME_ATTR_LENGTH] { 0 };
    WORD                wDefThemeVerMajor, wDefThemeVerMinor;
    WORD                wImportThemeVerMajor, wImportThemeVerMinor;
    BYTE                btThemeApply;
    OPENFILENAME        ofn { 0 };
    LPNMHDR             lpNMH;
    LPWINDOWPOS         lpPos;
    RECT                rctWorkArea;

    switch(nMsg)
    {
        case WM_NOTIFY:
        {
            lpNMH = (LPNMHDR)lParam;
            if (lpNMH->hwndFrom == _hTabSettings)
            {
                if (lpNMH->code == TCN_SELCHANGE)
                    this->_HandleTabContents();
            }
            break;
        }
        case WM_SIZE:
        {
            this->_HandleSizing();
            return FALSE;
        }
        case WM_GETMINMAXINFO:
        {
            ((LPMINMAXINFO)lParam)->ptMinTrackSize.x = _iMinWidth;
            ((LPMINMAXINFO)lParam)->ptMinTrackSize.y = _iMinHeight;
            return FALSE;
        }
        // Do not allow the window to move out of the work area
        case WM_WINDOWPOSCHANGING:
        {
            SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);
            lpPos = (LPWINDOWPOS)lParam;
            if (lpPos->cx + lpPos->x > rctWorkArea.right) lpPos->x = rctWorkArea.right - lpPos->cx;
            if (lpPos->x < rctWorkArea.left) lpPos->x = rctWorkArea.left;
            if (lpPos->cy + lpPos->y > rctWorkArea.bottom) lpPos->y = rctWorkArea.bottom - lpPos->cy;
            if (lpPos->y < rctWorkArea.top) lpPos->y = rctWorkArea.top;

            return 0;
        }
        case WM_COMMAND:
        {
            if ((HWND)lParam == _hCmdCancel || (HWND)lParam == _hCmdOK)
            {
                ShowWindow(_hWndSettings, SW_HIDE);
                return _WM_CANCEL;
            }
            else if ((HWND)lParam == _hCmdApply)
            {
                _settings._btCloseAction = (SendMessage(_hOptCloseQuit, BM_GETCHECK, 0, 0) == BST_CHECKED) ? DEWOPT_CLOSE_QUIT : DEWOPT_CLOSE_TRAY;
                _settings._btMinimizeAction = (SendMessage(_hOptMinMinimize, BM_GETCHECK, 0, 0) == BST_CHECKED) ? DEWOPT_MINIMIZE_MINIMIZE : DEWOPT_MINIMIZE_TRAY;
                _settings._bRememberHistory = (SendMessage(_hChkRememberHist, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
                _settings._bTopmostInDewMode = (SendMessage(_hChkDewModeTopmost, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
                _settings._bNotifyOnSongChange = (SendMessage(_hChkNotifyOnSongChange, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
                _settings._bDeepScan = (SendMessage(_hChkDeepScan, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
                GetWindowText(_hTxtThemeFile, wsText, MAX_CHAR_PATH);
                if (lstrcmpi(wsText, _wsDefaultThemeRepr))
                {
                    // Refreshes are costly. Do a refresh only if the theme file has changed from a differently previous one
                    if (lstrcmpi(_settings._wsThemeFile, wsText)) _bThemeNeedsRefresh = TRUE;

                    btThemeApply = _dewTheme.LoadThemeFile(wsText, &wImportThemeVerMajor, &wImportThemeVerMinor);
                    if (btThemeApply == DEWTHEME_ACTION_THEME_APPLIED)
                    {
                        StringCchPrintf(_settings._wsThemeFile, MAX_CHAR_PATH, wsText);
                    }
                    else
                    {
                        StringCchPrintf(_settings._wsThemeFile, MAX_CHAR_PATH, L".");
                        MessageBox(_hWndSettings, L"The provided theme could not be applied. Reverting to default theme.",
                                   L"Theme Error", MB_OK | MB_ICONEXCLAMATION);
                    }
                }
                else
                {
                    // Refreshes are costly. Do a refresh only if the theme file has changed from a differently previous one
                    if (lstrcmpi(_settings._wsThemeFile, L".")) _bThemeNeedsRefresh = TRUE;
                    StringCchPrintf(_settings._wsThemeFile, MAX_CHAR_PATH, L".");
                    _dewTheme.Reset();
                }
                ShowWindow(_hWndSettings, SW_HIDE);
                return _WM_APPLY;
            }
            else if ((HWND)lParam == _hCmdBrowseThemeFile)
            {
                DEWTHEME::GetDefaultThemeVersion(&wDefThemeVerMajor, &wDefThemeVerMinor);

                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = _hWndSettings;
                ofn.lpstrFilter = L"DewDrop Theme Files (*.dth)\0*.dth\0\0";
                ofn.lpstrFile = wsText;
                ofn.nMaxFile = MAX_CHAR_PATH;
                ofn.lpstrTitle = L"Select a DewDrop theme file";
                ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
                if (GetOpenFileName(&ofn))
                {
                    btThemeApply = _dewTheme.LoadThemeFile(wsText, &wImportThemeVerMajor, &wImportThemeVerMinor, TRUE, wsThemeName, wsThemeAuthor);
                    if (btThemeApply == DEWTHEME_ACTION_THEME_APPLIED)
                    {
                        this->_TrimPathForLocalPath(wsText);
                        SetWindowText(_hTxtThemeFile, wsText);
                        StringCchPrintf(wsText, MAX_CHAR_PATH, L"Theme name: %s", wsThemeName);
                        SetWindowText(_hLblThemeName, wsText);
                        StringCchPrintf(wsText, MAX_CHAR_PATH, L"Author: %s", wsThemeAuthor);
                        SetWindowText(_hLblThemeAuthor, wsText);
                    }
                    else if (btThemeApply == DEWTHEME_ACTION_THEME_VER_MISMATCH)
                    {
                        StringCchPrintf(wsText, MAX_CHAR_PATH, L"The selected theme (version %u.%u) is " \
                                        L"not compatible with the current version of " \
                                        L"theme (version %u.%u) supported by Dew Drop Player. " \
                                        L"Please select a version compatible theme.",
                                        wImportThemeVerMajor, wImportThemeVerMinor,
                                        wDefThemeVerMajor, wDefThemeVerMinor);
                        MessageBox(_hWndSettings, wsText, L"Theme Error", MB_OK | MB_ICONEXCLAMATION);

                    }
                    else
                    {
                        MessageBox(_hWndSettings, L"The provided theme could not be loaded. Retaining the previous theme.",
                                   L"Theme Error", MB_OK | MB_ICONEXCLAMATION);
                    }
                }
            }
            else if ((HWND)lParam == _hCmdResetTheme)
            {
                StringCchPrintf(wsText, MAX_CHAR_PATH, L"Theme name: %s", DEWTHEME_DEFAULT_NAME);
                SetWindowText(_hLblThemeName, wsText);
                StringCchPrintf(wsText, MAX_CHAR_PATH, L"Author: %s", DEWTHEME_DEFAULT_AUTHOR);
                SetWindowText(_hLblThemeAuthor, wsText);
                SetWindowText(_hTxtThemeFile, _wsDefaultThemeRepr);
            }
            break;
        }
        case WM_CLOSE:
        {
            // Just Hide the window. Re-show when settings is clicked again
            // from the main application. Saves some time with setting up the
            // whole window all over again
            ShowWindow(_hWndSettings, SW_HIDE);
            PostMessage(_hWndParent, WM_DEWMSG_CHILD_CLOSED, 0, (LPARAM)_hWndSettings);
            SetForegroundWindow(_hWndParent);
            return TRUE;
        }
    }
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

HWND DEWSETTINGS::ShowSettings(HWND hWndParent)
{
    int             iWinWidth, iWinHeight;
    int             iScreenWidth, iScreenHeight, iXPos, iYPos;
    RECT            rctParent, rctWorkArea, rctThis;

    if (!_bIsCreated)
    {
        iWinWidth = _iMinWidth;
        iWinHeight = _iMinHeight;
    }
    else
    {
        GetWindowRect(_hWndSettings, &rctThis);
        iWinWidth = rctThis.right - rctThis.left;
        iWinHeight = rctThis.bottom - rctThis.top;
    }
    _bThemeNeedsRefresh = FALSE;
    _hWndParent = hWndParent;

    if (_hWndParent == NULL)
    {
        iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
        iXPos = (iScreenWidth - iWinWidth)/2;
        iYPos = (iScreenHeight - iWinHeight)/2;
    }
    else
    {
        GetWindowRect(_hWndParent, &rctParent);
        iScreenWidth = rctParent.right - rctParent.left;
        iScreenHeight = rctParent.bottom - rctParent.top;
        iXPos = (iScreenWidth - iWinWidth)/2 + rctParent.left;
        iYPos = (iScreenHeight - iWinHeight)/2 + rctParent.top;
    }

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);
    if (iXPos < rctWorkArea.left) iXPos = rctWorkArea.left;
    if (iYPos < rctWorkArea.top) iYPos = rctWorkArea.top;
    if ((iXPos + iWinWidth) > rctWorkArea.right) iXPos = rctWorkArea.right - iWinWidth;
    if ((iYPos + iWinHeight) > rctWorkArea.bottom) iYPos = rctWorkArea.bottom - iWinHeight;


    if (!_bIsCreated)
    {
        WNDCLASSEX          wcex;
        TCITEM              tci { 0 };
        LVCOLUMN            lvc { 0 };
        LVGROUP             lvg { 0 };
        RECT                rctMrg;
        BUTTON_IMAGELIST    bImg { 0 };

        const wchar_t   *wsSettingsClass = L"DEWDROP.SETTINGS.WND";
        const wchar_t   *wsTitle = L"Dew Drop Player - Settings";
        const DWORD     dwWinStyle = WS_OVERLAPPEDWINDOW;
        const DWORD     dwTabStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_SINGLELINE | TCS_BUTTONS | TCS_FLATBUTTONS;
        const DWORD     dwLblStyle = WS_CHILD | WS_VISIBLE;
        const DWORD     dwBtnStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
        const DWORD     dwOptStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON;
        const DWORD     dwChkStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX;
        const DWORD     dwCmbStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST;
        const DWORD     dwGrpStyle = WS_CHILD | WS_VISIBLE | BS_GROUPBOX;
        const DWORD     dwTxtStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_READONLY;
        const DWORD     dwLstStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_NOCOLUMNHEADER;
        const DWORD     dwLstExStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;

        wcex.cbSize         = sizeof(WNDCLASSEX);
        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc    = DefWindowProc;
        wcex.cbClsExtra     = 0;
        wcex.cbWndExtra     = 0;
        wcex.hInstance      = _hInstance;
        wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
        wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
        wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground  = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        wcex.lpszMenuName   = NULL;
        wcex.lpszClassName  = wsSettingsClass;

        RegisterClassEx(&wcex);

        _hWndSettings = CreateWindowEx(WS_EX_TOOLWINDOW, wsSettingsClass, wsTitle, dwWinStyle,
                                       iXPos, iYPos, iWinWidth, iWinHeight,
                                       _hWndParent, NULL, _hInstance, NULL);

        _hTabSettings = CreateWindowEx(0, WC_TABCONTROL, L"", dwTabStyle, 0, 0,
                                       0, 0, _hWndSettings, NULL, _hInstance, NULL);

        _hLblCloseAction = CreateWindowEx(0, L"STATIC", L"When the CLOSE button is clicked:",
                                          dwLblStyle, 0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hLblMinAction = CreateWindowEx(0, L"STATIC", L"When the MINIMIZE button is clicked:",
                                        dwLblStyle, 0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hLblThemeFile = CreateWindowEx(0, L"STATIC", L"Theme file:", dwLblStyle,
                                        0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hLblThemeName = CreateWindowEx(0, L"STATIC", L"Theme name:", dwLblStyle,
                                        0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hLblThemeAuthor = CreateWindowEx(0, L"STATIC", L"Author:", dwLblStyle,
                                          0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);

        _hGrpMinAction = CreateWindowEx(0, L"BUTTON", L"Minimize Button Settings", dwGrpStyle,
                                        0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hGrpCloseAction = CreateWindowEx(0, L"BUTTON", L"Close Button Settings", dwGrpStyle,
                                          0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hGrpTheme = CreateWindowEx(0, L"BUTTON", L"Theme Settings", dwGrpStyle,
                                    0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hGrpPlaylist = CreateWindowEx(0, L"BUTTON", L"Playlist Settings", dwGrpStyle,
                                       0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hGrpDewMode = CreateWindowEx(0, L"BUTTON", L"Dew Mode Settings", dwGrpStyle,
                                      0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hGrpEngine = CreateWindowEx(0, L"BUTTON", L"Engine Settings", dwGrpStyle,
                                     0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);

        _hOptMinMinimize = CreateWindowEx(0, L"BUTTON", L"Minimize player", dwOptStyle | WS_GROUP,
                                        0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hOptMinTray = CreateWindowEx(0, L"BUTTON", L"Minimize to tray", dwOptStyle,
                                            0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);

        _hOptCloseQuit = CreateWindowEx(0, L"BUTTON", L"Quit player", dwOptStyle | WS_GROUP,
                                        0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hOptCloseTray = CreateWindowEx(0, L"BUTTON", L"Minimize to tray", dwOptStyle,
                                        0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);

        _hTxtThemeFile = CreateWindowEx(WS_EX_STATICEDGE, L"EDIT", L"", dwTxtStyle,
                                        0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hCmdBrowseThemeFile = CreateWindowEx(0, L"BUTTON", L"...", dwBtnStyle,
                                              0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hCmdResetTheme = CreateWindowEx(0, L"BUTTON", L"Reset", dwBtnStyle,
                                         0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);

        _hChkRememberHist = CreateWindowEx(0, L"BUTTON", L"Remember playlist &history on exit", dwChkStyle,
                                           0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hChkDewModeTopmost = CreateWindowEx(0, L"BUTTON", L"Make window topmost in &Dew Mode", dwChkStyle,
                                           0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hChkNotifyOnSongChange = CreateWindowEx(0, L"BUTTON", L"Get a tray &notification when a song starts playing",
                                                 dwChkStyle, 0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hChkDeepScan = CreateWindowEx(0, L"BUTTON", L"Enable deep &scan of files while enqueuing (restart required)",
                                       dwChkStyle, 0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);

        _hCmdApply = CreateWindowEx(0, L"BUTTON", L"&Apply", dwBtnStyle,
                                    0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);
        _hCmdCancel = CreateWindowEx(0, L"BUTTON", L"&Cancel", dwBtnStyle,
                                     0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);

        _hLstHotkeys = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_COMPOSITED, WC_LISTVIEW, L"", dwLstStyle,
                                      0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);

        _hCmdOK = CreateWindowEx(0, L"BUTTON", L"&OK", dwBtnStyle,
                                     0, 0, 0, 0, _hWndSettings, NULL, _hInstance, NULL);

        _hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
                                   WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   _hWndSettings, NULL, NULL, NULL);
        SetWindowPos(_hToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        // WARNING! Do not move this around! Since DEWSETTINGS is instantiated
        //          without explicit pointer, it will trigger before
        //          GdiplusStartup is called. If we move this to the CTOR()
        //          FontFamily collection will be called even before GDI+ is
        //          initialized! This will crash the application. Additionally,
        //          all subsequent calls to the button icons use this object's
        //          images. So the images MUST be ready for use before those calls.
        _lpIconRepo = new DEWICONREPOSITORY(_iDPI, _fScale);

        rctMrg.left = _F(5);
        rctMrg.top = _F(1);
        rctMrg.right = _F(1);
        rctMrg.bottom = _F(1);
        bImg.margin = rctMrg;
        bImg.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;

        bImg.himl = _lpIconRepo->ImgResetTheme;
        SendMessage(_hCmdResetTheme, BCM_SETIMAGELIST, 0, (LPARAM)&bImg);

        bImg.himl = _lpIconRepo->ImgCancel;
        SendMessage(_hCmdCancel, BCM_SETIMAGELIST, 0, (LPARAM)&bImg);

        bImg.himl = _lpIconRepo->ImgOK;
        SendMessage(_hCmdApply, BCM_SETIMAGELIST, 0, (LPARAM)&bImg);

        bImg.himl = _lpIconRepo->ImgOK;
        SendMessage(_hCmdOK, BCM_SETIMAGELIST, 0, (LPARAM)&bImg);

        // Tab control initializations -----------------------------------------
        TabCtrl_SetPadding(_hTabSettings, _F(10), _F(10));
        TabCtrl_SetImageList(_hTabSettings, _lpIconRepo->ImgTab);

        tci.mask = TCIF_TEXT | TCIF_IMAGE;
        tci.pszText = L"Configuration Options";
        tci.cchTextMax = 21;
        tci.iImage = 0;
        TabCtrl_InsertItem(_hTabSettings, 0, &tci);

        tci.mask = TCIF_TEXT | TCIF_IMAGE;
        tci.pszText = L"Hot Keys";
        tci.cchTextMax = 8;
        tci.iImage = 1;
        TabCtrl_InsertItem(_hTabSettings, 1, &tci);
        // ---------------------------------------------------------------------


        // ListView control initializations ------------------------------------
        SendMessage(_hLstHotkeys, LVM_SETEXTENDEDLISTVIEWSTYLE, dwLstExStyle, dwLstExStyle);
        ListView_EnableGroupView(_hLstHotkeys, TRUE);

        lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;
        lvc.fmt = LVCFMT_LEFT;

        lvc.pszText = (LPWSTR)L"Hot Key";
        lvc.cchTextMax = 7;
        lvc.iSubItem = 0;
        ListView_InsertColumn(_hLstHotkeys, 0, &lvc);

        lvc.pszText = (LPWSTR)L"Action";
        lvc.cchTextMax = 6;
        lvc.iSubItem = 1;
        ListView_InsertColumn(_hLstHotkeys, 1, &lvc);

        lvg.cbSize = sizeof(LVGROUP);
        lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_STATE | LVGF_ALIGN;
        lvg.state = LVGS_COLLAPSIBLE;
        lvg.uAlign = LVGA_HEADER_CENTER;

        lvg.pszHeader = L"Hotkey Combo (Normal Mode & Common)";
        lvg.cchHeader = 35;
        lvg.iGroupId = _GRP_HOTKEY_ALL_MODE;
        ListView_InsertGroup(_hLstHotkeys, 0, &lvg);

        lvg.pszHeader = L"Hotkey Combo (Dew Mode)";
        lvg.cchHeader = 23;
        lvg.iGroupId = _GRP_HOTKEY_DEW_MODE;
        ListView_InsertGroup(_hLstHotkeys, 1, &lvg);

        lvg.pszHeader = L"Hotkey Combo (Layout Mode)";
        lvg.cchHeader = 26;
        lvg.iGroupId = _GRP_HOTKEY_LAYOUT_MODE;
        ListView_InsertGroup(_hLstHotkeys, 2, &lvg);

        lvg.pszHeader = L"Hotkey Combo (Playlist)";
        lvg.cchHeader = 23;
        lvg.iGroupId = _GRP_HOTKEY_PLAYLIST;
        ListView_InsertGroup(_hLstHotkeys, 3, &lvg);


        lvg.pszHeader = L"Command Line Illustrations";
        lvg.cchHeader = 26;
        lvg.iGroupId = _GRP_CMDLINE;
        ListView_InsertGroup(_hLstHotkeys, 4, &lvg);

        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Media Play/Pause/Stop/Next/Previous keys>", L"Control song state (all modes and when in system tray)", TRUE);
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + <Shift> + D", L"Toggle between Normal and Dew modes");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + <Shift> + L", L"Toggle between Normal and Layout modes");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + A", L"Select all songs in the playlist (Normal mode only)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + S", L"Export the selected songs in the playlist to a .M3U8 or a .PLS file (Normal mode only)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + <Shift> + S", L"Export all the songs in the playlist to a .M3U8 or a .PLS file (Normal mode only)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + X", L"Show this Configuration/Hotkeys UI (Normal and Dew modes)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + R", L"Cycle through the repeat modes (Normal and Dew modes)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + P", L"Toggle playlist visibility (Normal mode only)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + O", L"Browse and select audio file(s) (Normal and Dew modes)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + <SHIFT> + O", L"Browse and recursively add all audio file(s) of a folder (Normal and Dew modes)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + J", L"View extended information (Normal and Dew modes)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Ctrl> + T", L"Open the auto-shutdown countdown timer (Normal and Dew modes)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<F1>", L"Show about screen (all modes)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<Keyboard Menu Key>", L"Open context menu (all modes)");
        this->_AddHotkey(_GRP_HOTKEY_ALL_MODE, L"<CTRL> + Q", L"Quit Dew Drop Player (all modes)");

        this->_AddHotkey(_GRP_HOTKEY_DEW_MODE, L"<Ctrl> + <Up>/<Down>/<Left>/<Right>", L"Snap the player to window edges (Dew mode only)");

        this->_AddHotkey(_GRP_HOTKEY_LAYOUT_MODE, L"<Up>/<Down>/<Left>/<Right>", L"Manually move components around (Layout mode only)");
        this->_AddHotkey(_GRP_HOTKEY_LAYOUT_MODE, L"<Shift> + <Up>/<Down>/<Left>/<Right>", L"Manually move components around with faster pace (Layout mode only)");

        this->_AddHotkey(_GRP_HOTKEY_PLAYLIST, L"<Enter>", L"Play the selected song from the playlist");
        this->_AddHotkey(_GRP_HOTKEY_PLAYLIST, L"<Delete>", L"Remove the selected songs from the playlist");

        this->_AddHotkey(_GRP_CMDLINE, L"<EXE Name> <File 1> <File 2> <Folder 1> ...", L"When Dew Drop player is NOT running: Opens Dew Drop player, populates the file/folder arguments to the playlist and starts playing the first song");
        this->_AddHotkey(_GRP_CMDLINE, L"<EXE Name> <File 1> <File 2> <Folder 1> ...", L"When Dew Drop player is running: Replaces the playlist of the existing player instance with the file/folder arguments and starts playing the first song");
        this->_AddHotkey(_GRP_CMDLINE, L"<EXE Name> /e <File 1> <File 2> <Folder 1> ...", L"When Dew Drop player is NOT running: Opens Dew Drop player and populates the file/folder arguments to the playlist");
        this->_AddHotkey(_GRP_CMDLINE, L"<EXE Name> /e <File 1> <File 2> <Folder 1> ...", L"When Dew Drop player is running: Replaces the playlist of the existing player instance with the file/folder arguments");

        // ---------------------------------------------------------------------

        SetWindowSubclass(_hWndSettings, _StgsMsgHandler, (UINT_PTR)_hWndSettings, (DWORD_PTR)this);
        _bIsCreated = TRUE;
    }
    else
    {
        SetWindowPos(_hWndSettings, 0, iXPos, iYPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }

    this->_ApplyChildFont();

    // Apply the current settings
    SendMessage(_hOptCloseQuit, BM_SETCHECK, (_settings._btCloseAction == DEWOPT_CLOSE_QUIT ? BST_CHECKED : BST_UNCHECKED), 0);
    SendMessage(_hOptCloseTray, BM_SETCHECK, (_settings._btCloseAction == DEWOPT_CLOSE_TRAY ? BST_CHECKED : BST_UNCHECKED), 0);

    SendMessage(_hOptMinMinimize, BM_SETCHECK, (_settings._btMinimizeAction == DEWOPT_MINIMIZE_MINIMIZE ? BST_CHECKED : BST_UNCHECKED), 0);
    SendMessage(_hOptMinTray, BM_SETCHECK, (_settings._btMinimizeAction == DEWOPT_MINIMIZE_TRAY ? BST_CHECKED : BST_UNCHECKED), 0);

    if (lstrcmpi(_settings._wsThemeFile, L"."))
        SetWindowText(_hTxtThemeFile, _settings._wsThemeFile);
    else
        SetWindowText(_hTxtThemeFile, _wsDefaultThemeRepr);

    SendMessage(_hChkRememberHist, BM_SETCHECK, (_settings._bRememberHistory ? BST_CHECKED : BST_UNCHECKED), 0);
    SendMessage(_hChkDewModeTopmost, BM_SETCHECK, (_settings._bTopmostInDewMode ? BST_CHECKED : BST_UNCHECKED), 0);
    SendMessage(_hChkNotifyOnSongChange, BM_SETCHECK, (_settings._bNotifyOnSongChange ? BST_CHECKED : BST_UNCHECKED), 0);
    SendMessage(_hChkDeepScan, BM_SETCHECK, (_settings._bDeepScan ? BST_CHECKED : BST_UNCHECKED), 0);
    this->_HandleTabContents();

    if (_hWndParent) EnableWindow(_hWndParent, FALSE);
    ShowWindow(_hWndSettings, SW_SHOWNORMAL);
    UpdateWindow(_hWndSettings);
    SetActiveWindow(_hWndSettings);

    return _hWndSettings;
}

void DEWSETTINGS::_DeleteUIComponents()
{
    HWND        hWndChild = NULL, hWndDelChild = NULL;

    DeleteObject(_hFntTab);
    DeleteObject(_hFntUI);
    if (_lpIconRepo) { delete _lpIconRepo; _lpIconRepo = NULL; }

    if (_hWndSettings != NULL)
    {
        RemoveWindowSubclass(_hWndSettings, _StgsMsgHandler, (UINT_PTR)_hWndSettings);
        hWndChild = FindWindowEx(_hWndSettings, NULL, NULL, NULL);
        if(!hWndChild)
        {
            do
            {
                hWndChild = FindWindowEx(_hWndSettings, hWndChild, NULL, NULL);
                hWndDelChild = hWndChild;
                DestroyWindow(hWndDelChild);
            } while(hWndChild);
        }
        // Since this does not contain an explicit WS_CHILD style, it will have
        // to be removed explicitly
        DestroyWindow(_hToolTip);
        DestroyWindow(_hWndSettings);
    }
    return;
}
