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

#ifndef _DEWSHUTDOWNTIMER_H_
#define _DEWSHUTDOWNTIMER_H_

#include "DewUpDownControl.h"
#include "DewUIObject.h"

typedef class DEWSHUTDOWNTIMER
{
    private:
        HWND                    _hWndParent = NULL;
        HWND                    _hWndTimer = NULL;
        HWND                    _hCmdAbort = NULL, _hCmdStart = NULL, _hCmdOK = NULL;
        HDC                     _hDC = NULL, _hDCMem = NULL;
        HINSTANCE               _hInstance = NULL;
        HANDLE                  _hTrdCountdown = NULL;
        LPDEWUPDOWNCONTROL      _lpUpDownHours = NULL, _lpUpDownMinutes = NULL, _lpUpDownSeconds = NULL;
        Pen                     *_pPenCountdown = NULL, *_pPenWinBorder = NULL;
        SolidBrush              *_pBrText = NULL, *_pBrWinBack = NULL;
        Font                    *_pFont = NULL;
        Font                    *_pFontCountdown = NULL;
        HFONT                   _hFntUI = NULL;
        HFONT                   _hFntIcon = NULL;
        HPEN                    _hPenBoundary = NULL, _hPenFocus = NULL;
        HBRUSH                  _hbrBtnBack = NULL;
        HBITMAP                 _hBmpCountdown = NULL;
        COLORREF                _crBtnText;
        BOOL                    _bCountdownMode = FALSE;
        BOOL                    _bVisible = FALSE; // Flag to detect if visible. We'll not unnecessarily create images if not visible
        int                     _iDPI = 96, _iUpDownWidth, _iUpDownHeight;
        float                   _fScale = 1.0f;
        const DWORD             _WINWIDTH = 400;
        const DWORD             _WINHEIGHT = 250;
        const DWORD             _CIRCLE_DIAMETER = 121;
        const int               _COUNTDOWN_BRUSH_THICKNESS = 7;
        const DWORD             _POLL_FREQ = 10; // ms

        static LRESULT CALLBACK _TimerMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);
        static DWORD WINAPI     _TrdCountdown(LPVOID lpv);
        void                    _DrawButton(const LPDRAWITEMSTRUCT& lpDIS, LPCWSTR wsCaption, LPCWSTR wsIcon);
        void                    _ResetObjects();
        LRESULT                 _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        void                    _HandleSizing();
        void                    _Draw();
        void                    _Countdown(); // To be called from the thread only. Do NOT call this directly!

    public:
        const HWND&             Handle;
        const BOOL&             CountdownMode;
                                DEWSHUTDOWNTIMER(HWND hWndParent, int iDPI, float fScale);
        void                    ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw = TRUE);
        HWND                    Show(const DEWTHEME& Theme);
        void                    Abort();
                                ~DEWSHUTDOWNTIMER();

} DEWSHUTDOWNTIMER, *LPDEWSHUTDOWNTIMER;

#endif // _DEWSHUTDOWNTIMER_H_
