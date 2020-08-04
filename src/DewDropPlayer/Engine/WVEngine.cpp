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

#include "WVEngine.h"

// Release the context and the 32-bit integer ememory
void WVENGINE::_Cleanup()
{
    if (_pWPC)
        _pWPC = WavpackCloseFile(_pWPC);
    if (_lpSampleData)
    {
        LocalFree(_lpSampleData);
        _lpSampleData = NULL;
    }
}

// The main decoding logic.
// ** NOTE: There is no concept of an overflow here (unlike other engines)
// That is because we've already calculated the no. of frames, decoding which
// will yield exactly CHUNKSIZE bytes
BYTE WVENGINE::_Decode()
{
    DWORD   dwCumulDecoded = 0, dwSampleIdx, dwChIndex, dwDecoded = 0;

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;
    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;
    // Flush the data buffer memory
    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);

    // Loop over the 'calculated' sample count
    for (dwSampleIdx = 0; dwSampleIdx < _dwSamplesForChunk; dwSampleIdx++)
    {
        WavpackUnpackSamples(_pWPC, (int32_t*)_lpSampleData, 1);
        // Add a value of 0x80 to both the channels if it is 8 bits
        if (_wBitsPerSample == 8)
        {
            // Do it per channel
            for (dwChIndex = 0; dwChIndex < _btChannels; dwChIndex++)
                _lpSampleData[dwChIndex] += 0x80;
        }
        dwDecoded = _wBitsPerSample / 8;
        // Copy the decoded sample (per channel) content from the LPDWORD
        // into the decoded buffer
        for (dwChIndex = 0; dwChIndex < _btChannels; dwChIndex++)
        {
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), &(_lpSampleData[dwChIndex]), dwDecoded);
            dwCumulDecoded += dwDecoded;
        }
        // Increment the internal sample index
        _dwCurrentPos++;
        // Get the bitrate at this instant
        _dwBitrate = (DWORD)WavpackGetInstantBitrate(_pWPC);
        if (_dwCurrentPos >= _dwTotalSamples)
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
    }
    return DEWDEC_DECODE_OK;
}

// Same seeking logic. Set the sample pointer in proportion to the time.
BYTE WVENGINE::_Seek(DWORD dwMS)
{
    double      dDuration = 0, dSeekTime, dSeekSample;
    DWORD       dwSeekSample = 0;

    if (!_pWPC) return DEWERR_MM_ACTION;

    // Do not allow any seeking unless the player is playing or paused
    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        // Calculate the pointer distance by time ratio (use double for more accuracy)
        dDuration = (double)_dwDuration;
        dSeekTime = (double)dwMS;

        if (dDuration == 0) return DEWERR_SUCCESS;
        if (dSeekTime >= dDuration)
            dSeekSample = _dwTotalSamples;
        else
            dSeekSample = (dSeekTime * (double)_dwTotalSamples) / dDuration;
        _dwCurrentPos = (DWORD)dSeekSample;
        // Enable the guard to stop decoding using WavpackUnpackSamples
        if (!WavpackSeekSample(_pWPC, _dwCurrentPos))  return DEWERR_MM_ACTION;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}

// Same initialization. However, the version string provided by the
// library is ANSI. Need to convert it to UNICODE
WVENGINE::WVENGINE()
{
    wchar_t *pwsLib = NULL;
    DWORD   dwLen = 0;

    _btMediaType = DEWMT_WV;
    dwLen = lstrlenA(WavpackGetLibraryVersionString());

    pwsLib = (wchar_t*)LocalAlloc(LPTR, ((dwLen + 1) * sizeof(wchar_t)));
    MultiByteToWideChar(CP_UTF8, 0, WavpackGetLibraryVersionString(), dwLen, pwsLib, dwLen);
    pwsLib[dwLen] = L'\0';
    StringCchPrintf(_wsLibrary, MAX_PATH, L"libwavpack %s (modded with Unicode support)", pwsLib);
    LocalFree(pwsLib);
    pwsLib = NULL;
    return;
}

// The main loading logic/function
BYTE WVENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    char        sError[80] { 0 };
    int         modeMask = 0;
    MMRESULT    mmr = 0;

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;

    _hWndNotify = notificationWindow;
    StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", srcFile);

    // Open the file. Note that the original function expects a char* intead of
    // a wchar_t*. You'd need to provide OPEN_FILE_UTF8 for unicode.  However,
    // I couldn't make it to work and so modded the function to expect (and handle)
    // unicode input file name
    _pWPC = WavpackOpenFileInput(_wsSrcFile, sError, OPEN_2CH_MAX | OPEN_NORMALIZE, 0);
    if (lstrlenA(sError) > 0 || _pWPC == NULL) return DEWERR_FILE_READ;

    // Get the necessary details from the library functions
    _dwTotalSamples = WavpackGetNumSamples(_pWPC);
    _dwBitrate = (DWORD)WavpackGetAverageBitrate(_pWPC, 0);
    _wBitsPerSample = (WORD)WavpackGetBitsPerSample(_pWPC);
    _dwSampleRate = WavpackGetSampleRate(_pWPC);
    _btChannels = WavpackGetNumChannels(_pWPC);
    _dwDuration = (DWORD)((double)_dwTotalSamples/((double)_dwSampleRate/1000.0f));
    _bVBR = 0;

    // We cannot handle anything apart from 8 and 16 bits depths
    if (_wBitsPerSample != 8 && _wBitsPerSample != 16) return DEWERR_UNSUPPORTED;

    // CHUNKSIZE MUST EXACTLYBE DIVISIBLE BY 4
    // This will make sure dwOverflow is never needed as data will never overflow
    _dwSamplesForChunk = CHUNKSIZE / ((_btChannels * _wBitsPerSample) / 8);
    _lpSampleData = (LPDWORD)LocalAlloc(LPTR, (sizeof(DWORD) * _btChannels));

    // Extended Info
    _dwExInfoLength = 4;
    _lpExInfo = (LPEXTENDEDINFO)LocalAlloc(LPTR, sizeof(EXTENDEDINFO) * _dwExInfoLength);

    // Utilize the library functions to get the metadata information
    modeMask = WavpackGetMode(_pWPC);
    _lpExInfo[0].SetType(L"Compression Mode");
    if (modeMask & MODE_HYBRID) _lpExInfo[0].SetValue(L"Hybrid");
    else if (modeMask & MODE_LOSSLESS) _lpExInfo[0].SetValue(L"Lossless");
    else _lpExInfo[0].SetValue(L"Unknown");

    _lpExInfo[1].SetType(L"Correction File");
    if (modeMask & MODE_WVC) _lpExInfo[1].SetValue(L"Available");
    else _lpExInfo[1].SetValue(L"Unavailable");

    _lpExInfo[2].SetType(L"Compression Preset");
    if (modeMask & MODE_HIGH) _lpExInfo[2].SetValue(L"High");
    else if (modeMask & MODE_FAST) _lpExInfo[2].SetValue(L"Fast");
    else if (modeMask & MODE_EXTRA) _lpExInfo[2].SetValue(L"Extra");
    else if (modeMask & MODE_VERY_HIGH) _lpExInfo[2].SetValue(L"Very High");
    else _lpExInfo[2].SetValue(L"Unknown");

    _lpExInfo[3].SetType(L"MD5 Checksum");
    _lpExInfo[3].SetValue((modeMask & MODE_MD5) ? L"Present" : L"Absent");

    // Setup the WAVEFORMATEX
    _wfex.wFormatTag = WAVE_FORMAT_PCM;
    _wfex.cbSize = 0;
    _wfex.nChannels = (WORD)_btChannels;
    _wfex.nSamplesPerSec = _dwSampleRate;
    _wfex.wBitsPerSample = _wBitsPerSample;
    _wfex.nBlockAlign = (_wfex.nChannels * _wfex.wBitsPerSample) / 8;
    _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;
    _dwCurrentPos = 0;

    // All good!
    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// We won't support byte-mode file loading functionality
BYTE WVENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    return DEWERR_NOT_IMPLEMENTED;
}

// ** NOTE: Using dwCurrentPos is not recommended as the index is incremented
// after a decode cycle. This will create a latency. Use the library function
// instead to get a much lower latency report on the current sample position
DWORD WVENGINE::Tell()
{
    if (!_pWPC) return (DWORD)INVALID_HANDLE_VALUE;
    return (DWORD)((float)WavpackGetSampleIndex(_pWPC) / ((float)_dwSampleRate/1000.0f));
}

// Houskeeping and cleanup
WVENGINE::~WVENGINE()
{
    this->_Cleanup();
    return;
}
