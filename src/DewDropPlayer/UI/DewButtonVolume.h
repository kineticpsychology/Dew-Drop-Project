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

#ifndef _DEWVOLUMEBUTTON_H_
#define _DEWVOLUMEBUTTON_H_

#include "DewUIObject.h"

#define DEW_MAX_VOL_LEVEL      0xFFFF   // Volume 0 - 0xFFFF is the per-channel range accepted by waveOutXXX procs 

typedef class DEWVOLUMEBUTTON : public DEWUIOBJECT
{
    protected:
        Image               *_pImgVol00 = NULL;
        Image               *_pImgVol33 = NULL;
        Image               *_pImgVol67 = NULL;
        Image               *_pImgVol100 = NULL;
        Image               **_pImgCurr = NULL; // This will not be an isolated instance. It will 'roam around' amongst the other _pImgVolXX pointers
        wchar_t             _wsVolBtnClass[32];
        const DWORD         _PADDING = 8;
        const DWORD         _VOLMINMAXTOLERANCE = 2;
        const DWORD         _VOLSTYLE = WS_CHILD | WS_VISIBLE;
        DWORD               _dwCurrVolLevel = 0;
        float               _fVolUnitScale = 0.0f;
        DWORD               _dwVolBarLength = 0;
        Rect                _rctBounds;
        BOOL                _bDepressed = FALSE;
        TRACKMOUSEEVENT     _tme;

        virtual void        _Draw(HDC);
        virtual void        _ClearImages();
        virtual void        _CreateVolumeImages(Image **ppImg, LPCWSTR wsVolCaption);
        virtual void        _PrepareImages();
        virtual void        _ClickedEvent(DWORD dwNewVolLevel);
        virtual LRESULT     _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        virtual void        _Scale(BYTE btIconScale);
        virtual void        _MoveToDefaultLocation();

    public:
        const DWORD&        VolumeLevel;
                            DEWVOLUMEBUTTON(LPDEWUIOBJECT Parent, const DEWTHEME& Theme, DWORD dwInitVolLevel = DEW_MAX_VOL_LEVEL);
        virtual void        ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw = TRUE, BOOL bCalcScale = TRUE);
        virtual void        SetVolumeLevel(DWORD dwVolLevel, BOOL bForceDraw = TRUE);
        virtual void        SetLayoutMode(BOOL bLayoutMode, BOOL bForceDraw);
        virtual void        ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue);
        virtual             ~DEWVOLUMEBUTTON();
} DEWVOLUMEBUTTON, *LPDEWVOLUMEBUTTON;

#endif // _DEWVOLUMEBUTTON_H_
