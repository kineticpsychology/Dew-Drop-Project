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

#include "DewButtonVolume.h"

void DEWVOLUMEBUTTON::_Draw(HDC hDC)
{
    HDC         hDCCanvas = NULL;
    Graphics    *pGr;
    REAL        fVolLength = 0;

    hDCCanvas = (hDC == NULL) ? GetDC(_hWnd) : hDC;
    pGr = Graphics::FromHDC(hDCCanvas);
    if (_pImgCurr)
    {
        pGr->DrawImage(*_pImgCurr, 0, 0);
        fVolLength = (_fVolUnitScale * (float)_dwVolBarLength);
        pGr->FillRectangle(_bAlternateIconMode ? _pBrBack : _pBrText, (INT)_fDimension + _F(_PADDING) + _F(1), _F(_PADDING) + _F(1),
                           (INT)fVolLength - _F(1), (INT)_fDimension - 2 * _F(_PADDING) - _F(1));
    }

    if (_bLayoutMode)
        pGr->DrawRectangle(_pPenOutline, -_F(0), -_F(0), _iWidth - _F(1), _iHeight - _F(1));

    delete pGr;
    pGr = NULL;

    if (hDC == NULL) ReleaseDC(_hWnd, hDCCanvas);
    return;
}

void DEWVOLUMEBUTTON::_ClearImages()
{
    if (_pImgVol00) { delete _pImgVol00; _pImgVol00 = NULL; }
    if (_pImgVol33) { delete _pImgVol33; _pImgVol33 = NULL; }
    if (_pImgVol67) { delete _pImgVol67; _pImgVol67 = NULL; }
    if (_pImgVol100) { delete _pImgVol100; _pImgVol100 = NULL; }

    return;
}

void DEWVOLUMEBUTTON::_CreateVolumeImages(Image **ppImg, LPCWSTR wsVolCaption)
{
    Graphics    *pGr = NULL;
    PointF      ptStart(0.0f, 0.0f);
    RectF       rctPos;

    pGr = Graphics::FromImage(*ppImg);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->MeasureString(wsVolCaption, -1, _pFont, ptStart, &rctPos);
    ptStart.X = (_fDimension - rctPos.Width) / 2.0f + _fScale * 0.0f;
    ptStart.Y = (_fDimension - rctPos.Height) / 2.0f + _fScale * 1.0f;
    if (_bAlternateIconMode)
        pGr->DrawString(wsVolCaption, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(wsVolCaption, -1, _pFont, ptStart, _pBrText);

    delete pGr;
    pGr = NULL;
    return;
}

void DEWVOLUMEBUTTON::_PrepareImages()
{
    Graphics        *pGr = NULL;
    Image           *pImgBase = NULL;

    this->_ClearImages();

    pImgBase = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(pImgBase);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
        pGr->FillRectangle(_bAlternateIconMode ? _pBrText : _pBrBack, _rctBounds);
    pGr->DrawRectangle(_pPenOutline, _rctBounds);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    if (!_bTransparentIcons)
        pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), (INT)_fDimension - _F(1), _iHeight - _F(1));
    if (!_bAlternateIconMode)
        pGr->DrawEllipse(_pPenOutline, 0, 0, (INT)_fDimension - _F(1), _iHeight - _F(1));
    delete pGr;
    pGr = NULL;

    _pImgVol00 = pImgBase->Clone(); this->_CreateVolumeImages(&_pImgVol00, DEWUI_SYMBOL_VOL00);
    _pImgVol33 = pImgBase->Clone(); this->_CreateVolumeImages(&_pImgVol33, DEWUI_SYMBOL_VOL33);
    _pImgVol67 = pImgBase->Clone(); this->_CreateVolumeImages(&_pImgVol67, DEWUI_SYMBOL_VOL67);
    _pImgVol100 = pImgBase->Clone(); this->_CreateVolumeImages(&_pImgVol100, DEWUI_SYMBOL_VOL100);

    delete pImgBase;
    pImgBase = NULL;

    return;
}

void DEWVOLUMEBUTTON::_ClickedEvent(DWORD dwNewVolLevel)
{
    if (_Parent)
        _Parent->ProcessMessage(this, DEWUI_MSG_VOL_CHANGED, DEWUI_NCODE_NEXT_LOGICAL_VALUE, dwNewVolLevel);
    return;
}

LRESULT DEWVOLUMEBUTTON::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    RECT            rctClient, rctLock;
    LONG            lVolLevel;
    POINT           ptPos;
    LRESULT         lrHit;
    HDC             hDC = NULL;
    PAINTSTRUCT     ps { 0 };
    LPWINDOWPOS     lpPos;
    static POINT    ptLastPos;
    static DWORD    dwPrevVolLevel;

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
            if ((wParam & MK_LBUTTON) == MK_LBUTTON && !_bLayoutMode)
            {
                _bDepressed = TRUE;
                // Start tracking the mouse
                ZeroMemory(&_tme, sizeof(TRACKMOUSEEVENT));
                _tme.cbSize = sizeof(TRACKMOUSEEVENT);
                _tme.dwFlags = TME_LEAVE;
                _tme.hwndTrack = _hWnd;
                TrackMouseEvent(&_tme);
                // Keep a backup of the original volume level & the cursor position
                // We'll restore them after the user is done 'playing' with the volumne bar
                dwPrevVolLevel = _dwCurrVolLevel;
                GetCursorPos(&ptLastPos);

                // Restrict the cursor within the volume bar rectangle
                rctLock.left = _rctBounds.GetLeft();
                rctLock.top = _rctBounds.GetTop();
                rctLock.right = _rctBounds.GetRight();
                rctLock.bottom = _rctBounds.GetBottom();
                MapWindowPoints(_hWnd, HWND_DESKTOP, (LPPOINT)&rctLock, 2);
                ClipCursor(&rctLock);
                ShowCursor(FALSE);

                // Set the initial point of the cursor to the current volume bar
                ptPos.x = (INT)(_fDimension + _fVolUnitScale * (float)_dwVolBarLength) + _F(_PADDING);
                ptPos.y = rctLock.left + _rctBounds.Height / 2;
                MapWindowPoints(_hWnd, HWND_DESKTOP, &ptPos, 1);
                SetCursorPos(ptPos.x, ptPos.y);

                SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, SWP_NOZORDER);
                this->_Draw(NULL);
                return FALSE;
            }
            break;
        }

        case WM_LBUTTONUP:
        {
            if (_bDepressed && !_bLayoutMode)
            {
                // Remove the lock
                ClipCursor(NULL);
                // Restore the original position
                SetCursorPos(ptLastPos.x, ptLastPos.y);
                // Display the cursor back
                ShowCursor(TRUE);
                GetClientRect(_hWnd, &rctClient);
                // Revert to the 'trimmed' shape
                SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, (INT)_fDimension, _iHeight, SWP_NOZORDER);
                ptPos.x = LOWORD(lParam);
                ptPos.y = HIWORD(lParam);
                if (ptPos.x >= rctClient.left && ptPos.x <= rctClient.right &&
                    ptPos.y >= rctClient.top && ptPos.y <= rctClient.bottom)
                {
                    // Click has happened
                    _bDepressed = FALSE;
                    lVolLevel = (LONG)_dwCurrVolLevel;
                    // Retain the old actual volume level
                    this->SetVolumeLevel(dwPrevVolLevel);
                    // and send the new 'intended' volume level to parent
                    // Parent will decide whether or not to set the volume to
                    // that level. (Depends on whether volume adjustment
                    // is handled successfully in the engine). An example
                    // is MIDI, where the volume adjustment won't work
                    // In such situations, we'd like to stick to the original value
                    this->_ClickedEvent((DWORD)lVolLevel);
                    return FALSE;
                }
                _bDepressed = FALSE;
                this->_Draw(NULL);
                return FALSE;
            }
            break;
        }

        case WM_MOUSEMOVE:
        {
            if (!_bLayoutMode && _bDepressed)
            {
                ptPos.x = LOWORD(lParam);
                // A ~VOLMINMAXTOLERANCE (DPI adjusted) pixel tolerance for the extreme MIN/MAX values)
                if (ptPos.x >= _rctBounds.GetRight() - _F(_VOLMINMAXTOLERANCE))
                {
                    lVolLevel = DEW_MAX_VOL_LEVEL;
                }
                else if (ptPos.x <= _rctBounds.GetLeft() + _F(_VOLMINMAXTOLERANCE))
                {
                    lVolLevel = 0;
                }
                else
                {
                    lVolLevel = (LONG)ptPos.x - (LONG)_rctBounds.GetLeft();
                    if (lVolLevel < 0) lVolLevel = 0;
                    lVolLevel = (long)((float)lVolLevel * (float)(DEW_MAX_VOL_LEVEL) / (float)(_rctBounds.Width));
                    
                }
                this->SetVolumeLevel((DWORD)lVolLevel);
                // Keep sending the 'new' value continuously to the parent (albeit the original value is retained)
                this->_ClickedEvent((DWORD)lVolLevel);

                return FALSE;
            }
        }

        case WM_MOUSELEAVE:
        {
            if (_bDepressed)
            {
                _bDepressed = FALSE;
                // Remove the lock
                ClipCursor(NULL);
                // Display the cursor back
                ShowCursor(TRUE);
                // Do NOT restore the cursor position!
                // Restore the window to the 'trimmed' size
                SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, (INT)_fDimension, _iHeight, SWP_NOZORDER);
                // Restore the last (before mouse down) volume level
                this->SetVolumeLevel(dwPrevVolLevel);
            }
            return FALSE;
        }

        case WM_NCHITTEST:
        {
            if (_bLayoutMode)
            {
                lrHit = DefWindowProc(hWnd, nMsg, wParam, lParam);
                if (lrHit == HTCLIENT) lrHit = HTCAPTION;
                return lrHit;
            }
            break;
        }
        case WM_MOVE:
        {
            if (_bLayoutMode)
            {
                GetWindowRect(_hWnd, &rctClient);
                MapWindowPoints(HWND_DESKTOP, _Parent->Handle, (LPPOINT)&rctClient, 2);
                _ptPos.x = rctClient.left;
                _ptPos.y = rctClient.top;
                this->_PrepareImages();
                this->_Draw(NULL);
                if (_Parent)
                    SendMessage(_Parent->Handle, WM_DEWMSG_CHILD_POS_CHANGED, _btObjectID, MAKELPARAM(_ptPos.x, _ptPos.y));
                return FALSE;
            }
            break;
        }

        case WM_WINDOWPOSCHANGING:
        {
            if (_bLayoutMode)
            {
                lpPos = (LPWINDOWPOS)lParam;

                if (lpPos->x < _F(1)) lpPos->x = _F(1);
                else if ((lpPos->x + _iWidth) >= _F(DEWUI_DIM_WINDOW_X) - _F(1)) lpPos->x = _F(DEWUI_DIM_WINDOW_X) - _iWidth - _F(1);

                if (lpPos->y < _F(1)) lpPos->y = _F(1);
                else if ((lpPos->y + lpPos->cy) >= _F(DEWUI_DIM_WINDOW_Y_NOPL) - _F(1)) lpPos->y = _F(DEWUI_DIM_WINDOW_Y_NOPL) - lpPos->cy - _F(1);

                return FALSE;
            }
            break;
        }

        case WM_NCLBUTTONDOWN:
        {
            if (_bLayoutMode && _Parent)
            {
                SendMessage(_Parent->Handle, WM_DEWMSG_LAYOUT_OBJ_ACTIVATED,
                            _btObjectID, (LPARAM)_hWnd);
            }
            break;
        }

    }

    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWVOLUMEBUTTON::_Scale(BYTE btIconScale)
{
    if (_btObjectID != DEWUI_OBJ_VOLUME_BUTTON) return; // Rhetoric check!
    _fIconScale = DEWUIOBJECT::_sfIconScalingIndex[btIconScale];
    _iFontSize = 0x3F & DEWUI_FNTSPEC_BUTTON;
    _iFontSize = (int)((float)_iFontSize * _fIconScale);
    _bBold = ((DEWUI_FNTSPEC_BUTTON & 0x40) == 0x40);
    _fDimension = DEWUI_DIM_BUTTON * _fScale * _fIconScale;
    _iWidth = (int)((float)DEWUI_DIM_VOL_BUTTON_X * _fScale * _fIconScale);
    _iHeight = (int)_fDimension;
    _dwVolBarLength = _iWidth - (INT)_fDimension - 2 * _F(_PADDING);

    _rctBounds.X = (int)_fDimension + _F(_PADDING);
    _rctBounds.Y = _F(_PADDING);
    _rctBounds.Width = _dwVolBarLength;
    _rctBounds.Height = (INT)_fDimension - 2 * _F(_PADDING);

    return;
}

void DEWVOLUMEBUTTON::_MoveToDefaultLocation()
{
    int     xOffset = DEWUI_DIM_PADDING_X;
    int     yOffset = _F((DEWUI_DIM_WINDOW_Y_NOPL - DEWUI_DIM_PADDING_Y_FOOTER)) - (int)(((float)DEWUI_DIM_BUTTON) * _fScale * _fIconScale);

    _ptPos.x = _F(xOffset);
    _ptPos.y = yOffset;
    this->_PrepareImages();
    SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, 0, 0, SWP_NOSIZE);
    this->_Draw(NULL);
    return;
}

DEWVOLUMEBUTTON::DEWVOLUMEBUTTON(LPDEWUIOBJECT Parent, const DEWTHEME& Theme, DWORD dwInitVolLevel) : VolumeLevel(_dwCurrVolLevel)
{
    RECT            rctClient;
    WNDCLASSEX      wcex { 0 };

    StringCchPrintf(_wsVolBtnClass, 32, L"DEWDROP.VOLBUTTON.WND");

    _btObjectID = DEWUI_OBJ_VOLUME_BUTTON;
    _btObjectType = DEWUI_OBJ_TYPE_MMBUTTON;
    _Parent = Parent;
    ZeroMemory(_wsText, DEWUI_MAX_TITLE * sizeof(wchar_t));
    _iDPI = Theme.DPI;
    _fScale = Theme.Scale;

    this->_Scale(Theme.IconScale);
    this->SetVolumeLevel(dwInitVolLevel, FALSE);
    GetClientRect((_Parent->Handle), &rctClient);
    _ptPos.x = 0;
    _ptPos.y = 0;
    this->ApplyTheme(Theme, FALSE, FALSE);

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
    wcex.lpszClassName  = _wsVolBtnClass;

    RegisterClassEx(&wcex);
    // Create the 'trimmed' version. We will 'exapnd' as long as mouse is down
    _hWnd = CreateWindowEx(WS_EX_TOPMOST, _wsVolBtnClass, _wsText, _VOLSTYLE, _ptPos.x, _ptPos.y,
                           (INT)_fDimension, _iHeight, _Parent->Handle, NULL, _hInstance, NULL);
    _hDC = GetDC(_hWnd);
    this->_CreateTooltip();
    this->_InitiateSubclassing();

    return;
}

void DEWVOLUMEBUTTON::ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw, BOOL bCalcScale)
{
    FontStyle   gdiFontStyle;

    if (_btObjectType != DEWUI_OBJ_TYPE_MMBUTTON) return;

    _crBack.SetFromCOLORREF(Theme.MMButtonStyle.BackColor);
    _crText.SetFromCOLORREF(Theme.MMButtonStyle.TextColor);
    _crOutline.SetFromCOLORREF(Theme.MMButtonStyle.OutlineColor);

    if (_pBrText) delete _pBrText;
    if (_pBrBack) delete _pBrBack;
    if (_pPenOutline) delete _pPenOutline;
    if (_pPenText) delete _pPenText;

    _pBrText = new SolidBrush(_crText);
    _pBrBack = new SolidBrush(_crBack);
    _pPenOutline = new Pen(_crOutline, (REAL)_F(1));
    _pPenText = new Pen(_crText, (REAL)_F(1));

    _bAlternateIconMode = (Theme.IconMode == DEWTHEME_ICON_MODE_ALTERNATE);
    _bTransparentIcons = (Theme.TransparentIcons == 1);
    // bCalcScale will be true only after the application is up and running
    // and has been called by browsing a theme file
    if (bCalcScale)
    {
        this->_Scale(Theme.IconScale);
        if (_hWnd) SetWindowPos(_hWnd, 0, 0, 0, (INT)_fDimension, _iHeight, SWP_NOZORDER | SWP_NOMOVE);
    }

    // Update the font sizes
    if (_pFont) { delete _pFont; _pFont = NULL; }
    _bItalic = FALSE;
    if (_bBold)
        gdiFontStyle = _bItalic ? FontStyleBoldItalic : FontStyleBold;
    else
        gdiFontStyle = _bItalic ? FontStyleItalic : FontStyleRegular;
    _pFont = new Font(_lpSymbolFont->SymbolFontGDIPlus, (REAL)_iFontSize, gdiFontStyle, UnitPoint);

    this->_PrepareImages();
    if (bForceDraw)
        this->_Draw(NULL);
    return;
}

void DEWVOLUMEBUTTON::SetVolumeLevel(DWORD dwVolLevel, BOOL bForceDraw)
{
    if (dwVolLevel >= DEW_MAX_VOL_LEVEL)
    {
        _dwCurrVolLevel = DEW_MAX_VOL_LEVEL;
        _fVolUnitScale = 1.0f;
    }
    else if (dwVolLevel == 0)
    {
        _dwCurrVolLevel = dwVolLevel;
        _fVolUnitScale = 0.0f;
    }
    else
    {
        _dwCurrVolLevel = dwVolLevel;
        _fVolUnitScale = ((float)_dwCurrVolLevel) / ((float)DEW_MAX_VOL_LEVEL);
    }

    if (_fVolUnitScale == 0.0f) _pImgCurr = &_pImgVol00;
    else if (_fVolUnitScale > 0.0f && _fVolUnitScale <= 0.33f) _pImgCurr = &_pImgVol33;
    else if (_fVolUnitScale > 0.33f && _fVolUnitScale <= 0.67f) _pImgCurr = &_pImgVol67;
    else if (_fVolUnitScale > 0.67f) _pImgCurr = &_pImgVol100;

    if (bForceDraw)
        this->_Draw(NULL);

    return;
}

void DEWVOLUMEBUTTON::SetLayoutMode(BOOL bLayoutMode, BOOL bForceDraw)
{
    this->_bLayoutMode = bLayoutMode;
    if (bLayoutMode)
        SetWindowPos(_hWnd, 0, 0, 0, _iWidth, _iHeight, SWP_NOZORDER | SWP_NOMOVE);
    else
        SetWindowPos(_hWnd, 0, 0, 0, (int)_fDimension, _iHeight, SWP_NOZORDER | SWP_NOMOVE);
    if (bForceDraw)
        this->Refresh();
    return;
}

void DEWVOLUMEBUTTON::ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue) {} // NOP

DEWVOLUMEBUTTON::~DEWVOLUMEBUTTON()
{
    this->_ClearImages();
    UnregisterClass(_wsVolBtnClass, _hInstance);
    return;
}
