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

#ifndef _AC3ENGINE_H_
#define _AC3ENGINE_H_

#include <stdint.h>
#include "WaveOutEngine.h"

#include "codecs/include/ac3/a52.h"
#include "codecs/include/ac3/attributes.h"
#include "codecs/include/ac3/audio_out.h"
#include "codecs/include/ac3/mm_accel.h"
#include "codecs/include/ac3/tendra.h"

#define AC3_BLOCKS_PER_FRAME                6
#define AC3_SAMPLES_PER_BLOCK_PER_CHANNEL   256
#define AC3_INTENDED_OUT_CHANNELS           2
#define AC3_BLOCK_DECODED_SIZE              1024 // AC3_SAMPLES_PER_BLOCK_PER_CHANNEL * AC3_INTENDED_OUT_CHANNELS * Bit depth (16) / 2
#define AC3_FRAME_DECODED_SIZE              6144 // AC3_BLOCK_DECODED_SIZE * AC3_BLOCKS_PER_FRAME

#ifdef _DEBUG
#pragma comment (lib, "./Engine/codecs/lib/debug/liba52.lib")
#else
#pragma comment (lib, "./Engine/codecs/lib/release/liba52.lib")
#endif

typedef class AC3ENGINE : public WAVEOUTENGINE
{
    private:
        int16_t         _convert(int32_t i);
        void            _float2s16_2(float * _f, int16_t * s16);

    protected:
        BYTE            _lpDecodedChunk[AC3_FRAME_DECODED_SIZE];
        DWORD           _dwTotalSamples = 0;
        a52_state_t     *_ac3State = NULL;
        LPDWORD         _FrameLookup = NULL;
        DWORD           _dwFrameCount = 0;
        float           _fFPMS; // Frames / Milli-Secs

        virtual void    _ParseA52Frames();
        virtual void    _Cleanup();
        virtual BYTE    _Decode();
        virtual BYTE    _Seek(DWORD dwMS);

    public:
                        AC3ENGINE();
        virtual BYTE    Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE    Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual DWORD   Tell();
                        ~AC3ENGINE();
        static BOOL     IsLoadable(LPCWSTR testFile);
} AC3ENGINE, *LPAC3ENGINE;

#endif
