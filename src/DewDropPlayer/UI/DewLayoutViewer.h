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

#ifndef _DEWLAYOUTVIEWER_H_
#define _DEWLAYOUTVIEWER_H_

#include "DewUIObject.h"

typedef class DEWLAYOUTVIEWER : public DEWUIOBJECT
{
    protected:
        HWND                _hLstLayout = NULL;
        HFONT               _hFont = NULL;
        Pen                 *_pPenWinOutline = NULL;
        LPDEWUICOMPONENT    _lpComponentStack;
        const DWORD         _LVSTYLE = WS_CHILD | WS_VISIBLE;
        const DWORD         _LSTSTYLE = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | WS_HSCROLL;

        static LRESULT      CALLBACK _LstLayoutMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);
        virtual void        _Scale(BYTE btIconScale);
        virtual void        _Draw(HDC);
        virtual void        _PrepareImages();
        virtual LRESULT     _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT     _LstLayoutWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        virtual void        _MoveToDefaultLocation();

    public:
                            DEWLAYOUTVIEWER(LPDEWUIOBJECT Parent, const DEWTHEME& Theme);
        virtual void        ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw = TRUE, BOOL bCalcScale = TRUE);
        virtual void        SetComponentStack(LPDEWUICOMPONENT ComponentStack, UINT nStackLength);
        virtual void        UpdateComponentLocations(BYTE btObjectID);
        virtual void        ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue);
        virtual             ~DEWLAYOUTVIEWER();
} DEWLAYOUTVIEWER, *LPDEWLAYOUTVIEWER;

#endif // _DEWLAYOUTVIEWER_H_
