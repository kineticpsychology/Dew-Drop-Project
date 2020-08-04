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

#include "ThemerWindow.h"

LRESULT CALLBACK THEMERWINDOW::_ThemerMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    return ((LPTHEMERWINDOW)dwRefData)->_WndProc(hWnd, nMsg, wParam, lParam);
}

void THEMERWINDOW::_HandleUIState(BYTE btNewState, BOOL bReflectChanges)
{
    int         iDispState;
    HWND        hWndChild = NULL;
    wchar_t     wsTitle[128];
    wchar_t     wsStatusText[MAX_CHAR_PATH];

    _btUIState = btNewState;

    // Hide everything if app is starting for the first time OR
    // if user is closing the currently saved theme.
    iDispState = ((_btUIState == THEMERWINDOW::_UI_STATE_READY) ? SW_HIDE : SW_SHOW);
    do
    {
        hWndChild = FindWindowEx(_hWnd, hWndChild, NULL, NULL);
        if (hWndChild)
            ShowWindow(hWndChild, iDispState);
    } while (hWndChild);
    // Explicitly hide the preview window
    _lpPreviewWindow->SetVisible((_btUIState != THEMERWINDOW::_UI_STATE_READY));
    // Explicitly hide the Password button
    ShowWindow(_hCmdPwd, (_btUIState == THEMERWINDOW::_UI_STATE_READY) ? SW_HIDE : SW_SHOW);
    // Explicitly 'ALWAYS' show the status bar
    ShowWindow(_hStsInfo, SW_SHOW);

    // Set current theme to default (if user is starting 'New')
    if (_btUIState == THEMERWINDOW::_UI_STATE_NEW_NO_CHANGE)
        _CurrentTheme = _DefaultSettings.Theme;

    // Apply Theme state values to the UI
    if (_btUIState != THEMERWINDOW::_UI_STATE_READY && bReflectChanges)
        this->_SetupUIWithCurrThemeValues();

    // Proportionately enable disable items that are mostly going to be in those states
    this->_EnableMenuItem(IDM_FILE_NEW); // Will always be enabled
    this->_EnableMenuItem(IDM_FILE_OPEN); // Will always be enabled
    this->_EnableMenuItem(IDM_FILE_SAVE, FALSE); // Will be mostly disabled
    this->_EnableMenuItem(IDM_FILE_SAVEAS); // Will be mostly enabled
    this->_EnableMenuItem(IDM_FILE_CLOSE); // Will be mostly enabled
    this->_EnableMenuItem(IDM_FILE_EXIT); // Will always be enabled
    this->_EnableMenuItem(IDM_THEME_RESET, FALSE); // Will be mostly disabled
    this->_EnableMenuItem(IDM_THEME_DEFAULT); // Will be mostly enabled

    // Now the specific enable/disablement of menu items & status info
    switch (_btUIState)
    {
        case THEMERWINDOW::_UI_STATE_READY:
            SendMessage(_hStsInfo, SB_SETTEXT, MAKEWPARAM(((0 << 8) | 0), 0), (LPARAM)L" Ready...");
            this->_EnableMenuItem(IDM_FILE_OPEN);
            this->_EnableMenuItem(IDM_FILE_SAVEAS, FALSE);
            this->_EnableMenuItem(IDM_FILE_CLOSE, FALSE);
            this->_EnableMenuItem(IDM_THEME_DEFAULT, FALSE);
            StringCchPrintf(wsTitle, 128, L"%s", _wsTitlePart);
            break;

        case THEMERWINDOW::_UI_STATE_NEW_NO_CHANGE:
            if (_CurrentTheme.Password == NULL || lstrlenA(_CurrentTheme.Password) <= 0)
                SetWindowText(_hCmdPwd, L"Set &Password");
            else
                SetWindowText(_hCmdPwd, L"Change &Password");
            SendMessage(_hStsInfo, SB_SETTEXT, MAKEWPARAM(((0 << 8) | 0), 0), (LPARAM)L" <Default Theme>");
            this->_EnableMenuItem(IDM_FILE_OPEN);
            this->_EnableMenuItem(IDM_THEME_DEFAULT, FALSE);
            StringCchPrintf(wsTitle, 128, L"[%s] - %s", _CurrentTheme.Name, _wsTitlePart);
            SetWindowText(_hWnd, wsTitle);
            break;

        case THEMERWINDOW::_UI_STATE_NEW_CHANGED:
            if (_CurrentTheme.Password == NULL || lstrlenA(_CurrentTheme.Password) <= 0)
                SetWindowText(_hCmdPwd, L"Set &Password");
            else
                SetWindowText(_hCmdPwd, L"Change &Password");
            SendMessage(_hStsInfo, SB_SETTEXT, MAKEWPARAM(((0 << 8) | 0), 0), (LPARAM)L" <Unsaved Theme>");
            this->_EnableMenuItem(IDM_FILE_OPEN);
            this->_EnableMenuItem(IDM_THEME_RESET);
            StringCchPrintf(wsTitle, 128, L"[%s*] - %s", _CurrentTheme.Name, _wsTitlePart);
            SetWindowText(_hWnd, wsTitle);
            break;

        case THEMERWINDOW::_UI_STATE_OPEN_NO_CHANGE:
            SetWindowText(_hCmdPwd, L"Change &Password");
            StringCchPrintf(wsStatusText, MAX_CHAR_PATH, L" %s", _wsThemeFilePath);
            SendMessage(_hStsInfo, SB_SETTEXT, MAKEWPARAM(((0 << 8) | 0), 0), (LPARAM)wsStatusText);
            this->_EnableMenuItem(IDM_FILE_OPEN);
            StringCchPrintf(wsTitle, 128, L"[%s] - %s", _CurrentTheme.Name, _wsTitlePart);
            SetWindowText(_hWnd, wsTitle);
            break;

        case THEMERWINDOW::_UI_STATE_OPEN_CHANGED:
            SetWindowText(_hCmdPwd, L"Change &Password");
            StringCchPrintf(wsStatusText, MAX_CHAR_PATH, L" %s", _wsThemeFilePath);
            SendMessage(_hStsInfo, SB_SETTEXT, MAKEWPARAM(((0 << 8) | 0), 0), (LPARAM)wsStatusText);
            this->_EnableMenuItem(IDM_FILE_OPEN);
            this->_EnableMenuItem(IDM_FILE_SAVE);
            this->_EnableMenuItem(IDM_THEME_RESET);
            StringCchPrintf(wsTitle, 128, L"[%s*] - %s", _CurrentTheme.Name, _wsTitlePart);
            SetWindowText(_hWnd, wsTitle);
            break;

        default:
            StringCchPrintf(wsTitle, 128, L"%s", _wsTitlePart);
            break;
    }

    SetWindowText(_hWnd, wsTitle);

    return;
}

LRESULT THEMERWINDOW::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL        bRet;
    wchar_t     wsThemeFile[MAX_CHAR_PATH] { 0 };
    UINT        nRet;
    DWORD       dwPrevState;

    switch(nMsg)
    {
        case WM_DROPFILES:
        {
            nRet = DragQueryFile((HDROP)wParam, 0, NULL, 0);
            if (!nRet) return TRUE;
            nRet = DragQueryFile((HDROP)wParam, 0, wsThemeFile, MAX_CHAR_PATH);
            DragFinish((HDROP)wParam);
            this->_OpenTheme(wsThemeFile);
            return TRUE;
        }
        case WM_COMMAND:
        {
            // Menu Commands
            if (lParam == 0x00)
            {
                switch(LOWORD(wParam))
                {
                    case IDM_FILE_NEW:
                        this->_File_New_Theme();
                        break;
                    case IDM_FILE_OPEN:
                        this->_File_Open_Theme();
                        break;
                    case IDM_FILE_SAVE:
                        this->_File_Save_Theme(_wsThemeFilePath);
                        break;
                    case IDM_FILE_SAVEAS:
                        this->_File_SaveAs_Theme();
                        break;
                    case IDM_FILE_CLOSE:
                        this->_File_Close_Theme();
                        break;
                    case IDM_FILE_EXIT:
                        SendMessage(_hWnd, WM_CLOSE, 0, 0);
                        break;
                    case IDM_THEME_RESET:
                        this->_Theme_Reset();
                        break;
                    case IDM_THEME_DEFAULT:
                        this->_Theme_Default();
                        break;
                    case IDM_HELP_ABOUT:
                        _hWndFocus = _lpAboutWindow->Show();
                        break;
                }
            }
            // Theme Name
            else if ((HWND)lParam == _hTxtName && HIWORD(wParam) == EN_CHANGE && _bTextChangeConsideredChange)
            {
                GetWindowText(_hTxtName, (_CurrentTheme.Name), DEWTHEME_ATTR_LENGTH);
                this->_FlagChange();
            }
            // Author
            else if ((HWND)lParam == _hTxtAuthor && HIWORD(wParam) == EN_CHANGE && _bTextChangeConsideredChange)
            {
                GetWindowText(_hTxtAuthor, (_CurrentTheme.Author), DEWTHEME_ATTR_LENGTH);
                this->_FlagChange();
            }
            // Color Component (Drop Down)
            else if ((HWND)lParam == _hCmbColorComponent && HIWORD(wParam) == CBN_SELCHANGE)
            {
                this->_SetCTLColorBrushes();
                break;
            }
            // Back Color
            else if ((HWND)lParam == _hLblBackColorPreview && HIWORD(wParam) == STN_CLICKED)
            {
                bRet = FALSE;
                switch(SendMessage(_hCmbColorComponent, CB_GETCURSEL, 0, 0))
                {
                    case THEMER_COMPONENT_SEQ_WINDOW:
                        bRet = this->_SetColor(&(_CurrentTheme.WinStyle.BackColor));
                        break;
                    case THEMER_COMPONENT_SEQ_UI_BUTTONS:
                        bRet = this->_SetColor(&(_CurrentTheme.UIButtonStyle.BackColor));
                        break;
                    case THEMER_COMPONENT_SEQ_MM_BUTTONS:
                        bRet = this->_SetColor(&(_CurrentTheme.MMButtonStyle.BackColor));
                        break;
                    case THEMER_COMPONENT_SEQ_MODULES:
                        bRet = this->_SetColor(&(_CurrentTheme.ModuleStyle.BackColor));
                        break;
                    default:
                        break;
                }
                if (bRet)
                {
                    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                    this->_SetCTLColorBrushes();
                }
                break;
            }
            // Back Color 2
            else if ((HWND)lParam == _hLblBackColor2Preview && HIWORD(wParam) == STN_CLICKED)
            {
                if (this->_SetColor(&(_CurrentTheme.WinStyle.BackColor2)))
                {
                    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                    this->_SetCTLColorBrushes();
                }
                break;
            }
            // Text Color
            else if ((HWND)lParam == _hLblTextColorPreview && HIWORD(wParam) == STN_CLICKED)
            {
                bRet = FALSE;
                switch(SendMessage(_hCmbColorComponent, CB_GETCURSEL, 0, 0))
                {
                    case THEMER_COMPONENT_SEQ_WINDOW:
                        bRet = this->_SetColor(&(_CurrentTheme.WinStyle.TextColor));
                        break;
                    case THEMER_COMPONENT_SEQ_UI_BUTTONS:
                        bRet = this->_SetColor(&(_CurrentTheme.UIButtonStyle.TextColor));
                        break;
                    case THEMER_COMPONENT_SEQ_MM_BUTTONS:
                        bRet = this->_SetColor(&(_CurrentTheme.MMButtonStyle.TextColor));
                        break;
                    case THEMER_COMPONENT_SEQ_MODULES:
                        bRet = this->_SetColor(&(_CurrentTheme.ModuleStyle.TextColor));
                        break;
                    default:
                        break;
                }
                if (bRet)
                {
                    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                    this->_SetCTLColorBrushes();
                }
                break;
            }
            // Outline Color
            else if ((HWND)lParam == _hLblOutlineColorPreview && HIWORD(wParam) == STN_CLICKED)
            {
                bRet = FALSE;
                switch(SendMessage(_hCmbColorComponent, CB_GETCURSEL, 0, 0))
                {
                    case THEMER_COMPONENT_SEQ_WINDOW:
                        bRet = this->_SetColor(&(_CurrentTheme.WinStyle.OutlineColor));
                        break;
                    case THEMER_COMPONENT_SEQ_UI_BUTTONS:
                        bRet = this->_SetColor(&(_CurrentTheme.UIButtonStyle.OutlineColor));
                        break;
                    case THEMER_COMPONENT_SEQ_MM_BUTTONS:
                        bRet = this->_SetColor(&(_CurrentTheme.MMButtonStyle.OutlineColor));
                        break;
                    case THEMER_COMPONENT_SEQ_MODULES:
                        bRet = this->_SetColor(&(_CurrentTheme.ModuleStyle.OutlineColor));
                        break;
                    default:
                        break;
                }
                if (bRet)
                {
                    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                    this->_SetCTLColorBrushes();
                }
                break;
            }
            // Default Icon Mode
            else if ((HWND)lParam == _hOptIconModeDefault && SendMessage(_hOptIconModeDefault, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                _CurrentTheme.IconMode = DEWTHEME_ICON_MODE_DEFAULT;
                _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                this->_FlagChange();
                break;
            }
            // Alternate Icon Mode
            else if ((HWND)lParam == _hOptIconModeAlternate && SendMessage(_hOptIconModeAlternate, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                _CurrentTheme.IconMode = DEWTHEME_ICON_MODE_ALTERNATE;
                _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                this->_FlagChange();
                break;
            }
            // Transparent Icon Toggle
            else if ((HWND)lParam == _hChkTransparentIcon)
            {
                _CurrentTheme.TransparentIcons = ((SendMessage(_hChkTransparentIcon, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
                _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                this->_FlagChange();
                break;
            }
            // Seekbar  Time Display Toggle
            else if ((HWND)lParam == _hChkSeekbarTimes)
            {
                _CurrentTheme.ShowSeekbarTimes = ((SendMessage(_hChkSeekbarTimes, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
                _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                this->_FlagChange();
                break;
            }
            // Icon Scale
            else if ((HWND)lParam == _hCmbIconScale && HIWORD(wParam) == CBN_SELCHANGE)
            {
                _CurrentTheme.IconScale = (BYTE)(SendMessage(_hCmbIconScale, CB_GETCURSEL, 0, 0));
                _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                this->_FlagChange();
                break;
            }
            // Title Font
            else if ((HWND)lParam == _hCmdTitleFont)
            {
                if (this->_SetFont(_CurrentTheme.TitleFontStyle.FontName,
                               &(_CurrentTheme.TitleFontStyle.FontSize),
                               &(_CurrentTheme.TitleFontStyle.IsBold),
                               &(_CurrentTheme.TitleFontStyle.IsItalic)))
                {
                    this->_ApplyTitleFont();
                    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                }

                break;
            }
            // Text Font
            else if ((HWND)lParam == _hCmdTextFont)
            {
                if (this->_SetFont(_CurrentTheme.TextFontStyle.FontName,
                               &(_CurrentTheme.TextFontStyle.FontSize),
                               &(_CurrentTheme.TextFontStyle.IsBold),
                               &(_CurrentTheme.TextFontStyle.IsItalic)))
                {
                    this->_ApplyTextFont();
                    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                }

                break;
            }
            // Theme Background Image
            else if ((HWND)lParam == _hCmdBackground)
            {
                if (this->_SetBackgroundImage())
                    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                break;
            }
            // Theme Background Image Reset
            else if ((HWND)lParam == _hCmdResetBackground)
            {
                if (this->_ResetBackgroundImage())
                    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                break;
            }
            // Theme App Icon
            else if ((HWND)lParam == _hCmdAppIcon)
            {
                if (this->_SetAppIcon())
                    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                break;
            }
            // Theme App Icon Reset
            else if ((HWND)lParam == _hCmdResetAppIcon)
            {
                if (this->_ResetAppIcon())
                    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                break;
            }
            // Set/Change Password
            else if ((HWND)lParam == _hCmdPwd)
            {
                // Invalid Status
                if (_btUIState == THEMERWINDOW::_UI_STATE_READY) return FALSE;
                // Password unavailable: 'Set Password' mode
                if (_CurrentTheme.Password == NULL || lstrlenA(_CurrentTheme.Password) <= 0)
                {
                    _dwTransitionState = (_dwTransitionState | THEMER_TRANSITION_STATE_NONE);
                    _hWndFocus = _lpPassWindow->Show(THEMER_PW_MODE_SET);
                }
                // Password available: 'Change Password' mode
                else
                {
                    _dwTransitionState = (_dwTransitionState | THEMER_TRANSITION_STATE_NONE);
                    _hWndFocus = _lpPassWindow->Show(THEMER_PW_MODE_CHANGE);
                }
                break;
            }
            // Exit Button
            else if ((HWND)lParam == _hCmdClose)
            {
                this->_File_Close_Theme();
                break;
            }
            break;
        }
        case WM_CTLCOLORSTATIC:
        {
            if ((HWND)lParam == _hLblBackColorPreview) return (LRESULT)_hbrBack;
            else if ((HWND)lParam == _hLblBackColor2Preview) return (LRESULT)_hbrBack2;
            else if ((HWND)lParam == _hLblTextColorPreview) return (LRESULT)_hbrText;
            else if ((HWND)lParam == _hLblOutlineColorPreview) return (LRESULT)_hbrOutline;
            break;
        }
        case WM_SIZE:
        {
            this->_HandleSizing();
            return FALSE;
        }
        case WM_DEWMSG_CHILD_CLOSED:
        {
            // A child window has closed. Set the focus back to this window
            EnableWindow(_hWnd, TRUE);
            _hWndFocus = _hWnd;

            // Password window closed
            if ((HWND)lParam == _lpPassWindow->Handle)
            {
                // The User Clicked OK. Need to take any 'Previously Stored' state action forward
                if (_lpPassWindow->ThemeAction.ReturnCode == THEMER_RET_OK)
                {
                    switch (_lpPassWindow->ThemeAction.ActionMode)
                    {
                        case THEMER_PW_MODE_CHECK:
                        {
                            _lpPreviewWindow->ApplyTheme(_CurrentTheme);
                            _OriginalTheme = _CurrentTheme; // Keep a backup
                            StringCchPrintf(_wsThemeFilePath, MAX_CHAR_PATH, _lpPassWindow->ThemeAction.ThemeFile);
                            _bTextChangeConsideredChange = FALSE;
                            this->_HandleUIState(THEMERWINDOW::_UI_STATE_OPEN_NO_CHANGE);
                            _bTextChangeConsideredChange = TRUE;
                            break;
                        }
                        case THEMER_PW_MODE_SET:
                        {
                            this->_FlagChange();
                            // If the password window was opened while saving (without setting password),
                            // then we'll get back that info via transition state value and
                            // will to re-open that 'Save As' file dialog accordingly
                            dwPrevState = _dwTransitionState; // Keep this backup as the close flag will be flushed out by _File_SaveAs_Theme()
                            if ((dwPrevState & THEMER_TRANSITION_STATE_SAVEAS) == THEMER_TRANSITION_STATE_SAVEAS)
                                this->_File_SaveAs_Theme();
                            // Save as has happened. The user has either OKayed or cancelled
                            // from the 'Save As' dialog. We will understand that from the UI State.
                            // We'll also recollect if window was supposed to be closed after saving
                            // (This is where the backup 'btPrevState' of _dwTransitionState will be useful)
                            // If it is to be closed out, then close the window
                            if ((dwPrevState & THEMER_TRANSITION_STATE_CLOSE) == THEMER_TRANSITION_STATE_CLOSE)
                            {
                                if (_btUIState == THEMERWINDOW::_UI_STATE_OPEN_NO_CHANGE)
                                    this->_File_Close_Theme();
                                else // Clear out the close/quit flags
                                    _dwTransitionState = _dwTransitionState & 0xFF;
                            }
                            break;
                        }
                        case THEMER_PW_MODE_CHANGE:
                        {
                            this->_FlagChange();
                            break;
                        }
                    }
                }
                else
                {
                    // The user clicked 'CANCEL' or some error occurred.
                    // Keep the window open and flush out the transition state
                    _dwTransitionState = THEMER_TRANSITION_STATE_NONE;
                }
            }

            return FALSE;
        }
        case WM_CLOSE:
        {
            // No need of confirmation, if theme is already closed
            if (_btUIState != THEMERWINDOW::_UI_STATE_READY)
            {
                // Set the application quit flag & try to close the theme
                _dwTransitionState = _dwTransitionState | THEMER_TRANSITION_STATE_QUIT;
                this->_File_Close_Theme();
                return FALSE;
            }

            ShowWindow(_hWnd, SW_HIDE);
            PostQuitMessage(0);
            return TRUE;
        }
        case WM_QUIT:
        {
             return TRUE;
        }
    }

    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void THEMERWINDOW::_CreateMainMenu()
{
    HMENU       hMnuSub;

    _hMnuMain = CreateMenu();

    // Create the Menu
    hMnuSub = CreatePopupMenu();
    AppendMenu(hMnuSub, MF_STRING, IDM_FILE_NEW, L"&New\tCtrl+N");
    AppendMenu(hMnuSub, MF_STRING, IDM_FILE_OPEN, L"&Open\tCtrl+O");
    AppendMenu(hMnuSub, MF_STRING, IDM_FILE_SAVE, L"&Save\tCtrl+S");
    AppendMenu(hMnuSub, MF_STRING, IDM_FILE_SAVEAS, L"Save &As");
    AppendMenu(hMnuSub, MF_STRING, IDM_FILE_CLOSE, L"&Close\tCtrl+F4");
    AppendMenu(hMnuSub, MF_SEPARATOR, 0, L"");
    AppendMenu(hMnuSub, MF_STRING, IDM_FILE_EXIT, L"E&xit\tAlt+F4");
    AppendMenu(_hMnuMain, MF_POPUP, (UINT_PTR)hMnuSub, L"&File");

    hMnuSub = CreatePopupMenu();
    AppendMenu(hMnuSub, MF_STRING, IDM_THEME_RESET, L"&Reset Theme\tCtrl+R");
    AppendMenu(hMnuSub, MF_STRING, IDM_THEME_DEFAULT, L"&Default Theme\tCtrl+D");
    AppendMenu(_hMnuMain, MF_POPUP, (UINT_PTR)hMnuSub, L"&Theme");

    hMnuSub = CreatePopupMenu();
    AppendMenu(hMnuSub, MF_STRING, IDM_HELP_ABOUT, L"&About\tF1");
    AppendMenu(_hMnuMain, MF_POPUP, (UINT_PTR)hMnuSub, L"&Help");

    // Create the Accelerator table
    ACCEL       acclUI[7] = { { FCONTROL | FVIRTKEY, VK_N,  IDM_FILE_NEW },
                              { FCONTROL | FVIRTKEY, VK_O,  IDM_FILE_OPEN },
                              { FCONTROL | FVIRTKEY, VK_S,  IDM_FILE_SAVE },
                              { FCONTROL | FVIRTKEY, VK_F4, IDM_FILE_CLOSE},

                              { FCONTROL | FVIRTKEY, VK_R,  IDM_THEME_RESET},
                              { FCONTROL | FVIRTKEY, VK_D,  IDM_THEME_DEFAULT},

                              { FVIRTKEY,            VK_F1, IDM_HELP_ABOUT } };
    _hAccel = CreateAcceleratorTable(acclUI, 7);

    return;
}

void THEMERWINDOW::_HandleSizing()
{
    int         iX, iY, iW, iH;
    RECT        rctClient { 0 };
    int         iParts[2];
    wchar_t     wsText[DEWTHEME_ATTR_LENGTH];

    GetClientRect(_hWnd, &rctClient);

    iX = _F(10); iY = _F(10);
    SetWindowPos(_lpPreviewWindow->Handle, 0, iX, iY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    // Theme Info Section ------------------------------------------------------
    iX = _iXOffset; iY = _iYOffset; iW = rctClient.right - _iXOffset - _F(10); iH = _F(80);
    SetWindowPos(_hGrpInfo, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(15); iW = _F(50); iH = _F(20);
    SetWindowPos(_hLblName, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(70); iY = _iYOffset + _F(15); iW = rctClient.right - _iXOffset - _F(90); iH = _F(20);
    SetWindowPos(_hTxtName, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(45); iW = _F(50); iH = _F(20);
    SetWindowPos(_hLblAuthor, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(70); iY = _iYOffset + _F(45); iW = rctClient.right - _iXOffset - _F(90); iH = _F(20);
    SetWindowPos(_hTxtAuthor, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------


    // Theme Color Section -----------------------------------------------------
    iX = _iXOffset; iY = _iYOffset + _F(90); iW = rctClient.right - _iXOffset - _F(10); iH = _F(110);
    SetWindowPos(_hGrpColors, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(105); iW = _F(130); iH = _F(20);
    SetWindowPos(_hLblColor, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(150); iY = _iYOffset + _F(105); iW = rctClient.right - _iXOffset - _F(170); iH = _F(100);
    SetWindowPos(_hCmbColorComponent, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(135); iW = _F(100); iH = _F(20);
    SetWindowPos(_hLblBackColor, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(120); iY = _iYOffset + _F(135); iW = _F(50); iH = _F(20);
    SetWindowPos(_hLblBackColorPreview, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(310); iY = _iYOffset + _F(135); iW = _F(100); iH = _F(20);
    SetWindowPos(_hLblBackColor2, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(420); iY = _iYOffset + _F(135); iW = _F(50); iH = _F(20);
    SetWindowPos(_hLblBackColor2Preview, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(165); iW = _F(100); iH = _F(20);
    SetWindowPos(_hLblTextColor, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(120); iY = _iYOffset + _F(165); iW = _F(50); iH = _F(20);
    SetWindowPos(_hLblTextColorPreview, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(310); iY = _iYOffset + _F(165); iW = _F(100); iH = _F(20);
    SetWindowPos(_hLblOutlineColor, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(420); iY = _iYOffset + _F(165); iW = _F(50); iH = _F(20);
    SetWindowPos(_hLblOutlineColorPreview, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------


    // Theme Font Section ------------------------------------------------------
    iX = _iXOffset; iY = _iYOffset + _F(210); iW = rctClient.right - _iXOffset - _F(10); iH = _F(135);
    SetWindowPos(_hGrpFonts, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(240); iW = rctClient.right - _iXOffset - _F(65); iH = _F(40);
    SetWindowPos(_hLblTitleFont, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = rctClient.right - _F(50); iY = _iYOffset + _F(245); iW = _F(30); iH = _F(30);
    SetWindowPos(_hCmdTitleFont, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(290); iW = rctClient.right - _iXOffset - _F(65); iH = _F(40);
    SetWindowPos(_hLblTextFont, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = rctClient.right - _F(50); iY = _iYOffset + _F(295); iW = _F(30); iH = _F(30);
    SetWindowPos(_hCmdTextFont, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------


    // Theme Icon Section ------------------------------------------------------
    iX = _iXOffset; iY = _iYOffset + _F(355); iW = rctClient.right - _iXOffset - _F(10); iH = _F(110);
    SetWindowPos(_hGrpIconStyle, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(370); iW = _F(70); iH = _F(20);
    SetWindowPos(_hLblIconMode, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(100); iY = _iYOffset + _F(370); iW = _F(150); iH = _F(20);
    SetWindowPos(_hOptIconModeDefault, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(265); iY = _iYOffset + _F(370); iW = _F(150); iH = _F(20);
    SetWindowPos(_hOptIconModeAlternate, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(400); iW = _F(70); iH = _F(20);
    SetWindowPos(_hLblIconScale, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(90); iY = _iYOffset + _F(400); iW = _F(150); iH = _F(100);
    SetWindowPos(_hCmbIconScale, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(275); iY = _iYOffset + _F(400); iW = rctClient.right - _F(295) - _iXOffset; iH = _F(20);
    SetWindowPos(_hChkTransparentIcon, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(430); iW = rctClient.right - _iXOffset - _F(30); iH = _F(20);
    SetWindowPos(_hChkSeekbarTimes, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------


    // Theme Images ------------------------------------------------------------
    iX = _iXOffset; iY = _iYOffset + _F(475); iW = rctClient.right - _iXOffset - _F(10); iH = _F(130);
    SetWindowPos(_hGrpImages, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(495); iW = rctClient.right - _iXOffset - _F(150); iH = _F(20);
    SetWindowPos(_hLblBackground, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(515); iW = rctClient.right - _iXOffset - _F(150); iH = _F(20);
    SetWindowPos(_hTxtBackground, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = rctClient.right - _F(130); iY = _iYOffset + _F(505); iW = _F(30); iH = _F(30);
    SetWindowPos(_hCmdBackground, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = rctClient.right - _F(95); iY = _iYOffset + _F(505); iW = _F(75); iH = _F(30);
    SetWindowPos(_hCmdResetBackground, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(550); iW = rctClient.right - _iXOffset - _F(150); iH = _F(20);
    SetWindowPos(_hLblAppIcon, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = _iXOffset + _F(10); iY = _iYOffset + _F(570); iW = rctClient.right - _iXOffset - _F(150); iH = _F(20);
    SetWindowPos(_hTxtAppIcon, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = rctClient.right - _F(130); iY = _iYOffset + _F(560); iW = _F(30); iH = _F(30);
    SetWindowPos(_hCmdAppIcon, 0, iX, iY, iW, iH, SWP_NOZORDER);
    iX = rctClient.right - _F(95); iY = _iYOffset + _F(560); iW = _F(75); iH = _F(30);
    SetWindowPos(_hCmdResetAppIcon, 0, iX, iY, iW, iH, SWP_NOZORDER);
    // -------------------------------------------------------------------------

    iX = _iXOffset; iY = rctClient.bottom - _F(40) - _iSBHeight; iW = _F(125); iH = _F(30);
    SetWindowPos(_hCmdPwd, 0, iX, iY, iW, iH, SWP_NOZORDER);

    iX = rctClient.right - _F(85); iY = rctClient.bottom - _F(40) - _iSBHeight; iW = _F(75); iH = _F(30);
    SetWindowPos(_hCmdClose, 0, iX, iY, iW, iH, SWP_NOZORDER);

    // Status Bar initial text -------------------------------------------------
    iParts[0] = rctClient.right - _F(140); iParts[1] = -1;
    SendMessage(_hStsInfo, SB_SETPARTS, 2, (LPARAM)&iParts);
    StringCchPrintf(wsText, 64, L" Themer Version: %u.%u", _DefaultTheme.VersionMajor, _DefaultTheme.VersionMinor);
    SendMessage(_hStsInfo, SB_SETTEXT, MAKEWPARAM(((0 << 8) | 1), 0), (LPARAM)wsText);
    // -------------------------------------------------------------------------


    // Tooltips ----------------------------------------------------------------
    this->_SetTooltip(_hTxtName, L"Give a name to your theme. This cannot be empty.");
    this->_SetTooltip(_hTxtAuthor, L"Who created this theme? This cannot be empty.");

    this->_SetTooltip(_hLblBackColorPreview, L"Click to change the Background Color");
    this->_SetTooltip(_hLblBackColor2Preview, L"Click to change the lower (gradient) Background Color");
    this->_SetTooltip(_hLblTextColorPreview, L"Click to change the Text Color");
    this->_SetTooltip(_hLblOutlineColorPreview, L"Click to change the Outline Color");

    this->_SetTooltip(_hCmdTitleFont, L"Click to set the Title Font");
    this->_SetTooltip(_hCmdTextFont, L"Click to set the Text Font");

    this->_SetTooltip(_hOptIconModeDefault, L"Icons are drawn with Text and Outline Colors; filled with Background Color");
    this->_SetTooltip(_hOptIconModeAlternate, L"Icons are drawn with Background Color and filled with Outline color, no borders or outlines");

    this->_SetTooltip(_hChkTransparentIcon, L"Remove filling up of icon background with Back Color");
    this->_SetTooltip(_hChkSeekbarTimes, L"Show/Hide the current and total times displayed over the Seekbar");

    this->_SetTooltip(_hCmdBackground, L"Browse and set a background for your theme with standard image formats");
    this->_SetTooltip(_hCmdResetBackground, L"Remove background image from the theme");

    this->_SetTooltip(_hCmdAppIcon, L"Browse and set a taskbar/titlebar icon for your theme");
    this->_SetTooltip(_hCmdResetAppIcon, L"Revert to the default theme taskbar/titlebar icon");

    this->_SetTooltip(_hCmdPwd, L"Set or change your theme password");
    this->_SetTooltip(_hCmdClose , L"Close the current theme");
    // -------------------------------------------------------------------------

    return;
}

void THEMERWINDOW::_SetTooltip(HWND hCtrl, LPCWSTR wsTipText)
{
    TOOLINFO ti { 0 };

    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hCtrl;
    ti.hinst = GetModuleHandle(NULL);
    ti.lpszText = (LPWSTR)wsTipText;
    GetClientRect(hCtrl, &(ti.rect));

    SendMessage(_hToolTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
    return;

}


void THEMERWINDOW::_ApplyUIFont()
{
    LOGFONT     lgfFont { 0 };
    HWND        hWndChild = NULL;

    lgfFont.lfHeight = -MulDiv(8, _iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = FW_NORMAL;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, L"Tahoma", 32 * sizeof(wchar_t));

    if (_hFntUI) { DeleteObject(_hFntUI); _hFntUI = NULL; }
    _hFntUI = CreateFontIndirect(&lgfFont);

    do
    {
        hWndChild = FindWindowEx(_hWnd, hWndChild, NULL, NULL);
        if(hWndChild)
            SendMessage(hWndChild, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));
    } while(hWndChild);

    return;
}

void THEMERWINDOW::_ApplyTitleFont()
{
    LOGFONT     lgfFont { 0 };

    lgfFont.lfHeight = -MulDiv(_CurrentTheme.TitleFontStyle.FontSize, _iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = (_CurrentTheme.TitleFontStyle.IsBold ? FW_BOLD : FW_NORMAL);
    lgfFont.lfItalic = (_CurrentTheme.TitleFontStyle.IsItalic ? 1 : 0);
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, _CurrentTheme.TitleFontStyle.FontName, 32 * sizeof(wchar_t));

    if (_hFntTitle) { DeleteObject(_hFntTitle); _hFntTitle = NULL; }
    _hFntTitle = CreateFontIndirect(&lgfFont);
    SendMessage(_hLblTitleFont, WM_SETFONT, (WPARAM)_hFntTitle, MAKELPARAM(TRUE, 0));

    return;
}

void THEMERWINDOW::_ApplyTextFont()
{
    LOGFONT     lgfFont { 0 };

    lgfFont.lfHeight = -MulDiv(_CurrentTheme.TextFontStyle.FontSize, _iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = (_CurrentTheme.TextFontStyle.IsBold ? FW_BOLD : FW_NORMAL);
    lgfFont.lfItalic = (_CurrentTheme.TextFontStyle.IsItalic ? 1 : 0);
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, _CurrentTheme.TextFontStyle.FontName, 32 * sizeof(wchar_t));

    if (_hFntText) { DeleteObject(_hFntText); _hFntText = NULL; }
    _hFntText = CreateFontIndirect(&lgfFont);
    SendMessage(_hLblTextFont, WM_SETFONT, (WPARAM)_hFntText, MAKELPARAM(TRUE, 0));

    return;
}

void THEMERWINDOW::_SetCTLColorBrushes(BOOL bDelOnly)
{
    int     iIndex;

    iIndex = (int)SendMessage(_hCmbColorComponent, CB_GETCURSEL, 0, 0);
    if (iIndex == -1) return;

    if (_hbrBack) { DeleteObject(_hbrBack); _hbrBack = NULL; }
    if (_hbrBack2) { DeleteObject(_hbrBack2); _hbrBack2 = NULL; }
    if (_hbrText) { DeleteObject(_hbrText); _hbrText = NULL; }
    if (_hbrOutline) { DeleteObject(_hbrOutline); _hbrOutline = NULL; }

    _hbrBack2 = CreateSolidBrush(_CurrentTheme.WinStyle.BackColor2);
    switch (iIndex)
    {
        case THEMER_COMPONENT_SEQ_WINDOW:
        {
            _hbrBack = CreateSolidBrush(_CurrentTheme.WinStyle.BackColor);
            _hbrText = CreateSolidBrush(_CurrentTheme.WinStyle.TextColor);
            _hbrOutline = CreateSolidBrush(_CurrentTheme.WinStyle.OutlineColor);
            break;
        }
        case THEMER_COMPONENT_SEQ_UI_BUTTONS:
        {
            _hbrBack = CreateSolidBrush(_CurrentTheme.UIButtonStyle.BackColor);
            _hbrText = CreateSolidBrush(_CurrentTheme.UIButtonStyle.TextColor);
            _hbrOutline = CreateSolidBrush(_CurrentTheme.UIButtonStyle.OutlineColor);
            break;
        }
        case THEMER_COMPONENT_SEQ_MM_BUTTONS:
        {
            _hbrBack = CreateSolidBrush(_CurrentTheme.MMButtonStyle.BackColor);
            _hbrText = CreateSolidBrush(_CurrentTheme.MMButtonStyle.TextColor);
            _hbrOutline = CreateSolidBrush(_CurrentTheme.MMButtonStyle.OutlineColor);
            break;
        }
        case THEMER_COMPONENT_SEQ_MODULES:
        {
            _hbrBack = CreateSolidBrush(_CurrentTheme.ModuleStyle.BackColor);
            _hbrText = CreateSolidBrush(_CurrentTheme.ModuleStyle.TextColor);
            _hbrOutline = CreateSolidBrush(_CurrentTheme.ModuleStyle.OutlineColor);
            break;
        }
    }
    // Back Color 2 will only show for windows
    ShowWindow(_hLblBackColor2, (iIndex == THEMER_COMPONENT_SEQ_WINDOW ? SW_SHOW : SW_HIDE));
    ShowWindow(_hLblBackColor2Preview, (iIndex == THEMER_COMPONENT_SEQ_WINDOW ? SW_SHOW : SW_HIDE));

    // Back Color & Back Color 2 will not show for UI buttons
    ShowWindow(_hLblBackColor, (iIndex == THEMER_COMPONENT_SEQ_UI_BUTTONS ? SW_HIDE : SW_SHOW));
    ShowWindow(_hLblBackColorPreview, (iIndex == THEMER_COMPONENT_SEQ_UI_BUTTONS ? SW_HIDE: SW_SHOW));

    InvalidateRect(_hLblBackColorPreview, NULL, TRUE);
    InvalidateRect(_hLblBackColor2Preview, NULL, TRUE);
    InvalidateRect(_hLblTextColorPreview, NULL, TRUE);
    InvalidateRect(_hLblOutlineColorPreview, NULL, TRUE);

    return;

}

void THEMERWINDOW::_SetupUIWithCurrThemeValues()
{
    SetWindowText(_hTxtName, _CurrentTheme.Name);
    SetWindowText(_hTxtAuthor, _CurrentTheme.Author);

    SendMessage(_hCmbColorComponent, CB_SETCURSEL, 0, 0);
    this->_SetCTLColorBrushes();

    this->_ApplyTitleFont();
    this->_ApplyTextFont();

    SendMessage(_hOptIconModeDefault, BM_SETCHECK,
                ((_CurrentTheme.IconMode == DEWTHEME_ICON_MODE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED), 0);
    SendMessage(_hOptIconModeAlternate, BM_SETCHECK,
                ((_CurrentTheme.IconMode == DEWTHEME_ICON_MODE_ALTERNATE) ? BST_CHECKED : BST_UNCHECKED), 0);

    SendMessage(_hChkTransparentIcon, BM_SETCHECK,
                ((_CurrentTheme.TransparentIcons == 1) ? BST_CHECKED : BST_UNCHECKED), 0);

    SendMessage(_hChkSeekbarTimes, BM_SETCHECK,
                ((_CurrentTheme.ShowSeekbarTimes == 1) ? BST_CHECKED : BST_UNCHECKED), 0);


    SendMessage(_hCmbIconScale, CB_SETCURSEL, _CurrentTheme.IconScale, 0);

    if (_CurrentTheme.BackgroundImageSize > 0)
        SetWindowText(_hTxtBackground, L"<Background Image Used>");
    else
        SetWindowText(_hTxtBackground, _ws_NO_BACK_IMG);

    if (_CurrentTheme.IconImageSize > 0)
        SetWindowText(_hTxtAppIcon, L"<Custom Icon Used>");
    else
        SetWindowText(_hTxtAppIcon, _ws_DEF_APP_ICON);

    return;
}


void THEMERWINDOW::_File_New_Theme()
{
    int     iAns;

    if (_btUIState == THEMERWINDOW::_UI_STATE_NEW_CHANGED || _btUIState == THEMERWINDOW::_UI_STATE_OPEN_CHANGED)
    {
        iAns = MessageBox(_hWnd, L"You have unsaved changes. Creating a new theme file " \
                          L"will cause the unsaved changes to be lost. Are you " \
                          L"sure you want to proceed?", L"Confirm", MB_YESNO | MB_ICONQUESTION);
        if (iAns == IDNO) return;
    }

    _CurrentTheme = _DefaultTheme;
    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
    _bTextChangeConsideredChange = FALSE;
    this->_HandleUIState(THEMERWINDOW::_UI_STATE_NEW_NO_CHANGE);
    _bTextChangeConsideredChange = TRUE;
    _dwTransitionState = THEMER_TRANSITION_STATE_NONE;
    return;
}

void THEMERWINDOW::_File_Open_Theme()
{
    OPENFILENAME    ofn { 0 };
    int             iAns;
    wchar_t         wsThemeFilePath[MAX_CHAR_PATH] { 0 };

    if (_btUIState == THEMERWINDOW::_UI_STATE_NEW_CHANGED || _btUIState == THEMERWINDOW::_UI_STATE_OPEN_CHANGED)
    {
        iAns = MessageBox(_hWnd, L"You have unsaved changes. Opening a theme file " \
                          L"will cause the unsaved changes to be lost. Are you " \
                          L"sure you want to proceed?", L"Confirm", MB_YESNO | MB_ICONQUESTION);
        if (iAns == IDNO) return;
    }

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = _hWnd;
    ofn.lpstrFilter = L"Dew Drop Theme Files (*.dth)\0*.dth\0\0";
    ofn.lpstrFile = wsThemeFilePath;
    ofn.nMaxFile = MAX_CHAR_PATH;
    ofn.lpstrTitle = L"Open a Dew Drop Theme File";
    ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    if (GetOpenFileName(&ofn))
        this->_OpenTheme(wsThemeFilePath, TRUE);
    return;
}

void THEMERWINDOW::_File_Save_Theme(LPCWSTR wsThemeFilePath)
{
    BYTE        btSaveRet;

    // Invalid data
    if (!wsThemeFilePath || lstrlen(wsThemeFilePath) <= 0) return;

    // Bail if the theme doesn't contain any name/author
    if (lstrlen(_CurrentTheme.Name) == 0 ||
        lstrlen(_CurrentTheme.Author) == 0)
    {
        MessageBox(_hWnd, L"The theme name and/or author is missing. Please " \
                   L"provide valid theme name and author name before saving.",
                   L"Error", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    // Enumerate the error codes to report the appropriate message
    btSaveRet = _CurrentTheme.WriteThemeFile(wsThemeFilePath);
    if (btSaveRet != DEWTHEME_SAVE_OK)
    {
        switch (btSaveRet)
        {
            // Missing password
            case DEWTHEME_SAVE_NO_PASSWORD:
            {
                MessageBox(_hWnd, L"Please provide a password before saving the theme file.",
                           L"Theme Password Missing", MB_OK | MB_ICONEXCLAMATION);
                return;
            }

            // Access Denied
            case DEWTHEME_SAVE_NO_ACCESS:
            {
                MessageBox(_hWnd, L"You don't seem to have access to the location " \
                           L"where you are trying to save the theme file. Please " \
                           L"try a different location.", L"Theme Password Missing",
                           MB_OK | MB_ICONEXCLAMATION);
                break;
            }

            // Internal/Generic Error
            case DEWTHEME_SAVE_GENERIC_ERROR:
            {
                MessageBox(_hWnd, L"There was a problem trying to save the theme file. Please try again.",
                           L"Error", MB_OK | MB_ICONERROR);
                break;
            }
        }
    }
    // Theme Saved
    else
    {
        StringCchPrintf(_wsThemeFilePath, MAX_CHAR_PATH, L"%s", wsThemeFilePath);
        _OriginalTheme = _CurrentTheme;
        _bTextChangeConsideredChange = FALSE;
        this->_HandleUIState(THEMERWINDOW::_UI_STATE_OPEN_NO_CHANGE);
        _bTextChangeConsideredChange = TRUE;
    }
    return;
}

void THEMERWINDOW::_File_SaveAs_Theme()
{
    OPENFILENAME    ofn { 0 };
    wchar_t         wsThemeFilePath[MAX_CHAR_PATH] { 0 };

    // If a password is unavailable during save, instead of the annoying
    // 'Please set password', we would like the user to take to the
    // password set screen directly and then continue with the save
    // if the user set the password properly.
    if (_CurrentTheme.Password == NULL || lstrlenA(_CurrentTheme.Password) <= 0)
    {
        _dwTransitionState = (_dwTransitionState | THEMER_TRANSITION_STATE_SAVEAS);
        _hWndFocus = _lpPassWindow->Show(THEMER_PW_MODE_SET);
        return;
    }

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = _hWnd;
    ofn.lpstrFilter = L"Dew Drop Theme Files (*.dth)\0*.dth\0\0";
    ofn.lpstrFile = wsThemeFilePath;
    ofn.nMaxFile = MAX_CHAR_PATH;
    ofn.lpstrTitle = L"Save Dew Drop Theme File";
    ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = L"dth";
    if (GetSaveFileName(&ofn))
        this->_File_Save_Theme(wsThemeFilePath);
    return;
}

void THEMERWINDOW::_File_Close_Theme()
{
    int     iAns;

    if (_btUIState == THEMERWINDOW::_UI_STATE_NEW_CHANGED || _btUIState == THEMERWINDOW::_UI_STATE_OPEN_CHANGED)
    {
        iAns = MessageBox(_hWnd, L"You have unsaved changes. Would you like to " \
                          L"save the changes before closing the theme?",
                          L"Confirm", MB_YESNOCANCEL | MB_ICONQUESTION);
        if (iAns == IDCANCEL) return;
        if (iAns == IDYES)
        {
            if (_btUIState == THEMERWINDOW::_UI_STATE_NEW_CHANGED || _wsThemeFilePath == NULL || lstrlen(_wsThemeFilePath) <= 0)
            {
                _dwTransitionState = (_dwTransitionState | THEMER_TRANSITION_STATE_CLOSE); // Set the 'CLOSE afterwards'  flag
                this->_File_SaveAs_Theme();
                // _btUIState will definitely change to _UI_STATE_OPEN_NO_CHANGE if save is successful
                // In that case, let the code fall through and close the theme
                if (_btUIState != THEMERWINDOW::_UI_STATE_OPEN_NO_CHANGE) return;
            }
            else if (_btUIState == THEMERWINDOW::_UI_STATE_OPEN_CHANGED)
            {
                this->_File_Save_Theme(_wsThemeFilePath);
                // _btUIState will definitely change to _UI_STATE_OPEN_NO_CHANGE if save is successful
                // In that case, let the code fall through and close the theme
                if (_btUIState != THEMERWINDOW::_UI_STATE_OPEN_NO_CHANGE) return;
            }
        }
    }

    // Check if quitting is required. Skip the last part if we are to quit anyways.
    if ((_dwTransitionState & THEMER_TRANSITION_STATE_QUIT) == THEMER_TRANSITION_STATE_QUIT)
    {
        _btUIState = THEMERWINDOW::_UI_STATE_READY;
        SendMessage(_hWnd, WM_CLOSE, 0, 0);
        return;
    }

    _CurrentTheme = _DefaultTheme;
    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
    _bTextChangeConsideredChange = FALSE;
    this->_HandleUIState(THEMERWINDOW::_UI_STATE_READY);
    return;
}

void THEMERWINDOW::_Theme_Reset()
{
    int         iAns;
    char        sPassword[DEWTHEME_ATTR_LENGTH] { 0 };

    StringCchPrintfA(sPassword, 64, "%s", _CurrentTheme.Password);

    iAns = MessageBox(_hWnd, L"This will reset the theme to the original state. " \
                      L"You'll lose all your changes done since your last save. " \
                      L"Are you sure you want to continue?", L"Confirm Reset",
                      MB_YESNO | MB_ICONQUESTION);
    if (iAns != IDYES) return;
    _CurrentTheme = _OriginalTheme;
    _CurrentTheme.SetPassword(sPassword); // Do NOT overwrite the password. Keep it the same as before
    this->_FlagChange();
    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
    this->_SetupUIWithCurrThemeValues();
    return;
}

void THEMERWINDOW::_Theme_Default()
{
    int         iAns;
    char        sPassword[DEWTHEME_ATTR_LENGTH] { 0 };

    StringCchPrintfA(sPassword, 64, "%s", _CurrentTheme.Password);

    iAns = MessageBox(_hWnd, L"This will change the look and feel of the theme " \
                      L"to the default syle and your unsaved changes will be lost. " \
                      L"Are you sure you want to continue?", L"Confirm Change To Default",
                      MB_YESNO | MB_ICONQUESTION);
    if (iAns != IDYES) return;

    _CurrentTheme = _DefaultTheme;
    _CurrentTheme.SetPassword(sPassword); // Do NOT overwrite the password. Keep it the same as before
    this->_FlagChange();
    _lpPreviewWindow->ApplyTheme(_CurrentTheme);
    this->_SetupUIWithCurrThemeValues();
    return;
}


void THEMERWINDOW::_OpenTheme(LPCWSTR wsThemeFile, BOOL bFromMenu)
{
    int             iAns;

    HANDLE          hThemeFile;
    DWORD           dwHeader, dwSize;

    hThemeFile = CreateFile(wsThemeFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hThemeFile == NULL || hThemeFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(_hWnd, L"There was an error trying to open the specified theme file. " \
                   L"Please try a different theme file.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    dwSize = GetFileSize(hThemeFile, NULL);
    if (dwSize <= sizeof(DWORD))
    {
        MessageBox(_hWnd, L"The specified file is an invalid theme. " \
                   L"Please try a different theme file.", L"Error", MB_OK | MB_ICONERROR);
        CloseHandle(hThemeFile);
        return;
    }
    if (dwSize >= sizeof(DWORD)) ReadFile(hThemeFile, &dwHeader, sizeof(DWORD), &dwSize, NULL);
    CloseHandle(hThemeFile);

    if (dwHeader != DEWTHEME_HEADER)
    {
        MessageBox(_hWnd, L"The specified file is an invalid theme. " \
                   L"Please try a different theme file.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // This check is needed yet again (in case it is not sent from MENU)
    // An example would be a drag-n-drop
    if (!bFromMenu)
    {
        if (_btUIState == THEMERWINDOW::_UI_STATE_NEW_CHANGED || _btUIState == THEMERWINDOW::_UI_STATE_OPEN_CHANGED)
        {
            iAns = MessageBox(_hWnd, L"You have unsaved changes. Opening a theme file " \
                              L"will cause the unsaved changes to be lost. Are you " \
                              L"sure you want to proceed?", L"Confirm", MB_YESNO | MB_ICONQUESTION);
            if (iAns == IDNO) return;
        }
    }
    _dwTransitionState = THEMER_TRANSITION_STATE_NONE;
    _hWndFocus = _lpPassWindow->Show(THEMER_PW_MODE_CHECK, wsThemeFile);

    return;
}

void THEMERWINDOW::_EnableMenuItem(UINT wID, BOOL bEnable)
{
    MENUITEMINFO        mii { 0 };

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STATE | MIIM_ID;
    mii.wID = wID;
    mii.fState = (bEnable ? MFS_ENABLED : MFS_DISABLED);

    SetMenuItemInfo(_hMnuMain, wID, FALSE, &mii);
    return;
}

BOOL THEMERWINDOW::_SetFont(LPWSTR wsFontName, int *piFontSize, BOOL *pBold, BOOL *pItalic)
{
    LOGFONT     lgfFont { 0 };
    CHOOSEFONT  cf { 0 };

    lgfFont.lfHeight = -MulDiv(*piFontSize, _iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = (*pBold ? FW_BOLD : FW_NORMAL);
    lgfFont.lfItalic = (*pItalic ? 1 : 0);
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, wsFontName, 32 * sizeof(wchar_t));

    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = _hWnd;
    cf.lpLogFont = &lgfFont;
    cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_NOSCRIPTSEL | CF_SCALABLEONLY | CF_LIMITSIZE;
    cf.nSizeMin = 6;
    cf.nSizeMax = 24;

    if (ChooseFont(&cf))
    {
        CopyMemory(wsFontName, lgfFont.lfFaceName, 32 * sizeof(wchar_t));
        *piFontSize = MulDiv(-lgfFont.lfHeight, 72, _iDPI);
        *pBold = (lgfFont.lfWeight == FW_BOLD);
        *pItalic = (lgfFont.lfItalic != 0);
        this->_FlagChange();
        return TRUE;
    }

    return FALSE;
}

BOOL THEMERWINDOW::_SetColor(COLORREF *pCrResult)
{
    CHOOSECOLOR     cc { 0 };
    static COLORREF crCust[16] { 0 };

    cc.lStructSize = sizeof(CHOOSECOLOR);
    cc.hwndOwner = _hWnd;
    cc.rgbResult = *pCrResult;
    cc.lpCustColors = crCust;
    cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColor(&cc))
    {
        *pCrResult = cc.rgbResult;
        this->_FlagChange();
        return TRUE;
    }
    return FALSE;
}

BOOL THEMERWINDOW::_SetBackgroundImage()
{
    BOOL            bRepeat = FALSE;
    Image           *pImg = NULL;
    float           fWidth, fHeight, fAspectRatio;
    wchar_t         wsImgFile[MAX_CHAR_PATH] { 0 };
    HANDLE          hImgFile = NULL;
    int             iAns;
    OPENFILENAME    ofn { 0 };

    // Invalid or redundant changes will not trigger anything
    if (_btUIState == THEMERWINDOW::_UI_STATE_READY) // Redundant State
        return FALSE;

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = _hWnd;
    ofn.lpstrFilter = L"Supported image files\0*.jpg;*.jpeg;*.bmp;*.png\0\0";
    ofn.lpstrFile = wsImgFile;
    ofn.nMaxFile = MAX_CHAR_PATH;
    ofn.lpstrTitle = L"Select Theme Background Image";
    ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

    do
    {
        bRepeat = FALSE;
        if (pImg) { delete pImg; pImg = NULL; }
        ZeroMemory(wsImgFile, MAX_CHAR_PATH * sizeof(wchar_t));

        if (GetOpenFileName(&ofn))
        {
            pImg = Image::FromFile(wsImgFile);
            if (pImg)
            {
                fWidth = (float)pImg->GetWidth();
                fHeight = (float)pImg->GetHeight();
                fAspectRatio = fWidth/fHeight;
                delete pImg;
                pImg = NULL;
                iAns = IDYES;
            }
            else
            {
                fAspectRatio = 0.0f;
            }
            if (fAspectRatio > 1.78f || fAspectRatio < 1.44)
                iAns = MessageBox(_hWnd, L"The image you have selected does not have " \
                                  L"a suitable aspect ratio range (between 16:9 to 16:11). " \
                                  L"This will cause the image to be stretched " \
                                  L"horizontally/vertically and might not look good.\n\n" \
                                  L"Are you sure want to continue with this image?",
                                  L"Unsuitable Image Aspect Ratio", MB_YESNO | MB_ICONQUESTION);
            if (iAns == IDNO)
            {
                bRepeat = TRUE;
            }
            else
            {
                hImgFile = CreateFile(wsImgFile, GENERIC_READ, FILE_SHARE_READ,
                                      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hImgFile == NULL || hImgFile == INVALID_HANDLE_VALUE)
                {
                    MessageBox(_hWnd, L"There was an error trying to integrate " \
                               L"the background image into the theme. Please try again.",
                               L"Error", MB_OK | MB_ICONERROR);
                    return FALSE;
                }
                _CurrentTheme.BackgroundImageSize = GetFileSize(hImgFile, NULL);
                if (_CurrentTheme.BackgroundImageData)
                {
                    LocalFree(_CurrentTheme.BackgroundImageData);
                    _CurrentTheme.BackgroundImageData = NULL;
                }
                _CurrentTheme.BackgroundImageData = (LPBYTE)LocalAlloc(LPTR, _CurrentTheme.BackgroundImageSize);
                ReadFile(hImgFile, (_CurrentTheme.BackgroundImageData),
                         (_CurrentTheme.BackgroundImageSize), &(_CurrentTheme.BackgroundImageSize), NULL);
                CloseHandle(hImgFile);
                hImgFile = NULL;
                SetWindowText(_hTxtBackground, wsImgFile);
                this->_FlagChange();
                return TRUE;
            }
        }

    } while (bRepeat);
    return FALSE;
}

BOOL THEMERWINDOW::_ResetBackgroundImage()
{
    // Invalid or redundant changes will not trigger anything
    if (_btUIState == THEMERWINDOW::_UI_STATE_READY) // Redundant State
        return FALSE;

    if (_CurrentTheme.BackgroundImageSize == 0) // Nothing to reset
        return FALSE;

    if (_CurrentTheme.BackgroundImageData)
    {
        LocalFree(_CurrentTheme.BackgroundImageData);
        _CurrentTheme.BackgroundImageData = NULL;
    }
    _CurrentTheme.BackgroundImageSize = 0;
    SetWindowText(_hTxtBackground, _ws_NO_BACK_IMG);
    this->_FlagChange();

    return TRUE;
}

BOOL THEMERWINDOW::_SetAppIcon()
{
    Image           *pImg = NULL;
    wchar_t         wsIcoFile[MAX_CHAR_PATH] { 0 };
    int             iIcoWidth = 0, iIcoHeight = 0;
    HANDLE          hIcoFile = NULL;
    OPENFILENAME    ofn { 0 };

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = _hWnd;
    ofn.lpstrFilter = L"Icon files (*.ico)\0*.ico\0\0";
    ofn.lpstrFile = wsIcoFile;
    ofn.nMaxFile = MAX_CHAR_PATH;
    ofn.lpstrTitle = L"Select App Icon";
    ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    if (GetOpenFileName(&ofn))
    {
        pImg = Image::FromFile(wsIcoFile);
        if (pImg)
        {
            iIcoWidth = pImg->GetWidth();
            iIcoHeight = pImg->GetHeight();
            delete pImg;
            pImg = NULL;
        }

        if (iIcoWidth <= 0 || iIcoHeight <= 0)
        {
            MessageBox(_hWnd, L"You have selected an invalid icon file. Please try again.",
                       L"Error", MB_OK | MB_ICONERROR);
            return FALSE;
        }
        hIcoFile = CreateFile(wsIcoFile, GENERIC_READ, FILE_SHARE_READ,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hIcoFile == NULL || hIcoFile == INVALID_HANDLE_VALUE)
        {
            MessageBox(_hWnd, L"There was an error trying to integrate " \
                       L"the selected icon into the theme. Please try again.",
                       L"Error", MB_OK | MB_ICONERROR);
            return FALSE;
        }
        _CurrentTheme.IconImageSize = GetFileSize(hIcoFile, NULL);
        if (_CurrentTheme.IconImageData)
        {
            LocalFree(_CurrentTheme.IconImageData);
            _CurrentTheme.IconImageData = NULL;
        }
        _CurrentTheme.IconImageData = (LPBYTE)LocalAlloc(LPTR, _CurrentTheme.IconImageSize);
        ReadFile(hIcoFile, (_CurrentTheme.IconImageData),
                 (_CurrentTheme.IconImageSize), &(_CurrentTheme.IconImageSize), NULL);
        CloseHandle(hIcoFile);
        hIcoFile = NULL;
        SetWindowText(_hTxtAppIcon, wsIcoFile);
        this->_FlagChange();
        return TRUE;
    }
    return FALSE;
}

BOOL THEMERWINDOW::_ResetAppIcon()
{
    // Invalid or redundant changes will not trigger anything
    if (_btUIState == THEMERWINDOW::_UI_STATE_READY) // Redundant State
        return FALSE;

    if (_CurrentTheme.IconImageSize == 0) // Nothing to reset
        return FALSE;

    if (_CurrentTheme.IconImageData)
    {
        LocalFree(_CurrentTheme.IconImageData);
        _CurrentTheme.IconImageData = NULL;
    }
    _CurrentTheme.IconImageSize = 0;
    SetWindowText(_hTxtAppIcon, _ws_DEF_APP_ICON);
    this->_FlagChange();

    return TRUE;
}

void THEMERWINDOW::_FlagChange()
{
    // Invalid or redundant changes will not trigger anything
    if (_btUIState == THEMERWINDOW::_UI_STATE_READY) // Redundant State
        return;

    if (_btUIState == THEMERWINDOW::_UI_STATE_NEW_NO_CHANGE ||
        _btUIState == THEMERWINDOW::_UI_STATE_NEW_CHANGED)
        this->_HandleUIState(THEMERWINDOW::_UI_STATE_NEW_CHANGED, FALSE);
    else if (_btUIState == THEMERWINDOW::_UI_STATE_OPEN_NO_CHANGE ||
             _btUIState == THEMERWINDOW::_UI_STATE_OPEN_CHANGED)
        this->_HandleUIState(THEMERWINDOW::_UI_STATE_OPEN_CHANGED, FALSE);
    this->_SetCTLColorBrushes();
    return;
}


THEMERWINDOW::THEMERWINDOW() : _DefaultTheme(_DefaultSettings.Theme)
{
    int                 iScreenWidth, iScreenHeight, iXPos, iYPos;
    int                 iWidth, iHeight, iBtnImgDim;
    RECT                rctWindow, rctWorkArea, rctStatusBar { 0 };
    HWND                hWndDummySB = NULL;
    RECT                rctMrg = { 5, 1, 1, 1 };
    BUTTON_IMAGELIST    biml { 0 };
    const UINT          uiILC_FLAGS = ILC_COLOR32 | ILC_MASK;
    WNDCLASSEX          wcex { 0 };
    MENUITEMINFO        mii { 0 };

    const wchar_t   *wsClass = L"DEWDROP.THEMER.APP.CLASS";
    const DWORD     dwWinStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
    const DWORD     dwWinExStyle = WS_EX_ACCEPTFILES;

    const DWORD     dwLblStyle = WS_CHILD | WS_VISIBLE;
    const DWORD     dwActiveLblStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | SS_NOTIFY;
    const DWORD     dwGrpStyle = WS_CHILD | WS_VISIBLE | BS_GROUPBOX;
    const DWORD     dwCmbStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST;
    const DWORD     dwOptStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON;
    const DWORD     dwChkStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX;
    const DWORD     dwROTxtStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_READONLY;
    const DWORD     dwTxtStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL;
    const DWORD     dwCmdStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;

    _iDPI = _DefaultSettings.Theme.DPI;
    _fScale = _DefaultSettings.Theme.Scale;
    _hInstance = GetModuleHandle(NULL);
    _DefaultSettings.DewMode = 0;
    iBtnImgDim = _F(DEWUI_DIM_SYM_BUTTON_ICON);

    // Apparently, there's no fucking way to know the default height of a status bar.
    // So we do a nasty workaround of creating an invisible status bar, and grabbing
    // the WINDOW (note: NOT client!) height of it and add it to the client area
    hWndDummySB = CreateWindowEx(0, STATUSCLASSNAME, L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, _hInstance, NULL);
    GetWindowRect(hWndDummySB, &rctStatusBar);
    DestroyWindow(hWndDummySB);
    hWndDummySB = NULL;
    _iSBHeight = (rctStatusBar.bottom - rctStatusBar.top);

    ZeroMemory(_wsTitlePart, DEWTHEME_ATTR_LENGTH * sizeof(wchar_t));
    StringCchPrintf(_wsTitlePart, 64, L"Dew Drop Player Themer (v %u.%u)", _DefaultTheme.VersionMajor, _DefaultTheme.VersionMinor);

    rctWindow.left = 0;
    rctWindow.top = 0;
    rctWindow.right = _F((DEWUI_DIM_WINDOW_X * 2 + 30)); // Padding of 10 pixels in left, divider (between preview window and controls), right
    rctWindow.bottom = _F((DEWUI_DIM_WINDOW_Y + 20)) + _iSBHeight; // Padding of 10 pixels in top & bottom

    AdjustWindowRectEx(&rctWindow, dwWinStyle, TRUE, dwWinExStyle);
    iWidth = rctWindow.right - rctWindow.left;
    iHeight = rctWindow.bottom - rctWindow.top;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);
    iScreenWidth = rctWorkArea.right - rctWorkArea.left;
    iScreenHeight = rctWorkArea.bottom - rctWorkArea.top;
    iXPos = (iScreenWidth - iWidth)/2 + rctWorkArea.left;
    iYPos = (iScreenHeight - iHeight)/2 + rctWorkArea.top;

    _iXOffset = _F((DEWUI_DIM_WINDOW_X + 20));
    _iYOffset = _F(10);

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = DefWindowProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = _hInstance;
    wcex.hIcon          = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_APP_THEMER));
    wcex.hIconSm        = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_APP_THEMER));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = wsClass;

    RegisterClassEx(&wcex);

    this->_CreateMainMenu();

    _hWnd = CreateWindowEx(dwWinExStyle, wsClass, _wsTitlePart, dwWinStyle,
                           iXPos, iYPos, iWidth, iHeight, NULL, _hMnuMain, _hInstance, NULL);

    _lpPreviewWindow = new DEWWINDOW(_DefaultSettings, _hWnd);
    EnableWindow(_lpPreviewWindow->Handle, FALSE);

    HWND        hWndChild = NULL;
    do
    {
        hWndChild = FindWindowEx(_lpPreviewWindow->Handle, hWndChild, NULL, NULL);
        if (hWndChild) EnableWindow(hWndChild, FALSE);
    } while (hWndChild);

    _lpPassWindow = new THEMEPASSWORDWINDOW(_hWnd, _CurrentTheme, _iDPI, _fScale);
    _lpAboutWindow = new THEMERABOUTWINDOW(_hWnd, _iDPI, _fScale);
    _lpIconRepo = new DEWICONREPOSITORY(_iDPI, _fScale);

    // Theme Info Section ------------------------------------------------------
    _hGrpInfo = CreateWindowEx(0, L"BUTTON", L"Theme Information", dwGrpStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblName = CreateWindowEx(0, L"STATIC", L"Name", dwLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hTxtName = CreateWindowEx(WS_EX_STATICEDGE, L"EDIT", L"", dwTxtStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblAuthor = CreateWindowEx(0, L"STATIC", L"Author", dwLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hTxtAuthor = CreateWindowEx(WS_EX_STATICEDGE, L"EDIT", L"", dwTxtStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    // -------------------------------------------------------------------------


    // Theme Color Section -----------------------------------------------------
    _hGrpColors = CreateWindowEx(0, L"BUTTON", L"Theme Colors", dwGrpStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblColor = CreateWindowEx(0, L"STATIC", L"Select Component", dwLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmbColorComponent = CreateWindowEx(0, L"COMBOBOX", L"", dwCmbStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblBackColor = CreateWindowEx(0, L"STATIC", L"Background Color", dwLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblBackColorPreview = CreateWindowEx(0, L"STATIC", L"", dwActiveLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblBackColor2 = CreateWindowEx(0, L"STATIC", L"Background Color 2", dwLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblBackColor2Preview = CreateWindowEx(0, L"STATIC", L"", dwActiveLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblTextColor = CreateWindowEx(0, L"STATIC", L"Text Color", dwLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblTextColorPreview = CreateWindowEx(0, L"STATIC", L"", dwActiveLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblOutlineColor = CreateWindowEx(0, L"STATIC", L"Outline Color", dwLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblOutlineColorPreview = CreateWindowEx(0, L"STATIC", L"", dwActiveLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    // -------------------------------------------------------------------------


    // Theme Font Section ------------------------------------------------------
    _hGrpFonts = CreateWindowEx(0, L"BUTTON", L"Theme Fonts", dwGrpStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblTitleFont = CreateWindowEx(0, L"STATIC", L"Title Font Preview", dwLblStyle | WS_BORDER, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmdTitleFont = CreateWindowEx(0, L"BUTTON", L"", dwCmdStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblTextFont = CreateWindowEx(0, L"STATIC", L"Text Font Preview", dwLblStyle | WS_BORDER, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmdTextFont = CreateWindowEx(0, L"BUTTON", L"", dwCmdStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    // -------------------------------------------------------------------------


    // Theme Icon Section ------------------------------------------------------
    _hGrpIconStyle = CreateWindowEx(0, L"BUTTON", L"Theme Icon Styles", dwGrpStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblIconMode = CreateWindowEx(0, L"STATIC", L"Icon Mode", dwLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hOptIconModeDefault = CreateWindowEx(0, L"BUTTON", L"Default Mode", dwOptStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hOptIconModeAlternate = CreateWindowEx(0, L"BUTTON", L"Alternate Mode", dwOptStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblIconScale = CreateWindowEx(0, L"STATIC", L"Icon Scale", dwLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmbIconScale = CreateWindowEx(0, L"COMBOBOX", L"", dwCmbStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hChkTransparentIcon = CreateWindowEx(0, L"BUTTON", L"Transparent Icons", dwChkStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hChkSeekbarTimes = CreateWindowEx(0, L"BUTTON", L"Show Seekbar Times", dwChkStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    // -------------------------------------------------------------------------


    // Theme Images ------------------------------------------------------------
    _hGrpImages = CreateWindowEx(0, L"BUTTON", L"Theme Images", dwGrpStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblBackground = CreateWindowEx(0, L"STATIC", L"Theme Background Image:", dwLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hTxtBackground = CreateWindowEx(WS_EX_STATICEDGE, L"EDIT", L"", dwROTxtStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmdBackground = CreateWindowEx(0, L"BUTTON", L"", dwCmdStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmdResetBackground = CreateWindowEx(0, L"BUTTON", L"Reset", dwCmdStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblAppIcon = CreateWindowEx(0, L"STATIC", L"Theme Player Icon:", dwLblStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hTxtAppIcon = CreateWindowEx(WS_EX_STATICEDGE, L"EDIT", L"", dwROTxtStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmdAppIcon =  CreateWindowEx(0, L"BUTTON", L"", dwCmdStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmdResetAppIcon = CreateWindowEx(0, L"BUTTON", L"Reset", dwCmdStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    // -------------------------------------------------------------------------


    _hCmdPwd = CreateWindowEx(0, L"BUTTON", L"&Password", dwCmdStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmdClose =  CreateWindowEx(0, L"BUTTON", L"&Close", dwCmdStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, _hWnd, NULL, NULL, NULL);
    _hStsInfo = CreateWindowEx(WS_EX_STATICEDGE, STATUSCLASSNAME, L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);

    // Do NOT disrupt this sequence. We will no go with CBS_HASSTRINGS.
    // Instead we will simply get the index to control our logic
    SendMessage(_hCmbColorComponent, CB_ADDSTRING, 0, (LPARAM)L"Main Player Window"); // THEMER_COMPONENT_SEQ_WINDOW
    SendMessage(_hCmbColorComponent, CB_ADDSTRING, 0, (LPARAM)L"UI Buttons (Minimize/Close)"); // THEMER_COMPONENT_SEQ_UI_BUTTONS
    SendMessage(_hCmbColorComponent, CB_ADDSTRING, 0, (LPARAM)L"Multimedia Buttons"); // THEMER_COMPONENT_SEQ_MM_BUTTONS
    SendMessage(_hCmbColorComponent, CB_ADDSTRING, 0, (LPARAM)L"Player modules (Album Art, Playlist, Seekbar)"); // THEMER_COMPONENT_SEQ_MODULES

    // Do NOT disrupt this sequence. We will no go with CBS_HASSTRINGS.
    // Instead we will simply get the index to control our logic
    SendMessage(_hCmbIconScale, CB_ADDSTRING, 0, (LPARAM)L"Small Icons"); // DEWTHEME_ICON_SCALE_SMALL
    SendMessage(_hCmbIconScale, CB_ADDSTRING, 0, (LPARAM)L"Medium Icons"); // DEWTHEME_ICON_SCALE_MEDIUM
    SendMessage(_hCmbIconScale, CB_ADDSTRING, 0, (LPARAM)L"Large Icons"); // DEWTHEME_ICON_SCALE_LARGE

    SetClassLongPtr(_hLblBackColorPreview, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));
    SetClassLongPtr(_hLblBackColor2Preview, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));
    SetClassLongPtr(_hLblTextColorPreview, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));
    SetClassLongPtr(_hLblOutlineColorPreview, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));


    // Button Icons ------------------------------------------------------------
    biml.margin = rctMrg;

    // Browse Buttons
    rctMrg.left = 1;
    biml.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;

    _hImlBtnBrowse = ImageList_Create(iBtnImgDim, iBtnImgDim, uiILC_FLAGS, 1, 1);
    ImageList_ReplaceIcon(_hImlBtnBrowse, -1,
                          (HICON)LoadImage(_hInstance, MAKEINTRESOURCE(IDI_THEMER_BROWSE), IMAGE_ICON, iBtnImgDim, iBtnImgDim, LR_DEFAULTCOLOR));
    biml.himl = _hImlBtnBrowse;
    SendMessage(_hCmdTitleFont, BCM_SETIMAGELIST, 0, (LPARAM)&biml);
    SendMessage(_hCmdTextFont, BCM_SETIMAGELIST, 0, (LPARAM)&biml);
    SendMessage(_hCmdBackground, BCM_SETIMAGELIST, 0, (LPARAM)&biml);
    SendMessage(_hCmdAppIcon, BCM_SETIMAGELIST, 0, (LPARAM)&biml);

    biml.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;
    rctMrg.left = 5;

    // Reset Buttons
    _hImlBtnReset = ImageList_Create(iBtnImgDim, iBtnImgDim, uiILC_FLAGS, 1, 1);
    ImageList_ReplaceIcon(_hImlBtnReset, -1,
                          (HICON)LoadImage(_hInstance, MAKEINTRESOURCE(IDI_THEMER_RESET_IMAGE), IMAGE_ICON, iBtnImgDim, iBtnImgDim, LR_DEFAULTCOLOR));
    biml.himl = _hImlBtnReset;
    SendMessage(_hCmdResetBackground, BCM_SETIMAGELIST, 0, (LPARAM)&biml);
    SendMessage(_hCmdResetAppIcon, BCM_SETIMAGELIST, 0, (LPARAM)&biml);

    // Password
    _hImlBtnPassword = ImageList_Create(iBtnImgDim, iBtnImgDim, uiILC_FLAGS, 1, 1);
    ImageList_ReplaceIcon(_hImlBtnPassword, -1,
                          (HICON)LoadImage(_hInstance, MAKEINTRESOURCE(IDI_THEMER_PASSWORD), IMAGE_ICON, iBtnImgDim, iBtnImgDim, LR_DEFAULTCOLOR));
    biml.himl = _hImlBtnPassword;
    SendMessage(_hCmdPwd, BCM_SETIMAGELIST, 0, (LPARAM)&biml);

    // Close Button
    _hImlBtnClose = ImageList_Create(iBtnImgDim, iBtnImgDim, uiILC_FLAGS, 1, 1);
    ImageList_ReplaceIcon(_hImlBtnClose, -1,
                          (HICON)LoadImage(_hInstance, MAKEINTRESOURCE(IDI_THEMER_CLOSE), IMAGE_ICON, iBtnImgDim, iBtnImgDim, LR_DEFAULTCOLOR));
    biml.himl = _hImlBtnClose;
    SendMessage(_hCmdClose, BCM_SETIMAGELIST, 0, (LPARAM)&biml);

    // -------------------------------------------------------------------------


    // Menu Icons --------------------------------------------------------------
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_BITMAP;

    mii.hbmpItem = _lpIconRepo->BmpOpenFile;
    SetMenuItemInfo(_hMnuMain, IDM_FILE_NEW, FALSE, &mii);

    mii.hbmpItem = _lpIconRepo->BmpOpenDir;
    SetMenuItemInfo(_hMnuMain, IDM_FILE_OPEN, FALSE, &mii);

    mii.hbmpItem = _lpIconRepo->BmpSave;
    SetMenuItemInfo(_hMnuMain, IDM_FILE_SAVE, FALSE, &mii);

    mii.hbmpItem = _lpIconRepo->BmpQuit;
    SetMenuItemInfo(_hMnuMain, IDM_FILE_EXIT, FALSE, &mii);

    mii.hbmpItem = _lpIconRepo->BmpResetLayout;
    SetMenuItemInfo(_hMnuMain, IDM_THEME_RESET, FALSE, &mii);

    mii.hbmpItem = _lpIconRepo->BmpDefaultTheme;
    SetMenuItemInfo(_hMnuMain, IDM_THEME_DEFAULT, FALSE, &mii);

    mii.hbmpItem = _lpIconRepo->BmpAbout;
    SetMenuItemInfo(_hMnuMain, IDM_HELP_ABOUT, FALSE, &mii);
    // -------------------------------------------------------------------------

    SetWindowPos(_hToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    _hWndFocus = _hWnd;
    _hDC = GetDC(_hWnd);
    this->_ApplyUIFont();

    _btUIState = THEMERWINDOW::_UI_STATE_READY;
    ZeroMemory(_wsThemeFilePath, MAX_CHAR_PATH * sizeof(wchar_t));
    this->_HandleUIState(THEMERWINDOW::_UI_STATE_READY);
    SetWindowSubclass(_hWnd, _ThemerMsgHandler, (UINT_PTR)_hWnd, (DWORD_PTR)this);

    return;
}

int THEMERWINDOW::Run()
{
    MSG         msg { 0 };
    int         iArgs = 0;
    LPWSTR      *ppArgs;

    ShowWindow(_hWndFocus, SW_SHOWNORMAL);
    UpdateWindow(_hWndFocus);

    // Check if any theme file was sent via command line
    ppArgs = CommandLineToArgvW(GetCommandLine(), &iArgs);
    if (iArgs > 1)
        this->_OpenTheme(ppArgs[1]);

    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        if (_hWndFocus == _hWnd)
        {
            // The accelerators are applicable only for the main UI
            // Other windows should not be impacted by this
            if(!TranslateAccelerator(_hWndFocus, _hAccel, &msg) &&
               !IsDialogMessage(_hWndFocus, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if(!IsDialogMessage(_hWndFocus, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    return (int)msg.wParam;
}

THEMERWINDOW::~THEMERWINDOW()
{
    HWND        hWndChild = NULL, hWndDelChild = NULL;

    if (_lpPreviewWindow) { delete _lpPreviewWindow; _lpPreviewWindow = NULL; }

    hWndChild = FindWindowEx(_hWnd, hWndChild, NULL, NULL);
    do
    {
        hWndDelChild = hWndChild;
        hWndChild = FindWindowEx(_hWnd, hWndChild, NULL, NULL);

        if(hWndDelChild)
            DestroyWindow(hWndDelChild);
    } while(hWndChild);

    DestroyMenu(_hMnuMain);
    this->_SetCTLColorBrushes(TRUE);

    RemoveWindowSubclass(_hWnd, _ThemerMsgHandler, (UINT_PTR)_hWnd);
    ReleaseDC(_hWnd, _hDC);

    if (_hFntUI) { DeleteObject(_hFntUI); _hFntUI = NULL; }
    if (_hFntTitle) { DeleteObject(_hFntTitle); _hFntTitle = NULL; }
    if (_hFntText) { DeleteObject(_hFntText); _hFntText = NULL; }
    if (_lpPassWindow) { delete _lpPassWindow; _lpPassWindow = NULL; }
    if (_lpAboutWindow) { delete _lpAboutWindow; _lpAboutWindow = NULL; }

    if (_hImlBtnBrowse) { ImageList_Destroy(_hImlBtnBrowse); _hImlBtnBrowse = NULL; }
    if (_hImlBtnReset) { ImageList_Destroy(_hImlBtnReset); _hImlBtnReset = NULL; }
    if (_hImlBtnPassword) { ImageList_Destroy(_hImlBtnPassword); _hImlBtnPassword = NULL; }
    if (_hImlBtnClose) { ImageList_Destroy(_hImlBtnClose); _hImlBtnClose = NULL; }

    _hWndFocus = NULL;
    _hWnd = NULL;

    return;
}
