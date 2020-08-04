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

#ifndef _DEWSYMBOLFONT_H_
#define _DEWSYMBOLFONT_H_

#include "DewUICommon.h"
#ifdef DEW_THEMER_MODE
#include "../../DewDropThemer/DewDropThemerResource.h"
#endif

// Please do not inherit this. It is designed to refer to the same set of objects
// irrespective of the count/inheritance of instance(s)
typedef class DEWSYMBOLFONT
{
    private:
        void                _RemoveSymbolFont();
        void                _AddSymbolFont();

        static FontFamily   *_psffSymbolFontFamily;
        static HANDLE       _shSymbolFont;
        static UINT         _snInstanceCount;

    public:
        const FontFamily*   SymbolFontGDIPlus;

                            DEWSYMBOLFONT();
                            ~DEWSYMBOLFONT();

} DEWSYMBOLFONT, *LPDEWSYMBOLFONT;

#endif // _DEWSYMBOLFONT_H_