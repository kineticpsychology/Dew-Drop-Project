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

#ifndef _DEWABOUTWINDOW_H_
#define _DEWABOUTWINDOW_H_

#include <stdlib.h>
#include <time.h>
#include "DewTheme.h"
#include "DewSymbolFont.h"

// Do not inherit please
typedef class DEWABOUTWINDOW
{
    private:
        struct CREDITBLOCK
        {
            private:
                const UINT      _CREDIT_CTL_HEIGHT = 20;
                const UINT      _CREDIT_CTL_ABOUT_HEIGHT = 80;
                const DWORD     _LBLSTYLE = WS_CHILD | WS_VISIBLE;
                const DWORD     _URLSTYLE = WS_CHILD | WS_VISIBLE | SS_NOTIFY;

                int             _iDPI;
                float           _fScale;
                WORD            _wImageID{0x00};
                Image           *_pImg = NULL;
                Point           _ptImgStart;
                int             _iImgWidth = 0;
                int             _iImgHeight = 0;
                int             _iConsumedHeight;
                int             _iStartY = 0;
                int             _iEndY = 0;
                HWND            _hWndName = NULL; // Content Size: DEWUI_MAX_TITLE
                HWND            _hWndURL = NULL; // Content Size: DEWUI_MAX_TITLE
                HWND            _hWndAboutDescr = NULL; // Content Size: MAX_CHAR_PATH

            public:
                Image*&         Image;
                const int&      ImgWidth;
                const int&      ImgHeight;
                const Point&    ImgStart;
                const int&      ConsumedHeight;
                const int&      StartY;
                const int&      EndY;
                const HWND&     hWndName;
                const HWND&     hWndURL;
                const HWND&     hWndAboutDescr;

                            CREDITBLOCK(HINSTANCE hInstance, HWND hWndParent, int iDPI, float fScale, const WORD wResID, int iX, int iY, LPCWSTR wsName, LPCWSTR wsURL, LPCWSTR wsAbout);
                            ~CREDITBLOCK();
        };

        typedef void (DEWABOUTWINDOW::*ANIM_FUNC)(DWORD);

        const UINT      _CREDIT_SEQ_MPG = 0x00;
        const UINT      _CREDIT_SEQ_MPC = 0x01;
        const UINT      _CREDIT_SEQ_OGG = 0x02;
        const UINT      _CREDIT_SEQ_OPUS = 0x03;
        const UINT      _CREDIT_SEQ_FLAC = 0x04;
        const UINT      _CREDIT_SEQ_APE = 0x05;
        const UINT      _CREDIT_SEQ_AAC = 0x06;
        const UINT      _CREDIT_SEQ_AC3 = 0x07;
        const UINT      _CREDIT_SEQ_ALAC = 0x08;
        const UINT      _CREDIT_SEQ_AIFF = 0x09;
        const UINT      _CREDIT_SEQ_WV = 0x0A;
        const UINT      _CREDIT_SEQ_ISOM = 0x0B;
        const UINT      _CREDIT_SEQ_TAG = 0x0C;
        const UINT      _CREDIT_SEQ_FILESIG = 0x0D;
        const UINT      _CREDIT_SEQ_ISOMSPEC = 0x0E;
        const UINT      _CREDIT_SEQ_ADTS = 0x0F;
        const UINT      _CREDIT_SEQ_CDDA = 0x10;
        const UINT      _CREDIT_SEQ_SO = 0x11;
        const UINT      _CREDIT_SEQ_SYMB = 0x12;
        const UINT      _CREDIT_SEQ_FF = 0x13;
        const UINT      _CREDIT_COUNT = 0x14; // 20 (13 + 7)

        const DWORD     _WINWIDTH = 480;
        const DWORD     _WINHEIGHT = 560;
        const DWORD     _CREDITSHEIGHT = 360;
        const INT       _THANKSHEIGHT = 45;
        const INT       _ABOUT_ICON_SIZE = 128; // Must be a little lesser than (_WINHEIGHT - _CREDITSHEIGHT - 50)
        const DWORD     _ABTSTYLE = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN;
        const DWORD     _CDTSTYLE = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_VSCROLL | WS_TABSTOP;
        const DWORD     _BTNSTYLE = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_FLAT | BS_OWNERDRAW;
        const DWORD     _URLSTYLE = WS_CHILD | WS_VISIBLE | SS_NOTIFY;
        const WORD      _EXPANDHINT = 0x25BC;
        const WORD      _COLLAPSEHINT = 0x25B2;
        const ANIM_FUNC _Animations[4];

        HINSTANCE       _hInstance = NULL;
        HWND            _hWndParent = NULL;
        HWND            _hWndAbout = NULL, _hWndCredits = NULL;
        HDC             _hDC = NULL, _hDCIcoMem = NULL, _hDCScrollMem = NULL, _hDCCredits = NULL, _hDCCreditsMem = NULL;
        HWND            _hLblCredits = NULL, _hCmdOK = NULL;
        CREDITBLOCK     **_plpCredits = NULL;
        LPDEWSYMBOLFONT _lpSymbolFont = NULL;
        HANDLE          _hTrdLogoAnimation = NULL;
        HANDLE          _hTrdScrollAnimation = NULL;

        HBRUSH          _hbrAboutBack = NULL, _hbrBtnBack = NULL;
        HBRUSH          _hbrCreditsBack = NULL, _hbrTitleBack = NULL;
        HPEN            _hPenBtnFocus = NULL, _hPenBtnBorder = NULL, _hPenModuleBorder = NULL, _hPenWndBorder = NULL;
        HFONT           _hFntUI = NULL, _hFntTitle = NULL, _hFntURL = NULL, _hFntDescr = NULL, _hFntIcon = NULL;
        Font            *_pFntSymbol = NULL;
        Image           *_pImgThanks = NULL;
        Bitmap          *_pImgAbout = NULL;
        COLORREF        _crBtnText = 0x00;
        COLORREF        _crModuleText = 0x00;
        RECT            _rctAboutTitleArea;
        RECT            _rctAboutTextArea;
        const wchar_t   *_wsAboutTitle = L"Dew Drop Player\n(A win32-based multi-format audio player)";
        wchar_t         _wsAboutText[MAX_CHAR_PATH] { 0 };
        wchar_t         _wsExpandCredits[DEWUI_MAX_TITLE]{ 0 };
        wchar_t         _wsCollapseCredits[DEWUI_MAX_TITLE] { 0 };

        int             _iDPI = 96;
        float           _fScale = 1.0f;
        BOOL            _bCreditMode = FALSE;
        int             _iAboutWidth = 0, _iAboutHeight = 0;
        int             _iCreditsWidth = 0, _iCreditsHeight = 0;
        int             _iCollapsedHeight = 0;
        int             _iCreditsMaxHeight = 0;
        int             _iIconXPos = 0;
        int             _iIconYPos = 0;

        static LRESULT  CALLBACK _CreditsMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);
        LRESULT         _CreditsWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        void            _HandleCreditsSizing();
        void            _DrawCredits();

        static DWORD    WINAPI _TrdAnimateLogo(LPVOID lpv);
        void            _Animation_Beat(DWORD dwDelay);
        void            _Animation_Oscillate(DWORD dwDelay);
        void            _Animation_Fade(DWORD dwDelay);
        void            _Animation_Tint(DWORD dwDelay);
        void            _AnimateLogo();
        static DWORD    WINAPI _TrdScrollNames(LPVOID lpv);
        void            _Scroll_Names();


        static LRESULT  CALLBACK _AboutMsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);
        void            _ResetGDIObjects();
        void            _CreateThanksImage();
        void            _CreateAboutContent();
        int             _CreateCreditsBlock();
        void            _HandleAboutSizing();
        void            _CreateUIControls();
        void            _SetFonts();
        LRESULT         _AboutWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
        void            _DrawAbout();
        void            _DrawButton(const LPDRAWITEMSTRUCT& lpDIS, LPCWSTR wsText, LPCWSTR wsSymbol);

    public:
                        DEWABOUTWINDOW(HWND hWndParent, int iDPI, float fScale);
        HWND            Show(const DEWTHEME& Theme);
                        ~DEWABOUTWINDOW();

} DEWABOUTWINDOW, *LPDEWABOUTWINDOW;

#endif // _DEWABOUTWINDOW_H_
