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

#include "DewButtonPlayPause.h"

void DEWPLAYPAUSEBUTTON::_UpdateImgPointers()
{
    switch(_btCurrState)
    {
        case DEWUI_STATE_PLAYING:
        {
            _pImgRaised = _pImgPauseRaised;
            _pImgSunken = _pImgPauseSunken;
            break;
        }
        case DEWUI_STATE_NOT_PLAYING:
        {
            _pImgRaised = _pImgPlayRaised;
            _pImgSunken = _pImgPlaySunken;
            break;
        }
        default:
        {
            _pImgRaised = NULL;
            _pImgSunken = NULL;
        }
    }
    return;
}

void DEWPLAYPAUSEBUTTON::_PrepareImages()
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
    if (!_bTransparentIcons)
        pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    if (!_bAlternateIconMode)
        pGr->DrawEllipse(_pPenOutline, 0, 0, _iWidth - _F(1), _iHeight - _F(1));
    pGr->MeasureString(DEWUI_SYMBOL_PLAY, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f +_fScale * 3.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 3.0f;
    if (_bAlternateIconMode)
        pGr->DrawString(DEWUI_SYMBOL_PLAY, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(DEWUI_SYMBOL_PLAY, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;

    _pImgPlaySunken = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgPlaySunken);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
        pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    if (!_bAlternateIconMode)
        pGr->DrawEllipse(_pPenOutline, 0, 0, _iWidth - _F(1), _iHeight - _F(1));
    pGr->MeasureString(DEWUI_SYMBOL_PLAY, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f +_fScale * 4.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 4.0f;
    if (_bAlternateIconMode)
        pGr->DrawString(DEWUI_SYMBOL_PLAY, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(DEWUI_SYMBOL_PLAY, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;
    // -------------------------------------------------------------------------


    // 'PAUSE' Images ----------------------------------------------------------
    _pImgPauseRaised = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgPauseRaised);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
        pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    if (!_bAlternateIconMode)
        pGr->DrawEllipse(_pPenOutline, 0, 0, _iWidth - _F(1), _iHeight - _F(1));
    pGr->MeasureString(DEWUI_SYMBOL_PAUSE, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f +_fScale * 1.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 2.0f;
    // Since these are rectangular bars, we don't want any soft edges!
    pGr->SetSmoothingMode(SmoothingModeHighSpeed);
    if (_bAlternateIconMode)
        pGr->DrawString(DEWUI_SYMBOL_PAUSE, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(DEWUI_SYMBOL_PAUSE, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;

    _pImgPauseSunken = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgPauseSunken);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
        pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    if (!_bAlternateIconMode)
        pGr->DrawEllipse(_pPenOutline, 0, 0, _iWidth - _F(1), _iHeight - _F(1));
    pGr->MeasureString(DEWUI_SYMBOL_PAUSE, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f +_fScale * 2.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 3.0f;
    // Since these are rectangular bars, we don't want any soft edges!
    pGr->SetSmoothingMode(SmoothingModeHighSpeed);
    if (_bAlternateIconMode)
        pGr->DrawString(DEWUI_SYMBOL_PAUSE, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(DEWUI_SYMBOL_PAUSE, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;
    // -------------------------------------------------------------------------

    this->_UpdateImgPointers();

    return;
}

void DEWPLAYPAUSEBUTTON::_MoveToDefaultLocation()
{
    int     iX, iY;
    float   fXGap, fAlbumArt, fPNS, fSeekBarTop, fAlbumArtBase;

    fAlbumArt = ((float)DEWUI_DIM_ALBUM_ART * _fScale * _fIconScale);
    fPNS = ((float)DEWUI_DIM_PNS_BUTTON * _fScale * _fIconScale);
    fXGap = ( ((DEWUI_DIM_WINDOW_X - DEWUI_DIM_PADDING_X * 2) * _fScale) -
              ((DEWUI_DIM_ALBUM_ART + DEWUI_DIM_PLAY_BUTTON + DEWUI_DIM_PNS_BUTTON * 3) * _fScale * _fIconScale) ) / 4.0f;
    fSeekBarTop = ((DEWUI_DIM_WINDOW_Y_NOPL - DEWUI_DIM_PADDING_Y_FOOTER) * _fScale) -
                   (DEWUI_DIM_BUTTON * _fScale * _fIconScale + DEWUI_DIM_SEEKBAR_HEIGHT * _fScale) -
                   (10.0f / _fIconScale);
    fAlbumArtBase = ( ((fSeekBarTop - DEWUI_DIM_PADDING_Y_HEADER * _fScale) - fAlbumArt) / 2.0f +
                      DEWUI_DIM_PADDING_Y_HEADER * _fScale + fAlbumArt );
    
    iX = (int)(DEWUI_DIM_PADDING_X * _fScale + fAlbumArt + fXGap + fPNS + fXGap);
    iY = (int)(fAlbumArtBase - _fDimension);
    _ptPos.x = iX;
    _ptPos.y = iY;
    this->_PrepareImages();
    SetWindowPos(_hWnd, 0, _ptPos.x, _ptPos.y, 0, 0, SWP_NOSIZE);
    this->_Draw(NULL);
    return;
}

void DEWPLAYPAUSEBUTTON::_Cleanup()
{
    if (_pImgPlayRaised) { delete _pImgPlayRaised; _pImgPlayRaised = NULL; }
    if (_pImgPlaySunken) { delete _pImgPlaySunken; _pImgPlaySunken = NULL; }
    if (_pImgPauseRaised) { delete _pImgPauseRaised; _pImgPauseRaised = NULL; }
    if (_pImgPauseSunken) { delete _pImgPauseSunken; _pImgPauseSunken = NULL; }
    return;
}

DEWPLAYPAUSEBUTTON::DEWPLAYPAUSEBUTTON() : State(_btCurrState), NextState(_btNextState) {} // NOP

DEWPLAYPAUSEBUTTON::DEWPLAYPAUSEBUTTON(LPDEWUIOBJECT Parent, const DEWTHEME& Theme) :
State(_btCurrState), NextState(_btNextState)
{
    RECT    rctClient;

    _btObjectID = DEWUI_OBJ_PLAY_PAUSE_BUTTON;
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

void DEWPLAYPAUSEBUTTON::SetState(BYTE btState)
{
    // Invalid or no change in state. NOP.
    if (!(btState == DEWUI_STATE_PLAYING || btState == DEWUI_STATE_NOT_PLAYING)
        || btState == _btCurrState)
        return;
    _btCurrState = btState;
    _btNextState = ((_btCurrState == DEWUI_STATE_PLAYING) ? DEWUI_STATE_NOT_PLAYING : DEWUI_STATE_PLAYING);
    this->_UpdateImgPointers();
    this->_Draw(NULL);
    return;
}

DEWPLAYPAUSEBUTTON::~DEWPLAYPAUSEBUTTON()
{
    this->_Cleanup();
    // These are mere pointers to the images. So once the actual ones are
    // deleted, 'NULL'ify these as well. Or else these will bomb in the
    // parent (DEWBUTTON) DTOR()!
    _pImgRaised = NULL;
    _pImgSunken = NULL;
    return;
}
