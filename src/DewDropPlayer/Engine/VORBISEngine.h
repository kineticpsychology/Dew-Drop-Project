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

#ifndef _VORBISENGINE_H_
#define _VORBISENGINE_H_

#include "WaveOutEngine.h"

#include "codecs/include/ogg/ogg.h"
#include "codecs/include/ogg/os_types.h"
#include "codecs/include/vorbis/codec.h"
#include "codecs/include/vorbis/vorbisenc.h"
#include "codecs/include/vorbis/vorbisfile.h"


#ifdef _DEBUG
#pragma comment(lib, "./Engine/codecs/lib/debug/libogg_static.lib")
#pragma comment(lib, "./Engine/codecs/lib/debug/libvorbis_static.lib")
#pragma comment(lib, "./Engine/codecs/lib/debug/libvorbisfile_static.lib")
#else
#pragma comment(lib, "./Engine/codecs/lib/release/libogg_static.lib")
#pragma comment(lib, "./Engine/codecs/lib/release/libvorbis_static.lib")
#pragma comment(lib, "./Engine/codecs/lib/release/libvorbisfile_static.lib")
#endif

typedef class VORBISENGINE : public WAVEOUTENGINE
{
    protected:
        OggVorbis_File  _OVF;
        FILE            *_fp = NULL;
        const DWORD     _VORBISCHUNK = 0x1000;
        LPBYTE          _lpDecodedChunk = NULL;
        ogg_int64_t     _nTotalSamples = 0;

        virtual void    _Cleanup();
        virtual BYTE    _Decode();
        virtual BYTE    _Seek(DWORD dwMS);

    public:
                        VORBISENGINE();
        virtual BYTE    Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE    Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual DWORD   Tell();
                        ~VORBISENGINE();
        static BOOL     IsLoadable(LPCWSTR testFile);
} VORBISENGINE, *LPVORBISENGINE;

#endif
