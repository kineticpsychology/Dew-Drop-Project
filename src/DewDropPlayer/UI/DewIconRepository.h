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

#ifndef _DEWICONREPOSITORY_H_
#define _DEWICONREPOSITORY_H_

#include "DewSymbolFont.h"

// This class is designed in a way that no matter how many
// instances/inheritances are made out of this, all instances
// will re-use the same images. This makes the class flexible
// to be instantiated at will, at various spots, without the
// hassle of memory overload with redundant copies of the
// same set of images
typedef class DEWICONREPOSITORY
{
    private:

        #pragma region STATIC (RE-USED) CONTENTS

        static LPDEWSYMBOLFONT  _slpSymbolFont;

        // Individual image lists for various buttons
        static HIMAGELIST   _shImgOK;
        static HIMAGELIST   _shImgCancel;
        static HIMAGELIST   _shImgResetTheme;

        // A multi-image image list for tab control (Settings window)
        static HIMAGELIST   _shImgTab;

        // Various Bitmaps for Menu items:
        // Main window context menu bitmaps
        static HBITMAP      _shBmpLayoutMode;
        static HBITMAP      _shBmpResetLayout;
        static HBITMAP      _shBmpDewMode;
        static HBITMAP      _shBmpAbout;
        static HBITMAP      _shBmpQuit;

        // Playlist menu bitmaps
        static HBITMAP      _shBmpDelete;
        static HBITMAP      _shBmpExportSel;
        static HBITMAP      _shBmpExportAll;

        // Browse & open bitmaps
        static HBITMAP      _shBmpOpenFile;
        static HBITMAP      _shBmpOpenDir;
        static HBITMAP      _shBmpOpenDisc;

        // Tray bitmaps
        static HBITMAP      _shBmpPlay;
        static HBITMAP      _shBmpPause;
        static HBITMAP      _shBmpStop;
        static HBITMAP      _shBmpPrevious;
        static HBITMAP      _shBmpNext;
        static HBITMAP      _shBmpRestore;

        // Themer Application Menu Bitmaps (2 additional)
        #ifdef DEW_THEMER_MODE
        static HBITMAP      _shBmpDefaultTheme;
        static HBITMAP      _shBmpSave;
        #endif

        static UINT         _snInstanceCount;

        #pragma endregion

        int                 _iDPI = 96;
        float               _fScale = 1.0f;

        virtual void        _CreateMenuHBITMAP(LPCWSTR wsSymbolText, int iIconSize, HBITMAP* phBmp);
        virtual void        _CreateButtonHIMAGELIST(LPCWSTR wsSymbolText, int iIconSize, HIMAGELIST* phiml);
        virtual void        _CreateStaticObjects();
        virtual void        _DestroyStaticObjects();

    public:
        // And their outside-world const counterparts
        const HIMAGELIST&   ImgOK;
        const HIMAGELIST&   ImgCancel;
        const HIMAGELIST&   ImgResetTheme;

        const HIMAGELIST&   ImgTab;

        const HBITMAP&      BmpLayoutMode;
        const HBITMAP&      BmpResetLayout;
        const HBITMAP&      BmpDewMode;
        const HBITMAP&      BmpAbout;
        const HBITMAP&      BmpQuit;

        const HBITMAP&      BmpDelete;
        const HBITMAP&      BmpExportSel;
        const HBITMAP&      BmpExportAll;

        const HBITMAP&      BmpOpenFile;
        const HBITMAP&      BmpOpenDir;
        const HBITMAP&      BmpOpenDisc;

        const HBITMAP&      BmpPlay;
        const HBITMAP&      BmpPause;
        const HBITMAP&      BmpStop;
        const HBITMAP&      BmpPrevious;
        const HBITMAP&      BmpNext;
        const HBITMAP&      BmpRestore;

        #ifdef DEW_THEMER_MODE
        const HBITMAP&      BmpDefaultTheme;
        const HBITMAP&      BmpSave;
        #endif

                            DEWICONREPOSITORY(int iDPI, float fScale);
        virtual             ~DEWICONREPOSITORY();

} DEWICONREPOSITORY, *LPDEWICONREPOSITORY;

#endif
