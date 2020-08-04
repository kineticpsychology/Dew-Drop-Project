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

#ifndef _FLACENGINE_H_
#define _FLACENGINE_H_

#ifndef FLAC__NO_DLL
#define FLAC__NO_DLL
#endif

#include "WaveOutEngine.h"

#include "codecs/include/flac/all.h"
#include "codecs/include/flac/assert.h"
#include "codecs/include/flac/callback.h"
#include "codecs/include/flac/export.h"
#include "codecs/include/flac/format.h"
#include "codecs/include/flac/metadata.h"
#include "codecs/include/flac/ordinals.h"
#include "codecs/include/flac/stream_decoder.h"
#include "codecs/include/flac/stream_encoder.h"

#ifdef _DEBUG
#pragma comment(lib, "./Engine/codecs/lib/debug/libFLAC_static.lib")
#else
#pragma comment(lib, "./Engine/codecs/lib/release/libFLAC_static.lib")
#endif

typedef class FLACENGINE : public WAVEOUTENGINE
{
    protected:
        const WORD              _ENGINE_BITS_PER_SAMPLE = 16; // Always use 16 bits. Even for 24, downscale to 16
        FLAC__StreamDecoder     *_pFlacDec = NULL;
        FILE                    *_fpFlac = NULL;
        WORD                    _wBitsPerSample;
        FLAC__uint64            _u64TotalSamples;
        FLAC__uint64            _u64CurrentSample = 0;
        LPBYTE                  _lpDecodedChunk = NULL;
        DWORD                   _dwMaxDecodedSize = 0;
        DWORD                   _dwDecodedChunk = 0;

        virtual void            _Cleanup();
        virtual BYTE            _Decode();
        virtual BYTE            _Seek(DWORD dwMS);

        static FLAC__StreamDecoderWriteStatus _writeCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data);
        static void             _errorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);
        static void             _metadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);

    public:
                                FLACENGINE();
        virtual BYTE            Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE            Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual DWORD           Tell();
                                ~FLACENGINE();
        static  BOOL            IsLoadable(LPCWSTR testFile);
        static  BOOL            IsOggLoadable(LPCWSTR testFile);
} FLACENGINE, *LPFLACENGINE;

#endif
