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

#include "DewButtonBrowse.h"

void DEWBROWSEBUTTON::_PrepareImages()
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
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 0.0f;
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
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 1.0f;
    if (_bAlternateIconMode)
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);

    delete pGr;
    pGr = NULL;

    return;
}

void DEWBROWSEBUTTON::_MoveToDefaultLocation()
{
    float   fSeq = 5.0f;
    float   fGap;

    fGap = (
        (((float)(DEWUI_DIM_WINDOW_X - DEWUI_DIM_PADDING_X * 2)) * _fScale)
        - (_fDimension * 7) // Seven buttons at the base row
        - (((float)DEWUI_DIM_VOL_BUTTON_X) * _fScale * _fIconScale) // + one volume button with its own expanding area
        ) / 7.0f;

    int     xOffset = _F((DEWUI_DIM_WINDOW_X - DEWUI_DIM_PADDING_X)) - (int)(((fSeq - 1.0f) * fGap) + (_fDimension * fSeq));
    int     yOffset = _F((DEWUI_DIM_WINDOW_Y_NOPL - DEWUI_DIM_PADDING_Y_FOOTER)) - (int)(((float)DEWUI_DIM_BUTTON) * _fScale * _fIconScale);

    _ptPos.x = xOffset;
    _ptPos.y = yOffset;
    this->_PrepareImages();
    SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, 0, 0, SWP_NOSIZE);
    this->_Draw(NULL);
    return;
}

DEWBROWSEBUTTON::DEWBROWSEBUTTON(LPDEWUIOBJECT Parent, const DEWTHEME& Theme)
{
    RECT    rctClient;

    _btObjectID = DEWUI_OBJ_BROWSE_BUTTON;
    _btObjectType = DEWUI_OBJ_TYPE_MMBUTTON;
    _Parent = Parent;
    StringCchPrintf(_wsText, DEWUI_MAX_TITLE, DEWUI_SYMBOL_BROWSE);
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

DEWBROWSEBUTTON::~DEWBROWSEBUTTON() {} // NOP

