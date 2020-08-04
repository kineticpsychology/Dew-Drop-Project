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

#ifndef _ALACENGINE_H_
#define _ALACENGINE_H_

#include "WaveOutEngine.h"

#include "codecs/include/alac/ALACLib.h"

#ifdef _DEBUG
#pragma comment (lib, "./Engine/codecs/lib/Debug/alaclib.lib")
#else
#pragma comment (lib, "./Engine/codecs/lib/Release/alaclib.lib")
#endif

typedef class ALACENGINE : public WAVEOUTENGINE
{
    protected:
        BOOL            _bALACLibInitialized = FALSE;
        LPBYTE          _lpDecodedChunk = NULL;
        DWORD           _dwMaxDecodeSize = 0;
        LPDWORD         _SampleLookup = NULL;
        DWORD           _dwTotalSamples = 0;
        float           _fSPMS; // Samples per milli-second (for seeking/telling)

        virtual void    _Cleanup();
        virtual BYTE    _Decode();
        virtual BYTE    _Seek(DWORD dwMS);

    public:
                        ALACENGINE();
        virtual BYTE    Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE    Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual BYTE    Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize, LPDWORD sampleTable, DWORD dwTableSize, BYTE channelCount, DWORD sampleRate, DWORD duration);
        virtual DWORD   Tell();
                        ~ALACENGINE();
} ALACENGINE, *LPALACENGINE;

#endif
