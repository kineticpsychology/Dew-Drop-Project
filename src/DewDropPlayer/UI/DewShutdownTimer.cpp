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

#include "DewShutdownTimer.h"

LRESULT CALLBACK DEWSHUTDOWNTIMER::_TimerMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    return ((LPDEWSHUTDOWNTIMER)dwRefData)->_WndProc(hWnd, nMsg, wParam, lParam);
}

DWORD WINAPI DEWSHUTDOWNTIMER::_TrdCountdown(LPVOID lpv)
{
    ((LPDEWSHUTDOWNTIMER)lpv)->_Countdown();
    ((LPDEWSHUTDOWNTIMER)lpv)->_hTrdCountdown = NULL;
    SendMessage(((LPDEWSHUTDOWNTIMER)lpv)->_hWndTimer, WM_DEWMSG_COUNTDOWN_OVER, 0, 0);
    return 0;
}

void DEWSHUTDOWNTIMER::_DrawButton(const LPDRAWITEMSTRUCT& lpDIS, LPCWSTR wsCaption, LPCWSTR wsIcon)
{
    RECT                rctText, rctIcon;
    HGDIOBJ             hObjOld, hOblOldIcon;
    HDC                 hDCMem;
    HBITMAP             hBmpIcon;

    // Current DC Settings
    SetBkMode(lpDIS->hDC, TRANSPARENT);
    SetTextColor(lpDIS->hDC, _crBtnText);

    // Mem DC (Icon) Settings
    rctIcon.left = 0;
    rctIcon.top = 0;
    rctIcon.right = lpDIS->rcItem.bottom - _F(7);
    rctIcon.bottom = lpDIS->rcItem.bottom - _F(7);
    hDCMem = CreateCompatibleDC(lpDIS->hDC);
    hBmpIcon = CreateCompatibleBitmap(lpDIS->hDC, rctIcon.right, rctIcon.bottom);
    hOblOldIcon = SelectObject(hDCMem, hBmpIcon);
    SelectObject(hDCMem, _hFntIcon);
    FillRect(hDCMem, &rctIcon, _hbrBtnBack);
    SetTextColor(hDCMem, _crBtnText);
    SetBkMode(hDCMem, TRANSPARENT);
    DrawText(hDCMem, wsIcon, -1, &rctIcon, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    hObjOld = SelectObject(lpDIS->hDC, _hbrBtnBack);
    SelectObject(lpDIS->hDC, _hFntUI);

    SelectObject(lpDIS->hDC, _hPenBoundary);
    Rectangle(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom);

    if ((lpDIS->itemState & ODS_FOCUS) == ODS_FOCUS)
    {
        CopyMemory(&rctText, &(lpDIS->rcItem), sizeof(RECT));
        InflateRect(&rctText, -_F(2), -_F(2));
        SelectObject(lpDIS->hDC, _hPenFocus);
        Rectangle(lpDIS->hDC, rctText.left, rctText.top, rctText.right, rctText.bottom);
    }

    if ((lpDIS->itemState & ODS_SELECTED) == ODS_SELECTED)
        BitBlt(lpDIS->hDC, _F(4), _F(4), rctIcon.right + _F(1), rctIcon.bottom + _F(1), hDCMem, 0, 0, SRCCOPY);
    else
        BitBlt(lpDIS->hDC, _F(3), _F(3), rctIcon.right, rctIcon.bottom, hDCMem, 0, 0, SRCCOPY);
    SelectObject(hDCMem, hOblOldIcon);
    DeleteObject(hBmpIcon);
    DeleteDC(hDCMem);

    CopyMemory(&rctText, &(lpDIS->rcItem), sizeof(RECT));
    rctText.left = _F(30);
    if ((lpDIS->itemState & ODS_SELECTED) == ODS_SELECTED)
        OffsetRect(&rctText, _F(1), _F(1));
    DrawText(lpDIS->hDC, wsCaption, -1, &rctText, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    SelectObject(lpDIS->hDC, hObjOld);

    return;
}

void DEWSHUTDOWNTIMER::_ResetObjects()
{
    if (_pPenCountdown) { delete _pPenCountdown; _pPenCountdown = NULL; }
    if (_pPenWinBorder) { delete _pPenWinBorder; _pPenWinBorder = NULL; }
    if (_pBrText) { delete _pBrText; _pBrText = NULL; }
    if (_pBrWinBack) { delete _pBrWinBack; _pBrWinBack = NULL; }
    if (_pFont) { delete _pFont; _pFont = NULL; }
    if (_pFontCountdown) { delete _pFontCountdown; _pFontCountdown = NULL; }
    if (_hFntUI) { DeleteObject(_hFntUI); _hFntUI = NULL; }
    if (_hFntIcon) { DeleteObject(_hFntIcon); _hFntIcon = NULL; }
    if (_hbrBtnBack) { DeleteObject(_hbrBtnBack); _hbrBtnBack = NULL; }
    if (_hPenBoundary) { DeleteObject(_hPenBoundary); _hPenBoundary = NULL; }
    if (_hPenFocus) { DeleteObject(_hPenFocus); _hPenFocus = NULL; }

    return;
}

LRESULT DEWSHUTDOWNTIMER::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT     ps { 0 };
    RECT            rctWorkArea;
    LPWINDOWPOS     lpPos;

    switch (nMsg)
    {
        case WM_DEWMSG_UPDOWN_VALUE_CHANGED:
        {
            this->_Draw();
            if (_lpUpDownHours->Value || _lpUpDownMinutes->Value || _lpUpDownSeconds->Value)
                ShowWindow(_hCmdStart, SW_SHOW);
            else
                ShowWindow(_hCmdStart, SW_HIDE);
            return FALSE;
        }

        case WM_COMMAND:
        {
            if (    ((HWND)lParam == _hCmdOK) ||
                    ((HWND)lParam == 0x00 && LOWORD(wParam) == IDCANCEL)    )
            {
                SendMessage(_hWndTimer, WM_CLOSE, 0, 0);
                return FALSE;
            }
            else if ((HWND)lParam == _hCmdStart)
            {
                _hTrdCountdown = CreateThread(NULL, 0, _TrdCountdown, this, 0, NULL);
                return FALSE;
            }
            else if ((HWND)lParam == _hCmdAbort)
            {
                this->Abort();
                return FALSE;
            }
            break;
        }

        case WM_ERASEBKGND:
        {
            return NULL;
        }

        case WM_PAINT:
        {
            BeginPaint(hWnd, &ps);
            this->_Draw();
            EndPaint(hWnd, &ps);
            return FALSE;
        }

        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT    lpDIS;

            lpDIS = (LPDRAWITEMSTRUCT)lParam;
            if (lpDIS->hwndItem == _hCmdOK)
                this->_DrawButton(lpDIS, L"&OK", DEWUI_SYMBOL_BUTTON_OK);
            else if (lpDIS->hwndItem == _hCmdStart)
                this->_DrawButton(lpDIS, L"&Start", DEWUI_SYMBOL_BUTTON_START);
            else if (lpDIS->hwndItem == _hCmdAbort)
                this->_DrawButton(lpDIS, L"&Abort", DEWUI_SYMBOL_BUTTON_CANCEL);
            else
                break;
            return TRUE;
        }

        case WM_SIZE:
        {
            this->_HandleSizing();
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

        case WM_CLOSE:
        {
            _bVisible = FALSE;
            ShowWindow(_hWndTimer, SW_HIDE);
            PostMessage(_hWndParent, WM_DEWMSG_CHILD_CLOSED, 0, 0);
            SetForegroundWindow(_hWndParent);
            return FALSE;
        }
    }

    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWSHUTDOWNTIMER::_HandleSizing()
{
    RECT        rctClient;
    int         iX, iY, iW, iH, iSpacing;

    GetClientRect(_hWndTimer, &rctClient);

    iW = _iUpDownWidth; iH = _iUpDownHeight;
    iSpacing = ((rctClient.right - rctClient.left) - (iW * 3))/4;
    iY = (((rctClient.bottom - rctClient.top) - _F(30)) - iH)/2;

    iX = rctClient.left + iSpacing;
    _lpUpDownHours->Move(iX, iY);

    iX += (iW + iSpacing);
    _lpUpDownMinutes->Move(iX, iY);

    iX += (iW + iSpacing);
    _lpUpDownSeconds->Move(iX, iY);

    iW = _F(75); iH = _F(30); iY = rctClient.bottom - _F(40);

    iX = rctClient.right - _F(85);
    SetWindowPos(_hCmdOK, 0, iX, iY, iW, iH, SWP_NOZORDER);

    iX = _F(10); // Cancel and Abort buttons are exactly at the same spot
    SetWindowPos(_hCmdStart, 0, iX, iY, iW, iH, SWP_NOZORDER);
    SetWindowPos(_hCmdAbort, 0, iX, iY, iW, iH, SWP_NOZORDER);

    return;
}

void DEWSHUTDOWNTIMER::_Draw()
{
    if (_bVisible)
    {
        Graphics    *pGr;
        wchar_t     wsMessage[128];
        RECT        rctClient;

        GetClientRect(_hWndTimer, &rctClient);
        pGr = Graphics::FromHDC(_hDC);
        pGr->FillRectangle(_pBrWinBack, 0, 0, rctClient.right, rctClient.bottom);
        pGr->DrawRectangle(_pPenWinBorder, 0, 0, rctClient.right - _F(1), rctClient.bottom - _F(1));

        // Draw the Info texts
        if (!_bCountdownMode)
        {
            if (_lpUpDownHours->Value || _lpUpDownMinutes->Value || _lpUpDownSeconds->Value)
                StringCchPrintf(wsMessage, 128, L"Schedule auto-shutdown in %u hours, %u minutes, %u seconds",
                                _lpUpDownHours->Value, _lpUpDownMinutes->Value, _lpUpDownSeconds->Value);
            else
                StringCchPrintf(wsMessage, 128, L"Select a preset time for auto-shutdown (less than 6 hours)");
            pGr->DrawString(wsMessage, -1, _pFont, PointF((REAL)_F(10), (REAL)_F(10)), _pBrText);
        }
    }
    return;
}

void DEWSHUTDOWNTIMER::_Countdown()
{
    LARGE_INTEGER   liFreq, liStart, liNow;
    double          fDuration;
    DWORD           dwCountdownSeconds, dwElapsedSeconds = 1, dwLeftSeconds;
    INT             iImgWidth, iImgHeight, iSize, iImgOffsetX, iImgOffsetY;
    wchar_t         wsTime[64];
    StringFormat    sfDefault;
    RectF           rctTime;
    RECT            rctClient;
    Graphics        *pGr = NULL;
    REAL            fSweepAngle = 0.0f;
    UINT            nHour;

    _bCountdownMode = TRUE;
    dwCountdownSeconds = (_lpUpDownHours->Value * 60 * 60) +
                         (_lpUpDownMinutes->Value * 60) +
                         (_lpUpDownSeconds->Value);

    GetClientRect(_hWndTimer, &rctClient);
    iImgWidth = (rctClient.right - rctClient.left) - _F(2);
    iImgHeight = (rctClient.bottom - rctClient.top) - _F(2);
    iSize = _F(_CIRCLE_DIAMETER);
    iImgOffsetX = (iImgWidth - iSize) / 2;
    iImgOffsetY = (iImgHeight - iSize) / 2;
    sfDefault.SetAlignment(StringAlignmentCenter);
    sfDefault.SetLineAlignment(StringAlignmentCenter);

    rctTime.X = 0; rctTime.Y = 0;
    rctTime.Width = (REAL)iImgWidth; rctTime.Height = (REAL)iImgHeight;
    ShowWindow(_hCmdAbort, SW_SHOW);
    ShowWindow(_hCmdStart, SW_HIDE);
    _lpUpDownHours->SetVisible(FALSE);
    _lpUpDownMinutes->SetVisible(FALSE);
    _lpUpDownSeconds->SetVisible(FALSE);

    dwElapsedSeconds = 0;
    QueryPerformanceFrequency(&liFreq);
    QueryPerformanceCounter(&liStart);
    while (dwElapsedSeconds < dwCountdownSeconds)
    {
        QueryPerformanceCounter(&liNow);
        fDuration = (((double)(liNow.QuadPart)) - ((double)(liStart.QuadPart)))/((double)(liFreq.QuadPart));
        dwElapsedSeconds = (DWORD)fDuration;
        dwLeftSeconds = dwCountdownSeconds - dwElapsedSeconds;

        // Do the drawing illustration only if the screen in visible
        if (_bVisible)
        {
            pGr = Graphics::FromHDC(_hDCMem);
            pGr->SetSmoothingMode(SmoothingModeAntiAlias);
            pGr->FillRectangle(_pBrWinBack, 0, 0, rctClient.right, rctClient.bottom);
            nHour = dwLeftSeconds / (60 * 60);
            if (nHour)
                StringCchPrintf(wsTime, 64, L"%02u:%02u:%02u", nHour, (dwLeftSeconds - (nHour * 3600)) / 60, dwLeftSeconds % 60);
            else
                StringCchPrintf(wsTime, 64, L"%02u : %02u", dwLeftSeconds/60, dwLeftSeconds%60);
            fSweepAngle = (REAL)(((((REAL)dwCountdownSeconds) - fDuration) * 360.0f) / ((REAL)dwCountdownSeconds));
            pGr->DrawArc(_pPenCountdown, iImgOffsetX + _F(_COUNTDOWN_BRUSH_THICKNESS), iImgOffsetY + _F(_COUNTDOWN_BRUSH_THICKNESS),
                         iSize - _F(_COUNTDOWN_BRUSH_THICKNESS) * 2, iSize - _F(_COUNTDOWN_BRUSH_THICKNESS) * 2, 90, fSweepAngle);
            pGr->DrawString(wsTime, -1, _pFontCountdown, rctTime, &sfDefault, _pBrText);
            delete pGr;
            if (dwElapsedSeconds < dwCountdownSeconds)
                BitBlt(_hDC, _F(1), _F(1), iImgWidth, iImgHeight, _hDCMem, 0, 0, SRCCOPY);
        }
        Sleep(_POLL_FREQ); // Keep Polling
    }

    _bCountdownMode = FALSE;

    // No need to restore visibility of anything. We're shutting down anyways
    SendMessage(_hWndTimer, WM_CLOSE, 0, 0);
    PostMessage(_hWndParent, WM_DEWMSG_COUNTDOWN_OVER, 0, 0);

    _hTrdCountdown = NULL;
    return;
}

DEWSHUTDOWNTIMER::DEWSHUTDOWNTIMER(HWND hWndParent, int iDPI, float fScale) :
Handle (_hWndTimer), CountdownMode(_bCountdownMode)
{
    int             iWidth, iHeight, iPrimaryMult;
    const wchar_t   *wsClass = L"DEWDROP.TIMER.WND";
    const DWORD     dwBtnStyle = WS_CHILD | WS_TABSTOP | BS_OWNERDRAW;
    WNDCLASSEX      wcex { 0 };
    RECT            rctClient;

    _hInstance = GetModuleHandle(NULL);
    _iDPI = iDPI;
    _fScale = fScale;
    _hWndParent = hWndParent;

    iWidth = _F(_WINWIDTH);
    iHeight = _F(_WINHEIGHT);

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = 0;
    wcex.lpfnWndProc    = DefWindowProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = _hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = wsClass;

    RegisterClassEx(&wcex);

    _hWndTimer = CreateWindowEx(WS_EX_TOOLWINDOW, wsClass, L"Dew Drop Player - Shutdown Timer",
                                WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN, 0, 0, iWidth, iHeight,
                                _hWndParent, NULL, _hInstance, NULL);

    _iUpDownWidth = _F(60);
    _iUpDownHeight = _F(90);
    iPrimaryMult = 3;

    _lpUpDownHours = new DEWUPDOWNCONTROL(_hWndTimer, _iUpDownWidth, _iUpDownHeight, iPrimaryMult, 0, 5);
    _lpUpDownMinutes = new DEWUPDOWNCONTROL(_hWndTimer, _iUpDownWidth, _iUpDownHeight, iPrimaryMult, 0, 59);
    _lpUpDownSeconds = new DEWUPDOWNCONTROL(_hWndTimer, _iUpDownWidth, _iUpDownHeight, iPrimaryMult, 0, 59);

    _hCmdStart = CreateWindowEx(0, L"BUTTON", L"&Start", dwBtnStyle,
                                 0, 0, 0, 0, _hWndTimer, NULL, _hInstance, NULL);
    _hCmdAbort = CreateWindowEx(0, L"BUTTON", L"&Abort", dwBtnStyle,
                                0, 0, 0, 0, _hWndTimer, NULL, _hInstance, NULL);
    _hCmdOK = CreateWindowEx(0, L"BUTTON", L"&OK", dwBtnStyle,
                             0, 0, 0, 0, _hWndTimer, NULL, _hInstance, NULL);
    GetClientRect(_hWndTimer, &rctClient);
    _hDC = GetDC(_hWndTimer);
    _hDCMem = CreateCompatibleDC(_hDC);
    _hBmpCountdown = CreateCompatibleBitmap(_hDC, (rctClient.right - rctClient.left), (rctClient.bottom - rctClient.top));
    SelectObject(_hDCMem, _hBmpCountdown);

    _lpUpDownHours->SetVisible();
    _lpUpDownMinutes->SetVisible();
    _lpUpDownSeconds->SetVisible();
    ShowWindow(_hCmdOK, SW_SHOW);

    SetWindowSubclass(_hWndTimer, _TimerMsgHandler, (UINT_PTR)_hWndTimer, (DWORD_PTR)this);
    return;
}

void DEWSHUTDOWNTIMER::ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw)
{
    Color       crRef;
    LOGFONT     lgfFont { 0 };
    FontStyle   gdiFontStyle;

    this->_ResetObjects();

    crRef.SetFromCOLORREF(Theme.ModuleStyle.OutlineColor);
    _pPenCountdown = new Pen(crRef, (float)_COUNTDOWN_BRUSH_THICKNESS * _fScale);
    _pPenCountdown->SetStartCap(LineCapRound);
    _pPenCountdown->SetEndCap(LineCapRound);

    crRef.SetFromCOLORREF(Theme.WinStyle.OutlineColor);
    _pPenWinBorder = new Pen(crRef, _fScale);

    crRef.SetFromCOLORREF(Theme.WinStyle.TextColor);
    _pBrText = new SolidBrush(crRef);

    crRef.SetFromCOLORREF(Theme.WinStyle.BackColor);
    _pBrWinBack = new SolidBrush(crRef);

    _crBtnText = Theme.MMButtonStyle.TextColor;

    _hbrBtnBack = CreateSolidBrush(Theme.MMButtonStyle.BackColor);
    _hPenBoundary = CreatePen(PS_SOLID, _F(1), Theme.MMButtonStyle.OutlineColor);
    _hPenFocus = CreatePen(PS_DOT, _F(1), Theme.MMButtonStyle.OutlineColor);

    lgfFont.lfHeight = -MulDiv(Theme.TextFontStyle.FontSize, _iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = FW_NORMAL;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, Theme.TextFontStyle.FontName, 32 * sizeof(wchar_t));
    _hFntUI = CreateFontIndirect(&lgfFont);

    lgfFont.lfHeight = -MulDiv(DEWUI_DIM_SYM_BUTTON_FONT, _iDPI, 72);
    CopyMemory(lgfFont.lfFaceName, DEWUI_SYMBOL_FONT_NAME, 32 * sizeof(wchar_t));
    _hFntIcon = CreateFontIndirect(&lgfFont);

    if (Theme.TextFontStyle.IsBold)
        gdiFontStyle = Theme.TextFontStyle.IsItalic ? FontStyleBoldItalic : FontStyleBold;
    else
        gdiFontStyle = Theme.TextFontStyle.IsItalic ? FontStyleItalic : FontStyleRegular;
    _pFont = new Font(Theme.TextFontStyle.FontName, (REAL)Theme.TextFontStyle.FontSize, gdiFontStyle, UnitPoint);
    _pFontCountdown = new Font(Theme.TextFontStyle.FontName, (REAL)(Theme.TextFontStyle.FontSize + 5), gdiFontStyle, UnitPoint);

    SendMessage(_hCmdStart, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));
    SendMessage(_hCmdAbort, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));
    SendMessage(_hCmdOK, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));

    _lpUpDownHours->SetFont(Theme.TextFontStyle.FontName, Theme.TextFontStyle.FontSize + 4, Theme.TextFontStyle.FontSize, bForceDraw);
    _lpUpDownMinutes->SetFont(Theme.TextFontStyle.FontName, Theme.TextFontStyle.FontSize + 4, Theme.TextFontStyle.FontSize, bForceDraw);
    _lpUpDownSeconds->SetFont(Theme.TextFontStyle.FontName, Theme.TextFontStyle.FontSize + 4, Theme.TextFontStyle.FontSize, bForceDraw);

    _lpUpDownHours->ApplyTheme(Theme);
    _lpUpDownMinutes->ApplyTheme(Theme);
    _lpUpDownSeconds->ApplyTheme(Theme);

    if (bForceDraw)
        this->_Draw();
    return;
}

HWND DEWSHUTDOWNTIMER::Show(const DEWTHEME& Theme)
{
    int     iScreenWidth, iScreenHeight, iXPos, iYPos;
    RECT    rctParent, rctWorkArea;

    GetWindowRect(_hWndParent, &rctParent);
    iScreenWidth = rctParent.right - rctParent.left;
    iScreenHeight = rctParent.bottom - rctParent.top;
    iXPos = (iScreenWidth - _F(_WINWIDTH))/2 + rctParent.left;
    iYPos = (iScreenHeight - _F(_WINHEIGHT))/2 + rctParent.top;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);
    if (iXPos < rctWorkArea.left) iXPos = rctWorkArea.left;
    if (iYPos < rctWorkArea.top) iYPos = rctWorkArea.top;
    if ((iXPos + _F(_WINWIDTH)) > rctWorkArea.right) iXPos = rctWorkArea.right - _F(_WINWIDTH);
    if ((iYPos + _F(_WINHEIGHT)) > rctWorkArea.bottom) iYPos = rctWorkArea.bottom - _F(_WINHEIGHT);

    this->ApplyTheme(Theme, FALSE);

    _bVisible = TRUE;
    EnableWindow(_hWndParent, FALSE);
    SetWindowPos(_hWndTimer, 0, iXPos, iYPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    this->_Draw();
    ShowWindow(_hWndTimer, SW_SHOW);
    UpdateWindow(_hWndTimer);
    SetActiveWindow(_hWndTimer);
    return _hWndTimer;
}

void DEWSHUTDOWNTIMER::Abort()
{
    if (!_bCountdownMode) return;

    TerminateThread(_hTrdCountdown, 0x00);
    _bCountdownMode = FALSE;

    _lpUpDownHours->SetValue(0x00);
    _lpUpDownMinutes->SetValue(0x00);
    _lpUpDownSeconds->SetValue(0x00);

    _lpUpDownHours->SetVisible();
    _lpUpDownMinutes->SetVisible();
    _lpUpDownSeconds->SetVisible();

    ShowWindow(_hCmdAbort, SW_HIDE);
    // 'Start' button will again be hidden because all values are set to zero now
    ShowWindow(_hCmdStart, SW_HIDE);

    InvalidateRect(_hWndTimer, NULL, TRUE);
    return;
}

DEWSHUTDOWNTIMER::~DEWSHUTDOWNTIMER()
{
    this->Abort();
    if (_lpUpDownHours) { delete _lpUpDownHours; _lpUpDownHours = NULL; }
    if (_lpUpDownMinutes) { delete _lpUpDownMinutes; _lpUpDownMinutes = NULL; }
    if (_lpUpDownSeconds) { delete _lpUpDownSeconds; _lpUpDownSeconds = NULL; }

    DestroyWindow(_hCmdOK);
    DestroyWindow(_hCmdAbort);
    DestroyWindow(_hCmdStart);

    RemoveWindowSubclass(_hWndTimer, _TimerMsgHandler, (UINT_PTR)_hWndTimer);
    DeleteObject(_hBmpCountdown);
    ReleaseDC(_hWndTimer, _hDC);
    DeleteDC(_hDCMem);
    DestroyWindow(_hWndTimer);
    this->_ResetObjects();
    return;
}
