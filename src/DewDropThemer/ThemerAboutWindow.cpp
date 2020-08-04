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

#include "ThemerAboutWindow.h"

DWORD WINAPI THEMERABOUTWINDOW::_TrdDisco(LPVOID lpv)
{
    ((LPTHEMERABOUTWINDOW)lpv)->_Disco();
    return 0x00;
}

LRESULT CALLBACK THEMERABOUTWINDOW::_AboutMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    return ((LPTHEMERABOUTWINDOW)dwRefData)->_WndProc(hWnd, nMsg, wParam, lParam);
}

void THEMERABOUTWINDOW::_Disco()
{
    RECT        rctMem;
    int         iR, iG, iB;
    int         iRDir = 1, iGDir = 1, iBDir = 1;
    const int   R_SPEED = 1;
    const int   G_SPEED = 2;
    const int   B_SPEED = 4;
    const wchar_t   *wsDedication = L" ~~~ For RAI ~~~";

    iR = GetRValue(_crText); iG = GetGValue(_crText); iB = GetBValue(_crText);
    SetBkMode(_hDCMem, TRANSPARENT);
    rctMem.left = 0; rctMem.top = 0;
    rctMem.right = _rctDisco.right - _rctDisco.left;
    rctMem.bottom = _rctDisco.bottom - _rctDisco.top;
    do
    {
        if (_bVisible)
        {
            FillRect(_hDCMem, &rctMem, _hbrBack);
            SetTextColor(_hDCMem, RGB(((BYTE)iR), ((BYTE)iG), ((BYTE)iB)));
            DrawText(_hDCMem, wsDedication, -1, &rctMem, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
        }

        iR += (R_SPEED * iRDir); if (iR > 255) { iR = 255; iRDir = -1; } if (iR < 0) { iR = 0; iRDir = 1; }
        iG += (G_SPEED * iGDir); if (iG > 255) { iG = 255; iGDir = -1; } if (iG < 0) { iG = 0; iGDir = 1; }
        iB += (B_SPEED * iBDir); if (iB > 255) { iB = 255; iBDir = -1; } if (iB < 0) { iB = 0; iBDir = 1; }

        if (_bVisible)
            BitBlt(_hDC, _rctDisco.left, _rctDisco.top, rctMem.right, rctMem.bottom, _hDCMem, 0, 0, SRCCOPY);
        Sleep(_DISCO_SPEED);

    } while (_bDiscoReady); // This is an infinite loop anyways. This is just a fancy way to stop the compiler from complaining about it.

    return;
}

LRESULT THEMERABOUTWINDOW::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT     ps { 0 };

    switch(nMsg)
    {
        case WM_PAINT:
        {
            BeginPaint(_hWnd, &ps);
            this->_Draw();
            EndPaint(_hWnd, &ps);
            return FALSE;
        }

        case WM_DRAWITEM:
        {
            if (((LPDRAWITEMSTRUCT)lParam)->hwndItem == _hCmdOK)
            {
                this->_DrawButton(((LPDRAWITEMSTRUCT)lParam), L"&OK");
                return TRUE;
            }
            break;
        }

        case WM_CTLCOLORSTATIC:
        {
            if ((HWND)lParam == _hLblCredit)
            {
                SetBkMode((HDC)wParam, TRANSPARENT);
                SetTextColor((HDC)wParam, _crText);
                return (LRESULT)_hbrBack;
            }
            else if ((HWND)lParam == _hLblURL)
            {
                SetBkMode((HDC)wParam, TRANSPARENT);
                SetTextColor((HDC)wParam, _crURL);
                return (LRESULT)_hbrBack;
            }
            break;
        }

        case WM_SIZE:
        {
            this->_HandleSizing();
            return FALSE;
        }

        case WM_COMMAND:
        {
            if (    ((HWND)lParam == _hCmdOK) ||
                    (lParam == 0x00 && LOWORD(wParam) == IDOK) ||
                    (lParam == 0x00 && LOWORD(wParam) == IDCANCEL)  )
                SendMessage(_hWnd, WM_CLOSE, 0, 0);
            else if ((HWND)lParam == _hLblURL)
                ShellExecute(NULL, L"open", L"http://www.iconarchive.com/show/oxygen-icons-by-oxygen-icons.org.html", NULL, NULL, SW_SHOWDEFAULT);
            return FALSE;
        }

        case WM_CLOSE:
        {
            ShowWindow(_hWnd, SW_HIDE);
            if (_hWndParent)
            {
                PostMessage(_hWndParent, WM_DEWMSG_CHILD_CLOSED, 0, (LPARAM)_hWnd);
                SetForegroundWindow(_hWndParent);
            }
            _bVisible = FALSE;
            return FALSE;
        }
    }

    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void THEMERABOUTWINDOW::_HandleSizing()
{
    RECT        rctClient;
    int         iX, iY, iW, iH;

    GetClientRect(_hWnd, &rctClient);

    iX = rctClient.right / 4; iY = _F(140); iW = _F(90); iH = _F(20);
    SetWindowPos(_hLblCredit, 0, iX, iY, iW, iH, SWP_NOZORDER);

    iX = rctClient.right / 4 + _F(90); iY = _F(140); iW = rctClient.right - _F(10) - iX; iH = _F(20);
    SetWindowPos(_hLblURL, 0, iX, iY, iW, iH, SWP_NOZORDER);

    iW = _F(75); iH = _F(25); iX = rctClient.right - _F(10) - iW; iY = rctClient.bottom - _F(10) - iH;
    SetWindowPos(_hCmdOK, 0, iX, iY, iW, iH, SWP_NOZORDER);

    if (!_bDiscoReady)
    {
        _rctDisco.left = _F(10); _rctDisco.top = rctClient.bottom - _F(35);
        _rctDisco.right = rctClient.right - _F(105); _rctDisco.bottom = _rctDisco.top + _F(20);
        _hDCMem = CreateCompatibleDC(_hDC);
        _hBmpDisco = CreateCompatibleBitmap(_hDC, _rctDisco.right - _rctDisco.left, _rctDisco.bottom - _rctDisco.top);
        SelectObject(_hDCMem, _hBmpDisco);
        SelectObject(_hDCMem, _hFntUI);
        _bDiscoReady = TRUE;
        _hTrdDisco = CreateThread(NULL, 0, _TrdDisco, this, 0, NULL);
    }
    return;
}

void THEMERABOUTWINDOW::_GetAboutInfo()
{
    wchar_t             wsProcessImgFile[MAX_CHAR_PATH] { 0 };
    DWORD               dwVISize = 0, dwStub = 0;
    UINT                nFFInfoSize = 0;
    LPBYTE              lpVI;
    VS_FIXEDFILEINFO    *pffInfo = NULL;
    WORD                wMajor = 0, wMinor = 0;

    ZeroMemory(_wsAboutInfo, MAX_CHAR_PATH * sizeof(wchar_t));


    GetModuleFileName(NULL, wsProcessImgFile, MAX_CHAR_PATH);
    dwVISize = GetFileVersionInfoSize(wsProcessImgFile, &dwStub);
    if (dwVISize > 0)
    {
        lpVI = (LPBYTE)LocalAlloc(LPTR, dwVISize);
        if (GetFileVersionInfo(wsProcessImgFile, 0, dwVISize, lpVI))
        {
            if (VerQueryValue(lpVI, L"\\", (LPVOID*)&pffInfo, &nFFInfoSize))
            {
                wMajor = HIWORD((pffInfo->dwFileVersionMS));
                wMinor = LOWORD((pffInfo->dwFileVersionMS));
            }
        }
        LocalFree(lpVI);
    }

    wsprintf(_wsAboutInfo,
             L"Dew Drop Themer\n" \
             L"(A Theming Tool for the Dew Drop Player)\n\n" \
             L"Version: %u.%u\n" \
             L"Author: Polash Majumdar\n" \
             L"Release: August, 2020\n" \
             L"Email: Polash.Majumdar@gmail.com\n",
             wMajor, wMinor);
    return;
}

void THEMERABOUTWINDOW::_SetFonts()
{
    LOGFONT     lgfFont { 0 };

    if (_hFntUI) { DeleteObject(_hFntUI); _hFntUI = NULL; }
    if (_hFntAbout) { DeleteObject(_hFntAbout); _hFntAbout = NULL; }

    lgfFont.lfWidth = 0;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;

    lgfFont.lfWeight = FW_NORMAL;
    lgfFont.lfHeight = -MulDiv(8, _iDPI, 72);
    CopyMemory(lgfFont.lfFaceName, L"Tahoma", 32 * sizeof(wchar_t));
    _hFntUI = CreateFontIndirect(&lgfFont);

    lgfFont.lfWeight = FW_BOLD;
    lgfFont.lfHeight = -MulDiv(9, _iDPI, 72);
    CopyMemory(lgfFont.lfFaceName, L"Tahoma", 32 * sizeof(wchar_t));
    _hFntAbout = CreateFontIndirect(&lgfFont);

    SendMessage(_hLblCredit, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));
    SendMessage(_hLblURL, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));
    SendMessage(_hCmdOK, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));

    return;
}

void THEMERABOUTWINDOW::_Draw()
{
    RECT        rctClient;
    RECT        rctText;

    GetClientRect(_hWnd, &rctClient);
    FillRect(_hDC, &rctClient, _hbrBack);

    rctText.left = rctClient.right / 4;
    rctText.top = _F(10);
    rctText.right = rctClient.right - _F(10);
    rctText.bottom = _F(120);

    SetBkMode(_hDC, TRANSPARENT);

    SetTextColor(_hDC, _crShadow);
    OffsetRect(&rctText, _F(1), _F(1));
    DrawText(_hDC, _wsAboutInfo, -1, &rctText, DT_CENTER);

    SetTextColor(_hDC, _crText);
    OffsetRect(&rctText, -_F(1), -_F(1));
    DrawText(_hDC, _wsAboutInfo, -1, &rctText, DT_CENTER);

    DrawIconEx(_hDC, _F(10), (rctClient.bottom - _F(45) - _F(64))/2, _hIcoAbout, _F(64), _F(64), 0, NULL, DI_NORMAL);
    return;
}

void THEMERABOUTWINDOW::_DrawButton(const LPDRAWITEMSTRUCT& lpDIS, LPCWSTR wsText)
{
    RECT                rctText;
    HGDIOBJ             hObjOld;

    hObjOld = SelectObject(lpDIS->hDC, _hbrBack);
    SelectObject(lpDIS->hDC, _hFntUI);
    SelectObject(lpDIS->hDC, _hPenBorder);

    SetBkMode(lpDIS->hDC, TRANSPARENT);
    SetTextColor(lpDIS->hDC, _crText);
    Rectangle(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom);

    if ((lpDIS->itemState & ODS_FOCUS) == ODS_FOCUS)
    {
        CopyMemory(&rctText, &(lpDIS->rcItem), sizeof(RECT));
        InflateRect(&rctText, -_F(2), -_F(2));
        SelectObject(lpDIS->hDC, _hPenFocus);
        Rectangle(lpDIS->hDC, rctText.left, rctText.top, rctText.right, rctText.bottom);
    }

    CopyMemory(&rctText, &(lpDIS->rcItem), sizeof(RECT));
    if ((lpDIS->itemState & ODS_SELECTED) == ODS_SELECTED)
        OffsetRect(&rctText, _F(1), _F(1));
    DrawText(lpDIS->hDC, wsText, -1, &rctText, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    SelectObject(lpDIS->hDC, hObjOld);

    return;
}


THEMERABOUTWINDOW::THEMERABOUTWINDOW(HWND hWndParent, int iDPI, float fScale)
{
    WNDCLASSEX      wcex { 0 };

    const wchar_t   *wsClass = L"DEWDROP.THEMER.ABOUT.CLASS";
    const DWORD     dwWinStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN;
    const DWORD     dwLblStyle = WS_CHILD | WS_VISIBLE;
    const DWORD     dwCmdStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW;

    _hInstance = GetModuleHandle(NULL);
    _hWndParent = hWndParent;
    _iDPI = iDPI;
    _fScale = fScale;
    _crShadow = RGB(0x00, 0x00, 0x00);
    _crText = RGB(0xB0, 0xB0, 0xB0);
    _crBack = RGB(0x40, 0x40, 0x40);
    _crURL = RGB(0x00, 0xFF, 0xFF);
    _hPenBorder = CreatePen(PS_SOLID, _F(1), _crText);
    _hPenFocus = CreatePen(PS_DOT, _F(1), _crText);
    _hbrBack = CreateSolidBrush(_crBack);
    _hIcoAbout = (HICON)LoadImage(_hInstance, MAKEINTRESOURCE(IDI_APP_THEMER), IMAGE_ICON, _F(64), _F(64), LR_DEFAULTCOLOR);

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = DefWindowProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = _hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = _hbrBack;
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = wsClass;

    RegisterClassEx(&wcex);

    _hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, wsClass, L"About Dew Drop Themer", dwWinStyle, 0, 0, 0, 0, _hWndParent, NULL, _hInstance, NULL);
    _hLblCredit = CreateWindowEx(0, L"STATIC", L"Icon Courtesy: ", dwLblStyle | SS_RIGHT, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hLblURL = CreateWindowEx(0, L"STATIC", L"Oxygen Icons (by Oxygen Team)", dwLblStyle | SS_NOTIFY, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);
    _hCmdOK = CreateWindowEx(0, L"BUTTON", L"&OK", dwCmdStyle, 0, 0, 0, 0, _hWnd, NULL, _hInstance, NULL);

    _hDC = GetDC(_hWnd);

    SetClassLongPtr(_hLblURL, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));

    this->_SetFonts();
    this->_GetAboutInfo();
    _hObjOld = SelectObject(_hDC, _hFntAbout);

    SetWindowSubclass(_hWnd, _AboutMsgHandler, (UINT_PTR)_hWnd, (DWORD_PTR)this);
    return;
}

HWND THEMERABOUTWINDOW::Show()
{
    int     iWidth, iHeight, iScreenWidth, iScreenHeight, iXPos, iYPos;
    RECT    rctParent;

    GetWindowRect(_hWndParent, &rctParent);
    iScreenWidth = rctParent.right - rctParent.left;
    iScreenHeight = rctParent.bottom - rctParent.top;

    iWidth = _F(384);
    iHeight = _F(240);
    iXPos = (iScreenWidth - iWidth) / 2 + rctParent.left;
    iYPos = (iScreenHeight - iHeight) / 2 + rctParent.top;

    if (iXPos < 0) iXPos = 0;
    if (iYPos < 0) iYPos = 0;

    SetWindowPos(_hWnd, 0, iXPos, iYPos, iWidth, iHeight, SWP_NOZORDER);

    ShowWindow(_hWnd, SW_SHOW);
    UpdateWindow(_hWnd);
    SetActiveWindow(_hWnd);
    if (_hWndParent) EnableWindow(_hWndParent, FALSE);
    _bVisible = TRUE;

    return _hWnd;
}

THEMERABOUTWINDOW::~THEMERABOUTWINDOW()
{
    if (_hTrdDisco) { TerminateThread(_hTrdDisco, 0x00); _hTrdDisco = NULL; }

    RemoveWindowSubclass(_hWnd, _AboutMsgHandler, (UINT_PTR)_hWnd);
    SelectObject(_hDC, _hObjOld);
    ReleaseDC(_hWnd, _hDC);

    DeleteDC(_hDCMem);
    DeleteObject(_hBmpDisco);

    DestroyWindow(_hLblCredit);
    DestroyWindow(_hLblURL);
    DestroyWindow(_hCmdOK);

    DestroyIcon(_hIcoAbout);

    DeleteObject(_hFntUI);
    DeleteObject(_hFntAbout);
    DeleteObject(_hPenBorder);
    DeleteObject(_hPenFocus);
    DeleteObject(_hbrBack);

    DestroyWindow(_hWnd);
    return;
}
