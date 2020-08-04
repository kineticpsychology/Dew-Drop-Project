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

/**/
// TODO: Find a way to unlock the guard statement below
// Apparently uncommenting this currently, doesn't work
// since 'ISOM' appears alphabetically before 'MP3', it appears
// MP3Engine.h is NOT included the second time around :( ... bummer!
/*
#ifndef _MP3ENGINE_H_
#define _MP3ENGINE_H_
*/

#include "WaveOutEngine.h"

#include "codecs/include/mp3/mpg123.h"
#include "codecs/include/mp3/mpg123.h.in"
#include "codecs/include/mp3/fmt123.h"

#ifdef _DEBUG
#pragma comment(lib, "./Engine/codecs/lib/debug/libmpg123.lib")
#else
#pragma comment(lib, "./Engine/codecs/lib/release/libmpg123.lib")
#endif

typedef class MP3ENGINE : public WAVEOUTENGINE
{
    protected:
        mpg123_handle   *_hMP3 = NULL;
        LPBYTE          _lpDecodedChunk = NULL;
        long            _lTotalSamples = 0;
        virtual void    _Cleanup();
        virtual BYTE    _Decode();
        virtual BYTE    _Seek(DWORD dwMS);

    public:
                        MP3ENGINE();
        virtual BYTE    Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE    Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual DWORD   Tell();
                        ~MP3ENGINE();
        static BOOL     IsLoadable(LPCWSTR testFile);
} MP3ENGINE, *LPMP3ENGINE;

// #endif
