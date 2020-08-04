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

#include "DewInfoLabel.h"

wchar_t DEWINFOLABEL::_swsInfoLabelClass[32] { 0 };
BOOL DEWINFOLABEL::_sbClassRegistered = FALSE;
UINT DEWINFOLABEL::_snInstanceCount = 0;

void DEWINFOLABEL::_Draw(HDC hDC)
{
    HDC         hDCCanvas = NULL;
    Graphics    *pGr;

    hDCCanvas = (hDC == NULL) ? GetDC(_hWnd) : hDC;
    pGr = Graphics::FromHDC(hDCCanvas);
    if (_bLayoutMode || lstrlen(_wsText) > 0)
        pGr->DrawImage(_pImgInfo, _F(0), _F(0));
    else
        pGr->DrawImage(_pImgBlank, _F(0), _F(0));
    if (_bLayoutMode)
        pGr->DrawRectangle(_pPenOutline, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    delete pGr;
    pGr = NULL;

    if (hDC == NULL) ReleaseDC(_hWnd, hDCCanvas);
    return;
}

void DEWINFOLABEL::_PrepareImages()
{
    Graphics        *pGr = NULL;
    PointF          ptStart;
    RectF           rctPos;
    REAL            rTextStart, rTextAllowedLength;
    StringFormat    sf(StringFormatFlagsNoWrap, LANG_NEUTRAL);

    if (_pImgInfo) { delete _pImgInfo; _pImgInfo = NULL; }
    if (_pImgBlank) { delete _pImgBlank; _pImgBlank = NULL; }

    // Always transparent, no 'alternate' mode
    _pImgInfo = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgInfo);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);

    // Keep a backup of the 'blank' state image. We'll use that when there's no info to display
    _pImgBlank = _pImgInfo->Clone();

    // Legend Symbol
    ptStart.X = 0.0f; ptStart.Y = 0.0f;
    pGr->MeasureString(_wsLegend, -1, _pFontLegend, ptStart, &rctPos);
    ptStart.X = 0.0f; // Start from left-most
    ptStart.Y = (((REAL)_iHeight - rctPos.Height)/2.0f + _fIconScale);
    pGr->DrawString(_wsLegend, -1, _pFontLegend, ptStart, _pBrText);

    // Follow up text
    rTextStart = ((float)DEWUI_DIM_INFO_TEXT_PADDING * _fScale * _fIconScale);
    rTextAllowedLength = (REAL)_iWidth - rTextStart - _dwPadding * _fIconScale; // Take into account, the padding after the text ends
    ptStart.X = 0.0f; ptStart.Y = 0.0f;
    pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
    ptStart.X = rTextStart;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2;
    pGr->DrawString(_wsText, -1, _pFont, RectF(ptStart.X, ptStart.Y, rTextAllowedLength, rctPos.Height), &sf, _pBrText);

    // All done
    delete pGr;
    pGr = NULL;
}

LRESULT DEWINFOLABEL::_WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    HDC         hDC = NULL;
    PAINTSTRUCT ps { 0 };
    LRESULT     lrHit;
    RECT        rctClient { 0 };
    LPWINDOWPOS lpPos;

    switch(nMsg)
    {
        case WM_ERASEBKGND:
            return (LRESULT)NULL;

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

void DEWINFOLABEL::_Scale(BYTE btIconScale)
{
    float   fXPos;

    if (_btObjectType == DEWUI_OBJ_TYPE_NONE)
        return;

    _fIconScale = DEWUIOBJECT::_sfIconScalingIndex[btIconScale];

    fXPos = (float)DEWUI_DIM_PADDING_X * _fScale +
            ((float)DEWUI_DIM_ALBUM_ART * _fScale * _fIconScale) +
            10.0f * _fScale;

    _fDimension = 0.0f;
    _iWidth = (int)(((float)(DEWUI_DIM_WINDOW_X - DEWUI_DIM_PADDING_X)) * _fScale - fXPos);
    _iHeight = (int)(DEWUI_DIM_INFO_Y * _fScale * _fIconScale);
    return;
}

void DEWINFOLABEL::_MoveToDefaultLocation()
{
    int     iSeq = (int)_btInfoLabelType;
    float   fAlbumArt, fSeekBarTop, fAlbumArtTop, fAlbumArtBase, fPlayTop;
    float   fTitleBar, fHeight, fYPadding;

    fAlbumArt = ((float)DEWUI_DIM_ALBUM_ART * _fScale * _fIconScale);
    fSeekBarTop = ((DEWUI_DIM_WINDOW_Y_NOPL - DEWUI_DIM_PADDING_Y_FOOTER) * _fScale) -
                   (DEWUI_DIM_BUTTON * _fScale * _fIconScale + DEWUI_DIM_SEEKBAR_HEIGHT * _fScale) -
                   (10.0f / _fIconScale );
    fAlbumArtTop = (((fSeekBarTop - DEWUI_DIM_PADDING_Y_HEADER * _fScale) - fAlbumArt) / 2.0f + DEWUI_DIM_PADDING_Y_HEADER * _fScale);
    fAlbumArtBase = fAlbumArtTop + fAlbumArt;
    fPlayTop = fAlbumArtBase - DEWUI_DIM_PLAY_BUTTON * _fScale * _fIconScale - _F(1);
    fHeight = (DEWUI_DIM_INFO_Y * _fScale * _fIconScale);
    fTitleBar = ((DEWUI_DIM_SYSBUTTON + 1)* _fScale * _fIconScale);
    fYPadding = (fPlayTop - (max(fAlbumArtTop, fTitleBar)) - (fHeight * 3)) / 2.0f;

    _ptPos.x = _F(DEWUI_DIM_PADDING_X) + (int)fAlbumArt + _F(10);
    _ptPos.y = (int)((max(fAlbumArtTop, fTitleBar)) + (iSeq - 1) * (fYPadding + fHeight));

    this->_PrepareImages();
    SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, 0, 0, SWP_NOSIZE);
    this->_Draw(NULL);
    return;
}

DEWINFOLABEL::DEWINFOLABEL(LPDEWUIOBJECT Parent, const DEWTHEME& Theme, BYTE btInfoLabelType, LPCWSTR wsInfo)
{
    (DEWINFOLABEL::_snInstanceCount)++;

    _btObjectID = DEWUI_OBJ_NONE;
    _btObjectType = DEWUI_OBJ_TYPE_MODULE;
    _Parent = Parent;
    _wsLegend[0] = L'\0';
    _btInfoLabelType = btInfoLabelType;
    switch(_btInfoLabelType)
    {
        case DEWUI_INFOLABEL_TYPE_TITLE:
        {
            _btObjectID = DEWUI_OBJ_INFO_TITLE;
            StringCchPrintf(_wsLegend, 2, DEWUI_SYMBOL_SONG_TITLE);
            break;
        }
        case DEWUI_INFOLABEL_TYPE_ARTIST:
        {
            _btObjectID = DEWUI_OBJ_INFO_ARTIST;
            StringCchPrintf(_wsLegend, 2, DEWUI_SYMBOL_SONG_ARTIST);
            break;
        }
        case DEWUI_INFOLABEL_TYPE_ALBUM:
        {
            _btObjectID = DEWUI_OBJ_INFO_ALBUM;
            StringCchPrintf(_wsLegend, 2, DEWUI_SYMBOL_SONG_ALBUM);
            break;
        }
        default:
            _btInfoLabelType = DEWUI_INFOLABEL_TYPE_INVALID;
    }

    StringCchPrintf(_wsText, DEWUI_MAX_TITLE, wsInfo);
    _iDPI = Theme.DPI;
    _fScale = Theme.Scale;

    // First time instance
    if (!(DEWINFOLABEL::_sbClassRegistered))
    {
        WNDCLASSEX          wcex { 0 };

        StringCchPrintf((DEWINFOLABEL::_swsInfoLabelClass), 32, L"DEWDROP.INFOLABEL.WND");

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
        wcex.lpszClassName  = DEWINFOLABEL::_swsInfoLabelClass;

        RegisterClassEx(&wcex);

        DEWINFOLABEL::_sbClassRegistered = TRUE;
    }


    this->_Scale(Theme.IconScale);

    // Multiple instances would be created and these would be overridden anyways
    _ptPos.x = _F(0);
    _ptPos.y = _F(0);
    this->ApplyTheme(Theme, FALSE, FALSE);

    _hWnd = CreateWindowEx(0, DEWINFOLABEL::_swsInfoLabelClass, _wsText,
                           _INFOSTYLE, _ptPos.x, _ptPos.y,
                           _iWidth, _iHeight, _Parent->Handle, NULL, _hInstance, NULL);
    _hDC = GetDC(_hWnd);
    this->_CreateTooltip();
    this->_InitiateSubclassing();
    return;
}

void DEWINFOLABEL::ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw, BOOL bCalcScale)
{
    FontStyle   gdiFontStyle;

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

    CopyMemory(_wsFontName, Theme.TextFontStyle.FontName, 32*sizeof(wchar_t));
    _fIconScale = DEWUIOBJECT::_sfIconScalingIndex[Theme.IconScale];
    _iFontSize = (int)((REAL)(Theme.TextFontStyle.FontSize));
    _bBold = Theme.TextFontStyle.IsBold;
    _bItalic = Theme.TextFontStyle.IsItalic;
    if (_bBold)
        gdiFontStyle = _bItalic ? FontStyleBoldItalic : FontStyleBold;
    else
        gdiFontStyle = _bItalic ? FontStyleItalic : FontStyleRegular;

    if (_pFont) delete _pFont;
    _pFont = new Font(_wsFontName, (REAL)_iFontSize, gdiFontStyle, UnitPoint);

    if (_pFontLegend) delete _pFontLegend;
    _pFontLegend = new Font(_lpSymbolFont->SymbolFontGDIPlus,
                            ((float)DEWUI_FNTSPEC_INFO_SYMBOL) * _fIconScale,
                            FontStyleRegular, UnitPoint);

    _fDimension = 0.0f;
    _bAlternateIconMode = (Theme.IconMode == DEWTHEME_ICON_MODE_ALTERNATE);

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

void DEWINFOLABEL::SetInfo(LPCWSTR wsInfo, BOOL bForceDraw)
{
    if (!wsInfo || lstrlen(wsInfo) <= 0)
        StringCchPrintf(_wsText, DEWUI_MAX_TITLE, L"");
    else
        StringCchPrintf(_wsText, DEWUI_MAX_TITLE, L"%s", wsInfo);
    this->_PrepareImages();

    if (bForceDraw)
        this->_Draw(NULL);
    return;
}

void DEWINFOLABEL::ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue) {} // NOP

DEWINFOLABEL::~DEWINFOLABEL()
{
    if (_pImgInfo) { delete _pImgInfo; _pImgInfo = NULL; }
    if (_pImgBlank) { delete _pImgBlank; _pImgBlank = NULL; }
    if (_pFontLegend) { delete _pFontLegend; _pFontLegend = NULL; }
    (DEWINFOLABEL::_snInstanceCount)--;
    // Last instance removed. Time to de-register the class
    if (DEWINFOLABEL::_snInstanceCount == 0)
    {
        UnregisterClass(DEWINFOLABEL::_swsInfoLabelClass, _hInstance);
        DEWINFOLABEL::_sbClassRegistered = FALSE;
    }
    return;
}
