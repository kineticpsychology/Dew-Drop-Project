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

#ifndef _THEMERABOUTWINDOW_H_
#define _THEMERABOUTWINDOW_H_

#include "DewDropThemerCommon.h"

// Please do not inherit
typedef class THEMERABOUTWINDOW
{
    private:
        HWND            _hWndParent = NULL;
        HINSTANCE       _hInstance = NULL;
        HWND            _hWnd = NULL;
        HDC             _hDC = NULL;
        HDC             _hDCMem = NULL;
        HBITMAP         _hBmpDisco = NULL;
        HGDIOBJ         _hObjOld = NULL;
        HWND            _hCmdOK = NULL;
        HWND            _hLblCredit = NULL;
        HWND            _hLblURL = NULL;
        HANDLE          _hTrdDisco = NULL;
        HICON           _hIcoAbout = NULL;
        HFONT           _hFntUI = NULL;
        HFONT           _hFntAbout = NULL;
        HPEN            _hPenBorder = NULL;
        HPEN            _hPenFocus = NULL;
        HBRUSH          _hbrBack = NULL;

        wchar_t         _wsAboutInfo[MAX_CHAR_PATH];
        COLORREF        _crShadow = 0x00, _crText = 0x00, _crBack = 0x00, _crURL = 0x00;
        RECT            _rctDisco;
        BOOL            _bDiscoReady = FALSE;
        BOOL            _bVisible = FALSE;
        int             _iDPI = 96;
        float           _fScale = 1.0f;
        const DWORD     _DISCO_SPEED = 10;

        static DWORD    WINAPI _TrdDisco(LPVOID lpv);
        static LRESULT  CALLBACK _AboutMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);
        void            _Disco();
        LRESULT         _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        void            _HandleSizing();
        void            _GetAboutInfo();
        void            _SetFonts();
        void            _Draw();
        void            _DrawButton(const LPDRAWITEMSTRUCT& lpDIS, LPCWSTR wsText);

    public:
                        THEMERABOUTWINDOW(HWND hWndParent, int iDPI, float fScale);
        HWND            Show();
                        ~THEMERABOUTWINDOW();
} THEMERABOUTWINDOW, *LPTHEMERABOUTWINDOW;

#endif // _THEMERABOUTWINDOW_H_
