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

#ifndef _DEWBUTTON_H_
#define _DEWBUTTON_H_

#include "DewUIObject.h"

typedef class DEWBUTTON : public DEWUIOBJECT
{
    protected:
        Image               *_pImgRaised = NULL;
        Image               *_pImgSunken = NULL;
        BOOL                _bDepressed = FALSE;
        const DWORD         _BTNSTYLE = WS_CHILD | WS_VISIBLE;

        static wchar_t      _swsButtonClassName[32];
        static UINT         _snInstanceCount;
        static BOOL         _sbClassRegistered;

        virtual void        _MoveToDefaultLocation() = 0;
        virtual void        _Draw(HDC);
        virtual void        _ClickedEvent();
        virtual LRESULT     _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

    public:
                            DEWBUTTON();
        virtual void        ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw = TRUE, BOOL bCalcScale = TRUE);
        virtual void        ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue);
        virtual             ~DEWBUTTON();
} DEWBUTTON, *LPDEWBUTTON;

#endif // _DEWBUTTON_H_
