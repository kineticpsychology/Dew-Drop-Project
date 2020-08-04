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

#ifndef _AACENGINE_H_
#define _AACENGINE_H_

#include "WaveOutEngine.h"

#include "codecs/include/aac/neaacdec.h"
#include "codecs/include/aac/faad.h"

#ifdef _DEBUG
#pragma comment (lib, "./Engine/codecs/lib/debug/libfaad.lib")
#else
#pragma comment (lib, "./Engine/codecs/lib/release/libfaad.lib")
#endif


typedef class AACENGINE : public WAVEOUTENGINE
{
    protected:
        DWORD           _dwRawAACOffset = 0;
        DWORD           _dwFrameCount = 0;
        float           _fFPMS = 0; // Frames/Mili-Sec
        BYTE            _btMPEGVersion = 0;
        BYTE            _btAACProfile = 0;
        BYTE            _btProtectionAbsent = 0;
        LPBYTE          _lpDecodedChunk = NULL;

        LPDWORD         _FrameLookup = NULL;
        NeAACDecHandle  _naacDec = NULL;
        const DWORD     _AAC_PACK = 0x400;

        virtual BOOL    _ParseADTS();
        virtual void    _Cleanup();
        virtual BYTE    _Decode();
        virtual BYTE    _Seek(DWORD dwMS);

    public:
                        AACENGINE();
        virtual BYTE    Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE    Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual DWORD   Tell();
                        ~AACENGINE();
} AACENGINE, *LPAACENGINE;

#endif