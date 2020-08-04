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

#ifndef _CDDAENGINE_H_
#define _CDDAENGINE_H_

#define CDDA_FPS                75      // Frames per sec
#define CDDA_BYTES_PER_SECTOR   2352    // 176400/75
#define CDDA_CDA_STUB_FILE_SIZE 44


#include "WaveOutEngine.h"
#include <Ntddcdrm.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

typedef class CDDAENGINE : public WAVEOUTENGINE
{
    protected:
        HANDLE                  _hCD = NULL;
        DWORD                   _dwBytesPerSector = 0;
        DWORD                   _dwTrackAddrStart = 0, _dwTrackAddrEnd = 0;
        DWORD                   _dwTotalTrackLen;
        UINT                    _nTrackToPlay = 0;
        DISK_GEOMETRY           _driveGeometry;
        CDROM_TOC               _toc;
        RAW_READ_INFO           _rawInfo;
        BYTE                    _trackData[CDDA_BYTES_PER_SECTOR];

        HANDLE                  _hTwoBlocksReady = NULL, _hReleaseWrite = NULL;
        HANDLE                  _hTrdStream = NULL;
        DWORD                   _dwTotalThreadDecoded = 0;
        IStream                 *_pDecodedStream = NULL;
        BOOL                    _bReady = FALSE;

        virtual void            _Cleanup();
        virtual DWORD           _AddressToSector(const UCHAR Address[4]);
        static DWORD WINAPI     _TrdDecodeToStream(LPVOID lpv);
        virtual void            _DecodeToStream();
        virtual BYTE            _Decode();
        virtual BYTE            _Seek(DWORD dwMS);
    public:
                                CDDAENGINE();
        virtual BYTE            Load(HWND notificationWindow, LPCWSTR srcFile);
        virtual BYTE            Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize);
        virtual DWORD           Tell();
                                ~CDDAENGINE();

        static BOOL             PrepareCDDACode(LPCWSTR cdaFile, LPWSTR cddaCode, UINT cddaCodeLen);
} CDDAENGINE, *LPCDDAENGINE;

#endif
