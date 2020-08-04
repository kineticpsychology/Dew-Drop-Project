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

#ifndef _DEWDISCSELECTOR_H_
#define _DEWDISCSELECTOR_H_

#include "DewTheme.h"
#include "DewSymbolFont.h"

// Do not inherit please
typedef class DEWDISCSELECTOR
{
    private:
        HINSTANCE       _hInstance = NULL;
        HWND            _hWndParent = NULL;
        HWND            _hWndDiscSelector = NULL;
        HWND            _hLstDrives = NULL;
        HWND            _hCmdOK = NULL, _hCmdCancel = NULL;
        HDC             _hDC = NULL;
        LPDEWSYMBOLFONT _lpSymbolFont = NULL;
        HIMAGELIST      _hImlDriveTypes = NULL;
        HBRUSH          _hbrWndBack = NULL, _hbrBtnBack = NULL;
        HPEN            _hPenBtnFocus = NULL, _hPenBtnBorder = NULL, _hPenLstBorder = NULL, _hPenWndBorder = NULL;
        HFONT           _hFntUI = NULL, _hFntIcon = NULL;
        Font            *_pFntSymbol = NULL;
        SolidBrush      *_pBrText = NULL;
        COLORREF        _crBtnText = 0x00;
        int             _iDPI = 96;
        float           _fScale = 1.0f;
        wchar_t         _wsSelPath[4];
        static UINT     _snInstanceCount;

        static LRESULT  CALLBACK _DSMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);

        void            _HandleSizing();
        void            _EnumDrives();
        void            _AddDiscInfo(LPWSTR wsPath, LPWSTR wsLabel, LPWSTR wsDriveType, LPWSTR wsFSType, BOOL bFlushAll = FALSE);
        void            _DestroyGDIObjects();
        void            _ApplyTheme(const DEWTHEME& Theme);
        void            _AddImageListImage(LPCWSTR wsSymbol);
        void            _Draw();
        void            _DrawButton(const LPDRAWITEMSTRUCT& lpDIS, LPCWSTR wsText, LPCWSTR wsSymbol);
        void            _SetCurrentSelection();
        LRESULT         _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

    public:
        const wchar_t*  SelectedPath;
                        DEWDISCSELECTOR(HWND hWndParent, int iDPI, float fScale);
        HWND            Show(const DEWTHEME& Theme);
                        ~DEWDISCSELECTOR();

} DEWDISCSELECTOR, *LPDEWDISCSELECTOR;

#endif // _DEWDISCSELECTOR_H_
