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

#include "DewAboutWindow.h"

#pragma region CREDITBLOCK Section

DEWABOUTWINDOW::CREDITBLOCK::CREDITBLOCK(HINSTANCE hInstance, HWND hWndParent,
                         int iDPI, float fScale, const WORD wResID, int iX, int iY,
                         LPCWSTR wsName, LPCWSTR wsURL, LPCWSTR wsAbout) :

                         Image(_pImg), ImgWidth(_iImgWidth), ImgHeight(_iImgHeight), ImgStart(_ptImgStart),
                         ConsumedHeight(_iConsumedHeight), StartY(_iStartY), EndY(_iEndY),
                         hWndName(_hWndName), hWndURL(_hWndURL), hWndAboutDescr(_hWndAboutDescr)
{
    RECT        rctClient;

    _iDPI = iDPI;
    _fScale = fScale;
    GetClientRect(hWndParent, &rctClient);

    if (wResID)
    {
        IStream         *pStrImg = NULL;
        HRESULT         hr = S_OK;
        HRSRC           hRsrc = NULL;
        DWORD           dwRCSize = 0x00;
        HGLOBAL         hgImg = NULL;
        LPBYTE          lpImgData;
        LARGE_INTEGER   liPos { 0 };

        hRsrc = FindResource(hInstance, MAKEINTRESOURCE(wResID), RT_RCDATA);
        if (hRsrc)
        {
            dwRCSize = SizeofResource(hInstance, hRsrc);
            if (dwRCSize > 0)
            {
                hgImg = LoadResource(hInstance, hRsrc);
                if (hgImg)
                {
                    lpImgData = (BYTE*)LockResource(hgImg);
                    if (lpImgData)
                    {
                        pStrImg = SHCreateMemStream(lpImgData, dwRCSize);
                        if (pStrImg)
                        {
                            hr = pStrImg->Seek(liPos, STREAM_SEEK_SET, NULL);
                            if (SUCCEEDED(hr))
                            {
                                Bitmap      *pBmpSrc = Bitmap::FromStream(pStrImg);

                                if (pBmpSrc)
                                {
                                    float   fImgScale;
                                    fImgScale = _fScale/2.0f;
                                    _pImg = new Bitmap((int)((float)(pBmpSrc->GetWidth()) * fImgScale),
                                                       (int)((float)(pBmpSrc->GetHeight()) * fImgScale),
                                                       PixelFormat24bppRGB);
                                    if (_pImg)
                                    {
                                        Graphics    *pGr;
                                        pGr = Graphics::FromImage(_pImg);
                                        pGr->ScaleTransform(fImgScale/_fScale, fImgScale/_fScale);
                                        pGr->DrawImage(pBmpSrc, 0, 0,
                                            0, 0,
                                            (int)pBmpSrc->GetWidth(),
                                            (int)pBmpSrc->GetHeight(), UnitPixel);

                                        _iImgWidth = _pImg->GetWidth();
                                        _iImgHeight = _pImg->GetHeight();

                                        delete pGr;
                                    } // if (_pImg)

                                    delete pBmpSrc;
                                } // if (pBmpSrc)
                            } // pStrImg->Seek (hr == S_OK)

                            pStrImg->Release();
                            pStrImg = NULL;
                        } // if (pStrImg)
                    } // if (lpImgData)
                } // if (hgImg)
            } // if (dwRCSize > 0)
        } // if (hRsrc)
    } // if (wResID)

    // Arranged in order: Header -> Image -> URL -> About/Description
    _iStartY = iY;
    _iConsumedHeight = 0;
    _hWndName = CreateWindowEx(0, L"STATIC", wsName, _LBLSTYLE | SS_CENTER, iX, iY + _iConsumedHeight, rctClient.right - iX * 2, _F(_CREDIT_CTL_HEIGHT), hWndParent, NULL, hInstance, NULL);
    _iConsumedHeight += _F(_CREDIT_CTL_HEIGHT);

    _ptImgStart.X = (rctClient.right - _iImgWidth) / 2;
    _ptImgStart.Y = _iStartY + _iConsumedHeight;
    _iConsumedHeight += _iImgHeight;

    _hWndURL = CreateWindowEx(0, L"STATIC", wsURL, _URLSTYLE, iX, iY + _iConsumedHeight, rctClient.right - iX * 2, _F(_CREDIT_CTL_HEIGHT), hWndParent, NULL, hInstance, NULL);
    _iConsumedHeight += _F(_CREDIT_CTL_HEIGHT);
    _hWndAboutDescr = CreateWindowEx(0, L"STATIC", wsAbout, _LBLSTYLE, iX, iY + _iConsumedHeight, rctClient.right - iX * 2, _F(_CREDIT_CTL_ABOUT_HEIGHT), hWndParent, NULL, hInstance, NULL);
    _iConsumedHeight += _F(_CREDIT_CTL_ABOUT_HEIGHT);

    SetClassLongPtr(_hWndURL, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));

    _iConsumedHeight += _F(20); // A small padding at the end for the next credit block
    _iEndY = _iConsumedHeight;
    return;
}

DEWABOUTWINDOW::CREDITBLOCK::~CREDITBLOCK()
{
    if (_pImg) { delete _pImg; _pImg = NULL; }
    DestroyWindow(_hWndName);
    DestroyWindow(_hWndURL);
    DestroyWindow(_hWndAboutDescr);
    return;
}

#pragma endregion

#pragma region Credits Window Section

LRESULT CALLBACK DEWABOUTWINDOW::_CreditsMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    return ((LPDEWABOUTWINDOW)dwRefData)->_CreditsWndProc(hWnd, nMsg, wParam, lParam);
}

LRESULT DEWABOUTWINDOW::_CreditsWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT     ps { 0 };
    SCROLLINFO      si { 0 };
    int             iYPos, iDelta, iIndex, iHeight;
    UINT            nIndex;
    RECT            rctClient;
    wchar_t         wsURL[DEWUI_MAX_TITLE];

    switch(nMsg)
    {
        case WM_PAINT:
        {
            BeginPaint(hWnd, &ps);
            this->_DrawCredits();
            EndPaint(hWnd, &ps);
            return FALSE;
        }
        case WM_CTLCOLORSTATIC:
        {
            SetBkMode((HDC)wParam, TRANSPARENT);
            for (nIndex = 0; nIndex < _CREDIT_COUNT; nIndex++)
            {
                if (_plpCredits[nIndex]->hWndURL == (HWND)lParam)
                {
                    SetTextColor((HDC)wParam, RGB(0x00, 0x00, 0xFF));
                    return (LRESULT)_hbrCreditsBack;
                }
                else if (_plpCredits[nIndex]->hWndName == (HWND)lParam)
                {
                    return (LRESULT)_hbrTitleBack;
                }
            }
            SetTextColor((HDC)wParam, RGB(0x00, 0x00, 0x00));
            return (LRESULT)_hbrCreditsBack;
        }
        case WM_SIZE:
        {
            this->_HandleCreditsSizing();
            return FALSE;
        }
        case WM_VSCROLL:
        {
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            GetScrollInfo(hWnd, SB_VERT, &si);
            iYPos = si.nPos;
            switch (LOWORD (wParam))
            {
                case SB_TOP: { si.nPos = si.nMin; break; } // User clicked the HOME keyboard key.
                case SB_BOTTOM: { si.nPos = si.nMax;  break; } // User clicked the END keyboard key.
                case SB_LINEUP: { si.nPos -= _F(1); break; } // User clicked the top arrow.
                case SB_LINEDOWN: { si.nPos += _F(1); break; } // User clicked the bottom arrow.
                case SB_PAGEUP: { si.nPos -= si.nPage; break; } // User clicked the scroll bar shaft above the scroll box.
                case SB_PAGEDOWN: { si.nPos += si.nPage; break; } // User clicked the scroll bar shaft below the scroll box.
                case SB_THUMBPOSITION: // User dragged the scroll box.
                case SB_THUMBTRACK: // User is dragging the scroll box.
                { si.nPos = HIWORD(wParam); break; }
                default: break;
            }
            si.fMask = SIF_POS;
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
            GetScrollInfo(hWnd, SB_VERT, &si);
            ScrollWindow(hWnd, 0, iYPos - si.nPos, NULL, NULL);
            return FALSE;
        }
        case WM_MOUSEWHEEL:
        {
            iDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_POS;
            GetScrollInfo(_hWndCredits, SB_VERT, &si);

            if (iDelta > 0) // Mouse wheel scrolled upwards (away from user). Pull page DOWNWARDS (like hitting PAGE-UP).
            {
                if (si.nPos <= _plpCredits[0]->StartY) // No 'Credit' section is remaining above si.nPos. Scroll to the top
                    iYPos = 0;
                else
                {
                    iYPos = si.nPos;
                    // Catch the last 'Credit' section above si.nPos and scroll to _F(10) pizels above that
                    for (iIndex = _CREDIT_COUNT - 1; iIndex >= 0; iIndex--)
                    {
                        if (_plpCredits[iIndex]->StartY < si.nPos)
                        {
                            iYPos = _plpCredits[iIndex]->StartY - _F(10);
                            break;
                        }
                    }
                }
                SendMessage(_hWndCredits, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, iYPos), NULL);
            }
            else if (iDelta < 0) // Mouse wheel scrolled downwards (towards user). Pull page UPWARDS (like hitting PAGE-DOWN).
            {
                BOOL bBeyondLastCrediBlock = FALSE;
                GetClientRect(hWnd, &rctClient);
                iHeight = rctClient.bottom - rctClient.top;
                if (si.nPos + iHeight >= _iCreditsMaxHeight) // Reached the end of page. No more scroll.
                    return FALSE;
                iYPos = si.nPos;
                // Catch the first 'Credit' Section, which is more than _F(10) pixels away from si.nPos
                // and that section's StartY, added with client area height, will be contained within _iCreditsMaxHeight
                for (iIndex = 0; iIndex < (int)_CREDIT_COUNT; iIndex++)
                {
                    if (_plpCredits[iIndex]->StartY + iHeight <= _iCreditsMaxHeight &&
                        _plpCredits[iIndex]->StartY > si.nPos + _F(10))
                    {
                        iYPos = _plpCredits[iIndex]->StartY - _F(10);
                        bBeyondLastCrediBlock = TRUE;
                        break;
                    }
                }
                // si.nPos is at a position where no further 'Credit' block can meet the above criteria
                // but si.nPos + iHeight is still falling short of _iCreditsMaxHeight.
                // In that case, just offset si.nPos, so that si.nPos + iHeight exactly matches _iCreditsMaxHeight
                if (!bBeyondLastCrediBlock)
                {
                    iYPos = _iCreditsMaxHeight - iHeight;
                }
                SendMessage(_hWndCredits, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, iYPos), NULL);
            }

            return FALSE;
        }
        case WM_COMMAND:
        {
            for (nIndex = 0; nIndex < _CREDIT_COUNT; nIndex++)
            {
                if (_plpCredits[nIndex]->hWndURL == (HWND)lParam)
                {
                    GetWindowText((_plpCredits[nIndex]->hWndURL), wsURL, DEWUI_MAX_TITLE);
                    ShellExecute(NULL, L"open", wsURL, NULL, NULL, SW_SHOWDEFAULT);
                    return FALSE;
                }
            }
            break;
        }
    }
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWABOUTWINDOW::_HandleCreditsSizing()
{
    static int      iYPos, iHeight;
    RECT            rctClient { 0 };
    SCROLLINFO      si { 0 };

    GetClientRect(_hWndCredits, &rctClient);
    iHeight = rctClient.bottom - rctClient.top;
    si.cbSize = sizeof(SCROLLINFO);

    si.fMask = SIF_POS | SIF_RANGE;
    GetScrollInfo(_hWndCredits, SB_VERT, &si);
    iYPos = si.nPos;
    iHeight = rctClient.bottom - rctClient.top;

    si.fMask = SIF_ALL;
    si.nMin = 0;
    si.nPage = iHeight;
    si.nMax = _iCreditsMaxHeight;
    si.nPos = iYPos;
    si.nTrackPos = iYPos;
    SetScrollInfo(_hWndCredits, SB_VERT, &si, TRUE);
    GetScrollInfo(_hWndCredits, SB_VERT, &si);
    ScrollWindow(_hWndCredits, 0, iYPos - si.nPos, NULL, NULL);
    return;
}

void DEWABOUTWINDOW::_DrawCredits()
{
    RECT        rctClient;
    SCROLLINFO  si{ 0 };
    int         iYPos;
    UINT        nIndex;
    HGDIOBJ     hObjOld = NULL;

    GetClientRect(_hWndCredits, &rctClient);
    FillRect(_hDCCredits, &rctClient, _hbrCreditsBack);

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS;
    GetScrollInfo(_hWndCredits, SB_VERT, &si);

    // Draw the scrollable 'Thank You' image at the top
    if (_pImgThanks &&
        si.nPos <= (int)(_pImgThanks->GetHeight()))
    {
        iYPos = 0 - si.nPos;
        BitBlt(_hDCCredits, 0, iYPos, rctClient.right - rctClient.top, _pImgThanks->GetHeight(), _hDCCreditsMem, 0, 0, SRCCOPY);
    }

    // Loop through each Credit block
    for (nIndex = 0; nIndex < _CREDIT_COUNT; nIndex++)
    {
        // Check if an image is applicable
        if (_plpCredits[nIndex]->Image)
        {
            // Calculate the position in the viewable area
            // where the image will be drawn
            iYPos = _plpCredits[nIndex]->ImgStart.Y - si.nPos;

            // Make sure that the image is in the viewable area
            // Don't spend CPU in drawing unnecessarily
            if (iYPos < rctClient.bottom &&
                iYPos + (_plpCredits[nIndex]->ImgHeight) > 0)
            {
                BitBlt(_hDCCredits,
                       _plpCredits[nIndex]->ImgStart.X, iYPos,
                       _plpCredits[nIndex]->ImgWidth, _plpCredits[nIndex]->ImgHeight,
                       _hDCCreditsMem,
                       _plpCredits[nIndex]->ImgStart.X,
                       _plpCredits[nIndex]->ImgStart.Y,
                       SRCCOPY);
            }
        }
    }

    return;
}

#pragma endregion

#pragma region About Window Section

DWORD WINAPI DEWABOUTWINDOW::_TrdAnimateLogo(LPVOID lpv)
{
    ((LPDEWABOUTWINDOW)lpv)->_AnimateLogo();
    return 0;
}

void DEWABOUTWINDOW::_Animation_Beat(DWORD dwDelay)
{
    Graphics        *pGr = NULL;
    Blur            blur;
    BlurParams      blurParams;
    float           fValue = 0.0f;
    float           fDir = 1.0f;
    const float     fIncr = 1.0f;
    const int       iIconDim = _F(_ABOUT_ICON_SIZE);
    RECT            rctBack = { 0, 0, iIconDim, iIconDim };

    do
    {
        blurParams.radius = fValue;
        blurParams.expandEdge = TRUE;
        blur.SetParameters(&blurParams);
        
        // Perform the GDI operations only when the window is open
        FillRect(_hDCIcoMem, &rctBack, _hbrAboutBack);
        if (_pImgAbout)
        {
            pGr = new Graphics(_hDCIcoMem);
            if (pGr)
            {
                Bitmap *pImgEffect = _pImgAbout->Clone(0, 0, iIconDim, iIconDim, _pImgAbout->GetPixelFormat());
                if (pImgEffect)
                {
                    pImgEffect->ApplyEffect(&blur, NULL);
                    pGr->DrawImage(pImgEffect, 0, 0);
                    delete pImgEffect;
                    pImgEffect = NULL;
                }
                delete pGr;
                pGr = NULL;
            }
        }
        BitBlt(_hDC, _iIconXPos, _iIconYPos, iIconDim, iIconDim, _hDCIcoMem, 0, 0, SRCCOPY);
        fValue += (fDir*fIncr);
        if (fValue >= 10.0f) fDir = -1.0f;
        if (fValue <= 0.0f) fDir = 1.0f;
        Sleep(dwDelay);
 
   // Perform the GDI operations till the window is open
    } while (IsWindowVisible(_hWndAbout));

    return;
}

void DEWABOUTWINDOW::_Animation_Oscillate(DWORD dwDelay)
{
    const int       iIconDim = _F(_ABOUT_ICON_SIZE);
    const PointF    ptCenter((REAL)iIconDim / 2.0f, (REAL)iIconDim / 2.0f);
    RECT            rctBack = { 0, 0, iIconDim, iIconDim };
    REAL            rAngle = 0.0f;
    Graphics        *pGr = NULL;
    float           fDir = 1.0f;
    const float     fIncr = 0.5f;

    do
    {
        FillRect(_hDCIcoMem, &rctBack, _hbrAboutBack);
        if (_pImgAbout)
        {
            pGr = new Graphics(_hDCIcoMem);
            if (pGr)
            {
                Matrix  matrix;

                matrix.RotateAt(rAngle, ptCenter);
                pGr->SetTransform(&matrix);
                pGr->DrawImage(_pImgAbout, 0, 0);
                delete pGr;
                pGr = NULL;
            }
        }
        BitBlt(_hDC, _iIconXPos, _iIconYPos, iIconDim, iIconDim, _hDCIcoMem, 0, 0, SRCCOPY);

        rAngle += (fDir*fIncr);
        if (rAngle >= 5.0f) fDir = -1;
        if (rAngle <= -5.0f) fDir = 1;

        Sleep(dwDelay);

    // Perform the GDI operations till the window is open
    } while (IsWindowVisible(_hWndAbout));

    return;
}

void DEWABOUTWINDOW::_Animation_Fade(DWORD dwDelay)
{
    const int       iIconDim = _F(_ABOUT_ICON_SIZE);
    RECT            rctBack = { 0, 0, iIconDim, iIconDim };
    REAL            rAlpha = 0.0f;
    Rect            rctDest(0, 0, iIconDim, iIconDim);
    float           fDir = 1.0;
    Graphics        *pGr = NULL;
    const float     fIncr = 0.0625f;

    do
    {
        ColorMatrix colorMatrix = { 1.0f, 0.0f, 0.0f, 0.0f,   0.0f,
                                    0.0f, 1.0f, 0.0f, 0.0f,   0.0f,
                                    0.0f, 0.0f, 1.0f, 0.0f,   0.0f,
                                    0.0f, 0.0f, 0.0f, rAlpha, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f,   1.0f };
        
        FillRect(_hDCIcoMem, &rctBack, _hbrAboutBack);
        if (_pImgAbout)
        {
            pGr = new Graphics(_hDCIcoMem);
            if (pGr)
            {
                ImageAttributes imgAttr;

                imgAttr.SetColorMatrix(&colorMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
                pGr->DrawImage(_pImgAbout, rctDest, 0, 0, iIconDim, iIconDim, UnitPixel, &imgAttr);
                delete pGr;
                pGr = NULL;
            }
        }
        BitBlt(_hDC, _iIconXPos, _iIconYPos, iIconDim, iIconDim, _hDCIcoMem, 0, 0, SRCCOPY);

        rAlpha += (fDir*fIncr);
        if (rAlpha >= 1.0f) fDir = -1;
        if (rAlpha <= 0.0f) fDir = 1;

        Sleep(dwDelay);

    // Perform the GDI operations till the window is open
    } while (IsWindowVisible(_hWndAbout));

    return;
}

void DEWABOUTWINDOW::_Animation_Tint(DWORD dwDelay)
{
    const int       iIconDim = _F(_ABOUT_ICON_SIZE);
    RECT            rctBack = { 0, 0, iIconDim, iIconDim };
    Graphics        *pGr = NULL;
    float           fR = 1.0f, fG = 1.0f, fB = 1.0f;
    float           fRDir = 1.0f, fGDir = 1.0f, fBDir = 1.0f;
    const float     fRAmt = 0.093f;
    const float     fGAmt = 0.083f;
    const float     fBAmt = 0.057f;
    Rect            rctDest(0, 0, iIconDim, iIconDim);

    do
    {
        ColorMatrix colorMatrix = { fR,   0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, fG,   0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, fB,   0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

        FillRect(_hDCIcoMem, &rctBack, _hbrAboutBack);
        if (_pImgAbout)
        {
            pGr = new Graphics(_hDCIcoMem);
            if (pGr)
            {
                ImageAttributes imgAttr;

                imgAttr.SetColorMatrix(&colorMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
                pGr->DrawImage(_pImgAbout, rctDest, 0, 0, iIconDim, iIconDim, UnitPixel, &imgAttr);
                delete pGr;
                pGr = NULL;
            }
        }
        BitBlt(_hDC, _iIconXPos, _iIconYPos, iIconDim, iIconDim, _hDCIcoMem, 0, 0, SRCCOPY);

        fR += (fRDir*fRAmt);
        if (fR >= 1.0f) fRDir = -1;
        if (fR <= 0.0f) fRDir = 1;

        fG += (fGDir*fGAmt);
        if (fG >= 1.0f) fGDir = -1;
        if (fG <= 0.0f) fGDir = 1;

        fB += (fBDir*fBAmt);
        if (fB >= 1.0f) fBDir = -1;
        if (fB <= 0.0f) fBDir = 1;

        Sleep(dwDelay);

    // Perform the GDI operations till the window is open
    } while (IsWindowVisible(_hWndAbout));

    return;
}

void DEWABOUTWINDOW::_AnimateLogo()
{
    HBITMAP     hBmp = NULL;
    HGDIOBJ     hObjOld = NULL;
    int         iIconDim = _F(_ABOUT_ICON_SIZE);
    const DWORD dwAnimationDelay = 50;
    int         iIndex;

    srand((UINT)time(0));
    iIndex = (rand()%4); // Pick one randome animation function everytime the thread starts
    
    hBmp = CreateCompatibleBitmap(_hDC, iIconDim, iIconDim);
    hObjOld = SelectObject(_hDCIcoMem, hBmp); // Smaller means faster

    (this->*_Animations[iIndex])(dwAnimationDelay);

    SelectObject(_hDCIcoMem, hObjOld);
    DeleteObject(hBmp);
    hBmp = NULL;
    _hTrdLogoAnimation = NULL;

    return;
}

DWORD WINAPI DEWABOUTWINDOW:: _TrdScrollNames(LPVOID lpv)
{
    ((LPDEWABOUTWINDOW)lpv)->_Scroll_Names();
    return 0x00;
}

void DEWABOUTWINDOW::_Scroll_Names()
{
    Graphics    *pGr = NULL;
    Font        *pFntUI = NULL;
    PointF      ptStart(0.0f, 0.0f);
    RectF       rctBounds;
    int         iTAWidth, iTAHeight, iScrollSpot = 0;
    HGDIOBJ     hObjOld;

    // Get the designated Text Area Width & Height
    iTAWidth = _rctAboutTextArea.right - _rctAboutTextArea.left;
    iTAHeight = _rctAboutTextArea.bottom - _rctAboutTextArea.top;

    // Derive the DC and calculate the dimensions of the about text
    pGr = Graphics::FromHDC(_hDCScrollMem);
    pFntUI = new Font(_hDCScrollMem, _hFntUI);
    pGr->MeasureString(_wsAboutText, -1, pFntUI, ptStart, &rctBounds);
    delete pGr; pGr = NULL;
    delete pFntUI; pFntUI = NULL;

    // Formulate the bounding rectangle for drawing the text
    // Since we will auto-center anyways, we don't actually need the image's width
    // e.g. (INT)roundf(ceil(rctBounds.Width)). We just care about the height of the bitmap
    RECT    rctMemDCTextArea = { 0, 0, iTAWidth, (LONG)roundf(ceilf(rctBounds.Height)) };
    RECT    rctShade = { 0, 0, iTAWidth, (LONG)roundf(ceilf(rctBounds.Height)) };
    
    // Set the color, font & background
    hObjOld = SelectObject(_hDCScrollMem, _hFntUI);
    SelectObject(_hDCScrollMem, _hbrAboutBack);
    SetTextColor(_hDCScrollMem, _crModuleText);
    SetBkMode(_hDCScrollMem, TRANSPARENT);
    
    iScrollSpot = iTAHeight;

    do
    {

        // Fill out the Mem DC background with the similar size as _rct
        FillRect(_hDCScrollMem, &rctShade, _hbrAboutBack);
        
        // But Draw the offset-ed text
        rctMemDCTextArea.top = rctShade.top + iScrollSpot;
        DrawText(_hDCScrollMem, _wsAboutText, -1, &rctMemDCTextArea, DT_CENTER);

        BitBlt(_hDC,
               _rctAboutTextArea.left, _rctAboutTextArea.top, iTAWidth,iTAHeight,
               _hDCScrollMem, 0, 0, SRCCOPY);

        iScrollSpot--;
        if (iScrollSpot <= -rctShade.bottom) iScrollSpot = iTAHeight;
        Sleep(50);

    } while (IsWindowVisible(_hWndAbout));

    SelectObject(_hDCScrollMem, hObjOld);


    return;
}


LRESULT CALLBACK DEWABOUTWINDOW::_AboutMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData)
{
    return ((LPDEWABOUTWINDOW)dwRefData)->_AboutWndProc(hWnd, nMsg, wParam, lParam);
}

void DEWABOUTWINDOW::_ResetGDIObjects()
{
    // GDI Brushes
    if (_hbrAboutBack) { DeleteObject(_hbrAboutBack); _hbrAboutBack = NULL; }
    if (_hbrBtnBack) { DeleteObject(_hbrBtnBack); _hbrBtnBack = NULL; }
    if (_hbrCreditsBack) { DeleteObject(_hbrCreditsBack); _hbrCreditsBack = NULL; }
    if (_hbrTitleBack) { DeleteObject(_hbrTitleBack); _hbrTitleBack = NULL; }

    // GDI Pens
    if (_hPenBtnFocus) { DeleteObject(_hPenBtnFocus); _hPenBtnFocus = NULL; }
    if (_hPenBtnBorder) { DeleteObject(_hPenBtnBorder); _hPenBtnBorder = NULL; }
    if (_hPenModuleBorder) { DeleteObject(_hPenModuleBorder); _hPenModuleBorder = NULL; }
    if (_hPenWndBorder) { DeleteObject(_hPenWndBorder); _hPenWndBorder = NULL; }

    return;
}

void DEWABOUTWINDOW::_CreateThanksImage()
{
    IStream         *pStrImg = NULL;
    HRESULT         hr = S_OK;
    HRSRC           hRsrc = NULL;
    DWORD           dwRCSize = 0x00;
    HGLOBAL         hgImg = NULL;
    LPBYTE          lpImgData;
    LARGE_INTEGER   liPos { 0 };

    hRsrc = FindResource(_hInstance, MAKEINTRESOURCE(IDIMG_THANKS), RT_RCDATA);
    if (hRsrc)
    {
        dwRCSize = SizeofResource(_hInstance, hRsrc);
        if (dwRCSize > 0)
        {
            hgImg = LoadResource(_hInstance, hRsrc);
            if (hgImg)
            {
                lpImgData = (BYTE*)LockResource(hgImg);
                if (lpImgData)
                {
                    pStrImg = SHCreateMemStream(lpImgData, dwRCSize);
                    if (pStrImg)
                    {
                        hr = pStrImg->Seek(liPos, STREAM_SEEK_SET, NULL);
                        if (SUCCEEDED(hr))
                        {
                            Bitmap      *pBmpSrc = Bitmap::FromStream(pStrImg);

                            if (pBmpSrc)
                            {
                                float   fImgScale;

                                fImgScale = ((float)_THANKSHEIGHT * _fScale) / ((float)(pBmpSrc->GetHeight()));
                                _pImgThanks = new Bitmap((int)((float)(pBmpSrc->GetWidth()) * fImgScale),
                                                         (int)((float)(pBmpSrc->GetHeight()) * fImgScale),
                                                         PixelFormat24bppRGB);
                                if (_pImgThanks)
                                {
                                    Graphics    *pGr;
                                    pGr = Graphics::FromImage(_pImgThanks);
                                    pGr->ScaleTransform(fImgScale/_fScale, fImgScale/_fScale);
                                    pGr->DrawImage(pBmpSrc, 0, 0);
                                    delete pGr;
                                } // if (_pImgThanks)

                                delete pBmpSrc;
                            } // if (pBmpSrc)
                        } // pStrImg->Seek (hr == S_OK)

                        pStrImg->Release();
                        pStrImg = NULL;
                    } // if (pStrImg)
                } // if (lpImgData)
            } // if (hgImg)
        } // if (dwRCSize > 0)
    } // if (hRsrc)

    return;
}

void DEWABOUTWINDOW::_CreateAboutContent()
{
    RECT                rctClient;
    wchar_t             wsProcessImgFile[MAX_CHAR_PATH]{ 0 };
    DWORD               dwVISize = 0, dwStub = 0;
    UINT                nFFInfoSize = 0;
    LPBYTE              lpVI;
    VS_FIXEDFILEINFO    *pffInfo = NULL;
    WORD                wAppMajorMS = 0, wAppMinorMS = 0, wAppMajorLS = 0, wAppMinorLS = 0;
    WORD                wThemeMajor = 0, wThemeMinor = 0;

    
    // ABOUT-TEXT Preparation --------------------------------------------------
    GetClientRect(_hWndAbout, &rctClient);

    _rctAboutTitleArea.left = rctClient.left + _F(10) + _F(_ABOUT_ICON_SIZE);
    _rctAboutTitleArea.top = _F(10);
    _rctAboutTitleArea.right = rctClient.right - _F(10);
    _rctAboutTitleArea.bottom = _F(50);

    _rctAboutTextArea.left = _rctAboutTitleArea.left;
    _rctAboutTextArea.top = _rctAboutTitleArea.bottom + _F(10);
    _rctAboutTextArea.right = _rctAboutTitleArea.right;
    _rctAboutTextArea.bottom = _rctAboutTextArea.top + _F(70);

    GetModuleFileName(NULL, wsProcessImgFile, MAX_CHAR_PATH);
    dwVISize = GetFileVersionInfoSize(wsProcessImgFile, &dwStub);
    if (dwVISize > 0)
    {
        lpVI = (LPBYTE)LocalAlloc(LPTR, dwVISize);
        if (GetFileVersionInfo(wsProcessImgFile, 0, dwVISize, lpVI))
        {
            if (VerQueryValue(lpVI, L"\\", (LPVOID*)&pffInfo, &nFFInfoSize))
            {
                wAppMajorMS = HIWORD((pffInfo->dwProductVersionMS));
                wAppMinorMS = LOWORD((pffInfo->dwProductVersionMS));
                wAppMajorLS = HIWORD((pffInfo->dwProductVersionLS));
                wAppMinorLS = LOWORD((pffInfo->dwProductVersionLS));

                wThemeMajor = HIWORD((pffInfo->dwFileVersionMS));
                wThemeMinor = LOWORD((pffInfo->dwFileVersionMS));
            }
        }
        LocalFree(lpVI);
    }

    StringCchPrintf(_wsAboutText, MAX_CHAR_PATH,
                    L"[Versions]\n--------------------\n" \
                    L"Player Version: %u.%u.%u.%u\n" \
                    L"Theme Version: %u.%u\n\n\n" \
                    
                    L"[Release]\n--------------------\n" \
                    L"April, 2020\n\n\n" \

                    L"[Toolkits]\n--------------------\n" \
                    L"Microsoft VC++\n" \
                    L"Win32 API\n" \
                    L"GDI+\n\n\n" \

                    L"[License]\n--------------------\n" \
                    L"Purely open source\n\n\n" \
                    
                    L"[Designs]\n--------------------\n" \
                    L"UI Designs && Architecture: Alone\n" \
                    L"Audio Engine Design: Alone\n" \
                    L"[Polash.Majumdar@gmail.com]\n\n\n" \

                    L"[Special Thanks]\n--------------------\n" \
                    L"Sonali Majumdar\nDipankar Ghosh\nThanks for all your moral support!",

                    wAppMajorMS, wAppMinorMS, wAppMajorLS, wAppMinorLS,
                    wThemeMajor, wThemeMinor);
    // -------------------------------------------------------------------------
    

    // ABOUT-LOGO Preparation --------------------------------------------------
    // Start with the group icon named 'IDI_DEWAPP'
    // We will traverse each image and pick the image
    // which is 128x128 (a.k.a _ABOUT_ICON_SIZE)
    // Read this article (by Raymond Chen) for a nice reference:
    // https://devblogs.microsoft.com/oldnewthing/20120720-00/?p=7083
    
    HRSRC       hRsrc = NULL;
    DWORD       dwResSize = NULL;
    HGLOBAL     hGlbl = NULL;
    LPBYTE      lpData;
    WORD        wIcoImgID = 0;
    
    if (_pImgAbout) { delete _pImgAbout; _pImgAbout = NULL; }

    hRsrc = FindResource(_hInstance, MAKEINTRESOURCE(IDI_DEWAPP), RT_GROUP_ICON);
    if (hRsrc && hRsrc != INVALID_HANDLE_VALUE)
    {
        dwResSize = SizeofResource(_hInstance, hRsrc);
        if (dwResSize)
        {
            hGlbl = LoadResource(_hInstance, hRsrc);
            if (hGlbl)
            {
                lpData = (LPBYTE)LockResource(hGlbl);
                if (lpData)
                {
                    // Thank you, Raymond Chen!
                    typedef struct GRPICONDIRENTRY
                    {
                        BYTE  bWidth;
                        BYTE  bHeight;
                        BYTE  bColorCount;
                        BYTE  bReserved;
                        WORD  wPlanes;
                        WORD  wBitCount;
                        DWORD dwBytesInRes;
                        WORD  nId;
                    } GRPICONDIRENTRY;
                    
                    typedef struct GRPICONDIR
                    {
                        WORD idReserved;
                        WORD idType;
                        WORD idCount;
                    } GRPICONDIR;
                    
                    GRPICONDIR          grpIconDir{ 0 };
                    GRPICONDIRENTRY     grpIconDirEntry{ 0 };
                    DWORD               dwIndex, dwOffset;
                    const DWORD         GRPICONDIR_SIZE = 0x06;
                    const DWORD         GRPICONDIRENTRY_SIZE = 0x0E; // (BYTE x 4 + WORD x 2 + DWORD x 1 + WORD x 1)

                    // We've got the Group Icon. Now scan in this way:
                    // 1. Get the count of images. (3rd WORD of lpData)
                    // 2. Read 0x0E elements and check the dimension
                    // 3. Repeat from #2 if dimensions are not matching _ABOUT_ICON_SIZE
                    // 4. If match found, get the ID
                    // 5. Find that from the resource
                    // 6. Lock the bytes and transfer them into an IStream
                    // 7. Create the GDI+ image from that IStream

                    CopyMemory(&grpIconDir, &(lpData[0]), GRPICONDIR_SIZE); // The &(lpData[0]) is used for consistent readability
                    dwOffset = GRPICONDIR_SIZE;
                    for (dwIndex = 0; dwIndex < grpIconDir.idCount; dwIndex++)
                    {
                        CopyMemory(&grpIconDirEntry, &(lpData[dwOffset]), GRPICONDIRENTRY_SIZE);
                        dwOffset += GRPICONDIRENTRY_SIZE;
                        if (grpIconDirEntry.bWidth == _ABOUT_ICON_SIZE &&
                            grpIconDirEntry.bHeight == _ABOUT_ICON_SIZE)
                        {
                            hRsrc = NULL;
                            hGlbl = NULL;
                            lpData = NULL;
                            dwResSize = 0;

                            hRsrc = FindResource(_hInstance, MAKEINTRESOURCE(grpIconDirEntry.nId), RT_ICON);
                            if (hRsrc)
                            {
                                dwResSize = SizeofResource(_hInstance, hRsrc);
                                if (dwResSize)
                                {
                                    hGlbl = LoadResource(_hInstance, hRsrc);
                                    if (hGlbl)
                                    {
                                        lpData = (LPBYTE)LockResource(hGlbl);
                                        if (lpData)
                                        {
                                            IStream *pStrImg = NULL;
                                            pStrImg = SHCreateMemStream(lpData, dwResSize);
                                            if (pStrImg)
                                            {
                                                LARGE_INTEGER   liStart{ 0 };
                                                HRESULT         hr = S_OK;
                                                
                                                hr = pStrImg->Seek(liStart, STREAM_SEEK_SET, NULL);
                                                if (SUCCEEDED(hr))
                                                {
                                                    _pImgAbout = Bitmap::FromStream(pStrImg);
                                                } // if (SUCCEEDED(hr)) - Icon Image

                                                pStrImg->Release();
                                                pStrImg = NULL;
                                            } // if (pStrImg) - Icon Image

                                        } // if (lpData) - Icon Image
                                    } // if (hGlbl) - Icon Image
                                } // if (dwResSize) - Icon Image
                            } // if (hRsrc) - Icon Image

                            break;

                        }// Size Match
                    } // for nIndex loop

                } // if (lpData) - Group Icon
            } // if (hGlbl) - Group Icon
        } // if (dwResSize) - Group Icon
    } // if (hRsrc && hRsrc != INVALID_HANDLE_VALUE) - Group Icon
    // -------------------------------------------------------------------------

    return;
}

int DEWABOUTWINDOW::_CreateCreditsBlock()
{
    DWORD       dwYOffset = 0x00;
    DWORD       dwXOffset = _F(4);

    _plpCredits = (CREDITBLOCK**)LocalAlloc(LPTR, sizeof(CREDITBLOCK*) * _CREDIT_COUNT);

    dwYOffset = _F(_THANKSHEIGHT);

    // MPG123
    _plpCredits[_CREDIT_SEQ_MPG] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, IDIMG_MPG, dwXOffset, dwYOffset,
                                                   L"---- MP2/MP3 Library (mpg123 1.25.10) ----", L"https://www.mpg123.org/index.shtml",

                                                   L"The mpg123 is a very fast MPG (Layer 1/2/3) decoder. The support for " \
                                                   L".MP2 and .MP3  files have been possible in Dew Drop Player, thanks to " \
                                                   L"this wonderful library. Overall, it has a very light memory footprint. " \
                                                   L"It is a free software and is licensed under LGPL 2.1.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_MPG]->ConsumedHeight;

    // Musepack
    _plpCredits[_CREDIT_SEQ_MPC] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, IDIMG_MPC, dwXOffset, dwYOffset,
                                                   L"---- Musepack Library (musepack r475) ----",
                                                   L"https://www.musepack.net/",

                                                   L"Musepack is an open source audio codec, which started off from MP2. " \
                                                   L"It is a very good codec, that performs better than MPEG Layer 3 " \
                                                   L"at mid bitrate ranges (~128 - ~192 Kbps). The SDK available from " \
                                                   L"the official site was used to enable .MPC file support in Dew Drop Player.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_MPC]->ConsumedHeight;

    // Ogg/Vorbis
    _plpCredits[_CREDIT_SEQ_OGG] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, IDIMG_OGG, dwXOffset, dwYOffset,
                                                   L"---- Xiph Ogg/Vorbis Libraries (libogg 1.3.3, vorbisfile 1.3.6) ----",
                                                   L"https://www.xiph.org/",

                                                   L"Nothing additional to say about this open source, high fidelity codec. " \
                                                   L"The site is very well maintained, organized and documented. " \
                                                   L"The .OGG container parsing and vorbis decoding in Dew Drop Player, " \
                                                   L"has been possible because of these awesome folks @ Xiph.org foundation!");
    dwYOffset += _plpCredits[_CREDIT_SEQ_OGG]->ConsumedHeight;

    // Opus
    _plpCredits[_CREDIT_SEQ_OPUS] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, IDIMG_OPUS, dwXOffset, dwYOffset,
                                                    L"---- Opus Library (opusfile 0.9) ----",
                                                    L"http://opus-codec.org/",

                                                    L"This is one of the best sounding open source codec for lower bitrates. " \
                                                    L"The quality of this codec is at par (and better at times), compared to " \
                                                    L"the AAC codec. A trimmed down version of this codec (excluding streaming " \
                                                    L"support), has been used in Dew Drop Player.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_OPUS]->ConsumedHeight;

    // Flac
    _plpCredits[_CREDIT_SEQ_FLAC] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, IDIMG_FLAC, dwXOffset, dwYOffset,
                                                    L"---- Flac Library (libFLAC 1.3.3) ----",
                                                    L"https://xiph.org/flac/",

                                                    L"The BEST lossless, open source codec as of the release of this " \
                                                    L"Dew Drop Player release date. The decoding is very fast, with very " \
                                                    L"low memory footprint. The site/API documentation is one of the best. " \
                                                    L"The SDK also supports native parsing of flac codec in ogg containers.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_FLAC]->ConsumedHeight;

    // APE
    _plpCredits[_CREDIT_SEQ_APE] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, IDIMG_APE, dwXOffset, dwYOffset,
                                                   L"---- Monkey's Audio Library (MAClib 5.48) ----",
                                                   L"https://www.monkeysaudio.com/",

                                                   L"Another good lossless audio codec. Thank you Matthew T. Ashland " \
                                                   L"for releasing the visual-studio-ready static .lib files, " \
                                                   L"which could be easily integrated into the Dew Drop Player " \
                                                   L"and support for .APE files was possible.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_APE]->ConsumedHeight;

    // AAC
    _plpCredits[_CREDIT_SEQ_AAC] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, 0x00, dwXOffset, dwYOffset,
                                                   L"---- Nero/AudioCoding AAC Library (libfaad2 2.8.8) ----",
                                                   L"https://www.audiocoding.com/faad2.html",

                                                   L"A rare find, this library is an open source SDK that supports " \
                                                   L"AAC decoding. This potent SDK has support for AAC and AAC+ " \
                                                   L"(HE-AAC & HE-AAC v2). Courtesy of this SDK, .AAC files and " \
                                                   L"raw AAC stream support was possible in Dew Drop Player.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_AAC]->ConsumedHeight;

    // AC3
    _plpCredits[_CREDIT_SEQ_AC3] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, 0x00, dwXOffset, dwYOffset,
                                                   L"---- ATSC A/52 Stream Decoder Library (liba52 0.7.4) ----",
                                                   L"http://liba52.sourceforge.net/",

                                                   L"Although it appears that this decoder is no longer under active " \
                                                   L"development, it is still a brilliant A/52 decoder. Thank you " \
                                                   L"Aaron Holtzman && Michel Lespinasse for your contribution, which " \
                                                   L"helped support .AC3 file support in Dew Drop Player.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_AC3]->ConsumedHeight;

    // ALAC
    _plpCredits[_CREDIT_SEQ_ALAC] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, 0x00, dwXOffset, dwYOffset,
                                                    L"---- ALAC GitHub Project ----",
                                                    L"https://github.com/macosforge/alac",
                                                    L"A minimalistic github project, this helped in providing support for " \
                                                    L"ALAC encoded .M4A/.MP4 files. A tweaked version of the code (by " \
                                                    L"changing the decoding to WAV instead of CAF), has been implemented in " \
                                                    L"Dew Drop Player. Thank you, Ryan Schmidt.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_ALAC]->ConsumedHeight;

    // AIFF
    _plpCredits[_CREDIT_SEQ_AIFF] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, 0x00, dwXOffset, dwYOffset,
                                                    L"---- LibAiff Library (libaiff 5.0) ----",
                                                    L"http://aifftools.sourceforge.net/libaiff/",

                                                    L"An excellent project by Marco Trillo, who helped provide " \
                                                    L"extensive documentation for using the source code for this " \
                                                    L"AIFF library. Thanks to this project/ADK, .AIFF file support " \
                                                    L"was possible in Dew Drop Player.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_AIFF]->ConsumedHeight;

    // WavPack
    _plpCredits[_CREDIT_SEQ_WV] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, IDIMG_WV, dwXOffset, dwYOffset,
                                                  L"---- WavPack Library (libwavpack 5.3.0) ----",
                                                  L"http://www.wavpack.com/index.html",

                                                  L"WavPack brings the best of both (compression and quality), by " \
                                                  L"implementing a unique hybrid mode. A big shout-out to David " \
                                                  L"and the WavPack team, for providing the exhaustive suite of test audio " \
                                                  L"files which helped in providing .WV support in Dew Drop Player.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_WV]->ConsumedHeight;

    // Bento4 (ISOM)
    _plpCredits[_CREDIT_SEQ_ISOM] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, 0x00, dwXOffset, dwYOffset,
                                                    L"---- Bento4 MP4/DASH Library (bento4 1.5.1-628) ----",
                                                    L"https://www.bento4.com/",

                                                    L"A surprise find for me, Bento4 proved crucial for parsing through " \
                                                    L"the ISO media files (.MP4/.M4A/.3GP) and extracting the correct audio " \
                                                    L"streams for the playback. Thanks to this critical project, the above " \
                                                    L"format support could be implemented in Dew Drop Player.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_ISOM]->ConsumedHeight;

    // TagLib
    _plpCredits[_CREDIT_SEQ_TAG] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, IDIMG_TAG, dwXOffset, dwYOffset,
                                                   L"---- TagLib Audio Tag Library (taglib 1.11.1) ----",
                                                   L"https://taglib.org/",

                                                   L"This project was my one-stop-shop for all my tagging solutions. " \
                                                   L"Clear build instructions, accompanied with top-notch documentation, " \
                                                   L"class-hierarchy diagrams and example codes, Dew Drop Player " \
                                                   L"could be equipped to recognize the tags for all its supported file formats.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_TAG]->ConsumedHeight;

    // File Signatures
    _plpCredits[_CREDIT_SEQ_FILESIG] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, 0x00, dwXOffset, dwYOffset,
                                                       L"---- Gary Kessler's File Signature Index Repository ----",
                                                       L"https://www.garykessler.net/library/file_sigs.html",

                                                       L"A superb website which is a knowledge repository for a huge " \
                                                       L"number of file formats. A must-read for anybody implementing " \
                                                       L"format sanity checks, not just for audio files but for " \
                                                       L"other files as well. The 'Deep Scan' feature of " \
                                                       L"Dew Drop Player, extensively uses the file signatures listed in this site.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_FILESIG]->ConsumedHeight;

    // ISOM Specifications
    _plpCredits[_CREDIT_SEQ_ISOMSPEC] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, 0x00, dwXOffset, dwYOffset,
                                                        L"---- ISO Base Media Specifications ----",
                                                        L"https://tools.ietf.org/html/rfc6381#section-3.3",

                                                        L"The official specification for ISO base media container files.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_ISOMSPEC]->ConsumedHeight;

    // ADTS Header Specification
    _plpCredits[_CREDIT_SEQ_ADTS] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, 0x00, dwXOffset, dwYOffset,
                                                    L"---- ADTS Header Specification (AAC) ----",
                                                    L"https://wiki.multimedia.cx/index.php/ADTS",

                                                    L"The detailed ADTS header specification. This illustration helped " \
                                                    L"in preparing AAC headers from ISOM (.MP4/.M4A/.3GP) files and also " \
                                                    L"helped make raw .AAC files seekable.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_ADTS]->ConsumedHeight;

    // CD Track Reading Approach
    _plpCredits[_CREDIT_SEQ_CDDA] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, 0x00, dwXOffset, dwYOffset,
                                                    L"---- Michel Helms' article on parsing Audio CDs ----",
                                                    L"https://www.codeproject.com/Articles/15725/Tutorial-on-reading-Audio-CDs",

                                                    L"A superb codeproject article, it completely removed any dependency on " \
                                                    L"any external library for parsing Audio CDs. Thank you, Michel Helms, " \
                                                    L"for the brilliant article with equally good explanation, which made " \
                                                    L"native Audio CD and .CDA file support, possible in Dew Drop Player.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_CDDA]->ConsumedHeight;

    // Stack Overflow
    _plpCredits[_CREDIT_SEQ_SO] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, IDIMG_SO, dwXOffset, dwYOffset,
                                                  L"---- Stack Overflow ----",
                                                  L"https://stackoverflow.com/questions",

                                                  L"It would be a blasphemy if I don't list this website. SO was to the rescue, " \
                                                  L"every time I had a question. I can honestly say that every inch of " \
                                                  L"Dew Drop Player is laden with contributions from the awesome " \
                                                  L"stalwarts who grace the website with their stupendous knowledge. Hail SO!");
    dwYOffset += _plpCredits[_CREDIT_SEQ_SO]->ConsumedHeight;

    // Symbola Font
    _plpCredits[_CREDIT_SEQ_SYMB] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, 0x00, dwXOffset, dwYOffset,
                                                  L"---- Symbola Font ----",
                                                  L"https://github.com/gearit/ttf-symbola",

                                                  L"The super glyph repository that covers all the multimedia unicode symbols " \
                                                  L"used throughout the Dew Drop Player. This eliminated the need for " \
                                                  L"for icons and helped make the depth of icon customization possible. " \
                                                  L"Thanks to the artist(s) behind this awesome symbol font.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_SYMB]->ConsumedHeight;

    // Font Forge
    _plpCredits[_CREDIT_SEQ_FF] = new CREDITBLOCK(_hInstance, _hWndCredits, _iDPI, _fScale, IDIMG_FF, dwXOffset, dwYOffset,
                                                  L"---- Font Forge ----",
                                                  L"https://fontforge.org/en-US/",

                                                  L"A superb de-facto, open source program for font handling. This program" \
                                                  L"made it very easy to create a custom, trimmed down version of the Symbola " \
                                                  L"font, that contains only the dingbat glyphs necessary in the Dew Drop Player UI.");
    dwYOffset += _plpCredits[_CREDIT_SEQ_FF]->ConsumedHeight;

    return (int)dwYOffset;
}

void DEWABOUTWINDOW::_CreateUIControls()
{
    const wchar_t   *wsAboutClass = L"DEWDROP.ABOUT.WND";
    const wchar_t   *wsCreditsClass = L"DEWDROP.ABOUT.CREDITS.WND";
    WNDCLASSEX      wcex { 0 };
    RECT            rctClient;
    int             iProposedClientHeight;
    const wchar_t   *wsCredits = L"   Licenses/Acknowledgements";

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = 0;
    wcex.lpfnWndProc    = DefWindowProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = _hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    wcex.lpszMenuName   = NULL;

    wcex.lpszClassName  = wsAboutClass;
    RegisterClassEx(&wcex);

    wcex.lpszClassName  = wsCreditsClass;
    RegisterClassEx(&wcex);

    _iCollapsedHeight = _F(10) + // Gap after title bar
                        _F(40) + // About Title Height
                        _F(10) + // Separator Line
                        _F(70) + // About Text Height
                        _F(10) + // Button Separator
                        _F(30) + // Button Height
                        _F(10);  // Padding after button

    iProposedClientHeight = _iCollapsedHeight + // Upper Section
                            _F(_CREDITSHEIGHT) + // Credits window
                            _F(10); // Padding at the end

    _iAboutWidth = _F(_WINWIDTH);
    _iCreditsHeight = _F(_CREDITSHEIGHT);
    _iIconXPos = _F(10);
    _iIconYPos = (_iCollapsedHeight - _F(_ABOUT_ICON_SIZE))/2;

    rctClient.left = 0; rctClient.top = 0; rctClient.right = _F(_WINWIDTH); // Width will be ignored
    rctClient.bottom = _iCollapsedHeight;
    AdjustWindowRectEx(&rctClient, _ABTSTYLE, FALSE, WS_EX_TOOLWINDOW);
    _iCollapsedHeight =  rctClient.bottom - rctClient.top;

    rctClient.left = 0; rctClient.top = 0; rctClient.right = _F(_WINWIDTH); // Width will be ignored
    rctClient.bottom = iProposedClientHeight;
    AdjustWindowRectEx(&rctClient, _ABTSTYLE, FALSE, WS_EX_TOOLWINDOW);
    _iAboutHeight =  rctClient.bottom - rctClient.top;


    _hWndAbout = CreateWindowEx(WS_EX_TOOLWINDOW, wsAboutClass, L"About Dew Drop Player", _ABTSTYLE,
                                0, 0, _iAboutWidth, _iAboutHeight, _hWndParent, NULL, _hInstance, NULL);
    GetClientRect(_hWndAbout, &rctClient);
    _iCreditsWidth = rctClient.right - _F(20);

    CopyMemory(&(_wsExpandCredits[1]), wsCredits, lstrlen(wsCredits) * sizeof(wchar_t));
    CopyMemory(_wsExpandCredits, &_EXPANDHINT, 2);
    CopyMemory(&(_wsCollapseCredits[1]), wsCredits, lstrlen(wsCredits) * sizeof(wchar_t));
    CopyMemory(_wsCollapseCredits, &_COLLAPSEHINT, 2);

    _hWndCredits = CreateWindowEx(0, wsAboutClass, L"Credits", _CDTSTYLE,
                                  0, 0, _iCreditsWidth, _iCreditsHeight, _hWndAbout, NULL, _hInstance, NULL);
    _iCreditsMaxHeight = this->_CreateCreditsBlock();

    _hLblCredits = CreateWindowEx(0, L"STATIC", _wsExpandCredits, _URLSTYLE,
                                  0, 0, 0, 0, _hWndAbout, NULL, _hInstance, NULL);
    _hCmdOK = CreateWindowEx(0, L"BUTTON", L"&OK", _BTNSTYLE,
                             0, 0, 0, 0, _hWndAbout, NULL, _hInstance, NULL);

    return;
}

void DEWABOUTWINDOW::_HandleAboutSizing()
{
    RECT        rctClient { 0 };
    int         iX, iY, iW, iH;

    GetClientRect(_hWndAbout, &rctClient);

    if (_bCreditMode)
    {
        iX = _F(10); iY = rctClient.bottom - _iCreditsHeight - _F(10);
        SetWindowPos(_hWndCredits, 0, iX, iY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        ShowWindow(_hWndCredits, SW_SHOW);
    }
    else
    {
        ShowWindow(_hWndCredits, SW_HIDE);
    }

    iX = _rctAboutTextArea.left; iY = _rctAboutTextArea.bottom + _F(20);
    iW = (_rctAboutTextArea.right - _rctAboutTextArea.left) - _F(85);
    iH = _F(20);
    SetWindowPos(_hLblCredits, 0, iX, iY, iW, iH, SWP_NOZORDER);

    iX = rctClient.right - _F(85); iY = _rctAboutTextArea.bottom + _F(10); iW = _F(75); iH = _F(30);
    SetWindowPos(_hCmdOK, 0, iX, iY, iW, iH, SWP_NOZORDER);

    return;
}

void DEWABOUTWINDOW::_SetFonts()
{
    LOGFONT     lgfFont { 0 };
    UINT        nIndex;

    lgfFont.lfWidth = 0;
    lgfFont.lfCharSet = DEFAULT_CHARSET;
    lgfFont.lfQuality = CLEARTYPE_QUALITY;
    lgfFont.lfPitchAndFamily = DEFAULT_PITCH;

    lgfFont.lfHeight = -MulDiv(8, _iDPI, 72);
    lgfFont.lfWeight = FW_NORMAL;
    CopyMemory(lgfFont.lfFaceName, L"Tahoma", 32 * sizeof(wchar_t));
    _hFntUI = CreateFontIndirect(&lgfFont);

    lgfFont.lfHeight = -MulDiv(10, _iDPI, 72);
    lgfFont.lfWeight = FW_BOLD;
    CopyMemory(lgfFont.lfFaceName, L"Trebuchet MS", 32 * sizeof(wchar_t));
    _hFntTitle = CreateFontIndirect(&lgfFont);
    lgfFont.lfWeight = FW_NORMAL;

    lgfFont.lfHeight = -MulDiv(9, _iDPI, 72);
    lgfFont.lfUnderline = 1;
    CopyMemory(lgfFont.lfFaceName, L"Trebuchet MS", 32 * sizeof(wchar_t));
    _hFntURL = CreateFontIndirect(&lgfFont);
    lgfFont.lfUnderline = 0;

    lgfFont.lfHeight = -MulDiv(9, _iDPI, 72);
    CopyMemory(lgfFont.lfFaceName, L"Trebuchet MS", 32 * sizeof(wchar_t));
    _hFntDescr = CreateFontIndirect(&lgfFont);

    lgfFont.lfHeight = -MulDiv(DEWUI_DIM_SYM_BUTTON_FONT, _iDPI, 72);
    CopyMemory(lgfFont.lfFaceName, DEWUI_SYMBOL_FONT_NAME, 32 * sizeof(wchar_t));
    _hFntIcon = CreateFontIndirect(&lgfFont);

    _pFntSymbol = new Font(_lpSymbolFont->SymbolFontGDIPlus, (REAL)_F(DEWUI_DIM_SYM_BUTTON_FONT), FontStyleRegular, UnitPoint);

    SendMessage(_hLblCredits, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));
    SendMessage(_hCmdOK, WM_SETFONT, (WPARAM)_hFntUI, MAKELPARAM(TRUE, 0));

    for (nIndex = 0; nIndex < _CREDIT_COUNT; nIndex++)
    {
        SendMessage((_plpCredits[nIndex]->hWndName), WM_SETFONT, (WPARAM)_hFntTitle, MAKELPARAM(TRUE, 0));
        SendMessage((_plpCredits[nIndex]->hWndURL), WM_SETFONT, (WPARAM)_hFntURL, MAKELPARAM(TRUE, 0));
        SendMessage((_plpCredits[nIndex]->hWndAboutDescr), WM_SETFONT, (WPARAM)_hFntDescr, MAKELPARAM(TRUE, 0));
    }

    return;
}

LRESULT DEWABOUTWINDOW::_AboutWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT     ps { 0 };
    RECT            rctWorkArea, rctAbout;
    int             iXPos, iYPos, iNewHeight;
    LPWINDOWPOS     lpPos;

    switch(nMsg)
    {
        case WM_CTLCOLORSTATIC:
        {
            if ((HWND)lParam == _hLblCredits)
            {
                SetTextColor((HDC)wParam, _crModuleText);
                SetBkMode((HDC)wParam, TRANSPARENT);
                return (LRESULT)_hbrAboutBack;
            }
            return FALSE;
        }
        case WM_PAINT:
        {
            BeginPaint(hWnd, &ps);
            this->_DrawAbout();
            EndPaint(hWnd, &ps);
            return FALSE;
        }
        case WM_DRAWITEM:
        {
            if (((LPDRAWITEMSTRUCT)lParam)->hwndItem == _hCmdOK)
                this->_DrawButton(((LPDRAWITEMSTRUCT)lParam), L"&OK", DEWUI_SYMBOL_BUTTON_OK);
            else
                break;
            return TRUE;
        }
        case WM_SIZE:
        {
            this->_HandleAboutSizing();
            return FALSE;
        }
        case WM_COMMAND:
        {
            if ( ((HWND)lParam == 0x00 && LOWORD(wParam) == IDOK) ||
                 ((HWND)lParam == 0x00 && LOWORD(wParam) == IDCANCEL) ||
                 ((HWND)lParam == _hCmdOK) )
            {
                SendMessage(_hWndAbout, WM_CLOSE, 0, 0);
                return FALSE;
            }
            else if ((HWND)lParam == _hLblCredits)
            {
                _bCreditMode = !_bCreditMode;
                SetWindowText(_hLblCredits, (_bCreditMode ? _wsCollapseCredits : _wsExpandCredits));

                SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);
                GetWindowRect(_hWndAbout, &rctAbout);
                iXPos = rctAbout.left;
                iYPos = rctAbout.top;
                iNewHeight = (_bCreditMode ? _iAboutHeight : _iCollapsedHeight);
                if ((iYPos + iNewHeight) > rctWorkArea.bottom)
                    iYPos = rctWorkArea.bottom - iNewHeight;

                SetWindowPos(_hWndAbout, 0, iXPos, iYPos, _iAboutWidth, iNewHeight, SWP_NOZORDER);
                if (!_bCreditMode)
                    this->_DrawAbout();
            }
            break;
        }
        case WM_KEYDOWN:
        {
            if (wParam == VK_PRIOR && (lParam & 0x40000000))
                SendMessage(_hWndCredits, WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), NULL);
            if (wParam == VK_NEXT && (lParam & 0x40000000))
                SendMessage(_hWndCredits, WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), NULL);
            return FALSE;
        }
        // Do not allow the window to move out of the work area
        case WM_WINDOWPOSCHANGING:
        {
            SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);
            lpPos = (LPWINDOWPOS)lParam;
            if (lpPos->cx + lpPos->x > rctWorkArea.right) lpPos->x = rctWorkArea.right - lpPos->cx;
            if (lpPos->x < rctWorkArea.left) lpPos->x = rctWorkArea.left;
            if (lpPos->cy + lpPos->y > rctWorkArea.bottom) lpPos->y = rctWorkArea.bottom - lpPos->cy;
            if (lpPos->y < rctWorkArea.top) lpPos->y = rctWorkArea.top;

            return 0;
        }
        case WM_CLOSE:
        {
            ShowWindow(_hWndAbout, SW_HIDE);
            if (_hWndParent)
            {
                PostMessage(_hWndParent, WM_DEWMSG_CHILD_CLOSED, 0, (LPARAM)_hWndAbout);
                SetForegroundWindow(_hWndParent);
            }
            return TRUE;
        }
    }
    return DefSubclassProc(hWnd, nMsg, wParam, lParam);
}

void DEWABOUTWINDOW::_DrawAbout()
{
    RECT        rctClient;
    HGDIOBJ     hObjOld;
    RECT        rctCredits;

    // WARNING! NEVER do anything with (like triggering/terminating)
    //          the _hTrdAnimation thread from here!!!
    
    GetClientRect(_hWndAbout, &rctClient);

    // The shaded rectangle with border
    hObjOld = SelectObject(_hDC, _hPenWndBorder);
    SelectObject(_hDC, _hbrAboutBack);
    Rectangle(_hDC, rctClient.left, rctClient.top, rctClient.right, rctClient.bottom);

    if (_bCreditMode)
    {
        // Get the 'Credits' window span w.r.t to the main About window
        GetWindowRect(_hWndCredits, &rctCredits);
        MapWindowPoints(HWND_DESKTOP, _hWndAbout, (LPPOINT)&rctCredits, 2);
        InflateRect(&rctCredits, _F(1), _F(1));

        // And draw a 'module-color' border around it
        SelectObject(_hDC, _hPenModuleBorder);
        SelectObject(_hDC, _hbrCreditsBack);
        Rectangle(_hDC, rctCredits.left, rctCredits.top, rctCredits.right, rctCredits.bottom);
    }

    SelectObject(_hDC, _hPenModuleBorder);

    SetTextColor(_hDC, _crModuleText);
    SetBkMode(_hDC, TRANSPARENT);

    SelectObject(_hDC, _hFntTitle);
    DrawText(_hDC, _wsAboutTitle, -1, &_rctAboutTitleArea, DT_CENTER);

    MoveToEx(_hDC, _rctAboutTitleArea.left, _rctAboutTitleArea.bottom + _F(5), NULL);
    LineTo(_hDC, _rctAboutTitleArea.right, _rctAboutTitleArea.bottom + _F(5));

    SelectObject(_hDC, hObjOld);

    return;
}

void DEWABOUTWINDOW::_DrawButton(const LPDRAWITEMSTRUCT& lpDIS, LPCWSTR wsText, LPCWSTR wsSymbol)
{
    RECT                rctText, rctIcon;
    HGDIOBJ             hObjOld, hOblOldIcon;
    HDC                 hDCMem;
    HBITMAP             hBmpIcon;

    // Current DC Settings
    SetBkMode(lpDIS->hDC, TRANSPARENT);
    SetTextColor(lpDIS->hDC, _crBtnText);

    // Mem DC (Icon) Settings
    rctIcon.left = 0;
    rctIcon.top = 0;
    rctIcon.right = lpDIS->rcItem.bottom - _F(7);
    rctIcon.bottom = lpDIS->rcItem.bottom - _F(7);
    hDCMem = CreateCompatibleDC(lpDIS->hDC);
    hBmpIcon = CreateCompatibleBitmap(lpDIS->hDC, rctIcon.right, rctIcon.bottom);
    hOblOldIcon = SelectObject(hDCMem, hBmpIcon);
    SelectObject(hDCMem, _hFntIcon);
    FillRect(hDCMem, &rctIcon, _hbrBtnBack);
    SetTextColor(hDCMem, _crBtnText);
    SetBkMode(hDCMem, TRANSPARENT);
    DrawText(hDCMem, wsSymbol, -1, &rctIcon, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    hObjOld = SelectObject(lpDIS->hDC, _hbrBtnBack);
    SelectObject(lpDIS->hDC, _hFntUI);

    SelectObject(lpDIS->hDC, _hPenBtnBorder);
    Rectangle(lpDIS->hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom);

    if ((lpDIS->itemState & ODS_FOCUS) == ODS_FOCUS)
    {
        CopyMemory(&rctText, &(lpDIS->rcItem), sizeof(RECT));
        InflateRect(&rctText, -_F(2), -_F(2));
        SelectObject(lpDIS->hDC, _hPenBtnFocus);
        Rectangle(lpDIS->hDC, rctText.left, rctText.top, rctText.right, rctText.bottom);
    }

    if ((lpDIS->itemState & ODS_SELECTED) == ODS_SELECTED)
        BitBlt(lpDIS->hDC, _F(4), _F(4), rctIcon.right + _F(1), rctIcon.bottom + _F(1), hDCMem, 0, 0, SRCCOPY);
    else
        BitBlt(lpDIS->hDC, _F(3), _F(3), rctIcon.right, rctIcon.bottom, hDCMem, 0, 0, SRCCOPY);

    SelectObject(hDCMem, hOblOldIcon);
    DeleteObject(hBmpIcon);
    DeleteDC(hDCMem);

    CopyMemory(&rctText, &(lpDIS->rcItem), sizeof(RECT));
    rctText.left = _F(30);
    if ((lpDIS->itemState & ODS_SELECTED) == ODS_SELECTED)
        OffsetRect(&rctText, _F(1), _F(1));
    DrawText(lpDIS->hDC, wsText, -1, &rctText, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    SelectObject(lpDIS->hDC, hObjOld);
}

#pragma endregion

#pragma region Public Section

DEWABOUTWINDOW::DEWABOUTWINDOW(HWND hWndParent, int iDPI, float fScale) :
// Thank you: https://isocpp.org/wiki/faq/pointers-to-members
_Animations{ &DEWABOUTWINDOW::_Animation_Beat, &DEWABOUTWINDOW::_Animation_Oscillate,
             &DEWABOUTWINDOW::_Animation_Fade, &DEWABOUTWINDOW::_Animation_Tint }
{
    RECT        rctCredits, rctClient;
    UINT        nIndex;
    Graphics    *pGr;
    HBITMAP     hBmp;
    RECT        rctThanks;
    HICON       hIcoAbout = NULL;

    _hInstance = GetModuleHandle(NULL);
    _hWndParent = hWndParent;
    _iDPI = iDPI;
    _fScale = fScale;
    _lpSymbolFont = new DEWSYMBOLFONT();

    this->_CreateUIControls();
    this->_SetFonts();
    this->_CreateThanksImage();
    this->_CreateAboutContent();

    GetClientRect(_hWndAbout, &rctClient);
    _hDC = GetDC(_hWndAbout);
    _hDCIcoMem = CreateCompatibleDC(_hDC);
    _hDCScrollMem = CreateCompatibleDC(_hDC);
    _hDCCredits = GetDC(_hWndCredits);
    

    // Now that we have all the images in place, we'll create a BITMAP with
    // all the images drawn. This bitmap will be large with the maximum height
    // with all the credits blocks included. Later on, we'll just BitBlt them,
    // to avoid the flicker while scrolling. BitBlt outperforms DrawImage by miles.
    GetClientRect(_hWndCredits, &rctCredits); // The width is constant
    _hDCCreditsMem = CreateCompatibleDC(_hDCCredits);
    hBmp = CreateCompatibleBitmap(_hDCCredits,
                                  rctCredits.right - rctCredits.left,
                                  _iCreditsMaxHeight);
    SelectObject(_hDCCreditsMem, hBmp);
    pGr = Graphics::FromHDC(_hDCCreditsMem);
    rctThanks.left = 0; rctThanks.top = 0;
    rctThanks.right = rctCredits.right; rctThanks.bottom = _pImgThanks->GetWidth();
    if (_pImgThanks)
    {
        // Fill the header banner with white background. This way, we'll
        // rip off the entire section from (0, 0) -> (client width, banner height)
        // while using BitBlt
        FillRect(_hDCCreditsMem, &rctThanks, _hbrCreditsBack);
        // And center-draw the 'Thank You' image.
        pGr->DrawImage(_pImgThanks,
                       (rctCredits.right - rctCredits.left - _pImgThanks->GetWidth())/2,
                       0);
    }
    // Loop through all the Credits block and draw the images (wherever applicable)
    for (nIndex = 0; nIndex < _CREDIT_COUNT; nIndex++)
    {
        // Draw the (applicable) image in its absolute position
        if (_plpCredits[nIndex]->Image)
        {
            pGr->DrawImage((_plpCredits[nIndex]->Image),
                           _plpCredits[nIndex]->ImgStart.X,
                           _plpCredits[nIndex]->ImgStart.Y);
        }
    }
    delete pGr;
    DeleteObject(hBmp);

    // Do NOT change this order! First, _hWndCredits size needs to be fixed.
    // Accordingly, the VSCROLLBAR of the _hWndCredits window will be calculated
    SetWindowSubclass(_hWndAbout, _AboutMsgHandler, (UINT_PTR)_hWndAbout, (DWORD_PTR)this);
    SetWindowSubclass(_hWndCredits, _CreditsMsgHandler, (UINT_PTR)_hWndAbout, (DWORD_PTR)this);
    this->_HandleCreditsSizing();

    return;
}

HWND DEWABOUTWINDOW::Show(const DEWTHEME& Theme)
{
    int         iXPos, iYPos, iParentWidth, iParentHeight;
    RECT        rctParent, rctWorkArea;
    HBITMAP     hBmpAboutText = NULL;

    GetWindowRect(_hWndParent, &rctParent);
    iParentWidth = rctParent.right - rctParent.left;
    iParentHeight = rctParent.bottom - rctParent.top;

    iXPos = (iParentWidth - _iAboutWidth)/2 + rctParent.left;
    iYPos = (iParentHeight - _iCollapsedHeight)/2 + rctParent.top;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWorkArea, 0);
    if (iXPos < rctWorkArea.left) iXPos = rctWorkArea.left;
    if (iYPos < rctWorkArea.top) iYPos = rctWorkArea.top;
    if ((iXPos + _iAboutWidth) > rctWorkArea.right) iXPos = rctWorkArea.right - _iAboutWidth;
    if ((iYPos + _iCollapsedHeight) > rctWorkArea.bottom) iYPos = rctWorkArea.bottom - _iCollapsedHeight;

    this->_ResetGDIObjects();

    _hPenWndBorder = CreatePen(PS_SOLID, _F(1), Theme.WinStyle.OutlineColor);
    _hPenModuleBorder = CreatePen(PS_SOLID, _F(1), Theme.ModuleStyle.OutlineColor);
    _hPenBtnBorder = CreatePen(PS_SOLID, _F(1), Theme.MMButtonStyle.OutlineColor);
    _hPenBtnFocus = CreatePen(PS_DOT, _F(1), Theme.MMButtonStyle.OutlineColor);

    _hbrCreditsBack = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
    _hbrTitleBack = CreateSolidBrush(RGB(0xD0, 0xD0, 0xD0));
    _hbrAboutBack = CreateSolidBrush(Theme.WinStyle.BackColor);
    _hbrBtnBack = CreateSolidBrush(Theme.MMButtonStyle.BackColor);

    _crModuleText = Theme.ModuleStyle.TextColor;
    _crBtnText = Theme.MMButtonStyle.TextColor;

    hBmpAboutText = CreateCompatibleBitmap(_hDC, _rctAboutTextArea.right - _rctAboutTextArea.left,
                                                   _rctAboutTextArea.bottom - _rctAboutTextArea.top);
    SelectObject(_hDCScrollMem, hBmpAboutText);

    _bCreditMode = FALSE;
    SetWindowText(_hLblCredits, _wsExpandCredits);
    SetWindowPos(_hWndAbout, 0, iXPos, iYPos, _iAboutWidth, _iCollapsedHeight, SWP_NOZORDER);
    SendMessage(_hWndCredits, WM_VSCROLL, MAKEWPARAM(SB_TOP, 0), NULL);

    ShowWindow(_hWndAbout, SW_SHOW);
    UpdateWindow(_hWndAbout);

    // Start the animation thread immediately AFTER showing the window
    // The threads will loop till the window is visible.
    // So make sure BOTH Show/Update-Window() methods are called
    // before restarting the thread
    if (_hTrdLogoAnimation)
    {
        TerminateThread(_hTrdLogoAnimation, 0x00);
        _hTrdLogoAnimation = NULL;
    }
    _hTrdLogoAnimation = CreateThread(NULL, 0, _TrdAnimateLogo, this, 0, NULL);

    if (_hTrdScrollAnimation)
    {
        TerminateThread(_hTrdScrollAnimation, 0x00);
        _hTrdScrollAnimation = NULL;
    }
    _hTrdScrollAnimation = CreateThread(NULL, 0, _TrdScrollNames, this, 0, NULL);

    SetActiveWindow(_hWndAbout);
    if (_hWndParent)
        EnableWindow(_hWndParent, FALSE);

    return _hWndAbout;
}

DEWABOUTWINDOW::~DEWABOUTWINDOW()
{
    UINT        nIndex;

    if (_hTrdLogoAnimation)
        TerminateThread(_hTrdLogoAnimation, 0x00);
    _hTrdLogoAnimation = NULL;

    if (_hTrdScrollAnimation)
        TerminateThread(_hTrdScrollAnimation, 0x00);
    _hTrdScrollAnimation = NULL;

    this->_ResetGDIObjects(); // Delete the theme'able GDI/GDI+ objects

    // Remove the GDI Fonts
    if (_hFntUI) { DeleteObject(_hFntUI); _hFntUI = NULL; }
    if (_hFntTitle) { DeleteObject(_hFntTitle); _hFntTitle = NULL; }
    if (_hFntURL) { DeleteObject(_hFntURL); _hFntURL = NULL; }
    if (_hFntDescr) { DeleteObject(_hFntDescr); _hFntDescr = NULL; }
    if (_hFntIcon) { DeleteObject(_hFntIcon); _hFntIcon = NULL; }

    // Remove the GDI+ Font
    if (_pFntSymbol) { delete _pFntSymbol; _pFntSymbol = NULL; }

    if (_pImgThanks) { delete _pImgThanks; _pImgThanks = NULL; } // Delete the 'Thank You' image
    if (_pImgAbout) { delete _pImgAbout; _pImgAbout = NULL; } // Delete the 'About Icon' image

    // Remove the subclassings
    RemoveWindowSubclass(_hWndCredits, _CreditsMsgHandler, (UINT_PTR)_hWndCredits);
    RemoveWindowSubclass(_hWndAbout, _AboutMsgHandler, (UINT_PTR)_hWndAbout);

    // Release the Device Contexts
    DeleteDC(_hDCCreditsMem);
    DeleteDC(_hDCIcoMem);
    DeleteDC(_hDCScrollMem);
    ReleaseDC(_hWndCredits, _hDCCredits);
    ReleaseDC(_hWndAbout, _hDC);

    // Loop through and delete each Credit block
    for (nIndex = 0; nIndex < _CREDIT_COUNT; nIndex++)
    {
        delete (_plpCredits[nIndex]);
        _plpCredits[nIndex] = NULL;
    }
    // And release the Credit Block array
    LocalFree(_plpCredits);
    _plpCredits = NULL;

    // Delete the Symbol Font repo class
    delete _lpSymbolFont;

    // Destroy the (child & main) windows
    DestroyWindow(_hWndCredits);
    DestroyWindow(_hCmdOK);
    DestroyWindow(_hWndAbout);

    return;
}

#pragma endregion
