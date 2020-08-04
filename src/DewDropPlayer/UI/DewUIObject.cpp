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

#include "DewUIObject.h"

int DEWUIOBJECT::_siDPI = 96;
float DEWUIOBJECT::_sfScale = 1.0f;
HINSTANCE DEWUIOBJECT::_shInstance = NULL;
UINT DEWUIOBJECT::_snInstanceCount = 0;
LPDEWSYMBOLFONT DEWUIOBJECT::_slpSymbolFont = NULL;
Bitmap* DEWUIOBJECT::_pImgBackground = NULL;
BOOL DEWUIOBJECT::_sbCommonOpsDone = FALSE;
float DEWUIOBJECT::_sfIconScalingIndex[3] = { 0.75f, 1.00f, 1.25f };

LRESULT CALLBACK DEWUIOBJECT::_MsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam,
                                          LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    LPDEWUIOBJECT       lpUIObject;

    lpUIObject = (LPDEWUIOBJECT)dwRefData;
    if ((HWND)nID == lpUIObject->_hWnd)
        return lpUIObject->_WndProc(hWnd, nMsg, wParam, lParam);
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWUIOBJECT::_InitiateSubclassing()
{
    if (this->_hWnd && this->_hWnd != INVALID_HANDLE_VALUE)
        SetWindowSubclass(_hWnd, DEWUIOBJECT::_MsgHandler, (UINT_PTR)_hWnd, (DWORD_PTR)this);
    return;
}

void DEWUIOBJECT::_CreateTooltip()
{
    _hWndTooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
                                  WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  _hWnd, NULL, NULL, NULL);
    SetWindowPos(_hWndTooltip, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void DEWUIOBJECT::_Scale(BYTE btIconScale)
{
    int     iBaseSize = 0;

    if (_btObjectType == DEWUI_OBJ_TYPE_NONE)
        return;
    if (!(btIconScale == DEWTHEME_ICON_SCALE_SMALL ||
          btIconScale == DEWTHEME_ICON_SCALE_MEDIUM ||
          btIconScale == DEWTHEME_ICON_SCALE_LARGE))
        return;

    _fIconScale = DEWUIOBJECT::_sfIconScalingIndex[btIconScale];

    switch (_btObjectID)
    {
        case DEWUI_OBJ_MIN_BUTTON:
        case DEWUI_OBJ_CLOSE_BUTTON:
            iBaseSize = DEWUI_DIM_SYSBUTTON;
            _iFontSize = 0x3F & DEWUI_FNTSPEC_SYS_BUTTON;
            _iFontSize = (int)((float)_iFontSize * _fIconScale);
            _bBold = ((DEWUI_FNTSPEC_SYS_BUTTON & 0x40) == 0x40);
            break;

        case DEWUI_OBJ_PREV_BUTTON:
        case DEWUI_OBJ_NEXT_BUTTON:
        case DEWUI_OBJ_STOP_BUTTON:
            iBaseSize = DEWUI_DIM_PNS_BUTTON;
            _iFontSize = 0x3F & DEWUI_FNTSPEC_PNS_BUTTON;
            _iFontSize = (int)((float)_iFontSize * _fIconScale);
            _bBold = ((DEWUI_FNTSPEC_PNS_BUTTON & 0x40) == 0x40);
            break;

        case DEWUI_OBJ_PLAY_PAUSE_BUTTON:
            iBaseSize = DEWUI_DIM_PLAY_BUTTON;
            _iFontSize = 0x3F & DEWUI_FNTSPEC_PLAY_BUTTON;
            _iFontSize = (int)((float)_iFontSize * _fIconScale);
            _bBold = ((DEWUI_FNTSPEC_PLAY_BUTTON & 0x40) == 0x40);
            break;

        case DEWUI_OBJ_DM_PLAY_PAUSE_BUTTON:
            _iFontSize = 0x3F & DEWUI_FNTSPEC_PNS_BUTTON;
            _iFontSize = (int)((float)_iFontSize * _fIconScale);
            _bBold = ((DEWUI_FNTSPEC_PNS_BUTTON & 0x40) == 0x40);
            _fDimension = (float)DEWUI_DIM_DM_PLAY_BUTTON * _fScale;
            _iWidth = (int)_fDimension;
            _iHeight = (int)_fDimension;
            return;

        case DEWUI_OBJ_ALBUM_ART:
            iBaseSize = DEWUI_DIM_ALBUM_ART;
            _iFontSize = 0; // N/A
            _bBold = FALSE; // N/A
            break;

        default:
            /*
            // Covers:
            DEWUI_OBJ_SETTINGS_BUTTON
            DEWUI_OBJ_DEWMODE_BUTTON
            DEWUI_OBJ_BROWSE_BUTTON
            DEWUI_OBJ_VOLUME_BUTON
            DEWUI_OBJ_REPEAT_BUTTON
            DEWUI_OBJ_EXTAG_BUTTON
            DEWUI_OBJ_PLAYLIST_BUTTON
            // Not listed here? Will be overridden in the child class (e.g. InfoLabel)
            */
            iBaseSize = DEWUI_DIM_BUTTON;
            _iFontSize = 0x3F & DEWUI_FNTSPEC_BUTTON;
            _iFontSize = (int)((float)_iFontSize * _fIconScale);
            _bBold = ((DEWUI_FNTSPEC_BUTTON & 0x40) == 0x40);
            break;
    }
    _fDimension = iBaseSize * _fScale * _fIconScale;
    _iWidth = (int)_fDimension;
    _iHeight = (int)_fDimension;
    return;
}

DEWUIOBJECT::DEWUIOBJECT() : _lpSymbolFont(DEWUIOBJECT::_slpSymbolFont),
Text(_wsText), Handle(_hWnd), ObjectType(_btObjectType), ObjectID(_btObjectID),
Width(_iWidth), Height(_iHeight), Position(_ptPos)
{
    // The DPI, Scaling and hInstance will remain common across the application
    // consistently throughout the application lifespan.
    // No need to unnecessarily calculate these with every instance
    // Once calculated, keep re-using the value from the static variables
    if (!(DEWUIOBJECT::_sbCommonOpsDone))
    {
        HDC         hDCDesk = NULL;

        hDCDesk = GetDC(HWND_DESKTOP);
        DEWUIOBJECT::_siDPI = GetDeviceCaps(hDCDesk, LOGPIXELSY);
        DEWUIOBJECT::_fScale = (float)(DEWUIOBJECT::_siDPI)/96.0f;
        DEWUIOBJECT::_shInstance = GetModuleHandle(NULL);
        DEWUIOBJECT::_sfIconScalingIndex[DEWTHEME_ICON_SCALE_SMALL] = 0.75f;
        DEWUIOBJECT::_sfIconScalingIndex[DEWTHEME_ICON_SCALE_MEDIUM] = 1.00f;
        DEWUIOBJECT::_sfIconScalingIndex[DEWTHEME_ICON_SCALE_LARGE] = 1.25f;
        ReleaseDC(HWND_DESKTOP, hDCDesk);
        hDCDesk = NULL;
        DEWUIOBJECT::_sbCommonOpsDone = TRUE;

        DEWUIOBJECT::_slpSymbolFont = new DEWSYMBOLFONT();
    }

    this->_iDPI = DEWUIOBJECT::_siDPI;
    this->_fScale = DEWUIOBJECT::_sfScale;
    this->_hInstance = DEWUIOBJECT::_shInstance;

    DEWUIOBJECT::_snInstanceCount++;

    ZeroMemory(_wsFontName, 32 * sizeof(wchar_t));
    ZeroMemory(_wsText, DEWUI_MAX_TITLE * sizeof(wchar_t));

    return;
}

void DEWUIOBJECT::SetLayoutMode(BOOL bLayoutMode, BOOL bForceDraw)
{
    this->_bLayoutMode = bLayoutMode;
    if (bForceDraw)
        this->Refresh();
    return;
}

void DEWUIOBJECT::SetText(LPCWSTR wsText)
{
    if (!wsText) return;
    if (!lstrcmp(this->_wsText, wsText)) return; // No change in title. Bail.
    ZeroMemory(this->_wsText, DEWUI_MAX_TITLE * sizeof(wchar_t));
    if (lstrlen(wsText) >= DEWUI_MAX_TITLE)
        CopyMemory(this->_wsText, wsText, (DEWUI_MAX_TITLE - 1) * sizeof(wchar_t));
    else
        CopyMemory(this->_wsText, wsText, lstrlen(wsText) * sizeof(wchar_t));
    SetWindowText(this->_hWnd, this->_wsText);
    this->_Draw(_hDC);
    return;
}

void DEWUIOBJECT::SetVisible(BOOL bVisible)
{
    ShowWindow(_hWnd, (bVisible ? SW_SHOW : SW_HIDE));
    return;
}

void DEWUIOBJECT::Refresh()
{
    this->_Draw(_hDC);
    return;
}

void DEWUIOBJECT::Move(int iXPos, int iYPos)
{
    _ptPos.x = iXPos;
    _ptPos.y = iYPos;
    this->_PrepareImages();
    SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, SWP_NOZORDER);
    this->_Draw(NULL);
    return;
}

void DEWUIOBJECT::SetLocation(DWORD dwPresetLocation)
{
    int     iX, iY;

    iX = LOWORD(dwPresetLocation);
    iY = HIWORD(dwPresetLocation);

    if (iX != 0 && iY != 0)
    {
        _ptPos.x = iX;
        _ptPos.y = iY;
        this->_PrepareImages();
        SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, 0, 0, SWP_NOSIZE);
        this->_Draw(NULL);
        return;
    }

    this->_MoveToDefaultLocation();
    return;
}

void DEWUIOBJECT::SetTip(LPCWSTR wsTooltip)
{
    if (!_hWndTooltip || _hWndTooltip == INVALID_HANDLE_VALUE) return;

    TOOLINFO ti { 0 };

    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = _hWnd;
    ti.hinst = _hInstance;
    ti.lpszText = (LPWSTR)wsTooltip;
    GetClientRect(_hWnd, &ti.rect);

    SendMessage(_hWndTooltip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
    return;
}

DEWUIOBJECT::~DEWUIOBJECT()
{
    if (_hDC) { ReleaseDC(_hWnd, _hDC); _hDC = NULL; }
    if (_hWndTooltip) DestroyWindow(_hWndTooltip);
    if (_hWnd)
    {
        RemoveWindowSubclass(_hWnd, DEWUIOBJECT::_MsgHandler, (UINT_PTR)_hWnd);
        DestroyWindow(_hWnd);
        _hWnd = NULL;
    }
    if (_pBrBack) { delete _pBrBack; _pBrBack = NULL; }
    if (_pBrText) { delete _pBrText; _pBrText = NULL; }
    if (_pPenOutline) { delete _pPenOutline; _pPenOutline = NULL; }
    if (_pPenText) { delete _pPenText; _pPenText = NULL; }
    if (_pFont) { delete _pFont; _pFont = NULL; }

    DEWUIOBJECT::_snInstanceCount--;

    if (DEWUIOBJECT::_snInstanceCount == 0)
    {
        // No more objects of type DEWUIOBJECT left in memory.
        // Safe to remove the shared resources.
        if (DEWUIOBJECT::_pImgBackground)
        {
            delete DEWUIOBJECT::_pImgBackground;
            DEWUIOBJECT::_pImgBackground = NULL;
        }
        if (DEWUIOBJECT::_slpSymbolFont) delete (DEWUIOBJECT::_slpSymbolFont);
        DEWUIOBJECT::_slpSymbolFont = NULL;
    }

    return;
}
