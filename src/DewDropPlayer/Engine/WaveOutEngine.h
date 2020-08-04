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

#ifndef _WAVEOUTENGINE_H_
#define _WAVEOUTENGINE_H_

#include "IAudioEngine.h"

typedef class WAVEOUTENGINE : public IAUDIOENGINE
{
    protected:
        HANDLE                  _hTrdPlay = NULL;
        HANDLE                  _hEvtPrepare = NULL, _hEvtDecode = NULL;
        LPBYTE                  _lpDecodedData = NULL, _lpOverflowData = NULL;
        HWAVEOUT                _hWO = NULL;
        WAVEHDR                 *_blocks = NULL;

        DWORD                   CHUNKSIZE = 0;
        DWORD                   CHUNKCOUNT = 0;
        WAVEFORMATEX            _wfex{ 0 };
        DWORD                   _dwCurrentIndex = 0, _dwCurrentPos = 0, _dwOverflow = 0;
        BOOL                    _bNoMoreData = FALSE, _bOverloadedLoadFunctionCall = FALSE, _bVBR = FALSE, _bStopDecoding = FALSE;

        static void             CALLBACK _WAVHandler(HWAVEOUT hwo, UINT nMsg, DWORD_PTR dwRefData, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        virtual void            _WAVProc(HWAVEOUT hwo, UINT nMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        static DWORD            WINAPI _TrdPlay(LPVOID lpv);
        virtual void            _Cleanup();
        virtual BYTE            _Decode() = 0;
        virtual BYTE            _Seek(DWORD dwSeconds) = 0;
        
    public:
                                WAVEOUTENGINE();
        virtual BYTE            Load(HWND notificationWindow, LPCWSTR srcFile) = 0;
        virtual BYTE            Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize) = 0;
        virtual BYTE            Play();
        virtual BYTE            Pause();
        virtual BYTE            Resume();
        virtual BYTE            Stop();
        // Make sure this is not implemented/overridden in child classes!
        // Instead, use the internal function _Seek(). This public function
        // will stop the decoding, seek and then resume decoding.
                BYTE            Seek(DWORD dwMS);
        virtual BYTE            SetVolume(WORD wLevel);
        virtual BYTE            Unload();
        virtual                 ~WAVEOUTENGINE();
} WAVEOUTENGINE, *LPWAVEOUTENGINE;

#endif
