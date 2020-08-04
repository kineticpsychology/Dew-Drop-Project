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

#include "AACEngine.h"

// Custom function to trace the details of all the frames
// available in the AAC stream
BOOL AACENGINE::_ParseADTS()
{
    WORD        wSyncWord;
    LPBYTE      lpADTS = NULL;
    DWORD       FREQLOOKUP[13];
    DWORD       dwFrameLength = 0;
    DWORD       dwFrameOffset = 0;

    // Consult this link for ADTS header:
    // https://wiki.multimedia.cx/index.php/ADTS

    FREQLOOKUP[0] = 96000;
    FREQLOOKUP[1] = 88200;
    FREQLOOKUP[2] = 64000;
    FREQLOOKUP[3] = 48000;
    FREQLOOKUP[4] = 44100;
    FREQLOOKUP[5] = 32000;
    FREQLOOKUP[6] = 24000;
    FREQLOOKUP[7] = 22050;
    FREQLOOKUP[8] = 16000;
    FREQLOOKUP[9] = 12000;
    FREQLOOKUP[10] = 11025;
    FREQLOOKUP[11] = 8000;
    FREQLOOKUP[12] = 7350;

    // Start from the 'discardable' byte offset
    lpADTS = &(_lpEncodedSrcData[_dwRawAACOffset]);
    wSyncWord = (lpADTS[0] << 4 | lpADTS[1] >> 4);
    // Sync word MUST be 0xFFF. If not, bail.
    if (wSyncWord != 0xFFF) return TRUE;
    //0 for MPEG-4
    //1 for MPEG-2
    _btMPEGVersion = (lpADTS[1] & 0x08) >> 3;
    //1: Absent (no CRC), 0: 2: Present (CRC)
    _btProtectionAbsent = (lpADTS[1] & 0x01);
    //1: AAC Main
    //2: AAC LC (Low Complexity)
    //3: AAC SSR (Scalable Sample Rate)
    //4: AAC LTP (Long Term Prediction)
    _btAACProfile = ((lpADTS[2] & 0xC0) >> 6) + 1;
    _dwSampleRate = FREQLOOKUP[((lpADTS[2] & 0x3C) >> 2)];
    _btChannels = ((lpADTS[2] & 0x01) >> 5) | ((lpADTS[3] & 0xC0) >> 6);
    // Loop through each frame, adding up the frame size until EOF is reached.
    // and build up a frame lookup dictionary
    dwFrameOffset = _dwRawAACOffset;
    do
    {
        dwFrameLength = ((lpADTS[3] & 0x03) << 11) | (lpADTS[4] << 3) | ((lpADTS[5] & 0xE0) >> 5);
        _dwFrameCount++;
        if (_dwFrameCount == 1)
            _FrameLookup = (LPDWORD)LocalAlloc(LPTR, _dwFrameCount * sizeof(DWORD));
        else
            _FrameLookup = (LPDWORD)LocalReAlloc(_FrameLookup, _dwFrameCount * sizeof(DWORD), LHND);
        _FrameLookup[_dwFrameCount - 1] = dwFrameOffset;
        dwFrameOffset += dwFrameLength;
        lpADTS = &(_lpEncodedSrcData[dwFrameOffset]);
    } while (dwFrameOffset < _dwSrcDataSize);
    return TRUE;
}

// Unload the decoder handle & release the Frame Lookup index
void AACENGINE::_Cleanup()
{
    if (_naacDec)
    {
        NeAACDecClose(_naacDec);
        _naacDec = NULL;
    }
    if (_FrameLookup)
    {
        LocalFree(_FrameLookup);
        _FrameLookup = NULL;
    }
    return;
}

// We will jump from frame to frame.
BYTE AACENGINE::_Decode()
{
    DWORD               dwCumulDecoded = 0, dwDecoded = 0;
    LPBYTE              lpWAVBuff;
    NeAACDecFrameInfo   frameInfo { 0 };

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;
    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;

    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);
    dwCumulDecoded = _dwOverflow;
    // Copy whatever was left (dwOverflow)
    if (_dwOverflow > 0)
    {
        CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), _lpDecodedChunk, _dwOverflow);
        LocalFree(_lpDecodedChunk);
        _lpDecodedChunk = NULL;
    }
    do
    {
        // We have the position of the current frame to be decoded.
        // Use the source data from that frame position to the next frame offset
        // Only for the last frame, we'll deduce the frame length
        // by subtracting second-last frame offset from the source file length
        if (_dwCurrentPos == (_dwFrameCount - 1))
            lpWAVBuff = (LPBYTE)NeAACDecDecode(_naacDec, &frameInfo, &(_lpEncodedSrcData[_FrameLookup[_dwCurrentPos]]), _dwSrcDataSize - _FrameLookup[_dwCurrentPos]);
        else
            lpWAVBuff = (LPBYTE)NeAACDecDecode(_naacDec, &frameInfo, &(_lpEncodedSrcData[_FrameLookup[_dwCurrentPos]]), _FrameLookup[_dwCurrentPos + 1] - _FrameLookup[_dwCurrentPos]);
        if (lpWAVBuff == NULL) return DEWDEC_DECODE_ERROR;
        dwDecoded = frameInfo.samples * 2; // Channels per sample (hard-coded to 2 for us);
        _dwCurrentPos++;
        // Same ol' logic to handle overflow. Check the MP3/OGG engine implementations
        if ((dwCumulDecoded + dwDecoded) >= CHUNKSIZE)
        {
            // Keep the overflow amount in the lpDecodedChunk buffer
            _dwOverflow = (dwCumulDecoded + dwDecoded) - CHUNKSIZE;
            if (_dwOverflow > 0)
            {
                _lpDecodedChunk = (LPBYTE)LocalAlloc(LPTR, _dwOverflow);
                CopyMemory(_lpDecodedChunk, &(lpWAVBuff[CHUNKSIZE - dwCumulDecoded]), _dwOverflow);
            }
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), lpWAVBuff, CHUNKSIZE - dwCumulDecoded);
            if (_dwCurrentPos >= (_dwFrameCount -1)) _bNoMoreData = TRUE;
            return DEWDEC_DECODE_OK;
        }
        else
        {
            // Not yet there. Add up the cumulative decoded data value
            // and continue with the looped decoding
            CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), lpWAVBuff, dwDecoded);
            dwCumulDecoded += dwDecoded;
        }
        // We'll loop till the last but one frame.
        if (_dwCurrentPos >= (_dwFrameCount - 1))
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
    } while (TRUE);
    return DEWDEC_DECODE_ERROR;
}

// Utilize the Current frame indicator variable to seek
BYTE AACENGINE::_Seek(DWORD dwMS)
{
    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        if (_dwDuration == 0) return DEWERR_SUCCESS;
        if (dwMS >= _dwDuration) dwMS = _dwDuration;
        _dwCurrentPos = (DWORD)((float)dwMS * _fFPMS);
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}


// Basic initilaization (media type & library version info)
AACENGINE::AACENGINE()
{
    _btMediaType = DEWMT_AAC;
    StringCchPrintf(_wsLibrary, MAX_PATH, L"libfaad2 version 2.8.8");
    return;
}

// Stub method. We'll just load the file and call the main 'Load' function
BYTE AACENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
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

// The main 'Load' method
BYTE AACENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    long            lBytesToSkip = 0;
    WAVEFORMATEX    wfex { 0 };
    DWORD           dwSampleRateAAC = 0;
    MMRESULT        mmr = 0;

    // Sanity checks
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

    // Start the Nero decoder
    _naacDec = NeAACDecOpen();
    if (_naacDec == NULL || _naacDec == INVALID_HANDLE_VALUE) return DEWERR_ENGINE_STARTUP;
    // Find the offset to the AAC data stream
    lBytesToSkip = NeAACDecInit(_naacDec, _lpEncodedSrcData, _dwSrcDataSize, &dwSampleRateAAC, (LPBYTE)&_btChannels);
    if (lBytesToSkip < 0) return DEWERR_ENGINE_STARTUP;
    _dwRawAACOffset = (DWORD)lBytesToSkip;
    // Call the frame analysis function
    if (!_ParseADTS()) return DEWERR_INVALID_FILE;

    // Setup the WAVEFORMATEX
    _wfex.wFormatTag = WAVE_FORMAT_PCM;
    _wfex.cbSize = 0;
    _wfex.nChannels = 2; // We'll always decode for two channels.
    _wfex.nSamplesPerSec = dwSampleRateAAC;
    _wfex.wBitsPerSample = 16;
    _wfex.nBlockAlign = (_wfex.nChannels * _wfex.wBitsPerSample) / 8;
    _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;

    // Sample Rate and Channels are already populated
    _dwDuration = (DWORD)(((float)(_dwFrameCount * _AAC_PACK)) / (((float)_dwSampleRate)/1000.0f));
    _fFPMS = (float)_dwFrameCount / (float)_dwDuration;
    _bVBR = 0; // We'll continuously calculate the Bitrate
    // No idea how/why this works, but it works :S
    _dwBitrate = (DWORD)(((((double)_dwSrcDataSize / (double)_dwFrameCount) * (double)(_fFPMS*1000.0f)) + 0.5f) * 8.0f);

    _dwCurrentPos = 0; // Reuse this parent variable for seeking through frame indices

    // Extended Info
    _dwExInfoLength = 3;
    _lpExInfo = (LPEXTENDEDINFO)LocalAlloc(LPTR, sizeof(EXTENDEDINFO) * _dwExInfoLength);
    _lpExInfo[0].SetType(L"MPEG Version");
    switch (_btMPEGVersion)
    {
        case 0: _lpExInfo[0].SetValue(L"MPEG-4"); break;
        case 1: _lpExInfo[0].SetValue(L"MPEG-2"); break;
        default: _lpExInfo[0].SetValue(L"Unknown"); break;
    }
    _lpExInfo[1].SetType(L"AAC Profile");
    switch (_btAACProfile)
    {
        case 1: _lpExInfo[1].SetValue(L"AAC Main"); break;
        case 2: _lpExInfo[1].SetValue(L"AAC LC (Low Complexity)"); break;
        case 3: _lpExInfo[1].SetValue(L"AAC SSR (Scalable Sample Rate)"); break;
        case 4: _lpExInfo[1].SetValue(L"AAC LTP (Long Term Prediction)"); break;
        default: _lpExInfo[1].SetValue(L"Unknown"); break;
    }
    _lpExInfo[2].SetType(L"Protection Enabled");
    _lpExInfo[2].SetValue(_btProtectionAbsent == 1 ? L"No" : L"Yes");
    
    // All set
    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// Utilize the Current frame indicator variable to tell
DWORD AACENGINE::Tell()
{ 
    if (!_naacDec) return (DWORD)INVALID_HANDLE_VALUE;
    return (DWORD)((float)_dwCurrentPos/_fFPMS);
}

// Housekeeping & cleanup
AACENGINE::~AACENGINE()
{
    this->_Cleanup();
    return;
}
