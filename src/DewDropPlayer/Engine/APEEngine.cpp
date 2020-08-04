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

#include "APEEngine.h"

/*
 __.
(@.@)
 (_)

*/

// Delete the interface pointer
void APEENGINE::_Cleanup()
{
    if (_pApeDec)
    {
        delete _pApeDec;
        _pApeDec = NULL;
    }
    return;
}

// Main decoding function
BYTE APEENGINE::_Decode()
{
    APE::int64       nDecoded = 0;

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;

    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;

    // Decoding will *ALWAYS* happen in Blocks * Block Align number of bytes
    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex* CHUNKSIZE]), CHUNKSIZE);
    // Decode (best one-liner!)
    _pApeDec->GetData((char*)(&(_lpDecodedData[_dwCurrentIndex*CHUNKSIZE])), _nTargetBlocks, &nDecoded);
    // Premature completion. This means end of data has been reached
    if (nDecoded < (APE::intn)_nTargetBlocks)
    {
        _bNoMoreData = TRUE;
        return DEWDEC_DECODE_EOD;
    }
    return DEWDEC_DECODE_OK;
}

// The library is so slow in decoding that runtime seek crashes the decoder thread!
BYTE APEENGINE::_Seek(DWORD dwMS)
{
    APE::intn   nSeekBlock = 0;
    BYTE        btOldStatus = DEWS_MEDIA_NONE;
    MMRESULT    mmr = MMSYSERR_NOERROR;
    UINT        nIndex;

    if (!_hWO || _hWO == INVALID_HANDLE_VALUE) return DEWERR_MM_ACTION;
    if (!_pApeDec) return DEWERR_MM_ACTION;

    // Unlike typical cases, Seek will work in two modes:
    // One when playing/paused, and the other when
    // Media is stopped.

    // When playing/paused, we will break both the threads (play/WavProc)
    // reset all the events and then seek to the intended position.
    // This will ensure GetData() is NEVER called in parallel to Seek()
    if ((_btStatus == DEWS_MEDIA_PLAYING || _btStatus == DEWS_MEDIA_PAUSED) &&
        !_bNoMoreData)
    {
        btOldStatus = _btStatus;
        if (dwMS > _dwDuration)
            dwMS = _dwDuration;
        if (_dwDuration == 0) return DEWERR_SUCCESS;

        // Calculate the block to seek to
        nSeekBlock = (DWORD)(((double)dwMS/(double)_dwDuration) * (double)_nTotalBlocks);

        // For some weird reason, _pApeDec->Seek causes 
        // _pApeDec->GetData() to crash. Only way to deal with this
        // is to stop everything, seek, take a breather (waveOutWrite)
        // and start everything all over again!
        if (this->_hTrdPlay)
        {
            TerminateThread(this->_hTrdPlay, 0x00);
            this->_hTrdPlay = NULL;
        }
        mmr = waveOutReset(_hWO);
        if (mmr != MMSYSERR_NOERROR) return DEWERR_MM_ACTION;
        ResetEvent(this->_hEvtDecode);
        // Now that the ever-running cycle of events are closed
        // do the Seek peacefully!
        _pApeDec->Seek(nSeekBlock);

        // Kind of the same activity as Play, only without the
        // WAVEHDR preparation part
        for (nIndex = 0; nIndex < CHUNKCOUNT; nIndex++)
        {
            _dwCurrentIndex = nIndex;
            this->_Decode(); // Keep the data decoded
        }
        // Kick-start the monitoring/decoding thread
        this->_hTrdPlay = CreateThread(NULL, 0, _TrdPlay, this, 0, NULL);

        // Now start the event-based cycle with the waveOutWrite trigger
        for (nIndex = 0; nIndex < CHUNKCOUNT; nIndex++)
        {
            mmr = waveOutWrite(_hWO, &_blocks[nIndex], sizeof(WAVEHDR));
            if (mmr != MMSYSERR_NOERROR) return DEWERR_MM_ACTION;
        }
        return DEWERR_SUCCESS;
    }
    else if (_bNoMoreData) // Means Stop() function called the Seek
    {
        if (!_hWO || _hWO == INVALID_HANDLE_VALUE) return DEWERR_MM_ACTION;
        _pApeDec->Seek(0);
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}


// Nothing to be done
APEENGINE::APEENGINE()
{
    _btMediaType = DEWMT_APE;
    StringCchPrintf(_wsLibrary, MAX_PATH, L"MACLib (MAC SDK) version 5.48");
    return;
}

// The implemented Loader function
BYTE APEENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    int             iError = 0;
    APE::int64       pVal = 0;
    wchar_t         wsValue[EXINFO_VAL_SIZE];

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;

    // Set the notification window
    _hWndNotify = notificationWindow;
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", srcFile);

    // Start the decoder interface
    _pApeDec = CreateIAPEDecompress(_wsSrcFile, &iError);
    // Bail on error
    if (iError != ERROR_SUCCESS) return DEWERR_INVALID_FILE;

    // Common info + wfex structure population
    pVal = _pApeDec->GetInfo(APE::APE_DECOMPRESS_LENGTH_MS, 0, 0);
    _dwDuration = (DWORD)pVal;
    pVal = _pApeDec->GetInfo(APE::APE_DECOMPRESS_AVERAGE_BITRATE, 0, 0);
    _dwBitrate = (DWORD)pVal * 1000;
    pVal = _pApeDec->GetInfo(APE::APE_INFO_WAVEFORMATEX, (APE::intn)&_wfex, 0);
    _btChannels = (BYTE)_wfex.nChannels;
    _dwSampleRate = _wfex.nSamplesPerSec;

    // Other decoding calculation related info
    _nBlockAlign = _wfex.nBlockAlign;
    pVal = _pApeDec->GetInfo(APE::APE_DECOMPRESS_TOTAL_BLOCKS, 0, 0);
    _nTotalBlocks = (DWORD)pVal;
    _nTargetBlocks = CHUNKSIZE / _nBlockAlign;

    _dwExInfoLength = 4;
    pVal = _pApeDec->GetInfo(APE::APE_INFO_FILE_VERSION, 0, 0); // 3990 -> 3.99
    _lpExInfo = (LPEXTENDEDINFO)LocalAlloc(LPTR, sizeof(EXTENDEDINFO) * _dwExInfoLength);

    _lpExInfo[0].SetType(L"Encoder Version");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%.3f", float(_pApeDec->GetInfo(APE::APE_INFO_FILE_VERSION)) / float(1000));
    _lpExInfo[0].SetValue(wsValue);

    _lpExInfo[1].SetType(L"Compression Level");
    switch ((UINT)_pApeDec->GetInfo(APE::APE_INFO_COMPRESSION_LEVEL))
    {
        case MAC_COMPRESSION_LEVEL_FAST: _lpExInfo[1].SetValue(L"Fast (Low - 1000)"); break;
        case MAC_COMPRESSION_LEVEL_NORMAL: _lpExInfo[1].SetValue(L"Normal (Medium - 2000)"); break;
        case MAC_COMPRESSION_LEVEL_HIGH: _lpExInfo[1].SetValue(L"High (3000)"); break;
        case MAC_COMPRESSION_LEVEL_EXTRA_HIGH: _lpExInfo[1].SetValue(L"Extra High (4000)"); break;
        case MAC_COMPRESSION_LEVEL_INSANE: _lpExInfo[1].SetValue(L"Insane (5000)"); break;
        default: _lpExInfo[1].SetValue(L"Unknown (?)"); break;
    }

    _lpExInfo[2].SetType(L"APL Format");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%s", _pApeDec->GetInfo(APE::APE_INFO_APL) == 1 ? L"Yes" : L"No");
    _lpExInfo[2].SetValue(wsValue);

    _lpExInfo[3].SetType(L"Peak Level");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%d", _pApeDec->GetInfo(APE::APE_INFO_PEAK_LEVEL));
    _lpExInfo[3].SetValue(wsValue);

    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// We won't support byte/memory based data feed
BYTE APEENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    return DEWERR_NOT_IMPLEMENTED;
}

// Use the native API to tell the time
DWORD APEENGINE::Tell()
{
    if (_pApeDec)
        return (DWORD)(_pApeDec->GetInfo(APE::APE_DECOMPRESS_CURRENT_MS, 0, 0));
    return 0;
}

// Housekeeping
APEENGINE::~APEENGINE()
{
    this->_Cleanup();
    return;
}
