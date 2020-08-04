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

#ifndef _DEWUPDOWNCONTROL_H_
#define _DEWUPDOWNCONTROL_H_

#include "DewSettings.h"

// Please do not inherit this class
typedef class DEWUPDOWNCONTROL
{
    private:
        HWND            _hWnd = NULL;
        HDC             _hDC = NULL;
        HINSTANCE       _hInstance = NULL;
        HWND            _hWndParent = NULL;
        WORD            _wMinVal = 0;
        WORD            _wMaxVal = 100;
        WORD            _wCurrVal = 0;
        int             _iPrimaryMult = 1;
        int             _iDPI = 96;
        int             _iSectionHeight;
        float           _fScale = 1.0f;
        COLORREF        _crOutline;
        COLORREF        _crText;
        COLORREF        _crBack;
        HPEN            _hPenDiv;
        HBRUSH          _hbrBack;
        HFONT           _hFntPrimary = NULL, _hFntSecondary = NULL;
        static wchar_t  _swsDewUpDownCtrlClass[32];
        static UINT     _snInstanceCount;
        static BOOL     _sbCommonOpsComplete;

        static LRESULT  CALLBACK _UpDownMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);
        LRESULT         _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        void            _Draw(HDC hDC);


    public:
        const HWND&     Handle;
        const WORD&     Value;
                        DEWUPDOWNCONTROL(HWND hWndParent, int iX, int iY, int iPrimaryMult, WORD wMinVal, WORD wMaxVal);
        void            SetValue(WORD wCurrVal);
        void            ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw = TRUE);
        void            SetFont(LPCWSTR wsFontName, int iFontSizePrimary, int iFontSizeSecondary, BOOL bForceDraw = TRUE);
        void            SetVisible(BOOL bVisible = TRUE);
        void            Move(int iXPos, int iYPos);
                        ~DEWUPDOWNCONTROL();

} DEWUPDOWNCONTROL, *LPDEWUPDOWNCONTROL;

#endif // _DEWUPDOWNCONTROL_H_
