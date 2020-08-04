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

#ifndef _DEWALBUMART_H_
#define _DEWALBUMART_H_

#include "DewUIObject.h"

typedef class DEWALBUMART : public DEWUIOBJECT
{
    protected:
        Image               *_pImgDefault = NULL;
        Image               *_pImgFromEngine = NULL;
        Image               *_pImgAlbumArt = NULL;
        const DWORD         _ARTSTYLE = WS_CHILD | WS_VISIBLE;

        virtual void        _Draw(HDC);
        virtual void        _PrepareImages();
        virtual LRESULT     _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        virtual void        _MoveToDefaultLocation();

    public:
                            DEWALBUMART(LPDEWUIOBJECT Parent, const DEWTHEME& Theme, Image* pImgAlbumArt = NULL);
        virtual void        ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw = TRUE, BOOL bCalcScale = TRUE);
        virtual void        SetAlbumArt(Image* pImgAlbumArt, BOOL bForceDraw = TRUE);
        virtual void        SetLocation(DWORD dwPresetLocation);
        virtual void        ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue);
        virtual             ~DEWALBUMART();
} DEWALBUMART, *LPDEWALBUMART;

#endif // _DEWALBUMART_H_
