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

#include "DewAlbumArt.h"

void DEWALBUMART::_Draw(HDC hDC)
{
    HDC         hDCCanvas = NULL;
    Graphics    *pGr;

    hDCCanvas = (hDC == NULL) ? GetDC(_hWnd) : hDC;
    pGr = Graphics::FromHDC(hDCCanvas);
    pGr->DrawImage(_pImgAlbumArt, 0, 0);
    if (_bLayoutMode)
        pGr->DrawRectangle(_pPenOutline, -_F(0), -_F(0), _iWidth - _F(1), _iHeight - _F(1));
    delete pGr;
    pGr = NULL;

    if (hDC == NULL) ReleaseDC(_hWnd, hDCCanvas);
    return;
}

void DEWALBUMART::_PrepareImages()
{
    Graphics        *pGr = NULL;
    REAL            rSrcWidth, rSrcHeight, rDestWidth, rDestHeight;
    REAL            rSrcRatio = 1.0f, rDestRatio;
    REAL            rScaledWidth = 0.0f, rScaledHeight = 0.0f;
    BOOL            bAllIn = FALSE;
    PointF          ptStart(0.0f, 0.0f);

    if (!_pImgDefault)
    {
        RectF       rctPos(0, 0, 0, 0);
        REAL        rImgDim;

        _pImgDefault = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
        pGr = Graphics::FromImage(_pImgDefault);
        pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
        if (!_bTransparentIcons)
            pGr->FillRectangle(_pBrBack, 0, 0, _iWidth, _iHeight);
        pGr->DrawRectangle(_pPenOutline, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
        pGr->SetSmoothingMode(SmoothingModeAntiAlias);
        pGr->SetTextRenderingHint(TextRenderingHintAntiAlias);
        pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
        rImgDim = _fDimension - _fScale * 2.0f;

        ptStart.X = (rImgDim - rctPos.Width)/2.0f;
        ptStart.Y = (rImgDim - rctPos.Height)/2.0f;

        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);
        delete pGr;
        pGr = NULL;
    }

    if (!_pImgFromEngine)
    {
        // We could simply assign. But later on, we are flushing _pImgAlbumArt
        // So, if _pImgAlbumArt was previously poiting to _pImgDefault,
        // things would go south, with _pImgDefault becoming a limbo-pointer!
        if (_pImgAlbumArt) { delete _pImgAlbumArt; _pImgAlbumArt = NULL; }
        _pImgAlbumArt = _pImgDefault->Clone();
        return;
    }

    rSrcWidth = (REAL)_pImgFromEngine->GetWidth();
    rSrcHeight = (REAL)_pImgFromEngine->GetHeight();
    rDestWidth = _fDimension - _fScale * 2.0f;
    rDestHeight = rDestWidth;

    rSrcRatio = rSrcWidth / rSrcHeight;
    rDestRatio = 1.0f; // We know this as the album art control is always a square

    if (rSrcWidth < rDestWidth && rSrcHeight < rDestHeight) bAllIn = TRUE;

    if (bAllIn)
    {
        rScaledWidth = rSrcWidth;
        rScaledHeight = rSrcHeight;
    }
    else
    {
        rScaledWidth = (rSrcRatio > rDestRatio ? rDestWidth : (rDestHeight/rSrcHeight*rSrcWidth));
        rScaledHeight = (rSrcRatio < rDestRatio ? rDestHeight : (rDestWidth/rSrcWidth*rSrcHeight));
    }
    ptStart.X = (((INT)rDestWidth - (INT)rScaledWidth) / 2.0f) + _F(1);
    ptStart.Y = (((INT)rDestHeight - (INT)rScaledHeight) / 2.0f) + _F(1);

    if (_pImgAlbumArt) { delete _pImgAlbumArt; _pImgAlbumArt = NULL; }
    _pImgAlbumArt = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgAlbumArt);
    if (_bTransparentIcons)
    {
        pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    }
    else
    {
        pGr->FillRectangle(_pBrBack, 0, 0, _iWidth, _iHeight);
        pGr->DrawRectangle(_pPenOutline, 0, 0, _iWidth - _F(1), _iHeight - _F(1)); // <- BIG DEBATE: Whether this should respect TRANSPARENCY!
    }
    pGr->DrawImage(_pImgFromEngine, RectF(ptStart.X, ptStart.Y, rScaledWidth, rScaledHeight), 0, 0, rSrcWidth, rSrcHeight, UnitPixel);

    delete pGr;
    pGr = NULL;

    return;
}

LRESULT DEWALBUMART::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC             hDC = NULL;
    PAINTSTRUCT     ps { 0 };
    LRESULT         lrHit;
    RECT            rctClient { 0 };
    LPWINDOWPOS     lpPos;

    switch (nMsg)
    {
        case WM_PAINT:
        {
            hDC = BeginPaint(hWnd, &ps);
            this->_Draw(hDC);
            EndPaint(hWnd, &ps);
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
                if (_pImgDefault) { delete _pImgDefault; _pImgDefault = NULL; }
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

void DEWALBUMART::_MoveToDefaultLocation()
{
    int     iX, iY;
    float   fSeekBarTop, fXGap;

    iX = DEWUI_DIM_PADDING_X;
    iY = DEWUI_DIM_PADDING_Y_HEADER;
    fXGap = ( ((DEWUI_DIM_WINDOW_X - DEWUI_DIM_PADDING_X * 2) * _fScale) -
              ((DEWUI_DIM_ALBUM_ART + DEWUI_DIM_PLAY_BUTTON + DEWUI_DIM_PNS_BUTTON * 3) * _fScale * _fIconScale) ) / 4.0f;
    fSeekBarTop = ((DEWUI_DIM_WINDOW_Y_NOPL - DEWUI_DIM_PADDING_Y_FOOTER) * _fScale) -
                   (DEWUI_DIM_BUTTON * _fScale * _fIconScale + DEWUI_DIM_SEEKBAR_HEIGHT * _fScale) -
                   (10.0f / _fIconScale );
    
    _ptPos.x = _F(iX);
    _ptPos.y = (int)(((fSeekBarTop - DEWUI_DIM_PADDING_Y_HEADER * _fScale) - _fDimension) / 2.0f + DEWUI_DIM_PADDING_Y_HEADER * _fScale);
    if (_pImgDefault) { delete _pImgDefault; _pImgDefault = NULL; }
    this->_PrepareImages();
    SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, 0, 0, SWP_NOSIZE);
    this->_Draw(NULL);
    return;
}

DEWALBUMART::DEWALBUMART(LPDEWUIOBJECT Parent, const DEWTHEME& Theme, Image* pImgAlbumArt)
{
    const wchar_t       *wsAlbumArtClass = L"DEWDROP.ALBUMART.WND";
    RECT                rctClient;
    WNDCLASSEX          wcex { 0 };

    _btObjectID = DEWUI_OBJ_ALBUM_ART;
    _btObjectType = DEWUI_OBJ_TYPE_MODULE;
    _Parent = Parent;
    StringCchPrintf(_wsText, DEWUI_MAX_TITLE, DEWUI_SYMBOL_ALBUM_ART);
    _iDPI = Theme.DPI;
    _fScale = Theme.Scale;

    this->_Scale(Theme.IconScale);
    GetClientRect((_Parent->Handle), &rctClient);
    _ptPos.x = 0;
    _ptPos.y = 0;
    this->ApplyTheme(Theme, FALSE, FALSE);
    this->SetAlbumArt(pImgAlbumArt, FALSE);

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
    wcex.lpszClassName  = wsAlbumArtClass;

    RegisterClassEx(&wcex);

    _hWnd = CreateWindowEx(0, wsAlbumArtClass, _wsText, _ARTSTYLE, _ptPos.x, _ptPos.y,
                           _iWidth, _iHeight, _Parent->Handle, NULL, _hInstance, NULL);
    _hDC = GetDC(_hWnd);
    this->_InitiateSubclassing();
    return;
}

void DEWALBUMART::ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw, BOOL bCalcScale)
{
    _crBack.SetFromCOLORREF(Theme.ModuleStyle.BackColor);
    _crText.SetFromCOLORREF(Theme.ModuleStyle.TextColor);
    _crOutline.SetFromCOLORREF(Theme.ModuleStyle.OutlineColor);

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
    if (Theme.IconScale != DEWTHEME_ICON_SCALE_SMALL &&
        Theme.IconScale != DEWTHEME_ICON_SCALE_MEDIUM &&
        Theme.IconScale != DEWTHEME_ICON_SCALE_LARGE)
        _fIconScale = DEWUIOBJECT::_sfIconScalingIndex[DEWTHEME_ICON_SCALE_MEDIUM];
    else
        _fIconScale = DEWUIOBJECT::_sfIconScalingIndex[Theme.IconScale];
    // bCalcScale will be true only after the application is up and running
    // and has been called by browsing a theme file
    if (bCalcScale)
    {
        this->_Scale(Theme.IconScale);
        if (_pImgDefault) { delete _pImgDefault; _pImgDefault = NULL; }
        if (_hWnd) SetWindowPos(_hWnd, 0, 0, 0, _iWidth, _iHeight, SWP_NOZORDER | SWP_NOMOVE);
    }

    if (_pFont) { delete _pFont; _pFont = NULL; }
    _pFont = new Font(_lpSymbolFont->SymbolFontGDIPlus,
                      (float)DEWUI_FNTSPEC_ALBUM_ART * _fIconScale,
                      FontStyleRegular, UnitPoint);
    this->_PrepareImages();
    if (bForceDraw)
        this->_Draw(NULL);
    return;
}

void DEWALBUMART::SetAlbumArt(Image* pImgAlbumArt, BOOL bForceDraw)
{
    Graphics    *pGr = NULL;
    REAL        rSrcWidth = 0, rSrcHeight = 0;

    if (_pImgFromEngine) { delete _pImgFromEngine; _pImgFromEngine = NULL; }
    if (pImgAlbumArt)
    {
        // Could have used a 'clone'. But we deliberately want to avoid any 32 bpp transparency
        rSrcWidth = (REAL)pImgAlbumArt->GetWidth();
        rSrcHeight = (REAL)pImgAlbumArt->GetHeight();
        if (rSrcWidth > 0 && rSrcHeight > 0)
        {
            _pImgFromEngine = new Bitmap((INT)rSrcWidth, (INT)rSrcHeight, PixelFormat24bppRGB);
            pGr = Graphics::FromImage(_pImgFromEngine);
            pGr->DrawImage(pImgAlbumArt, 0.0f, 0.0f, rSrcWidth, rSrcHeight);
            delete pGr;
            pGr = NULL;
        }
    }

    if (bForceDraw)
    {
        this->_PrepareImages();
        this->_Draw(NULL);
    }
    return;
}

void DEWALBUMART::SetLocation(DWORD dwPresetLocation)
{
    int     iX, iY;

    iX = LOWORD(dwPresetLocation);
    iY = HIWORD(dwPresetLocation);

    if (iX != 0 && iY != 0)
    {
        _ptPos.x = iX;
        _ptPos.y = iY;
        if (_pImgDefault) { delete _pImgDefault; _pImgDefault = NULL; }
        this->_PrepareImages();
        SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, 0, 0, SWP_NOSIZE);
        this->_Draw(NULL);
        return;
    }

    this->_MoveToDefaultLocation();
    return;
}


void DEWALBUMART::ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue) {} // NOP

DEWALBUMART::~DEWALBUMART()
{
    if (_pImgDefault) { delete _pImgDefault; _pImgDefault = NULL; }
    if (_pImgFromEngine) { delete _pImgFromEngine; _pImgFromEngine = NULL; }
    if (_pImgAlbumArt) { delete _pImgAlbumArt; _pImgAlbumArt = NULL; }
    return;
}
