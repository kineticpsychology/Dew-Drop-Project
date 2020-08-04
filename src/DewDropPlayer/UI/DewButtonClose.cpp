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

#include "DewButtonClose.h"

void DEWCLOSEBUTTON::_PrepareImages()
{
    Graphics        *pGr = NULL;
    RectF           rctPos;
    PointF          ptStart(0.0f, 0.0f);

    if (_pImgRaised) { delete _pImgRaised; _pImgRaised = NULL; }
    if (_pImgRaised) { delete _pImgSunken; _pImgSunken = NULL; }

    // This control will not respect transparency. It will always be transparent
    _pImgRaised = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgRaised);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f +_fScale * 0.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 0.0f;
    pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;

    ptStart.X = 0.0f; ptStart.Y = 0.0f;
    _pImgSunken = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgSunken);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f + _fScale * 1.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 1.0f;
    pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;

    return;
}

void DEWCLOSEBUTTON::_MoveToDefaultLocation()
{
    _ptPos.x = _F(DEWUI_DIM_WINDOW_X) - _iWidth - _F(1);
    _ptPos.y = _F(1);
    this->_PrepareImages();
    SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, 0, 0, SWP_NOSIZE);
    this->_Draw(NULL);
    return;
}

DEWCLOSEBUTTON::DEWCLOSEBUTTON(LPDEWUIOBJECT Parent, const DEWTHEME& Theme)
{
    RECT    rctClient;

    _btObjectID = DEWUI_OBJ_CLOSE_BUTTON;
    _btObjectType = DEWUI_OBJ_TYPE_UIBUTTON;
    _Parent = Parent;
    StringCchPrintf(_wsText, DEWUI_MAX_TITLE, DEWUI_SYMBOL_CLOSE);
    _iDPI = Theme.DPI;
    _fScale = Theme.Scale;

    this->_Scale(Theme.IconScale);
    GetClientRect((_Parent->Handle), &rctClient);
    _ptPos.x = (rctClient.right - rctClient.left) - _iWidth - _F(1);
    _ptPos.y = _F(1);
    this->ApplyTheme(Theme, FALSE, FALSE);

    _hWnd = CreateWindowEx(0, DEWBUTTON::_swsButtonClassName, _wsText, _BTNSTYLE, _ptPos.x, _ptPos.y,
                           _iWidth, _iHeight, _Parent->Handle, NULL, _hInstance, NULL);
    _hDC = GetDC(_hWnd);
    this->_CreateTooltip();
    this->_InitiateSubclassing();
    return;
}

DEWCLOSEBUTTON::~DEWCLOSEBUTTON() {} // NOP
