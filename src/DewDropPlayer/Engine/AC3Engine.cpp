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

#include "AC3Engine.h"

// Don't ask. Ripped from the example provided in the liba52 sample decoder code
int16_t AC3ENGINE::_convert(int32_t i)
{
    if (i > 0x43c07fff)
        return 32767;
    else if (i < 0x43bf8000)
        return -32768;
    else
        return i - 0x43c00000;
}

// Don't ask. Ripped from the example provided in the liba52 sample decoder code
void AC3ENGINE::_float2s16_2(float * _f, int16_t * s16)
{
    int i;
    int32_t * f = (int32_t *)_f;

    for (i = 0; i < 256; i++) {
        s16[2 * i] = this->_convert(f[i]);
        s16[2 * i + 1] = this->_convert(f[i + 256]);
    }
}

// Similar to AAC, do a manual parsing of the frames like a lookup
// so that we can do a more easy seek/tell going by frame index
void AC3ENGINE::_ParseA52Frames()
{
    DWORD           dwFrameOffset, dwFrameSize, dwFlags = 0;

    dwFrameOffset = 0;
    do
    {
        dwFrameSize = a52_syncinfo(&(_lpEncodedSrcData[dwFrameOffset]), (int*)&dwFlags, (int*)&_dwSampleRate, (int*)&_dwBitrate);
        _btChannels = (BYTE)(dwFlags & A52_CHANNEL_MASK);
        if (dwFrameSize > 0)
        {
            _dwFrameCount++;
            if (_dwFrameCount == 1)
                _FrameLookup = (LPDWORD)LocalAlloc(LPTR, _dwFrameCount * sizeof(DWORD));
            else
                _FrameLookup = (LPDWORD)LocalReAlloc(_FrameLookup, _dwFrameCount * sizeof(DWORD), LHND);
            _FrameLookup[_dwFrameCount - 1] = dwFrameOffset;
        }
        dwFrameOffset += dwFrameSize;
    } while (dwFrameOffset < _dwSrcDataSize);
    return;
}

// Free up the Frame dictionary
void AC3ENGINE::_Cleanup()
{
    if (_FrameLookup)
    {
        LocalFree(_FrameLookup);
        _FrameLookup = NULL;
    }
    if (_ac3State)
    {
        a52_free(_ac3State);
        _ac3State = NULL;
    }
    return;
}

// The main decoding logic
BYTE AC3ENGINE::_Decode()
{
    DWORD       dwCumulDecoded = 0, dwFlags, dwIndex;
    float       level = 1.0, bias = 384.0;
    sample_t    *samples = NULL;
    // It might be tempting but do NOT use this variable as class member
    // The reason is two pieces of 'Decode()' will eventually be called
    // from a thread and might result in data overwriting!
    BYTE        FrameDecodeData[AC3_BLOCK_DECODED_SIZE] { 0 };

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;

    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);
    // Take up the leftovers
    if (_dwOverflow > 0)
        CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), _lpDecodedChunk, _dwOverflow);
    dwCumulDecoded = _dwOverflow;

    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;

    do
    {
        dwFlags = AC3_INTENDED_OUT_CHANNELS;
        a52_frame(_ac3State, &_lpEncodedSrcData[_FrameLookup[_dwCurrentPos]], (int*)&dwFlags, &level, bias);
        // Set this flag ASAP so that the other threaded call to Decode can
        // understand that no more data is left
        if (_dwCurrentPos >= (_dwFrameCount - 1)) _bNoMoreData = TRUE;
        samples = a52_samples(_ac3State);
        for (dwIndex = 0; dwIndex < AC3_BLOCKS_PER_FRAME; dwIndex++)
        {
            a52_block(_ac3State);
            this->_float2s16_2(samples, (int16_t*)FrameDecodeData);
            CopyMemory(&(_lpDecodedChunk[dwIndex * AC3_BLOCK_DECODED_SIZE]), FrameDecodeData, AC3_BLOCK_DECODED_SIZE);
        }
        _dwCurrentPos++;
        if ((dwCumulDecoded + AC3_FRAME_DECODED_SIZE) >= CHUNKSIZE)
        {
            // Keep the overflow amount in the lpDecodedChunk buffer
            _dwOverflow = (dwCumulDecoded + AC3_FRAME_DECODED_SIZE) - CHUNKSIZE;
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _lpDecodedChunk, CHUNKSIZE - dwCumulDecoded);
            CopyMemory(_lpDecodedChunk, &(_lpDecodedChunk[CHUNKSIZE - dwCumulDecoded]), _dwOverflow);
            return DEWDEC_DECODE_OK;
        }
        else
        {
            // Not yet there. Add up the cumulative decoded data value
            // and continue with the looped decoding
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _lpDecodedChunk, AC3_FRAME_DECODED_SIZE);
            dwCumulDecoded += AC3_FRAME_DECODED_SIZE;
        }
        if (_dwCurrentPos >= (_dwFrameCount - 1))
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
    } while (TRUE);

    return DEWDEC_DECODE_ERROR;
}

// Set the frame cursor to the appropriate proportionate famre offset
BYTE AC3ENGINE::_Seek(DWORD dwMS)
{
    // As typical, we won't allow seeking if the media is not playing or paused
    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        if (!_ac3State) return DEWERR_MM_ACTION;

        if (_dwDuration == 0) return DEWERR_SUCCESS;
        if (dwMS >= _dwDuration)
            _dwCurrentPos = _dwFrameCount;
        else
            _dwCurrentPos = (DWORD)((float)dwMS * _fFPMS);
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}


// The same. Initialize the library name and the media type
AC3ENGINE::AC3ENGINE()
{
    _btMediaType = DEWMT_AC3;
    StringCchPrintf(_wsLibrary, MAX_PATH, L"liba52 version 0.7.4");
}

// This is a stub indirect delegate to the main 'Load' function
BYTE AC3ENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;

    // Store the entire file into the internal storage
    if (!this->_StoreRawInputData(srcFile)) return DEWERR_FILE_READ;

    // Set the flag to indicate that we're going to make an overloaded call
    _bOverloadedLoadFunctionCall = TRUE;
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", srcFile);
    // Then call the actual function to load the media
    return this->Load(notificationWindow, _lpEncodedSrcData, _dwSrcDataSize);
}

// The main 'Load' function which loads by input source data
BYTE AC3ENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    MMRESULT        mmr = 0;
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcDataBytes) return DEWERR_INVALID_PARAM;
    if (dataSize <= 0x00) return DEWERR_INVALID_PARAM;

    // Set the notification window
    _hWndNotify = notificationWindow;

    // Proceed with lpEncodedSrcData population only if this function
    // has been called explicitly from outside (and not an overloaded call)
    if (!_bOverloadedLoadFunctionCall)
    {
        _dwSrcDataSize = dataSize;
        _lpEncodedSrcData = (LPBYTE)LocalAlloc(LPTR, _dwSrcDataSize);
        CopyMemory(_lpEncodedSrcData, srcDataBytes, _dwSrcDataSize);
    }

    _ac3State = a52_init(MM_ACCEL_DJBFFT);
    if (!_ac3State) return DEWERR_ENGINE_STARTUP;

    // Setup the frame list and set the continuous frame marker
    _ParseA52Frames();
    _dwCurrentPos = 0;

    // Setup WAVEFORMATEX (we'll go with 16 bit depth).
    _wfex.cbSize = 0;
    _wfex.wFormatTag = WAVE_FORMAT_PCM;
    _wfex.wBitsPerSample = 16;
    _wfex.nChannels = AC3_INTENDED_OUT_CHANNELS;
    _wfex.nSamplesPerSec = _dwSampleRate;
    _wfex.nBlockAlign = _wfex.wBitsPerSample * _wfex.nChannels / 8;
    _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;

    // Since the samples per frame is fixed and we have the frame count
    // get the total sample count as well.
    _dwTotalSamples = _dwFrameCount * AC3_BLOCKS_PER_FRAME * AC3_SAMPLES_PER_BLOCK_PER_CHANNEL;
    _dwDuration = (DWORD)((float)_dwTotalSamples / (((float)_dwSampleRate)/1000.0f));
    _fFPMS = (float)_dwFrameCount / (float)_dwDuration;
    
    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// Derive the time from the frame cursor
DWORD AC3ENGINE::Tell()
{
    if (!_ac3State)
        return (DWORD)0xFFFFFFFF; // INVALID_HANDLE_VALUE
    return ((_dwCurrentPos * _dwDuration)/_dwFrameCount);
}

// Housekeeping and cleanup
AC3ENGINE::~AC3ENGINE()
{
    this->_Cleanup();
    return;
}

// Isolated method to dabble and see if the file is loadable
BOOL AC3ENGINE::IsLoadable(LPCWSTR testFile)
{
    HANDLE      hTmpAC3 = NULL;
    DWORD       dwLength = 0;
    // Minimum bytes to satisfoctoily conclude if the stream looks like AC3
    const DWORD MIN_AC3_BYTES = 7;
    BYTE        lpTmpAC3Data[MIN_AC3_BYTES];
    a52_state_t *tmpAC3State = NULL;
    DWORD       dwTmpFlags, dwTmpSampleRate, dwTmpBitrate;
    int         iRet = 0;

    hTmpAC3 = CreateFile(testFile, GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTmpAC3 == NULL || hTmpAC3 == INVALID_HANDLE_VALUE) return FALSE;
    dwLength = GetFileSize(hTmpAC3, NULL);
    if (dwLength < MIN_AC3_BYTES)
    {
        CloseHandle(hTmpAC3);
        return FALSE;
    }
    ReadFile(hTmpAC3, lpTmpAC3Data, MIN_AC3_BYTES, &dwLength, NULL);
    CloseHandle(hTmpAC3);
    tmpAC3State = a52_init(MM_ACCEL_DJBFFT);
    if (!tmpAC3State) return FALSE;
    iRet = a52_syncinfo(lpTmpAC3Data, (int*)&dwTmpFlags,
                        (int*)&dwTmpSampleRate, (int*)&dwTmpBitrate);
    a52_free(tmpAC3State);
    return (iRet > 0);
}
