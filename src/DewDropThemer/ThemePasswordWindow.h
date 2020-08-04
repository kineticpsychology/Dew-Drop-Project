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

#ifndef _THEMEPASSWORDWINDOW_H_
#define _THEMEPASSWORDWINDOW_H_

#include "DewDropThemerCommon.h"

// Please do not inherit
typedef class THEMEPASSWORDWINDOW
{
    private:
        HWND            _hWndParent = NULL;
        HINSTANCE       _hInstance = NULL;
        HWND            _hWnd = NULL;
        HWND            _hLblBanner = NULL;
        HWND            _hTxtOldPwd = NULL, _hTxtCurrPwd = NULL, _hTxtVerifyCurrPwd = NULL;
        HWND            _hCmdOK = NULL, _hCmdCancel = NULL;
        HICON           _hIcoPwd = NULL;
        HIMAGELIST      _hImlBtnOK = NULL, _hImlBtnCancel = NULL;
        BROWSE_THEME_ACTION _browseAction;
        DEWTHEME&       _TargetTheme;
        HFONT           _hFntUI = NULL;
        int             _iDPI = 96;
        float           _fScale = 1.0f;
        int             _iIcoDim;
        POINT           _ptIcon;
        RECT            _rctEtch;
        const DWORD     _WINSTYLE = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_BORDER;
        const DWORD     _WINEXSTYLE = WS_EX_TOOLWINDOW | WS_EX_CLIENTEDGE;
        const wchar_t   *_wsDisclaimer = L"NOTE: Do not enter any special UNICODE character for password!";
        WORD            _wPassChar = 0x25CF; // Use 0x25A0 to show black squares, instead of circles

        static LRESULT  CALLBACK _TPMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);
        LRESULT         _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        void            _SetUIFont();
        void            _HandleSizing();
        BOOL            _Verify(); // TRUE: Keep the screen open and make user (re)enter the info. FALSE: Close window

    public:
        const BROWSE_THEME_ACTION&     ThemeAction;
        const HWND&     Handle;

                        THEMEPASSWORDWINDOW(HWND hWndParent, DEWTHEME& TargetTheme, int iDPI, float fScale);
        HWND            Show(BYTE btDisplayMode, LPCWSTR wsOpenThemeFile = NULL);
                        ~THEMEPASSWORDWINDOW();

} THEMEPASSWORDWINDOW, *LPTHEMEPASSWORDWINDOW;

#endif
