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

#include "DewSymbolFont.h"

FontFamily* DEWSYMBOLFONT::_psffSymbolFontFamily = NULL;
HANDLE DEWSYMBOLFONT::_shSymbolFont = NULL;
UINT DEWSYMBOLFONT::_snInstanceCount = 0;

void DEWSYMBOLFONT::_RemoveSymbolFont()
{
    if (DEWSYMBOLFONT::_psffSymbolFontFamily)
    {
        delete (DEWSYMBOLFONT::_psffSymbolFontFamily);
        DEWSYMBOLFONT::_psffSymbolFontFamily = NULL;
    }

    if (DEWSYMBOLFONT::_shSymbolFont)
            RemoveFontMemResourceEx(DEWSYMBOLFONT::_shSymbolFont);
    return;
}

void DEWSYMBOLFONT::_AddSymbolFont()
{
    this->_RemoveSymbolFont();

    HRSRC                   hRCFont = NULL;
    DWORD                   dwRCFontSize = 0, dwFonts = 0;
    HGLOBAL                 hgFont = NULL;
    LPBYTE                  lpFontRes = NULL;
    HINSTANCE               hInstance = NULL;
    PrivateFontCollection   coll;

    hInstance = GetModuleHandle(NULL);
    hRCFont = FindResource(hInstance, MAKEINTRESOURCE(IDF_SYMBOL), RT_RCDATA);
    dwRCFontSize = SizeofResource(GetModuleHandle(NULL), hRCFont);
    hgFont = LoadResource(GetModuleHandle(NULL), hRCFont);
    lpFontRes = (LPBYTE)LockResource(hgFont);
    // Add font for GDI+ Usages
    DEWSYMBOLFONT::_psffSymbolFontFamily = new FontFamily();
    coll.AddMemoryFont(lpFontRes, dwRCFontSize);
    coll.GetFamilies(1, (DEWSYMBOLFONT::_psffSymbolFontFamily), (INT*)&dwFonts);
    // Add font for GDI Usage
    DEWSYMBOLFONT::_shSymbolFont = AddFontMemResourceEx(lpFontRes, dwRCFontSize, NULL, &dwFonts);
}

DEWSYMBOLFONT::DEWSYMBOLFONT()
{
    if ((DEWSYMBOLFONT::_snInstanceCount) == 0)
        this->_AddSymbolFont();
    SymbolFontGDIPlus = DEWSYMBOLFONT::_psffSymbolFontFamily;
    (DEWSYMBOLFONT::_snInstanceCount)++;
}

DEWSYMBOLFONT::~DEWSYMBOLFONT()
{
    (DEWSYMBOLFONT::_snInstanceCount)--;
    if ((DEWSYMBOLFONT::_snInstanceCount) == 0)
        this->_RemoveSymbolFont();
}
