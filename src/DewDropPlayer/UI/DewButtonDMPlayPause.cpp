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

#include "DewButtonDMPlayPause.h"

void DEWDMPLAYPAUSEBUTTON::_PrepareImages()
{
    Graphics        *pGr = NULL;
    RectF           rctPos;
    PointF          ptStart(0.0f, 0.0f);

    this->_Cleanup();

    // 'PLAY' Images -----------------------------------------------------------
    _pImgPlayRaised = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgPlayRaised);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    pGr->MeasureString(DEWUI_SYMBOL_PLAY, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width) / 2.0f + _fScale * 0.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height) / 2.0f + _fScale * 1.0f;
    pGr->DrawString(DEWUI_SYMBOL_PLAY, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;

    _pImgPlaySunken = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgPlaySunken);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    pGr->MeasureString(DEWUI_SYMBOL_PLAY, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width) / 2.0f + _fScale * 1.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height) / 2.0f + _fScale * 2.0f;
    pGr->DrawString(DEWUI_SYMBOL_PLAY, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;
    // -------------------------------------------------------------------------


    // 'PAUSE' Images ----------------------------------------------------------
    _pImgPauseRaised = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgPauseRaised);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    pGr->MeasureString(DEWUI_SYMBOL_PAUSE, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width) / 2.0f + _fScale * 0.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height) / 2.0f + _fScale * 0.0f;
    // Since these are rectangular bars, we don't want any soft edges!
    pGr->SetSmoothingMode(SmoothingModeHighSpeed);
    pGr->DrawString(DEWUI_SYMBOL_PAUSE, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;

    _pImgPauseSunken = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgPauseSunken);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    pGr->MeasureString(DEWUI_SYMBOL_PAUSE, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width) / 2.0f + _fScale * 1.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height) / 2.0f + _fScale * 1.0f;
    // Since these are rectangular bars, we don't want any soft edges!
    pGr->SetSmoothingMode(SmoothingModeHighSpeed);
    pGr->DrawString(DEWUI_SYMBOL_PAUSE, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;
    // -------------------------------------------------------------------------

    this->_UpdateImgPointers();

    return;
}

void DEWDMPLAYPAUSEBUTTON::_MoveToDefaultLocation()
{
    _ptPos.x = _F(0);
    _ptPos.y = _F(0);
    this->_PrepareImages();
    SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, 0, 0, SWP_NOSIZE);
    this->_Draw(NULL);
    return;
}

DEWDMPLAYPAUSEBUTTON::DEWDMPLAYPAUSEBUTTON(LPDEWUIOBJECT Parent, const DEWTHEME& Theme)
{
     RECT    rctClient;

    _btObjectID = DEWUI_OBJ_DM_PLAY_PAUSE_BUTTON;
    _btObjectType = DEWUI_OBJ_TYPE_MMBUTTON;
    _Parent = Parent;
    StringCchPrintf(_wsText, DEWUI_MAX_TITLE, DEWUI_SYMBOL_PLAY);
    _iDPI = Theme.DPI;
    _fScale = Theme.Scale;

    _btCurrState = DEWUI_STATE_NOT_PLAYING;
    _btNextState = DEWUI_STATE_PLAYING;

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

DEWDMPLAYPAUSEBUTTON::~DEWDMPLAYPAUSEBUTTON() {} // NOP
