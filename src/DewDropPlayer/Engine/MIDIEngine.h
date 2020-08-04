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

#ifndef _MIDIENGINE_H_
#define _MIDIENGINE_H_

#ifndef UNICODE
#define UNICODE
#endif

#include "IAudioEngine.h"
#include <objbase.h>
#include <DShow.h>
#include <initguid.h>
#include <math.h>
#include <Shlwapi.h>

#pragma comment (lib, "strmiids")
#pragma comment (lib, "version")


typedef class MIDIENGINE : public IAUDIOENGINE
{
    protected:
        IGraphBuilder           *_pIGB = NULL;
        IBasicAudio             *_pIBA = NULL;
        IMediaControl           *_pIMC = NULL;
        IMediaSeeking           *_pIMS = NULL;
        IBaseFilter             *_pIPFileFilter = NULL;
        IPin                    *_pFileOut = NULL;

        // Renderers
        IBaseFilter             *_pMIDIRenderer = NULL;

        // Renderer input pins
        IPin                    *_pMIDIIn = NULL;

        HANDLE                  _hTrdPoll = NULL;
        HANDLE                  _hPoll = NULL;
        BOOL                    _bInitialized = FALSE;
        LONGLONG                _llCurr = 0, _llTotal = 0;
        const DWORD             _dwPollFreq = 10;

        static DWORD            WINAPI _TrdPoll(LPVOID lpv);
        virtual void            _GetDSVersion();
        virtual IPin*           _GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir);
        virtual void            _GetExInfo();
        virtual void            _Cleanup();

    public:
                                MIDIENGINE();
        virtual BYTE            Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE            Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual BYTE            Play();
        virtual BYTE            Pause();
        virtual BYTE            Resume();
        virtual BYTE            Stop();
        virtual BYTE            Seek(DWORD dwMS);
        virtual DWORD           Tell();
        virtual BYTE            SetVolume(WORD wLevel);
        virtual BYTE            Unload();
                                ~MIDIENGINE();
} MIDIENGINE, *LPMIDIENGINE;

#endif
