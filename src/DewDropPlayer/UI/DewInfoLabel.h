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

#ifndef _DEWINFOLABEL_H_
#define _DEWINFOLABEL_H_

#include "DewUIObject.h"

#define DEWUI_INFOLABEL_TYPE_INVALID    0
#define DEWUI_INFOLABEL_TYPE_TITLE      1
#define DEWUI_INFOLABEL_TYPE_ARTIST     2
#define DEWUI_INFOLABEL_TYPE_ALBUM      3

typedef class DEWINFOLABEL : public DEWUIOBJECT
{
    protected:
        BYTE                _btInfoLabelType = DEWUI_INFOLABEL_TYPE_INVALID;
        Image               *_pImgInfo = NULL;
        Image               *_pImgBlank = NULL;
        Font                *_pFontLegend = NULL;
        const DWORD         _dwPadding = 4;
        HWND                _hLblInfo = NULL;
        const DWORD         _INFOSTYLE = WS_CHILD | WS_VISIBLE;
        wchar_t             _wsLegend[2];

        static wchar_t      _swsInfoLabelClass[32];
        static BOOL         _sbClassRegistered;
        static UINT         _snInstanceCount;

        virtual void        _Draw(HDC);
        virtual void        _PrepareImages();
        virtual LRESULT     _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        virtual void        _Scale(BYTE btIconScale);
        virtual void        _MoveToDefaultLocation();

    public:
                            DEWINFOLABEL(LPDEWUIOBJECT Parent, const DEWTHEME& Theme, BYTE btInfoLabelType, LPCWSTR wsInfo = NULL);
        virtual void        ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw = TRUE, BOOL bCalcScale = TRUE);
        virtual void        SetInfo(LPCWSTR wsInfo, BOOL bForceDraw = TRUE);
        virtual void        ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue);
        virtual             ~DEWINFOLABEL();
} DEWINFOLABEL, *LPDEWINFOLABEL;

#endif // _DEWINFOLABEL_H_

