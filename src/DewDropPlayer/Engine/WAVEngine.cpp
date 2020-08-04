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

#include "WAVEngine.h"

// MAIN DECODING LOGIC
// Driven by the CHUNKSIZE-block index to populate
BYTE WAVENGINE::_Decode()
{
    UINT        nSize = 0, nIndex = 0;
    DWORD       dwSizeToCopy;

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;
    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;

    // Check the net amount of (decoded) data to be placed into the buffer
    // This is  going to be 1:1 copy for 16 bits, but 1:1.5 copy for 24 bits
    dwSizeToCopy = _bIs24Bit ? (CHUNKSIZE * 3) / 2 : CHUNKSIZE;

    if (_dwCurrentPos + dwSizeToCopy > _dwSrcDataSize)
        nSize = _dwSrcDataSize - _dwCurrentPos;
    else
        nSize = dwSizeToCopy;
    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);

    // 24-bit audio. Scrap the LSB (NOTE: not bit, but BYTE!)
    if (_bIs24Bit)
    {
        // Skip the LSB Byte and copy the next two bytes
        for (nIndex = 0; nIndex < nSize; nIndex += 3)
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + (nIndex*2/3)]),
                       &(_lpEncodedSrcData[_dwCurrentPos + nIndex+1]),
                       2);
    }
    else
    {
        // Simple. Just copy the whole Chunk
        CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]),
                   &(_lpEncodedSrcData[_dwCurrentPos]), nSize);
    }

    _dwCurrentPos += nSize;

    // Check if the end has reached. Set the no-more-data flag, if true
    if (_dwCurrentPos >= _dwSrcDataSize)
    {
        _bNoMoreData = TRUE;
        return DEWDEC_DECODE_EOD;
    }
    return DEWDEC_DECODE_OK;
}

// Stop the engine. Calculate the new position. Move the pointer there. Prepare the blocks
BYTE WAVENGINE::_Seek(DWORD dwMS)
{
    BYTE        btResult = DEWERR_SUCCESS, btPrevState = DEWS_MEDIA_NONE;
    MMRESULT    mmr = 0;
    double      dDuration = 0, dSeekTime, dSeekPoint;
    DWORD       dwSanitizationFactor;

    // Do not allow any seeking unless the player is playing or paused
    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        if (dwMS > _dwDuration)
            dwMS = _dwDuration;
        if (_dwDuration == 0) return DEWERR_SUCCESS;

        dSeekTime = (double)dwMS;
        // Note that we are not using the _wfex's avg bytes per sec.
        // It could have well been modified in case of 24 bit audio
        dDuration = ((double)_dwSrcDataSize) / ((double)_dwAvgBytesPerSec/1000.0f);
        // The length is so minimial that it tends to 0 - 1 sec.
        // Really no seeking necessary
        if (dDuration == 0) return DEWERR_SUCCESS;
        // Seek time is greater than total time. This can happen if UI slider 
        // is made to hit the end by the user. In such case, return the end
        if (dSeekTime > dDuration)
            dSeekPoint = _dwSrcDataSize;
        else
            dSeekPoint = (dSeekTime * (double)_dwSrcDataSize) / dDuration;

        // Before pushing the seek point as-is, sanitize the seek position
        // to ensure that it is a proper multiple of block-aligned bytes.
        // e.g. wfex.nBlockAlign * sizoef(BYTE) ==> _wfex.nBlockAlign * (_wfex.wBitsPerSample / 8);
        // This will ensure mid-block data is not accidentally picked up.
        dwSanitizationFactor = _wfex.nBlockAlign * (_wfex.wBitsPerSample / 8);
        _dwCurrentPos = (((DWORD)dSeekPoint) / dwSanitizationFactor) * dwSanitizationFactor;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}

// Strangely, there's really nothing to do apart from setting the media type
WAVENGINE::WAVENGINE()
{
    _btMediaType = DEWMT_WAV;
}

// Load by (UNICODE) file name
BYTE WAVENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    LPBYTE      lpTempSrcData = NULL;
    DWORD       dwTempSrcData = 0;
    HANDLE      hTempFile = NULL;
    BYTE        btReturn = DEWERR_SUCCESS;

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", srcFile);

    // Read the entire contents into a temporary buffer
    hTempFile = CreateFile(srcFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTempFile == NULL || hTempFile == INVALID_HANDLE_VALUE) return DEWERR_FILE_READ;
    dwTempSrcData = GetFileSize(hTempFile, NULL);
    lpTempSrcData = (LPBYTE)LocalAlloc(LPTR, dwTempSrcData);
    ReadFile(hTempFile, lpTempSrcData, dwTempSrcData, &dwTempSrcData, NULL);
    CloseHandle(hTempFile);
    // Pass the temp data to the overloaded function
    btReturn = Load(notificationWindow, lpTempSrcData, dwTempSrcData);
    // Release the memory
    LocalFree(lpTempSrcData);
    return btReturn;
}

// Load by raw data from the source file
BYTE WAVENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    UINT        nIndex;
    DWORD       dwData, dwFmtPos = 0, dwDataPos = 0;
    MMRESULT    mmr = 0;
    double      dDuration = 0.0f;
    WAVEOUTCAPS wcp { 0 };

    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcDataBytes) return DEWERR_INVALID_PARAM;
    if (dataSize <= 0x04) return DEWERR_INVALID_PARAM;

    // Set the notification window
    _hWndNotify = notificationWindow;

    // Hunt for the 'fmt ' and the 'data' tags/chunks
    for (nIndex = 0; nIndex < dataSize - 4; nIndex++)
    {
        dwData = 0;
        CopyMemory(&dwData, &(srcDataBytes[nIndex]), sizeof(DWORD));
        if (dwData == 0x20746D66) // 'fmt '
        {
            dwFmtPos = nIndex + 0x08; // 'fmt ' is followed by 4 bytes of fmt size
            if (dwDataPos > 0)
                break;
        }
        else if (dwData == 0x61746164) // 'data'
        {
            dwDataPos = nIndex + 0x08; // 'data' is followed by 4 bytes of data size
            if (dwFmtPos > 0)
                break;
        }
    }
    // Bail if the IDs were not found
    if (dwFmtPos <= 0 || dwDataPos <= 0) return DEWERR_FILE_READ;

    // Read the WAVE format into the structure
    CopyMemory(&_wfex, &(srcDataBytes[dwFmtPos]), sizeof(WAVEFORMATEX));

    // We only support WAVE_FORMAT_PCM
    if (_wfex.wFormatTag != WAVE_FORMAT_PCM) return DEWERR_UNSUPPORTED;

    // We only support 16 and 24 bit audio. 32 bit is not supported
    if (_wfex.wBitsPerSample != 16 && _wfex.wBitsPerSample != 24) return DEWERR_UNSUPPORTED;

    // Flush the buffer (highly unlikely that this will not be empty!)
    if (_lpEncodedSrcData)
    {
        LocalFree(_lpEncodedSrcData);
        _lpEncodedSrcData = NULL;
    }
    _dwSrcDataSize = dataSize - dwDataPos;
    _lpEncodedSrcData = (LPBYTE)LocalAlloc(LPTR, _dwSrcDataSize);

    // Store the raw data without any headers
    CopyMemory(_lpEncodedSrcData, &(srcDataBytes[dwDataPos]), _dwSrcDataSize);

    // This is for fancy info purposes only.
    // We MIGHT modify this setup, if 24 bit is detected
    _dwBitrate = _wfex.nSamplesPerSec * _wfex.wBitsPerSample * _wfex.nChannels;
    _dwSampleRate = _wfex.nSamplesPerSec;
    _btChannels = (BYTE)_wfex.nChannels;
    _dwCurrentPos = 0;
    _dwAvgBytesPerSec = _wfex.nAvgBytesPerSec;
    dDuration = ((double)_dwSrcDataSize) / ((double)_dwAvgBytesPerSec/1000.0f);
    _dwDuration = (DWORD)dDuration;

    // Special (patchwork) action needs to happen if it is 24 bit
    if (_wfex.wBitsPerSample == 24)
    {
        // Re-setup with 16-bit format.
        // We will strip each channel's LSB (Lowest Significant Byte)
        _bIs24Bit = TRUE;
        _wfex.wBitsPerSample = 16;
        _wfex.nBlockAlign = _wfex.nChannels * _wfex.wBitsPerSample / 8;
        _wfex.cbSize = 0;
        _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;
    }

    StringCchPrintf(_wsLibrary, MAX_PATH, L"Microsoft waveOut default mapper");

    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// Apply the same formula as duration calculation.
// Just use dwCurrentPos as the reference numerator
DWORD WAVENGINE::Tell()
{
    double      dDuration = 0;
    // Note that we are not using the _wfex's avg bytes per sec.
    // It could have well been modified in case of 24 bit audio
    dDuration = ((double)_dwCurrentPos) / ((double)_dwAvgBytesPerSec/1000.0f);
    dDuration = round(dDuration);

    return (DWORD)(dDuration); // return in ms
}

// Nothing much. Just cleanup
WAVENGINE::~WAVENGINE()
{
    this->_Cleanup();
    return;
}
