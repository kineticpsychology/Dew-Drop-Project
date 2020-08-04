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

#ifndef _WMAENGINE_H_
#define _WMAENGINE_H_

#include "WaveOutEngine.h"

#include <wmsdk.h>

#ifdef _DEBUG
#pragma comment(lib, "wmvcore.lib")
#else
#pragma comment(lib, "wmvcore.lib")
#endif

typedef class WMAENGINE : public WAVEOUTENGINE
{
    protected:
        template <class T> void SafeRelease(T **ppT)
        {
            if (*ppT)
            {
                (*ppT)->Release();
                *ppT = NULL;
            }
        }

        template <class T> void SafeFree(T **ppT)
        {
            if (*ppT)
            {
                LocalFree(*ppT);
                *ppT = NULL;
            }
        }

        IWMSyncReader       *_pSyncReader = NULL;
        WORD                _wStreamIdx = 0xFFFF;
        DWORD               _dwOutNum = 0xFFFFFFFF;
        DWORD               _dwMaxOverflow = 0;
        DWORD               _dwCurrTime = 0;
        LPBYTE              _lpDecodedChunk = NULL;
        BOOL                _bStopInitiatedSeek = FALSE;
        virtual void        _Cleanup();
        virtual BYTE        _Decode();
        virtual void        _SetExtendedInfo();
        virtual BYTE        _Seek(DWORD dwMS);

    public:
                            WMAENGINE();
        virtual BYTE        Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE        Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual BYTE        Stop();
        virtual DWORD       Tell();
                            ~WMAENGINE();
} WMAENGINE, *LPWMAENGINE;

#endif
