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

#include "ThemePasswordWindow.h"


LRESULT CALLBACK THEMEPASSWORDWINDOW::_TPMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    return ((LPTHEMEPASSWORDWINDOW)dwRefData)->_WndProc(hWnd, nMsg, wParam, lParam);
}

LRESULT THEMEPASSWORDWINDOW::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC     hDC;
    PAINTSTRUCT     ps { 0 };

    switch (nMsg)
    {
        case WM_PAINT:
        {
            hDC = BeginPaint(hWnd, &ps);
            DrawEdge(hDC, &_rctEtch, EDGE_ETCHED, BF_RECT);
            DrawIconEx(hDC, _ptIcon.x, _ptIcon.y, _hIcoPwd, _iIcoDim, _iIcoDim, 0, NULL, DI_NORMAL);
            EndPaint(hWnd, &ps);
            return FALSE;
        }
        case WM_SIZE:
        {
            this->_HandleSizing();
            return FALSE;
        }
        case WM_COMMAND:
        {
            if ( (lParam == 0x00 && LOWORD(wParam) == IDCANCEL) ||
                 ((HWND)lParam == _hCmdCancel) )
                SendMessage(_hWnd, WM_CLOSE, FALSE, 0);
            else if ( (lParam == 0x00 && LOWORD(wParam) == IDOK) ||
                      ((HWND)lParam == _hCmdOK) )
            {
                if (!this->_Verify())
                    SendMessage(_hWnd, WM_CLOSE, 0, 0);
            }
            break;
        }
        case WM_CLOSE:
        {
            ShowWindow(_hWnd, SW_HIDE);
            if (_hWndParent)
            {
                PostMessage(_hWndParent, WM_DEWMSG_CHILD_CLOSED, 0, (LPARAM)_hWnd);
                SetForegroundWindow(_hWndParent);
            }
            return FALSE;
        }
    }
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void THEMEPASSWORDWINDOW::_SetUIFont()
{
    LOGFONT     lgfFont { 0 };
    HWND        hWndChild = NULL;

    if (_hFntUI) { DeleteObject(_hFntUI); _hFntUI = NULL; }

    lgfFont.lfHeight = -MulDiv(8, _iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = FW_NORMAL;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, L"Tahoma", 32 * sizeof(wchar_t));

    _hFntUI = CreateFontIndirect(&lgfFont);

    do
    {
        hWndChild = FindWindowEx(_hWnd, hWndChild, NULL, NULL);
        if (hWndChild)
            SendMessage(hWndChild, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));
    } while (hWndChild);

    return;
}

void THEMEPASSWORDWINDOW::_HandleSizing()
{
    RECT        rctClient { 0 };
    int         iX, iY, iW, iH;
    wchar_t     wsHeader[MAX_PATH];

    if (_browseAction.ActionMode < THEMER_PW_MODE_CHECK ||
        _browseAction.ActionMode > THEMER_PW_MODE_CHANGE)
        return;

    GetClientRect(_hWnd, &rctClient);

    // Hide all the labels/windows
    ShowWindow(_hTxtOldPwd, SW_HIDE);
    ShowWindow(_hTxtCurrPwd, SW_HIDE);
    ShowWindow(_hTxtVerifyCurrPwd, SW_HIDE);

    // Clear all the texts
    SetWindowText(_hTxtOldPwd, L"");
    SetWindowText(_hTxtCurrPwd, L"");
    SetWindowText(_hTxtVerifyCurrPwd, L"");

    iX = _F(10); iY = _F(10); iW = rctClient.right - _F(20); iH = _F(40);
    SetWindowPos(_hLblBanner, 0, iX, iY, iW, iH, SWP_NOZORDER);

    switch (_browseAction.ActionMode)
    {
        case THEMER_PW_MODE_CHECK:
        {
            iX = _F(70); iY = _F(70); iW = rctClient.right - _F(90); iH = _F(20);
            SetWindowPos(_hTxtCurrPwd, 0, iX, iY, iW, iH, SWP_NOZORDER);

            ShowWindow(_hTxtCurrPwd, SW_SHOW);
            SendMessage(_hTxtCurrPwd, EM_SETCUEBANNER, TRUE, (LPARAM)L" Theme password");
            StringCchPrintf(wsHeader, MAX_PATH, L"Enter the password to open the theme.\n%s", _wsDisclaimer);
            SetWindowText(_hLblBanner, wsHeader);
            _rctEtch.left = _F(10); _rctEtch.top = _F(50); _rctEtch.right = rctClient.right - _F(10); _rctEtch.bottom = _F(110);
            SetFocus(_hTxtCurrPwd);
            break;
        }

        case THEMER_PW_MODE_SET:
        {
            iX = _F(70); iY = _F(70); iW = rctClient.right - _F(90); iH = _F(20);
            SetWindowPos(_hTxtCurrPwd, 0, iX, iY, iW, iH, SWP_NOZORDER);

            iX = _F(70); iY = _F(100); iW = rctClient.right - _F(90); iH = _F(20);
            SetWindowPos(_hTxtVerifyCurrPwd, 0, iX, iY, iW, iH, SWP_NOZORDER);

            ShowWindow(_hTxtCurrPwd, SW_SHOW);
            ShowWindow(_hTxtVerifyCurrPwd, SW_SHOW);
            SendMessage(_hTxtCurrPwd, EM_SETCUEBANNER, TRUE, (LPARAM)L" Enter password");
            SendMessage(_hTxtVerifyCurrPwd, EM_SETCUEBANNER, TRUE, (LPARAM)L" Confirm password");
            StringCchPrintf(wsHeader, MAX_PATH, L"Create a password for the theme file.\n%s", _wsDisclaimer);
            SetWindowText(_hLblBanner, wsHeader);
            _rctEtch.left = _F(10); _rctEtch.top = _F(50); _rctEtch.right = rctClient.right - _F(10); _rctEtch.bottom = _F(140);
            SetFocus(_hTxtCurrPwd);
            break;
        }

        case THEMER_PW_MODE_CHANGE:
        {
            iX = _F(70); iY = _F(70); iW = rctClient.right - _F(90); iH = _F(20);
            SetWindowPos(_hTxtOldPwd, 0, iX, iY, iW, iH, SWP_NOZORDER);

            iX = _F(70); iY = _F(100); iW = rctClient.right - _F(90); iH = _F(20);
            SetWindowPos(_hTxtCurrPwd, 0, iX, iY, iW, iH, SWP_NOZORDER);

            iX = _F(70); iY = _F(130); iW = rctClient.right - _F(90); iH = _F(20);
            SetWindowPos(_hTxtVerifyCurrPwd, 0, iX, iY, iW, iH, SWP_NOZORDER);

            ShowWindow(_hTxtOldPwd, SW_SHOW);
            ShowWindow(_hTxtCurrPwd, SW_SHOW);
            ShowWindow(_hTxtVerifyCurrPwd, SW_SHOW);
            SendMessage(_hTxtOldPwd, EM_SETCUEBANNER, TRUE, (LPARAM)L" Old password");
            SendMessage(_hTxtCurrPwd, EM_SETCUEBANNER, TRUE, (LPARAM)L" New password");
            SendMessage(_hTxtVerifyCurrPwd, EM_SETCUEBANNER, TRUE, (LPARAM)L" Confirm password");
            StringCchPrintf(wsHeader, MAX_PATH, L"Update the information below to change the password.\n%s", _wsDisclaimer);
            SetWindowText(_hLblBanner, wsHeader);
            _rctEtch.left = _F(10); _rctEtch.top = _F(50); _rctEtch.right = rctClient.right - _F(10); _rctEtch.bottom = _F(170);
            SetFocus(_hTxtOldPwd);
            break;
        }
    }

    iX = rctClient.right - _F(85); iY = rctClient.bottom - _F(40); iW = _F(75); iH = _F(30);
    SetWindowPos(_hCmdOK, 0, iX, iY, iW, iH, SWP_NOZORDER);

    iX = _F(10); iY = rctClient.bottom - _F(40); iW = _F(75); iH = _F(30);
    SetWindowPos(_hCmdCancel, 0, iX, iY, iW, iH, SWP_NOZORDER);

    return;
}

BOOL THEMEPASSWORDWINDOW::_Verify()
{
    char        sOldPass[DEWTHEME_ATTR_LENGTH] { 0 };
    char        sCurrPass[DEWTHEME_ATTR_LENGTH] { 0 };
    char        sVerifyPass[DEWTHEME_ATTR_LENGTH] { 0 };
    DEWTHEME    tmpOpenTheme;
    BYTE        btRetVal;
    WORD        wThemeVerMajor, wThemeVerMinor;
    wchar_t     wsText[MAX_PATH];

    switch (_browseAction.ActionMode)
    {
        // Open Theme
        case THEMER_PW_MODE_CHECK:
        {
            // [No Password provided]
            GetWindowTextA(_hTxtCurrPwd, sCurrPass, DEWTHEME_ATTR_LENGTH - 1);
            if (lstrlenA(sCurrPass) <= 0)
            {
                _browseAction.ReturnCode = THEMER_CHECK_NO_PASSWORD;
                SetFocus(_hTxtCurrPwd);
                MessageBox(_hWndParent, L"You've not provided any password. " \
                           L"Please provide the password to open the theme.",
                           L"Missing Password", MB_OK | MB_ICONEXCLAMATION);
                return TRUE; // Keep the user in the screen
            }

            btRetVal = tmpOpenTheme.LoadThemeFile(_browseAction.ThemeFile, &wThemeVerMajor, &wThemeVerMinor);
            switch (btRetVal)
            {
                // [Generic open error]
                case DEWTHEME_ACTION_DEFAULT:
                case DEWTHEME_ACTION_THEME_OVERRIDE:
                case DEWTHEME_ACTION_THEME_FAIL:
                {
                    _browseAction.ReturnCode = THEMER_CHECK_LOAD_FAIL;
                    MessageBox(_hWndParent, L"There was an error trying to open the provided theme file. Please try again.",
                               L"Error", MB_OK | MB_ICONERROR);
                    return FALSE; // User will have to select another theme file
                }
                // [Version Mismatch]
                case DEWTHEME_ACTION_THEME_VER_MISMATCH:
                {
                    _browseAction.ReturnCode = THEMER_CHECK_LOAD_FAIL;
                    StringCchPrintf(wsText, MAX_PATH, L"The selected theme (version %u.%u) is " \
                                    L"not compatible with the current version of " \
                                    L"Dew Drop Themer (version %u.%u). Please select " \
                                    L"a version compatible theme.", wThemeVerMajor, wThemeVerMinor,
                                    tmpOpenTheme.VersionMajor, tmpOpenTheme.VersionMinor);
                    MessageBox(_hWnd, wsText, L"Error", MB_OK | MB_ICONERROR);
                    return FALSE; // User will have to select another theme
                }

                case DEWTHEME_ACTION_THEME_APPLIED:
                {
                    // [Incorrect Password]
                    if (lstrcmpA(sCurrPass, tmpOpenTheme.Password))
                    {
                        _browseAction.ReturnCode = THEMER_CHECK_INCORRECT_PASSWORD;
                        MessageBox(_hWndParent, L"The password is incorrect. Please try again.",
                                   L"Incorrect Password", MB_OK | MB_ICONEXCLAMATION);
                        SetFocus(_hTxtCurrPwd);
                        return TRUE; // Keep the user in the screen
                    }

                    // [Success]
                    _browseAction.ReturnCode = THEMER_CHECK_OK;
                    _TargetTheme = tmpOpenTheme;
                    return FALSE; // User is good to go!
                }
                default:
                {
                    _browseAction.ReturnCode = DEWTHEME_ACTION_THEME_FAIL;
                    return FALSE;
                }
            }
        }

        // Save Theme (will appear only once when setting password)
        case THEMER_PW_MODE_SET:
        {
            GetWindowTextA(_hTxtCurrPwd, sCurrPass, DEWTHEME_ATTR_LENGTH - 1);
            GetWindowTextA(_hTxtVerifyCurrPwd, sVerifyPass, DEWTHEME_ATTR_LENGTH - 1);

            // [One of the password fields is empty]
            if (lstrlenA(sCurrPass) <= 0 || lstrlenA(sVerifyPass) <= 0)
            {
                _browseAction.ReturnCode = THEMER_SET_NO_PASSWORD;
                MessageBox(_hWndParent, L"One or both the password field(s) is/are empty. Please provide the same valid password in both the fields.",
                           L"Missing Password", MB_OK | MB_ICONEXCLAMATION);
                if (lstrlenA(sCurrPass) <= 0) SetFocus(_hTxtCurrPwd);
                else if (lstrlenA(sVerifyPass) <= 0) SetFocus(_hTxtVerifyCurrPwd);
                return TRUE; // Keep the user in the screen
            }

            // [The passwords do not match]
            if (lstrcmpA(sCurrPass, sVerifyPass))
            {
                _browseAction.ReturnCode = THEMER_SET_INCORRECT_PASSWORD;
                MessageBox(_hWndParent, L"The passwords do not match. Please provide the same valid password in both the fields.",
                           L"Password Mismatch", MB_OK | MB_ICONEXCLAMATION);
                SetFocus(_hTxtCurrPwd);
                return TRUE; // Keep the user in the screen

            }

            // [Success]
            _browseAction.ReturnCode = THEMER_SET_OK;
            _TargetTheme.SetPassword(sCurrPass);
            return FALSE;
        }

        // Changing Password
        case THEMER_PW_MODE_CHANGE:
        {
            GetWindowTextA(_hTxtOldPwd, sOldPass, DEWTHEME_ATTR_LENGTH - 1);
            GetWindowTextA(_hTxtCurrPwd, sCurrPass, DEWTHEME_ATTR_LENGTH - 1);
            GetWindowTextA(_hTxtVerifyCurrPwd, sVerifyPass, DEWTHEME_ATTR_LENGTH - 1);

            // [One of the password fields is empty]
            if (lstrlenA(sOldPass) <= 0|| lstrlenA(sCurrPass) <= 0 || lstrlenA(sVerifyPass) <= 0)
            {
                _browseAction.ReturnCode = THEMER_CHANGE_NO_PASSWORD;
                MessageBox(_hWndParent, L"One or both the password field(s) is/are empty. Please provide the passwords as indicated.",
                           L"Missing Password", MB_OK | MB_ICONEXCLAMATION);
                if (lstrlenA(sOldPass) <= 0) SetFocus(_hTxtOldPwd);
                else if (lstrlenA(sCurrPass) <= 0) SetFocus(_hTxtCurrPwd);
                else if (lstrlenA(sVerifyPass) <= 0) SetFocus(_hTxtVerifyCurrPwd);
                return TRUE; // Keep the user in the screen
            }

            // [The new passwords do not match]
            if (lstrcmpA(sCurrPass, sVerifyPass))
            {
                _browseAction.ReturnCode = THEMER_CHANGE_INCORRECT_PASSWORD;
                MessageBox(_hWndParent, L"The new passwords do not match. Please provide the same password in the new fields.",
                           L"Password Mismatch", MB_OK | MB_ICONEXCLAMATION);
                SetFocus(_hTxtCurrPwd);
                return TRUE; // Keep the user in the screen

            }

            // [Incorrect Password]
            if (lstrcmpA(sOldPass, _TargetTheme.Password))
            {
                _browseAction.ReturnCode = THEMER_CHANGE_INCORRECT_PASSWORD;
                MessageBox(_hWndParent, L"The old password is incorrect. Please try again.",
                           L"Incorrect Password", MB_OK | MB_ICONEXCLAMATION);
                SetFocus(_hTxtOldPwd);
                return TRUE; // Keep the user in the screen
            }

            // [Same password]
            if (!lstrcmpA(sCurrPass, sOldPass))
            {
                _browseAction.ReturnCode = THEMER_CHANGE_SAME_PASSWORD;
                MessageBox(_hWndParent, L"The new password cannot be the same as old password. Please try again.",
                           L"Incorrect Password", MB_OK | MB_ICONEXCLAMATION);
                SetFocus(_hTxtCurrPwd);
                return TRUE; // Keep the user in the screen
            }

            // [Success]
            _browseAction.ReturnCode = THEMER_SET_OK;
            _TargetTheme.SetPassword(sCurrPass);
            return FALSE;
        }

    }

    _browseAction.ReturnCode = THEMER_CHECK_LOAD_FAIL;
    return FALSE;
}

THEMEPASSWORDWINDOW::THEMEPASSWORDWINDOW(HWND hWndParent, DEWTHEME& TargetTheme, int iDPI, float fScale) :
_TargetTheme(TargetTheme), ThemeAction(_browseAction), Handle(_hWnd)
{
    WNDCLASSEX          wcex { 0 };
    wchar_t             wsPassChar;
    int                 iBtnImgDim;
    RECT                rctMrg = { 5, 1, 1, 1 };
    BUTTON_IMAGELIST    biml { 0 };

    const wchar_t   *wsClass = L"DEWDROP.THEMER.PWD.CLASS";
    const DWORD     dwBnrStyle = WS_CHILD | WS_VISIBLE;
    const DWORD     dwLblStyle = WS_CHILD | WS_VISIBLE | SS_RIGHT;
    const DWORD     dwTxtStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_PASSWORD;
    const DWORD     dwCmdStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_FLAT;

    const UINT      uiILC_FLAGS = ILC_COLOR32 | ILC_MASK;

    _hInstance = GetModuleHandle(NULL);
    _hWndParent = hWndParent;
    _iDPI = iDPI;
    _fScale = fScale;
    _browseAction.ActionMode = 0x00;
    _browseAction.ReturnCode = DEWTHEME_ACTION_THEME_FAIL;
    ZeroMemory(_browseAction.ThemeFile, MAX_CHAR_PATH * sizeof(wchar_t));
    iBtnImgDim =_F(DEWUI_DIM_SYM_BUTTON_ICON);
    _iIcoDim = _F(32);

    CopyMemory(&wsPassChar, &_wPassChar, sizeof(WORD));

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
    wcex.lpszClassName  = wsClass;

    RegisterClassEx(&wcex);

    _hWnd = CreateWindowEx(_WINEXSTYLE, wsClass, L"Theme Password", _WINSTYLE, 0, 0, 0, 0, _hWndParent, NULL, _hInstance, NULL);
    _hLblBanner = CreateWindowEx(0, L"STATIC", _wsDisclaimer, dwBnrStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hTxtOldPwd = CreateWindowEx(WS_EX_STATICEDGE, L"EDIT", L"", dwTxtStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hTxtCurrPwd = CreateWindowEx(WS_EX_STATICEDGE, L"EDIT", L"", dwTxtStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hTxtVerifyCurrPwd = CreateWindowEx(WS_EX_STATICEDGE, L"EDIT", L"", dwTxtStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmdOK = CreateWindowEx(0, L"BUTTON", L"&OK", dwCmdStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmdCancel = CreateWindowEx(0, L"BUTTON", L"&Cancel", dwCmdStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);

    SendMessage(_hTxtOldPwd, EM_SETPASSWORDCHAR, (WPARAM)wsPassChar, 0);
    SendMessage(_hTxtCurrPwd, EM_SETPASSWORDCHAR, (WPARAM)wsPassChar, 0);
    SendMessage(_hTxtVerifyCurrPwd, EM_SETPASSWORDCHAR, (WPARAM)wsPassChar, 0);

    // Button Images
    biml.margin = rctMrg;
    biml.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;

    _hImlBtnOK = ImageList_Create(iBtnImgDim, iBtnImgDim, uiILC_FLAGS, 1, 1);
    ImageList_ReplaceIcon(_hImlBtnOK, -1,
                          (HICON)LoadImage(_hInstance, MAKEINTRESOURCE(IDI_THEMER_OK), IMAGE_ICON, iBtnImgDim, iBtnImgDim, LR_DEFAULTCOLOR));
    biml.himl = _hImlBtnOK;
    SendMessage(_hCmdOK, BCM_SETIMAGELIST, 0, (LPARAM)&biml);

    _hImlBtnCancel = ImageList_Create(iBtnImgDim, iBtnImgDim, uiILC_FLAGS, 1, 1);
    ImageList_ReplaceIcon(_hImlBtnCancel, -1,
                          (HICON)LoadImage(_hInstance, MAKEINTRESOURCE(IDI_THEMER_CLOSE), IMAGE_ICON, iBtnImgDim, iBtnImgDim, LR_DEFAULTCOLOR));
    biml.himl = _hImlBtnCancel;
    SendMessage(_hCmdCancel, BCM_SETIMAGELIST, 0, (LPARAM)&biml);

    iBtnImgDim = _iIcoDim;
    _hIcoPwd = (HICON)LoadImage(_hInstance, MAKEINTRESOURCE(IDI_THEMER_PASSWORD),
                                IMAGE_ICON, iBtnImgDim, iBtnImgDim, LR_DEFAULTCOLOR);

    this->_SetUIFont();

    SetWindowSubclass(_hWnd, _TPMsgHandler, (UINT_PTR)_hWnd, (DWORD_PTR)this);
}

HWND THEMEPASSWORDWINDOW::Show(BYTE btDisplayMode, LPCWSTR wsOpenThemeFile)
{
    int     iWidth, iHeight, iScreenWidth, iScreenHeight, iXPos, iYPos;
    RECT    rctParent, rctClient;

    _browseAction.ActionMode = btDisplayMode;
    _browseAction.ReturnCode = DEWTHEME_ACTION_THEME_FAIL;

    // Invalid mode check
    if (_browseAction.ActionMode < THEMER_PW_MODE_CHECK ||
        _browseAction.ActionMode > THEMER_PW_MODE_CHANGE)
        return _hWndParent;

    // A valid theme file is required for 'THEMER_PW_MODE_CHECK'
    if (_browseAction.ActionMode == THEMER_PW_MODE_CHECK &&
         (!wsOpenThemeFile || lstrlen(wsOpenThemeFile) <= 0) )
        return _hWndParent;

    iWidth = _F(450);
    rctClient.left = 0; rctClient.right = iWidth; rctClient.top = 0;
    // Flush the internal theme file path on every run
    ZeroMemory(_browseAction.ThemeFile, MAX_CHAR_PATH * sizeof(wchar_t));
    _ptIcon.x = _F(10) + ((_F(60) - _iIcoDim)/2); // (50 - 32)/2

    switch (_browseAction.ActionMode)
    {
        case THEMER_PW_MODE_CHECK:
        {
            rctClient.bottom = _F(160);
            _ptIcon.y = (rctClient.bottom - _iIcoDim)/2;
            AdjustWindowRectEx(&rctClient, _WINSTYLE, FALSE, _WINEXSTYLE);
            iHeight = rctClient.bottom - rctClient.top;
            CopyMemory(_browseAction.ThemeFile, wsOpenThemeFile, lstrlen(wsOpenThemeFile) * sizeof(wchar_t));
            break;
        }
        case THEMER_PW_MODE_SET:
        {
            rctClient.bottom = _F(190);
            _ptIcon.y = (rctClient.bottom - _iIcoDim)/2;
            AdjustWindowRectEx(&rctClient, _WINSTYLE, FALSE, _WINEXSTYLE);
            iHeight = rctClient.bottom - rctClient.top;
            break;
        }
        case THEMER_PW_MODE_CHANGE:
        {
            rctClient.bottom = _F(220);
            _ptIcon.y = (rctClient.bottom - _iIcoDim)/2;
            AdjustWindowRectEx(&rctClient, _WINSTYLE, FALSE, _WINEXSTYLE);
            iHeight = rctClient.bottom - rctClient.top;
            break;
        }
    }

    GetWindowRect(_hWndParent, &rctParent);
    iScreenWidth = rctParent.right - rctParent.left;
    iScreenHeight = rctParent.bottom - rctParent.top;

    iXPos = (iScreenWidth - iWidth) / 2 + rctParent.left;
    iYPos = (iScreenHeight - iHeight) / 2 + rctParent.top;

    if (iXPos < 0) iXPos = 0;
    if (iYPos < 0) iYPos = 0;

    SetWindowPos(_hWnd, 0, iXPos, iYPos, iWidth, iHeight, SWP_NOZORDER);

    ShowWindow(_hWnd, SW_SHOW);
    UpdateWindow(_hWnd);
    SetActiveWindow(_hWnd);
    if (_hWndParent)
        EnableWindow(_hWndParent, FALSE);

    this->_HandleSizing();
    return _hWnd;

}

THEMEPASSWORDWINDOW::~THEMEPASSWORDWINDOW()
{
    HWND        hWndChild = NULL, hWndDelChild = NULL;

    if (_hFntUI) { DeleteObject(_hFntUI); _hFntUI = NULL; }

    RemoveWindowSubclass(_hWnd, _TPMsgHandler, (UINT_PTR)_hWnd);
    hWndChild = FindWindowEx(_hWnd, hWndChild, NULL, NULL);
    do
    {
        hWndDelChild = hWndChild;
        hWndChild = FindWindowEx(_hWnd, hWndDelChild, NULL, NULL);
        if (hWndDelChild) DestroyWindow(hWndDelChild);
    } while (hWndChild);

    DestroyWindow(_hWnd);
    DestroyIcon(_hIcoPwd);

    if (_hImlBtnOK) { ImageList_Destroy(_hImlBtnOK); _hImlBtnOK = NULL; }
    if (_hImlBtnCancel) { ImageList_Destroy(_hImlBtnCancel); _hImlBtnCancel = NULL; }

    return;
}
