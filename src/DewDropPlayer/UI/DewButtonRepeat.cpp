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

#include "DewButtonRepeat.h"

void DEWREPEATBUTTON::_UpdateImgPointers()
{
    switch(_btCurrState)
    {
        case DEWOPT_REPEAT_ALL:
        {
            _pImgRaised = _pImgRepeatAllRaised;
            _pImgSunken = _pImgRepeatAllSunken;
            this->SetTip(L"Repeating all songs");
            break;
        }
        case DEWOPT_REPEAT_ONE:
        {
            _pImgRaised = _pImgRepeatOneRaised;
            _pImgSunken = _pImgRepeatOneSunken;
            this->SetTip(L"Repeating currently playing song");
            break;
        }
        default:
        {
            _pImgRaised = _pImgRepeatOffRaised;
            _pImgSunken = _pImgRepeatOffSunken;
            this->SetTip(L"Repeat is OFF");
            break;
        }

    }
    return;
}

void DEWREPEATBUTTON::_ClickedEvent()
{
    this->SetNextState();
    if (_Parent)
        _Parent->ProcessMessage(this, DEWUI_MSG_CLICKED, 0, 0);
    return;
}

void DEWREPEATBUTTON::_PrepareImages()
{
    Graphics        *pGr = NULL;
    RectF           rctPos;
    PointF          ptStart(0.0f, 0.0f);
    PointF          ptStartOne(0.0f, 0.0f);
    Font            *pFontOne = NULL;
    Pen             *pPenBack = NULL;
    const wchar_t   *wsOne = L"1";

    this->_Cleanup();

    // REPEAT OFF Images -------------------------------------------------------
    pPenBack = new Pen(_crBack, (REAL)_F(1));
    _pImgRepeatOffRaised = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgRepeatOffRaised);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
        pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    GraphicsPath    pathCut;
    pathCut.AddArc(0, 0, _iWidth - _F(1), _iHeight - _F(1), 135, 180);
    pathCut.AddArc(0, 0, _iWidth - _F(1), _iHeight - _F(1), 135, -180);
    pGr->DrawPath((_bAlternateIconMode ? pPenBack : _pPenOutline), &pathCut);

    pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f +_fScale * 1.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 2.0f;
    if (_bAlternateIconMode)
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;

    ptStart.X = 0.0f; ptStart.Y = 0.0f;
    _pImgRepeatOffSunken = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgRepeatOffSunken);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
            pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    pGr->DrawPath((_bAlternateIconMode ? pPenBack : _pPenOutline), &pathCut);

    pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f + _fScale * 2.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 3.0f;
    if (_bAlternateIconMode)
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);
    delete pPenBack;
    pPenBack = NULL;
    // -------------------------------------------------------------------------


    // REPEAT ALL Images -------------------------------------------------------
    ptStart.X = 0.0f; ptStart.Y = 0.0f;
    _pImgRepeatAllRaised = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgRepeatAllRaised);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
        pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    if (!_bAlternateIconMode)
        pGr->DrawEllipse(_pPenOutline, 0, 0, _iWidth - _F(1), _iHeight - _F(1));

    pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f +_fScale * 1.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 2.0f;
    if (_bAlternateIconMode)
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);
    delete pGr;
    pGr = NULL;

    ptStart.X = 0.0f; ptStart.Y = 0.0f;
    _pImgRepeatAllSunken = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgRepeatAllSunken);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
            pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    if (!_bAlternateIconMode)
        pGr->DrawEllipse(_pPenOutline, 0, 0, _iWidth - _F(1), _iHeight - _F(1));
    pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f + _fScale * 2.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 3.0f;
    if (_bAlternateIconMode)
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrBack);
    else
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);
    // -------------------------------------------------------------------------


    // REPEAT ONE Images -------------------------------------------------------
    pFontOne = new Font(L"Tahoma", (REAL)_F(8), FontStyleBold, UnitPixel);
    _pImgRepeatOneRaised = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgRepeatOneRaised);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
        pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    if (!_bAlternateIconMode)
        pGr->DrawEllipse(_pPenOutline, 0, 0, _iWidth - _F(1), _iHeight - _F(1));

    ptStartOne.X = 0.0f; ptStartOne.Y = 0.0f;
    pGr->MeasureString(wsOne, -1, pFontOne, ptStartOne, &rctPos);
    ptStartOne.X = ((REAL)_iWidth - rctPos.Width)/2.0f +_fScale * 0.0f;
    ptStartOne.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 0.0f;

    ptStart.X = 0.0f; ptStart.Y = 0.0f;
    pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f +_fScale * 1.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 2.0f;
    if (_bAlternateIconMode)
    {
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrBack);
        pGr->DrawString(wsOne, -1, pFontOne, ptStartOne, _pBrBack);
    }
    else
    {
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);
        pGr->DrawString(wsOne, -1, pFontOne, ptStartOne, _pBrText);
    }

    _pImgRepeatOneSunken = new Bitmap(_iWidth, _iHeight, PixelFormat24bppRGB);
    pGr = Graphics::FromImage(_pImgRepeatOneSunken);
    pGr->SetSmoothingMode(SmoothingModeAntiAlias);
    pGr->DrawImage(DEWUIOBJECT::_pImgBackground, 0, 0, _ptPos.x, _ptPos.y, _iWidth, _iHeight, UnitPixel);
    if (!_bTransparentIcons)
            pGr->FillEllipse(_bAlternateIconMode ? _pBrText : _pBrBack, _F(0), _F(0), _iWidth - _F(1), _iHeight - _F(1));
    if (!_bAlternateIconMode)
        pGr->DrawEllipse(_pPenOutline, 0, 0, _iWidth - _F(1), _iHeight - _F(1));


    ptStartOne.X = 0.0f; ptStartOne.Y = 0.0f;
    pGr->MeasureString(wsOne, -1, pFontOne, ptStartOne, &rctPos);
    ptStartOne.X = ((REAL)_iWidth - rctPos.Width)/2.0f +_fScale * 1.0f;
    ptStartOne.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 1.0f;
    ptStart.X = 0.0f; ptStart.Y = 0.0f;
    pGr->MeasureString(_wsText, -1, _pFont, ptStart, &rctPos);
    ptStart.X = ((REAL)_iWidth - rctPos.Width)/2.0f + _fScale * 2.0f;
    ptStart.Y = ((REAL)_iHeight - rctPos.Height)/2.0f + _fScale * 3.0f;
    if (_bAlternateIconMode)
    {
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrBack);
        pGr->DrawString(wsOne, -1, pFontOne, ptStartOne, _pBrBack);
    }
    else
    {
        pGr->DrawString(_wsText, -1, _pFont, ptStart, _pBrText);
        pGr->DrawString(wsOne, -1, pFontOne, ptStartOne, _pBrText);
    }
    delete pFontOne;
    pFontOne = NULL;
    // -------------------------------------------------------------------------

    this->_UpdateImgPointers();

    delete pGr;
    pGr = NULL;

    return;
}

void DEWREPEATBUTTON::_MoveToDefaultLocation()
{
    float   fSeq = 7.0f;
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

void DEWREPEATBUTTON::_Cleanup()
{
    if (_pImgRepeatOffRaised) { delete _pImgRepeatOffRaised; _pImgRepeatOffRaised = NULL; }
    if (_pImgRepeatOffSunken) { delete _pImgRepeatOffSunken; _pImgRepeatOffSunken = NULL; }
    if (_pImgRepeatAllRaised) { delete _pImgRepeatAllRaised; _pImgRepeatAllRaised = NULL; }
    if (_pImgRepeatAllSunken) { delete _pImgRepeatAllSunken; _pImgRepeatAllSunken = NULL; }
    if (_pImgRepeatOneRaised) { delete _pImgRepeatOneRaised; _pImgRepeatOneRaised = NULL; }
    if (_pImgRepeatOneSunken) { delete _pImgRepeatOneSunken; _pImgRepeatOneSunken = NULL; }
}

DEWREPEATBUTTON::DEWREPEATBUTTON(LPDEWUIOBJECT Parent, const DEWTHEME& Theme, BYTE btInitState) :
State(_btCurrState), NextState(_btNextState)
{
    RECT    rctClient;

    _btObjectID = DEWUI_OBJ_REPEAT_BUTTON;
    _btObjectType = DEWUI_OBJ_TYPE_MMBUTTON;
    _Parent = Parent;
    StringCchPrintf(_wsText, DEWUI_MAX_TITLE, DEWUI_SYMBOL_REPEAT);
    _iDPI = Theme.DPI;
    _fScale = Theme.Scale;

    _btCurrState = btInitState;
    if (_btCurrState == DEWOPT_REPEAT_ONE)
        _btNextState = DEWOPT_REPEAT_OFF;
    else if (_btCurrState == DEWOPT_REPEAT_ALL)
        _btNextState = DEWOPT_REPEAT_ONE;
    else // Repeat OFF
        _btNextState = DEWOPT_REPEAT_ALL;

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

void DEWREPEATBUTTON::SetNextState()
{
    if (_btCurrState == DEWOPT_REPEAT_OFF)
    {
        _btCurrState = DEWOPT_REPEAT_ALL;
        _btNextState = DEWOPT_REPEAT_ONE;
    }
    else if (_btCurrState == DEWOPT_REPEAT_ALL)
    {
        _btCurrState = DEWOPT_REPEAT_ONE;
        _btNextState = DEWOPT_REPEAT_OFF;
    }
    else
    {
        _btCurrState = DEWOPT_REPEAT_OFF;
        _btNextState = DEWOPT_REPEAT_ALL;
    }
    this->_UpdateImgPointers();
    this->_Draw(NULL);
    return;
}

DEWREPEATBUTTON::~DEWREPEATBUTTON()
{
    this->_Cleanup();
    // These are mere pointers to the images. So once the actual ones are
    // deleted, 'NULL'ify these as well. Or else these will bomb in the
    // parent (DEWBUTTON) DTOR()!
    _pImgRaised = NULL;
    _pImgSunken = NULL;
    return;
}
