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

#include "MP3Engine.h"

// Clear out the memories & delete the temp file (if created)
void MP3ENGINE::_Cleanup()
{
    if (_lpDecodedChunk)
    {
        LocalFree(_lpDecodedChunk);
        _lpDecodedChunk = NULL;
    }
    if (_hMP3 != NULL && _hMP3 != INVALID_HANDLE_VALUE)
        mpg123_tclose(_hMP3); // Frankly, I don't fucking care about the return since this is dead anyways :P
    if (_btStatus != DEWS_MEDIA_NONE)
        mpg123_exit();
    if (!_bOverloadedLoadFunctionCall)
        DeleteFile(_wsInternalTemp);
    return;
}

// MAIN DECODING LOGIC
// Driven by the CHUNKSIZE-block index to populate
BYTE MP3ENGINE::_Decode()
{
    int                 iResult;
    DWORD               dwRead, dwCumulDecoded = 0;
    mpg123_frameinfo    mp3FI;

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;
    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;

    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);
    // Prepopulate any leftovers from the previous iteration
    // this means 'CHUNKSIZE - dwOverflow' amount of data will have to be
    // decoded as the dwOverflow is already being pre-populated
    dwRead = CHUNKSIZE - _dwOverflow;
    // This (overflow) adds to the current iteration's cumulative decoded data
    dwCumulDecoded = _dwOverflow;
    // Copy whatever was left (dwOverflow)
    if (_dwOverflow > 0)
        CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), _lpDecodedChunk, _dwOverflow);
    // Flush the decoding chunk just to be extra sure
    ZeroMemory(_lpDecodedChunk, CHUNKSIZE);
    do
    {
        // Decode
        iResult = mpg123_read(_hMP3, _lpDecodedChunk, dwRead, (size_t*)&dwRead);
        //If nothing was decoded, then EOF has been reached
        if (dwRead <= 0)
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
        if (iResult == MPG123_OK || iResult == MPG123_DONE)
        {
            // Update the bitrate if the mp3 is VBR
            if (_bVBR)
            {
                mpg123_info(_hMP3, &mp3FI);
                _dwBitrate = mp3FI.bitrate * 1000;
            }

            // Check if the decoding stream has reached its end. Set the flag.
            if (iResult == MPG123_DONE || mpg123_tell(_hMP3) >= _lTotalSamples)
                _bNoMoreData = TRUE;

            // Before adding up the cumulative decoded size, check if it
            // overflows beyond the CHUNKSIZE
            if ((dwCumulDecoded + dwRead) >= CHUNKSIZE)
            {
                // Keep the overflow amount in the lpDecodedChunk buffer
                _dwOverflow = (dwCumulDecoded + dwRead) - CHUNKSIZE;
                CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _lpDecodedChunk, CHUNKSIZE - dwCumulDecoded);
                CopyMemory(_lpDecodedChunk, &(_lpDecodedChunk[CHUNKSIZE - dwCumulDecoded]), _dwOverflow);
                return DEWDEC_DECODE_OK;
            }
            else
            {
                // Not yet there. Add up the cumulative decoded data value
                // and continue with the looped decoding
                CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE + dwCumulDecoded]), _lpDecodedChunk, dwRead);
                dwCumulDecoded += dwRead;
            }

            // At this point, it is possible that the stream has reached its end.
            // If so, return the EOD flag
            if (_bNoMoreData) return DEWDEC_DECODE_EOD;

        }
        // Set the 'to-be-read' value to the default 'CHUNKSIZE' again
        // for the next loop iteration
        dwRead = CHUNKSIZE;
    } while (TRUE);
    return DEWDEC_DECODE_OK;
}

// Stop the engine. Calculate the new position. Move the pointer there. Prepare the blocks
BYTE MP3ENGINE::_Seek(DWORD dwMS)
{
    double      dDuration = 0, dSeekTime, dSeekSample;
    int         iResult = 0;

    // Do not allow any seeking unless the player is playing or paused
    if (_btStatus == DEWS_MEDIA_PLAYING || DEWS_MEDIA_PAUSED)
    {
        // Calculate the pointer distance by time ratio (use double for more accuracy)
        dDuration = (double)_dwDuration;
        dSeekTime = (double)dwMS;

        if (dDuration == 0) return DEWERR_SUCCESS;
        if (dSeekTime >= dDuration)
            dSeekSample = _lTotalSamples;
        else
            dSeekSample = (dSeekTime * (double)_lTotalSamples) / dDuration;
        iResult = mpg123_seek(_hMP3, (long)dSeekSample, SEEK_SET);
        if (iResult != MPG123_OK) return DEWERR_MM_ACTION;
        return DEWERR_SUCCESS;
    }
    return DEWERR_MM_ACTION;
}


// Nothing to be done here really
MP3ENGINE::MP3ENGINE()
{
    _btMediaType = DEWMT_MP3;
    _lpDecodedChunk = (LPBYTE)LocalAlloc(LPTR, CHUNKSIZE);
    StringCchPrintf(_wsLibrary, MAX_PATH, L"libmpg version 1.25.10");
}

// Dabble with the file to check if it can be loaded
BOOL MP3ENGINE::IsLoadable(LPCWSTR testFile)
{
    int     iTmpResult = 0;
    BOOL    bLoadable = FALSE;
    mpg123_handle   *hTmpMP3;

    iTmpResult = mpg123_init();
    if (iTmpResult != MPG123_OK) goto __dabble_level1;
    hTmpMP3 = mpg123_new(NULL, NULL);
    if (hTmpMP3 == NULL || hTmpMP3 == INVALID_HANDLE_VALUE) goto __dabble_level2;
    iTmpResult = mpg123_topen(hTmpMP3, testFile);
    if (iTmpResult != MPG123_OK) goto __dabble_level3;
    bLoadable = TRUE;
__dabble_level3:
    mpg123_tclose(hTmpMP3);
__dabble_level2:
    mpg123_exit();
__dabble_level1:
    return bLoadable;
}

// Just a tub method to mug up all the source data bytes
BYTE MP3ENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
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

// The 'Real' function
BYTE MP3ENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    int                 iResult = 0, nEncoding = 0;
    mpg123_frameinfo    mp3FI;
    wchar_t             wsExValue[EXINFO_VAL_SIZE];
    MMRESULT            mmr = 0;

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
    iResult = mpg123_init();
    if (iResult != MPG123_OK) return DEWERR_ENGINE_STARTUP;
    _hMP3 = mpg123_new(NULL, NULL);
    if (_hMP3 == NULL || _hMP3 == INVALID_HANDLE_VALUE) return DEWERR_ENGINE_STARTUP;
    iResult = mpg123_open_feed(_hMP3);
    if (iResult != MPG123_OK) return DEWERR_ENGINE_STARTUP;
    // Straightforward calculation. Read the unicode file directly.
    if (_bOverloadedLoadFunctionCall)
    {
        if (_wsSrcFile)
            iResult = mpg123_topen(_hMP3, _wsSrcFile);
        else
            return DEWERR_ENGINE_STARTUP;
    }
    // Othwerwise, write out the contents to a temp and load that!
    else
    {
        HANDLE  hTmpFile = NULL;
        hTmpFile = CreateFile(_wsInternalTemp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTmpFile == NULL || hTmpFile == INVALID_HANDLE_VALUE) return DEWERR_ENGINE_STARTUP;
        WriteFile(hTmpFile, _lpEncodedSrcData, _dwSrcDataSize, &_dwSrcDataSize, NULL);
        CloseHandle(hTmpFile);
        iResult = mpg123_topen(_hMP3, _wsInternalTemp);
    }
    // FOR SOME WEIRD REASON, BACKWARDS SEEKING FAILS ON mpg123_feed
    // iResult = mpg123_feed(hMP3, lpEncodedSrcData, dwSrcDataSize);
    // Instead, we'll replicate the behavior by creating a small temp file
    // and deleting that at the end
    if (iResult != MPG123_OK) return DEWERR_ENGINE_STARTUP;
    iResult = mpg123_getformat(_hMP3, (long*)&_dwSampleRate, (int*)&_btChannels, &nEncoding);
    if (iResult != MPG123_OK) return DEWERR_ENGINE_STARTUP;

    _wfex.wFormatTag = WAVE_FORMAT_PCM;
    _wfex.cbSize = 0;
    _wfex.nChannels = (WORD)_btChannels;
    _wfex.nSamplesPerSec = _dwSampleRate;
    _wfex.wBitsPerSample = 16;
    _wfex.nBlockAlign = (_wfex.nChannels * _wfex.wBitsPerSample) / 8;
    _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;

    _lTotalSamples = mpg123_length(_hMP3);
    if (_lTotalSamples < MPG123_OK) return DEWERR_ENGINE_STARTUP;
    _dwDuration = (DWORD)(((float)_lTotalSamples) / (((float)_dwSampleRate) / 1000.0f));
    iResult = mpg123_info(_hMP3, &mp3FI);
    _dwBitrate = mp3FI.bitrate * 1000; // Convert to bps from kbps
    
    // Extended Info
    _dwExInfoLength = 4;
    _lpExInfo = (LPEXTENDEDINFO)LocalAlloc(LPTR, sizeof(EXTENDEDINFO) * _dwExInfoLength);

    _lpExInfo[0].SetType(L"MPEG Version");
    switch (mp3FI.version)
    {
        case MPG123_1_0:
             _lpExInfo[0].SetValue(L"MPEG Version 1.0");
             break;
        case MPG123_2_0:
             _lpExInfo[0].SetValue(L"MPEG Version 2.0");
             break;
        case MPG123_2_5:
             _lpExInfo[0].SetValue(L"MPEG Version 2.5");
             break;
        default:
             _lpExInfo[0].SetValue(L"Unknown");
             break;
    }

    StringCchPrintf(wsExValue, EXINFO_VAL_SIZE, L"Layer %d", mp3FI.layer);
    _lpExInfo[1].SetType(L"MPEG Layer");
    _lpExInfo[1].SetValue(wsExValue);

    _lpExInfo[2].SetType(L"Audio mode");
    switch (mp3FI.mode)
    {
        case MPG123_M_STEREO:
             _lpExInfo[2].SetValue(L"Standard Stereo");
             break;
        case MPG123_M_JOINT:
             _lpExInfo[2].SetValue(L"Joint Stereo Channel");
             break;
        case MPG123_M_DUAL:
             _lpExInfo[2].SetValue(L"Dual Channel");
             break;
        case MPG123_M_MONO:
             _lpExInfo[2].SetValue(L"Single Channel");
             break;
        default:
             _lpExInfo[2].SetValue(L"Unknown");
             break;
    }

    _lpExInfo[3].SetType(L"Bitrate mode");
    switch (mp3FI.vbr)
    {
        case MPG123_CBR:
             _lpExInfo[3].SetValue(L"Constant Bitrate (CBR)");
             break;
        case MPG123_VBR:
             _lpExInfo[3].SetValue(L"Variable Bitrate (VBR)");
             _bVBR = TRUE;
             break;
        case MPG123_ABR:
             _lpExInfo[3].SetValue(L"Average Bitrate (ABR)");
             break;
        default:
             _lpExInfo[3].SetValue(L"Unknown");
             break;
    }

    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// Use the library to tell us the poition (in seconds)
DWORD MP3ENGINE::Tell()
{
    long        lCurrSample = 0;

    lCurrSample = mpg123_tell(_hMP3);
    if (lCurrSample < 0)
        return (DWORD)0xFFFFFFFF; // INVALID_HANDLE_VALUE
    return (DWORD)((float)lCurrSample / (((float)_dwSampleRate) / 1000.0f));
}

// Housekeeping
MP3ENGINE::~MP3ENGINE()
{
    this->_Cleanup();
    return;
}
