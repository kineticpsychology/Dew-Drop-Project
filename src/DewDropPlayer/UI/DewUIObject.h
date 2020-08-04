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

#ifndef _DEWUIOBJECT_H_
#define _DEWUIOBJECT_H_

#include "DewSettings.h"
#include "DewSymbolFont.h"

// Abstract class
typedef class DEWUIOBJECT
{
    protected:
        DEWUIOBJECT         *_Parent;
        HINSTANCE           _hInstance = NULL;
        HWND                _hWnd = NULL;
        HWND                _hWndTooltip = NULL;
        HDC                 _hDC = NULL;
        POINT               _ptPos;
        Color               _crBack;
        Color               _crText;
        Color               _crOutline;
        Brush               *_pBrBack = NULL;
        Brush               *_pBrText = NULL;
        Pen                 *_pPenOutline = NULL;
        Pen                 *_pPenText = NULL;
        Font                *_pFont = NULL;
        wchar_t             _wsFontName[32];
        wchar_t             _wsText[DEWUI_MAX_TITLE];
        BOOL                _bBold = FALSE;
        BOOL                _bItalic = FALSE;
        BOOL                _bAlternateIconMode = FALSE;
        BOOL                _bLayoutMode = FALSE;
        BOOL                _bTransparentIcons = FALSE;
        int                 _iFontSize;
        int                 _iDPI = 96;
        int                 _iWidth = 0;
        int                 _iHeight = 0;
        float               _fScale = 1.0f;
        float               _fIconScale = 1.0f;
        float               _fDimension = 0.0f;
        BYTE                _btObjectID = DEWUI_OBJ_NONE;
        BYTE                _btObjectType = DEWUI_OBJ_TYPE_NONE;
        const LPDEWSYMBOLFONT& _lpSymbolFont;

        static float        _sfIconScalingIndex[3];
        static int          _siDPI;
        static float        _sfScale;
        static HINSTANCE    _shInstance;
        static UINT         _snInstanceCount;
        static LPDEWSYMBOLFONT  _slpSymbolFont;
        static Bitmap       *_pImgBackground;
        static BOOL         _sbCommonOpsDone;

        static LRESULT      CALLBACK _MsgHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, UINT_PTR nID, DWORD_PTR dwRefData);

        virtual void        _Draw(HDC) = 0;
        virtual LRESULT     _WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam) = 0;
        virtual void        _PrepareImages() = 0; // Any image(s) that would need to be prepared for display
        virtual void        _InitiateSubclassing();
        virtual void        _CreateTooltip();
        virtual void        _Scale(BYTE btIconScale);
        virtual void        _MoveToDefaultLocation() = 0;

    public:
        const wchar_t*      Text;
        const HWND&         Handle;
        const BYTE&         ObjectType;
        const BYTE&         ObjectID;
        const int&          Width;
        const int&          Height;
        const POINT&        Position;

                            DEWUIOBJECT();
        virtual void        SetLayoutMode(BOOL bLayoutMode, BOOL bForceDraw = FALSE); // Typically, need not be implemented anywhere in child classes.
        virtual void        ApplyTheme(const DEWTHEME& Theme, BOOL bForceDraw = TRUE, BOOL bCalcScale = TRUE) = 0; // To be implemented by object-style level classes
        virtual void        SetText(LPCWSTR wsText); // This will mostly be non-overridden
        virtual void        SetVisible(BOOL bVisible = TRUE); // This will mostly be non-overridden
        virtual void        Refresh(); // This will mostly be non-overridden
        virtual void        Move(int iXPos, int iYPos); // This will mostly be non-overridden
        virtual void        SetLocation(DWORD dwPresetLocation);
        virtual void        SetTip(LPCWSTR wsTooltip);
        // This method will most likely be consumed only by the Window Object
        // However, to avoid cross-linking the headers, this is declared here
        // so that the child controls can blindly post to this method without
        // having to know who it belongs to.
        virtual void        ProcessMessage(DEWUIOBJECT* lpObjSender, BYTE btMessage, BYTE btNotificationCode, DWORD_PTR dwValue) = 0;
        virtual             ~DEWUIOBJECT();

} DEWUIOBJECT, *LPDEWUIOBJECT;

typedef struct _DEWUICOMPONENT
{
    LPDEWUIOBJECT       _lpObject;
    wchar_t             _wsCaption[32];
} DEWUICOMPONENT, *LPDEWUICOMPONENT;

#endif // _DEWUIOBJECT_H_
