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

#include "DewUpDownControl.h"

wchar_t DEWUPDOWNCONTROL::_swsDewUpDownCtrlClass[32] { 0 };
BOOL DEWUPDOWNCONTROL::_sbCommonOpsComplete = FALSE;
UINT DEWUPDOWNCONTROL::_snInstanceCount = 0;

LRESULT CALLBACK DEWUPDOWNCONTROL::_UpDownMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    return ((LPDEWUPDOWNCONTROL)dwRefData)->_WndProc(hWnd, nMsg, wParam, lParam);
}

LRESULT DEWUPDOWNCONTROL::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC             hDC = NULL;
    PAINTSTRUCT     ps { 0 };
    int             iYPos;

    switch(nMsg)
    {
        case WM_PAINT:
        {
            hDC = BeginPaint(hWnd, &ps);
            this->_Draw(hDC);
            EndPaint(hWnd, &ps);
            return FALSE;
        }
        case WM_LBUTTONDOWN:
        {
            iYPos = HIWORD(lParam);
            if (iYPos >= 0 && iYPos <= _iSectionHeight)
            {
                _wCurrVal = (_wCurrVal == _wMinVal ? _wMaxVal : (_wCurrVal - 1));
                if (_hWndParent)
                    SendMessage(_hWndParent, WM_DEWMSG_UPDOWN_VALUE_CHANGED, (WPARAM)_wCurrVal, (LPARAM)_hWnd);
                this->_Draw(NULL);
                return FALSE;
            }
            else if (iYPos >= 2 * _iSectionHeight && iYPos <= 3 * _iSectionHeight)
            {
                _wCurrVal = (_wCurrVal == _wMaxVal ? _wMinVal : (_wCurrVal + 1));
                if (_hWndParent)
                    SendMessage(_hWndParent, WM_DEWMSG_UPDOWN_VALUE_CHANGED, (WPARAM)_wCurrVal, (LPARAM)_hWnd);
                this->_Draw(NULL);
                return FALSE;
            }
            break;
        }
        case WM_MOUSEMOVE:
        {
            iYPos = HIWORD(lParam);

            if (    (iYPos >= 0 && iYPos <= _iSectionHeight) ||
                    (iYPos >= 2 * _iSectionHeight && iYPos <= 3 * _iSectionHeight)  )
                SetClassLongPtr(_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));
            else
                SetClassLongPtr(_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
            return FALSE;
        }
    }
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWUPDOWNCONTROL::_Draw(HDC hDC)
{
    HGDIOBJ     hObjOld;
    RECT        rctClient, rctText;
    HDC         hDCCanvas;
    wchar_t     wsValue[32] { 0 };
    WORD        wPrevVal, wNextVal;

    hDCCanvas = (hDC == NULL) ? GetDC(_hWnd) : hDC;

    wPrevVal = (_wCurrVal == _wMinVal ? _wMaxVal : _wCurrVal - 1);
    wNextVal = (_wCurrVal == _wMaxVal ? _wMinVal : _wCurrVal + 1);

    GetClientRect(_hWnd, &rctClient);
    CopyMemory(&rctText, &rctClient, sizeof(RECT));
    SetBkMode(hDCCanvas, TRANSPARENT);
    SetTextColor(hDCCanvas, _crText);
    hObjOld = SelectObject(hDCCanvas, _hPenDiv);
    FillRect(hDCCanvas, &rctClient, _hbrBack);

    // Primary Text
    SelectObject(hDCCanvas, _hFntPrimary);
    rctText.top = _iSectionHeight; rctText.bottom = rctText.top + _iSectionHeight;
    StringCchPrintf(wsValue, 32, L"%u", _wCurrVal);
    DrawText(hDCCanvas, wsValue, -1, &rctText, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    SelectObject(hDCCanvas, _hFntSecondary);

    // Top Secondary
    rctText.top = rctClient.left; rctText.bottom = _iSectionHeight;
    StringCchPrintf(wsValue, 32, L"%u", wPrevVal);
    DrawText(hDCCanvas, wsValue, -1, &rctText, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    // Bottom Secondary
    rctText.top = _iSectionHeight * 2; rctText.bottom = rctText.top + _iSectionHeight;
    StringCchPrintf(wsValue, 32, L"%u", wNextVal);
    DrawText(hDCCanvas, wsValue, -1, &rctText, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    // Divider Lines
    MoveToEx(hDCCanvas, rctClient.left, _iSectionHeight, NULL);
    LineTo(hDCCanvas, rctClient.right, _iSectionHeight);

    MoveToEx(hDCCanvas, rctClient.left, _iSectionHeight * 2, NULL);
    LineTo(hDCCanvas, rctClient.right, _iSectionHeight * 2);

    SelectObject(hDCCanvas, hObjOld);

    if (hDC == NULL) ReleaseDC(_hWnd, hDCCanvas);
    return;
}

DEWUPDOWNCONTROL::DEWUPDOWNCONTROL(HWND hWndParent, int iX, int iY, int iPrimaryMult, WORD wMinVal, WORD wMaxVal) :
Handle(_hWnd), Value(_wCurrVal)
{
    RECT            rctClient { 0 };
    const wchar_t   *wsClass = L"DEW.UPDOWNCTRL.WND";

    _hInstance = GetModuleHandle(NULL);
    _hWndParent = hWndParent;
    _iPrimaryMult = iPrimaryMult;
    _wMinVal = wMinVal;
    _wMaxVal = wMaxVal;
    _wCurrVal = _wMinVal;

    if (!(DEWUPDOWNCONTROL::_sbCommonOpsComplete))
    {
        WNDCLASSEX      wcex { 0 };

        wcex.cbSize         = sizeof(WNDCLASSEX);
        wcex.style          = CS_HREDRAW | CS_VREDRAW;
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
        DEWUPDOWNCONTROL::_sbCommonOpsComplete = TRUE;
    }

    (DEWUPDOWNCONTROL::_snInstanceCount)++;
    _hWnd = CreateWindowEx(0, wsClass, L"", WS_CHILD, 0, 0, iX, iY, _hWndParent, NULL, _hInstance, NULL);
    _hDC = GetDC(_hWnd);

    GetClientRect(_hWnd, &rctClient);
    _iSectionHeight = (rctClient.bottom - rctClient.top) / 3;

    SetWindowSubclass(_hWnd, _UpDownMsgHandler, (UINT_PTR)_hWnd, (DWORD_PTR)this);
    return;
}

void DEWUPDOWNCONTROL::SetValue(WORD wCurrVal)
{
    if (wCurrVal == _wCurrVal) return; // Nothing needs to be refreshed. Bail.

    if (wCurrVal > _wMaxVal) _wCurrVal = _wMaxVal;
    else if (wCurrVal < _wMinVal) _wCurrVal = _wMinVal;
    else _wCurrVal = wCurrVal;
    this->_Draw(NULL);
    return;
}

void DEWUPDOWNCONTROL::ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw)
{
    _crOutline = Theme.ModuleStyle.OutlineColor;
    _crText = Theme.ModuleStyle.TextColor;
    _crBack = Theme.ModuleStyle.BackColor;

    if (_hPenDiv) { DeleteObject(_hPenDiv); _hPenDiv = NULL; }
    if (_hbrBack) { DeleteObject(_hbrBack); _hbrBack = NULL; }

    _hbrBack = CreateSolidBrush(_crBack);
    _hPenDiv = CreatePen(PS_SOLID, _F(1), _crOutline);

    if (bForceDraw) this->_Draw(NULL);
    return;
}

void DEWUPDOWNCONTROL::SetFont(LPCWSTR wsFontName, int iFontSizePrimary, int iFontSizeSecondary, BOOL bForceDraw)
{
    LOGFONT     lgfFont { 0 };

    if (_hFntPrimary) { DeleteObject(_hFntPrimary); _hFntPrimary = NULL; }
    if (_hFntSecondary) { DeleteObject(_hFntSecondary); _hFntSecondary = NULL; }

    lgfFont.lfHeight = -MulDiv(iFontSizePrimary, _iDPI, 72);
    lgfFont.lfWidth = 0;
    lgfFont.lfWeight = FW_BOLD;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;
    CopyMemory(lgfFont.lfFaceName, wsFontName, 32 * sizeof(wchar_t));
    _hFntPrimary = CreateFontIndirect(&lgfFont);

    lgfFont.lfHeight = -MulDiv(iFontSizeSecondary, _iDPI, 72);
    lgfFont.lfWeight = FW_NORMAL;
    _hFntSecondary = CreateFontIndirect(&lgfFont);

    if (bForceDraw) this->_Draw(NULL);
    return;
}

void DEWUPDOWNCONTROL::SetVisible(BOOL bVisible)
{
    ShowWindow(_hWnd, bVisible ? SW_SHOW : SW_HIDE);
    return;
}

void DEWUPDOWNCONTROL::Move(int iXPos, int iYPos)
{
    SetWindowPos(_hWnd, 0, iXPos, iYPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    return;
}

DEWUPDOWNCONTROL::~DEWUPDOWNCONTROL()
{
    (DEWUPDOWNCONTROL::_snInstanceCount)--;
    RemoveWindowSubclass(_hWnd, _UpDownMsgHandler, (UINT_PTR)_hWnd);
    ReleaseDC(_hWnd, _hDC);
    DestroyWindow(_hWnd);
    if((DEWUPDOWNCONTROL::_snInstanceCount) == 0)
        UnregisterClass(DEWUPDOWNCONTROL::_swsDewUpDownCtrlClass, _hInstance);
    if (_hPenDiv) { DeleteObject(_hPenDiv); _hPenDiv = NULL; }
    if (_hbrBack) { DeleteObject(_hbrBack); _hbrBack = NULL; }
    if (_hFntPrimary) { DeleteObject(_hFntPrimary); _hFntPrimary = NULL; }
    if (_hFntSecondary) { DeleteObject(_hFntSecondary); _hFntSecondary = NULL; }

    return;
}
