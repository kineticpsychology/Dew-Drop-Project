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

#ifndef _DEWDROPAPP_H_
#define _DEWDROPAPP_H_

#include "DewStartup.h"
#include "DewWindow.h"

// WARNING: THIS IS THE MAIN STARTUP. DO NOT INHERIT/MODIFY AT ANY COST!!!
typedef class DEWDROPAPP
{
    private:
        static ULONG_PTR            _sgdiToken;
        static GdiplusStartupInput  _sgsi;
        static UINT                 _snInstance;
        static BOOL                 _sbRunExecuted;
        static int                  _siRun();

    public:
                DEWDROPAPP();
        int     Run();
                ~DEWDROPAPP();
} DEWDROPAPP, *LPDEWDROPAPP;

#endif // _DEWDROPAPP_H_
