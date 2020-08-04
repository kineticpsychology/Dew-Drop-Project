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

#include "DewButtonNext.h"

void DEWNEXTBUTTON::_PrepareImages()
{
    Graphics        *pGr = NULL;
    RectF           rctPos;
    PointF          ptStart(0.0f, 0.0f);

    if (_pImgRaised) { delete _pImgRaised; _pImgRaised = NULL; }
    if (_pImgRaised) { delete _pImgSunken; _pImgSunken = NULL; }

    _pImgRaised = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgRaised);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
        pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    if (!_bAlternateIconMode)
        pGr->DrawEllipse(_pPenOutline, 0, 0, _iWidth - _F(1), _iHeight - _F(1));
    pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f +_fScale * 0.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 1.0f;
    if (_bAlternateIconMode)
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;

    ptStart.X = 0.0f; ptStart.Y = 0.0f;
    _pImgSunken = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgSunken);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
            pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    if (!_bAlternateIconMode)
        pGr->DrawEllipse(_pPenOutline, 0, 0, _iWidth - _F(1), _iHeight - _F(1));
    pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f + _fScale * 1.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 2.0f;
    if (_bAlternateIconMode)
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);

    delete pGr;
    pGr = NULL;

    return;
}

DEWNEXTBUTTON::DEWNEXTBUTTON(LPDEWUIOBJECT Parent, const DEWTHEME& Theme)
{
    RECT    rctClient;

    _btObjectID = DEWUI_OBJ_NEXT_BUTTON;
    _btObjectType = DEWUI_OBJ_TYPE_MMBUTTON;
    _Parent = Parent;
    StringCchPrintf(_wsText, DEWUI_MAX_TITLE, DEWUI_SYMBOL_NEXT);
    _iDPI = Theme.DPI;
    _fScale = Theme.Scale;

    this->_Scale(Theme.IconScale);
    GetClientRect((_Parent->Handle), &rctClient);
    _ptPos.x = 0;
    _ptPos.y = 0;
    this->ApplyTheme(Theme, FALSE, FALSE);

    _hWnd = CreateWindowEx(0, DEWBUTTON::_swsButtonClassName, _wsText, _BTNSTYLE, _ptPos.x, _ptPos.y,
                           _iWidth, _iHeight, _Parent->Handle, NULL, _hInstance, NULL);
    _hDC = GetDC(_hWnd);
    this->_CreateTooltip();
    this->_InitiateSubclassing();
    return;
}

void DEWNEXTBUTTON::_MoveToDefaultLocation()
{
    int     iX, iY;
    float   fXGap, fAlbumArt, fPlay, fPNS, fSeekBarTop, fAlbumArtBase;

    fAlbumArt = ((float)DEWUI_DIM_ALBUM_ART * _fScale * _fIconScale);
    fPNS = ((float)DEWUI_DIM_PNS_BUTTON * _fScale * _fIconScale);
    fXGap = ( ((DEWUI_DIM_WINDOW_X - DEWUI_DIM_PADDING_X * 2) * _fScale) -
              ((DEWUI_DIM_ALBUM_ART + DEWUI_DIM_PLAY_BUTTON + DEWUI_DIM_PNS_BUTTON * 3) * _fScale * _fIconScale) ) / 4.0f;
    fSeekBarTop = ((DEWUI_DIM_WINDOW_Y_NOPL - DEWUI_DIM_PADDING_Y_FOOTER) * _fScale) -
                   (DEWUI_DIM_BUTTON * _fScale * _fIconScale + DEWUI_DIM_SEEKBAR_HEIGHT * _fScale) -
                   (10.0f / _fIconScale);
    fAlbumArtBase = ( ((fSeekBarTop - DEWUI_DIM_PADDING_Y_HEADER * _fScale) - fAlbumArt) / 2.0f +
                      DEWUI_DIM_PADDING_Y_HEADER * _fScale + fAlbumArt );
    fPlay = ((float)DEWUI_DIM_PLAY_BUTTON * _fScale * _fIconScale);
    iX = (int)(DEWUI_DIM_PADDING_X * _fScale + fAlbumArt + fXGap + fPNS + fXGap + fPlay + fXGap);
    iY = (int)(fAlbumArtBase - fPlay + fPlay / 2 - _fDimension / 2);
    _ptPos.x = iX;
    _ptPos.y = iY;
    this->_PrepareImages();
    SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, 0, 0, SWP_NOSIZE);
    this->_Draw(NULL);
    return;
}

DEWNEXTBUTTON::~DEWNEXTBUTTON() {} // NOP
