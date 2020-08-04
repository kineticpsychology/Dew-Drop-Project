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

#include "AIFFEngine.h"

// Release the library and the transient decode buffer
void AIFFENGINE::_Cleanup()
{
    if (_bAIFFLibInitialized)
    {
        AIFFLIB_Flush();
        _bAIFFLibInitialized = FALSE;
    }
    if (_lpDecodedChunk)
    {
        LocalFree(_lpDecodedChunk);
        _lpDecodedChunk = NULL;
    }
    return;
}

// The main decoding logic
BYTE AIFFENGINE::_Decode()
{
    DWORD       dwDecoded = 0, dwCumulDecoded = 0, dwRet = 0;

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;
    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;

    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);

    dwCumulDecoded = _dwOverflow;
    // Copy whatever was left (dwOverflow)
    if (_dwOverflow > 0)
        CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), _lpDecodedChunk, _dwOverflow);
    ZeroMemory(_lpDecodedChunk, CHUNKSIZE);

    do
    {
        dwDecoded = CHUNKSIZE;
        // Call the custom library to do the decoding
        dwRet = AIFFLIB_Decode(_lpDecodedChunk, &dwDecoded);
        if (dwRet != AIFFLIB_ERROR_SUCCESS || dwDecoded <= 0)
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }

        // Same old logic to check for decoded data overflow
        if ((dwCumulDecoded + dwDecoded) >= CHUNKSIZE)
        {
            // Keep the overflow amount in the lpDecodedChunk buffer
            _dwOverflow = (dwCumulDecoded + dwDecoded) - CHUNKSIZE;
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _lpDecodedChunk, CHUNKSIZE - dwCumulDecoded);
            CopyMemory(_lpDecodedChunk, &(_lpDecodedChunk[CHUNKSIZE - dwCumulDecoded]), _dwOverflow);
            return DEWDEC_DECODE_OK;
        }
        else
        {
            // Not yet there. Add up the cumulative decoded data value
            // and continue with the looped decoding
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _lpDecodedChunk, dwDecoded);
            dwCumulDecoded += dwDecoded;
        }
    } while (TRUE);
    return DEWDEC_DECODE_ERROR;
}

// Seek based on the sample index provided to the custom aiff library
BYTE AIFFENGINE::_Seek(DWORD dwMS)
{
    double      dDuration = 0, dSeekTime;
    DWORD       dwSeekSample;
    int         iResult = 0;

    // Do not allow any seeking unless the player is playing or paused
    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        // Calculate the pointer distance by time ratio (use double for more accuracy)
        dDuration = (double)_dwDuration;
        dSeekTime = (double)dwMS;

        if (dDuration == 0) return DEWERR_SUCCESS;
        if (dSeekTime >= dDuration)
            dwSeekSample = _dwTotalSamples;
        else
            dwSeekSample = (DWORD)((dSeekTime * ((double)_dwTotalSamples)) / dDuration);
        if (AIFFLIB_Seek(dwSeekSample) != AIFFLIB_ERROR_SUCCESS) return DEWERR_MM_ACTION;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}

// Initalizations. Alo set the transient decoding buffer to CHUNKSIZE
AIFFENGINE::AIFFENGINE()
{
    this->_btMediaType = DEWMT_AIFF;
    StringCchPrintf(_wsLibrary, MAX_PATH, L"libaiff (MSVC) version 1.1");
    _lpDecodedChunk = (LPBYTE)LocalAlloc(LPTR, CHUNKSIZE);
}

BYTE AIFFENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    MMRESULT        mmr = 0;
    DWORD           dwInit = 0;

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;

    _hWndNotify = notificationWindow;
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", srcFile);
    dwInit = AIFFLIB_Init(_wsSrcFile, &_btChannels, &_dwSampleRate, &_dwTotalSamples,
                          &_wBitsPerSample, &_dwDuration, &_dwBitrate);
    if (dwInit != AIFFLIB_ERROR_SUCCESS)
        return DEWERR_ENGINE_STARTUP;

    _bAIFFLibInitialized = TRUE;

    _wfex.wFormatTag = WAVE_FORMAT_PCM;
    _wfex.cbSize = 0;
    _wfex.nChannels = (WORD)_btChannels;
    _wfex.nSamplesPerSec = _dwSampleRate;
    _wfex.wBitsPerSample = _wBitsPerSample;
    _wfex.nBlockAlign = (_wfex.nChannels * _wfex.wBitsPerSample) / 8;
    _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;

    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// Byte-mode data loading is not supported
BYTE AIFFENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    return DEWERR_NOT_IMPLEMENTED;
}

// Direct call to the custom library to get the time
DWORD AIFFENGINE::Tell()
{
    DWORD       dwCurrSample = 0;

    dwCurrSample = AIFFLIB_Tell();
    if (dwCurrSample == (DWORD)INVALID_HANDLE_VALUE) return DEWERR_MM_ACTION;
    return (DWORD)((((double)dwCurrSample) * ((double)_dwDuration)) / ((double)_dwTotalSamples));
}

// Housekeeping & cleanup
AIFFENGINE::~AIFFENGINE()
{
    this->_Cleanup();
    return;
}
