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

#ifndef _OPUSENGINE_H_
#define _OPUSENGINE_H_

#define OPUS_BUFF_SIZE          11520       // 120 (ms) * 48 (KHz) * 2 (always stereo)
#define OPUS_BUFF_SIZE_LE       23040       // 11520 * 2 (1 sample -> Little Endian)
#define OPUS_OUT_FREQ           48000       // Decode @ 48 KHz

#include "WaveOutEngine.h"

#include "codecs/include/opus/opusfile.h"

#ifdef _DEBUG
#pragma comment(lib, "./Engine/codecs/lib/debug/opusfile.lib")
#pragma comment(lib, "./Engine/codecs/lib/debug/opus.lib")
#else
#pragma comment(lib, "./Engine/codecs/lib/release/opusfile.lib")
#pragma comment(lib, "./Engine/codecs/lib/release/opus.lib")
#endif


typedef class OPUSENGINE : public WAVEOUTENGINE
{
    protected:
        opus_int16              _OpusSample[OPUS_BUFF_SIZE];
        BYTE                    _OpusSampleLE[OPUS_BUFF_SIZE_LE];
        BYTE                    _lpDecodedChunk[OPUS_BUFF_SIZE_LE]; // Hold overflows of data (if any)
        OggOpusFile             *_pOOF = NULL;
        ogg_uint64_t            _u64TotalSamples = 0;

        virtual void            _Cleanup();
        virtual BYTE            _Decode();
        virtual BYTE            _Seek(DWORD dwSeconds);

    public:
                                OPUSENGINE();
        virtual BYTE            Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE            Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual DWORD           Tell();
                                ~OPUSENGINE();
        static  BOOL            IsLoadable(LPCWSTR testFile);
} OPUSENGINE, *LPOPUSENGINE;

#endif
