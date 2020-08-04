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

#ifndef _DEWSEEKBAR_H_
#define _DEWSEEKBAR_H_

#include "DewUIObject.h"

typedef class DEWSEEKBAR : public DEWUIOBJECT
{
    protected:
        Image               *_pImgBase = NULL;
        Image               *_pImgInst = NULL; // Instantaneous image
        SolidBrush          *_pBrOutline = NULL;
        Rect                _rctBar;
        RectF               _rctTextPos;
        REAL                _rTimePerBar;
        BOOL                _bDisplayTimes = TRUE;
        BOOL                _bTimesInitialized = FALSE;
        BOOL                _bDepressed = FALSE;
        DWORD               _dwCurrTime = 0;
        DWORD               _dwClickedTime = 0;
        DWORD               _dwTotalTime = 0;
        TOOLINFO            _ti{ 0 };
        const DWORD         _SEEKSTYLE = WS_CHILD | WS_VISIBLE;

        virtual void        _ClickedEvent(DWORD dwNewTime);
        virtual void        _Scale(BYTE btIconScale);
        virtual void        _Draw(HDC);
        virtual void        _PrepareImages();
        virtual void        _PrepareUninitializedImage();
        virtual LRESULT     _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        virtual void        _MoveToDefaultLocation();
        virtual void        _SetTimeTip(int iXPos, int iYPos);

    public:
                            DEWSEEKBAR(LPDEWUIOBJECT Parent, const DEWTHEME& Theme);
        virtual void        ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw = TRUE, BOOL bCalcScale = TRUE);
        virtual void        SetTime(DWORD dwCurrTime, BOOL bForceDraw = TRUE);
        virtual void        SetLength(DWORD dwTotalTime);
        virtual void        ShowTimes(BOOL bShow = TRUE);
        virtual void        Reset();
        virtual void        SetTip(LPCWSTR wsTooltip);
        virtual void        ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue);
        virtual             ~DEWSEEKBAR();
} DEWSEEKBAR, *LPDEWSEEKBAR;

#endif // _DEWSEEKBAR_H_

