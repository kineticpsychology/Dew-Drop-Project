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

#include "VORBISEngine.h"

/*
~~~ THE XIPH FISH ~~~
\m/ YOU GUYS ROCK! I FUCKING LOVE OPUS \m/

    ,-"""".    .
   < "      \_/|
    `-____/---\|

*/

// Free up the OggVorbis_File and close the file pointer
void VORBISENGINE::_Cleanup()
{
    if (_lpDecodedChunk) LocalFree(_lpDecodedChunk);
    ov_clear(&_OVF);
    if (_fp)
    {
        fclose(_fp);
        _fp = NULL;
    }
    DeleteFile(_wsInternalTemp);
}

// Main decoding logic
BYTE VORBISENGINE::_Decode()
{
    DWORD       dwCumulDecoded = 0;
    int         iBitStream = 0, iRet = 0, iInstBR = 0;
    DWORD       dwDecoded = 0;

    // Invalid chunk ID. Bail
    if (_dwCurrentIndex >= CHUNKCOUNT) return DEWDEC_DECODE_ERROR;
    // No more data left to decode. Bail.
    if (_bNoMoreData) return DEWDEC_DECODE_EOD;

    ZeroMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), CHUNKSIZE);
    dwCumulDecoded = _dwOverflow;
    // Copy whatever was left (dwOverflow)
    if (_dwOverflow > 0)
        CopyMemory(&(_lpDecodedData[_dwCurrentIndex * CHUNKSIZE]), _lpDecodedChunk, _dwOverflow);
    // Flush the decoding chunk just to be extra sure
    ZeroMemory(_lpDecodedChunk, _VORBISCHUNK);
    do
    {
        iRet = ov_read(&_OVF, (char*)_lpDecodedChunk, _VORBISCHUNK, 0, 2, 1, &iBitStream);
        if (_bVBR) // Valid data decoded
        {
            iInstBR = ov_bitrate_instant(&_OVF);
            if (iInstBR > 0) _dwBitrate = (DWORD)iInstBR;
        }
        if (iRet <= 0) // No data decoded. Indicates the end of stream or something messed up. Bail.
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
        else if (iRet < 0) 
        {
            _bNoMoreData = TRUE;
            return DEWDEC_DECODE_EOD;
        }
        dwDecoded = (DWORD)iRet;
        if ((dwCumulDecoded + dwDecoded) > CHUNKSIZE)
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
    _bNoMoreData = TRUE;
    return DEWDEC_DECODE_ERROR;
}

// Use the library provided function to seek to the given position
BYTE VORBISENGINE::_Seek(DWORD dwMS)
{
    if (dwMS > _dwDuration) dwMS = _dwDuration;
    if (_dwDuration <= 0) return DEWERR_SUCCESS;
    if (_OVF.seekable == 1)
    {
        double      dSeekSample;
        dSeekSample = ((double)dwMS / (double)_dwDuration) * (double)_nTotalSamples;
        
        if (ov_pcm_seek(&_OVF, (ogg_int64_t)dSeekSample) == 0)
            return DEWERR_SUCCESS;
        else
            return DEWERR_MM_ACTION;
    }
    return DEWERR_MM_ACTION;
}

// Set the media type and reserve the maximum advised 'decodable' memory
// to handle the decoding overflow
VORBISENGINE::VORBISENGINE()
{
    _btMediaType = DEWMT_VORBIS;
    _lpDecodedChunk = (LPBYTE)LocalAlloc(LPTR, _VORBISCHUNK);
    StringCchPrintf(_wsLibrary, MAX_PATH, L"libvorbis/vorbisfile version 1.3.6");
    _bVBR = TRUE;
}

// The actual function to load the file
BYTE VORBISENGINE::Load(HWND notificationWindow, LPCWSTR srcFile)
{
    vorbis_info     *pVI;
    wchar_t         wsValue[EXINFO_VAL_SIZE];
    MMRESULT        mmr = 0;

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcFile) return DEWERR_INVALID_FILE;

    // If this is an overloaded call, it will not contain the actual file name
    if (!_bOverloadedLoadFunctionCall)
        StringCchPrintf(_wsSrcFile, MAX_CHAR_PATH, L"%s", srcFile);
    _hWndNotify = notificationWindow;

    ZeroMemory(&_OVF, sizeof(OggVorbis_File));
    // Open the actual file if not in memory mode
    if (!_bOverloadedLoadFunctionCall)
        _wfopen_s(&_fp, _wsSrcFile, L"rb");
    else
        _wfopen_s(&_fp, srcFile, L"rb");
    if (!_fp) return DEWERR_FILE_READ;
    if (ov_open(_fp, &_OVF, NULL, 0) != 0) return DEWERR_ENGINE_STARTUP;
    pVI = ov_info(&_OVF, -1);
    if (!pVI) return DEWERR_ENGINE_STARTUP;
    _btChannels = (BYTE)pVI->channels;
    _dwSampleRate = (DWORD)pVI->rate;
    _nTotalSamples = ov_pcm_total(&_OVF, -1);
    _dwDuration = (DWORD)((double)_nTotalSamples / ((double)(pVI->rate) / 1000.0f));
    _dwBitrate = (DWORD)ov_bitrate(&_OVF, -1);

    _wfex.wFormatTag = WAVE_FORMAT_PCM;
    _wfex.cbSize = 0;
    _wfex.nChannels = (WORD)_btChannels;
    _wfex.nSamplesPerSec = _dwSampleRate;
    _wfex.wBitsPerSample = 16;
    _wfex.nBlockAlign = (_wfex.nChannels * _wfex.wBitsPerSample) / 8;
    _wfex.nAvgBytesPerSec = _wfex.nSamplesPerSec * _wfex.nBlockAlign;

    _dwExInfoLength = 4;
    _lpExInfo = (LPEXTENDEDINFO)LocalAlloc(LPTR, sizeof(EXTENDEDINFO) * _dwExInfoLength);
    _lpExInfo[0].SetType(L"Vorbis Encoder Version");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%d", pVI->version);
    _lpExInfo[0].SetValue(wsValue);
    
    _lpExInfo[1].SetType(L"Logical Bitstream Count");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%ld", ov_streams(&_OVF));
    _lpExInfo[1].SetValue(wsValue);

    _lpExInfo[2].SetType(L"Average Bitrate");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%.2f Kbps", (((double)(pVI->bitrate_nominal)) / 1000.000));
    _lpExInfo[2].SetValue(wsValue);

    _lpExInfo[3].SetType(L"Bitrate range");
    StringCchPrintf(wsValue, EXINFO_VAL_SIZE, L"%.2f Kbps - %.2f Kbps",
                    (((double)(pVI->bitrate_lower)) / 1000.000), (((double)(pVI->bitrate_upper)) / 1000.000) );
    _lpExInfo[3].SetValue(wsValue);

    _btStatus = DEWS_MEDIA_LOADED;
    return DEWERR_SUCCESS;
}

// Indirect function. We'll create a temp file and then call the overloaded load method
BYTE VORBISENGINE::Load(HWND notificationWindow, LPBYTE srcDataBytes, DWORD dataSize)
{
    HANDLE      hTmpFile = NULL;

    // Sanity checks
    if (notificationWindow == NULL || notificationWindow == INVALID_HANDLE_VALUE) return DEWERR_INVALID_PARAM;
    if (!srcDataBytes) return DEWERR_INVALID_PARAM;
    if (dataSize <= 0x00) return DEWERR_INVALID_PARAM;

    hTmpFile = CreateFile(_wsInternalTemp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTmpFile == NULL || hTmpFile == INVALID_HANDLE_VALUE) return DEWERR_ENGINE_STARTUP;
    WriteFile(hTmpFile, srcDataBytes, dataSize, &_dwSrcDataSize, NULL);
    CloseHandle(hTmpFile);

    _bOverloadedLoadFunctionCall = TRUE;
    return this->Load(notificationWindow, _wsInternalTemp);
}

// Use the current sample position and deduce the time
DWORD VORBISENGINE::Tell()
{
    ogg_int64_t nCurrentPos;

    if (_OVF.seekable == 1)
    {
        nCurrentPos = ov_pcm_tell(&_OVF);
        return (DWORD)(((double)nCurrentPos / (double)_nTotalSamples) * (double)_dwDuration);
    }
    return 0;
}

// Housekeeping and cleanup
VORBISENGINE::~VORBISENGINE()
{
    this->_Cleanup();
    return;
}

// Isolated static method to quickly dabble and see if
// a particular file is ogg/vorbis compatible
BOOL VORBISENGINE::IsLoadable(LPCWSTR testFile)
{
    FILE            *fpTmpVorbis = NULL;
    OggVorbis_File  tmpOVF { 0 };
    BOOL            bIsOK = FALSE;

    if (!testFile) return FALSE;
    _wfopen_s(&fpTmpVorbis, testFile, L"rb");
    if (!fpTmpVorbis) return DEWERR_FILE_READ;
    bIsOK = (ov_open(fpTmpVorbis, &tmpOVF, NULL, 0) != 0);
    fclose(fpTmpVorbis);
    return bIsOK;
}
