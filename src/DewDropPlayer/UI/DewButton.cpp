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

#include "DewButton.h"

BOOL DEWBUTTON::_sbClassRegistered = FALSE;
UINT DEWBUTTON::_snInstanceCount = 0;
wchar_t DEWBUTTON::_swsButtonClassName[32] { 0 };

void DEWBUTTON::_Draw(HDC hDC)
{
    HDC         hDCCanvas = NULL;
    Graphics    *pGr;

    hDCCanvas = (hDC == NULL) ? GetDC(_hWnd) : hDC;
    pGr = Graphics::FromHDC(hDCCanvas);
    if ((!_bDepressed || _bLayoutMode) && _pImgRaised)
        pGr->DrawImage(_pImgRaised, 0, 0);
    else if (_bDepressed && !_bLayoutMode && _pImgSunken)
        pGr->DrawImage(_pImgSunken, 0, 0);

    if (_bLayoutMode)
        pGr->DrawRectangle(_pPenOutline, -_F(0), -_F(0), _iWidth - _F(1), _iHeight - _F(1));

    delete pGr;
    pGr = NULL;

    if (hDC == NULL) ReleaseDC(_hWnd, hDCCanvas);
    return;
}

void DEWBUTTON::_ClickedEvent()
{
    if (_Parent)
        _Parent->ProcessMessage(this, DEWUI_MSG_CLICKED, 0, 0);
    return;
}

LRESULT DEWBUTTON::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    RECT        rctClient;
    POINT       ptPos;
    LRESULT     lrHit;
    HDC         hDC = NULL;
    PAINTSTRUCT ps { 0 };
    LPWINDOWPOS lpPos;

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
                SetCapture(_hWnd);
                this->_Draw(NULL);
                return FALSE;
            }
            break;
        }
        case WM_LBUTTONUP:
        {
            if (_bDepressed && !_bLayoutMode)
            {
                ReleaseCapture();
                GetClientRect(_hWnd, &rctClient);
                ptPos.x = LOWORD(lParam);
                ptPos.y = HIWORD(lParam);
                if (ptPos.x >= rctClient.left && ptPos.x <= rctClient.right &&
                    ptPos.y >= rctClient.top && ptPos.y <= rctClient.bottom)
                {
                    // Click has happened
                    _bDepressed = FALSE;
                    // First draw back the 'raised' state
                    this->_Draw(NULL);
                    this->_ClickedEvent();
                    return FALSE;
                }
                _bDepressed = FALSE;
                this->_Draw(NULL);
                return FALSE;
            }
            break;
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
                else if ((lpPos->x + lpPos->cx) >= _F(DEWUI_DIM_WINDOW_X) - _F(1)) lpPos->x = _F(DEWUI_DIM_WINDOW_X) - lpPos->cx - _F(1);

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

DEWBUTTON::DEWBUTTON()
{
    DEWBUTTON::_snInstanceCount++;

    if (!(DEWBUTTON::_sbClassRegistered))
    {
        WNDCLASSEX      wcex { 0 };
        StringCchPrintf((DEWBUTTON::_swsButtonClassName), 32, L"DEWDROP.BUTTON.WND");

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
        wcex.lpszClassName  = DEWBUTTON::_swsButtonClassName;

        RegisterClassEx(&wcex);
        DEWBUTTON::_sbClassRegistered = TRUE;
    }
    return;
}

void DEWBUTTON::ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw, BOOL bCalcScale)
{
    FontStyle   gdiFontStyle;

    if (_btObjectType == DEWUI_OBJ_TYPE_UIBUTTON)
    {
        _crBack.SetFromCOLORREF(Theme.UIButtonStyle.BackColor);
        _crText.SetFromCOLORREF(Theme.UIButtonStyle.TextColor);
        _crOutline.SetFromCOLORREF(Theme.UIButtonStyle.OutlineColor);
    }
    else if (_btObjectType == DEWUI_OBJ_TYPE_MMBUTTON)
    {
        _crBack.SetFromCOLORREF(Theme.MMButtonStyle.BackColor);
        _crText.SetFromCOLORREF(Theme.MMButtonStyle.TextColor);
        _crOutline.SetFromCOLORREF(Theme.MMButtonStyle.OutlineColor);
    }
    else
        return;

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
        if (_hWnd) SetWindowPos(_hWnd, 0, 0, 0, _iWidth, _iHeight, SWP_NOZORDER | SWP_NOMOVE);
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

void DEWBUTTON::ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue) {} // NOP

DEWBUTTON::~DEWBUTTON()
{
    if (_pImgRaised) { delete _pImgRaised; _pImgRaised = NULL; }
    if (_pImgSunken) { delete _pImgSunken; _pImgSunken = NULL; }
    DEWBUTTON::_snInstanceCount--;
    // Last instance removed. Safe to unregister the class
    if (DEWBUTTON::_snInstanceCount == 0)
    {
        UnregisterClass(DEWBUTTON::_swsButtonClassName, _hInstance);
        DEWBUTTON::_sbClassRegistered = FALSE;
    }
    return;
}
