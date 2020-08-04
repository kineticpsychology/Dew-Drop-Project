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

#ifndef _APEENGINE_H_
#define _APEENGINE_H_

#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS
#endif

#include "WaveOutEngine.h"

#include "codecs/include/ape/All.h"
#include "codecs/include/ape/MACLib.h"
#include "codecs/include/ape/SmartPtr.h"
#include "codecs/include/ape/WindowsEnvironment.h"

#ifdef _DEBUG
#pragma comment(lib, "./Engine/codecs/lib/debug/MACLib.lib")
#else
#pragma comment(lib, "./Engine/codecs/lib/release/MACLib.lib")
#endif

typedef class APEENGINE : public WAVEOUTENGINE
{
    protected:
        APE::IAPEDecompress     *_pApeDec = NULL;
        UINT                    _nTotalBlocks = 0, _nBlockAlign = 0, _nTargetBlocks = 0;

        virtual void    _Cleanup();
        virtual BYTE    _Decode();
        virtual BYTE    _Seek(DWORD dwMS);

    public:
                        APEENGINE();
        virtual BYTE    Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE    Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual DWORD   Tell();
                        ~APEENGINE();
} APEENGINE, *LPAPEENGINE;

#endif
