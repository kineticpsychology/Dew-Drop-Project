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

#include "CDDAEngine.h"

/*
                                 READ ME
--------------------------------------------------------------------------------
***** THIS IS A MODDED IMPLEMENTATION OF AN EXISTING CODE-PROJECT ARTICLE ******
***** AUTHOR: MICHEL HELMS                                                ******
***** Link: https://www.codeproject.com/Articles/15725/Tutorial-on-reading-Audio-CDs
--------------------------------------------------------------------------------

You can easily find the documentation explaining the core code concept.
This modified implementation merely keeps continuouos data decoding on a
parallel stream. Since we are dealing with device IO (not always .mdx/.nrg
virtual audio cd you know), sometimes there will be latency while reading from
a physical audio CD. A parallel stream running on a thread should take care of
it mostly (if not fully).
--------------------------------------------------------------------------------
*/

// Release the stream and close down the events/thread
void CDDAENGINE::_Cleanup()
{
    if (_pDecodedStream) { _pDecodedStream->Release(); _pDecodedStream = NULL; }
    if (_hTwoBlocksReady) { CloseHandle(_hTwoBlocksReady); _hTwoBlocksReady = NULL; }
    if (_hReleaseWrite) { CloseHandle(_hReleaseWrite); _hReleaseWrite = NULL; }
    if (_hTrdStream) { TerminateThread(_hTrdStream, NULL); _hTrdStream = NULL; }
    return;
}

// Address to sector converter
DWORD CDDAENGINE::_AddressToSector(const UCHAR Address[4])
{
    return (Address[1] * CDDA_FPS * 60 + Address[2] * CDDA_FPS + Address[3]) - (CDDA_FPS * 2);
}

// This thread delegates the stream decoding to the class method
DWORD WINAPI CDDAENGINE::_TrdDecodeToStream(LPVOID lpv)
{
    LPCDDAENGINE    lpCDDA;

    lpCDDA = (LPCDDAENGINE)lpv;
    lpCDDA->_DecodeToStream();
    lpCDDA->_hTrdStream = NULL;
    return 0;
}

// Parallel decoding to stream
void CDDAENGINE::_DecodeToStream()
{
    DWORD       dwDecoded, dwDecodePos;
    LARGE_INTEGER   li_Move { 0 };

    // dwDecodePos: Position marker w.r.t the Audio CD
    // dwCurrentPos: Position marker w.r.t. the current track
    dwDecodePos = _dwCurrentPos;
    _dwCurrentPos = 0;
    _dwTotalThreadDecoded = 0;
    _pDecodedStream = SHCreateMemStream(NULL, 0);
    if (_pDecodedStream == NULL) return;

    do
    {
        // Raw decoding logic
        _rawInfo.DiskOffset.QuadPart = (dwDecodePos)* _driveGeometry.BytesPerSector;
        DeviceIoControl(_hCD, IOCTL_CDROM_RAW_READ, &_rawInfo, sizeof(RAW_READ_INFO),
            _trackData, CDDA_BYTES_PER_SECTOR, &dwDecoded, NULL);
        li_Move.LowPart = _dwTotalThreadDecoded;
        // Block the 'hReleaseWrite' event so that if a parallel 'Decode' runs
        // it will NOT be able to interfere with the decoded data being written
        // at a wrong position (since reading decoded data is ALWAYS, ALWAYS
        // going to be lagging far behind the decoding speed).
        ResetEvent(_hReleaseWrite);
        _pDecodedStream->Seek(li_Move, STREAM_SEEK_SET, NULL);
        _pDecodedStream->Write(_trackData, dwDecoded, &dwDecoded);
        _dwTotalThreadDecoded += dwDecoded;
        // If the decoded data is enough to fill the first CHUNKCOUNT number of
        // blocks then signal the 'hTwoBlocksReady' event, so that
        // 'Load' method can continue on its own merry way. In the meanwhile,
        // this thread will continue decoding data into the stream
        if (_dwTotalThreadDecoded >= CHUNKSIZE * CHUNKCOUNT && !_bReady)
        {
            _bReady = TRUE;
            SetEvent(_hTwoBlocksReady);
        }
        dwDecodePos++;
        // Signal the 'hReleaseWrite' event after each write so that
        // if a parallel 'Decode' runs, it will be able to read from the stream
        SetEvent(_hReleaseWrite);
    } while (dwDecodePos < _dwTrackAddrEnd);
    return;
}

// The decoding logic
BYTE CDDAENGINE::_Decode()
{
    DWORD       dwDecoded = 0;
    LARGE_INTEGER   li_Seek { 0 };

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;
    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;
    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);

    // Set the stream seek position
    li_Seek.LowPart = _dwCurrentPos;
    // Check if data will overflow beyond the stream size limit
    if (_dwCurrentPos + CHUNKSIZE > _dwTotalTrackLen)
    {
        // If so, pick only so much till the end of the stream
        dwDecoded = (_dwCurrentPos + CHUNKSIZE) - _dwTotalTrackLen;
        _bNoMoreData = TRUE;
    }
    else
    {
        // Otherwise pick the full CHUNKSIZE
        dwDecoded = CHUNKSIZE;
    }
    // Now before reading, make sure decoding is not interfered.
    // So make sure that the decoding is complete by waiting for it to signal
    // the event
    WaitForSingleObject(_hReleaseWrite, INFINITE);
    _pDecodedStream->Seek(li_Seek, STREAM_SEEK_SET, NULL);
    _pDecodedStream->Read(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), dwDecoded, &dwDecoded);
    _dwCurrentPos += dwDecoded;
    return _bNoMoreData ? DEWDEC_DECODE_EOD : DEWDEC_DECODE_OK;
}

// Seeking will be done on the variable (dwCurrentPos) w.r.t. current track
BYTE CDDAENGINE::_Seek(DWORD dwMS)
{
    DWORD       dwSeekPos;

    // Do not allow any seeking unless the player is playing or paused
    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        // Simplest. CD always bas a BYTE rate of 176400
        dwSeekPos = (DWORD)((float)dwMS * 176.4f);
        if (dwSeekPos > _dwTotalTrackLen)
            dwSeekPos = _dwTotalTrackLen;
        _dwCurrentPos = dwSeekPos;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}

// Basic initialization (media type and library)
CDDAENGINE::CDDAENGINE()
{
    _btMediaType = DEWMT_CDDA;
    StringCchPrintf(_wsLibrary, MAX_PATH, L"Win32 API (DeviceIoControl) library");
}

// Parsing and loading of the Audio CD
// As metioned at the very top, go voer the beautiful documenation
// from the CodeProject link to get the understanding of this works.
//
// ** NOTE: Unlike other engines, the srcFile parameter can not only
// accept .cda files but also a string with the format: <DRIVE>|<TRACK_NO>
// e.g. If an audio CD is mounted on the F:\ drive and the 4th track needs to be
// played, then the correct string for srcFile will be: "F|4" (without the
// double quotes
BYTE CDDAENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    wchar_t         wsDrive[8], cddaCode[8];
    DWORD           dwRead = 0, dwFirstSector = 0, dwLastSector = 0;
    WAVEFORMATEX    wfex { 0 };
    wchar_t         wsValue[EXINFO_VAL_SIZE];
    MMRESULT        mmr = 0;

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;

    _hWndNotify = notificationWindow;

    StringCchPrintf(cddaCode, 8, L"%s", srcFile);
    if (!StrChr(cddaCode, L'|'))
    {
        if (!CDDAENGINE::PrepareCDDACode(srcFile, cddaCode, 8))
        {
            return DEWERR_INVALID_FILE;
        }
        else
        {
            if (!StrChr(cddaCode, L'|'))
            {
                return DEWERR_INVALID_PARAM;
            }
        }
    }
    
    StringCchPrintf(wsDrive, 8, L"\\\\.\\?:");
    CopyMemory(&wsDrive[4], cddaCode, 1 * sizeof(wchar_t));

    ZeroMemory(&_toc, sizeof(CDROM_TOC));
    _hCD = CreateFile(wsDrive, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (_hCD == NULL || _hCD == INVALID_HANDLE_VALUE)
        return DEWERR_INVALID_FILE;

    if (!DeviceIoControl(_hCD, IOCTL_CDROM_READ_TOC, 0, 0, &_toc, sizeof(CDROM_TOC), &dwRead, NULL))
        return DEWERR_ENGINE_STARTUP;

    if (!DeviceIoControl(_hCD, IOCTL_CDROM_GET_DRIVE_GEOMETRY, NULL, 0, &_driveGeometry, sizeof(DISK_GEOMETRY), &dwRead, NULL))
        return DEWERR_ENGINE_STARTUP;

    _dwBytesPerSector = _driveGeometry.BytesPerSector;
    dwLastSector = _AddressToSector(_toc.TrackData[_toc.LastTrack].Address);
    dwFirstSector = _AddressToSector(_toc.TrackData[_toc.FirstTrack - 1].Address);

    _dwTotalTrackLen = (dwLastSector - dwFirstSector) * CDDA_BYTES_PER_SECTOR;
    _dwTotalTrackLen /= 176400; // CD always has 176400 bytes per sec

    StringCchPrintf(wsDrive, 8, L"%s", &cddaCode[2]);
    _nTrackToPlay = (UINT)_wtoi(wsDrive);
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"Track %02u", _nTrackToPlay);

    _dwExInfoLength = 3;
    _lpExInfo = (LPEXTENDEDINFO)LocalAlloc(LPTR, sizeof(EXTENDEDINFO) * _dwExInfoLength);
    _lpExInfo[0].SetType(L"Current Track");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", _nTrackToPlay);
    _lpExInfo[0].SetValue(wsValue);
    _lpExInfo[1].SetType(L"Total Tracks");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%u", _toc.LastTrack - _toc.FirstTrack + 1);
    _lpExInfo[1].SetValue(wsValue);
    _lpExInfo[2].SetType(L"Total Play Time");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%02u:%02u", _dwTotalTrackLen / 60, _dwTotalTrackLen % 60);
    _lpExInfo[2].SetValue(wsValue);

    _dwTrackAddrStart = _AddressToSector(_toc.TrackData[_nTrackToPlay - 1].Address);
    _dwTrackAddrEnd = _AddressToSector(_toc.TrackData[_nTrackToPlay].Address);
    _dwTotalTrackLen = (_dwTrackAddrEnd - _dwTrackAddrStart) * CDDA_BYTES_PER_SECTOR;
    _dwCurrentPos = _dwTrackAddrStart;

    _dwSampleRate = 44100;
    _btChannels = 2;
    _dwDuration = (DWORD)((float)_dwTotalTrackLen / 176.4f);
    _dwBitrate = 44100 * 16 * _btChannels;

    _wfex.cbSize = 0;
    _wfex.wFormatTag = WAVE_FORMAT_PCM;
    _wfex.wBitsPerSample = 16;
    _wfex.nChannels = _btChannels;
    _wfex.nSamplesPerSec = _dwSampleRate;
    _wfex.nBlockAlign = (_wfex.wBitsPerSample >> 3) * _wfex.nChannels;
    _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;

    ZeroMemory(&_rawInfo, sizeof(RAW_READ_INFO));
    _rawInfo.TrackMode = CDDA;
    _rawInfo.SectorCount = 1;

    _hTwoBlocksReady = CreateEvent(NULL, TRUE, FALSE, L"CDDA.TWO.BLOCKS");
    if (_hTwoBlocksReady == NULL || _hTwoBlocksReady == INVALID_HANDLE_VALUE)
        return DEWERR_ENGINE_STARTUP;

    _hReleaseWrite = CreateEvent(NULL, TRUE, FALSE, L"CDDA.STREAM.WRITE");
    if (_hReleaseWrite == NULL || _hReleaseWrite == INVALID_HANDLE_VALUE)
        return DEWERR_ENGINE_STARTUP;

    _hTrdStream = CreateThread(NULL, 0, _TrdDecodeToStream, this, 0, NULL);
    if (_hTrdStream == NULL || _hTrdStream == INVALID_HANDLE_VALUE)
        return DEWERR_ENGINE_STARTUP;
    WaitForSingleObject(_hTwoBlocksReady, INFINITE);

    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;

}

// Byte mode file loading is not supported
BYTE CDDAENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    return DEWERR_NOT_IMPLEMENTED;
}

// Simple.CD always bas a BYTE rate of 176400s
DWORD CDDAENGINE::Tell()
{
    return (DWORD)((float)_dwCurrentPos/176.4f);
}

// Housekeeping & cleanup
CDDAENGINE::~CDDAENGINE()
{
    this->_Cleanup();
    return;
}

// Static method to convert a .cda file extension to <DRIVE>|<TRACK_NO> format
// because that is what it expects as 'srcFile' argument in the 'Load' function
BOOL CDDAENGINE::PrepareCDDACode(LPCWSTR cdaFile, LPWSTR cddaCode, UINT cddaCodeLen)
{
    HANDLE          hCDA = NULL;
    DWORD           dwCDASize = 0, dwData = 0, dwRead;
    WORD            wTrack = 0;
    const DWORD     keyRIFF = 0x46464952; // 'RIFF' text
    const DWORD     keyCDDA = 0x41444443; // 'CDDA' text
    const DWORD     keyCDASize = 0x2C; // 44 bytes
    const DWORD     keyCDDAOffset = 0x08; // Position offset for CDDA
    const DWORD     keyTrkOffset = 0x16; // Position indicator for track#
    BOOL            bOK = FALSE;

    ZeroMemory(cddaCode, cddaCodeLen);

    hCDA = CreateFile(cdaFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hCDA == NULL || hCDA == INVALID_HANDLE_VALUE)
        return bOK;
    dwCDASize = GetFileSize(hCDA, NULL);
    if (dwCDASize != keyCDASize) goto __pcc_cleanup;
    ReadFile(hCDA, &dwData, sizeof(DWORD), &dwRead, NULL);
    if (dwData != keyRIFF) goto __pcc_cleanup;
    SetFilePointer(hCDA, keyCDDAOffset, NULL, FILE_BEGIN);
    ReadFile(hCDA, &dwData, sizeof(DWORD), &dwRead, NULL);
    if (dwData != keyCDDA) goto __pcc_cleanup;
    SetFilePointer(hCDA, keyTrkOffset, NULL, FILE_BEGIN);
    ReadFile(hCDA, &wTrack, sizeof(WORD), &dwRead, NULL);
    
    StringCchPrintf(cddaCode, cddaCodeLen, L"%c|%u", cdaFile[0], wTrack);
    bOK = TRUE;

__pcc_cleanup:
    CloseHandle(hCDA);
    return bOK;
}
