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

#include "ALACEngine.h"

// Unload and freeups
void ALACENGINE::_Cleanup()
{
    int     iError = 0;
    if (_bALACLibInitialized)
    {
        iError = ALACLIB_Flush();
        _bALACLibInitialized = FALSE;
    }
    // Free up the sample lookup table
    if (_SampleLookup)
    {
        LocalFree(_SampleLookup);
        _SampleLookup = NULL;
    }
    return;
}

// The main decoding logic
BYTE ALACENGINE::_Decode()
{
    DWORD       dwDecoded = 0, dwCumulDecoded = 0;
    int         iError = 0;

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;
    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;
    
    // Clear out the main destination buffer
    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);

    // This (overflow) adds to the current iteration's cumulative decoded data
    dwCumulDecoded = _dwOverflow;
    // Copy whatever was left (dwOverflow)
    if (_dwOverflow > 0)
        CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), _lpDecodedChunk, _dwOverflow);

    // Flush the transient buffer as well
    ZeroMemory(_lpDecodedChunk, _dwMaxDecodeSize);

    // Decoding loop. Won't stop until a CHUNKSIZE is filled up
    // or error/end-of-source-data is encountered
    do
    {
        // Homemade ALACLib does this deocding
        iError = ALACLIB_Decode(&(_lpEncodedSrcData[_SampleLookup[_dwCurrentPos]]),
                                _SampleLookup[_dwCurrentPos + 1] - _SampleLookup[_dwCurrentPos],
                                _lpDecodedChunk, &dwDecoded);
        _dwCurrentPos++;
        // Proceed only if there is no error
        if (iError == ALACLIB_ERR_NO_ERROR)
        {
            if ((dwCumulDecoded + dwDecoded) >= CHUNKSIZE)
            {
                _dwOverflow = (dwCumulDecoded + dwDecoded) - CHUNKSIZE;
                CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _lpDecodedChunk, CHUNKSIZE - dwCumulDecoded);
                CopyMemory(_lpDecodedChunk, &(_lpDecodedChunk[CHUNKSIZE - dwCumulDecoded]), _dwOverflow);
                if (_dwCurrentPos >= _dwTotalSamples)
                {
                    _bNoMoreData = TRUE;
                    return DEWDEC_DECODE_EOD;
                }
                return DEWDEC_DECODE_OK;
            }
            else
            {
                CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _lpDecodedChunk, dwDecoded);
                dwCumulDecoded += dwDecoded;
            }
        }
        else // Error. Bail with 'No-Data' flag. Can't possibly decode any more of the input data
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }

        // End of the sample lookup table has reached. Bail.
        if (_dwCurrentPos >= _dwTotalSamples)
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
    } while (TRUE);

    return DEWDEC_DECODE_ERROR;
}

// Typical action. Grabe the seek time and derive the sample cursor.
// Move the cursor to that sample index. The decode cycle will take it up from there
BYTE ALACENGINE::_Seek(DWORD dwMS)
{
    if (!_bALACLibInitialized) return DEWERR_MM_ACTION;
    double      dDuration = 0, dSeekTime;
    int         iResult = 0;

    // Do not allow any seeking unless the player is playing or paused
    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        // Calculate the pointer distance by time ratio (use double for more accuracy)
        dDuration = (double)_dwDuration;
        dSeekTime = (double)dwMS;

        if (dDuration == 0) return DEWERR_SUCCESS;
        if (dSeekTime >= dDuration)
            _dwCurrentPos = _dwTotalSamples;
        else
            _dwCurrentPos = (DWORD)((dSeekTime * (double)_dwTotalSamples) / dDuration);
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}


// There is not "library" version. Code was taken from alac-master github master
// which in turn was from Apple's macforge site. This was then remodelled
// to become an ALACLib. Anyways we are safe as this is the ONLY engine which
// will NEVER interact with a file (since caf is not supported as of now)
ALACENGINE::ALACENGINE()
{
    _btMediaType = DEWMT_ALAC;
}

// This and the next mode of load will never come to be
BYTE ALACENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    return DEWERR_NOT_IMPLEMENTED;
}

// This and the next mode of load will never come to be
BYTE ALACENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    return DEWERR_NOT_IMPLEMENTED;
}

// Quite a few inputs are need to this engine. Please note that this method
// is specially geared to work the Bento4 library. All the inputs are to be
// provided by the caller (Bento4, in this case):
// Additional input params include the sample lookup (DWORD) table, its size,
// the channel count, the sample rate and the duration
BYTE ALACENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize,
                      LPDWORD sampleTable, DWORD dwTableSize,
                      BYTE channelCount, DWORD sampleRate, DWORD duration)
{
    int         iError = 0;
    MMRESULT    mmr = 0;

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcDataBytes || !sampleTable) return DEWERR_INVALID_PARAM;
    if (dataSize <= 0 || dwTableSize <= 0 || channelCount <= 0 || sampleRate <= 0 || duration <= 0) return DEWERR_INVALID_PARAM;

    // Set the internal variables/storages
    _hWndNotify = notificationWindow;
    _dwSrcDataSize = dataSize;
    _lpEncodedSrcData = (LPBYTE)LocalAlloc(LPTR, _dwSrcDataSize);
    CopyMemory(_lpEncodedSrcData, srcDataBytes, _dwSrcDataSize);

    _dwTotalSamples = dwTableSize;
    _SampleLookup = (LPDWORD)LocalAlloc(LPTR, sizeof(DWORD) * (_dwTotalSamples + 1));
    CopyMemory(_SampleLookup, sampleTable, sizeof(DWORD) * dwTableSize);
    // Additional offset to get the uniform sample data size while in loop
    _SampleLookup[_dwTotalSamples] = _dwSrcDataSize - _SampleLookup[_dwTotalSamples - 1];

    _btChannels = channelCount;
    _dwSampleRate = sampleRate;
    _dwDuration = duration;
    _fSPMS = (float)_dwTotalSamples / (float)_dwDuration;
    // Generic calculation: (total data/duration) * 8 (kilobits/sec).
    // Bento4 provides the duration in ms. So need a 1000 multiplier to get the
    // correct bitrate
    _dwBitrate = (DWORD)round(((((double)_dwSrcDataSize / (double)duration) * 8.0) * 1000));

    // Initialize
    iError = ALACLIB_Init(_btChannels, _dwSampleRate);
    if (iError != ALACLIB_ERR_NO_ERROR) return DEWERR_ENGINE_STARTUP;
    _bALACLibInitialized = TRUE;

    // Prepare the biggest block of decoded data that can come through
    // We'll prepare the transient buffer with this max value.
    iError = ALACLIB_GetMaxSizes(NULL, &_dwMaxDecodeSize);
    if (iError != ALACLIB_ERR_NO_ERROR) return DEWERR_ENGINE_STARTUP;
    _lpDecodedChunk = (LPBYTE)LocalAlloc(LPTR, _dwMaxDecodeSize);

    // Set the WAVEFORMATEX. We'll go with 16 bit depth
    _wfex.wFormatTag = WAVE_FORMAT_PCM;
    _wfex.cbSize = 0;
    _wfex.nChannels = (WORD)_btChannels;
    _wfex.nSamplesPerSec = _dwSampleRate;
    _wfex.wBitsPerSample = 16;
    _wfex.nBlockAlign = (_wfex.nChannels * _wfex.wBitsPerSample) / 8;
    _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;

    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// Even simpler. Unitary method to get the time division based on the sample cursor
DWORD ALACENGINE::Tell()
{
    if (!_bALACLibInitialized) return (DWORD)INVALID_HANDLE_VALUE;
    return (DWORD)((double)(_dwCurrentPos * _dwDuration)/(double)_dwTotalSamples);
}

// Housekeeping & cleanup
ALACENGINE::~ALACENGINE()
{
    this->_Cleanup();
    return;
}
