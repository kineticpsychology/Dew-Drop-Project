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

#include "DewSeekbar.h"

void DEWSEEKBAR::_ClickedEvent(DWORD dwNewTime)
{
    if (_Parent)
        _Parent->ProcessMessage(this, DEWUI_MSG_SEEKED, DEWUI_NCODE_NEXT_LOGICAL_VALUE, dwNewTime);
    return;
}

void DEWSEEKBAR::_Scale(BYTE btIconScale)
{
    RECT        rctParent;

    if (_btObjectType == DEWUI_OBJ_TYPE_NONE)
        return;
    if (!(btIconScale == DEWTHEME_ICON_SCALE_SMALL ||
          btIconScale == DEWTHEME_ICON_SCALE_MEDIUM ||
          btIconScale == DEWTHEME_ICON_SCALE_LARGE))
        return;

    GetClientRect(_Parent->Handle, &rctParent);
    _fDimension = 0.0f;
    _iWidth = (rctParent.right - rctParent.left) - _F(DEWUI_DIM_PADDING_X) * 2;
    _iHeight = _F(DEWUI_DIM_SEEKBAR_HEIGHT);
    return;
}

void DEWSEEKBAR::_Draw(HDC hDC)
{
    HDC         hDCCanvas = NULL;
    Graphics    *pGr;

    hDCCanvas = (hDC == NULL) ? GetDC(_hWnd) : hDC;
    pGr = Graphics::FromHDC(hDCCanvas);
    pGr->DrawImage(_pImgInst, _F(0), _F(0));

    if (_bLayoutMode)
        pGr->DrawRectangle(_pPenOutline, -_F(0), -_F(0), _iWidth - _F(1), _iHeight - _F(1));

    delete pGr;
    pGr = NULL;

    if (hDC == NULL) ReleaseDC(_hWnd, hDCCanvas);
    return;
}

void DEWSEEKBAR::_PrepareUninitializedImage()
{
    const wchar_t   *wsNATime = L"--:--";
    PointF          ptStart(0.0f, 0.0f);

    if (!_pImgBase) return;

    Graphics    *pGr = NULL;
    if (_pImgInst) { delete _pImgInst; _pImgInst = NULL; }
    _pImgInst = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgInst);
    pGr->DrawImage(_pImgBase, _F(0), _F(0));

    if (_bDisplayTimes)
    {
        ptStart.X = _fScale;
        ptStart.Y = (((REAL)(_iHeight - _F(DEWUI_DIM_SEEK_RECT_HEIGHT))) - _rctTextPos.Height)/2.0f;
        pGr->DrawString(wsNATime, -1, _pFont, ptStart, _pBrText);

        ptStart.X = _iWidth - _rctTextPos.Width - _fScale;
        pGr->DrawString(wsNATime, -1, _pFont, ptStart, _pBrText);
    }
    delete pGr;
    pGr = NULL;
}

void DEWSEEKBAR::_PrepareImages()
{
    Graphics    *pGr = NULL;
    PointF      ptStart(0.0f, 0.0f);
    wchar_t     wsZeroTime[8];

    StringCchPrintf(wsZeroTime, 8, L"00:00");

    if (_pImgBase) { delete _pImgBase; _pImgBase = NULL; }

    _pImgBase = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgBase);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);

    if (!_bTransparentIcons)
        pGr->FillRectangle(_pBrBack, _rctBar);
    pGr->DrawRectangle(_pPenOutline, _rctBar.X, _rctBar.Y, _rctBar.Width - _F(1), _F(DEWUI_DIM_SEEK_RECT_HEIGHT) - _F(1));
    pGr->MeasureString(wsZeroTime, -1, _pFont, ptStart, &_rctTextPos);

    this->_PrepareUninitializedImage();
    delete pGr;
    pGr = NULL;
}

LRESULT DEWSEEKBAR::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC                 hDC = NULL;
    PAINTSTRUCT         ps { 0 };
    LRESULT             lrHit;
    RECT                rctClient;
    int                 iXPos, iYPos;
    LPWINDOWPOS         lpPos;
    TRACKMOUSEEVENT     tme{ 0 };
    static BOOL         sbTrack = FALSE;

    switch(nMsg)
    {
        case WM_PAINT:
        {
            hDC = BeginPaint(hWnd, &ps);
            this->_Draw(hDC);
            EndPaint(hWnd, &ps);
            return FALSE;
        }

        case WM_MOUSELEAVE:
        {
            // As a fail-safe, dsiable the tooltip once the mouse is out
            SendMessage(_hWndTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&_ti);
            // Reset the tracker flag, so that re-tracking of mouse can start
            // once it enters back into the window
            sbTrack = FALSE;
            return FALSE;
        }

        case WM_MOUSEMOVE:
        {
            // Start the mouse tracking, if not started. It will be
            // alive till the mouse leaves the window
            if (!sbTrack)
            {
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = _hWnd;
                TrackMouseEvent(&tme);
                sbTrack = TRUE;
            }
            if (_bLayoutMode) break;
            iXPos = LOWORD(lParam);
            iYPos = HIWORD(lParam);
            if (iXPos >= _rctBar.X && iXPos <= (_rctBar.X + _rctBar.Width) &&
                iYPos >= _rctBar.Y && iYPos <= (_rctBar.Y + _rctBar.Height))
                SetClassLongPtr(_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));
            else
                SetClassLongPtr(_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));

            if (_bTimesInitialized) this->_SetTimeTip(iXPos, iYPos);
            if (!_bTimesInitialized) break;
            return FALSE;
        }

        case WM_LBUTTONDOWN:
        {
            if ((wParam & MK_LBUTTON) == MK_LBUTTON && !_bLayoutMode && sbTrack && _bTimesInitialized)
            {
                _bDepressed = TRUE;
                SetCapture(_hWnd);
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
                iXPos = LOWORD(lParam);
                iYPos = HIWORD(lParam);
                _bDepressed = FALSE;
                if (iXPos >= _rctBar.X && iXPos <= (_rctBar.X + _rctBar.Width) - _F(3) &&
                    iYPos >= _rctBar.Y && iYPos <= (_rctBar.Y + _rctBar.Height) &&
                    _bTimesInitialized)
                {
                    _dwClickedTime = (DWORD)((float)iXPos * _rTimePerBar);
                    this->_ClickedEvent(_dwClickedTime);
                }
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

void DEWSEEKBAR::_MoveToDefaultLocation()
{
    int     iX, iY;

    iX = DEWUI_DIM_PADDING_X;
    iY = _F((DEWUI_DIM_WINDOW_Y_NOPL - DEWUI_DIM_PADDING_Y_FOOTER)) - (int)((float)(DEWUI_DIM_BUTTON * _fScale * _fIconScale + DEWUI_DIM_SEEKBAR_HEIGHT * _fScale)) - (int)(10.0f/(_fIconScale));
    _ptPos.x = _F(iX);
    _ptPos.y = iY;
    this->_PrepareImages();
    SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, 0, 0, SWP_NOSIZE);
    this->_Draw(NULL);
    return;
}

void DEWSEEKBAR::_SetTimeTip(int iXPos, int iYPos)
{
    WORD        wXPos, wYPos;
    POINT       ptCur;
    DWORD       dwPosTime = (DWORD)((float)iXPos * _rTimePerBar);
    wchar_t     wsTip[32]{ 0 };

    if (  iXPos <= _rctBar.X || iXPos >= _rctBar.GetRight() - _F(1) ||
          iYPos <= _rctBar.GetTop() || iYPos >= (_rctBar.GetTop() + _F(DEWUI_DIM_SEEK_RECT_HEIGHT))  )
        SendMessage(_hWndTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&_ti);
    else
        SendMessage(_hWndTooltip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&_ti);

    StringCchPrintf(wsTip, 32, L"%02u:%02u.%03u",
                    (dwPosTime / 1000) / 60, (dwPosTime / 1000) % 60, (dwPosTime - ((dwPosTime / 1000) * 1000))
                   );
    _ti.lpszText = wsTip;
    SendMessage(_hWndTooltip, TTM_SETTOOLINFO, 0, (LPARAM)&_ti);

    GetCursorPos(&ptCur);
    wXPos = (WORD)ptCur.x + _F(10);
    wYPos = (WORD)GetSystemMetrics(SM_CYCURSOR) + (WORD)(ptCur.y);
    SendMessage(_hWndTooltip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(wXPos, wYPos));

    return;
}


DEWSEEKBAR::DEWSEEKBAR(LPDEWUIOBJECT Parent, const DEWTHEME& Theme)
{
    const wchar_t   *wsSeekbarClass = L"DEWDROP.SEEKBAR.WND";
    WNDCLASSEX      wcex { 0 };

    _btObjectID = DEWUI_OBJ_SEEKBAR;
    _btObjectType = DEWUI_OBJ_TYPE_MODULE;
    _Parent = Parent;
    _iDPI = Theme.DPI;
    _fScale = Theme.Scale;
    ZeroMemory(&_rctBar, sizeof(Rect));
    ZeroMemory(&_rctTextPos, sizeof(RectF));
    ZeroMemory(_wsText, DEWUI_MAX_TITLE * sizeof(wchar_t));

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
    wcex.lpszClassName  = wsSeekbarClass;

    RegisterClassEx(&wcex);

    this->_Scale(Theme.IconScale);
    _ptPos.x = _F(0);
    _ptPos.y = _F(0);
    this->ApplyTheme(Theme, FALSE, FALSE);

    // The 'Seekbar' rectangle
    _rctBar.X = 0;
    _rctBar.Y = _iHeight - _F(DEWUI_DIM_SEEK_RECT_HEIGHT);
    _rctBar.Width = _iWidth;
    _rctBar.Height = _F(DEWUI_DIM_PADDING_X);

    _rTimePerBar = 1.0f;

    _hWnd = CreateWindowEx(0, wsSeekbarClass, _wsText, _SEEKSTYLE, _ptPos.x, _ptPos.y,
                           _iWidth, _iHeight, _Parent->Handle, NULL, _hInstance, NULL);
    _hDC = GetDC(_hWnd);
    this->_CreateTooltip();

    _ti.cbSize = sizeof(TOOLINFO);
    _ti.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
    _ti.hwnd = _hWnd;
    _ti.hinst = _hInstance;
    _ti.lpszText = (LPWSTR)L"Well..";
    _ti.uId = (UINT_PTR)_hWnd;
    _ti.rect.left = _rctBar.GetLeft();
    _ti.rect.top = _rctBar.GetTop();
    _ti.rect.right = _rctBar.GetRight();
    _ti.rect.bottom = _ti.rect.top + _F(DEWUI_DIM_SEEK_RECT_HEIGHT);
    InflateRect(&_ti.rect, -_F(1), -_F(1));
    SendMessage(_hWndTooltip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&_ti);

    this->_InitiateSubclassing();
    return;
}

void DEWSEEKBAR::ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw, BOOL bCalcScale)
{
    FontStyle   gdiFontStyle;

    _crBack.SetFromCOLORREF(Theme.ModuleStyle.BackColor);
    _crText.SetFromCOLORREF(Theme.ModuleStyle.TextColor);
    _crOutline.SetFromCOLORREF(Theme.ModuleStyle.OutlineColor);

    if (_pBrText) delete _pBrText;
    if (_pBrBack) delete _pBrBack;
    if (_pBrOutline) delete _pBrOutline;
    if (_pPenOutline) delete _pPenOutline;
    if (_pPenText) delete _pPenText;

    _pBrText = new SolidBrush(_crText);
    _pBrBack = new SolidBrush(_crBack);
    _pBrOutline = new SolidBrush(_crOutline);
    _pPenOutline = new Pen(_crOutline, (REAL)_F(1));
    _pPenText = new Pen(_crText, (REAL)_F(1));

    CopyMemory(_wsFontName, Theme.TextFontStyle.FontName, 32*sizeof(wchar_t));
    _fIconScale = DEWUIOBJECT::_sfIconScalingIndex[Theme.IconScale];
    _iFontSize = (int)((REAL)(Theme.TextFontStyle.FontSize));
    _bBold = Theme.TextFontStyle.IsBold;
    _bItalic = Theme.TextFontStyle.IsItalic;
    _bDisplayTimes = (Theme.ShowSeekbarTimes == 1);

    if (_bBold)
        gdiFontStyle = _bItalic ? FontStyleBoldItalic : FontStyleBold;
    else
        gdiFontStyle = _bItalic ? FontStyleItalic : FontStyleRegular;

    if (_pFont) delete _pFont;
    _pFont = new Font(_wsFontName, (REAL)_iFontSize, gdiFontStyle, UnitPoint);
    _fDimension = 0.0f;
    _bAlternateIconMode = (Theme.IconMode == DEWTHEME_ICON_MODE_ALTERNATE);
    _bTransparentIcons = (Theme.TransparentIcons == 1);
    // bCalcScale will be true only after the application is up and running
    // and has been called by browsing a theme file
    if (bCalcScale)
    {
        this->_Scale(Theme.IconScale);
        if (_hWnd) SetWindowPos(_hWnd, 0, 0, 0, _iWidth, _iHeight, SWP_NOZORDER | SWP_NOMOVE);
    }

    this->_PrepareImages();
    if (bForceDraw)
        this->_Draw(NULL);
    return;
}

void DEWSEEKBAR::SetTime(DWORD dwCurrTime, BOOL bForceDraw)
{
    INT         iBarPos;
    wchar_t     wsCurrTime[16], wsTotalTime[16];
    PointF      ptStart;

    _dwClickedTime = 0;
    if (_bTimesInitialized)
    {
        if ((_dwCurrTime != dwCurrTime && dwCurrTime <= _dwTotalTime) || bForceDraw)
        {
            Graphics    *pGr = NULL;
            _dwCurrTime = (dwCurrTime > _dwTotalTime) ? _dwTotalTime : dwCurrTime;
            if (_pImgInst) { delete _pImgInst; _pImgInst = NULL; }
            _pImgInst = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
            pGr = Graphics::FromImage(_pImgInst);
            pGr->DrawImage(_pImgBase, _F(0), _F(0));

            if (_dwCurrTime == _dwTotalTime)
                iBarPos = _rctBar.Width - _F(2);
            else
                iBarPos = (INT)(((REAL)_dwCurrTime)/_rTimePerBar);
            pGr->FillRectangle(_pBrOutline, _rctBar.X + _F(1), _rctBar.Y + _F(1), iBarPos, _rctBar.Height - _F(2));
            if (_bDisplayTimes)
            {
                StringCchPrintf(wsCurrTime, 16, L"%02u:%02u",
                                (UINT)(((float)_dwCurrTime/1000.0f)/60.0f),
                                ((UINT)((float)_dwCurrTime/1000.0f))%60);
                ptStart.X = _fScale;
                ptStart.Y = (((REAL)(_iHeight - _F(DEWUI_DIM_SEEK_RECT_HEIGHT))) - _rctTextPos.Height)/2.0f;
                pGr->DrawString(wsCurrTime, -1, _pFont, ptStart, _pBrText);

                StringCchPrintf(wsTotalTime, 16, L"%02u:%02u",
                                (UINT)(((float)_dwTotalTime/1000.0f)/60.0f),
                                ((UINT)((float)_dwTotalTime/1000.0f))%60);
                ptStart.X = _iWidth - _rctTextPos.Width - _fScale;
                pGr->DrawString(wsTotalTime, -1, _pFont, ptStart, _pBrText);
            }
            delete pGr;
            pGr = NULL;
            this->_Draw(NULL);
        }
    }
    return;
}

void DEWSEEKBAR::SetLength(DWORD dwTotalTime)
{
    if (dwTotalTime == 0)
    {
        _bTimesInitialized = FALSE;
        _dwCurrTime = 0;
        _dwClickedTime = 0;
        _dwTotalTime = 0;
        _rTimePerBar = 1.0f; // Avoid any accidental Div/0 error
        this->_Draw(NULL);
        return;
    }
    _dwTotalTime = dwTotalTime;
    // Reset the current time. There can be no justification of adjusting
    // the total time, while a song is already in progress.
    _dwCurrTime = 0;
    _rTimePerBar = (float)_dwTotalTime / (float)(_iWidth - _F(2));
    _bTimesInitialized = TRUE;
    this->_Draw(NULL);
    return;
}

void DEWSEEKBAR::ShowTimes(BOOL bShow)
{
    if (_bDisplayTimes == bShow) return; // redundant. Nothing needs to be done
    _bDisplayTimes = bShow;
    this->_Draw(NULL);
    return;
}

void DEWSEEKBAR::Reset()
{
    _bTimesInitialized = FALSE;
    _dwCurrTime = 0;
    _dwClickedTime = 0;
    _dwTotalTime = 0;
    _rTimePerBar = 1.0f; // Avoid any accidental Div/0 error
    this->_PrepareImages();
    this->_Draw(NULL);
    return;
}

void DEWSEEKBAR::SetTip(LPCWSTR wsTooltip)
{
    return; // Intentional overridem, as seekbar's tooltip is of tracking type
}

void DEWSEEKBAR::ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue) {} // NOP

DEWSEEKBAR::~DEWSEEKBAR()
{
    if (_pImgBase) { delete _pImgBase; _pImgBase = NULL; }
    if (_pImgBase) { delete _pImgBase; _pImgBase = NULL; }
}
