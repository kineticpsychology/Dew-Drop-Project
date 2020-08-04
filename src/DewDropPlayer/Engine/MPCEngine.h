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

#ifndef _MPCENGINE_H_
#define _MPCENGINE_H_

#include "WaveOutEngine.h"

#include "codecs/include/mpc/datatypes.h"
#include "codecs/include/mpc/minimax.h"
#include "codecs/include/mpc/mpc_types.h"
#include "codecs/include/mpc/mpcdec.h"
#include "codecs/include/mpc/mpcmath.h"
#include "codecs/include/mpc/reader.h"
#include "codecs/include/mpc/streaminfo.h"

#ifdef _DEBUG
#pragma comment(lib, "./Engine/codecs/lib/debug/libmpccommon.lib")
#pragma comment(lib, "./Engine/codecs/lib/debug/libmpcdec.lib")
#else
#pragma comment(lib, "./Engine/codecs/lib/release/libmpccommon.lib")
#pragma comment(lib, "./Engine/codecs/lib/release/libmpcdec.lib")
#endif

#define MPC_BUFF_SAMPLE_SIZE        0x100
#define MPC_DECODED_BUFF_SIZE       MPC_BUFF_SAMPLE_SIZE * 2

typedef class MPCENGINE : public WAVEOUTENGINE
{
    protected:
        mpc_reader          _mpcReader;
        mpc_demux           *_pMpcDemux = NULL;
        FILE                *_fpMPC = NULL;
        mpc_uint64_t        _nTotalSamples = 0;
        MPC_SAMPLE_FORMAT   _sampleBuffer[MPC_DECODER_BUFFER_LENGTH];
        UINT                _iDecodedBuff[MPC_DECODED_BUFF_SIZE];
        UINT                _nBytesPerSample = 0;

        LPBYTE              _lpDecodedChunk = NULL;
        
        virtual void        _Float32ToInt16(float const *pSampleBuffer, LPBYTE lpOut, UINT nSampleCount);
        virtual void        _Cleanup();
        virtual BYTE        _Decode();
        virtual BYTE        _Seek(DWORD dwMS);

    public:
                        MPCENGINE();
        virtual BYTE    Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE    Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual DWORD   Tell();
                        ~MPCENGINE();
} MPCENGINE, *LPMPCENGINE;

#endif

