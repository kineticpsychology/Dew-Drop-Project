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

#ifndef _DEWTHEME_H_
#define _DEWTHEME_H_

#define DEWTHEME_ATTR_LENGTH                64

#define DEWTHEME_ICON_MODE_DEFAULT          0x00
#define DEWTHEME_ICON_MODE_ALTERNATE        0x01

#define DEWTHEME_ICON_SCALE_SMALL           0x00
#define DEWTHEME_ICON_SCALE_MEDIUM          0x01
#define DEWTHEME_ICON_SCALE_LARGE           0x02

#define DEWTHEME_ACTION_THEME_APPLIED       0x00
#define DEWTHEME_ACTION_DEFAULT             0x01
#define DEWTHEME_ACTION_THEME_OVERRIDE      0x02
#define DEWTHEME_ACTION_THEME_VER_MISMATCH  0x03
#define DEWTHEME_ACTION_THEME_FAIL          0xEF

#ifdef DEW_THEMER_MODE
#define DEWTHEME_SAVE_OK                    0x00
#define DEWTHEME_SAVE_NO_PASSWORD           0x01
#define DEWTHEME_SAVE_NO_ACCESS             0x02
#define DEWTHEME_SAVE_GENERIC_ERROR         0xEF
#endif

#define DEWTHEME_DECRYPT_OK                 0x00
#define DEWTHEME_DECRYPT_ERROR              0x01
#define DEWTHEME_DECRYPT_VER_MISMATCH       0x02

#define DEWTHEME_DEFAULT_NAME               L"Dew Drop Default Theme"
#define DEWTHEME_DEFAULT_AUTHOR             L"Dew Drop"

#define DEWTHEME_HEADER                     0x48545044 // DPTH (in LE)

#include "DewUICommon.h"

typedef class DEWTHEME
{
    protected:
        typedef struct _WINSTYLE
        {
            COLORREF        BackColor;
            COLORREF        BackColor2;
            COLORREF        OutlineColor;
            COLORREF        TextColor;
        } WINSTYLE, *LPWINSTYLE;

        typedef struct _BUTTONSTYLE
        {
            COLORREF        BackColor;
            COLORREF        OutlineColor;
            COLORREF        TextColor;
        } BUTTONSTYLE, *LPBUTTONSTYLE;

        typedef struct _MODULESTYLE
        {
            COLORREF        BackColor;
            COLORREF        OutlineColor;
            COLORREF        TextColor;
        } MODULESTYLE, *LPMODULESTYLE;

        typedef struct _FONTSTYLE
        {
            wchar_t         FontName[32];
            int             FontSize;
            BOOL            IsBold;
            BOOL            IsItalic;
        } FONTSTYLE, *LPFONTSTYLE;

        typedef struct _THEME
        {
            wchar_t         _wsName[DEWTHEME_ATTR_LENGTH];
            wchar_t         _wsAuthor[DEWTHEME_ATTR_LENGTH];
            BYTE            _btIconMode;
            BYTE            _btIconScale;
            BYTE            _btTransparentIcons;
            BYTE            _btShowSeekbarTimes;
            WINSTYLE        _winStyle;
            BUTTONSTYLE     _mmButtonStyle;
            BUTTONSTYLE     _uiButtonStyle;
            MODULESTYLE     _moduleStyle;
            FONTSTYLE       _titleFontStyle; // CAREFUL! Do NOT use 'FontStyle' (enum in gdiplus.h)
            FONTSTYLE       _textFontStyle; // CAREFUL! Do NOT use 'FontStyle' (enum in gdiplus.h)
            DWORD           _dwBackImgSize;
            DWORD           _dwIconImgSize;
        } THEME , *LPTHEME;
        LPBYTE              _lpThemeBackImgData = NULL; // Since this is dynamic, it cannot be added to the struct
        LPBYTE              _lpIconImgData = NULL; // Since this is dynamic, it cannot be added to the struct

        THEME               _theme;
        HFONT               _hFntUI = NULL;
        const UINT          _HEADER_BLOCK_SIZE = 0x09; // 4 bytes (HEADER ID) + 2 bytes (WORD, MAJOR Version) + 2 bytes (WORD, MINOR Version) + 1 byte (Unlock Pass length)
        int                 _iDPI = 96;
        float               _fScale = 1.0f;
        const char*         _lpDefPass = "D3W.7h3m3.D3f.P@$$";
        char                _sCurrPass[DEWTHEME_ATTR_LENGTH];
        WORD                _wThemeVerMajor = 0;
        WORD                _wThemeVerMinor = 0;
        BOOL                _bDefaultTheme = TRUE;

        virtual void        _SetDefaultTheme(BOOL bCalcDPI = TRUE);
        virtual void        _SetDPIInfo();
        virtual BOOL        _CreateHeader(const char* sPassword, LPBYTE *lplpData, LPDWORD lpdwEncLen);
        virtual BOOL        _EncryptTheme(const char* sPassword, const LPBYTE lpSrcData, const DWORD dwSrcLen, LPBYTE *lplpEncData, LPDWORD lpdwEncLen);
        virtual BOOL        _GetThemeUnlockKey(LPBYTE lpMainData, LPBYTE *lplpKey, LPDWORD lpdwKey);
        virtual BOOL        _DecryptTheme(const LPBYTE lpEncData, const DWORD dwEncLen, LPBYTE *lplpDecData, LPDWORD lpdwDecLen);

    public:

        static void        GetDefaultThemeVersion(LPWORD lpwThemeVerMajor, LPWORD lpwThemeVerMinor);
    #ifdef DEW_THEMER_MODE
        // These functions will be available only when in 'Themer' mode
        virtual BYTE        WriteThemeFile(LPCWSTR wsThemeFile); // Ability to export theme files
        virtual void        SetPassword(const char* sPassword);
    #endif
        virtual BYTE        LoadThemeFile(LPCWSTR wsThemeFile, LPWORD lpwThemeVerMajor, LPWORD lpwThemeVerMinor, BOOL bTestOnly = FALSE, LPWSTR wsName = NULL, LPWSTR wsAuthor = NULL, LPWSTR wsVersion = NULL);
        const int&          DPI;
        const float&        Scale;
        const HFONT         DefaultUIFont;
        // If not in 'Theme' mode, these references cannot be modified
    #ifndef DEW_THEMER_MODE
        const wchar_t       *Name;
        const wchar_t       *Author;
        const WINSTYLE&     WinStyle;
        const BUTTONSTYLE&  MMButtonStyle;
        const BUTTONSTYLE&  UIButtonStyle;
        const MODULESTYLE&  ModuleStyle;
        const FONTSTYLE&    TitleFontStyle;
        const FONTSTYLE&    TextFontStyle;
        const BYTE&         IconMode;
        const BYTE&         IconScale;
        const BYTE&         TransparentIcons;
        const BYTE&         ShowSeekbarTimes;
        const DWORD&        BackgroundImageSize;
        const LPBYTE&       BackgroundImageData;
        const DWORD&        IconImageSize;
        const LPBYTE&       IconImageData;
    #else
        const char          *Password;
        wchar_t             *Name;
        wchar_t             *Author;
        WINSTYLE&           WinStyle;
        BUTTONSTYLE&        MMButtonStyle;
        BUTTONSTYLE&        UIButtonStyle;
        MODULESTYLE&        ModuleStyle;
        FONTSTYLE&          TitleFontStyle;
        FONTSTYLE&          TextFontStyle;
        BYTE&               IconMode;
        BYTE&               IconScale;
        BYTE&               TransparentIcons;
        BYTE&               ShowSeekbarTimes;
        DWORD&              BackgroundImageSize;
        LPBYTE&             BackgroundImageData;
        DWORD&              IconImageSize;
        LPBYTE&             IconImageData;
    #endif
        const WORD&         VersionMajor;
        const WORD&         VersionMinor;
        const BOOL&         DefaultTheme;
                            DEWTHEME();
                            DEWTHEME(LPCWSTR wsThemeFile);
        virtual void        Reset();
    #ifdef DEW_THEMER_MODE
        virtual void        operator= (const DEWTHEME& Theme);
    #endif
        virtual             ~DEWTHEME();
} DEWTHEME, *LPDEWTHEME;

#endif // _DEWTHEME_H_
